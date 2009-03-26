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

#ifndef QBLUETOOTHNAMESPACE_P_H
#define QBLUETOOTHNAMESPACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qbluetoothnamespace.h>

#ifdef QTOPIA_BLUETOOTH
#include <QIcon>
#endif

class QString;
class QStringList;
class QBluetoothRemoteDevice;

QString convertDeviceMajorToString(QBluetooth::DeviceMajor dev_major);
QString convertDeviceMinorToString(QBluetooth::DeviceMajor major, qint8 minor);
QStringList convertServiceClassesToString(QBluetooth::ServiceClasses classes);
QBluetooth::DeviceMajor major_to_device_major(quint8 major);

//TODO: Need a better solution for this.  I think eventually
//We will need a QBluetoothGui library/namespace.
#ifdef QTOPIA_BLUETOOTH
QBLUETOOTH_EXPORT QIcon find_device_icon(const QBluetoothRemoteDevice &remote);
QBLUETOOTH_EXPORT QIcon find_device_icon(QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses);
#endif

// NOTE This header is from GPLed library, but only defines interfaces
// According to local GPL experts this should be fine
#include <bluetooth/bluetooth.h>
#include <QString>

void str2bdaddr(const QString &addr, bdaddr_t *bdaddr);
QString bdaddr2str(const bdaddr_t *bdaddr);
bool _q_getSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options);
bool _q_setSecurityOptions(int sockfd, QBluetooth::SecurityOptions options);
bool _q_getL2CapSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options);
bool _q_setL2CapSecurityOptions(int sockfd, QBluetooth::SecurityOptions options);

#endif
