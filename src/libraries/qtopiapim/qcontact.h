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

#ifndef QCONTACT_H
#define QCONTACT_H

#include <qpimrecord.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

#include <QSharedData>
#include <QPixmap>
#include <QDate>
#include <QMap>
#include <QStringList>
#include <QFile>

struct VObject;
class QContactData;
class QContactModel;
class QIcon;

struct QTOPIAPIM_EXPORT QContactAddress
{
    QString street;
    QString city;
    QString state;
    QString zip;
    QString country;

    bool isEmpty() const {
        return street.trimmed().isEmpty()
                && city.trimmed().isEmpty()
                && state.trimmed().isEmpty()
                && zip.trimmed().isEmpty()
                && country.trimmed().isEmpty();
    }

    bool operator==(const QContactAddress &other) const
    {
        if (street != other.street)
            return false;
        if (city != other.city)
            return false;
        if (state != other.state)
            return false;
        if (zip != other.zip)
            return false;
        if (country != other.country)
            return false;
        return true;
    }
};

class QTOPIAPIM_EXPORT QContact : public QPimRecord
{
public:
    QContact();
    QContact(const QContact &);
    virtual ~QContact();

    enum Location {
        /* base */
        Other = 0x00,
        Home = 0x01,
        Business = 0x02
    };

    /* Don't even THINK about using this as a bitmask */
    enum PhoneType {
        /* phone */
        OtherPhone = 0x00, // other land line
        HomePhone = 0x01, // home phone (not mobile, fax or pager
        BusinessPhone = 0x02, // work phone

        Mobile = 0x0100, // as in other MobilePhone
        HomeMobile = 0x0101,
        BusinessMobile = 0x0102,

        Fax = 0x0200, // as in other Fax
        HomeFax = 0x0201,
        BusinessFax = 0x0202,

        VOIP = 0x0300,
        HomeVOIP = 0x0301,
        BusinessVOIP = 0x0302,

        Pager = 0x0400, // as in other Pager
        HomePager = 0x0401,
        BusinessPager = 0x0402
    };

    QContact &operator=(const QContact &other);

    bool operator==(const QContact &other) const;
    bool operator!=(const QContact &other) const;

    static bool writeVCard( QIODevice *, const QList<QContact> & );
    static bool writeVCard( QIODevice *, const QContact & );
    static QList<QContact> readVCard( QIODevice * );

    /* deprecated - keep for source compatibility */
    static QList<QContact> readVCard( const QString &filename );
    static QList<QContact> readVCard( const QByteArray &vcard );
    static QList<QContact> readVCard( VObject* vobject );
    static bool writeVCard( const QString &filename, const QList<QContact> &contacts);
    bool writeVCard( const QString &filename ) const;
    void writeVCard( QFile &file ) const;
    void writeVCard( QDataStream *stream ) const;

    void setNameTitle( const QString &v ) { replace( NameTitle, v ); }
    void setFirstName( const QString &v ) { replace( FirstName, v ); }
    void setMiddleName( const QString &v ) { replace( MiddleName, v ); }
    void setLastName( const QString &v ) { replace( LastName, v ); }
    void setSuffix( const QString &v ) { replace( Suffix, v ); }
    void setFirstNamePronunciation( const QString &v ) { replace( FirstNamePronunciation, v ); }
    void setLastNamePronunciation( const QString &v ) { replace( LastNamePronunciation, v ); }
    void setCompanyPronunciation( const QString &v ) { replace( CompanyPronunciation, v ); }

    // default email address
    void setDefaultEmail( const QString &v );

    // inserts email to list and ensure's doesn't already exist
    void insertEmail( const QString &v );
    void removeEmail( const QString &v );
    void clearEmailList();
    void setEmailList( const QStringList &v );

    void setPhoneNumber(PhoneType, const QString &);
    void setAddress(Location, const QContactAddress &);

    // home
    void setHomeStreet( const QString &v );
    void setHomeCity( const QString &v );
    void setHomeState( const QString &v );
    void setHomeZip( const QString &v );
    void setHomeCountry( const QString &v );
    void setHomePhone( const QString &v ) { setPhoneNumber(HomePhone, v); }
    void setHomeFax( const QString &v ) { setPhoneNumber(HomeFax, v); }
    void setHomeMobile( const QString &v ) { setPhoneNumber(HomeMobile, v); }
    void setHomeVOIP( const QString &v ) { setPhoneNumber(HomeVOIP, v); }

    void setHomeWebpage( const QString &v ) { replace( HomeWebPage, v ); }

    // business
    void setCompany( const QString &v ) { replace( Company, v ); }
    void setBusinessStreet( const QString &v );

    void setBusinessCity( const QString &v );
    void setBusinessState( const QString &v );
    void setBusinessZip( const QString &v );
    void setBusinessCountry( const QString &v );

    void setBusinessWebpage( const QString &v ) { replace( BusinessWebPage, v ); }
    void setJobTitle( const QString &v ) { replace( JobTitle, v ); }
    void setDepartment( const QString &v ) { replace( Department, v ); }
    void setOffice( const QString &v ) { replace( Office, v ); }

    void setBusinessPhone( const QString &v ) { setPhoneNumber(BusinessPhone, v); }
    void setBusinessFax( const QString &v ) { setPhoneNumber(BusinessFax, v); }
    void setBusinessMobile( const QString &v ) { setPhoneNumber(BusinessMobile, v); }
    void setBusinessPager( const QString &v ) { setPhoneNumber(BusinessPager, v); }
    void setBusinessVOIP( const QString &v) { setPhoneNumber(BusinessVOIP, v); }

    void setProfession( const QString &v ) { replace( Profession, v ); }
    void setAssistant( const QString &v ) { replace( Assistant, v ); }
    void setManager( const QString &v ) { replace( Manager, v ); }

    // personal
    void setSpouse( const QString &v ) { replace( Spouse, v ); }
    enum GenderType { UnspecifiedGender=0, Male, Female };
    void setGender( GenderType g );
    void setBirthday( const QDate &d );
    void setAnniversary( const QDate &v );
    void setNickname( const QString &v ) { replace( Nickname, v ); }
    void setChildren( const QString &v ) { replace( Children, v ); }

    // other
    void setNotes( const QString &v );
    void setPortraitFile( const QString &v );

    void changePortrait( const QPixmap &p );
    void changePortrait( const QImage &p );
    void changePortrait( const QString &p, const QRect & = QRect() );

    bool match( const QString &regexp ) const;
    bool match( const QRegExp &regexp ) const;

    // name
    QString nameTitle() const { return find( NameTitle ); }
    QString firstName() const { return find( FirstName ); }
    QString middleName() const { return find( MiddleName ); }
    QString lastName() const { return find( LastName ); }
    QString suffix() const { return find( Suffix ); }
    QString lastNamePronunciation() const { return find( LastNamePronunciation ); }
    QString firstNamePronunciation() const { return find( FirstNamePronunciation ); }
    QString companyPronunciation() const { return find( CompanyPronunciation ); }

    static void setLabelFormat(const QString &);
    static QString labelFormat();
    static QStringList labelFields();

    QString label() const;

    // email
    QString defaultEmail() const { return find( DefaultEmail ); }
    QStringList emailList() const;

    QContactAddress address(Location) const;
    // home
    QString homeStreet() const { return address(Home).street; }
    QString homeCity() const { return address(Home).city; }
    QString homeState() const { return address(Home).state; }
    QString homeZip() const { return address(Home).zip; }
    QString homeCountry() const { return address(Home).country; }

    static QList<PhoneType> phoneTypes();
    static QIcon phoneIcon(PhoneType);
    static QString phoneIconResource(PhoneType);

    QString phoneNumber(PhoneType) const;

    QString homePhone() const { return phoneNumber(HomePhone); }
    QString homeFax() const { return phoneNumber(HomeFax); }
    QString homeMobile() const { return phoneNumber(HomeMobile); }
    QString homeVOIP() const { return phoneNumber(HomeVOIP); }

    QString homeWebpage() const { return find( HomeWebPage ); }
    // Multi line string containing all non-empty address info in the form
    // Street
    // City, State Zip
    // Country
    QString displayHomeAddress() const { return displayAddress(Home); }
    QString displayAddress(Location) const;

    // business
    QString company() const { return find( Company ); }

    QString businessStreet() const { return address(Business).street; }
    QString businessCity() const { return address(Business).city; }
    QString businessState() const { return address(Business).state; }
    QString businessZip() const { return address(Business).zip; }
    QString businessCountry() const { return address(Business).country; }

    QString businessWebpage() const { return find( BusinessWebPage ); }
    QString jobTitle() const { return find( JobTitle ); }
    QString department() const { return find( Department ); }
    QString office() const { return find( Office ); }

    QString businessPhone() const { return phoneNumber(BusinessPhone); }
    QString businessFax() const { return phoneNumber(BusinessFax); }
    QString businessMobile() const { return phoneNumber(BusinessMobile); }
    QString businessVOIP() const { return phoneNumber(BusinessVOIP); }
    QString businessPager() const { return phoneNumber(BusinessPager); }

    QString profession() const { return find( Profession ); }
    QString assistant() const { return find( Assistant ); }
    QString manager() const { return find( Manager ); }
    // Multi line string containing all non-empty address info in the form
    // Street
    // City, State Zip
    // Country
    QString displayBusinessAddress() const { return displayAddress(Business); }

    QMap<PhoneType, QString> phoneNumbers() const;
    QMap<Location, QContactAddress> addresses() const;

    void setPhoneNumbers(const QMap<PhoneType, QString> &);
    void setAddresses(const QMap<Location, QContactAddress> &);
    void clearPhoneNumbers();
    void clearAddresses();

    QString defaultPhoneNumber() const;

    void setDefaultPhoneNumber(const QString &);
    void setDefaultPhoneNumber(PhoneType);

    //personal
    QString spouse() const { return find( Spouse ); }
    GenderType gender() const;
    QDate birthday() const;
    QDate anniversary() const;
    QString nickname() const { return find( Nickname ); }
    QString children() const { return find( Children ); }

    // other
    QString notes() const { return find( Notes ); }
    QString portraitFile() const { return find( Portrait ); }
    //QString groups() const { return find( Groups ); }
    //QStringList groupList() const;

    QString toRichText() const;

    static QContact parseLabel(const QString &, const QContact &contact = QContact());

    QPixmap portrait() const;
    QIcon icon() const;
    QPixmap thumbnail() const;
    static QSize portraitSize();
    static QSize thumbnailSize();

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

protected:
    VObject *createVObject() const;
    QString displayAddress( const QString &street,
                            const QString &city,
                            const QString &state,
                            const QString &zip,
                            const QString &country ) const;

    QUniqueId &uidRef();
    const QUniqueId &uidRef() const;

    QList<QString> &categoriesRef();
    const QList<QString> &categoriesRef() const;

    QMap<QString, QString> &customFieldsRef();
    const QMap<QString, QString> &customFieldsRef() const;

private:
    void removeExistingPortrait();
    void saveScaledPortrait(const QImage &);

    enum ContactFields {
        NameTitle,
        FirstName,
        MiddleName,
        LastName,
        Suffix,

        JobTitle,
        Department,
        Company,

        // email
        DefaultEmail,

        // business
        BusinessWebPage,

        Office,
        Profession,
        Assistant,
        Manager,

        // home
        HomeWebPage,

        //personal
        Spouse,
        Nickname,
        Children,

        // other
        Portrait,
        Notes,

        // maps to YomiLastName, or is at least intended to
        LastNamePronunciation,
        FirstNamePronunciation,
        CompanyPronunciation,
    };

    void insert( int key, const QString &value );
    void replace( int key, const QString &value );
    QString find( int key ) const;

    static QIcon &corporationIcon();
    static QIcon &genericIcon();
    static QIcon &personalIcon();

    QSharedDataPointer<QContactData> d;
};

Q_DECLARE_USER_METATYPE(QContact)

#endif
