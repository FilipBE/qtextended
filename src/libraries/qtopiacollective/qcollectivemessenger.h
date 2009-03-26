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

#ifndef QCOLLECTIVEMESSENGER_H
#define QCOLLECTIVEMESSENGER_H

#include <qtopiaglobal.h>
#include <QAbstractIpcInterface>
#include <qcollectivenamespace.h>

class QCollectiveSimpleMessage;
class QCollectiveMessengerPrivate;

class QTOPIACOLLECTIVE_EXPORT QCollectiveMessenger : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    explicit QCollectiveMessenger(const QString &service = QString(),
                                 QObject *parent = 0, QAbstractIpcInterface::Mode = Client);
    ~QCollectiveMessenger();

public slots:
    virtual void sendMessage(const QCollectiveSimpleMessage &message);
    virtual void registerIncomingHandler(const QString &service);
    virtual void unregisterIncomingHandler(const QString &service);

signals:
    void messageSent(const QCollectiveSimpleMessage &message);
    void messageFailed(const QCollectiveSimpleMessage &message);

private:
    Q_DISABLE_COPY(QCollectiveMessenger)
    QCollectiveMessengerPrivate *d;
};

#endif
