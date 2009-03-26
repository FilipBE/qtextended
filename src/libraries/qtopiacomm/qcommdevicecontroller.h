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

#ifndef QCOMMDEVICECONTROLLER_H
#define QCOMMDEVICECONTROLLER_H

#include <qglobal.h>
#include <qobject.h>

#include <qtopiaglobal.h>

class QByteArray;

class QCommDeviceController_Private;

class QTOPIACOMM_EXPORT QCommDeviceController : public QObject
{
    Q_OBJECT

    friend class QCommDeviceController_Private;

public:
    enum PowerState { On, Off, OnOneItem, OnTimed };

    explicit QCommDeviceController(const QByteArray &deviceId, QObject *parent = 0);
    ~QCommDeviceController();

    const QByteArray &deviceId() const;

    void bringUp();
    void bringUpTimed(int secs);
    void bringUpOneItem();
    void bringDown();

    bool sessionsActive() const;

    // This grabs the value of the device out of the QValueSpace
    bool isUp() const;

    PowerState powerState() const;

signals:
    // These are optional, but would be nice to know when the device is up/down for
    // settings apps.  Should be just a QValueSpace signal anyway.
    void up();
    void down();

    void powerStateChanged(QCommDeviceController::PowerState state);

private:
    QCommDeviceController_Private *m_data;
};

#endif
