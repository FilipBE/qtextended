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

#ifndef MESSAGESSERVICE_H
#define MESSAGESSERVICE_H

#include <QContent>
#include <QMailMessageId>
#include <QMailMessage>
#include <QtopiaAbstractService>

class MessagesService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    MessagesService(QObject* parent);
    ~MessagesService();

signals:
    void view();
    void viewNew(bool);
    void view(const QMailMessageId&);
    void replyTo(const QMailMessageId&);
    void compose(QMailMessage::MessageType,
                 const QMailAddressList&,
                 const QString&,
                 const QString&,
                 const QContentList&,
                 QMailMessage::AttachmentsAction);
    void compose(const QMailMessage&);

public slots:
    void viewMessages();
    void viewNewMessages(bool userRequest);
    void viewMessage(QMailMessageId id);
    void replyToMessage(QMailMessageId id);
    void composeMessage(QMailMessage::MessageType,
                        QMailAddressList,
                        QString,
                        QString);
    void composeMessage(QMailMessage::MessageType,
                        QMailAddressList,
                        QString,
                        QString,
                        QContentList,
                        QMailMessage::AttachmentsAction);
    void composeMessage(QMailMessage);
};

#endif
