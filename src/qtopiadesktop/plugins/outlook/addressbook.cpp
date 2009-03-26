/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/
#include "outlooksync.h"
#include "trace.h"

#include <QBuffer>
#include <QXmlStreamReader>
#include <QFile>

class OutlookAddressbookSync : public OutlookSyncPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(OutlookAddressbookSync,OutlookSyncPlugin)
public:
    QString displayName() { return tr("Outlook Contacts"); }

    QString dataset() { return "contacts"; }
    QByteArray referenceSchema() { return "<Contact><Identifier/><NameTitle/><FirstName pronunciation=\"\"/><MiddleName/><LastName pronunciation=\"\"/><Suffix/><Company pronunciation=\"\"/><BusinessWebpage/><JobTitle/><Department/><Office/><Profession/><Assistant/><Manager/><Spouse/><Nickname/><Children/><Birthday/><Anniversary/><Notes/><Gender/><Addresses/><PhoneNumbers/><EmailAddresses/><Categories/></Contact>"; }

    Outlook::OlDefaultFolders folderEnum() { return Outlook::olFolderContacts; }
    Outlook::OlItemType itemEnum() { return Outlook::olContactItem; }
    bool isValidObject( IDispatchPtr dispatch )
    {
        TRACE(OutlookSyncPlugin) << "OutlookAddressbookSync::isValidObject";
        Outlook::_ContactItemPtr item( dispatch );
        LOG() << "The item class is" << dump_item_class(item->GetClass()) << "expecting" << dump_item_class(Outlook::olContact);
        return ( item->GetClass() == Outlook::olContact );
    }

    void getProperties( IDispatchPtr dispatch, QString &entryid, QDateTime &lastModified )
    {
        TRACE(OutlookSyncPlugin) << "OutlookAddressbookSync::getProperties";
        Outlook::_ContactItemPtr item( dispatch );
        try {
            entryid = bstr_to_qstring(item->GetEntryID());
        } catch (...) {
            entryid = QString();
            return;
        }
        lastModified = date_to_qdatetime(item->GetLastModificationTime());
    }

    void dump_item( IDispatchPtr dispatch, QXmlStreamWriter &stream )
    {
        TRACE(OutlookSyncPlugin) << "OutlookAddressbookSync::dump_item";
        Q_ASSERT( dispatch );
        Outlook::_ContactItemPtr item( dispatch );
        Outlook::UserPropertiesPtr props = item->GetUserProperties();
        Q_ASSERT(props);

        PREPARE_MAPI(Contact);

        stream.writeStartElement("Contact");
        DUMP_STRING(Identifier,EntryID);
        DUMP_STRING(NameTitle,Title);
        DUMP_STRING_ATTRIB(FirstName,FirstName,pronunciation,bstr_to_qstring(item->GetYomiFirstName()));
        DUMP_STRING(MiddleName,MiddleName);
        DUMP_STRING_ATTRIB(LastName,LastName,pronunciation,bstr_to_qstring(item->GetYomiLastName()));
        DUMP_STRING(Suffix,Suffix);
        DUMP_STRING_ATTRIB(Company,CompanyName,pronunciation,bstr_to_qstring(item->GetYomiCompanyName()));
        DUMP_STRING(BusinessWebpage,BusinessHomePage);
        DUMP_STRING(JobTitle,JobTitle);
        DUMP_STRING(Department,Department);
        DUMP_STRING(Office,OfficeLocation);
        DUMP_STRING(Profession,Profession);
        DUMP_STRING(Assistant,AssistantName);
        DUMP_STRING(Manager,ManagerName);
        DUMP_STRING(Spouse,Spouse);
        DUMP_STRING(Nickname,NickName);
        DUMP_STRING(Children,Children);
        DUMP_DATE(Birthday,Birthday);
        DUMP_DATE(Anniversary,Anniversary);
        DUMP_MAPI(Notes,Body);
        stream.writeStartElement("Gender");
        DUMP_ENUM(Gender,Gender,Outlook::olUnspecified,UnspecifiedGender);
        DUMP_ENUM(Gender,Gender,Outlook::olMale,Male);
        DUMP_ENUM(Gender,Gender,Outlook::olFemale,Female);
        stream.writeEndElement();
        stream.writeStartElement("Addresses");
        stream.writeAttribute("maxItems", "2");
        stream.writeStartElement("Address");
        stream.writeAttribute("type", "Home");
        DUMP_STRING(Street,HomeAddressStreet);
        DUMP_STRING(City,HomeAddressCity);
        DUMP_STRING(State,HomeAddressState);
        DUMP_STRING(Zip,HomeAddressPostalCode);
        DUMP_STRING(Country,HomeAddressCountry);
        stream.writeEndElement();
        stream.writeStartElement("Address");
        stream.writeAttribute("type", "Business");
        DUMP_STRING(Street,BusinessAddressStreet);
        DUMP_STRING(City,BusinessAddressCity);
        DUMP_STRING(State,BusinessAddressState);
        DUMP_STRING(Zip,BusinessAddressPostalCode);
        DUMP_STRING(Country,BusinessAddressCountry);
        stream.writeEndElement();
        stream.writeEndElement();
        stream.writeStartElement("PhoneNumbers");
        stream.writeAttribute("maxItems", "7");
        DUMP_STRING_ATTRIB(Number,HomeTelephoneNumber,type,"HomePhone");
        DUMP_STRING_ATTRIB(Number,Home2TelephoneNumber,type,"HomeMobile");
        DUMP_STRING_ATTRIB(Number,HomeFaxNumber,type,"HomeFax");
        DUMP_STRING_ATTRIB(Number,BusinessTelephoneNumber,type,"BusinessPhone");
        DUMP_STRING_ATTRIB(Number,MobileTelephoneNumber,type,"BusinessMobile");
        DUMP_STRING_ATTRIB(Number,BusinessFaxNumber,type,"BusinessFax");
        DUMP_STRING_ATTRIB(Number,PagerNumber,type,"BusinessPager");
        stream.writeEndElement();
        stream.writeStartElement("EmailAddresses");
        stream.writeAttribute("maxItems", "3");
        DUMP_MAPI(Email,Email1Address);
        DUMP_MAPI(Email,Email2Address);
        DUMP_MAPI(Email,Email3Address);
        stream.writeEndElement();
        stream.writeStartElement("Categories");
        foreach ( const QString &category, bstr_to_qstring(item->GetCategories()).split(", ", QString::SkipEmptyParts) )
            DUMP_EXPR(Category,category);
        stream.writeEndElement();
        stream.writeEndElement();
    }

    QString read_item( IDispatchPtr dispatch, const QByteArray &record )
    {
        TRACE(OutlookSyncPlugin) << "OutlookAddressbookSync::read_item";
        Q_ASSERT( dispatch );
        Outlook::_ContactItemPtr item( dispatch );
        Outlook::UserPropertiesPtr props = item->GetUserProperties();
        Q_ASSERT(props);

        enum State {
            Idle, HomeAddress, BusinessAddress, Categories
        };
        State state = Idle;
        int emailnumber = 1;

        QXmlStreamReader reader(record);
        QString key;
        QXmlStreamAttributes attributes;
        QString value;
        QStringList categories;
        while (!reader.atEnd()) {
            switch(reader.readNext()) {
                case QXmlStreamReader::StartElement:
                    key = reader.qualifiedName().toString();
                    value = QString();
                    attributes = reader.attributes();
                    if ( key == "Address" ) {
                        if ( attributes.value("type") == "Home" )
                            state = HomeAddress;
                        else if ( attributes.value("type") == "Business" )
                            state = BusinessAddress;
                    }
                    if ( key == "Categories" )
                        state = Categories;
                    // This tag replaces all of our phone numbers
                    if ( key == "PhoneNumbers" ) {
                        item->PutHomeTelephoneNumber( qstring_to_bstr("") );
                        item->PutHome2TelephoneNumber( qstring_to_bstr("") );
                        item->PutHomeFaxNumber( qstring_to_bstr("") );
                        item->PutBusinessTelephoneNumber( qstring_to_bstr("") );
                        item->PutMobileTelephoneNumber( qstring_to_bstr("") );
                        item->PutBusinessFaxNumber( qstring_to_bstr("") );
                        item->PutPagerNumber( qstring_to_bstr("") );
                    }
                    // This tag replaces all of our email addresses
                    if ( key == "EmailAddresses" ) {
                        item->PutEmail1Address( qstring_to_bstr("") );
                        item->PutEmail2Address( qstring_to_bstr("") );
                        item->PutEmail3Address( qstring_to_bstr("") );
                    }
                    // This tag replaces all of our addresses
                    if ( key == "Addresses" ) {
                        item->PutHomeAddressStreet( qstring_to_bstr("") );
                        item->PutHomeAddressCity( qstring_to_bstr("") );
                        item->PutHomeAddressState( qstring_to_bstr("") );
                        item->PutHomeAddressPostalCode( qstring_to_bstr("") );
                        item->PutHomeAddressCountry( qstring_to_bstr("") );
                        item->PutBusinessAddressStreet( qstring_to_bstr("") );
                        item->PutBusinessAddressCity( qstring_to_bstr("") );
                        item->PutBusinessAddressState( qstring_to_bstr("") );
                        item->PutBusinessAddressPostalCode( qstring_to_bstr("") );
                        item->PutBusinessAddressCountry( qstring_to_bstr("") );
                    }
                    break;
                case QXmlStreamReader::Characters:
                    value += reader.text().toString();
                    break;
                case QXmlStreamReader::EndElement:
                    key = reader.qualifiedName().toString();
                    READ_STRING(NameTitle,Title);
                    READ_STRING_ATTRIB(FirstName,FirstName,pronunciation,YomiFirstName);
                    READ_STRING(MiddleName,MiddleName);
                    READ_STRING_ATTRIB(LastName,LastName,pronunciation,YomiLastName);
                    READ_STRING(Suffix,Suffix);
                    READ_STRING_ATTRIB(Company,CompanyName,pronunciation,YomiCompanyName);
                    READ_STRING(BusinessWebpage,BusinessHomePage);
                    READ_STRING(JobTitle,JobTitle);
                    READ_STRING(Department,Department);
                    READ_STRING(Office,OfficeLocation);
                    READ_STRING(Profession,Profession);
                    READ_STRING(Assistant,AssistantName);
                    READ_STRING(Manager,ManagerName);
                    READ_STRING(Spouse,Spouse);
                    READ_STRING(Nickname,NickName);
                    READ_STRING(Children,Children);
                    READ_DATE(Birthday,Birthday);
                    READ_DATE(Anniversary,Anniversary);
                    READ_STRING(Notes,Body);
                    READ_ENUM(Gender,Gender,Outlook::olUnspecified,UnspecifiedGender);
                    READ_ENUM(Gender,Gender,Outlook::olMale,Male);
                    READ_ENUM(Gender,Gender,Outlook::olFemale,Female);
                    if ( (state == HomeAddress || state == BusinessAddress) && key == "Address" ) {
                        state = Idle;
                    }
                    if ( state == HomeAddress ) {
                        READ_STRING(Street,HomeAddressStreet);
                        READ_STRING(City,HomeAddressCity);
                        READ_STRING(State,HomeAddressState);
                        READ_STRING(Zip,HomeAddressPostalCode);
                        READ_STRING(Country,HomeAddressCountry);
                    }
                    if ( state == BusinessAddress ) {
                        READ_STRING(Street,BusinessAddressStreet);
                        READ_STRING(City,BusinessAddressCity);
                        READ_STRING(State,BusinessAddressState);
                        READ_STRING(Zip,BusinessAddressPostalCode);
                        READ_STRING(Country,BusinessAddressCountry);
                    }
                    if ( key == "Number" ) {
                        QStringRef v = attributes.value( "type" );
                        if ( v == "HomePhone" )
                            item->PutHomeTelephoneNumber( qstring_to_bstr(value) );
                        else if ( v == "HomeMobile" )
                            item->PutHome2TelephoneNumber( qstring_to_bstr(value) );
                        else if ( v == "HomeFax" )
                            item->PutHomeFaxNumber( qstring_to_bstr(value) );
                        else if ( v == "BusinessPhone" )
                            item->PutBusinessTelephoneNumber( qstring_to_bstr(value) );
                        else if ( v == "BusinessMobile" )
                            item->PutMobileTelephoneNumber( qstring_to_bstr(value) );
                        else if ( v == "BusinessFax" )
                            item->PutBusinessFaxNumber( qstring_to_bstr(value) );
                        else if ( v == "BusinessPager" )
                            item->PutPagerNumber( qstring_to_bstr(value) );
                    }
                    if ( key == "Email" ) {
                        switch ( emailnumber ) {
                            case 1:
                                READ_STRING(Email,Email1Address);
                                break;
                            case 2:
                                READ_STRING(Email,Email2Address);
                                break;
                            case 3:
                                READ_STRING(Email,Email3Address);
                                break;
                            default:
                                break;

                        }
                        emailnumber++;
                    }
                    if ( state == Categories ) {
                        if ( key == "Category" )
                            categories << value;
                        if ( key == "Categories" ) {
                            item->PutCategories( qstring_to_bstr(categories.join(", ")) );
                            state = Idle;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        LOG() << "item->Save()";
        item->Save();
        LOG() << "item->GetEntryID()";
        return bstr_to_qstring(item->GetEntryID());
    }

    void delete_item( IDispatchPtr dispatch )
    {
        Q_ASSERT( dispatch );
        Outlook::_ContactItemPtr item( dispatch );
        item->Delete();
    }
};

QD_REGISTER_PLUGIN(OutlookAddressbookSync);

#include "addressbook.moc"
