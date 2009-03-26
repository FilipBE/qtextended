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

#include "imapprotocol.h"

#include <QApplication>
#include <QTemporaryFile>

#include "mailtransport.h"
#include <private/longstream_p.h>
#include <private/longstring_p.h>
#include <QMailMessage>

#ifndef QT_NO_OPENSSL
#include <QSslError>
#endif


class ImapTransport : public MailTransport
{
public:
    ImapTransport(const char* name) :
        MailTransport(name)
    {
    }

#ifndef QT_NO_OPENSSL
protected:
    virtual bool ignoreCertificateErrors(const QList<QSslError>& errors)
    {
        MailTransport::ignoreCertificateErrors(errors);

        // Because we can't ask the user (due to string freeze), let's default
        // to ignoring these errors...
        foreach (const QSslError& error, errors)
            if (error.error() == QSslError::NoSslSupport)
                return false;

        return true;
    }
#endif
};


// Ensure a string is quoted, if required for IMAP transmission
static QString quoteImapString(const QString& input)
{
    // We can't easily catch controls other than those caught by \\s...
    static const QRegExp atomSpecials("[\\(\\)\\{\\s\\*%\\\\\"\\]]");

    // The empty string must be quoted
    if (input.isEmpty())
        return QString("\"\"");

    if (atomSpecials.indexIn(input) == -1)
        return input;
        
    // We need to quote this string because it is not an atom
    QString result(input);

    QString::iterator begin = result.begin(), it = begin;
    while (it != result.end()) {
        // We need to escape any characters specially treated in quotes
        if ((*it) == '\\' || (*it) == '"') {
            int pos = (it - begin);
            result.insert(pos, '\\');
            it = result.begin() + (pos + 1);
        }
        ++it;
    }

    return QMail::quoteString(result);
}


// Pack both the source mailbox path and the numeric UID into the UID value
// that we store for IMAP messages.  This will allow us find the owner 
// mailbox, even if the UID is present in multiple nested mailboxes.

static QString messageId(const QString& uid)
{
    int index = 0;
    if ((index = uid.lastIndexOf('|')) != -1)
        return uid.mid(index + 1);
    else
        return uid;
}

static QString messageUid(const QString& mailbox, const QString& id)
{
    return mailbox + '|' + id;
}

static QString extractUid(const QString& field, const QString& mailbox)
{
    QRegExp uidFormat("UID *(\\d+)");
    if (uidFormat.indexIn(field) != -1) {
        return messageUid(mailbox, uidFormat.cap(1));
    }

    return QString();
}

static uint extractSize(const QString& field)
{
    QRegExp sizeFormat("RFC822\\.SIZE *(\\d+)");
    if (sizeFormat.indexIn(field) != -1) {
        return sizeFormat.cap(1).toUInt();
    }

    return 0;
}

static bool parseFlags(const QString& field, MessageFlags& flags)
{
    QRegExp pattern("FLAGS *\\((.*)\\)");
    pattern.setMinimal(true);

    if (pattern.indexIn(field) == -1)
        return false;

    QString messageFlags = pattern.cap(1);

    flags = 0;
    if (messageFlags.indexOf("\\Seen") != -1)
        flags |= MFlag_Seen;
    if (messageFlags.indexOf("\\Answered") != -1)
        flags |= MFlag_Answered;
    if (messageFlags.indexOf("\\Flagged") != -1)
        flags |= MFlag_Flagged;
    if (messageFlags.indexOf("\\Deleted") != -1)
        flags |= MFlag_Deleted;
    if (messageFlags.indexOf("\\Draft") != -1)
        flags |= MFlag_Draft;
    if (messageFlags.indexOf("\\Recent") != -1)
        flags |= MFlag_Recent;

    return true;
}

ImapProtocol::ImapProtocol()
{
    read = 0;
    firstParseFetch = true;
    transport = 0;
    d = new LongStream();
    newMessage = true;
    fetchAllFailed = false;
    nonExistentMsg = false;
    connect(&incomingDataTimer, SIGNAL(timeout()),
            this, SLOT(incomingData()));
}

ImapProtocol::~ImapProtocol()
{
    delete d;
    delete transport;
}

bool ImapProtocol::open( const AccountConfiguration& config )
{
    if ( transport && transport->inUse() ) {
        QString msg("Cannot open account; transport in use");
        qLog(IMAP) << msg;
        emit connectionError(QMailMessageServer::ErrConnectionInUse, msg);
        return false;
    }

    status = IMAP_Init;

    errorList.clear();

    requestCount = 0;
    request.clear();
    d->reset();
    newMessage = true;

    if (!transport) {
        transport = new ImapTransport("IMAP");

        connect(transport, SIGNAL(updateStatus(QString)),
                this, SIGNAL(updateStatus(QString)));
        connect(transport, SIGNAL(errorOccurred(int,QString)),
                this, SLOT(errorHandling(int,QString)));
        connect(transport, SIGNAL(connected(AccountConfiguration::EncryptType)),
                this, SLOT(connected(AccountConfiguration::EncryptType)));
        connect(transport, SIGNAL(readyRead()),
                this, SLOT(incomingData()));
    }

    transport->open( config );

    return true;
}

void ImapProtocol::close()
{
    _name = "";
    if (transport)
        transport->close();
    d->reset();
    newMessage = true;
}

bool ImapProtocol::connected() const
{
    return (transport && transport->connected());
}

bool ImapProtocol::inUse() const
{
    return (transport && transport->inUse());
}

int ImapProtocol::exists()
{
    return _exists;
}

int ImapProtocol::recent()
{
    return _recent;
}

QString ImapProtocol:: mailboxUid()
{
    return _mailboxUid;
}

QString ImapProtocol::flags()
{
    return _flags;
}

QStringList ImapProtocol::mailboxUidList()
{
    return uidList;
}

QString ImapProtocol::selected()
{
    return _name;
}

void ImapProtocol::capability()
{
    status = IMAP_Capability;
    sendCommand( "CAPABILITY" );
}

void ImapProtocol::startTLS()
{
    QString cmd = "STARTTLS\r\n";
    status = IMAP_StartTLS;
    sendCommand( cmd );
}

/*  Type ignored for now    */
void ImapProtocol::login( QString user, QString password )
{
    status = IMAP_Login;
    sendCommand( "LOGIN " + quoteImapString(user) + " " + quoteImapString(password) );
}

void ImapProtocol::logout()
{
    status = IMAP_Logout;
    sendCommand( "LOGOUT" );
}

void ImapProtocol::idle()
{
    status = IMAP_Idle;
    sendCommand( "IDLE" );
}

void ImapProtocol::idleDone()
{
    status = IMAP_Idle_Done;
    transport->stream() << "DONE\r\n" << flush;
    qLog(IMAP) << "SEND:" << "DONE";
}

void ImapProtocol::list( QString reference, QString mailbox )
{
    status = IMAP_List;
    sendCommand( "LIST " + quoteImapString(reference) + " " + quoteImapString(mailbox) );
}

void ImapProtocol::select( QString mailbox )
{
    status = IMAP_Select;
    _name = mailbox;
    sendCommand( "SELECT " + quoteImapString(mailbox) );
}

void ImapProtocol::uidSearch( MessageFlags flags, const QString &range)
{
    QString str;
    if ( flags != 0 ) {
        if ( flags & MFlag_Recent )
            str += " RECENT";
        if (flags & MFlag_Deleted)
            str += " DELETED";
        if (flags & MFlag_Answered)
            str += " ANSWERED";
        if (flags & MFlag_Flagged)
            str += " FLAGGED";
        if (flags & MFlag_Seen)
            str += " SEEN";
        if (flags & MFlag_Unseen)
            str += " UNSEEN";
        if (flags & MFlag_Draft)
            str += " DRAFT";
    } else if ( range.isEmpty() ) {
        str = "ALL";
    }

    str = str.trimmed();
    if (!range.isEmpty())
        str.prepend(' ');

    status = IMAP_UIDSearch;
    sendCommand( QString("UID SEARCH %1%2").arg( range ).arg( str ) );
}

void ImapProtocol::uidFetch( FetchItemFlags items, const QString &range)
{
    dataItems = items;

    QString flags = "(FLAGS";
    if (dataItems & F_Uid)
        flags += " UID";
    if (dataItems & F_Rfc822_Size)
        flags += " RFC822.SIZE";
    if (dataItems & F_Rfc822_Header)
        flags += " RFC822.HEADER";
    if (dataItems & F_Rfc822) {
        flags += " BODY.PEEK[]";

        int index = range.indexOf(':');
        if (index != -1) {
            fetchUid = range.left(index);
        } else {
            fetchUid = range;
        }
    }

    flags += ")";

    messageLength = 0;
    status = IMAP_UIDFetch;
    firstParseFetch = true;
    sendCommand( QString( "UID FETCH %1 %2" ).arg( range ).arg( flags ) );
}

/*  Note that \Recent flag is ignored as only the server is allowed
    to set/unset that flag      */
void ImapProtocol::uidStore( MessageFlags flags, const QString &range)
{
    QString str = "+FLAGS.SILENT (";
    if (flags & MFlag_Deleted)
        str += "\\Deleted "; // No tr
    if (flags & MFlag_Answered)
        str += "\\Answered "; // No tr
    if (flags & MFlag_Flagged)
        str += "\\Flagged "; // No tr
    if (flags & MFlag_Seen)
        str += "\\Seen "; // No tr
    if (flags & MFlag_Draft)
        str += "\\Draft "; // No tr
    str = str.trimmed() + ")";

    status = IMAP_UIDStore;
    sendCommand( QString( "UID STORE %1 %2" ).arg( range ).arg( str ) );
}

void ImapProtocol::expunge()
{
    status = IMAP_Expunge;
    sendCommand( "EXPUNGE" );
}

void ImapProtocol::connected(AccountConfiguration::EncryptType encryptType)
{
#ifndef QT_NO_OPENSSL
    if (encryptType == AccountConfiguration::Encrypt_TLS)
    {
        ImapCommand cmd = IMAP_StartTLS;
        OperationState state = OpOk;
        emit finished(cmd, state);
    }
#else
    Q_UNUSED(encryptType);
#endif
}

void ImapProtocol::errorHandling(int status, QString msg)
{
    transport->close();

    if (msg.isEmpty())
        msg = tr("Connection failed");

    if (this->status != IMAP_Logout)
        emit connectionError(status, msg);
}

void ImapProtocol::sendCommand( QString cmd )
{
    QString command = newCommandId() + " " + cmd;

    request = command;
    d->reset();
    newMessage = true;

    transport->stream() << command << "\r\n" << flush;
    qLog(IMAP) << "SEND:" << qPrintable(command);
}

void ImapProtocol::incomingData()
{
    int readLines = 0;
    while (transport->canReadLine()) {
        response = transport->readLine();
        readLines++;
        read += response.length();

        if (response.length() > 1)
            qLog(IMAP) << "RECV:" << qPrintable(response.left(response.length() - 2));

        if (status == IMAP_Idle || status == IMAP_Idle_Done) {
            emit finished(status, operationState);
            response = "";
            read = 0;
            return;
        }
        if (status != IMAP_Init) {
            if (status == IMAP_UIDFetch && newMessage) {
                QString str = response;
                QRegExp fetchResponsePattern("\\*\\s+\\d+\\s+(\\w+)");
                if (fetchResponsePattern.indexIn(str) == 0) {
                    if (fetchResponsePattern.cap(1).compare("FETCH", Qt::CaseInsensitive) == 0) {
                        newMessage = false;
                        newMsgUid = extractUid(str, _name);
                        newMsgSize = extractSize(str);
                        newMsgFlags = 0;
                        newMsgFlagsParsed = parseFlags(str, newMsgFlags);
                        d->reset();
                    }
                } else if (str.toUpper().startsWith("* NO")) {
                    newMessage = false;
                    fetchAllFailed = true;
                    fetchAllFailMsg = str;
                } else {
                    nonExistentMsg = true;
                    nonExistentMsgStr = messageUid(_name, fetchUid);
                }
            } else {
                d->append( response );
            }
            if (d->status() == LongStream::OutOfSpace) {
                operationState = OpFailed;
                _lastError += LongStream::errorMessage( "\n" );
                status = IMAP_Full;
                emit finished(status, operationState);
                response = "";
                read = 0;
                return;
            }
        }

        if ((status == IMAP_UIDFetch) && (dataItems & F_Rfc822)) {
            if (!response.startsWith("* "))
                messageLength += response.length();

            if (readLines > MAX_LINES)
                emit downloadSize(messageLength);
        }

        if (readLines > MAX_LINES) {
            incomingDataTimer.start(0);
            return;
        }
    }

    incomingDataTimer.stop();

    nextAction();
}

void ImapProtocol::operationCompleted(ImapCommand &status, OperationState &operationState)
{
    emit finished(status, operationState);
    response = "";
    read = 0;
}

void ImapProtocol::nextAction()
{
    if (status == IMAP_Init) {
        operationState = OpOk;
        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_Logout) {
        transport->close();
        d->reset();
        newMessage = true;
        _name = "";
        operationState = OpOk;
        operationCompleted(status, operationState);
        return;
    }

    if ((status == IMAP_UIDFetch) && (dataItems & F_Rfc822))
        emit downloadSize( messageLength );

    /* Applies to all functions below   */
    if (!response.startsWith( commandId( request ))) {
        response = "";
        read = 0;

        /* TODO - what's going on here?
        QString msg("Protocol error - unhandled server response.");
        qLog(IMAP) << msg;
        emit connectionError(status, msg);
        */
        return;
    }

    if ((operationState = commandResponse( response )) != OpOk) {
        // The client decides whether the error is critical or not
        // tr string from server - this seems ambitious
        _lastError = tr( response.toAscii() );
        if (status == IMAP_UIDSearch)
            _lastError += QLatin1String("\n") + tr( "This server does not provide a complete "
                              "IMAP4rev1 implementation." );
        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_Capability) {
        parseCapability();

        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_StartTLS) {
#ifndef QT_NO_OPENSSL
        // Switch to encrypted comms mode
        transport->switchToEncrypted();
#endif
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_Login) {
        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_List) {
        for (QString str = d->first(); str != QString::null; str = d->next()) {
            if (str.startsWith("* LIST"))
                parseList( str.mid(7) );
        }

        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_Select) {
        parseSelect();

        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_UIDSearch) {
        parseUid();

        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_UIDFetch) {
        //temporary for now
        bool parsedInput = false;
        if (dataItems & F_Rfc822_Header) {
            parsedInput = parseFetch();
        } else {
            parsedInput = parseFetchAll();
        }
        
        if (parsedInput)
            operationCompleted(status, operationState);

        fetchUid = QString();
        return;
    }

    if (status == IMAP_UIDStore) {
        operationCompleted(status, operationState);
        return;
    }

    if (status == IMAP_Expunge) {
        operationCompleted(status, operationState);
        return;
    }

    response = "";
    read = 0;
}

QString ImapProtocol::newCommandId()
{
    QString id, out;

    requestCount++;
    id.setNum( requestCount );
    out = "a";
    out = out.leftJustified( 4 - id.length(), '0' );
    out += id;
    return out;
}

QString ImapProtocol::commandId( QString in )
{
    int pos = in.indexOf(' ');
    if (pos == -1)
        return "";

    return in.left( pos ).trimmed();
}

OperationState ImapProtocol::commandResponse( QString in )
{
    QString old = in;
    int start = in.indexOf( ' ' );
    start = in.indexOf( ' ', start );
    int stop = in.indexOf( ' ', start + 1 );
    if (start == -1 || stop == -1) {
        qLog(IMAP) << qPrintable("could not parse command response: " + in);
        return OpFailed;
    }

    in = in.mid( start, stop - start ).trimmed().toUpper();
    OperationState state = OpFailed;

    if (in == "OK")
        state = OpOk;
    if (in == "NO")
        state = OpNo;
    if (in == "BAD")
        state = OpBad;

    return state;
}

void ImapProtocol::parseList( QString in )
{
    QString flags, name, delimiter;
    int pos, index = 0;

    flags = token( in, '(', ')', &index );

    delimiter = token( in, ' ', ' ', &index );
    pos = 0;
    if (token(delimiter, '"', '"', &pos) != QString::null) {
        pos = 0;
        delimiter = token( delimiter, '"', '"', &pos );
    }

    index--;    //to point back to previous => () NIL "INBOX"
    name = token( in, ' ', '\n', &index ).trimmed();
    pos = 0;
    if (token( name, '"', '"', &pos ) != QString::null) {
        pos = 0;
        name = token( name, '"', '"', &pos );
    }

    emit mailboxListed( flags, delimiter, name );
}

void ImapProtocol::parseCapability()
{
    for (QString str = d->first(); str != QString::null; str = d->next()) {
        if (str.startsWith("* CAPABILITY")) {
            _capabilities = str.mid(12).trimmed().split(" ", QString::SkipEmptyParts);
        }
    }
}

void ImapProtocol::parseSelect()
{
    int start;
    bool result;
    QString str, temp;

    // reset all attributes in case some are not reported
    _exists = 0;
    _recent = 0;
    _flags = "";
    _mailboxUid = "";
    for (str = d->first(); str != QString::null; str = d->next()) {

        if (str.indexOf("EXISTS", 0) != -1) {
            start = 0;
            temp = token(str, ' ', ' ', &start);
            _exists =  temp.toInt(&result);
            if (!result)
                _exists = 0;
        } else if (str.indexOf("RECENT", 0) != -1) {
            start = 0;
            temp = token( str, ' ', ' ', &start );
            _recent = temp.toInt( &result );
            if (!result)
                _recent = 0;
        } else if (str.startsWith("* FLAGS")) {
            start = 0;
            _flags = token( str, '(', ')', &start );
        } else if (str.indexOf("UIDVALIDITY", 0) != -1) {
            start = 0;
            temp = token( str, '[', ']', &start );
            _mailboxUid = messageUid(_name, temp.mid( 12 ));
        }
    }
}

bool ImapProtocol::parseFetch()
{
    static const QByteArray crlfSequence( QMailMessage::CRLF );
    static const QByteArray headerTerminator( crlfSequence + crlfSequence );
    static const QByteArray popTerminator( crlfSequence + '.' + crlfSequence );

    if (fetchAllFailed) {
        qLog(IMAP) << qPrintable("fetch all failed: " + fetchAllFailMsg);
        fetchAllFailed = false;
        nonExistentMsg = false;
        return false;
    }
    
    QString str;
    if (firstParseFetch) {
        firstParseFetch = false;
        str = d->first();
    } else {
        str = d->current();
    }

    bool moreData(false);
    do {
        moreData = false;

        if (str == QString::null) {
            // We have exhausted the input
            return true;
        } else {
            MessageFlags flags = newMsgFlags;
            bool flagsParsed = newMsgFlagsParsed;
            QString msg;

            while (!str.startsWith("* ") && str != QString::null) {
                msg += str;
                str = d->next();
            }

            int endMsg = msg.indexOf( headerTerminator );
            if (endMsg != -1) {
                if (!flagsParsed) {
                    // See if the flags are trailing the header
                    QString trailer = msg.mid(endMsg + 2);
                    flagsParsed = parseFlags(trailer, flags);
                }

                msg.truncate(endMsg);
            }
            msg.truncate(newMsgSize);

            //to work with pop standard
            endMsg = msg.indexOf( popTerminator, -popTerminator.length() );
            if (endMsg == -1)
                msg.append( popTerminator );

            createMail( newMsgUid, newMsgSize, flags );
            return true;
        }
    } while (moreData);

    return false;
}

bool ImapProtocol::parseFetchAll()
{
    QString str;
    
    if (fetchAllFailed) {
        qLog(IMAP) << qPrintable("fetch all failed: " + fetchAllFailMsg);
        fetchAllFailed = false;
        nonExistentMsg = false;
        return false;
    }
    if (nonExistentMsg) {
        emit nonexistentMessage(nonExistentMsgStr, Client::Removed);
        fetchAllFailed = false;
        nonExistentMsg = false;
        return true;
    }
    
    if (firstParseFetch) {
        firstParseFetch = false;
        str = d->first();
    } else {
        str = d->current();
    }

    bool moreData(false);
    do {
        moreData = false;

        if (str == QString::null) {
            // We have exhausted the input
            return true;
        } else {
            uint msgSize = newMsgSize;
            MessageFlags flags = newMsgFlags;
            bool flagsParsed = newMsgFlagsParsed;
            QString msg;

            // Read the body data from the file
            LongString ls(d->fileName());
                    
            const QByteArray& orgData(ls.toQByteArray());
            uint remainder(orgData.length());
            if (msgSize)
                remainder = qMin(msgSize, (uint)orgData.length());

            QByteArray trailer;
            if (msgSize == 0) {
                // Find the trailing part of the imap server traffic
                trailer = QByteArray( (QByteArray( QMailMessage::CRLF ) + ')' + QMailMessage::CRLF) );
                int pos = orgData.lastIndexOf(trailer);
                if ((pos != -1) && (pos < (int)remainder)) {
                    // Leave the terminating CRLF
                    trailer = orgData.mid(pos + 2);
                    QFile::resize(d->fileName(), pos + 2);
                    msgSize = pos + 2;
                } else {
                    msgSize = orgData.length();
                }
            } else {
                trailer = QByteArray(orgData.constData() + remainder, orgData.length() - remainder);
                QFile::resize(d->fileName(), msgSize);
            }

            if (!flagsParsed) {
                // See if the flags follow the message data
                flagsParsed = parseFlags(QString(trailer), flags);
            }

            createMail( newMsgUid, msgSize, flags );
            return true;
        }
    } while (moreData);

    return false;
}

void ImapProtocol::parseUid()
{
    int index;
    QString str, temp;

    uidList.clear();
    for (str = d->first(); str != QString::null; str = d->next()) {
        if (str.startsWith("* SEARCH")) {
            index = 7;
            while ((temp = token( str, ' ', ' ', &index )) != QString::null) {
                uidList.append( messageUid(_name, temp) );
                index--;
            }
            temp = token( str, ' ', '\n', &index );
            if (temp != QString::null)
                uidList.append( messageUid(_name, temp) );
        }
    }
}

QString ImapProtocol::token( QString str, QChar c1, QChar c2, int *index )
{
    int start, stop;

    // The strings we're tokenizing use CRLF as the line delimiters - assume that the
    // caller considers the sequence to be atomic.
    if (c1 == QMailMessage::LineFeed)
        c1 = QMailMessage::CarriageReturn;
    start = str.indexOf( c1, *index, Qt::CaseInsensitive );
    if (start == -1)
        return QString::null;

    // Bypass the LF if necessary
    if (c1 == QMailMessage::CarriageReturn)
        start += 1;

    if (c2 == QMailMessage::LineFeed)
        c2 = QMailMessage::CarriageReturn;
    stop = str.indexOf( c2, ++start, Qt::CaseInsensitive );
    if (stop == -1)
        return QString::null;

    // Bypass the LF if necessary
    *index = stop + (c2 == QMailMessage::CarriageReturn ? 2 : 1);

    return str.mid( start, stop - start );
}

QString ImapProtocol::sequence( uint identifier )
{
    return QString("%1").arg(identifier);
}

QString ImapProtocol::sequenceRange( uint from, uint to )
{
    return QString("%1:%2").arg(from).arg(to);
}

QString ImapProtocol::uid( const QString &identifier )
{
    return messageId(identifier);
}

QString ImapProtocol::uidRange( const QString &from, const QString &to )
{
    return messageId(from) + ':' + messageId(to);
}

bool ImapProtocol::supportsCapability(const QString& name) const
{
    return _capabilities.contains(name);
}

void ImapProtocol::createMail( QString& id, int size, uint flags )
{
    QString detachedFile = d->detach();
    QMailMessage mail = QMailMessage::fromRfc2822File( detachedFile );

    if (flags & MFlag_Seen)
        mail.setStatus( QMailMessage::ReadElsewhere, true );
    if (flags & MFlag_Answered)
        mail.setStatus( QMailMessage::Replied, true );

    mail.setSize( size );
    mail.setServerUid( id.trimmed() );
    mail.setMessageType( QMailMessage::Email );
    mail.setHeaderField( "X-qtopia-internal-filename", detachedFile );

    emit messageFetched(mail);
    // Catch all cleanup of detached file
    QFile::remove(detachedFile);
}

