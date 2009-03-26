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

#ifndef QBLUETOOTHL2CAPSERVER_H
#define QBLUETOOTHL2CAPSERVER_H

#include <qbluetoothabstractserver.h>

class QBluetoothL2CapServerPrivate;
class QBluetoothAddress;
class QBluetoothL2CapSocket;

class QBLUETOOTH_EXPORT QBluetoothL2CapServer : public QBluetoothAbstractServer
{
public:
    explicit QBluetoothL2CapServer(QObject *parent = 0);
    ~QBluetoothL2CapServer();

    void close();

    bool listen(const QBluetoothAddress &local, int psm, int mtu = 672);

    int serverPsm() const;
    QBluetoothAddress serverAddress() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

    int mtu() const;

private:
    QBluetoothAbstractSocket *createSocket();

    Q_DISABLE_COPY(QBluetoothL2CapServer)
};

#endif
