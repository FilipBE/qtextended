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

#ifndef QMAILSTORE_H
#define QMAILSTORE_H

#include "qmailmessage.h"
#include "qmailmessagekey.h"
#include "qmailmessagesortkey.h"
#include "qmailfolderkey.h"
#include "qmailfoldersortkey.h"
#include "qmailaccountkey.h"
#include "qmailaccountsortkey.h"
#include "qmailmessageremovalrecord.h"

#include <QSharedData>
#include <qtopiaglobal.h>

class QMailAccountId;
class QMailAccount;
class QMailFolderId;
class QMailFolder;
class QMailMessageId;
class QMailStore;
class QMailStorePrivate;

class MailStoreTransaction;
class MailStoreReadLock;
class AccountConfiguration;

#ifdef QMAILSTOREINSTANCE_DEFINED_HERE
static QMailStore* QMailStoreInstance();
#endif

class QTOPIAMAIL_EXPORT QMailStore : public QObject
{
    Q_OBJECT

public:
    enum ReturnOption
    {
        ReturnAll,
        ReturnDistinct
    };

    enum MessageRemovalOption
    {
        NoRemovalRecord = 1,
        CreateRemovalRecord
    };

    struct ReadAccess {};
    struct WriteAccess {};

public:
    virtual ~QMailStore();

    bool initialized() const;
    static bool storeInitialized();

    bool addAccount(QMailAccount* account, AccountConfiguration* config = 0);
    bool addFolder(QMailFolder* f);
    bool addMessage(QMailMessage* m);
    bool addMessage(QMailMessageMetaData* m);

    bool removeAccount(const QMailAccountId& id);
    bool removeAccounts(const QMailAccountKey& key);

    bool removeFolder(const QMailFolderId& id, MessageRemovalOption option = NoRemovalRecord);
    bool removeFolders(const QMailFolderKey& key, MessageRemovalOption option = NoRemovalRecord);

    bool removeMessage(const QMailMessageId& id, MessageRemovalOption option = NoRemovalRecord);
    bool removeMessages(const QMailMessageKey& key, MessageRemovalOption option = NoRemovalRecord);

    bool updateAccount(QMailAccount* account, AccountConfiguration* config = 0);
    bool updateFolder(QMailFolder* f);
    bool updateMessage(QMailMessage* m);
    bool updateMessage(QMailMessageMetaData* m);
    bool updateMessagesMetaData(const QMailMessageKey& key,
                                const QMailMessageKey::Properties& properties,
                                const QMailMessageMetaData& data);
    bool updateMessagesMetaData(const QMailMessageKey& key, quint64 messageStatus, bool set);

    int countAccounts(const QMailAccountKey& key = QMailAccountKey()) const;
    int countFolders(const QMailFolderKey& key = QMailFolderKey()) const;
    int countMessages(const QMailMessageKey& key = QMailMessageKey()) const;

    int sizeOfMessages(const QMailMessageKey& key = QMailMessageKey()) const;

    const QMailAccountIdList queryAccounts(const QMailAccountKey& key = QMailAccountKey(),
                                           const QMailAccountSortKey& sortKey = QMailAccountSortKey()) const;
    const QMailFolderIdList queryFolders(const QMailFolderKey& key = QMailFolderKey(),
                                         const QMailFolderSortKey& sortKey = QMailFolderSortKey()) const;
    const QMailMessageIdList queryMessages(const QMailMessageKey& key = QMailMessageKey(),
                                           const QMailMessageSortKey& sortKey = QMailMessageSortKey()) const;

    QMailAccount account(const QMailAccountId& id) const;

    QMailFolder folder(const QMailFolderId& id) const;

    QMailMessage message(const QMailMessageId& id) const;
    QMailMessage message(const QString& uid, const QMailAccountId& accountId) const;

    QMailMessageMetaData messageMetaData(const QMailMessageId& id) const;
    QMailMessageMetaData messageMetaData(const QString& uid, const QMailAccountId& accountId) const;
    const QMailMessageMetaDataList messagesMetaData(const QMailMessageKey& key,
                                                    const QMailMessageKey::Properties& properties,
                                                    ReturnOption option = ReturnAll) const;

    const QMailMessageRemovalRecordList messageRemovalRecords(const QMailAccountId& parentAccountId,
                                                              const QString& fromMailbox = QString()) const;

    bool purgeMessageRemovalRecords(const QMailAccountId& parentAccountId, const QStringList& serverUid = QStringList());

    bool restoreToPreviousFolder(const QMailMessageId& id);
    bool restoreToPreviousFolder(const QMailMessageKey& key);

    bool asynchronousEmission() const;

    bool registerFolderStatusFlag(const QString& name);
    quint64 folderStatusMask(const QString& name) const;

    bool registerMessageStatusFlag(const QString& name);
    quint64 messageStatusMask(const QString& name) const;

    void flushIpcNotifications();

    static QMailStore* instance();
#ifdef QMAILSTOREINSTANCE_DEFINED_HERE
    friend QMailStore* QMailStoreInstance();
#endif

signals:
    void accountsAdded(const QMailAccountIdList& ids);
    void accountsRemoved(const QMailAccountIdList& ids);
    void accountsUpdated(const QMailAccountIdList& ids);
    void accountContentsModified(const QMailAccountIdList& ids);
    void messagesAdded(const QMailMessageIdList& ids);
    void messagesRemoved(const QMailMessageIdList& ids);
    void messagesUpdated(const QMailMessageIdList& ids);
    void foldersAdded(const QMailFolderIdList& ids);
    void foldersRemoved(const QMailFolderIdList& ids);
    void foldersUpdated(const QMailFolderIdList& ids);
    void folderContentsModified(const QMailFolderIdList& ids);

    void messageRemovalRecordsAdded(const QMailAccountIdList& ids);
    void messageRemovalRecordsRemoved(const QMailAccountIdList& ids);

private:
    enum AttemptResult { Success = 0, Failure, DatabaseFailure };
    
    QMailStore();

    bool addMessage(QMailMessageMetaData* m, const QString& mailfile);
    bool updateMessage(QMailMessageMetaData* m, QMailMessage* mail);

    QMailAccountIdList messageAccountIds(const QMailMessageKey&, bool inTransaction = false, AttemptResult* = 0) const;
    QMailFolderIdList messageFolderIds(const QMailMessageKey&, bool inTransaction = false, AttemptResult* = 0) const;
    QMailAccountIdList folderAccountIds(const QMailFolderKey&, bool inTransaction = false, AttemptResult* = 0) const;
    QMailFolderIdList folderAncestorIds(const QMailFolderIdList&, bool inTransaction = false, AttemptResult* = 0) const;

    AttemptResult affectedByMessageIds(const QMailMessageIdList &messages, QMailFolderIdList *folderIds, QMailAccountIdList *accountIds) const;
    AttemptResult affectedByFolderIds(const QMailFolderIdList &folders, QMailFolderIdList *folderIds, QMailAccountIdList *accountIds) const;

    void removeExpiredData(const QMailMessageIdList& messageIds,
                           const QStringList& mailfiles,
                           const QMailFolderIdList& folderIds = QMailFolderIdList(),
                           const QMailAccountIdList& accountIds = QMailAccountIdList());

    template<typename AccessType, typename FunctionType>
    bool repeatedly(FunctionType func, const QString &description) const;

    AttemptResult attemptAddAccount(QMailAccount *account, AccountConfiguration *config, MailStoreTransaction& t);
    AttemptResult attemptAddFolder(QMailFolder *folder, MailStoreTransaction& t);
    AttemptResult attemptAddMessage(QMailMessageMetaData *metaData, const QString &mailfile, MailStoreTransaction& t);

    AttemptResult attemptRemoveAccounts(const QMailAccountKey &key, MailStoreTransaction& t);
    AttemptResult attemptRemoveFolders(const QMailFolderKey &key, MessageRemovalOption option, MailStoreTransaction& t);
    AttemptResult attemptRemoveMessages(const QMailMessageKey &key, MessageRemovalOption option, MailStoreTransaction& t);

    AttemptResult attemptUpdateAccount(QMailAccount *account, AccountConfiguration *config, MailStoreTransaction& t);
    AttemptResult attemptUpdateFolder(QMailFolder *folder, MailStoreTransaction& t);
    AttemptResult attemptUpdateMessage(QMailMessageMetaData *metaData, QMailMessage *mail, MailStoreTransaction& t);
    AttemptResult attemptUpdateMessagesMetaData(const QMailMessageKey &key,
                                                const QMailMessageKey::Properties &properties,
                                                const QMailMessageMetaData &data, 
                                                MailStoreTransaction& t);
    AttemptResult attemptUpdateMessagesStatus(const QMailMessageKey &key, quint64 messageStatus, bool set, MailStoreTransaction& t);

    AttemptResult attemptCountAccounts(const QMailAccountKey &key, int *result, MailStoreReadLock&) const;
    AttemptResult attemptCountFolders(const QMailFolderKey &key, int *result, MailStoreReadLock&) const;
    AttemptResult attemptCountMessages(const QMailMessageKey &key, int *result, MailStoreReadLock&) const;

    AttemptResult attemptSizeOfMessages(const QMailMessageKey &key, int *result, MailStoreReadLock&) const;

    AttemptResult attemptQueryAccounts(const QMailAccountKey &key, const QMailAccountSortKey &sortKey, QMailAccountIdList *ids, MailStoreReadLock&) const;
    AttemptResult attemptQueryFolders(const QMailFolderKey &key, const QMailFolderSortKey &sortKey, QMailFolderIdList *ids, MailStoreReadLock&) const;
    AttemptResult attemptQueryMessages(const QMailMessageKey &key, const QMailMessageSortKey &sortKey, QMailMessageIdList *ids, MailStoreReadLock&) const;

    AttemptResult attemptAccount(const QMailAccountId &id, QMailAccount *result, MailStoreReadLock&) const;
    AttemptResult attemptFolder(const QMailFolderId &id, QMailFolder *result, MailStoreReadLock&) const;
    AttemptResult attemptMessage(const QMailMessageId &id, QMailMessage *result, MailStoreReadLock&) const;
    AttemptResult attemptMessage(const QString &uid, const QMailAccountId &accountId, QMailMessage *result, MailStoreReadLock&) const;

    AttemptResult attemptMessagesMetaData(const QMailMessageKey &key,
                                          const QMailMessageKey::Properties &properties,
                                          ReturnOption option,
                                          QMailMessageMetaDataList *result, 
                                          MailStoreReadLock&) const;

    AttemptResult attemptMessageRemovalRecords(const QMailAccountId& parentAccountId,
                                               const QString& fromMailbox,
                                               QMailMessageRemovalRecordList *result, 
                                               MailStoreReadLock&) const;

    AttemptResult attemptPurgeMessageRemovalRecords(const QMailAccountId &accountId, const QStringList &serverUids, MailStoreTransaction& t);

    AttemptResult attemptRestoreToPreviousFolder(const QMailMessageKey& key, MailStoreTransaction& t);

    AttemptResult attemptMessageAccountIds(const QMailMessageKey &key, QMailAccountIdList *ids, MailStoreReadLock&) const;
    AttemptResult attemptMessageFolderIds(const QMailMessageKey &key, QMailFolderIdList *ids, MailStoreReadLock&) const;
    AttemptResult attemptFolderAccountIds(const QMailFolderKey &key, QMailAccountIdList *ids, MailStoreReadLock&) const;
    AttemptResult attemptFolderAncestorIds(const QMailFolderIdList &ids, QMailFolderIdList *ancestorIds, MailStoreReadLock&) const;

    AttemptResult attemptStatusBit(const QString &name, const QString &context, int *result, MailStoreReadLock&) const;
    AttemptResult attemptRegisterStatusBit(const QString &name, const QString &context, int maximum, MailStoreTransaction& t);

    void preloadHeaderCache(const QMailMessageId& id) const;

    quint64 queryStatusMap(const QString &name, const QString &context, QMap<QString, quint64> &map) const;

private:
    friend class QMailStorePrivate;

    QMailStorePrivate* d;
};

#endif
