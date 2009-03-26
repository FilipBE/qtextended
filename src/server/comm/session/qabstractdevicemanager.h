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

#ifndef QABSTRACTDEVICEMANAGER_H
#define QABSTRACTDEVICEMANAGER_H

#include <qglobal.h>
#include <qobject.h>

class QString;
class QAbstractCommDeviceManager_Private;
class QUnixSocket;

// SERVER SIDE

class QAbstractCommDeviceManager : public QObject
{
    Q_OBJECT

    friend class QAbstractCommDeviceManager_Private;

public:
    QAbstractCommDeviceManager(const QByteArray &serverPath,
                               const QByteArray &devId,
                               QObject *parent = 0);
    virtual ~QAbstractCommDeviceManager();

    bool start();
    bool isStarted() const;
    void stop();

    // Assume UNIX domain socket implementation
    const QByteArray &serverPath() const;
    // Unique DeviceId this manager is handling
    const QByteArray &deviceId() const;

    bool sessionsActive() const;

protected:
    // Actual implementation of bringUp.  Needs to handle async bring up/down
    // situations.  On success up() signal should be sent, on error, error signal
    // should be sent
    virtual void bringUp() = 0;
    // same as above
    virtual void bringDown() = 0;

    // Should be here to handle the situation of manager being started and the device
    // already being in "on/off" position
    virtual bool isUp() const = 0;

    virtual bool shouldStartSession(QUnixSocket *socket) const;
    virtual bool shouldBringDown(QUnixSocket *socket) const;

signals:
    void upStatus(bool error, const QString &msg);
    void downStatus(bool error, const QString &msg);

private:
    QAbstractCommDeviceManager_Private *m_data;
};

#endif
