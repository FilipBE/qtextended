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

#include "systemclient.h"
#include "qtopialog.h"

#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QMailMessage>
#include <QSettings>
#include <QString>
#include <QtopiaChannel>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>


static const int retrievalTimeoutPeriod = 2000;


SystemClient::SystemClient(QObject* parent)
    : Client(parent)
{
    QtopiaChannel* channel = new QtopiaChannel("QPE/SysMessages", this);
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(received(QString,QByteArray)));

    connect(&retrievalTimer, SIGNAL(timeout()),
            this, SLOT(retrievalTimeout()));

    // See if there are any messages we have not yet received
    QtopiaServiceRequest req("SystemMessages", "collectMessages()");
    req.send();

    retrievalTimer.start(retrievalTimeoutPeriod);
}

SystemClient::~SystemClient()
{
}

bool SystemClient::hasDeleteImmediately() const
{
    return true;
}

void SystemClient::checkForNewMessages()
{
    if (retrievalTimer.isActive()) {
        // The retrieval action in progress will report new messages
    } else {
        // There are no outstanding messages
        emit allMessagesReceived();
    }
}

void SystemClient::received(const QString& message, const QByteArray& data)
{
    if (message == "postMessage(int,QDateTime,QString,QString)") {
        int messageId;
        QDateTime time;
        QString subject;
        QString text;

        // Extract the elements from this data stream
        QDataStream ds(data);
        ds >> messageId >> time >> subject >> text;

        // Create a message from the data
        QMailMessage message;
        message.setMessageType(QMailMessage::System);
        message.setStatus(QMailMessage::Downloaded, true);
        message.setStatus(QMailMessage::Read, false);
        message.setDate(QMailTimeStamp(time));
        message.setSubject(subject);
        message.setHeaderField("From", "System");

        QMailMessageContentType type("text/plain; charset=UTF-8");
        message.setBody(QMailMessageBody::fromData(text, type, QMailMessageBody::Base64));

        // Hand the message off for processing
        emit newMessage(message, false);

        // Acknowledge the reception of the event
        QtopiaIpcEnvelope e("QPE/SysMessages", "ackMessage(int)");
        e << messageId;

        // Don't wait indefinitely for completion tp be reported
        retrievalTimer.start(retrievalTimeoutPeriod);
    } else if (message == "processed()") {
        // All system messages have been consumed
        retrievalTimer.stop();
        emit allMessagesReceived();
    }
}

void SystemClient::retrievalTimeout()
{
    // We have timed out waiting for message delivery; report what we have to
    // allow the server to proceed
    qLog(Messaging) << "Timed out waiting for system message reception";
    retrievalTimer.stop();

    emit allMessagesReceived();
}

