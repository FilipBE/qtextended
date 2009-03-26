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

#ifndef QBLUETOOTHL2CAPSOCKET_H
#define QBLUETOOTHL2CAPSOCKET_H

#include <qbluetoothabstractsocket.h>

class QBluetoothL2CapSocketPrivate;

class QBLUETOOTH_EXPORT QBluetoothL2CapSocket : public QBluetoothAbstractSocket
{
public:
    explicit QBluetoothL2CapSocket(QObject *parent = 0);
    ~QBluetoothL2CapSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int psm,
                 int incomingMtu = 672,
                 int outgoingMtu = 672,
                 QBluetooth::SecurityOptions options = 0);

    QBluetoothAddress remoteAddress() const;
    QBluetoothAddress localAddress() const;
    int remotePsm() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;

    int incomingMtu() const;
    int outgoingMtu() const;

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
