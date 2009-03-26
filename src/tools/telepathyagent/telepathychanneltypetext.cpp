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

#include "telepathychanneltypetext.h"
#include "telepathyconnection.h"
#include "telepathychannel.h"

#include <qtopialog.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusError>

#define CHANNEL_TYPE_TEXT "org.freedesktop.Telepathy.Channel.Type.Text"

Q_DECLARE_METATYPE(QList<uint>)

class TelepathyChannelTypeTextPrivate
{
public:
    TelepathyChannelTypeTextPrivate();
    ~TelepathyChannelTypeTextPrivate();

    QDBusInterface *m_iface;
};

TelepathyChannelTypeTextPrivate::TelepathyChannelTypeTextPrivate()
{
    m_iface = 0;
}

TelepathyChannelTypeTextPrivate::~TelepathyChannelTypeTextPrivate()
{
    delete m_iface;
}

/*!
    \class TelepathyChannelTypeText
    \brief Class for sending and receiving messagaes in plain text.

    Corresponds to the Telepathy interface org.freedesktop.Telepathy.Channel.Type.Text
*/

/*!
    \fn lostMessage()

    Signal is emitted when a message is unable to be stored and forwarded.
*/

/*!
    \fn received(const TelepathyChannelTypeText::PendingTextMessage &message)

    Signal is emitted when a message \a message is received.
*/

/*!
    \fn sendError(TelepathyChannelTypeText::SendError error, const QDateTime &timestamp,
                   TelepathyChannelTypeText::MessageType, const QString &text

    Signal is emitted when the outgoing message has failed to send.
*/

/*!
    \fn sent(const QDateTime &timestamp, TelepathyChannelTypeText::MessageType, const QString &text)

    Signal is emitted when a message was sent at \a timestamp, of message type \a messageType and message \a text.
*/

/*!
    Constructs a TelepathyChannelTypeText on channel \a chan

*/
TelepathyChannelTypeText::TelepathyChannelTypeText(TelepathyChannel *chan)
    : QObject(chan)
{
    m_data = new TelepathyChannelTypeTextPrivate;
    QDBusConnection dbc = QDBusConnection::sessionBus();
    m_data->m_iface = new QDBusInterface(chan->connection()->service(),
                                         chan->path().path(),
                                         CHANNEL_TYPE_TEXT,
                                         dbc);

    dbc.connect(chan->connection()->service(), chan->path().path(),
                CHANNEL_TYPE_TEXT, "LostMessage",
                this, SIGNAL(lostMessage()));
    dbc.connect(chan->connection()->service(), chan->path().path(),
                CHANNEL_TYPE_TEXT, "Received",
                this, SIGNAL(qt_received(uint,uint,uint,uint,uint,QString)));
    dbc.connect(chan->connection()->service(), chan->path().path(),
                CHANNEL_TYPE_TEXT, "SendError",
                this, SIGNAL(qt_sendError(uint,uint,uint,QString)));
    dbc.connect(chan->connection()->service(), chan->path().path(),
                CHANNEL_TYPE_TEXT, "Sent",
                this, SIGNAL(qt_sent(uint,uint,QString)));
}

/*!
    Destructs TelepathyChannelTypeText
*/
TelepathyChannelTypeText::~TelepathyChannelTypeText()
{
    delete m_data;
}

/*!
    Returns true if successfully informing the channel that pending messages have been displayed, otherwise false.
*/
bool TelepathyChannelTypeText::acknowledgePendingMessages(const QList<uint> &ids)
{
    QDBusReply<void> reply = m_data->m_iface->call("AcknowledgePendingMessages",
            QVariant::fromValue(ids));

    if (!reply.isValid())
        return false;

    return true;
}

/*!
    Returns a list of all the pending messages.
*/
QList<TelepathyChannelTypeText::PendingTextMessage>
        TelepathyChannelTypeText::listPendingMessages(bool clear)
{
    QDBusMessage reply = m_data->m_iface->call("ListPendingMessages", clear);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "ListPendingMessages failed with error:" << reply;
        return QList<PendingTextMessage>();
    }

    const QDBusArgument &arg =
        *reinterpret_cast<const QDBusArgument *>(reply.arguments().at(0).constData());

    QList<PendingTextMessage> ret;

    arg.beginArray();
    while (!arg.atEnd()) {
        PendingTextMessage p;
        uint t;
        arg.beginStructure();
        arg >> p.message_id;
        arg >> t;
        if (t != 0)
            p.timestamp.setTime_t(t);
        arg >> p.sender;
        arg >> t;
        p.message_type = static_cast<MessageType>(t);
        arg >> t;
        p.flags = static_cast<MessageFlags>(t);
        arg >> p.text;
        arg.endStructure();
        ret.push_back(p);
    }
    arg.endArray();

    return ret;
}

/*!
    Returns true if the message of type \a type, with content \a text, sent to receive \a receiver, with success callback method of \a successSlot and failure callback method \a errorSlot.
*/
bool TelepathyChannelTypeText::send(MessageType type, const QString &text,
                                   QObject *receiver, const char *successSlot,
                                   const char *errorSlot)
{
    QVariantList args;
    args.append(static_cast<uint>(type));
    args.append(text);
    return m_data->m_iface->callWithCallback("Send", args, receiver, successSlot, errorSlot);
}

/*!
    Internal
*/
void TelepathyChannelTypeText::qt_received(uint id, uint timestamp, uint sender,
                                           uint type, uint flags, const QString &text)
{
    PendingTextMessage message;
    message.message_id = id;

    if (timestamp != 0)
        message.timestamp.setTime_t(timestamp);
    message.sender = sender;
    message.message_type = static_cast<MessageType>(type);
    message.flags = static_cast<MessageFlags>(flags);
    message.text = text;

    emit received(message);
}

/*!
    Internal
*/
void TelepathyChannelTypeText::qt_sendError(uint error, uint timestamp, uint type, const QString &text)
{
    QDateTime t;
    if (timestamp != 0)
        t.setTime_t(timestamp);

    emit sendError(static_cast<SendError>(error), t,
                   static_cast<MessageType>(type), text);
}

/*!
    Internal
*/
void TelepathyChannelTypeText::qt_sent(uint timestamp, uint type, const QString &text)
{
    QDateTime t;
    if (timestamp != 0)
        t.setTime_t(timestamp);

    emit sent(t, static_cast<MessageType>(type), text);
}
