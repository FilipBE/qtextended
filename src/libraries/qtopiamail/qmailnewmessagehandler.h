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

#ifndef QMAILNEWMESSAGEHANDLER_H
#define QMAILNEWMESSAGEHANDLER_H

#include <qtopiaglobal.h>

#include <QMailMessage>
#include <QtopiaAbstractService>
#include <QDSActionRequest>


class QTOPIAMAIL_EXPORT QMailNewMessageHandler : public QtopiaAbstractService
{
    Q_OBJECT

public:
    QMailNewMessageHandler(const QString &service, QObject* parent);
    ~QMailNewMessageHandler();

    virtual QMailMessage::MessageType messageType() const = 0;

signals:
    void newCountChanged(uint newCount);
    
public slots:
    void arrived(const QDSActionRequest &request);
    void setHandled(bool b);

private:
    QDSActionRequest request;
    bool pending;
};

class QTOPIAMAIL_EXPORT QMailNewSmsHandler : public QMailNewMessageHandler
{
public:
    QMailNewSmsHandler(QObject* parent = 0);

    virtual QMailMessage::MessageType messageType() const;
};

class QTOPIAMAIL_EXPORT QMailNewMmsHandler : public QMailNewMessageHandler
{
public:
    QMailNewMmsHandler(QObject* parent = 0);

    virtual QMailMessage::MessageType messageType() const;
};

class QTOPIAMAIL_EXPORT QMailNewEmailHandler : public QMailNewMessageHandler
{
public:
    QMailNewEmailHandler(QObject* parent = 0);

    virtual QMailMessage::MessageType messageType() const;
};

class QTOPIAMAIL_EXPORT QMailNewInstantMessageHandler : public QMailNewMessageHandler
{
public:
    QMailNewInstantMessageHandler(QObject* parent = 0);

    virtual QMailMessage::MessageType messageType() const;
};

class QTOPIAMAIL_EXPORT QMailNewSystemMessageHandler : public QMailNewMessageHandler
{
public:
    QMailNewSystemMessageHandler(QObject* parent = 0);

    virtual QMailMessage::MessageType messageType() const;
};

#endif
