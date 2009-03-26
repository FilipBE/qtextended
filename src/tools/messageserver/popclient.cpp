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

#include "popclient.h"
#include <private/longstream_p.h>
#include <private/longstring_p.h>
#include "mailtransport.h"

#include <QMailStore>

#include <qtopialog.h>


PopClient::PopClient(QObject* parent)
    : Client(parent)
{
    headerLimit = 0;
    preview = false;
    transport = 0;
    d = new LongStream();
}

PopClient::~PopClient()
{
    delete d;
    delete transport;
}

void PopClient::newConnection()
{
    if (transport && transport->connected())
        return;

    // Load the configuration for this account
    _config = AccountConfiguration(accountId);

    if ( _config.mailServer().isEmpty() ) {
        status = Exit;
        accountId = QMailAccountId();
        retrieveOperationCompleted();
        return;
    }

    status = Init;
    serverUidNumber.clear();
    serverUid.clear();
    serverSize.clear();
    uniqueUidlList.clear();
    selected = false;
    awaitingData = false;
    messageCount = 0;
    deleteList.clear();

    if (!transport) {
        // Set up the transport
        transport = new MailTransport("POP");

        connect(transport, SIGNAL(updateStatus(QString)),
                this, SLOT(transportStatus(QString)));
        connect(transport, SIGNAL(errorOccurred(int,QString)),
                this, SLOT(errorHandling(int,QString)));
        connect(transport, SIGNAL(readyRead()),
                this, SLOT(incomingData()));
    }

    transport->open(_config);
}

void PopClient::setAccount(const QMailAccountId &id)
{
    if ((transport && transport->inUse()) && (id != accountId)) {
        QString msg("Cannot open account; transport in use");
        emit errorOccurred(QMailMessageServer::ErrConnectionInUse, msg);
        return;
    }

    accountId = id;
    lastUidl = QMailAccount(accountId).serverUids();
}

QMailAccountId PopClient::account() const
{
    return accountId;
}

void PopClient::headersOnly(bool headers, int limit)
{
    preview = headers;
    headerLimit = limit;
}

void PopClient::setSelectedMails(const SelectionMap& data)
{
    selected = true;

    selectionMap = *(data.begin());
    selectionItr = selectionMap.begin();
    listSize = selectionMap.count();

    messageCount = 0;

    if (transport && transport->connected() && (status == Done)) {
        status = Retr;
        incomingData();
    }
}

void PopClient::errorHandling(int status, QString msg)
{
    if (transport && transport->inUse()) {
        transport->close();
    }
    emit updateStatus(QMailMessageServer::None, tr("Error occurred"));
    emit errorOccurred(status, msg);
}

void PopClient::closeConnection()
{
    if (transport->connected()) {
        if ( status == Exit ) {
            // We have already sent our quit command
            accountId = QMailAccountId();
            transport->close();
        } else {
            // Send a quit command
            status = Quit;
            incomingData();
        }
    } else if (transport->inUse()) {
        transport->close();
    }
}

void PopClient::sendCommand(const QString& cmd)
{
    transport->stream() << cmd << "\r\n" << flush;
    
    if (cmd.length()) {
        qLog(POP) << "SEND:" << qPrintable(cmd);
    }
}

QString PopClient::readResponse() 
{
    QString response = transport->readLine();

    if (response.length() > 1) {
        qLog(POP) << "RECV:" << qPrintable(response.left(response.length() - 2));
    }

    return response;
}

void PopClient::incomingData()
{
    QString response, temp;

    if ( (status != Dele) && (status != Quit) )
        response = readResponse();

    if (status == Init) {
        emit updateStatus(QMailMessageServer::None, tr("Logging in"));
        transport->stream().setCodec("UTF-8");
        sendCommand("USER " + _config.mailUserName());
        status = Pass;
    } else if (status == Pass) {
        if (response[0] != '+') {
            errorHandling(QMailMessageServer::ErrLoginFailed, "");
            return;
        }
        sendCommand("PASS " + _config.mailPassword());
        status = Uidl;
    } else if (status == Uidl) {
        if (response[0] != '+') {
            errorHandling(QMailMessageServer::ErrLoginFailed, "");
            return;
        }

        status = Guidl;
        awaitingData = false;
        sendCommand("UIDL");
        return;
    } else if (status == Guidl) {           //get list of uidls

        //means first time in, response should be "+Ok"
        if (! awaitingData) {
            if ( response[0] != '+' ) {
                errorHandling(QMailMessageServer::ErrUnknownResponse, response);
                return;
            }

            awaitingData = true;
            if ( transport->canReadLine() ) {
                response = readResponse();
            } else {
                return;
            }
        }

        // Extract the UID values
        QRegExp pattern("(\\d+) +(.*)");
        do {
            if (response == ".\r\n") {
                // Finished
                status = List;
                break;
            } else {
                // Extract the number and UID
                QString text(response.mid(0, response.length() - 2));
                if (pattern.indexIn(text) != -1) {
                    int number(pattern.cap(1).toInt());
                    QString uid(pattern.cap(2));

                    serverUidNumber.insert(uid, number);
                    serverUid.insert(number, uid);
                }
            }
        } while (transport->canReadLine() && (response = readResponse(), true));

        if (status != List) {
            // More UID data to come
            return;
        }
    }

    if (status == List) {
        sendCommand("LIST");
        awaitingData = false;
        status = Size;
        return;
    }

    if (status == Size) {
        //means first time in, response should be "+Ok"
        if (!awaitingData) {
            if ( response[0] != '+' ) {
                errorHandling(QMailMessageServer::ErrUnknownResponse, response);
                return;
            }

            awaitingData = true;
            if ( transport->canReadLine() )
                response = readResponse();
            else return;
        }

        // Extract the message sizes
        QRegExp pattern("(\\d+) +(\\d+)");
        do {
            if (response == ".\r\n") {
                // Finished
                status = Retr;
                uidlIntegrityCheck();
                break;
            } else {
                // Extract the size
                QString text(response.mid(0, response.length() - 2));
                if (pattern.indexIn(text) != -1) {
                    serverSize.insert(pattern.cap(1).toInt(), pattern.cap(2).toInt());
                }
            }
        } while (transport->canReadLine() && (response = readResponse(), true));

        if (status != Retr) {
            // More size data to come
            return;
        }
    }

    if (status == Retr) {

        msgNum = nextMsgServerPos();
        if (msgNum != -1) {
            if (!selected) {
                if (messageCount == 1)
                    emit updateStatus(QMailMessageServer::Retrieve, tr("Previewing","Previewing <no of messages>") +QChar(' ') + QString::number(uniqueUidlList.count()));
            } else {
                emit updateStatus(QMailMessageServer::Retrieve, tr("Completing %1 / %2").arg(messageCount).arg(listSize));
            }

            temp.setNum(msgNum);
            if (!preview || mailSize <= headerLimit) {
                sendCommand("RETR " + temp);
            } else {                                //only header
                sendCommand("TOP " + temp + " 0");
            }

            status = Ignore;
            return;
        } else {
            status = Acks;
        }
    }

    if (status == Ignore) {
        if (response[0] == '+') {
            message = "";
            d->reset();
            status = Read;
            if (!transport->canReadLine())    //sync. problems
                return;
            response = readResponse();
        } else errorHandling(QMailMessageServer::ErrUnknownResponse, response);
    }

    if (status == Read) {
        message += response;
        d->append( response );
        if (d->status() == LongStream::OutOfSpace) {
            errorHandling(QMailMessageServer::ErrFileSystemFull, LongStream::errorMessage( "\n" ));
            return;
        }

        while ( transport->canReadLine()) {
            response = readResponse();
            message += response;
            message = message.right( 100 );
            d->append( response );
            if (d->status() == LongStream::OutOfSpace) {
                errorHandling(QMailMessageServer::ErrFileSystemFull, LongStream::errorMessage( "\n" ));
                return;
            }
        }

        message = message.right( 100 ); // 100 > 5 need at least 5 chars
        if (message.indexOf("\r\n.\r\n", -5) == -1) {
            // We have an incomplete message 
            if (!retrieveUid.isEmpty())
                emit retrievalProgress(retrieveUid, d->length());

            return;
        }

        createMail();

        // in case user jumps out, don't download message on next entry
        if (preview) {
            lastUidl.append(msgUidl);
        }

        // We have now retrieved the entire message
        if (!retrieveUid.isEmpty()) {
            emit messageProcessed(retrieveUid);
            retrieveUid = QString();
        }

//        if(!preview || (preview && mailSize <= headerLimit) && _config.canDeleteMail())
//            _config.markMessageForDeletion(msgUidl,0);

        status = Retr;
        incomingData();         //remember: recursive
        return;
    }

    if (status == Acks) {
        //ok, delete mail, await user input
        if ( _config.canDeleteMail() ) {
            status = Dele;
            awaitingData = false;
            deleteList = QMailAccount(accountId).deletedMessages();
            if ( deleteList.isEmpty() )
                status = Done;
        } else {
            status = Done;
        }
    }

    if (status == Dele) {
        if ( awaitingData ) {
            response = readResponse();
            if ( response[0] != '+' ) {
                errorHandling(QMailMessageServer::ErrUnknownResponse, response);
                return;
            }
        } else {
            qLog(POP) << qPrintable(QString::number( deleteList.count() ) + " messages in mailbox to be deleted");
            emit updateStatus(QMailMessageServer::None, tr("Removing old messages"));
        }

        if ( deleteList.count() == 0 ) {
            status = Done;
        } else {
            QString str = deleteList.first();
            int pos = msgPosFromUidl( str );
            deleteList.removeAll( str );
            QMailStore::instance()->purgeMessageRemovalRecords(accountId, QStringList() << str);

            if ( pos != -1) {
                awaitingData = true;
                qLog(POP) << "deleting message at pos:" << pos;
                sendCommand("DELE " + QString::number(pos));
            } else {
                qLog(POP) << "unable to delete unlisted UID:" << str;
                incomingData();
                return;
            }
        }
    }

    if (status == Done) {
        if ( preview ) {
            emit partialRetrievalCompleted();
            return;
        }

        status = Quit;
    }

    if (status == Quit) {
        emit updateStatus(QMailMessageServer::None, tr("Logging out"));
        status = Exit;
        sendCommand("QUIT");
        return;
    }

    if (status == Exit) {
        transport->close();        //close regardless

        accountId = QMailAccountId();
        retrieveOperationCompleted();
    }
}

int PopClient::msgPosFromUidl(QString uidl)
{
    QMap<QString, int>::const_iterator it = serverUidNumber.find(uidl);
    if (it != serverUidNumber.end())
        return it.value();

    return -1;
}

int PopClient::nextMsgServerPos()
{
    int thisMsg = -1;

    if (preview) {
        if ( messageCount < uniqueUidlList.count() ) {
            msgUidl = uniqueUidlList.at(messageCount);
            thisMsg = msgPosFromUidl(msgUidl);
            mailSize = getSize(thisMsg);
            messageCount++;
        }
    }

    if (selected) {
        QString serverId;
        if (selectionItr != selectionMap.end()) {
            serverId = selectionItr.key();
            selectionItr++;
            ++messageCount;
        }

        // if requested mail is not on server, try to get a new mail from the list
        while ( (thisMsg == -1) && !serverId.isEmpty() ) {
            int pos = msgPosFromUidl(serverId);
            if (pos == -1) {
                emit nonexistentMessage(serverId, Client::Removed);

                if (selectionItr != selectionMap.end()) {
                    serverId = selectionItr.key();
                    selectionItr++;
                } else {
                    serverId = QString();
                }
            } else {
                thisMsg = pos;
                msgUidl = serverId;
                mailSize = getSize(thisMsg);
            }
        }

        if ((!serverId.isEmpty()) && !preview) {
            retrieveUid = serverId;
        }
    }

    return thisMsg;
}

// get the reported server size from stored list
int PopClient::getSize(int pos)
{
    QMap<int, int>::const_iterator it = serverSize.find(pos);
    if (it != serverSize.end())
        return it.value();

    return -1;
}

void PopClient::uidlIntegrityCheck()
{
    QStringList deletedUids = QMailAccount(accountId).deletedMessages();

    // create list of new entries that should be downloaded
    if (preview) {
        QStringList previousList;

        foreach (const QString &uid, serverUid.values()) {
            // this message still exists
            if (deletedUids.contains(uid)) {
                deletedUids.removeAll(uid);
            } else {
                if ( (lastUidl.filter(uid)).count() == 0 ) {
                    uniqueUidlList.append(uid);
                } else {
                    previousList.append(uid);
                }
            }
        }

        lastUidl = previousList;
        messageCount = 0;
    }

    // Remove any deletion records we don't need any more
    foreach (const QString &uid, deletedUids)
        emit nonexistentMessage(uid, Client::Removed);
}

void PopClient::createMail()
{
    QString detachedFile = d->detach();
    { // Remove trailing pop terminator
        LongString ls(detachedFile);
        if (ls.right(7).toQByteArray() == "\r\n\r\n.\r\n")
            QFile::resize(detachedFile, ls.length() - 7);
    } // release detachedFile when ls is destructed
    
    QMailMessage mail = QMailMessage::fromRfc2822File( detachedFile );
    bool isComplete = ((!preview ) || ((preview) && (mailSize <= headerLimit)));
    mail.setStatus( QMailMessage::Downloaded, isComplete );

    mail.setSize(mailSize);
    mail.setServerUid(msgUidl);

    mail.setParentAccountId(accountId);
    mail.setMessageType(QMailMessage::Email);
    mail.setFromMailbox("");

    if(selectionMap.contains(mail.serverUid()))
        mail.setId(selectionMap.value(mail.serverUid()));

    mail.setHeaderField( "X-qtopia-internal-filename", detachedFile );
    emit newMessage(mail, !isComplete);

    d->reset();
    // Catch all cleanup of detached file
    QFile::remove(detachedFile);
}

void PopClient::checkForNewMessages()
{
    // We can't have new messages without contacting the server
    emit allMessagesReceived();
}

void PopClient::transportStatus(const QString& status)
{
    emit updateStatus(QMailMessageServer::None, status);
}

void PopClient::cancelTransfer()
{
    errorHandling(QMailMessageServer::ErrCancel, tr("Cancelled by user"));
}

void PopClient::retrieveOperationCompleted()
{
    // This retrieval may have been asynchronous
    emit allMessagesReceived();

    // Or it may have been requested by a waiting client
    emit retrievalCompleted();
}

