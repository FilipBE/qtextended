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

#ifndef QMAILSTORE_P_H
#define QMAILSTORE_P_H

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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QCache>
#include <QTimer>
#include <QtopiaIpcEnvelope>
#include "qmailfolder.h"
#include "qmailmessage.h"
#include "qmailmessagekey.h"
#include "qmailmessagesortkey.h"
#include "qmailstore.h"

#include "qtopialog.h"

//#define QMAILSTORE_LOG_SQL //define to enable SQL query logging
//#define QMAILSTORE_USE_RTTI //define if RTTI is available to assist debugging

#ifdef QMAILSTORE_USE_RTTI
#include <typeinfo>
#endif

class QMailAccountId;
class QMailFolderId;
class QMailMessageId;
class QMailAccountKey;
class QMailFolderKey;
class QMailFolderSortKey;
class QMailAccountSortKey;
class QSqlRecord;
class QSqlQuery;
class QMailAccount;
class QSettings;
class QMailMessageRemovalRecord;
class ProcessMutex;
class ProcessReadLock;


typedef QMap<QMailMessageKey::Property,QString> MessagePropertyMap;
typedef QList<QMailMessageKey::Property> MessagePropertyList;

template <typename T, typename ID> 
class MailStoreCache
{
public:
    MailStoreCache(unsigned int size = 10);
    ~MailStoreCache();

    T lookup(const ID& id) const;
    void insert(const T& item);
    bool contains(const ID& id) const;
    void remove(const ID& id);

private:
    QCache<quint64,T> mCache;
};


class MailStoreTransaction
{
    QMailStorePrivate *m_d;
    bool m_initted;
    bool m_committed;

public:
    MailStoreTransaction(QMailStorePrivate *);
    ~MailStoreTransaction();

    bool commit();

    bool committed() const;
};

class MailStoreReadLock
{
    QMailStorePrivate *m_d;
    bool m_locked;

public:
    MailStoreReadLock(QMailStorePrivate *);
    ~MailStoreReadLock();
};

class QMailStorePrivate : public QObject
{
    Q_OBJECT

public:
    static const int headerCacheSize = 100;
    static const int folderCacheSize = 10;
    static const int accountCacheSize = 10;
    static const int lookAhead = 5;
    static const int maxComparitorsCutoff = 50;
    static const int maxNotifySegmentSize = 0;

    static QMailMessageKey::Properties updatableMessageProperties();
    static QMailMessageKey::Properties allMessageProperties();
    static const MessagePropertyMap& messagePropertyMap();
    static const MessagePropertyList& messagePropertyList();
    static QString comparitorWarning();
    static QString accountAddedSig();
    static QString accountRemovedSig();
    static QString accountUpdatedSig();
    static QString accountContentsModifiedSig();
    static QString messageAddedSig();
    static QString messageRemovedSig();
    static QString messageUpdatedSig();
    static QString folderAddedSig();
    static QString folderUpdatedSig();
    static QString folderRemovedSig();
    static QString folderContentsModifiedSig();
    static QString messageRemovalRecordsAddedSig();
    static QString messageRemovalRecordsRemovedSig();
    static QString messagesBodyPath();
    static QString accountSettingsFileName();
    static QString accountSettingsPrefix();
    static const int accountSettingsFileVersion = 100;
    static QString globalAccountSettingsKey();
    static int accountSettingsFileIdentifier();
    static QString messageFilePath(const QString &fileName);
    static int messageFileIdentifier(const QString &filePath);

    int databaseIdentifier(int n) const;

    enum ChangeType
    {
        Added,
        Removed,
        Updated,
        ContentsModified
    };

public:
    QMailStorePrivate(QMailStore* parent);
    ~QMailStorePrivate();

    void initStore();
    static bool initialized();

    QSqlQuery prepare(const QString& sql);
    bool execute(QSqlQuery& q, bool batch = false);
    int lastErrorNumber(void) const;

    bool idExists(const QMailAccountId& id, const QString& table = QString());
    bool idExists(const QMailFolderId& id, const QString& table = QString());
    bool idExists(const QMailMessageId& id, const QString& table = QString());

    QMailMessageMetaData messageMetaData(const QSqlRecord& r,
                                         QMailMessageKey::Properties recordProperties,
                                         const QMailMessageKey::Properties& properties = allMessageProperties()) const;
    QMailMessage buildQMailMessage(const QSqlRecord& r,
                                   const QMailMessageKey::Properties& properties = allMessageProperties()) const;
    QMailFolder buildQMailFolder(const QSqlRecord& r) const;

    QMailMessageRemovalRecord buildQMailMessageRemovalRecord(const QSqlRecord& r) const;

    QString buildOrderClause(const QMailAccountSortKey& key) const;
    QString buildOrderClause(const QMailFolderSortKey& key) const;
    QString buildOrderClause(const QMailMessageSortKey& key) const;

    bool containsProperty(const QMailMessageKey::Property& p,
                          const QMailMessageKey& key) const;
    bool containsProperty(const QMailMessageSortKey::Property& p,
                          const QMailMessageSortKey& sortkey) const;

    QString buildWhereClause(const QMailAccountKey& k) const;
    QString buildWhereClause(const QMailMessageKey& k) const;
    QString buildWhereClause(const QMailFolderKey& k) const;

    void bindWhereData(const QMailAccountKey& key, QSqlQuery& query) const;
    void bindWhereData(const QMailMessageKey& key, QSqlQuery& query) const;
    void bindWhereData(const QMailFolderKey& key, QSqlQuery& query) const;

    void bindUpdateData(const QMailMessageKey::Properties& properties,
                        const QMailMessageMetaData& data,
                        QSqlQuery& query) const;

    QString expandProperties(const QMailMessageKey::Properties& p,
                             bool update = false) const;

    int numComparitors(const QMailAccountKey& key) const;
    int numComparitors(const QMailMessageKey& key) const;
    int numComparitors(const QMailFolderKey& key) const;
    void checkComparitors(const QMailAccountKey& key) const;
    void checkComparitors(const QMailMessageKey& key) const;
    void checkComparitors(const QMailFolderKey& key) const;

    void notifyAccountsChange(const ChangeType& changeType,
                              const QMailAccountIdList& ids);
    void notifyMessagesChange(const ChangeType& changeType,
                              const QMailMessageIdList& ids);
    void notifyFoldersChange(const ChangeType& changeType,
                             const QMailFolderIdList& ids);

    void notifyMessageRemovalRecordsChange(const ChangeType& changeType,
                                           const QMailAccountIdList& ids);

    bool saveMessageBody(const QMailMessage& m, const QString& newFile);
    bool removeMessageBody(const QString& fileName);
    bool updateMessageBody(const QString& fileName, QMailMessage* data);
    bool loadMessageBody(const QString& fileName, QMailMessage* out) const;

    template<typename AccountType>
    static bool saveAccountSettings(const AccountType& account);

    template<typename AccountType>
    static bool updateAccountSettings(const AccountType& account, bool removeExisting);

    template<typename AccountType>
    static bool loadAccountSettings(AccountType* account);

    static bool removeAccountSettings(const QMailAccountId& accountId);

    bool checkPreconditions(const QMailFolder& folder, bool update = false);

    bool deleteMessages(const QMailMessageKey& key,
                        QMailStore::MessageRemovalOption option,
                        QMailMessageIdList& deletedMessageIds,
                        QStringList& expiredMailfiles,
                        QMailAccountIdList& modifiedAccounts,
                        QMailFolderIdList& modifiedFolders);

    bool deleteFolders(const QMailFolderKey& key,
                       QMailStore::MessageRemovalOption option,
                       QMailFolderIdList& deletedFolders,
                       QMailMessageIdList& deletedMessageIds,
                       QStringList& expiredMailfiles,
                       QMailAccountIdList& modifiedAccounts);

    bool deleteAccounts(const QMailAccountKey& key,
                        QMailAccountIdList& deletedAccounts,
                        QMailFolderIdList& deletedFolders,
                        QMailMessageIdList& deletedMessageIds,
                        QStringList& expiredMailfile);

    QSqlQuery simpleQuery(const QString& statement, const QString& descriptor, bool batch = false);
    QSqlQuery simpleQuery(const QString& statement, const QVariantList& bindValues, const QString& descriptor, bool batch = false);

    QSqlQuery simpleQuery(const QString& statement, const QMailMessageKey& key, const QString& descriptor, bool batch = false);
    QSqlQuery simpleQuery(const QString& statement, const QVariantList& bindValues, const QMailMessageKey& key, const QString& descriptor, bool batch = false);

    QSqlQuery simpleQuery(const QString& statement, const QMailFolderKey& key, const QString& descriptor, bool batch = false);
    QSqlQuery simpleQuery(const QString& statement, const QVariantList& bindValues, const QMailFolderKey& key, const QString& descriptor, bool batch = false);

    QSqlQuery simpleQuery(const QString& statement, const QMailAccountKey& key, const QString& descriptor, bool batch = false);
    QSqlQuery simpleQuery(const QString& statement, const QVariantList& bindValues, const QMailAccountKey& key, const QString& descriptor, bool batch = false);

    void createTemporaryTable(const QMailMessageKey &key) const;

    bool asynchronousEmission() const;

    void flushIpcNotifications();

    static QString parseSql(QTextStream& ts);

    static QString expandValueList(const QVariantList& valueList);
    static QString expandValueList(int valueCount);

    static QString generateUniqueFileName();
    static QString randomString(int length);

    static QVariantList whereClauseValues(const QMailAccountKey& key);
    static QVariantList whereClauseValues(const QMailMessageKey& key);
    static QVariantList whereClauseValues(const QMailFolderKey& key);

    static QVariantList updateValues(const QMailMessageKey::Properties& properties, const QMailMessageMetaData& data);

    static void updateMessageValues(const QMailMessageKey::Properties& properties, const QVariantList& values, QMailMessageMetaData& metaData);

    static QString temporaryTableName(const QMailMessageKey &key);

    template<typename ValueType>
    static ValueType extractValue(const QVariant& var, const ValueType &defaultValue = ValueType());

public slots:
    void processIpcMessageQueue();
    void ipcMessage(const QString& message, const QByteArray& data);
    void notifyFlush();

private:
    friend class MailStoreTransaction;
    friend class MailStoreReadLock;

    bool ensureVersionInfo();
    qint64 tableVersion(const QString &name) const;
    bool setTableVersion(const QString &name, qint64 version);

    bool createTable(const QString &name);

    typedef QPair<QString, qint64> TableInfo;
    bool setupTables(const QList<TableInfo> &tableList);

    typedef QPair<quint64, QString> FolderInfo;
    bool setupFolders(const QList<FolderInfo> &folderList);

    bool transaction(void);
    bool commit(void);
    void rollback(void);

    ProcessMutex& databaseMutex(void) const;
    ProcessReadLock& databaseReadLock(void) const;

    static ProcessMutex& accountSettingsFileMutex(void);
    static ProcessMutex& messageFileMutex(void);

    bool idValueExists(quint64 id, const QString& table);

    void messageMetaData(const QSqlRecord& r, QMailMessageKey::Properties recordProperties, const QMailMessageKey::Properties& properties, QMailMessageMetaData* metaData) const;

    QSqlQuery prepareSimpleQuery(const QString& statement, const QString& descriptor);
    void executeSimpleQuery(QSqlQuery& query, const QVariantList& bindValues, const QVariantList& whereValues, const QString& descriptor, bool batch);

    void setLastError(const QSqlError&, const QString& description = QString(), const QString& statement = QString());
    void clearLastError(void);

    void destroyTemporaryTables(void);

    bool emitIpcNotification();

    typedef QMap<QString, void (QMailStore::*)(const QMailAccountIdList&)> AccountUpdateSignalMap;
    static AccountUpdateSignalMap initAccountUpdateSignals();

    typedef QMap<QString, void (QMailStore::*)(const QMailFolderIdList&)> FolderUpdateSignalMap;
    static FolderUpdateSignalMap initFolderUpdateSignals();

    typedef QMap<QString, void (QMailStore::*)(const QMailMessageIdList&)> MessageUpdateSignalMap;
    static MessageUpdateSignalMap initMessageUpdateSignals();

public:
    QMailStore* q;
    mutable QSqlDatabase database;
    
    mutable QMailMessageIdList lastQueryMessageResult;
    mutable MailStoreCache<QMailMessageMetaData, QMailMessageId> headerCache;
    mutable MailStoreCache<QMailFolder, QMailFolderId> folderCache;
    mutable MailStoreCache<QMailAccount, QMailAccountId> accountCache;
    mutable QList<const QMailMessageKey*> requiredTableKeys;
    mutable QList<const QMailMessageKey*> temporaryTableKeys;
    QList<const QMailMessageKey*> expiredTableKeys;
    bool inTransaction;
    bool asyncEmission;
    mutable int lastError;
    ProcessMutex *mutex;
    ProcessReadLock *readLock;
    static bool init;
    static ProcessMutex *settingsMutex;
    static ProcessMutex *messageMutex;
    
    //IPC Condensation
    QTimer flushTimer;
    QTimer preFlushTimer;
    QSet<QMailAccountId> addAccountsBuffer;
    QSet<QMailFolderId> addFoldersBuffer;
    QSet<QMailMessageId> addMessagesBuffer;
    QSet<QMailAccountId> addMessageRemovalRecordsBuffer;
    QSet<QMailMessageId> updateMessagesBuffer;
    QSet<QMailFolderId> updateFoldersBuffer;
    QSet<QMailAccountId> updateAccountsBuffer;
    QSet<QMailAccountId> removeMessageRemovalRecordsBuffer;
    QSet<QMailMessageId> removeMessagesBuffer;
    QSet<QMailFolderId> removeFoldersBuffer;
    QSet<QMailAccountId> removeAccountsBuffer;
    QSet<QMailFolderId> folderContentsModifiedBuffer;
    QSet<QMailAccountId> accountContentsModifiedBuffer;

    QTimer queueTimer;
    QList <QPair<QString, QByteArray> > messageQueue;
};

template <typename ValueType>
ValueType QMailStorePrivate::extractValue(const QVariant &var, const ValueType &defaultValue)
{
    if (!qVariantCanConvert<ValueType>(var)) {
        qWarning() << "QMailStorePrivate::extractValue - Cannot convert variant to:"
#ifdef QMAILSTORE_USE_RTTI
                   << typeid(ValueType).name();
#else
                   << "requested type";
#endif
        return defaultValue;
    }

    return qVariantValue<ValueType>(var);
}


template <typename T, typename ID> 
MailStoreCache<T, ID>::MailStoreCache(unsigned int cacheSize)
:
    mCache(cacheSize)
{
}

template <typename T, typename ID> 
MailStoreCache<T, ID>::~MailStoreCache()
{
}

template <typename T, typename ID> 
T MailStoreCache<T, ID>::lookup(const ID& id) const
{
   if(!id.isValid())
       return T();
   else
   {
       T* cachedItem = mCache.object(id.toULongLong());
       if(!cachedItem)
           return T();
       else return *cachedItem;
   }
}

template <typename T, typename ID> 
void MailStoreCache<T, ID>::insert(const T& item)
{
    if(!item.id().isValid())
        return;
    else
        mCache.insert(item.id().toULongLong(),new T(item));
}

template <typename T, typename ID> 
bool MailStoreCache<T, ID>::contains(const ID& id) const
{
    return mCache.contains(id.toULongLong());
}

template <typename T, typename ID> 
void MailStoreCache<T, ID>::remove(const ID& id)
{
    mCache.remove(id.toULongLong());
}

#endif
