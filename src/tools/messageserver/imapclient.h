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

#ifndef IMAPCLIENT_H
#define IMAPCLIENT_H

#include "imapprotocol.h"
#include "client.h"

#include <private/accountconfiguration_p.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qlist.h>
#include <qtimer.h>

#include <QMailFolder>


class QMailAccount;

class ImapClient: public Client
{
    Q_OBJECT

public:
    enum FolderStatus
    {
        NoInferiors = 0x01,
        NoSelect = 0x02,
        Marked = 0x04,
        Unmarked = 0x08
    };

    ImapClient(QObject* parent);
    ~ImapClient();

    virtual void setAccount(const QMailAccountId& accountId);
    virtual QMailAccountId account() const;

    virtual void newConnection();
    virtual void closeConnection();
    virtual void foldersOnly(bool folders);
    virtual void setSelectedMails(const SelectionMap& data);
    virtual void checkForNewMessages();
    virtual void cancelTransfer();

signals:
    void retrievalProgress(const QString&, uint);
    void messageProcessed(const QString&);
    void fetchTotal(uint);
    void fetchProgress(uint);

public slots:
    void errorHandling(int, QString msg);
    void idleErrorHandling(int, QString msg);
    void idleErrorRecovery();

protected slots:
    void operationDone(ImapCommand &, OperationState &);
    void mailboxListed(QString &, QString &, QString &);
    void messageFetched(QMailMessage& mail);
    void downloadSize(int);
    void transportStatus(const QString& status);

    void idleOperationDone(ImapCommand &, OperationState &);
    void idleTimeOut();

private:
    enum TransferStatus
    {
        Init, List, Fetch, Retrieve
    };

    enum SearchStatus
    {
        All, Seen, Unseen, Inconclusive
    };

    void removeDeletedMailboxes();
    bool selectNextMailbox();
    bool nextMailbox();
    void handleSelect();
    void handleUid();
    void handleUidFetch();
    void handleSearch();

    bool setNextSeen();
    bool setNextDeleted();
    void fetchNextMail();
    void searchCompleted();
    void previewCompleted();
    void retrieveOperationCompleted();

private:
    ImapProtocol client;
    ImapProtocol idleConnection;

    QMailAccountId accountId;
    AccountConfiguration _config;
    SelectionMap selectionMap;
    SelectionMap::ConstIterator folderItr;
    MessageMap::ConstIterator selectionItr;
    int listSize;
    QString msgUidl;

    SearchStatus _searchStatus;
    QStringList _unseenUids;
    QStringList _seenUids;
    QStringList _newUids;
    QStringList _readUids;
    QStringList _removedUids;
    bool _expungeRequired;

    QList<QPair<QMailFolderId, QStringList> > _retrieveUids;

    TransferStatus status;
    int messageCount, mailSize, mailDropSize;
    bool selected;

    QStringList mailboxNames;
    QStringList unresolvedUid;
    QMailFolder currentMailbox;
    QMailFolderIdList mailboxList;

    QString retrieveUid;
    bool tlsEnabled;

    QMap<QMailFolderId, FolderStatus> folderStatus;

    bool supportsIdle;
    bool idling;
    QTimer idleTimer;
    int retryDelay;
    bool waitingForIdle;
    bool folders;
    uint fetchedCount;
};

#endif
