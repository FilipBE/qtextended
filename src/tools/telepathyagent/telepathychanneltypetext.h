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

#ifndef TELEPATHYCHANNELTYPETEXT_H
#define TELEPATHYCHANNELTYPETEXT_H

#include <QObject>
#include <QString>
#include <QDateTime>

class TelepathyChannel;
class TelepathyChannelTypeTextPrivate;

class TelepathyChannelTypeText : public QObject
{
    Q_OBJECT

public:

    enum MessageType {
        Normal = 0,
        Action = 1,
        Notice = 2,
        Auto_Reply = 3
    };

    enum MessageFlag {
        Truncated = 1,
    };

    Q_DECLARE_FLAGS(MessageFlags, MessageFlag)

    enum SendError {
        Error_Unknown = 0,
        Error_Offline = 1,
        Error_Invalid_Contact = 2,
        Error_Permission_Denied = 3,
        Error_Too_Long = 4,
        Error_Not_Implemented = 5
    };

    QList<MessageType> messageTypes() const;

    struct PendingTextMessage {
        uint message_id;
        QDateTime timestamp;
        uint sender;
        MessageType message_type;
        MessageFlags flags;
        QString text;
    };

    TelepathyChannelTypeText(TelepathyChannel *chan);
    ~TelepathyChannelTypeText();

    bool acknowledgePendingMessages(const QList<uint> &ids);
    QList<PendingTextMessage> listPendingMessages(bool clear);
    bool send(MessageType type, const QString &text,
             QObject *handler, const char *successSlot, const char *errorSlot);

signals:
    void lostMessage();
    void received(const TelepathyChannelTypeText::PendingTextMessage &message);
    void sendError(TelepathyChannelTypeText::SendError error, const QDateTime &timestamp,
                   TelepathyChannelTypeText::MessageType, const QString &text);
    void sent(const QDateTime &timestamp, TelepathyChannelTypeText::MessageType, const QString &text);

private slots:
    void qt_sent(uint timestamp, uint messageType, const QString &text);
    void qt_sendError(uint error, uint timestamp, uint messageType, const QString &text);
    void qt_received(uint id, uint timestamp, uint sender, uint type, uint flags, const QString &text);

private:
    TelepathyChannelTypeTextPrivate *m_data;
    Q_DISABLE_COPY(TelepathyChannelTypeText)
};

#endif
