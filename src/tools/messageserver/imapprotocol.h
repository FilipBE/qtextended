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

#ifndef IMAPPROTOCOL_H
#define IMAPPROTOCOL_H

#include "client.h"

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <private/accountconfiguration_p.h>

enum ImapCommand
{
    IMAP_Init = 0,
    IMAP_Capability,
    IMAP_Idle_Continuation,
    IMAP_StartTLS,
    IMAP_Login,
    IMAP_Logout,
    IMAP_List,
    IMAP_Select,
    IMAP_UIDSearch,
    IMAP_UIDFetch,
    IMAP_UIDStore,
    IMAP_Expunge,
    IMAP_Full,
    IMAP_Idle,
    IMAP_Idle_Done
};

enum MessageFlag
{
    MFlag_All       = 0x0000, // Not a true flag
    MFlag_Seen      = 0x0001,
    MFlag_Answered  = 0x0002,
    MFlag_Flagged   = 0x0004,
    MFlag_Deleted   = 0x0008,
    MFlag_Draft     = 0x0010,
    MFlag_Recent    = 0x0020,
    MFlag_Unseen    = 0x0040
};

typedef uint MessageFlags;

enum FetchDataItem
{
    F_Rfc822_Size   =   0x0001,
    F_Rfc822_Header =   0x0002,
    F_Rfc822        =   0x0004,
    F_Uid           =   0x0008,
    F_Flags         =   0x0010
};

typedef uint FetchItemFlags;

enum OperationState
{
    OpDone = 0,
    OpFailed,
    OpOk,
    OpNo,
    OpBad
};

class LongStream;
class Email;
class ImapTransport;

class ImapProtocol: public QObject
{
    Q_OBJECT

public:
    ImapProtocol();
    ~ImapProtocol();

    bool open(const AccountConfiguration& config);
    void close();
    bool connected() const;
    bool inUse() const;

    void capability();
    void startTLS();

    /*  Valid in non-authenticated state only    */
    void login(QString user, QString password);

    /* Valid in authenticated state only    */
    void list(QString reference, QString mailbox);
    void select(QString mailbox);

    /*  Valid in Selected state only */
    void uidSearch(MessageFlags flags, const QString &range = QString());
    void uidFetch(FetchItemFlags items, const QString &range);
    void uidStore(MessageFlags flags, const QString &range);
    void expunge();
    void idle();
    void idleDone();

    /*  Internal commands (stored from selected mailbox)    */
    QString selected();
    int exists();
    int recent();
    QString mailboxUid();
    QString flags();
    QStringList mailboxUidList();

    /*  Valid in all states */
    void logout();

    QString lastError() { return _lastError; };

    /* Query whether a capability is supported */
    bool supportsCapability(const QString& name) const;

    static QString token(QString str, QChar c1, QChar c2, int *index);

    static QString sequence(uint identifier);
    static QString uid(const QString &identifier);
    static QString sequenceRange(uint from, uint to);
    static QString uidRange(const QString &from, const QString &to);

signals:
    void mailboxListed(QString &flags, QString &delimiter, QString &name);
    void messageFetched(QMailMessage& mail);
    void downloadSize(int);
    void nonexistentMessage(const QString& uid, Client::DefunctReason);

    void finished(ImapCommand &, OperationState &);
    void updateStatus(const QString &);
    void connectionError(int status, QString msg);

protected slots:
    void connected(AccountConfiguration::EncryptType encryptType);
    void errorHandling(int status, QString msg);
    void incomingData();

private:
    void operationCompleted(ImapCommand &, OperationState &);
    void nextAction();

    QString newCommandId();
    QString commandId(QString in);
    OperationState commandResponse(QString in);
    void sendCommand(QString cmd);

    void parseCapability();
    void parseSelect();
    bool parseFetch();
    bool parseFetchAll();
    void parseUid();
    void parseChange();
    void parseList(QString in);

    void createMail( QString& uid, int size, uint flags);

private:
    ImapTransport *transport;

    ImapCommand status;
    OperationState operationState;
    MessageFlags messageFlags;
    FetchItemFlags dataItems;

    /*  Associated with the Mailbox */
    QString _name;
    int _exists, _recent;
    QString _flags, _mailboxUid;
    QStringList uidList;

    QString request;
    QStringList errorList;
    LongStream *d;
    int requestCount, internalId;
    int messageLength;

    QString _lastError;
    QString response;
    int read;
    QTimer incomingDataTimer;
    bool firstParseFetch;
    QString fetchUid;
    QStringList _capabilities;

    bool newMessage;
    QString newMsgUid;
    uint newMsgSize;
    MessageFlags newMsgFlags;
    bool newMsgFlagsParsed;
    bool fetchAllFailed;
    QString fetchAllFailMsg;
    bool nonExistentMsg;
    QString nonExistentMsgStr;

    static const int MAX_LINES = 30;
};

#endif
