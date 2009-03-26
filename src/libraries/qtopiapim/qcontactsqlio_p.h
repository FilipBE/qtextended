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

#ifndef QCONTACTSQLIO_P_H
#define QCONTACTSQLIO_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiasql.h>
#include <qsqlquery.h>
#include <qvariant.h>
#include <qmap.h>
#include <qhash.h>
#include <qlist.h>
#include <qcache.h>
#include <qset.h>
#include <qstring.h>

#include <qcontact.h>

#include "qcontactmodel.h"
#include "qpimsqlio_p.h"
#include "qpimsource.h"
#include "qcollectivepresenceinfo.h"

class QContactDefaultContextData;
class QTOPIAPIM_EXPORT QContactDefaultContext : public QContactContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QContactDefaultContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;

    using QContactContext::title;
    QString title() const;

    // better to be flags ?
    using QContactContext::editable;
    bool editable() const; // default true

    QPimSource defaultSource() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QContactContext::exists;
    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateContact(const QContact &);
    bool removeContact(const QUniqueId &);
    QUniqueId addContact(const QContact &, const QPimSource &);

    bool importContacts(const QPimSource &, const QList<QContact> &);

    QList<QContact> exportContacts(const QPimSource &, bool &) const;
    QContact exportContact(const QUniqueId &id, bool &) const;

private:
    QContactDefaultContextData *d;
};

class QSqlPimTableModel;
class ContactSimpleQueryCache;
class QTOPIA_AUTOTEST_EXPORT ContactSqlIO : public QPimSqlIO {

    Q_OBJECT

public:
    explicit ContactSqlIO( QObject *parent = 0, const QString &name = QString());

    ~ContactSqlIO();

    static QList<QContactModel::Field> labelKeys();
    static QStringList labelIdentifiers();

    static void setFormat(const QString &);
    static QString format();
    static QList<QContactModel::Field> formatFieldOrder();

    static int formatCount();
    static QList<QVariant> format(int);

    static QString formattedLabel(const QContact &);

    virtual QUniqueId matchEmailAddress(const QString &, int &) const;
    virtual QUniqueId matchPhoneNumber(const QString &, int &) const;
    virtual QList<QUniqueId> matchChatAddress(const QString &, const QString&) const;

    static QMap<QChar, QString> phoneButtonText();
    static QList<QContactModel::Field> labelSearchFields();
    static QList<QContactModel::Field> phoneNumberSearchFields();
    QUuid contextId() const;

    QContact simpleContact(int row) const;

    static QPresenceTypeMap presenceStatus(const QUniqueId& id);
    static QPresenceStringMap presenceStatusString(const QUniqueId& id);
    static QPresenceStringMap presenceDisplayName(const QUniqueId& id);
    static QPresenceStringMap presenceMessage(const QUniqueId& id);
    static QPresenceStringMap presenceAvatar(const QUniqueId& id);
    static QPresenceCapabilityMap presenceCapabilities(const QUniqueId& id);
    static QPresenceDateTimeMap presenceUpdateTime(const QUniqueId& id);

    QList<QUniqueId> match(QContactModel::Field field, const QVariant& value, Qt::MatchFlags flags) const;

    QContact contact(const QUniqueId &) const;
    QContact contact(int row) const;

    bool removeContact(int row);
    bool removeContact(const QUniqueId & id);
    bool removeContact(const QContact &t);
    bool removeContacts(const QList<int> &rows);
    bool removeContacts(const QList<QUniqueId> &ids);

    bool updateContact(const QContact &t);
    QUniqueId addContact(const QContact &contact, const QPimSource &s)
    { return addContact(contact, s, true); }
    QUniqueId addContact(const QContact &contact, const QPimSource &, bool);

    QVariant contactField(int row, QContactModel::Field k) const;
    bool setContactField(int row, QContactModel::Field k,  const QVariant &);

    void setFilter(const QString &, int);
    void clearFilter();

    void setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType> types);
    void clearPresenceFilter();

    void invalidateCache();

    void setOrderBy(QList<QContactModel::SortField> list);

    void checkAdded(const QUniqueId &) { invalidateCache(); }
    void checkRemoved(const QUniqueId &) { invalidateCache(); }
    void checkRemoved(const QList<QUniqueId> &) { invalidateCache(); }
    void checkUpdated(const QUniqueId &) { invalidateCache(); }

signals:
    void labelFormatChanged();

protected:
    void bindFields(const QPimRecord &, QPreparedSqlQuery &) const;
    QList<QContactModel::SortField> labelSortColumns() const;

    static QString sqlColumn(QContactModel::Field k);

    bool updateExtraTables(uint, const QPimRecord &);
    bool insertExtraTables(uint, const QPimRecord &);
    bool removeExtraTables(uint);

private slots:
    void updateSqlLabel();

private:
    static QString sqlLabel();
    static void updateLabelField();
    QString sqlField(QContactModel::Field) const;
    bool canUpdate(QContactModel::Field) const;
    QContact::PhoneType fieldToPhoneType(QContactModel::Field) const;
    void initMaps();

    void updateFilters();

    mutable bool contactByRowValid;
    mutable QContact lastContact;
    QString sqlLabelCache;

    QString mSearchText;
    int mSearchIndex;
    QSqlPimTableModel *searchTable;

    bool tmptable;

    typedef QMultiHash<QString, QPair<QUniqueId, QString> > LocalNumberCache;
    mutable LocalNumberCache mLocalNumberCache;

    static QMap<QContactModel::Field, QString> mFields;
    static QMap<QContactModel::Field, bool> mUpdateable;

    // Saved queries
    mutable QPreparedSqlQuery contactQuery;
    mutable QPreparedSqlQuery emailsQuery;
    mutable QPreparedSqlQuery addressesQuery;
    mutable QPreparedSqlQuery phoneQuery;
    mutable QPreparedSqlQuery insertEmailsQuery;
    mutable QPreparedSqlQuery insertAddressesQuery;
    mutable QPreparedSqlQuery insertPhoneQuery;
    mutable QPreparedSqlQuery insertPresenceQuery;
    mutable QPreparedSqlQuery removeEmailsQuery;
    mutable QPreparedSqlQuery removeAddressesQuery;
    mutable QPreparedSqlQuery removePhoneQuery;
    mutable QPreparedSqlQuery removePresenceQuery;
    mutable ContactSimpleQueryCache *simpleCache;

    void emitLabelFormatChanged();

    QList<QContactModel::SortField> mSortList;

    // Saved conditions & joins for setFilter & setPresenceFilter
    QString mPresenceFilter;
    QStringList mPresenceJoins;
    QString mTextFilter;
    QStringList mTextJoins;

    static void initFormat();

    static QSet<ContactSqlIO*> allIos;

    static QList< QList<QVariant> > mFormat;
    static QList< QContactModel::Field > mFormatFieldOrder;
    static bool mPhoneButtonTextRead;
    static QMap<QChar, QString> mPhoneButtonText;
};

#endif
