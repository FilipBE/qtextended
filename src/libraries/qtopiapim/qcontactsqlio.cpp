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

#include <qcontact.h>
#include <qcontactmodel.h>

#include <qtopialog.h>
#include <qphonenumber.h>

#include <qtopiaipcenvelope.h>
#include "qcontactsqlio_p.h"
#include "qannotator_p.h"
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include <qcategorymanager.h>
#include "qsqlpimtablemodel_p.h"

#include <QSettings>
#include <QString>
#include <QSqlError>
#include <QTimer>
#include <QDebug>
#include <QSqlField>
#include <QSqlDriver>
#include <QTcpSocket>
#include <QTranslatableSettings>

#include "qcollectivenamespace.h"
#include "qcollectivepresenceinfo.h"

#include "qfielddefinition.h"

// For contact birthday/anniversary feature:
#include "qappointment.h"
#include "qappointmentsqlio_p.h"
#include "qdependentcontexts_p.h"
#include "qpimdependencylist_p.h"

static const char *contextsource = "default";

/*
   data fields

title firstname middlename lastname suffix default_phone default_email jobtitle department company b_webpage office profession assistant manager h_webpage spouse gender birthday anniversary nickname children portrait lastname_pronunciation firstname_pronunciation company_pronunciation

data bindings
:t, :fn, :mn, :ln, :su, :dp, :de, :jt, :d, :c, :bphone, :bfax, :bmobile, :bstreet, :bcity, :bstate, :bzip, :bcountry, :bpager, :bwebpage, :o, :profession, :assistant, :manager, :hphone, :hfax, :hmobile, :hstreet, :hcity, :hstate, :hzip, :hcountry, :hwebpage, :sp, :g, :b, :a, :nickname, :children, :portrait, :lnp, :fnp, :cp
*/


class ContactSimpleQueryCache : public QPimQueryCache
{
public:
    struct ContactRow
    {
        ContactRow(const QVariant &a, const QVariant &b, const QVariant &c,
                const QVariant &d, const QVariant &e, const QVariant &f,
                const QVariant &g, const QVariant &h, const QVariant &i,
                const QVariant &j, const QVariant &k)
            : recid(a), nameTitle(b), firstName(c), middleName(d),
                    lastName(e), suffix(f), phoneNumber(g), email(h),
                    company(i), portraitFile(j), label(k)
        {}

        QVariant recid;
        QVariant nameTitle;
        QVariant firstName;
        QVariant middleName;
        QVariant lastName;
        QVariant suffix;
        QVariant phoneNumber;
        QVariant email;
        QVariant company;
        QVariant portraitFile;
        QVariant label;
    };

    ContactSimpleQueryCache()
        : QPimQueryCache(), cache(25)
    {
    }

    QString fields() const
    {
        static const QString result("t1.title, t1.firstname, t1.middlename, t1.lastname, t1.suffix, t1.default_phone, t1.default_email, t1.company, t1.portrait, t1.label");
        return result;

        //(select categoryid from contactcategories where contactcategories.recid = contacts.recid and categoryid = 'Business')
    }

    void cacheRow(int row, const QPreparedSqlQuery &q)
    {
        cache.insert(row, new ContactRow(q.value(0), q.value(2), q.value(3), q.value(4), q.value(5), q.value(6), q.value(7), q.value(8), q.value(9), q.value(10), q.value(11)));
    }

    void setMaxCost(int m) { cache.setMaxCost(m); contactCache.setMaxCost(m); }
    void clear() { cache.clear(); contactCache.clear(); }

    QCache<int, ContactRow> cache;
    QCache<int, QContact> contactCache;

};

QString ContactSqlIO::sqlColumn(QContactModel::Field k)
{
    switch(k) {
        default:
            return QString();
        case QContactModel::Label:
            return "label";
        case QContactModel::NameTitle:
            return "title";
        case QContactModel::FirstName:
            return "firstname";
        case QContactModel::MiddleName:
            return "middlename";
        case QContactModel::LastName:
            return "lastname";
        case QContactModel::Suffix:
            return "suffix";

        case QContactModel::JobTitle:
            return "jobtitle";
        case QContactModel::Department:
            return "department";
        case QContactModel::Company:
            return "company";

        case QContactModel::DefaultPhone:
            return "default_phone";

        // email
        case QContactModel::DefaultEmail:
        case QContactModel::Emails: // This is in a different table
            return "default_email";

        // business
        case QContactModel::BusinessWebPage:
            return "b_webpage";

        case QContactModel::Office:
            return "office";
        case QContactModel::Profession:
            return "profession";
        case QContactModel::Assistant:
            return "assistant";
        case QContactModel::Manager:
            return "manager";

        // home
        case QContactModel::HomeWebPage:
            return "h_webpage";

        //personal
        case QContactModel::Spouse:
            return "spouse";
        case QContactModel::Gender:
            return "gender";
        case QContactModel::Birthday:
            return "birthday";
        case QContactModel::Anniversary:
            return "anniversary";
        case QContactModel::Nickname:
            return "nickname";
        case QContactModel::Children:
            return "children";

        // other
        case QContactModel::LastNamePronunciation:
            return "lastname_pronunciation";
        case QContactModel::FirstNamePronunciation:
            return "firstname_pronunciation";
        case QContactModel::CompanyPronunciation:
            return "company_pronunciation";

    }
    return QString();
}

QSet<ContactSqlIO*> ContactSqlIO::allIos;

ContactSqlIO::ContactSqlIO(QObject *parent, const QString &)
    :
    QPimSqlIO( parent, contextId(), "contacts", "contactcategories", "contactcustom",

            "title = :t, firstname = :fn, middlename = :mn, lastname = :ln, suffix = :su, "
            "default_phone = :dp, default_email = :de, jobtitle = :jt, department = :d, company = :c, "
            "b_webpage = :bwebpage, office = :o, profession = :profession, "
            "assistant = :assistant, manager = :manager, "
            "h_webpage = :hwebpage, spouse = :sp, "
            "gender = :g, birthday = :b, anniversary = :a, nickname = :nickname, "
            "children = :children, portrait = :portrait, "
            "lastname_pronunciation = :lnp, firstname_pronunciation = :fnp, "
            "company_pronunciation = :cp, "
            "label = :lab",

            "(recid, context, title, firstname, middlename, lastname, suffix, default_phone, default_email, "
            "jobtitle, department, company, "
            "b_webpage, office, profession, assistant, manager, "
            "h_webpage, spouse, gender, birthday, anniversary, nickname, children, portrait, "
            "lastname_pronunciation, firstname_pronunciation, "
            "company_pronunciation, label)"
            " VALUES "
            "(:i, :context, :t, :fn, :mn, :ln, :su, :dp, :de, :jt, :d, :c, "
            ":bwebpage, :o, "
            ":profession, :assistant, :manager, "
            ":hwebpage, :sp, :g, :b, :a, :nickname, :children, "
            ":portrait, :lnp, :fnp, :cp, :lab)",
            "PIM/Contacts"),

            contactByRowValid(false),
            tmptable(false),
            contactQuery("SELECT recid, title, firstname, " // 3
                    "middlename, lastname, suffix, " // 7
                    "default_phone, default_email, jobtitle, department, company, " // 11
                    "b_webpage, office, profession, assistant, manager, " // 16
                    "h_webpage, spouse, gender, birthday, anniversary, " // 22
                    "nickname, children, portrait, lastname_pronunciation, " // 26
                    "firstname_pronunciation, company_pronunciation " // 29
                    "FROM contacts WHERE recid = :i"),
            emailsQuery("SELECT addr from emailaddresses where recid=:id"),
            addressesQuery("SELECT addresstype, street, city, state, zip, country from contactaddresses where recid=:id"),
            phoneQuery("SELECT phone_type, phone_number from contactphonenumbers where recid=:id"),
            insertEmailsQuery("INSERT INTO emailaddresses (recid, addr) VALUES (:i, :a)"),
            insertAddressesQuery("INSERT INTO contactaddresses (recid, addresstype, street, city, state, zip, country) VALUES (:i, :t, :s, :c, :st, :z, :co)"),
            insertPhoneQuery("INSERT INTO contactphonenumbers (recid, phone_type, phone_number) VALUES (:i, :t, :ph)"),
            insertPresenceQuery("INSERT INTO contactpresence (recid, uri, status, statusstring, message, displayname, updatetime,capabilities) "
                    "SELECT :i,uri,status,statusstring,message,displayname,updatetime,capabilities FROM contactpresence WHERE uri=:u AND recid=0"),
            removeEmailsQuery("DELETE from emailaddresses WHERE recid = :i"),
            removeAddressesQuery("DELETE from contactaddresses WHERE recid = :i"),
            removePhoneQuery("DELETE from contactphonenumbers WHERE recid = :i"),
            removePresenceQuery("DELETE FROM contactpresence WHERE recid = :i")
{
    simpleCache = new ContactSimpleQueryCache;
    setSimpleQueryCache(simpleCache);

    QList<QContactModel::SortField> l;
    l << qMakePair(QContactModel::Label, Qt::AscendingOrder);
    setOrderBy(l);

    initMaps();

    allIos.insert(this);

    connect(this, SIGNAL(labelFormatChanged()), this, SLOT(updateSqlLabel()));
}

ContactSqlIO::~ContactSqlIO()
{
    allIos.remove(this);
}

QUuid ContactSqlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("a7a2832c-cdb3-40b6-9d95-6cd31e05647d");
    return u;
}
void ContactSqlIO::bindFields(const QPimRecord& r, QPreparedSqlQuery &q) const
{
    const QContact &t = (const QContact &)r;
    q.bindValue(":t", t.nameTitle());
    q.bindValue(":fn", t.firstName());
    q.bindValue(":mn", t.middleName());
    q.bindValue(":ln", t.lastName());
    q.bindValue(":su", t.suffix());
    q.bindValue(":dp", t.defaultPhoneNumber());
    q.bindValue(":de", t.defaultEmail());
    q.bindValue(":jt", t.jobTitle());
    q.bindValue(":d", t.department());
    q.bindValue(":c", t.company());

    q.bindValue(":bwebpage", t.businessWebpage());
    q.bindValue(":o", t.office());
    q.bindValue(":profession", t.profession());
    q.bindValue(":assistant", t.assistant());
    q.bindValue(":manager", t.manager());

    q.bindValue(":hwebpage", t.homeWebpage());
    q.bindValue(":sp", t.spouse());
    q.bindValue(":g", t.gender());
    q.bindValue(":b", t.birthday());
    q.bindValue(":a", t.anniversary());
    q.bindValue(":nickname", t.nickname());
    q.bindValue(":children", t.children());
    q.bindValue(":portrait", t.portraitFile());
    q.bindValue(":lnp", t.lastNamePronunciation());
    q.bindValue(":fnp", t.firstNamePronunciation());
    q.bindValue(":cp", t.companyPronunciation());
    q.bindValue(":lab", t.label()); // We store this, but we don't set it back to the contact when we're done
}

// by uid doesn't neeed caching... always fast and unlikely to be in order?
QContact ContactSqlIO::contact( const QUniqueId & u ) const
{
    if (u.isNull())
        return QContact();
    // Getting a whole contact can be expensive.  Cache if able.
    // We're very likely to require the previous contact again.
    if (contactByRowValid && u == lastContact.uid())
        return lastContact;
    // Maybe it is in our cache?

    if (!contactQuery.prepare())
        return QContact();

    contactQuery.bindValue(":i", u.toUInt());

    QContact t;

    // get common parts
    retrieveRecord(u.toUInt(), t);

    if (!contactQuery.exec()) {
        contactByRowValid = false;
        return t;
    }

    if ( contactQuery.next() ) {
        QString defaultPhone;

        t.setUid(QUniqueId::fromUInt(contactQuery.value(0).toUInt()));
        t.setNameTitle(contactQuery.value(1).toString());
        t.setFirstName(contactQuery.value(2).toString());
        t.setMiddleName(contactQuery.value(3).toString());
        t.setLastName(contactQuery.value(4).toString());
        t.setSuffix(contactQuery.value(5).toString());

        defaultPhone = contactQuery.value(6).toString();

        t.setDefaultEmail(contactQuery.value(7).toString());
        t.setJobTitle(contactQuery.value(8).toString());
        t.setDepartment(contactQuery.value(9).toString());
        t.setCompany(contactQuery.value(10).toString());
        t.setBusinessWebpage(contactQuery.value(11).toString());
        t.setOffice(contactQuery.value(12).toString());
        t.setProfession(contactQuery.value(13).toString());
        t.setAssistant(contactQuery.value(14).toString());
        t.setManager(contactQuery.value(15).toString());
        t.setHomeWebpage(contactQuery.value(16).toString());
        t.setSpouse(contactQuery.value(17).toString());
        t.setGender((QContact::GenderType)contactQuery.value(18).toInt());
        t.setBirthday(contactQuery.value(19).toDate());
        t.setAnniversary(contactQuery.value(20).toDate());
        t.setNickname(contactQuery.value(21).toString());
        t.setChildren(contactQuery.value(22).toString());
        t.setPortraitFile(contactQuery.value(23).toString());
        t.setLastNamePronunciation(contactQuery.value(24).toString());
        t.setFirstNamePronunciation(contactQuery.value(25).toString());
        t.setCompanyPronunciation(contactQuery.value(26).toString());

        contactQuery.reset();

        // categories for this contact;
        if (emailsQuery.prepare()) {
            emailsQuery.bindValue(":id", u.toUInt());
            emailsQuery.exec();
            QStringList tlist;
            while(emailsQuery.next())
                tlist.append(emailsQuery.value(0).toString());
            t.setEmailList(tlist);
            emailsQuery.reset();
        }

        // and contact addresses
        if (addressesQuery.prepare()) {
            addressesQuery.bindValue(":id", u.toUInt());
            addressesQuery.exec();
            while(addressesQuery.next()) {
                QContactAddress a;
                QContact::Location l;
                l = (QContact::Location)addressesQuery.value(0).toInt();
                a.street = addressesQuery.value(1).toString();
                a.city = addressesQuery.value(2).toString();
                a.state = addressesQuery.value(3).toString();
                a.zip = addressesQuery.value(4).toString();
                a.country = addressesQuery.value(5).toString();
                t.setAddress(l, a);
            }
            addressesQuery.reset();
        }

        // and phone numbers
        if (phoneQuery.prepare()) {
            phoneQuery.bindValue(":id", u.toUInt());
            phoneQuery.exec();
            while(phoneQuery.next()) {
                QString number;
                QContact::PhoneType type;
                type = (QContact::PhoneType)phoneQuery.value(0).toInt();
                number = phoneQuery.value(1).toString();
                t.setPhoneNumber(type, number);
            }
            phoneQuery.reset();
        }

        if (!defaultPhone.isEmpty())
            t.setDefaultPhoneNumber(defaultPhone);

        lastContact = t;
        contactByRowValid = true;
    } else {
        contactQuery.reset();
        contactByRowValid = false;
    }

    return t;
}

void ContactSqlIO::invalidateCache()
{
    QPimSqlIO::invalidateCache();
    contactByRowValid = false;
    mLocalNumberCache.clear();
}

// if filtering/sorting/contacts doesn't change.
QContact ContactSqlIO::contact(int row) const
{
    return contact(id(row));
}

void ContactSqlIO::updateSqlLabel()
{
    // If the original sort contained label, we should reset this now
    setOrderBy(mSortList);
    invalidateCache();
}

void ContactSqlIO::setOrderBy(QList<QContactModel::SortField> list)
{
    /* Create the sort keys */
    QStringList sl;
    foreach (QContactModel::SortField pair, list) {
        QString column = sqlColumn(pair.first);
        if (pair.second == Qt::DescendingOrder)
            column.append(QLatin1String(" DESC")); // no tr
        sl.append(column);
    }
    QPimSqlIO::setOrderBy(sl);

    // Store the list, in case our label changes
    mSortList = list;
}

// assumes storage is row based.
// override if more efficient to grap partial contact.
QVariant ContactSqlIO::contactField(int row, QContactModel::Field k) const
{
    if (!simpleCache->cache.contains(row))
        id(row);

    ContactSimpleQueryCache::ContactRow *cr = simpleCache->cache.object(row);
    if (cr) {
        switch(k) {
            case QContactModel::Identifier:
                return QUniqueId::fromUInt(cr->recid.toUInt()).toByteArray();
            case QContactModel::NameTitle:
                return cr->nameTitle;
            case QContactModel::FirstName:
                return cr->firstName;
            case QContactModel::MiddleName:
                return cr->middleName;
            case QContactModel::LastName:
                return cr->lastName;
            case QContactModel::Suffix:
                return cr->suffix;
            case QContactModel::Company:
                return cr->company;
            case QContactModel::DefaultEmail:
                return cr->email;
            case QContactModel::DefaultPhone:
                return cr->phoneNumber;
            case QContactModel::Portrait:
                return cr->portraitFile;
            case QContactModel::Label:
                return cr->label;
                // Meta stuff
            case QContactModel::PresenceStatus:
                return qVariantFromValue(presenceStatus(QUniqueId::fromUInt(cr->recid.toUInt())));
            case QContactModel::PresenceStatusString:
                return qVariantFromValue(presenceStatusString(QUniqueId::fromUInt(cr->recid.toUInt())));
            case QContactModel::PresenceMessage:
                return qVariantFromValue(presenceMessage(QUniqueId::fromUInt(cr->recid.toUInt())));
            case QContactModel::PresenceDisplayName:
                return qVariantFromValue(presenceDisplayName(QUniqueId::fromUInt(cr->recid.toUInt())));
            case QContactModel::PresenceAvatar:
                return qVariantFromValue(presenceAvatar(QUniqueId::fromUInt(cr->recid.toUInt())));
            case QContactModel::PresenceCapabilities:
                return qVariantFromValue(presenceCapabilities(QUniqueId::fromUInt(cr->recid.toUInt())));
            case QContactModel::PresenceUpdateTime:
                return qVariantFromValue(presenceUpdateTime(QUniqueId::fromUInt(cr->recid.toUInt())));
            default:
                break;
        }
    }

    return QContactModel::contactField(contact(row), k);
}

QPresenceTypeMap ContactSqlIO::presenceStatus(const QUniqueId& id)
{
    QPresenceTypeMap ret;

    if (!id.isNull()) {
        static QPreparedSqlQuery presenceStatusQuery("SELECT uri,status FROM contactpresence WHERE recid=:i");
        presenceStatusQuery.prepare();
        presenceStatusQuery.bindValue(":i", id.toUInt());
        presenceStatusQuery.exec();

        while (presenceStatusQuery.next()) {
            ret.insert(presenceStatusQuery.value(0).toString(), (QCollectivePresenceInfo::PresenceType) presenceStatusQuery.value(1).toUInt());
        }

        presenceStatusQuery.reset();
    }
    return ret;
}

QPresenceStringMap ContactSqlIO::presenceStatusString(const QUniqueId& id)
{
    QPresenceStringMap ret;

    if (!id.isNull()) {
        static QPreparedSqlQuery presenceQuery("SELECT uri,statusstring FROM contactpresence WHERE recid=:i");
        presenceQuery.prepare();
        presenceQuery.bindValue(":i", id.toUInt());
        presenceQuery.exec();

        while (presenceQuery.next()) {
            ret.insert(presenceQuery.value(0).toString(), presenceQuery.value(1).toString());
        }

        presenceQuery.reset();
    }
    return ret;
}


QPresenceStringMap ContactSqlIO::presenceMessage(const QUniqueId& id)
{
    QPresenceStringMap ret;

    if (!id.isNull()) {
        static QPreparedSqlQuery presenceQuery("SELECT uri,message FROM contactpresence WHERE recid=:i");
        presenceQuery.prepare();
        presenceQuery.bindValue(":i", id.toUInt());
        presenceQuery.exec();

        while (presenceQuery.next()) {
            ret.insert(presenceQuery.value(0).toString(), presenceQuery.value(1).toString());
        }

        presenceQuery.reset();
    }
    return ret;
}


QPresenceStringMap ContactSqlIO::presenceAvatar(const QUniqueId& id)
{
    QPresenceStringMap ret;

    if (!id.isNull()) {
        static QPreparedSqlQuery presenceQuery("SELECT uri,avatar FROM contactpresence WHERE recid=:i");
        presenceQuery.prepare();
        presenceQuery.bindValue(":i", id.toUInt());
        presenceQuery.exec();

        while (presenceQuery.next()) {
            ret.insert(presenceQuery.value(0).toString(), presenceQuery.value(1).toString());
        }

        presenceQuery.reset();
    }
    return ret;
}


QPresenceStringMap ContactSqlIO::presenceDisplayName(const QUniqueId& id)
{
    QPresenceStringMap ret;

    if (!id.isNull()) {
        static QPreparedSqlQuery presenceQuery("SELECT uri,displayname FROM contactpresence WHERE recid=:i");
        presenceQuery.prepare();
        presenceQuery.bindValue(":i", id.toUInt());
        presenceQuery.exec();

        while (presenceQuery.next()) {
            ret.insert(presenceQuery.value(0).toString(), presenceQuery.value(1).toString());
        }

        presenceQuery.reset();
    }
    return ret;
}

QPresenceCapabilityMap ContactSqlIO::presenceCapabilities(const QUniqueId& id)
{
    QPresenceCapabilityMap ret;

    if (!id.isNull()) {
        static QPreparedSqlQuery presenceQuery("SELECT uri,capabilities FROM contactpresence WHERE recid=:i");
        presenceQuery.prepare();
        presenceQuery.bindValue(":i", id.toUInt());
        presenceQuery.exec();

        while (presenceQuery.next()) {
            ret.insert(presenceQuery.value(0).toString(), presenceQuery.value(1).toString().split(','));
        }

        presenceQuery.reset();
    }
    return ret;
}

QPresenceDateTimeMap ContactSqlIO::presenceUpdateTime(const QUniqueId& id)
{
    QPresenceDateTimeMap ret;
    if (!id.isNull()) {
        static QPreparedSqlQuery presenceQuery("SELECT uri,updatetime FROM contactpresence WHERE recid=:i");
        presenceQuery.prepare();
        presenceQuery.bindValue(":i", id.toUInt());
        presenceQuery.exec();

        while (presenceQuery.next()) {
            ret.insert(presenceQuery.value(0).toString(), presenceQuery.value(1).toDateTime());
        }

        presenceQuery.reset();
    }
    return ret;

}

QContact ContactSqlIO::simpleContact(int row) const
{
    if (simpleCache->contactCache.contains(row))
        return *(simpleCache->contactCache.object(row));

    if (!simpleCache->cache.contains(row))
        id(row);
    ContactSimpleQueryCache::ContactRow *cr = simpleCache->cache.object(row);
    if (cr) {
#ifdef GREENPHONE_EFFECTS
        bool isBus = false;
#else
        QPreparedSqlQuery q(QPimSqlIO::database());
        q.prepare("SELECT categoryid from contactcategories where recid = :r and categoryid = 'Business'");
        q.bindValue(":r", cr->recid);
        q.exec();
        bool isBus = q.next();
        q.clear();
#endif

        QContact *c = new QContact;
        c->setUid(QUniqueId::fromUInt(cr->recid.toUInt()));
        c->setNameTitle(cr->nameTitle.toString());
        c->setFirstName(cr->firstName.toString());
        c->setMiddleName(cr->middleName.toString());
        c->setLastName(cr->lastName.toString());
        c->setSuffix(cr->suffix.toString());
        c->setCompany(cr->company.toString());
        c->setPortraitFile(cr->portraitFile.toString());
        if (isBus)
            c->setCategories(QLatin1String("Business"));

        simpleCache->contactCache.insert(row, c);
        return *c;
    }
    return contact(row);
}

QMap<QContactModel::Field, QString> ContactSqlIO::mFields;
QMap<QContactModel::Field, bool> ContactSqlIO::mUpdateable;

void ContactSqlIO::initMaps()
{
    struct csiIM {
        QContactModel::Field k;
        const char *field;
        bool u;
    };

    static const csiIM i[] = {
        { QContactModel::NameTitle, "title", true },
        { QContactModel::FirstName, "firstname", true },
        { QContactModel::MiddleName, "middlename", true },
        { QContactModel::LastName, "lastname", true },
        { QContactModel::Suffix, "suffix", true },
        { QContactModel::Label, "label", false }, // Not directly updateable

        { QContactModel::JobTitle, "jobtitle", true },
        { QContactModel::Department, "department", true },
        { QContactModel::Company, "company", true },

        { QContactModel::DefaultPhone, "default_phone", true },

        // email
        { QContactModel::DefaultEmail, "default_email", true },
        { QContactModel::Emails, 0, true },

        // business
        { QContactModel::BusinessWebPage, "b_webpage", true },

        { QContactModel::Office, "office", true },
        { QContactModel::Profession, "profession", true },
        { QContactModel::Assistant, "assistant", true },
        { QContactModel::Manager, "manager", true },

        // home
        { QContactModel::HomeWebPage, "h_webpage", true },

        //personal
        { QContactModel::Spouse, "spouse", true },
        { QContactModel::Gender, "gender", true },
        { QContactModel::Birthday, "birthday", true },
        { QContactModel::Anniversary, "anniversary", true },
        { QContactModel::Nickname, "nickname", true },
        { QContactModel::Children, "children", true },

        // other
        { QContactModel::Portrait, "portrait", true },

        { QContactModel::LastNamePronunciation, "lastname_pronunciation", true },
        { QContactModel::FirstNamePronunciation, "firstname_pronunciation", true },
        { QContactModel::CompanyPronunciation, "company_pronunciation", true },

        { QContactModel::Invalid, 0, false }
    };

    const csiIM *item = i;
    while (item->k != QContactModel::Invalid) {
        if (item->field)
            mFields.insert(item->k, item->field);
        mUpdateable.insert(item->k, item->u);
        ++item;
    }

}

QString ContactSqlIO::sqlField(QContactModel::Field k) const
{
    if (mFields.contains(k))
        return mFields[k];
    return QString();
}

QContact::PhoneType ContactSqlIO::fieldToPhoneType(QContactModel::Field f) const
{
    switch (f) {
        case QContactModel::BusinessPhone:
            return QContact::BusinessPhone;
        case QContactModel::BusinessMobile:
            return QContact::BusinessMobile;
        case QContactModel::BusinessVOIP:
            return QContact::BusinessVOIP;
        case QContactModel::BusinessFax:
            return QContact::BusinessFax;
        case QContactModel::BusinessPager:
            return QContact::BusinessPager;

        case QContactModel::OtherPhone:
            return QContact::OtherPhone;
        case QContactModel::OtherMobile:
            return QContact::Mobile;
        case QContactModel::OtherFax:
            return QContact::Fax;
        case QContactModel::OtherPager:
            return QContact::Pager;

        case QContactModel::HomePhone:
            return QContact::HomePhone;
        case QContactModel::HomeVOIP:
            return QContact::HomeVOIP;
        case QContactModel::HomeMobile:
            return QContact::HomeMobile;
        case QContactModel::HomeFax:
            return QContact::HomeFax;
        case QContactModel::HomePager:
            return QContact::HomePager;

        default:
            return QContact::OtherPhone;
    }
}

bool ContactSqlIO::canUpdate(QContactModel::Field k) const
{
    if (mUpdateable.contains(k))
        return mUpdateable[k];
    return false;
}


bool ContactSqlIO::removeContact(int row)
{
    QUniqueId u = id(row);
    return removeContact(u);
}

bool ContactSqlIO::removeContact(const QUniqueId & id)
{
    if (id.isNull())
        return false;

    if (QPimSqlIO::removeRecord(id)) {
        return true;
    }
    return false;
}

bool ContactSqlIO::removeContact(const QContact &t)
{
    return removeContact(t.uid());
}

bool ContactSqlIO::removeContacts(const QList<int> &rows)
{
    return removeContacts(ids(rows));
}

bool ContactSqlIO::removeContacts(const QList<QUniqueId> &ids)
{
    if (QPimSqlIO::removeRecords(ids)) {
        return true;
    }
    return false;
}

bool ContactSqlIO::updateContact(const QContact &t)
{
    // Make sure we've initialized the labels before we add
    // in case we need to update the label field via setFormat (can't nest transactions)
    if (mFormat.count() == 0)
        initFormat();

    if (QPimSqlIO::updateRecord(t)) {
        return true;
    }
    return false;
}

QUniqueId ContactSqlIO::addContact(const QContact &contact, const QPimSource &source, bool createuid)
{
    QPimSource s;
    s.identity = contextsource;
    s.context = contextId();

    // Make sure we've initialized the labels before we add
    // in case we need to update the label field via setFormat (can't nest transactions)
    if (mFormat.count() == 0)
        initFormat();
    QUniqueId i = addRecord(contact, source.isNull() ? s : source, createuid);
    if (!i.isNull()) {
        QContact added = contact;
        added.setUid(i);
    }
    return i;
}

bool ContactSqlIO::updateExtraTables(uint uid, const QPimRecord &r)
{
    if (!removeExtraTables(uid))
        return false;
    return insertExtraTables(uid, r);
}

bool ContactSqlIO::removeExtraTables(uint uid)
{
    removeEmailsQuery.prepare();
    if (!removeAddressesQuery.prepare() || !removePhoneQuery.prepare() || !removePresenceQuery.prepare())
        return false;

    removeEmailsQuery.bindValue(":i", uid);
    if (!removeEmailsQuery.exec())
        return false;
    removeEmailsQuery.reset();

    removeAddressesQuery.bindValue(":i", uid);
    if (!removeAddressesQuery.exec())
        return false;
    removeAddressesQuery.reset();

    removePhoneQuery.bindValue(":i", uid);
    if (!removePhoneQuery.exec())
        return false;
    removePhoneQuery.reset();

    removePresenceQuery.bindValue(":i", uid);
    if (!removePresenceQuery.exec())
        return false;
    removePresenceQuery.reset();

    return true;
}

bool ContactSqlIO::insertExtraTables(uint uid, const QPimRecord &r)
{
    const QContact &c = (const QContact &)r;

    insertEmailsQuery.prepare();
    insertAddressesQuery.prepare();
    insertPhoneQuery.prepare();
    insertPresenceQuery.prepare();

    QStringList e = c.emailList();
    foreach(QString i, e) {
        // don't insert empties.
        if (!i.trimmed().isEmpty()) {
            insertEmailsQuery.bindValue(":i", uid);
            insertEmailsQuery.bindValue(":a", i);
            if (!insertEmailsQuery.exec())
                return false;
        }
    }
    insertEmailsQuery.reset();

    /* home address, business address */

    QMap<QContact::Location, QContactAddress> a = c.addresses();
    QMapIterator<QContact::Location, QContactAddress> i(a);
    while(i.hasNext()) {
        i.next();
        QContactAddress a = i.value();
        insertAddressesQuery.bindValue(":i", uid);
        insertAddressesQuery.bindValue(":t", i.key());
        insertAddressesQuery.bindValue(":s", a.street);
        insertAddressesQuery.bindValue(":c", a.city);
        insertAddressesQuery.bindValue(":st", a.state);
        insertAddressesQuery.bindValue(":z", a.zip);
        insertAddressesQuery.bindValue(":co", a.country);
        if (!insertAddressesQuery.exec())
            return false;
    }
    insertAddressesQuery.reset();

    /* all phone numbers */
    QMap<QContact::PhoneType, QString> ph = c.phoneNumbers();
    QMapIterator<QContact::PhoneType, QString> phi(ph);
    while(phi.hasNext()) {
        phi.next();
        insertPhoneQuery.bindValue(":i", uid);
        insertPhoneQuery.bindValue(":t", phi.key());
        insertPhoneQuery.bindValue(":ph", phi.value());
        if (!insertPhoneQuery.exec())
            return false;
    }
    insertPhoneQuery.reset();

    /* Presence stuff */
    QStringList fields = QContactFieldDefinition::fields("chat");

    foreach(QString field, fields) {
        QContactFieldDefinition def(field);
        if (!def.value(c).toString().isEmpty() && !def.provider().isEmpty()) {
            insertPresenceQuery.bindValue(":u", QCollective::encodeUri(def.provider(), def.value(c).toString()));
            insertPresenceQuery.bindValue(":i", uid);
            if (!insertPresenceQuery.exec())
                return false;
        }
    }

    insertPresenceQuery.reset();

    return true;
}

void ContactSqlIO::setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType> types)
{
    // Passing in an empty list might mean 'contacts with no presence information'
    // but we have an enum for that.
    if (types.count() == 0) {
        // fail the query
        mPresenceFilter = "(1 = 0)";
    } else {
        QString inclause;

        bool matchNone = false;
        foreach(QCollectivePresenceInfo::PresenceType type, types) {
            if (type == QCollectivePresenceInfo::None)
                matchNone = true;
            else {
                if (!inclause.isEmpty())
                    inclause.append(',');
                inclause.append(QString::number((unsigned int)type));
            }
        }

        if (matchNone) {
            // We have a match for "no presence information",
            // which means a left join
            mPresenceJoins = QStringList("LEFT contactpresence");

            // We want something like:
            // WHERE (contactpresence.recid IS NULL OR (contactpresence.recid = t1.recid AND contactpresence.status IN (...)))
            if (inclause.isEmpty()) {
                inclause = "(contactpresence.recid IS NULL)";
            } else {
                inclause.prepend("(contactpresence.recid IS NULL OR contactpresence.status IN (");
                inclause.append("))");
            }
            mPresenceFilter = inclause;
        } else {
            inclause.prepend("(contactpresence.status IN (");
            inclause.append("))");

            mPresenceFilter = inclause;
            mPresenceJoins = QStringList("contactpresence"); // normal inner join
        }
    }

    updateFilters();
}

void ContactSqlIO::clearPresenceFilter()
{
    mPresenceFilter.clear();
    mPresenceJoins.clear();

    updateFilters();
}

void ContactSqlIO::setFilter(const QString &text, int flags)
{
    /* assume text is not empty */
    QString searchFilter;
    QString chatFilter;

    QSqlDriver *driver = database().driver();

    QSqlField field("firstname", QVariant::String);

    field.setValue(text+"\%");
    QString escapedStartsWith = driver->formatValue(field);

    field.setValue("\% "+text+"\%");
    QString escapedWordStartsWith = driver->formatValue(field);

    /* should do on construction? or in ContactSqlIO construction */
    QMap<QChar, QString> pbt = phoneButtonText();

    QRegExp simIndex("^(\\d{1,3})#$");
    if (simIndex.exactMatch(text)) {
        mTextFilter = "simcardidmap.cardindex = " + simIndex.cap(1);
        mTextJoins = QStringList("simcardidmap");
        updateFilters();
        return;
    }
    bool allNumbers = true;
    for(int i = 0; i < text.length(); ++i) {
        if (!pbt.contains(text[i])) {
            allNumbers = false;
            break;
        }
    }
    if (allNumbers && !text.isEmpty()) {
        bool first = true;
        foreach (QContactModel::Field f, labelSearchFields()) {
            if (!first)
                searchFilter += " or ";
            else
                first = false;
            searchFilter += "(";
            QString fname = "t1."+sqlColumn(f);
            int i;
            /* Handle first letter with > and <, as will work on index and hence
               cut search results down much faster */
            QChar firstChar = text[0];
            searchFilter += fname + " >= '" + QString(pbt[firstChar][0]) + "'";
            if(firstChar.isDigit()) {
                int firstCharVal = firstChar.digitValue();
                QChar nextCharVal('0'+firstCharVal+1);
                if (firstCharVal < 9 && pbt.contains(nextCharVal))
                    searchFilter += " and " + fname + " < '" + QString(pbt[nextCharVal][0]) + "'";
            }
            for (i= 0; i < text.length(); i++) {
                searchFilter += " and ";
#ifdef QTOPIA_SQL_DIALECT_MIMER
                searchFilter += "substring(" + fname + " from " + QString::number(i+1) + " for 1) in (";
#else
                searchFilter += "lower(substr(" + fname + ", " + QString::number(i+1) + ", 1)) in (";
#endif
                QString letters = pbt[text[i]];
                for (int pos = 0; pos < letters.length(); ++pos) {
                    //if (letters[pos] == '\'' || letters[pos] == '@')
                        //break;
                    if (pos != 0)
                        searchFilter += ", ";
                    if (letters[pos] == '\'')
                        searchFilter += "''''";
                    else
                        searchFilter += "'" + letters[pos] + "'";
                }
                searchFilter += ")";
            }
            searchFilter += ")";
        }
    } else if (!text.isEmpty()) {
        /* text fields as mere 'starts with' */
        bool first = true;
        foreach (QContactModel::Field f, labelSearchFields()) {
            if (!first)
                searchFilter += " or ";
            else
                first = false;

            QString fname = "t1."+sqlColumn(f);
            // starts with
            searchFilter += "lower("+fname+") like " + escapedStartsWith + " ";
            // or contains a word starting with
            searchFilter += "or lower("+fname+") like " + escapedWordStartsWith + " ";
        }
    }

    /* flags
        ContainsMobileNumber
            inner join contactphonenumbers on recid group by contacts.recid;
        ContainsEmail
            inner join emailaddresses on recid group by contacts.recid;
        ContainsMailing
            inner join contactaddresses on recid group by contacts.recid;
        ContainsChat
            [depends on fields that have chat defined]
     */
    QStringList joins;
    if (flags & QContactModel::ContainsPhoneNumber)
        joins += "contactphonenumbers";
    if (flags & QContactModel::ContainsEmail)
        joins += "emailaddresses";
    if (flags & QContactModel::ContainsMailing)
        joins += "contactaddresses";
    if (flags & QContactModel::ContainsChat) {
        QStringList customNames;
        QStringList fields = QContactFieldDefinition::fields("chat");

        bool needsPhoneNumbers = false;
        foreach(QString field, fields) {
            QContactFieldDefinition def(field);
            QContactModel::Field f = QContactModel::identifierField(def.id());
            if (f == QContactModel::Invalid)
                customNames.append(def.id());
            else
                needsPhoneNumbers = true; // Assumed to be phonenumbers...
        }
        if (needsPhoneNumbers)
            joins += "contactphonenumbers";

        if (customNames.count() > 0) {
            joins += "contactcustom";
            QStringList escapedNames;
            foreach(QString name, customNames) {
                QSqlField field("fieldname", QVariant::String);
                field.setValue(name);
                escapedNames.append(driver->formatValue(field));
            }

            chatFilter = "(contactcustom.fieldname IN (" + escapedNames.join(",") + "))";
        }
    }
    mTextJoins = joins;
    if (!searchFilter.isEmpty() || !chatFilter.isEmpty()) {
        mTextFilter = "(";
        if (!searchFilter.isEmpty()) {
            mTextFilter.append('(');
            mTextFilter.append(searchFilter);
            mTextFilter.append(')');
            if (!chatFilter.isEmpty())
                mTextFilter.append(" AND ");
        }
        if (!chatFilter.isEmpty())
            mTextFilter.append(chatFilter);
        mTextFilter.append(')');
    } else
        mTextFilter.clear();

    updateFilters();
}

void ContactSqlIO::clearFilter()
{
    mTextFilter.clear();
    mTextJoins.clear();
    updateFilters();
}

void ContactSqlIO::updateFilters()
{
    QStringList joins = mTextJoins + mPresenceJoins;
    QPimSqlIO::setJoins(joins);

    QString filter = mTextFilter;
    if (!filter.isEmpty() && !mPresenceFilter.isEmpty())
        filter += " AND ";
    filter += mPresenceFilter;

    if (!filter.isEmpty())
        QPimSqlIO::setFilter(filter);
    else
        QPimSqlIO::clearFilter();
}

QList<QUniqueId> ContactSqlIO::matchChatAddress(const QString &address, const QString& provider) const
{
    if (address.isEmpty())
        return QList<QUniqueId>();

    QString chatFilter;
    QList<QContactFieldDefinition> customFields;

    QStringList fields = QContactFieldDefinition::fields("chat");

    // First grab any phone number fields..
    QList<QUniqueId> matched;

    foreach(QString field, fields) {
        QContactFieldDefinition def(field);
        if (provider.isEmpty() || def.provider() == provider) {
            QContactModel::Field f = QContactModel::identifierField(def.id());
            if (f == QContactModel::Invalid)
                customFields.append(def);
            else {
                QString uri = "collective:" + def.provider() + "/" + address;
                matched += match(f, uri, Qt::MatchExactly);
            }
        }
    }

    // Now find any custom fields (we already know the provider matches)
    foreach(QContactFieldDefinition def, customFields) {
        QPreparedSqlQuery q(database());
        QSqlDriver *driver = database().driver();
        QSqlField sqlfield("fieldtype", QVariant::String);

        QStringList conditions;

        sqlfield.setValue(address);
        conditions.append(QString("contactcustom.fieldvalue=%1").arg(driver->formatValue(sqlfield)));
        sqlfield.setValue(def.id()); // XXX def.id() is the custom field name
        conditions.append(QString("contactcustom.fieldname=%1").arg(driver->formatValue(sqlfield)));

        QString query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactcustom on (t1.recid = contactcustom.recid) "));
        q.prepare(query);
        q.exec();

        while (q.next())
            matched << QUniqueId::fromUInt(q.value(0).toUInt());
    }

    // Let something else do the ranking
    return matched;
}

QUniqueId ContactSqlIO::matchPhoneNumber(const QString &phnumber, int &bestMatch) const
{
    QString local = QPhoneNumber::localNumber(phnumber);
    if (local.isEmpty())
        return QUniqueId();

    /* For URL matching, strip off the protocol */
    int colonIdx = local.indexOf(':');
    if (colonIdx > 0 && colonIdx < local.length() - 1)
        local = local.mid(colonIdx + 1);

    /* Unfortunately we don't store the "localNumber" version of the entered number, so spaces, dashes etc
       cause an SQL LIKE clause to miss the number.. instead, maintain a cache of localNumber entries
       derived from the stored numbers. */
    if (mLocalNumberCache.isEmpty()) {
        QPreparedSqlQuery q(database());
        // Ensure numbers are added in a deterministic order
        q.prepare("SELECT recid, phone_number FROM contactphonenumbers ORDER BY recid");
        q.exec();

        while (q.next()) {
            QUniqueId numberId = QUniqueId::fromUInt(q.value(0).toUInt());
            QString phoneNumber = q.value(1).toString();
            QString localNumber = QPhoneNumber::localNumber(phoneNumber);

            if (!localNumber.isEmpty()) {
                colonIdx = localNumber.indexOf(':');
                if (colonIdx > 0 && colonIdx < localNumber.length() - 1)
                    localNumber = localNumber.mid(colonIdx + 1);
                // Index only by last 5 digits - resolve the rest by inspecting each.
                mLocalNumberCache.insert(localNumber.right(5), qMakePair(numberId, phoneNumber));
            }
        }
    }

    bestMatch = 0;
    QUniqueId bestContact;
    local = local.right(5);

    LocalNumberCache::const_iterator it = mLocalNumberCache.find(local), end = mLocalNumberCache.end();
    if (it != end) {
        // We have at least one exact match on the local number - see if there is a better one
        for ( ; (bestMatch != 100) && (it != end) && (it.key() == local); ++it) {
            const QPair<QUniqueId, QString>& matched(it.value());

            // The cache has everything in it, we may have filtered something out
            if (!contains(matched.first))
                continue;

            int match = QPhoneNumber::matchNumbers(phnumber, matched.second);
            if (match > bestMatch) {
                bestMatch = match;
                bestContact = matched.first;
            }
        }
    }

    qLog(Sql) << "QContactSqlIO::matchPhoneNumber() result:" << bestMatch << bestContact.toString();
    return bestContact;
}

QList<QUniqueId> ContactSqlIO::match(QContactModel::Field field, const QVariant& value, Qt::MatchFlags flags) const
{
    QList<QUniqueId> matches;
    QPreparedSqlQuery q(database());

    QSqlDriver *driver = database().driver();
    QSqlField sqlfield("firstname", QVariant::String);

    QString text = value.toString();
    QString whereClause("%1");
    if (!(flags & Qt::MatchCaseSensitive)) {
        whereClause = "lower(%1)";
        text = text.toLower();
    }

    QString escapedWildcards = text;
    escapedWildcards.replace('*', "[*]");
    escapedWildcards.replace('?', "[?]");

    // XXX Handle date & number fields differently (glob/string etc doesn't make sense)
    QString target;
    switch (flags & (Qt::MatchStartsWith | Qt::MatchExactly | Qt::MatchFixedString | Qt::MatchEndsWith | Qt::MatchContains | Qt::MatchWildcard)) {
        case Qt::MatchStartsWith:
            sqlfield.setValue(escapedWildcards + "*");
            whereClause.append(" GLOB ");
            whereClause.append(driver->formatValue(sqlfield));
            break;

        case Qt::MatchEndsWith:
            sqlfield.setValue("*" + escapedWildcards);
            whereClause.append(" GLOB ");
            whereClause.append(driver->formatValue(sqlfield));
            break;

        case Qt::MatchContains:
            sqlfield.setValue("*" + escapedWildcards + "*");
            whereClause.append(" GLOB ");
            whereClause.append(driver->formatValue(sqlfield));
            break;

        case Qt::MatchWildcard:
            sqlfield.setValue(text); // Assume it already has any wildcards needed
            whereClause.append(" GLOB ");
            whereClause.append(driver->formatValue(sqlfield));
            break;

        default:
            sqlfield.setValue(text);
            whereClause.append(" = ");
            whereClause.append(driver->formatValue(sqlfield));
            break;
    }

    QString query;
    QStringList conditions;
    /* Some fields are not in the main table */
    switch(field) {
            // Phone number table
        case QContactModel::HomePhone:
        case QContactModel::HomeFax:
        case QContactModel::HomeMobile:
        case QContactModel::HomePager:
        case QContactModel::HomeVOIP:
        case QContactModel::BusinessPhone:
        case QContactModel::BusinessFax:
        case QContactModel::BusinessMobile:
        case QContactModel::BusinessPager:
        case QContactModel::BusinessVOIP:
        case QContactModel::OtherPhone:
        case QContactModel::OtherFax:
        case QContactModel::OtherMobile:
        case QContactModel::OtherPager:
        case QContactModel::OtherVOIP:
            conditions << QString("contactphonenumbers.phone_type='%1'").arg(fieldToPhoneType(field));
            conditions << whereClause.arg("contactphonenumbers.phone_number");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactphonenumbers on (t1.recid = contactphonenumbers.recid) "));
            break;

            // contactpresence table
        case QContactModel::PresenceStatus:
            conditions << whereClause.arg("contactpresence.status");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;

        case QContactModel::PresenceStatusString:
            conditions << whereClause.arg("contactpresence.statusstring");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;

        case QContactModel::PresenceMessage:
            conditions << whereClause.arg("contactpresence.message");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;

        case QContactModel::PresenceDisplayName:
            conditions << whereClause.arg("contactpresence.displayname");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;

        case QContactModel::PresenceUpdateTime:
            conditions << whereClause.arg("contactpresence.updatetime");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;

        case QContactModel::PresenceCapabilities:
            conditions << whereClause.arg("contactpresence.capabilities");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;

        case QContactModel::PresenceAvatar:
            conditions << whereClause.arg("contactpresence.avatar");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactpresence on (t1.recid = contactpresence.recid)"));
            break;


            // Main table fields
        case QContactModel::NameTitle:
        case QContactModel::FirstName:
        case QContactModel::MiddleName:
        case QContactModel::LastName:
        case QContactModel::Suffix:
        case QContactModel::JobTitle:
        case QContactModel::Department:
        case QContactModel::Company:
        case QContactModel::DefaultEmail:
        case QContactModel::DefaultPhone:
        case QContactModel::Office:
        case QContactModel::Profession:
        case QContactModel::Assistant:
        case QContactModel::Manager:
        case QContactModel::Spouse:
        case QContactModel::Gender:
        case QContactModel::Birthday:
        case QContactModel::Anniversary:
        case QContactModel::Nickname:
        case QContactModel::Children:
        case QContactModel::LastNamePronunciation:
        case QContactModel::FirstNamePronunciation:
        case QContactModel::CompanyPronunciation:
        case QContactModel::HomeWebPage:
        case QContactModel::BusinessWebPage:
        case QContactModel::Label:
            conditions << whereClause.arg("t1." + sqlColumn(field));
            query = selectText("DISTINCT t1.recid", conditions);
            break;

            // Emails table
        case QContactModel::Emails:
            conditions << whereClause.arg("emailaddresses.addr");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN emailaddresses on (t1.recid = emailaddresses.recid) "));
            break;

        // Address table
        case QContactModel::BusinessStreet:
        case QContactModel::HomeStreet:
            conditions << QString("addresstype='%1'").arg(field == QContactModel::BusinessStreet ? QContact::Business : QContact::Home);
            conditions << whereClause.arg("contactaddresses.street");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactaddresses ON (t1.recid = contactaddresses.recid) "));
            break;

        case QContactModel::HomeCity:
        case QContactModel::BusinessCity:
            conditions << QString("addresstype='%1'").arg(field == QContactModel::BusinessCity ? QContact::Business : QContact::Home);
            conditions << whereClause.arg("contactaddresses.city");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactaddresses ON (t1.recid = contactaddresses.recid) "));
            break;

        case QContactModel::HomeState:
        case QContactModel::BusinessState:
            conditions << QString("addresstype='%1'").arg(field == QContactModel::BusinessState ? QContact::Business : QContact::Home);
            conditions << whereClause.arg("contactaddresses.state");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactaddresses ON (t1.recid = contactaddresses.recid) "));
            break;

        case QContactModel::HomeZip:
        case QContactModel::BusinessZip:
            conditions << QString("addresstype='%1'").arg(field == QContactModel::BusinessZip ? QContact::Business : QContact::Home);
            conditions << whereClause.arg("contactaddresses.zip");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactaddresses ON (t1.recid = contactaddresses.recid) "));
            break;

        case QContactModel::HomeCountry:
        case QContactModel::BusinessCountry:
            conditions << QString("addresstype='%1'").arg(field == QContactModel::BusinessCountry ? QContact::Business : QContact::Home);
            conditions << whereClause.arg("contactaddresses.country");
            query = model.selectText("DISTINCT t1.recid", conditions, QStringList(" JOIN contactaddresses ON (t1.recid = contactaddresses.recid) "));
            break;

            // Unhandled
        case QContactModel::Portrait:
        case QContactModel::Notes:
        case QContactModel::Identifier:
        case QContactModel::Categories:
        case QContactModel::Invalid:
            break;

    }

    QString sortColumnString = "t1." + model.orderBy().join(", t1.");

    if (!sortColumnString.isNull())
        query.append(" ORDER BY " + sortColumnString + ", t1.recid");
    else
        query.append(" ORDER BY t1.recid");

    q.prepare(query);
    q.exec();

    while (q.next()) {
        matches << QUniqueId::fromUInt(q.value(0).toUInt());
    }

    return matches;
}

/*!
  \internal

  Sets the value for field \a k of contact at \a row for the current filter and sort settings
  to \a v.  Returns true if successfully updated, otherwise returns false.
*/
bool ContactSqlIO::setContactField(int row, QContactModel::Field k,  const QVariant &v)
{
    QContact t = contact(row);
    if (QContactModel::setContactField(t, k, v))
        return updateContact(t);
    return false;
}

QList< QList<QVariant> > ContactSqlIO::mFormat;
QList< QContactModel::Field > ContactSqlIO::mFormatFieldOrder;

void ContactSqlIO::initFormat()
{
    QSettings config( "Trolltech", "Contacts" );
    config.beginGroup( "formatting" );
    int curfmtn = config.value( "NameFormat" ).toInt();
    QString curfmt = config.value( "NameFormatFormat"+QString::number(curfmtn) ).toString();
    setFormat( curfmt );
    config.endGroup();
}

/*!
  Returns the label for \a contact based on the other fields set for \a contact
  and the current label format set.

  \sa setFormat()
*/
QString ContactSqlIO::formattedLabel(const QContact &contact)
{
    if (mFormat.count() == 0)
        initFormat();
    foreach(QList<QVariant> f, mFormat) {
        QString value;
        QListIterator<QVariant> fit(f);
        bool match = true;
        while(fit.hasNext()) {
            QVariant v = fit.next();
            if (v.type() == QVariant::Int) {
                QContactModel::Field k = (QContactModel::Field)v.toInt();
                QString field = QContactModel::contactField(contact, k).toString();
                if (field.isEmpty()) {
                    match = false;
                    break;
                }
                value += field;
            } else if (v.type() == QVariant::String) {
                value += v.toString();
            }
        }
        if (match)
            return value;
    }
    return contact.firstName();
}

/*!
  Sets the format for labels of contacts returned by the QContactModel to
  \a value.

  The format is a set of pattern separated by '|'s.  Each pattern is
  a set of space separated tokens.  A token can either be _ for a space,
  an identifier as from identifierKey(), or any string.  The format for label
  will the first pattern for which all fields specified are non null for the contact.

  For example:

  LastName , _ FirstName | LastName | FirstName | Company
*/
void ContactSqlIO::setFormat(const QString &value) {

    QList< QList<QVariant> > newFormat;
    QList< QContactModel::Field > newFormatOrder;

    QList<QContactModel::Field> keys = labelKeys();
    QStringList tokens = value.split(' ');
    QList<QVariant> last;
    bool lastvalid = false;
    bool initiallyEmpty = (mFormat.count() == 0);
    while(tokens.count() > 0) {
        QString token = tokens.takeFirst();
        QContactModel::Field key = QContactModel::identifierField(token);
        if (keys.contains(key)) {
            lastvalid = true;
            last.append(key);
            if (!newFormatOrder.contains(key) && key != QContactModel::NameTitle && key != QContactModel::Suffix)
                newFormatOrder.append(key);
        } else if (token == "|") {
            if (lastvalid)
                newFormat.append(last);
            lastvalid = false;
            last.clear();
        } else {
            token.replace("_", " ");
            last.append(token);
        }
    }
    if (lastvalid)
        newFormat.append(last);
    if (newFormat.count() > 0) {
        mFormat = newFormat;
        mFormatFieldOrder = newFormatOrder;

        // The process that calls setFormat is responsible for
        // updating the database...
        updateLabelField();

        // if we weren't initially initializing, get our ios to change
        if (!initiallyEmpty) {
            foreach(ContactSqlIO * io, allIos)
                io->emitLabelFormatChanged();
        }
    }

}

QString ContactSqlIO::sqlLabel()
{
    int fc = formatCount();
    QString expression = "(CASE ";
    for (int i = 0; i < fc; i++) {
        QList<QVariant> f = format(i);
        expression += "WHEN ";
        bool firstkey = true;
        QListIterator<QVariant> it(f);
        while(it.hasNext()) {
            QVariant v = it.next();
            if (v.type() == QVariant::String)
                continue;
            if (!firstkey)
                expression += "AND ";
            firstkey = false;
            QContactModel::Field k = (QContactModel::Field)v.toInt();
            if (k == QContactModel::Invalid || k == QContactModel::Label)
                return sqlColumn(QContactModel::FirstName); // soft fail.
            expression += sqlColumn(k) + " IS NOT NULL ";
        }
        expression += "THEN ";
        QListIterator<QVariant> fit(f);
        while(fit.hasNext()) {
            QVariant v = fit.next();
            if (v.type() == QVariant::Int) {
                QContactModel::Field k = (QContactModel::Field)v.toInt();
                if (k == QContactModel::Invalid || k == QContactModel::Label)
                    return sqlColumn(QContactModel::FirstName); // soft fail.
                expression += sqlColumn(k) + " ";
            } else if (v.type() == QVariant::String) {
                expression += "\"" + v.toString() + "\" ";
            }
            if (fit.hasNext())
                expression += "|| ";
        }
    }
    expression += "ELSE NULL END)";

    return expression;
}

void ContactSqlIO::updateLabelField()
{
    // We should only update if our real label format has changed
    QSettings config( "Trolltech", "Contacts" );
    config.beginGroup( "formatting" );
    QString oldSql = config.value( "NameFormatSql" ).toString();

    QString currentSql = sqlLabel();

    if (oldSql != currentSql) {
        // Unfortunately we need to update the database for all the contacts.  Use
        // the constructed SQL so we don't have to read out everything and stuff it
        // back in.

        //
        // Testing shows that dropping the contactslabelindex and recreating it
        // gives better performance...
        //
        // but we have to make sure we aren't called in a transaction already.
        // if so, we don't make the change and leave the label column as is.
        // we don't update the ini field, however, so next time we get called
        // we will hopefully be able to update the label.
        QSqlQuery q(database());
        if(database().transaction()) {
            q.prepare("DROP INDEX contactslabelindex");
            q.exec();
            q.prepare("UPDATE contacts SET label=" + currentSql);
            q.exec();
            q.prepare("CREATE INDEX contactslabelindex ON contacts(label)");
            q.exec();
            database().commit();

            config.setValue("NameFormatSql", currentSql);
        }
    }
}

void ContactSqlIO::emitLabelFormatChanged()
{
    emit labelFormatChanged();
}

int ContactSqlIO::formatCount()
{
    if (mFormat.count() == 0)
        initFormat();
    return mFormat.count();
}

QList<QVariant> ContactSqlIO::format(int i)
{
    if (mFormat.count() == 0)
        initFormat();
    return mFormat[i];
}

QString ContactSqlIO::format()
{
    if (mFormat.count() == 0)
        initFormat();
    QString expression;

    int fc = ContactSqlIO::formatCount();

    for (int i = 0; i < fc; i++) {
        QList<QVariant> f = ContactSqlIO::format(i);
        QListIterator<QVariant> fit(f);
        while(fit.hasNext()) {
            QVariant v = fit.next();
            if (v.type() == QVariant::Int) {
                QContactModel::Field k = (QContactModel::Field)v.toInt();
                if (k == QContactModel::Invalid || k == QContactModel::Label)
                    return QContactModel::fieldIdentifier(QContactModel::FirstName); // soft fail.
                expression += QContactModel::fieldIdentifier(k) + " ";
            } else if (v.type() == QVariant::String) {
                QString s = v.toString();
                if (v == " ")
                    expression += "_ ";
                else
                    expression += v.toString() + " ";
            }
            if (fit.hasNext())
                expression += "| ";
        }
    }
    return expression;
}

QList<QContactModel::Field> ContactSqlIO::formatFieldOrder()
{
    if (mFormat.count() == 0)
        initFormat();
    return mFormatFieldOrder;
}

QList<QContactModel::Field> ContactSqlIO::labelKeys()
{
    QList<QContactModel::Field> result;
    result << QContactModel::NameTitle;
    result << QContactModel::FirstName;
    result << QContactModel::MiddleName;
    result << QContactModel::LastName;
    result << QContactModel::Suffix;
    result << QContactModel::Company;
    result << QContactModel::Department;
    result << QContactModel::JobTitle;
    result << QContactModel::Office;
    return result;
}

QStringList ContactSqlIO::labelIdentifiers()
{
    QList<QContactModel::Field> keys = labelKeys();
    QStringList result;
    foreach(QContactModel::Field k, keys) {
        result << QContactModel::fieldIdentifier(k);
    }
    return result;

}

// Note: this logic is also in QMailAddress.  Currently qtopiamail depends
// on qtopiapim, but not vice-versa, so a good way of sharing code will
// need to be determined.
static QString minimalEmailAddress(const QString& address)
{
    QString result;

    // Email addresses can contained quoted sections (delimited with ""), or
    // comments (including nested), delimited with parentheses.  Whitespace is
    // legal but removed to give the canonical form.

    int commentDepth = 0;
    bool quoted = false;
    bool escaped = false;

    // Remove any whitespace or comments from the email address
    const QChar* it = address.constData();
    const QChar* end = it + address.length();
    for ( ; it != end; ++it ) {
        if ( !escaped && ( *it == '\\' ) ) {
            escaped = true;
            continue;
        }

        if ( *it == '(' && !escaped && !quoted ) {
            commentDepth += 1;
        }
        else if ( *it == ')' && !escaped && !quoted && ( commentDepth > 0 ) ) {
            commentDepth -= 1;
        }
        else if ( quoted || !(*it).isSpace() ) {
            if ( !quoted && *it == '"' && !escaped ) {
                quoted = true;
            }
            else if ( quoted && *it == '"' && !escaped ) {
                quoted = false;
            }
            if ( commentDepth == 0 ) {
                result.append( *it );
            }
        }

        escaped = false;
    }

    return result;
}

QUniqueId ContactSqlIO::matchEmailAddress(const QString &address, int &bestMatch) const
{
    bestMatch = 0;
    const QString inputAddress(minimalEmailAddress(address));
    QList<QUniqueId> candidates = match(QContactModel::Emails, inputAddress, Qt::MatchContains);
    foreach (QUniqueId candidate, candidates) {
        QContact contact = ContactSqlIO::contact(candidate);

        QStringList addresses = contact.emailList();
        foreach (QString element, addresses) {
            if (inputAddress.compare(minimalEmailAddress(element)) == 0) {
                bestMatch = 100;
                return candidate;
            }
        }
    }
    return QUniqueId();
}

QMap<QChar, QString> ContactSqlIO::mPhoneButtonText;
bool ContactSqlIO::mPhoneButtonTextRead = false;

QMap<QChar, QString> ContactSqlIO::phoneButtonText()
{
    if (mPhoneButtonTextRead)
        return mPhoneButtonText;

    QTranslatableSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr

    cfg.beginGroup("TextButtons");

    QString buttons = cfg.value("Buttons").toString(); // No tr

    for (int i = 0; i < (int)buttons.length(); i++) {
        QChar ch = buttons[i];
        if (!ch.isDigit())
            continue;

        QString tapfunc("Tap"); // No tr
        tapfunc += ch;

        QString fn = cfg.value(tapfunc).toString();

        if (fn[0] != '\'' && fn[0] != '"' )
            continue;

        fn = fn.mid(1);
        mPhoneButtonText.insert(ch, fn);
    }

    mPhoneButtonTextRead = true;
    return mPhoneButtonText;
}


QList<QContactModel::Field> ContactSqlIO::labelSearchFields()
{
    QList<QContactModel::Field> l;
    l << QContactModel::FirstName;
    l << QContactModel::LastName;
    l << QContactModel::Company;
    return l;
}

QList<QContactModel::Field> ContactSqlIO::phoneNumberSearchFields()
{
    QList<QContactModel::Field> l;
    l << QContactModel::OtherPhone;
    l << QContactModel::OtherMobile;
    l << QContactModel::OtherFax;
    l << QContactModel::OtherPager;
    l << QContactModel::HomePhone;
    l << QContactModel::HomeMobile;
    l << QContactModel::HomeFax;
    l << QContactModel::HomePager;
    l << QContactModel::BusinessPhone;
    l << QContactModel::BusinessMobile;
    l << QContactModel::BusinessFax;
    l << QContactModel::BusinessPager;
    return l;
}

/***************
 * CONTEXT
 **************/
class QContactSync;
class QContactDefaultContextData
{
public:
    QContactDefaultContextData(QContactDefaultContext *parent) :
        importQuery("SELECT recid FROM contacts WHERE context = :cont AND ("
                "(title = :t OR (title IS NULL AND :t2 IS NULL)) AND "
                "(firstname = :fn OR (firstname IS NULL AND :fn2 IS NULL)) AND "
                "(middlename = :mn OR (middlename IS NULL AND :mn2 IS NULL)) AND "
                "(lastname = :ln OR (lastname IS NULL AND :ln2 IS NULL)) AND "
                "(suffix = :s OR (suffix IS NULL AND :s2 IS NULL))) "
                "OR (company = :c AND title IS NULL AND firstname IS NULL "
                "AND middlename IS NULL AND lastname IS NULL AND suffix IS NULL)"),
        syncer(0),
        mAppointmentAccess(0),
        mContactBirthdayContext(0),
        mContactAnniversaryContext(0),
        mDependencyList(0),
        q(parent)
    {}

    // Lazy
    QDependentAppointmentSqlIO* appointmentAccess()
    {
        if (!mAppointmentAccess)
            mAppointmentAccess = new QDependentAppointmentSqlIO(q);
        return mAppointmentAccess;
    }

    QContactBirthdayContext* contactBirthdayContext()
    {
        if (!mContactBirthdayContext)
            mContactBirthdayContext = new QContactBirthdayContext(q, appointmentAccess());
        return mContactBirthdayContext;
    }

    QContactAnniversaryContext* contactAnniversaryContext()
    {
        if (!mContactAnniversaryContext)
            mContactAnniversaryContext = new QContactAnniversaryContext(q, appointmentAccess());
        return mContactAnniversaryContext;
    }

    QString contactEventLabel(const QContact& contact)
    {
        QString description;
        if (!contact.firstName().isEmpty()) {
            if (contact.lastName().isEmpty())
                description = contact.firstName();
            else
                description = QContactDefaultContext::tr("%1 %2", "firstname lastname").arg(contact.firstName()).arg(contact.lastName());
        } else if (!contact.lastName().isEmpty())
            description = contact.lastName();
        else if (!contact.company().isEmpty())
            description = contact.company();
        return description;
    }

    // Helper functions
    void avoidNestedTransactions()
    {
        // Make sure we don't nest transactions..
        appointmentAccess()->setCurrentSyncTransactionTime(access->currentSyncTransactionTime());
    }

    bool addContactEvent(const QUniqueId& contactId, const QContact &contact, const QString& type)
    {
        bool ret = false;
        QAppointment a;
        QDate date;
        QDependentEventsContext *context;
        QString description = contactEventLabel(contact);
        QUniqueId newId;
        uint newContextId;
        if (type == "birthday") {
            date = contact.birthday();
            context = contactBirthdayContext();
            newContextId = context->mappedContext();
        } else {
            date = contact.anniversary();
            context = contactAnniversaryContext();
            newContextId = context->mappedContext();
        }

        // this is in QUniqueId, but private
        newId = QUniqueId::fromUInt(newContextId << 24 | contactId.toUInt() & 0x00ffffff);

        a.setStart(QDateTime(date, QTime(0,0)));
        a.setEnd(QDateTime(date, QTime(23,59)));
        a.setAllDay(true);
        a.setRepeatRule(QAppointment::Yearly);
        a.setDescription(description);
        a.setUid(newId);

        QUniqueId added = context->addAppointment(a, context->defaultSource());
        if (!added.isNull()) {
            if (QPimDependencyList::addDependency(contactId, added, type))
                ret = true;
            else {
                // Sigh, remove the appointment (XXX transactions)
                context->removeAppointment(added);
            }
        }

        return ret;
    }

    ContactSqlIO *access;
    QPreparedSqlQuery importQuery;
    QContactSync *syncer;

    QDependentAppointmentSqlIO *mAppointmentAccess;
    QContactBirthdayContext *mContactBirthdayContext;
    QContactAnniversaryContext *mContactAnniversaryContext;
    QPimDependencyList *mDependencyList;

    QContactDefaultContext* q;
};

QContactDefaultContext::QContactDefaultContext(QObject *parent, QObject *access)
    : QContactContext(parent)
{
    d = new QContactDefaultContextData(this);
    d->access = qobject_cast<ContactSqlIO *>(access);
    Q_ASSERT(d->access);
}

QIcon QContactDefaultContext::icon() const
{
    static QIcon conicon(":icon/contactgroup");
    return conicon;
}

QString QContactDefaultContext::description() const
{
    return tr("Default contact storage");
}

QString QContactDefaultContext::title() const
{
    return tr("Contacts");
}

bool QContactDefaultContext::editable() const
{
    return true;
}

QSet<QPimSource> QContactDefaultContext::sources() const
{
    QSet<QPimSource> list;
    list.insert(defaultSource());
    return list;
}

QPimSource QContactDefaultContext::defaultSource() const
{
    QPimSource s;
    s.context = d->access->contextId();
    s.identity = contextsource;
    return s;
}

QUuid QContactDefaultContext::id() const
{
    return d->access->contextId();
}

/* TODO set mapping to int */
void QContactDefaultContext::setVisibleSources(const QSet<QPimSource> &set)
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = d->access->contextFilter();

    if (set.contains(defaultSource()))
        filter.remove(context);
    else
        filter.insert(context);

    d->access->setContextFilter(filter);
}

QSet<QPimSource> QContactDefaultContext::visibleSources() const
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = d->access->contextFilter();
    if (!filter.contains(context))
        return sources();
    return QSet<QPimSource>();
}

bool QContactDefaultContext::exists(const QUniqueId &id) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    return d->access->exists(id) && d->access->context(id) == context;
}

QPimSource QContactDefaultContext::source(const QUniqueId &id) const
{
    if (exists(id))
        return defaultSource();
    return QPimSource();
}

bool QContactDefaultContext::updateContact(const QContact &contact)
{
    bool ret = d->access->updateContact(contact);

    if (ret) {
        d->avoidNestedTransactions();
        QList<QUniqueId> adeps = QPimDependencyList::typedChildrenRecords(contact.uid(), QString("anniversary"));
        QList<QUniqueId> bdeps = QPimDependencyList::typedChildrenRecords(contact.uid(), QString("birthday"));
        if (contact.anniversary().isValid()) {
            if (adeps.count() == 0) {
                d->addContactEvent(contact.uid(), contact, "anniversary");
            } else {
                foreach(QUniqueId id, adeps) {
                    QAppointment a = d->appointmentAccess()->appointment(id);
                    a.setDescription(d->contactEventLabel(contact));
                    a.setStart(QDateTime(contact.anniversary(), QTime(0,0)));
                    a.setEnd(QDateTime(contact.anniversary(), QTime(23,59)));
                    a.setAllDay(true);
                    d->contactAnniversaryContext()->updateAppointment(a);
                }
            }
        } else {
            foreach(QUniqueId id, adeps) {
                d->contactAnniversaryContext()->removeAppointment(id);
            }
        }
        if (contact.birthday().isValid()) {
            // Might need to add, if it was previously invalid
            if (bdeps.count() == 0) {
                d->addContactEvent(contact.uid(), contact, "birthday");
            } else {
                foreach(QUniqueId id, bdeps) {
                    QAppointment a = d->appointmentAccess()->appointment(id);
                    a.setDescription(d->contactEventLabel(contact));
                    a.setStart(QDateTime(contact.birthday(), QTime(0,0)));
                    a.setEnd(QDateTime(contact.birthday(), QTime(23,59)));
                    a.setAllDay(true);
                    d->contactBirthdayContext()->updateAppointment(a);
                }
            }
        } else {
            foreach(QUniqueId id, bdeps) {
                d->contactBirthdayContext()->removeAppointment(id);
            }
        }
    }
    return ret;
}

bool QContactDefaultContext::removeContact(const QUniqueId &id)
{
    bool ret = d->access->removeContact(id);

    if (ret) {
        QMap<QString, QUniqueId> deps = QPimDependencyList::childrenRecords(id);
        QMap<QString, QUniqueId>::const_iterator it = deps.constBegin();
        while(it != deps.constEnd()) {
            if (it.key() == "birthday") {
                if (d->contactBirthdayContext()->removeAppointment(it.value()))
                    QPimDependencyList::removeDependency(id, it.value(), it.key());
            } else if (it.key() == "anniversary") {
                if (d->contactAnniversaryContext()->removeAppointment(it.value()))
                    QPimDependencyList::removeDependency(id, it.value(), it.key());
            }
            ++it;
        }
    }

    return ret;
}

QUniqueId QContactDefaultContext::addContact(const QContact &contact, const QPimSource &source)
{
    if (source.isNull() || source == defaultSource()) {
        QUniqueId id = d->access->addContact(contact, defaultSource());
        if (!id.isNull()) {
            d->avoidNestedTransactions();
            if (contact.birthday().isValid())
                d->addContactEvent(id, contact, "birthday");
            if (contact.anniversary().isValid())
                d->addContactEvent(id, contact, "anniversary");
        }
        return id;
    }

    return QUniqueId();
}

static void mergeContact(QContact &destination, const QContact &source)
{
    // initially for importing sim contacts.  hence 'merge' only applies to
    // phone numbers of the contact.
    QStringList destNumbers = destination.phoneNumbers().values();
    QStringList sourceNumbers = source.phoneNumbers().values();

    QList<QContact::PhoneType> types = QContact::phoneTypes();

    foreach(QContact::PhoneType t, types) {
        QString number = source.phoneNumber(t);
        QString dnumber = destination.phoneNumber(t);
        // continue if number is already stored.
        if (number.isEmpty() || destNumbers.contains(number))
            continue;
        // continue if storing a number already from this contact (so above statement doesn't invalidate)
        if (!dnumber.isEmpty() && sourceNumbers.contains(dnumber)) {
            continue;
        }
        // now, we have a genuine conflict.
        // hard to say if should favor dest or source.  For now, siding with dest
        // since contact is still on source hence favoring dest does not reduce
        // information stored overall.
        if (!dnumber.isEmpty())
            continue;
        destination.setPhoneNumber(t, number);
    }
}

/*
   needs to export just this filter, not all shown
   by access belong to this context
*/
QList<QContact> QContactDefaultContext::exportContacts(const QPimSource &, bool &ok) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    // unfiltered? current filter?
    QList<QContact> result;
    // temporary to allow separate filters
    ContactSqlIO *exportAccess = new ContactSqlIO(0);
    exportAccess->setCategoryFilter(d->access->categoryFilter());
    QSet<int> set;
    set.insert(context);
    exportAccess->setContextFilter(set, QPimSqlIO::RestrictToContexts);

    // don't even try exporting more than this.  Exporting involves going via a list,
    // which can be expensive.  Since most sim cards only handle about 200 contacts,
    // this is mostly a 'don't try something silly' check.
    if (exportAccess->count() > 1000) {
        ok = false;
        delete exportAccess;
        return result;
    }
    for(int i = 0; i < exportAccess->count(); ++i) {
        result.append(exportAccess->contact(i));
    }
    ok = true;
    delete exportAccess;
    return result;
}

QContact QContactDefaultContext::exportContact(const QUniqueId &id, bool &ok) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    ok = d->access->exists(id) && d->access->context(id) == context;
    if (ok)
        return d->access->contact(id);
    return QContact();
}

/* for each contact, check if an identical named one is in the list.  if so, update it rather than add.
   */
bool QContactDefaultContext::importContacts(const QPimSource &s, const QList<QContact> &list)
{
    if (list.isEmpty())
        return false;
    int context = QPimSqlIO::sourceContext(s);

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    if (!d->access->startTransaction(syncTime))
        return false;

    foreach(QContact c, list) {
        // contacts are pretty much uniquely identified by name.  hence do a search for uid on the name fields.
        d->importQuery.prepare();
        d->importQuery.bindValue(":cont", context);
        d->importQuery.bindValue(":t", c.nameTitle());
        d->importQuery.bindValue(":t2", c.nameTitle());
        d->importQuery.bindValue(":fn", c.firstName());
        d->importQuery.bindValue(":fn2", c.firstName());
        d->importQuery.bindValue(":mn", c.middleName());
        d->importQuery.bindValue(":mn2", c.middleName());
        d->importQuery.bindValue(":ln", c.lastName());
        d->importQuery.bindValue(":ln2", c.lastName());
        d->importQuery.bindValue(":s", c.suffix());
        d->importQuery.bindValue(":s2", c.suffix());
        d->importQuery.bindValue(":c", c.company());

        if (!d->importQuery.exec()) {
            d->access->abortTransaction();
            return false;
        }
        if (d->importQuery.next()) {
            QUniqueId u(d->importQuery.value(0).toByteArray());
            QContact current = d->access->contact(u);
            mergeContact(current, c); // merges phone number information from sim to phone.  favours phone.
            d->importQuery.reset();
            updateContact(current);
        } else {
            d->importQuery.reset();
            addContact(c, s);
        }
    }
    return d->access->commitTransaction();
}

#include "qcontactsqlio.moc"
