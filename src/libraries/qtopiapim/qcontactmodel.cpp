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
#include <qcontactmodel.h>
#include "qcontactsqlio_p.h"
#include "vobject_p.h"
#include <qtopianamespace.h>
#ifdef QTOPIA_CELL
#include "qsimcontext_p.h"
#endif
#include <QSettings>
#include <QPainter>
#include <QMap>
#include <QSet>
#include <QPixmap>
#include <QGlobalPixmapCache>
#include <QImageReader>
#include <QFile>
#include <QTextDocument>

#include <QDebug>

#include "qtopiaipcenvelope.h"
#include "qtopiachannel.h"

QMap<QContactModel::Field, QString> QContactModel::k2i;
QMap<QString, QContactModel::Field> QContactModel::i2k;
QMap<QContactModel::Field, QString>  QContactModel::k2t;

/*!
  \internal

  Initializes mappings from column enums to translated and non-translated strings.
*/
void QContactModel::initMaps()
{
    if (k2t.count() > 0)
        return;
    struct KeyLookup {
        const char* ident;
        const char* trans;
        Field key;
    };
    static const KeyLookup l[] = {
        // name
        { "title", QT_TR_NOOP("Title"), NameTitle },
        { "firstname", QT_TR_NOOP( "First Name" ), FirstName },
        { "middlename", QT_TR_NOOP( "Middle Name" ), MiddleName },
        { "lastname", QT_TR_NOOP( "Last Name" ), LastName },
        { "suffix", QT_TR_NOOP( "Suffix" ), Suffix },
        { "label", QT_TR_NOOP( "Label" ), Label },

        // email
        { "defaultemail", QT_TR_NOOP( "Default Email" ), DefaultEmail },
        { "emails", QT_TR_NOOP( "Emails" ), Emails },

        { "defaultphone", QT_TR_NOOP( "Default Phone" ), DefaultPhone },
        // other
        { "otherphone", QT_TR_NOOP( "Home Phone" ), OtherPhone },
        { "otherfax", QT_TR_NOOP( "Home Fax" ), OtherFax},
        { "othermobile", QT_TR_NOOP( "Home Mobile" ), OtherMobile},
        { "otherpager", QT_TR_NOOP( "Home Pager" ), OtherPager},

        // home
        { "homestreet", QT_TR_NOOP( "Home Street" ), HomeStreet },
        { "homecity", QT_TR_NOOP( "Home City" ), HomeCity },
        { "homestate", QT_TR_NOOP( "Home State" ), HomeState },
        { "homezip", QT_TR_NOOP( "Home Zip" ), HomeZip },
        { "homecountry", QT_TR_NOOP( "Home Country" ), HomeCountry },
        { "homephone", QT_TR_NOOP( "Home Phone" ), HomePhone },
        { "homevoip", QT_TR_NOOP( "Home VOIP" ), HomeVOIP },
        { "homefax", QT_TR_NOOP( "Home Fax" ), HomeFax},
        { "homemobile", QT_TR_NOOP( "Home Mobile" ), HomeMobile},
        { "homepager", QT_TR_NOOP( "Home Pager" ), HomePager},
        { "homewebpage", QT_TR_NOOP( "Home Web Page" ), HomeWebPage},

        // business
        { "company", QT_TR_NOOP( "Company" ), Company},
        { "businessstreet", QT_TR_NOOP( "Business Street" ), BusinessStreet},
        { "businesscity", QT_TR_NOOP( "Business City" ), BusinessCity},
        { "businessstate", QT_TR_NOOP( "Business State" ), BusinessState},
        { "businesszip", QT_TR_NOOP( "Business Zip" ), BusinessZip},
        { "businesscountry", QT_TR_NOOP( "Business Country" ), BusinessCountry},
        { "businesswebPage", QT_TR_NOOP( "Business Web Page" ), BusinessWebPage},
        { "jobtitle", QT_TR_NOOP( "Job Title" ), JobTitle},
        { "department", QT_TR_NOOP( "Department" ), Department},
        { "office", QT_TR_NOOP( "Office" ), Office},
        { "businessphone", QT_TR_NOOP( "Business Phone" ), BusinessPhone},
        { "businessfax", QT_TR_NOOP( "Business Fax" ), BusinessFax},
        { "businessmobile", QT_TR_NOOP( "Business Mobile" ), BusinessMobile},
        { "businessvoip", QT_TR_NOOP( "Business VOIP" ), BusinessVOIP},
        { "businesspager", QT_TR_NOOP( "Business Pager" ), BusinessPager},
        { "profession", QT_TR_NOOP( "Profession" ), Profession},
        { "assistant", QT_TR_NOOP( "Assistant" ), Assistant},
        { "manager", QT_TR_NOOP( "Manager" ), Manager},

        //personal
        { "spouse", QT_TR_NOOP( "Spouse" ), Spouse},
        { "gender", QT_TR_NOOP( "Gender" ), Gender},
        { "birthday", QT_TR_NOOP( "Birthday" ), Birthday},
        { "anniversary", QT_TR_NOOP( "Anniversary" ), Anniversary},
        { "nickname", QT_TR_NOOP( "Nickname" ), Nickname},
        { "children", QT_TR_NOOP( "Children" ), Children},

        // Presence
        { "presencestatus", QT_TR_NOOP( "Presence Status Type"), PresenceStatus},
        { "presencemessage", QT_TR_NOOP( "Presence Message"), PresenceMessage},
        { "presencestatusstring", QT_TR_NOOP ("Presence Status" ), PresenceStatusString},
        { "presencedisplayname", QT_TR_NOOP( "Presence Display Name" ), PresenceDisplayName},
        { "presenceupdatetime", QT_TR_NOOP( "Presence Update Time"), PresenceUpdateTime},
        { "presencecapabilities", QT_TR_NOOP( "Presence Capabilities" ), PresenceCapabilities},
        { "presenceavatar", QT_TR_NOOP( "Presence Avatar"), PresenceAvatar},

        // other
        { "notes", QT_TR_NOOP( "Notes" ), Notes},
        // next to added in 4.0
        { "photofile", QT_TR_NOOP( "Photo File" ), Portrait},

        // Added in Qtopia 1.6
        { "lastnamepronunciation", QT_TR_NOOP( "Pronunciation" ), LastNamePronunciation},
        { "firstnamepronunciation", QT_TR_NOOP( "Pronunciation" ), FirstNamePronunciation},
        { "companypronunciation", QT_TR_NOOP( "Pronunciation" ), CompanyPronunciation},
        { "identifier", QT_TR_NOOP( "Identifier" ), Identifier},
        { "categories", QT_TR_NOOP( "Categories" ), Categories},
        { 0, 0, Invalid }
    };

    const KeyLookup *k = l;
    while (k->key != Invalid) {
        k2t.insert(k->key, tr(k->trans));
        k2i.insert(k->key, k->ident);
        i2k.insert(k->ident, k->key);
        ++k;
    }
}

/*!
  Returns a translated string describing the contact model \a field.

  \sa fieldIcon(), fieldIdentifier(), identifierField()
*/
QString QContactModel::fieldLabel(Field field)
{
    if (k2t.count() == 0)
        initMaps();
    if (!k2t.contains(field))
        return QString();
    return k2t[field];
}


/*!
  Returns a non-translated string describing the contact model \a field.

  \sa fieldLabel(), fieldIcon(), identifierField()
*/
QString QContactModel::fieldIdentifier(Field field)
{
    if (k2i.count() == 0)
        initMaps();
    if (!k2i.contains(field))
        return QString();
    return k2i[field];
}

/*!
  Returns the contact model field for the non-translated field \a identifier.

  \sa fieldLabel(), fieldIcon(), fieldIdentifier()
*/
QContactModel::Field QContactModel::identifierField(const QString &identifier)
{
    if (i2k.count() == 0)
        initMaps();
    if (!i2k.contains(identifier))
        return Invalid;
    return i2k[identifier];
}

/*!
  Returns the contact model fields that represent the phone numbers for a contact.
*/
QList<QContactModel::Field> QContactModel::phoneFields()
{
    // Used by QContact::phoneNumbers.  thats why its a static function.
    QList<Field> result;
    result.append(OtherPhone);
    result.append(OtherMobile);
    result.append(OtherFax);
    //result.append(OtherPager);
    result.append(HomePhone);
    result.append(HomeMobile);
    result.append(HomeVOIP);
    result.append(HomeFax);
    //result.append(HomePager);
    result.append(BusinessPhone);
    result.append(BusinessMobile);
    result.append(BusinessVOIP);
    result.append(BusinessFax);
    result.append(BusinessPager);
    return result;
}

/*!
  Returns the contact model fields that represent the label for a contact.
*/
QList<QContactModel::Field> QContactModel::labelFields()
{
    return ContactSqlIO::labelKeys();
}

/*!
  Returns known name titles for the current language settings.  These
  are used to assist in parsing and constructing contact name information.
  Contacts are not restricted to these name titles.
*/
QStringList QContactModel::localeNameTitles()
{
    return tr( "Mr;Mrs;Ms;Miss;Dr;Prof;" ).split(";", QString::SkipEmptyParts);
}

/*!
  Returns known suffixes for the current language settings.  These
  are used to assist in parsing and constructing contact name information.
  Contacts are not restricted to these suffixes.
*/
QStringList QContactModel::localeSuffixes()
{
    return tr( "Jr;Sr;I;II;III;IV;V" ).split(";", QString::SkipEmptyParts);
}

class QContactModelData
{
public:
    QContactModelData()
        : searchModel(0), filterFlags(0)
        {
        }

    ~QContactModelData() {
        /* other pointers collected via parenting */
        if (searchModel)
            delete searchModel;
    }

    mutable QContactModel *searchModel;
    mutable QString filterText;
    mutable int filterFlags;
    mutable QList<QCollectivePresenceInfo::PresenceType> presenceFilter;

    static QUniqueId mPersonalId;
    static bool mPersonalIdRead;

    ContactSqlIO *defaultio;
    QPimSource phoneSource;
    QPimSource simSource;
#ifdef QTOPIA_CELL
    uint simContextId;
    QContactSimContext *simContext;
#endif
    static QIcon getCachedIcon(const QString& path);
    static QHash<QString, QIcon> cachedIcons;
};

QUniqueId QContactModelData::mPersonalId;
bool QContactModelData::mPersonalIdRead;
QHash<QString, QIcon> QContactModelData::cachedIcons;

QIcon QContactModelData::getCachedIcon(const QString& path)
{
    if (cachedIcons.contains(path))
        return cachedIcons.value(path);

    cachedIcons.insert(path, QIcon(path));
    return cachedIcons.value(path);
}

/*!
  Returns a icon representing the contact model \a field.

  Returns a null icon if no icon is available.

  \sa fieldLabel(), fieldIdentifier(), identifierField()
*/
QIcon QContactModel::fieldIcon(Field field)
{
    QString ident = fieldIdentifier(field);

    if (ident.isEmpty() || !QFile::exists(":image/addressbook/" + ident))
        return QIcon();

    return QContactModelData::getCachedIcon(":image/addressbook/" + ident);
}


/*!
  \class QContactModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactModel class provides access to the Contacts data.

  User contacts are represented in the contact model as a table, with each row corresponding to a
  particular contact and each column as on of the fields of the contact.  Complete QContact objects can
  be retrieved using the contact() function which takes either a row, index, or unique identifier.

  The contact model is a descendant of QAbstractItemModel, so it is suitable for use with
  the Qt View classes such as QListView and QTableView, as well as QContactView and any custom views.

  The contact model provides functions for sorting and some filtering of items.
  For filters or sorting that is not provided by the contact model it is recommended that
  QSortFilterProxyModel is used to wrap the contact model.

  A QContactModel instance will also reflect changes made in other instances of QContactModel,
  both within this application and from other applications.  This will result in
  the modelReset() signal being emitted.

  \sa QContact, QContactListView, QSortFilterProxyModel, {Pim Library}
*/

/*!
  \enum QContactModel::QContactModelRole

  Extends Qt::ItemDataRole

  \value LabelRole
    A short formatted text label of the contacts name.
  \value SubLabelRole
    A short formatted text label with supplementary contact information, like phone number, email address or company.
  \value PortraitRole
    A pixmap of the contacts image, or a default image if
    none is specifically set.
  \value StatusIconRole
    An icon providing additional information about the contact.
    For instance marking as a business contact or marking the contact
    as recently called.
*/

/*!
  \enum QContactModel::Field

  Enumerates the columns when in table mode and columns used for sorting.
  Is a subset of data retrievable from a QContact.

  \value Invalid
    An invalid field
  \value Label
    An appropriate text label for the contact
  \value NameTitle
    The contact's title, such as Mr or Dr.  \sa localeNameTitles()
  \value FirstName
    The first name of the contact
  \value MiddleName
    The middle name of the contact
  \value LastName
    The last name of the contact
  \value Suffix
    The contact's suffix, such as Jr or II.  \sa localeSuffixes()
  \value JobTitle
    The contact's job title
  \value Department
    The contact's department
  \value Company
    The contact's company
  \value BusinessPhone
    The business phone number of the contact
  \value BusinessFax
    The business fax number of the contact
  \value BusinessMobile
    The business mobile number of the contact
  \value BusinessPager
    The business pager number of the contact
  \value HomePhone
    The home phone number of the contact
  \value HomeFax
    The home fax number of the contact
  \value HomeMobile
    The home mobile number of the contact
  \value DefaultPhone
    The default phone number for the contact
  \value DefaultEmail
    The default email address for the contact
  \value Emails
    The list of email addresses for the contact
  \omitvalue OtherPhone
  \omitvalue OtherFax
  \omitvalue OtherMobile
  \omitvalue OtherPager
  \omitvalue OtherVOIP
  \omitvalue HomePager
  \value BusinessStreet
    The business street address for the contact
  \value BusinessCity
    The city for the business address of the contact
  \value BusinessState
    The state for the business address of the contact
  \value BusinessZip
    The zip code for the business address of the contact
  \value BusinessCountry
    The country for the business address of the contact
  \value BusinessWebPage
    The business web page address of the contact
  \value Office
    The contact's office
  \value Profession
    The contact's profession
  \value Assistant
    The contact's assistant
  \value Manager
    The contact's manager
  \value HomeStreet
    The home street address for the contact
  \value HomeCity
    The city for the home address of the contact
  \value HomeState
    The state for the home address of the contact
  \value HomeZip
    The zip code for the home address of the contact
  \value HomeCountry
    The country for the home address of the contact
  \value HomeWebPage
    The home web page address of the contact
  \value Spouse
    The contact's spouse
  \value Gender
    The contact's gender
  \value Birthday
    The contact's birthday
  \value Anniversary
    The contact's anniversary
  \value Nickname
    The nickname of the contact
  \value Children
    The contact's children
  \value Portrait
    The portrait of the contact
  \value Notes
    The notes relating to the contact
  \value LastNamePronunciation
    The pronunciation of the last name of the contact
  \value FirstNamePronunciation
    The pronunciation of the first name of the contact
  \value CompanyPronunciation
    The pronunciation of the contact's company
  \value Identifier
    The identifier of the contact
  \value Categories
    The list of categories the contact belongs to
  \value HomeVOIP
    The URI for the contact's home VOIP id
  \value BusinessVOIP
    The URI for the contact's business VOIP id
  \value PresenceStatus
    A map (QPresenceTypeMap) of presence URIs to presence status types for this contact
  \value PresenceStatusString
    A map (QPresenceStringMap) of presence URIs to presence status strings for this contact
  \value PresenceMessage
    A map (QPresenceStringMap) of presence URIs to presence messages for this contact
  \value PresenceDisplayName
    A map (QPresenceStringMap) of presence URIs to display names for this contact
  \value PresenceUpdateTime
    A map (QPresenceDateTimeMap) of presence URIs to last update time for this contact
  \value PresenceAvatar
    A map (QPresenceStringMap) of presence URIs to avatar file paths for this contact
  \value PresenceCapabilities
    A map (QPresenceCapabilityMap) of presence URIs to presence capabilities for this contact
 */

/*!
  \typedef QContactModel::SortField

  \brief The QContactModel::SortField structure provides sorting instructions.

  This structure holds a QContactModel field to sort by, and a sorting direction.
  Typically an ordered list of these structures is used to allows arbitrary
  sorting to be performed on a QContactModel.

  \sa QContactModel
*/

/*!
  \fn bool QContactModel::isPersonalDetails(int row) const

  Returns true if the contact at the given \a row represents the personal details of the device owner.
*/

/*!
  Constructs contact model with the given \a parent.
*/
QContactModel::QContactModel(QObject *parent)
    : QPimModel(parent)
{
    d = new QContactModelData;
    QtopiaSql::instance()->openDatabase();

    ContactSqlIO *access = new ContactSqlIO(this);
    QContactDefaultContext *context = new QContactDefaultContext(this, access);

    setAccess(access);
    addContext(context);
    d->defaultio = access;

#ifdef QTOPIA_CELL
    QContactSimContext *scon = new QContactSimContext(this, access);
    d->simSource = scon->defaultSource();
    d->simContextId = QUniqueIdGenerator::mappedContext(d->simSource.context);
    addContext(scon);
    d->simContext = scon;
    connect(scon, SIGNAL(simLoaded(const QPimSource &)),
            this, SIGNAL(simLoaded(const QPimSource &)));
#endif

    QtopiaChannel *channel = new QtopiaChannel( "QPE/PIM",  this );

    connect( channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(pimMessage(QString,QByteArray)) );
}

/*!
  Destroys the contact model.
*/
QContactModel::~QContactModel()
{
    delete d;
}

/*!
  \reimp
*/
QMimeData * QContactModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    return 0;
}

/*!
  \reimp
*/
QStringList QContactModel::mimeTypes() const
{
    return QStringList();
}

/*!
    \overload

    Sorts the model by \a column in the specified \a order.
*/
void QContactModel::sort(int column, Qt::SortOrder order)
{
    SortField pair((Field)column, order);
    QList<SortField> list;
    list << pair;
    sort(list);
}

/*!
    \fn void QContactModel::sort(QList<SortField> list)

    Sorts the model by the fields specified in \a list.  Each
    entry in the list specifies a field and an order (ascending or descending).
*/
void QContactModel::sort(QList<SortField> list)
{
    d->defaultio->setOrderBy(list);
}



/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.

  The row of the index specifies which contact to access and the column of the index is treated as a \c QContactModel::Field.

  \sa contactField()
*/
QVariant QContactModel::data(const QModelIndex &index, int role) const
{
    /* Early out if we don't understand the role to avoid fetching a contact */
    if (index.column() == Label) {
        switch(role) {
            case Qt::DecorationRole:
            case PortraitRole:
            case Qt::DisplayRole:
            case Qt::EditRole:
            case LabelRole:
            case SubLabelRole:
            case StatusIconRole:
                break;

            default:
                return QVariant();
        }
    }

    int row = index.row();
    switch(index.column()) {
        case Label:
            if (row < rowCount()){
                // later, take better advantage of roles.
                switch(role) {
                    default:
                        break;
                    case Qt::DecorationRole:
                        // We can tell if this is a SIM contact, but not much else
                        return d->defaultio->simpleContact(row).icon();
                    case PortraitRole:
                        {
                            // Don't duplicate the thumbnail logic here
                            QContact c = d->defaultio->simpleContact(row);
                            return qvariant_cast<QPixmap>(c.thumbnail());
                        }
                    case Qt::DisplayRole:
                        return d->defaultio->contactField(row, Label);
                    case Qt::EditRole:
                        return QVariant(id(row).toByteArray());
                    case LabelRole:
                        {
                            QString l = d->defaultio->contactField(row, Label).toString();
                            return "<b>" + l + "</b>";
                        }
                    case SubLabelRole:
                        {
                            int f = filterFlags();
                            QString email = d->defaultio->contactField(row, DefaultEmail).toString();
                            // XXX ContainsChat

                            // If we are set to filter in email and not phones, check the email address
                            if (((f & (ContainsEmail|ContainsPhoneNumber)) == ContainsEmail) && !email.isEmpty())
                                return Qt::escape(email);
                            QString phone = d->defaultio->contactField(row, DefaultPhone).toString();
                            if (!phone.isEmpty())
                                return Qt::escape(phone);
                            // If we have any inclination towards email, use it
                            if ((f == 0 || f & ContainsEmail) && !email.isEmpty())
                                return Qt::escape(email);

                            QString label = d->defaultio->contactField(row, Label).toString();
                            QString company = d->defaultio->contactField(row, Company).toString();
                            // Otherwise use this other stuff
                            if (label != company)
                                return Qt::escape(company);
                        }
                    case StatusIconRole:
                        if (isPersonalDetails(id(row))) {
                            QPixmap pm;
                            static const QLatin1String key("pimcontact-personaldetails-thumb");
                            if (!QGlobalPixmapCache::find(key, pm)) {
                                QIcon i(":icon/addressbook/personaldetails");
                                pm = i.pixmap(QContact::thumbnailSize());
                                if (!pm.isNull()) {
                                    QGlobalPixmapCache::insert(key, pm);
                                }
                            }
                            return qvariant_cast<QPixmap>(pm);
                        } else
                            return QPixmap();
                }
            }
            break;
        default:
            if (index.column() > 0 && index.column() < columnCount())
                return d->defaultio->contactField(row, (Field)index.column());
            break;
    }
    return QVariant();
}

/*!
  \reimp
*/
int QContactModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return PresenceAvatar + 1;// last column + 1
}

/*!
  \overload
  Sets the \a role data for the item at \a index to \a value. Returns true if successful.

  The contact model only accepts data for the \c EditRole.  The column of the specified
  index specifies the \c QContactModel::Field to set and the row of the index
  specifies which contact to modify.

  \sa setContactField()
*/
bool QContactModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    QContact c = contact(index);
    if (!setContactField(c, (Field)index.column(), value))
        return false;
    return updateContact(c);

#if 0
    /*
        disabled due to 'notifyUpdated' require whole record.
        While writing whole record is less efficient than partial - at
        this stage it was the easiest way of fixing the bug where setData
        did not result in cross-model data change from being propagated properly
   */

    int i = index.row();
    const ContactSqlIO *model = qobject_cast<const QContactIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((ContactSqlIO *)model)->setContactField(r, (Field)index.column(), value);
    return false;
#endif
}

/*!
  \reimp
*/
bool QContactModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles)
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

/*!
  \reimp
*/
QMap<int,QVariant> QContactModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> result;
    switch (index.column()) {
        case Label:
            result.insert(Qt::DecorationRole, data(index, Qt::DecorationRole));
            result.insert(PortraitRole, data(index, PortraitRole));
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            result.insert(Qt::EditRole, data(index, Qt::EditRole));
            result.insert(LabelRole, data(index, LabelRole));
            result.insert(SubLabelRole, data(index, SubLabelRole));
            result.insert(StatusIconRole, data(index, StatusIconRole));
            break;
        default:
            result.insert(Qt::DecorationRole, data(index, Qt::DecorationRole));
            break;
    }
    return result;
}

/*!
  \reimp
*/
QVariant QContactModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (section >= 0 && section < columnCount()) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
        else if (role == Qt::DecorationRole)
            return qvariant_cast<QIcon>(fieldIcon((Field)section));
    }
    return QVariant();
}

/*!
  \reimp
*/
Qt::ItemFlags QContactModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

/*!
  Returns the contact for the row specified by \a index.
  The column of \a index is ignored.
*/
QContact QContactModel::contact(const QModelIndex &index) const
{
    return contact(index.row());
}

/*!
  Return the contact for the given \a row.
*/
QContact QContactModel::contact(int row) const
{
    return d->defaultio->contact(row);
}

/*!
  Returns the contact in the model with the given \a identifier.  The contact does
  not have to be in the current filter mode for it to be returned.
*/
QContact QContactModel::contact(const QUniqueId & identifier) const
{
    return d->defaultio->contact(identifier);
}

/*!
  Returns the value for the specified \a field of the given \a contact.
*/
QVariant QContactModel::contactField(const QContact &contact, QContactModel::Field field)
{
    switch(field) {
        default:
        case QContactModel::Invalid:
            break;
        case QContactModel::Identifier:
            return QVariant(contact.uid().toByteArray());
        case QContactModel::Categories:
            return QVariant(contact.categories());
        case QContactModel::NameTitle:
            return contact.nameTitle();
        case QContactModel::FirstName:
            return contact.firstName();
        case QContactModel::MiddleName:
            return contact.middleName();
        case QContactModel::LastName:
            return  contact.lastName();
        case QContactModel::Suffix:
            return contact.suffix();
        case QContactModel::Label:
            return contact.label();
        case QContactModel::JobTitle:
            return contact.jobTitle();
        case QContactModel::Department:
            return contact.department();
        case QContactModel::Company:
            return contact.company();
        case QContactModel::BusinessPhone:
            return contact.businessPhone();
        case QContactModel::BusinessFax:
            return contact.businessFax();
        case QContactModel::BusinessMobile:
            return contact.businessMobile();
        case QContactModel::BusinessVOIP:
            return contact.businessVOIP();
        case QContactModel::DefaultEmail:
            return contact.defaultEmail();
        case QContactModel::DefaultPhone:
            return contact.defaultPhoneNumber();
        case QContactModel::Emails:
            return contact.emailList();

        case QContactModel::OtherPhone:
            return contact.phoneNumber(QContact::OtherPhone);
        case QContactModel::OtherFax:
            return contact.phoneNumber(QContact::Fax);
        case QContactModel::OtherMobile:
            return contact.phoneNumber(QContact::Mobile);
        case QContactModel::OtherPager:
            return contact.phoneNumber(QContact::Pager);

        case QContactModel::HomePhone:
            return contact.phoneNumber(QContact::HomePhone);
        case QContactModel::HomeFax:
            return contact.phoneNumber(QContact::HomeFax);
        case QContactModel::HomeMobile:
            return contact.phoneNumber(QContact::HomeMobile);
        case QContactModel::HomeVOIP:
            return contact.phoneNumber(QContact::HomeVOIP);
        case QContactModel::HomePager:
            return contact.phoneNumber(QContact::HomePager);

        case QContactModel::BusinessStreet:
            return contact.businessStreet();
        case QContactModel::BusinessCity:
            return contact.businessCity();
        case QContactModel::BusinessState:
            return contact.businessState();
        case QContactModel::BusinessZip:
            return contact.businessZip();
        case QContactModel::BusinessCountry:
            return contact.businessCountry();
        case QContactModel::BusinessPager:
            return contact.businessPager();
        case QContactModel::BusinessWebPage:
            return contact.businessWebpage();
        case QContactModel::Office:
            return contact.office();
        case QContactModel::Profession:
            return contact.profession();
        case QContactModel::Assistant:
            return contact.assistant();
        case QContactModel::Manager:
            return contact.manager();
        case QContactModel::HomeStreet:
            return contact.homeStreet();
        case QContactModel::HomeCity:
            return contact.homeCity();
        case QContactModel::HomeState:
            return contact.homeState();
        case QContactModel::HomeZip:
            return contact.homeZip();
        case QContactModel::HomeCountry:
            return contact.homeCountry();
        case QContactModel::HomeWebPage:
            return contact.homeWebpage();
        case QContactModel::Spouse:
            return contact.spouse();
        case QContactModel::Gender:
            return contact.gender();
        case QContactModel::Birthday:
            return contact.birthday();
        case QContactModel::Anniversary:
            return contact.anniversary();
        case QContactModel::Nickname:
            return contact.nickname();
        case QContactModel::Children:
            return contact.children();
        case QContactModel::Portrait:
            return contact.portraitFile();
        case QContactModel::Notes:
            return contact.notes();
        case QContactModel::LastNamePronunciation:
            return contact.lastNamePronunciation();
        case QContactModel::FirstNamePronunciation:
            return contact.firstNamePronunciation();
        case QContactModel::CompanyPronunciation:
            return contact.companyPronunciation();

        //  Meta stuff (not present in contact itself)
        case QContactModel::PresenceStatus:
            return qVariantFromValue(ContactSqlIO::presenceStatus(contact.uid()));
        case QContactModel::PresenceMessage:
            return qVariantFromValue(ContactSqlIO::presenceMessage(contact.uid()));
        case QContactModel::PresenceStatusString:
            return qVariantFromValue(ContactSqlIO::presenceStatusString(contact.uid()));
        case QContactModel::PresenceDisplayName:
            return qVariantFromValue(ContactSqlIO::presenceDisplayName(contact.uid()));
        case QContactModel::PresenceAvatar:
            return qVariantFromValue(ContactSqlIO::presenceAvatar(contact.uid()));
        case QContactModel::PresenceCapabilities:
            return qVariantFromValue(ContactSqlIO::presenceCapabilities(contact.uid()));
        case QContactModel::PresenceUpdateTime:
            return qVariantFromValue(ContactSqlIO::presenceUpdateTime(contact.uid()));
    }
    return QVariant();
}

/*!
  Sets the value for the specified \a field of the given \a contact to \a value.

  Returns true if the contact was modified.

  \sa setData()
*/
bool QContactModel::setContactField(QContact &contact, QContactModel::Field field,  const QVariant &value)
{
    switch(field) {
        default:
        case QContactModel::Invalid:
        case QContactModel::Identifier: // not a settable field
        case QContactModel::Label:
            return false;
        case QContactModel::Categories:
            if (value.canConvert(QVariant::StringList)) {
                contact.setCategories(value.toStringList());
                return true;
            }
            return false;
        case QContactModel::NameTitle:
            if (value.canConvert(QVariant::String)) {
                contact.setNameTitle(value.toString());
                return true;
            }
            return false;
        case QContactModel::FirstName:
            if (value.canConvert(QVariant::String)) {
                contact.setFirstName(value.toString());
                return true;
            }
            return false;
        case QContactModel::MiddleName:
            if (value.canConvert(QVariant::String)) {
                contact.setMiddleName(value.toString());
                return true;
            }
            return false;
        case QContactModel::LastName:
            if (value.canConvert(QVariant::String)) {
                contact.setLastName(value.toString());
                return true;
            }
            return false;
        case QContactModel::Suffix:
            if (value.canConvert(QVariant::String)) {
                contact.setSuffix(value.toString());
                return true;
            }
            return false;
        case QContactModel::JobTitle:
            if (value.canConvert(QVariant::String)) {
                contact.setJobTitle(value.toString());
                return true;
            }
            return false;
        case QContactModel::Department:
            if (value.canConvert(QVariant::String)) {
                contact.setDepartment(value.toString());
                return true;
            }
            return false;
        case QContactModel::Company:
            if (value.canConvert(QVariant::String)) {
                contact.setCompany(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessPhone:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessPhone(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessFax:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessFax(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessMobile:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessMobile(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessVOIP:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::BusinessVOIP, value.toString());
                return true;
            }
            return false;
        case QContactModel::DefaultPhone:
            if (value.canConvert(QVariant::String)) {
                contact.setDefaultPhoneNumber(value.toString());
                return true;
            }
            return false;
        case QContactModel::DefaultEmail:
            if (value.canConvert(QVariant::String)) {
                contact.setDefaultEmail(value.toString());
                return true;
            }
            return false;
        case QContactModel::Emails:
            if (value.canConvert(QVariant::StringList)) {
                contact.setEmailList(value.toStringList());
                return true;
            }
            return false;
        case QContactModel::OtherPhone:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::OtherPhone, value.toString());
                return true;
            }
            return false;
        case QContactModel::OtherFax:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::Fax, value.toString());
                return true;
            }
            return false;
        case QContactModel::OtherMobile:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::Mobile, value.toString());
                return true;
            }
            return false;
        case QContactModel::OtherPager:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::Pager, value.toString());
                return true;
            }
            return false;
        case QContactModel::HomePhone:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomePhone, value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeVOIP:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomeVOIP, value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeFax:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomeFax, value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeMobile:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomeMobile, value.toString());
                return true;
            }
            return false;
        case QContactModel::HomePager:
            if (value.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomePager, value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessStreet:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessStreet(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessCity:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessCity(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessState:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessState(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessZip:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessZip(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessCountry:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessCountry(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessPager:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessPager(value.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessWebPage:
            if (value.canConvert(QVariant::String)) {
                contact.setBusinessWebpage(value.toString());
                return true;
            }
            return false;
        case QContactModel::Office:
            if (value.canConvert(QVariant::String)) {
                contact.setOffice(value.toString());
                return true;
            }
            return false;
        case QContactModel::Profession:
            if (value.canConvert(QVariant::String)) {
                contact.setProfession(value.toString());
                return true;
            }
            return false;
        case QContactModel::Assistant:
            if (value.canConvert(QVariant::String)) {
                contact.setAssistant(value.toString());
                return true;
            }
            return false;
        case QContactModel::Manager:
            if (value.canConvert(QVariant::String)) {
                contact.setManager(value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeStreet:
            if (value.canConvert(QVariant::String)) {
                contact.setHomeStreet(value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeCity:
            if (value.canConvert(QVariant::String)) {
                contact.setHomeCity(value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeState:
            if (value.canConvert(QVariant::String)) {
                contact.setHomeState(value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeZip:
            if (value.canConvert(QVariant::String)) {
                contact.setHomeZip(value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeCountry:
            if (value.canConvert(QVariant::String)) {
                contact.setHomeCountry(value.toString());
                return true;
            }
            return false;
        case QContactModel::HomeWebPage:
            if (value.canConvert(QVariant::String)) {
                contact.setHomeWebpage(value.toString());
                return true;
            }
            return false;
        case QContactModel::Spouse:
            if (value.canConvert(QVariant::String)) {
                contact.setSpouse(value.toString());
                return true;
            }
            return false;
        case QContactModel::Gender:
            if (value.canConvert(QVariant::Int)) {
                contact.setGender((QContact::GenderType)value.toInt());
                return true;
            }
            return false;
        case QContactModel::Birthday:
            if (value.canConvert(QVariant::Date)) {
                contact.setBirthday(value.toDate());
                return true;
            }
            return false;
        case QContactModel::Anniversary:
            if (value.canConvert(QVariant::Date)) {
                contact.setAnniversary(value.toDate());
                return true;
            }
            return false;
        case QContactModel::Nickname:
            if (value.canConvert(QVariant::String)) {
                contact.setNickname(value.toString());
                return true;
            }
            return false;
        case QContactModel::Children:
            if (value.canConvert(QVariant::String)) {
                contact.setChildren(value.toString());
                return true;
            }
            return false;
        case QContactModel::Portrait:
            if (value.canConvert(QVariant::String)) {
                contact.setPortraitFile(value.toString());
                return true;
            }
            return false;
        case QContactModel::Notes:
            if (value.canConvert(QVariant::String)) {
                contact.setNotes(value.toString());
                return true;
            }
            return false;
        case QContactModel::LastNamePronunciation:
            if (value.canConvert(QVariant::String)) {
                contact.setLastNamePronunciation(value.toString());
                return true;
            }
            return false;
        case QContactModel::FirstNamePronunciation:
            if (value.canConvert(QVariant::String)) {
                contact.setFirstNamePronunciation(value.toString());
                return true;
            }
            return false;
        case QContactModel::CompanyPronunciation:
            if (value.canConvert(QVariant::String)) {
                contact.setCompanyPronunciation(value.toString());
                return true;
            }
            return false;
    }
    return false;
}
/*!
  Updates the contact in the model with the same identifier as the specified \a contact to
  equal the specified contact.

  Returns true if the contact was successfully updated.
*/
bool QContactModel::updateContact(const QContact& contact)
{
    QContactContext *c= qobject_cast<QContactContext *>(context(contact.uid()));
    if (c) {
        bool result = c->updateContact(contact);
        if (result) {
            if (contact.uid() == personalID())
                updateBusinessCard(contact);
            refresh();
        }
        return result;
    }
    return false;
}

/*!
  Removes the contact from the model with the same identifier as the specified \a contact.

  Returns true if the contact was successfully removed.
*/
bool QContactModel::removeContact(const QContact& contact)
{
    return removeContact(contact.uid());
}

/*!
  Removes the contact from the model with the specified \a identifier.

  Returns true if the contact was successfully removed.
*/
bool QContactModel::removeContact(const QUniqueId& identifier)
{
    QContactContext *c = qobject_cast<QContactContext *>(context(identifier));
    if (c) {
        /* delete the portrait, if any XXX - we shouldn't have to fetch the whole contact */
        QString portraitFile = contact(identifier).portraitFile();
        bool result = c->removeContact(identifier);
        if (result) {
            if (!portraitFile.isEmpty() && !portraitFile.startsWith(QChar(':'))) {
                QString baseDir = Qtopia::applicationFileName( "addressbook", "contactimages/" );
                QFile pFile( baseDir + portraitFile );
                if( pFile.exists() )
                    pFile.remove();
            }
            if (identifier == personalID())
                clearPersonalDetails();
            refresh();
        }
        return result;
    }
    return false;
}

/*!
  Adds the \a contact to the model under the specified storage \a source.
  If source is null the function will add the contact to the default storage source.

  Returns a valid identifier for the contact if the contact was
  successfully added.  Otherwise returns a null identifier.

  Note the current identifier of the specified appointment is ignored.

  \sa phoneSource(), simSource(), mirrorToSource()
*/
QUniqueId QContactModel::addContact(const QContact& contact, const QPimSource &source)
{
    QContactContext *c = qobject_cast<QContactContext *>(context(source));

    QUniqueId id;
    if (c && !(id = c->addContact(contact, source)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Removes the records in the model specified by the list of \a identifiers.

  Returns true if contacts were successfully removed.
*/
bool QContactModel::removeList(const QList<QUniqueId> &identifiers)
{
    QUniqueId id;
    foreach(id, identifiers) {
        if (!exists(id))
            return false;
    }
    QUniqueId pid = personalID();
    foreach(id, identifiers) {
        removeContact(id);
        if (id == pid)
            removeBusinessCard();
    }
    return true;
}

/*!
  \overload

  Adds the PIM record encoded in \a bytes to the model under the specified storage \a source.
  The format of the record in \a bytes is given by \a format.  An empty format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  If the specified source is null the function will add the record to the default storage source.

  Returns a valid identifier for the record if the record was
  successfully added.  Otherwise returns a null identifier.

  Can only add PIM data that is represented by the model.  This means that only contact data
  can be added using a contact model.  Valid formats are "vCard" or an empty string.

*/
QUniqueId QContactModel::addRecord(const QByteArray &bytes, const QPimSource &source, const QString &format)
{
    if (format == "vCard") {
        QList<QContact> list = QContact::readVCard(bytes);
        if (list.count() == 1)
            return addContact(list[0], source);
    } else {
        QContact c;
        QDataStream ds(bytes);
        ds >> c;
        return addContact(c, source);
    }
    return QUniqueId();
}

/*!
  \overload
  Updates the corresponding record in the model to equal the record encoded in \a bytes.
  The format of the record in \a bytes is given by the \a format string.
  An empty \a format string will cause the record to be read using the data stream operators
  for the PIM data type of the model. If \a id is not null will set the record identifier to \a id
  before attempting to update the record.

  Returns true if the record was successfully updated.

  Valid formats are "vCalendar" or an empty string.

  \sa updateContact()
*/
bool QContactModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format)
{
    QContact c;
    if (format == "vCard") {
        QList<QContact> list = QContact::readVCard(bytes);
        if (list.count() == 1) {
            c = list[0];
        }
    } else {
        QDataStream ds(bytes);
        ds >> c;
    }
    if (!id.isNull())
        c.setUid(id);
    return updateContact(c);
}

/*!
  \fn bool QContactModel::removeRecord(const QUniqueId &identifier)
  \overload

  Removes the record from the model with the specified \a identifier.

  Returns true if the record was successfully removed.
*/

/*!
  \overload

  Returns the record in the model with the specified \a identifier encoded in the format specified by the \a format string.
  An empty format string will cause the record to be written using the data stream
  operators for the PIM data type of the model.

  Valid formats are "vCard" or an empty string.

  \sa contact()
*/
QByteArray QContactModel::record(const QUniqueId &identifier, const QString &format) const
{
    QContact c = contact(identifier);
    if (c.uid().isNull())
        return QByteArray();

    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    if (format == "vCard") {
        c.writeVCard(&ds);
        return bytes;
    } else {
        ds << c;
        return bytes;
    }
    return QByteArray();
}

void QContactModel::pimMessage(const QString& message, const QByteArray& data)
{
    if (message == QLatin1String("updatePersonalId(QUniqueId,QUniqueId)")) {
        QDataStream ds(data);

        QUniqueId newId;
        QUniqueId oldId;

        ds >> oldId;
        ds >> newId;

        d->mPersonalId = newId;
        d->mPersonalIdRead = true;

        // Notify views that we changed
        QModelIndex pd = index(newId);
        if (pd.isValid())
            emit dataChanged(pd, pd);

        if (oldId != newId) {
            pd = index(oldId);
            if (pd.isValid())
                emit dataChanged(pd, pd);
        }
    }
}

/*!
  Returns the identifier for the contact representing the personal details of the device owner.

  If no contact is specified as the personal details of the device owner, will return a
  null identifier.
*/
QUniqueId QContactModel::personalID() const
{
    // Cache this, since it is used by QContactDelegate
    // We broadcast when we change it.
    if (!d->mPersonalIdRead) {
        d->mPersonalIdRead = true;
        QSettings c("Trolltech","Pim");
        c.beginGroup("Contacts");
        if (c.contains("personalid"))
            d->mPersonalId = QUniqueId(c.value("personalid").toString());
        else
            d->mPersonalId = QUniqueId();
    }

    return d->mPersonalId;
}

/*!
  Returns the contact representing the personal details of the device owner.

  If no contact is specified as the personal details of the device owner, will return a null
  contact.
*/
QContact QContactModel::personalDetails() const
{
    return contact(personalID());
}

/*!
  Returns true if a contact in the contact model is specified as the personal details of the
  device owner.
*/
bool QContactModel::hasPersonalDetails() const
{
    QUniqueId id = personalID();
    return exists(id);
}

/*!
  Returns true if the contact for the specified \a row represents the personal details
  of the device owner.
*/
bool QContactModel::isPersonalDetails(const QModelIndex &row) const
{
    QUniqueId personalId = personalID();
    if (personalId == id(row) && exists(personalId))
        return true;
    return false;
}

/*!
  Clears the personal details of the device owner.  Does not remove
  and contacts from the contact model.

  /sa setPersonalDetails()
*/
void QContactModel::clearPersonalDetails()
{
    setPersonalDetails(QUniqueId());
}

/*!
  Sets the personal details of the device owner to the contact with the given \a identifier.
  If there is no contact with the specified identifier in the contact model this will clear the personal details.

  \sa clearPersonalDetails()
*/
void QContactModel::setPersonalDetails(const QUniqueId & identifier)
{
    // Force a read
    QUniqueId oldId = personalID();

    {
        QSettings c("Trolltech","Pim");
        c.beginGroup("Contacts");
        c.setValue("personalid", identifier.toString());
    }

    d->mPersonalId = identifier;
    d->mPersonalIdRead = true;

    // The IPC will send the appropriate signals, if required.
    {
        QtopiaIpcEnvelope e("QPE/PIM", "updatePersonalId(QUniqueId,QUniqueId)");
        e << oldId << identifier;
    }

    updateBusinessCard(contact(identifier));
}

/*!
  Returns true if the contact with specified \a identifier represents the personal details
  of the device owner.
*/
bool QContactModel::isPersonalDetails(const QUniqueId & identifier) const
{
    QUniqueId personalId = personalID();
    if (personalId == identifier && exists(personalId))
        return true;
    return false;
}

static QString businessCardName() {
    return Qtopia::applicationFileName("addressbook",
            "businesscard.vcf");
}

/*!
  \internal
  */
void QContactModel::updateBusinessCard(const QContact &cnt)
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Sync");
    cfg.setValue("ownername", cnt.label());

    if (cnt.uid().isNull())
        QFile::remove( businessCardName() );
    else
        cnt.writeVCard( businessCardName());
}

/*!
  \internal
  */
void QContactModel::removeBusinessCard()
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Sync");
    cfg.setValue("ownername", "");

    QFile::remove( businessCardName() );
}

/*!
  Returns the best match for the phone number \a text.  If no contact
  in the model has a phone number matching the given text returns a
  null contact.
*/
QContact QContactModel::matchPhoneNumber(const QString &text)
{
    int match;
    return contact(d->defaultio->matchPhoneNumber(text, match));
}

/*!
  Returns the best match for the email address \a text.  If no contact
  in the model has an email address matching the given text returns a
  null contact.
*/
QContact QContactModel::matchEmailAddress(const QString &text)
{
    int match;
    return contact(d->defaultio->matchEmailAddress(text, match));
}

/*!
  Returns the best match for the chat address \a text.  If the
  \a provider is not empty, restricts matches to contacts with
  chat addresses from that QCollectivePresence provider. If no contact
  in the model has a chat address matching the given text returns a
  null contact.

  \sa QCollectivePresence
*/
QContact QContactModel::matchChatAddress(const QString &text, const QString& provider)
{
    return contact(d->defaultio->matchChatAddress(text, provider).value(0));
}

/*!
  \enum QContactModel::FilterFlags

  These flags describe what kind of contact information to filter contacts on.

    \value ContainsPhoneNumber
        The contact must provide one or more phone numbers.
    \value ContainsEmail
        The contact must provide one or more email addresses.
    \value ContainsMailing
        The contact must provide one or more mailing addresses.
    \value ContainsChat
        The contact must provide one or more chat addresses.

  \sa setFilter()
*/

/*!
  Sets the model to filter contacts by labels that start with \a text
  and contact type information specified \a flags.  By default flags
  is 0 which means filtering will only occur based on label text.

  \sa FilterFlags, filterFlags(), filterText()
*/
void QContactModel::setFilter(const QString &text, int flags)
{
    if (text == filterText() && flags == filterFlags())
        return;

    d->filterText = text;
    d->filterFlags = flags;
    d->defaultio->setFilter(text, flags);
}

/*!
  Returns the filter text being used by the model.

  \sa setFilter()
*/
QString QContactModel::filterText() const
{
    return d->filterText;
}

/*!
  Returns the filter flags being used by the model.

  \sa setFilter()
*/
int QContactModel::filterFlags() const
{
    return d->filterFlags;
}

/*!
  Clears contact name and type filtering for the model.
  Does not affect category or presence filtering.
*/
void QContactModel::clearFilter()
{
    if (d->filterText.isEmpty() && d->filterFlags == 0)
        return;

    d->defaultio->clearFilter();

    d->filterText.clear();
    d->filterFlags = 0;
}


/*!
    Sets the model to only contain those contacts that have
    a presence status in the supplied \a types.
*/
void QContactModel::setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType> types)
{
    d->presenceFilter = types;
    d->defaultio->setPresenceFilter(types);
}

/*!
    Returns the presence filter being used by the model.

    \sa setPresenceFilter()
*/
QList<QCollectivePresenceInfo::PresenceType> QContactModel::presenceFilter() const
{
    return d->presenceFilter;
}

/*!
   Clears any presence filtering done by the model.

   \sa setPresenceFilter()
*/
void QContactModel::clearPresenceFilter()
{
    d->presenceFilter.clear();
    d->defaultio->clearPresenceFilter();
}

/*!
  Returns true if the contact at the given \a index is stored on the SIM card.
*/
bool QContactModel::isSimCardContact(const QModelIndex &index) const
{
    return isSimCardContact(id(index));
}

/*!
  Returns true if the contact with the given \a identifier is on the SIM card.
*/
bool QContactModel::isSimCardContact(const QUniqueId & identifier) const
{
#ifdef QTOPIA_CELL
    simSource();
    // mContext part of id
    return identifier.mappedContext() == d->simContextId;
#else
    Q_UNUSED(identifier);
    return false;
#endif
}

/*!
  Returns the identifier for storage sources relating to the SIM Card.
*/
QPimSource QContactModel::simSource() const
{
    return d->simSource;
}

/*!
  Returns the default identifier for storage sources relating to the device memory.
 */
QPimSource QContactModel::phoneSource() const
{
    return defaultSource();
}

/*!
  Exports the contact in the model with the specified \a identifier and imports
  it into the \a destination storage source.  The contact is modified by the
  source and destination storage contexts to account for storage restrictions.
  For instance a contact may be split into multiple contacts when mirrored to
  the active SIM card and contacts from the SIM card will be merged into contacts
  in local storage if they have equal name information.

  Returns true upon success.

  \sa mirrorAll()
*/
bool QContactModel::mirrorToSource(const QPimSource &destination, const QUniqueId &identifier)
{
    QContact c;
    QContactContext *sourceContext = qobject_cast<QContactContext *>(context(identifier));
    QPimSource source = QContactModel::source(identifier);
    QContactContext *destContext = qobject_cast<QContactContext *>(context(destination));

    if (source == destination)
        return false;

    if (sourceContext && destContext) {
        bool result;
        QContact c = sourceContext->exportContact(identifier, result);
        if (result)
            return destContext->importContact(destination, c);
    }

    return false;
}

/*!
  Exports the contacts stored in \a source storage source and imports them into the \a destination storage source.
  The contacts are modified by the source and destination storage contexts to account for storage restrictions.

  Returns true upon success.

  \sa mirrorToSource()
*/
bool QContactModel::mirrorAll(const QPimSource &source, const QPimSource &destination)
{
    if (source == destination || source.isNull() || destination.isNull())
        return false;
    QContactContext *sourceContext = qobject_cast<QContactContext *>(context(source));
    QContactContext *destContext = qobject_cast<QContactContext *>(context(destination));

    if (sourceContext && destContext) {
        bool result;
        QList<QContact> c = sourceContext->exportContacts(source, result);
        if (result)
            return destContext->importContacts(destination, c);
    }
    return false;
}

/*!
   Write vCards for all visible contacts in the model to the file
   specified by \a filename.

   Returns true if successful.
*/
bool QContactModel::writeVCard( const QString &filename )
{
    if( count() ) {

        QFile f( filename );
        if( ! f.open( QIODevice::WriteOnly ) ) {
            qWarning() << "Unable to open vcard file for write!";
            return false;
        }

        for( int i = 0; i < count(); i++ )
            contact(i).writeVCard( f );

        return true;
    }

    return false;
}

/* recommend using filter instead, better performance */
/*!
  \overload
    Returns a list of indexes for the items where the data
    matches the specified \a value.  The list that is returned may be empty.

    The search starts from the \a start index and continues until the number of
    matching data items equals \a hits or the search reaches the last row

    The arguments \a role and \a flags are currently ignored.
*/
QModelIndexList QContactModel::match(const QModelIndex &start, int role, const QVariant &value,
            int hits, Qt::MatchFlags flags) const
{
    /* role and flags ignored. */
    Q_UNUSED(role);
    Q_UNUSED(flags);

    QModelIndexList l;
    if ( 0 == hits )
        return l;

    if (d->searchModel == 0) {
        d->searchModel = new QContactModel(0);
    }
    d->searchModel->setCategoryFilter(categoryFilter());
    d->searchModel->setFilter(value.toString());

    for (int i = 0; i < d->searchModel->count(); i++) {
        QModelIndex idx = d->searchModel->index(i, 0, QModelIndex());
        if (idx.isValid()) {
            QModelIndex foundidx = index(d->searchModel->id(idx));
            if (foundidx.row() >= start.row()) {
                l.append(foundidx);
                if ( hits != -1 && l.count() >= hits )
                    break;
            }
        }
    }
    return l;
}

/*!
    Returns a list of indexes for the items where the data in the \a field value
    matches the specified \a value.  The list that is returned may be empty.

    The search starts from the \a start row and continues until the number of
    matching data items equals \a hits or the search reaches the last row.

    The \a flags argument can contain any of the following flags:

    - Qt::MatchWildcard
    - Qt::MatchFixedString (or equivalently Qt::MatchExactly)
    - Qt::MatchContains
    - Qt::MatchStartsWith
    - Qt::MatchCaseSensitive

*/
QModelIndexList QContactModel::match(Field field, const QVariant& value, Qt::MatchFlags flags, int start, int hits)
{
    QModelIndexList l;
    if ( 0 == hits )
        return l;

    QList<QUniqueId> matchedIds = d->defaultio->match(field, value, flags);

    foreach(QUniqueId id, matchedIds) {
        QModelIndex idx = index(id);
        if (idx.row() >= start) {
            l.append(idx);
            if (hits != -1 && l.count() >= hits)
                break;
        }
    }

    return l;
}

/*!
    Returns the identifier for the currently inserted SIM card.
*/
QString QContactModel::simCardIdentity() const
{
#ifdef QTOPIA_CELL
    return d->simContext->card();
#else
    return QString();
#endif
}

/*!
    Returns the index of the first entry for a SIM cnotact data \a source.
*/
int QContactModel::firstSimIndex(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->firstIndex();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the index of the last entry for a SIM contact data \a source.
*/
int QContactModel::lastSimIndex(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->lastIndex();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the maximum number of characters for the label component
    of a SIM contact data \a source entry.

    Note that when storing contacts with multiple phone numbers to a SIM,
    part of the label may be used to indicate the type phone number
    for the entries used.
*/
int QContactModel::simLabelLimit(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->labelLimit();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the maximum number of characters for the number component
    of a SIM contact data \a source entry.
*/
int QContactModel::simNumberLimit(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->numberLimit();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the total number of indexes available for a given
    SIM contact data \a source.
*/
int QContactModel::simIndexCount(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->indexCount();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the number of free indexes for a given SIM contact data \a source.
*/
int QContactModel::simFreeIndexCount(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->freeIndexCount();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the number of indexes used for a given SIM contact data \a source.
*/
int QContactModel::simUsedIndexCount(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->usedIndexCount();
#else
    Q_UNUSED(source)
    return 0;
#endif
}

/*!
    Returns the list of indexes in the given \a source of contact data
    on a SIM card for the contact with the given \a identifier.
*/
QList<int> QContactModel::simCardIndexes(const QUniqueId &identifier, const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->simCardIndexes("SM", identifier);
#else
    Q_UNUSED(identifier)
    Q_UNUSED(source)
    return QList<int>();
#endif
}

/*!
    Returns true if the entries for the given SIM contact \a source 
    are still being loaded into SQL storage.

    \sa simLoaded()
*/
bool QContactModel::loadingSim(const QPimSource &source) const
{
#ifdef QTOPIA_CELL
    Q_UNUSED(source)
    return d->simContext->loadingSim();
#else
    Q_UNUSED(source)
    return false;
#endif
}

/*!
    \fn void QContactModel::simLoaded(const QPimSource &source)

    This signal is emitted when the given \a source of contact
    data on a SIM card completes syncing to the SQL storage.

    \sa loadingSim()
*/
