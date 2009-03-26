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

#ifndef QMAILACCOUNT_H
#define QMAILACCOUNT_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qtopiaglobal.h>
#include <QSharedData>

#include <QMailMessage>
#include <QMailMessageKey>

class QMailAccountPrivate;
class QMailAccountId;
class QMailFolderId;
class QSettings;
class QTimer;

class QTOPIAMAIL_EXPORT QMailAccount
{
public:
    QMailAccount();
    explicit QMailAccount(const QMailAccountId& id);
    QMailAccount(const QMailAccount& other);
    QMailAccount& operator=(const QMailAccount& other);
    ~QMailAccount();

    void setId(const QMailAccountId& id);
    QMailAccountId id() const;

    QString displayName() const;

    QString accountName() const;
    void setAccountName(const QString &str);

    const QMailFolderIdList mailboxes() const;
    QMailFolderId getMailbox(const QString &name);

    bool canCollectMail() const;
    bool canSendMail() const;

    const QStringList serverUids() const;

    const QStringList serverUids(const QMailFolderId &boxId) const;
    const QStringList serverUids(const QMailFolderId &boxId, quint64 messageStatusFilter, bool set = true) const;

    const QStringList deletedMessages() const;
    const QStringList deletedMessages(const QMailFolderId &boxId) const;

    void saveSettings(QSettings *conf) const;
    void readSettings(QSettings *conf);
    bool hasSettings() const;

    /* Fragments of forthcoming plugin-supporting interface: */
    QMailMessage::MessageType messageType() const;

    QStringList messageSources() const;
    bool isMessageSource() const;

    QStringList messageSinks() const;
    bool isMessageSink() const;

    /* Temporary - only available until plugin sources are supported */
    static QMailAccount accountFromSource(const QString &sourceType);
    static QList<int> matchingAccountTypes(QMailMessage::MessageType type);

private:
    friend class QMailAccountPrivate;
    friend class AccountConfiguration;
    friend class QMailStore;
    friend class QMailStorePrivate;

    enum AccountType {
        POP = 0,
        IMAP = 1,
        SMS = 3,
        MMS = 4,
        System = 5,
        Collective = 6      // Multiple IM types, as supported by Telepathy
    };

    AccountType accountType() const;
    void setAccountType(AccountType at);

    QMailMessageKey messagesKey() const;
    QMailMessageKey trashKey() const;
    QMailMessageKey sentKey() const;
    QMailMessageKey draftsKey() const;

    QMailMessageKey messagesKey(const QMailFolderId &boxId) const;
    QMailMessageKey trashKey(const QMailFolderId &boxId) const;
    QMailMessageKey sentKey(const QMailFolderId& boxId) const;
    QMailMessageKey draftsKey(const QMailFolderId& boxId) const;

    QStringList serverUids(QMailMessageKey key) const;

    QSharedDataPointer<QMailAccountPrivate> d;
};

#endif
