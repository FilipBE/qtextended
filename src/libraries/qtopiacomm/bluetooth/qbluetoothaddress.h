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

#ifndef QBLUETOOTHADDRESS_H
#define QBLUETOOTHADDRESS_H

#include <qglobal.h>
#include <QString>

#include <qbluetoothglobal.h>

#ifdef QTOPIA_BLUETOOTH
#include <qtopiaipcmarshal.h>
#endif

class QBluetoothRemoteDevice;
class QBluetoothLocalDevice;

class QBLUETOOTH_EXPORT QBluetoothAddress
{

public:
    QBluetoothAddress();

    QBluetoothAddress(const QBluetoothAddress &addr);
    explicit QBluetoothAddress(const QString &addr);
    ~QBluetoothAddress();

    QBluetoothAddress &operator=(const QBluetoothAddress &other);
    bool operator==(const QBluetoothAddress &other) const;
    bool operator!=(const QBluetoothAddress &other) const
    {
        return !operator==(other);
    }

    bool isValid() const;
    QString toString() const;

    static const QBluetoothAddress invalid;
    static const QBluetoothAddress any;
    static const QBluetoothAddress all;
    static const QBluetoothAddress local;

#ifdef QTOPIA_BLUETOOTH
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
#endif

private:
    friend uint qHash(const QBluetoothAddress &addr);
    QString m_bdaddr;
    bool m_valid;
};

uint QBLUETOOTH_EXPORT qHash(const QBluetoothAddress &addr);

#ifdef QTOPIA_BLUETOOTH
Q_DECLARE_USER_METATYPE(QBluetoothAddress)
#endif

#endif
