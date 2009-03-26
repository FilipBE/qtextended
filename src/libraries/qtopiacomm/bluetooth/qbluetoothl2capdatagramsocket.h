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

#ifndef QBLUETOOTHL2CAPDATAGRAMSOCKET_H
#define QBLUETOOTHL2CAPDATAGRAMSOCKET_H

#include <qbluetoothabstractsocket.h>

class QBluetoothL2CapDatagramSocketPrivate;

class QBLUETOOTH_EXPORT QBluetoothL2CapDatagramSocket : public QBluetoothAbstractSocket
{
public:
    explicit QBluetoothL2CapDatagramSocket(QObject *parent = 0);
    ~QBluetoothL2CapDatagramSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int psm,
                 int incomingMtu = 672,
                 int outgoingMtu = 672);

    QBluetoothAddress remoteAddress() const;
    QBluetoothAddress localAddress() const;
    int remotePsm() const;
    int localPsm() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

    int incomingMtu() const;
    int outgoingMtu() const;

    bool bind(const QBluetoothAddress &local, int psm, int mtu = 672);
    qint64 readDatagram (char * data, qint64 maxSize,
                         QBluetoothAddress *address = 0, int *psm = 0 );

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
