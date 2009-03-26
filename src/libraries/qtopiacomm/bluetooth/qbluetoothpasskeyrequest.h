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

#ifndef QBLUETOOTHPASSKEYREQUEST_H
#define QBLUETOOTHPASSKEYREQUEST_H

#include <QString>
#include <qbluetoothglobal.h>
#include <qbluetoothaddress.h>

class QBLUETOOTH_EXPORT QBluetoothPasskeyRequest
{
public:
    QBluetoothPasskeyRequest(const QString &localDevice,
                                const QBluetoothAddress &remoteDevice);
    QBluetoothPasskeyRequest(const QBluetoothPasskeyRequest &req);

    ~QBluetoothPasskeyRequest();

    QBluetoothPasskeyRequest &operator=(const QBluetoothPasskeyRequest &other);
    bool operator==(const QBluetoothPasskeyRequest &other) const;
    bool operator!=(const QBluetoothPasskeyRequest &other) const;

    const QString &localDevice() const;
    const QBluetoothAddress &remoteDevice() const;

    void setRejected();
    bool isRejected() const;

    void setPasskey(const QString &passkey);
    const QString &passkey() const;

private:
    QString m_localDevice;
    QBluetoothAddress m_remoteDevice;
    QString m_passkey;
    bool m_rejected;
};

#endif
