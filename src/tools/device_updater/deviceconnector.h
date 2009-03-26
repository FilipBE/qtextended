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

#ifndef DEVICECONNECTOR_H
#define DEVICECONNECTOR_H

#include <QObject>
#include <QStringList>

class QTcpSocket;
class MessageRecord;
class QTimer;

class DeviceConnector : public QObject
{
    Q_OBJECT
public:
    enum MessageState {
        NoMessage,
        WaitForWelcome,
        User,
        WaitForUserOk,
        Password,
        WaitForPasswordOk,
        Message,
        WaitForMessageOk,
        MessageDelivered,
        MessageFailed
    };

    DeviceConnector();
    ~DeviceConnector();
    void sendQcop( const QString &, const QString &, const QByteArray & );
signals:
    void startingConnect();
    void finishedConnect();
    void sendProgress(int);
    void deviceConnMessage( const QString & );
private slots:
    void socketError();
    void socketReadyRead();
    void socketDisconnected();
    void processMessages();
private:
    void connect();
    void teardown();
    void sendMessage( const QString & );
    QTcpSocket *mSocket;
    QList<MessageRecord*> messageQueue;
    QStringList replyQueue;
    bool loginDone;
    QString loginName;
};

#endif
