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

#include "deviceconnector.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>

struct MessageRecord
{
    QString channel;
    QString message;
    QByteArray data;
    DeviceConnector::MessageState state;
};

/*!
  \class DeviceConnector
  \brief Connect to the Synchronization application on the device.

  Simple class to implement sending a message to the device over the qdsync
  protocol.  This protocol operates on a high port, and is listened for on the device
  by the Synchronization application.

  On the device, Synchronization picks up the message
  and responds by sending out a qcop message over the qcopbridge.
*/

DeviceConnector::DeviceConnector()
    : mSocket( 0 ), loginDone( false )
{
}

DeviceConnector::~DeviceConnector()
{
}

/*!
  Connect to the configured port and host for the device and setting up
  the socket.
*/
void DeviceConnector::connect()
{
    if ( mSocket )  // already connected
        return;
    emit startingConnect();
    loginDone = false;
    mSocket = new QTcpSocket();
    QObject::connect( mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError()) );
    mSocket->connectToHost( QHostAddress( "10.10.10.20" ), 4245 );
    if ( !mSocket->waitForConnected() )
    {
        emit deviceConnMessage( tr( "Error connecting to device:" ) +
                mSocket->errorString() );
        delete mSocket;
        mSocket = 0;
        return;
    }
    QObject::connect( mSocket, SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()) );
    QObject::connect( mSocket, SIGNAL(disconnected()),
            this, SLOT(socketDisconnected()) );
    emit deviceConnMessage( tr( "Device connected" ) );
    emit finishedConnect();
}

/*!
  Undo the connect and delete the socket.
*/
void DeviceConnector::teardown()
{
    if ( mSocket )
    {
        mSocket->deleteLater();
        mSocket = 0;
    }
}

/*!
  Respond to the sockets readyRead() signal by reading a line of
  message reply (if a full line is available).  Also trigger sending
  more messages.
  */
void DeviceConnector::socketReadyRead()
{
    // qDebug() << "socketReadyRead() - bytes:" << mSocket->bytesAvailable();
    if ( !mSocket->canReadLine() )
        return;
    QString reply;
    while ( mSocket->canReadLine() )
    {
        QByteArray bytes = mSocket->readLine();
        reply = QString( bytes ).trimmed();
        qDebug() << "recv:" << reply;
        replyQueue.append( reply );
    }
    while ( replyQueue.count() > 0 )
    {
        reply = replyQueue.takeFirst();
        if ( messageQueue.isEmpty() )
        {
            qDebug() << "ignoring:" << reply;
        }
        else
        {
            if ( messageQueue.first()->state == WaitForWelcome && reply.startsWith( "220" ))
            {
                QRegExp user("loginname=([^;]+)");
                if ( user.indexIn(reply) > -1 ) {
                    loginName = user.cap(1);
                }
                messageQueue.first()->state = User;
                qDebug() << "welcome ok:" << reply;
                break;
            }
            else if ( messageQueue.first()->state == WaitForUserOk && reply.startsWith( "331" ))
            {
                messageQueue.first()->state = Password;
                qDebug() << "user ok:" << reply;
                break;
            }
            else if ( messageQueue.first()->state == WaitForPasswordOk && reply.startsWith( "230" ))
            {
                messageQueue.first()->state = Message;
                qDebug() << "pass ok:" << reply;
                loginDone = true;
                break;
            }
            else if ( messageQueue.first()->state == WaitForMessageOk )
            {
                if ( reply.startsWith( "200" ))
                {
                    messageQueue.first()->state = MessageDelivered;
                    qDebug() << "msg ok:" << reply;
                }
                else if ( reply.startsWith( "500" ))
                {
                    messageQueue.first()->state = MessageFailed;
                    qDebug() << "msg fail:" << reply;
                }
            }
            else
            {
                qDebug() << "ignoring:" << reply;
            }
        }
    }
    if ( !messageQueue.isEmpty() )
        QTimer::singleShot( 0, this, SLOT(processMessages()) );
}

/*!
  Respond to an error signal from the socket, by
  deleting and setting the socket instance to zero, to indicate a
  new call to connect() is required.
*/
void DeviceConnector::socketError()
{
    if ( mSocket == 0 )
        return;
    QString err = mSocket->errorString();
    emit deviceConnMessage( tr( "Device connection problem:" ) + err );
    qDebug() << "socketError()" << err;
    teardown();
}

/*!
  Respond to the disconnected() signal from the socket, by deleting and
  setting the socket to null, to indicate that a new connect is required.
*/
void DeviceConnector::socketDisconnected()
{
    qDebug() << "socketDisconnected() - mSocket" << (long)mSocket;
    teardown();
}

/*!
  Send the message in the qdsync protocol
*/
void DeviceConnector::sendMessage( const QString &line )
{
    QTextStream os( mSocket );
    os << line << endl;
    qDebug() << "send:" << line;
    // mSocket->flush();
}

/*!
  Send a message to the Synchronization application which will cause a qcop message to be sent via the
  "qcop bridge".  This code taken from sendMessage(...) in src/qtopiadesktop/server.
  */
void DeviceConnector::sendQcop( const QString &channel, const QString &message, const QByteArray &data )
{
    MessageRecord *msg = new MessageRecord();
    msg->channel = channel;
    msg->message = message;
    msg->data = data;
    msg->state = NoMessage;
    messageQueue.append( msg );
    QTimer::singleShot( 0, this, SLOT(processMessages()) );
}

void DeviceConnector::processMessages()
{
    if ( messageQueue.isEmpty() )
        return;
    MessageRecord *msg = messageQueue.first();
    switch ( msg->state )
    {
        case NoMessage:
            connect();
            if ( !mSocket )
            {
                qDebug() << "No socket, retrying...";
                QTimer::singleShot( 500, this, SLOT(processMessages()) );
                return;
            }
            if ( loginDone )
                msg->state = Message;
            else
                msg->state = WaitForWelcome;
            QTimer::singleShot( 0, this, SLOT(processMessages()) );
            emit sendProgress( 10 );
            break;

        case WaitForWelcome:
            qDebug() << "wait for welcome";
            emit sendProgress( 20 );
            break;

        case User:
            // send user name
            // TODO this name comes in the 220 message!
            sendMessage( QString("USER %1").arg(loginName) );
            msg->state = WaitForUserOk;
            emit sendProgress( 30 );
            break;

        case WaitForUserOk:
            qDebug() << "wait for user ok";
            emit sendProgress( 40 );
            break;

        case Password:
            // send password
            sendMessage( "PASS Qtopia::deviceupdater" );
            msg->state = WaitForPasswordOk;
            emit sendProgress( 50 );
            break;

        case WaitForPasswordOk:
            qDebug() << "wait for user ok";
            emit sendProgress( 60 );
            break;

        case Message:
            // Now send the message
            emit sendProgress( 70 );
            {
                QTextStream os( mSocket );
                os << "CALLB " << msg->channel << " " << msg->message << " "
                   << msg->data.toBase64() << endl;
            }
            qDebug() << "send:" << msg->channel << "...";
            msg->state = WaitForMessageOk;
            break;

        case WaitForMessageOk:
            qDebug() << "wait for message ok";
            emit sendProgress( 80 );
            break;

        case MessageDelivered:
        case MessageFailed:
            emit sendProgress( 100 );
            if ( msg->state == MessageDelivered )
                emit deviceConnMessage( tr( "Device update message delivered" ));
            else
                emit deviceConnMessage( tr( "Device update message failed" ));
            if ( messageQueue.isEmpty() )
                teardown();
            Q_ASSERT( msg == messageQueue.first() );
            messageQueue.removeFirst();
            delete msg;
            break;

        default:
            qFatal( "Unknown state!" );
    }
}
