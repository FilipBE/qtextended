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

#ifndef TELEPATHYCHANNEL_H
#define TELEPATHYCHANNEL_H

#include "telepathynamespace.h"

class TelepathyHandle;
class TelepathyChannelPrivate;
class QDBusObjectPath;
class QStringList;
class QString;
class TelepathyConnection;

class TelepathyChannel : public QObject
{
    Q_OBJECT

public:
    TelepathyChannel();
    TelepathyChannel(const QString &type, const QDBusObjectPath &path,
                     TelepathyConnection *conn);
    TelepathyChannel(const QString &type, const QDBusObjectPath &path,
                     Telepathy::HandleType handleType, uint handle,
                     TelepathyConnection *conn);
    ~TelepathyChannel();

    bool isValid() const;

    Telepathy::HandleType handleType() const;
    uint handle() const;

    QDBusObjectPath path() const;
    TelepathyConnection *connection();

    QStringList interfaces() const;
    QString type() const;

    bool close();

    Telepathy::Error lastError() const;
    QString lastErrorString() const;

signals:
    void closed();

private slots:
    void channelClosed();

private:
    Q_DISABLE_COPY(TelepathyChannel)
    TelepathyChannelPrivate *m_data;

    void initialize(const QString &type, const QDBusObjectPath &path,
                    Telepathy::HandleType handleType, uint handle,
                    TelepathyConnection *conn);
};

#endif
