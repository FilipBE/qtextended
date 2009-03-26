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

#ifndef QBLUETOOTHRFCOMMSOCKET_H
#define QBLUETOOTHRFCOMMSOCKET_H

#include <qbluetoothabstractsocket.h>

class QBluetoothRfcommSocketPrivate;

class QBLUETOOTH_EXPORT QBluetoothRfcommSocket : public QBluetoothAbstractSocket
{
    Q_OBJECT
public:
    explicit QBluetoothRfcommSocket(QObject *parent = 0);
    ~QBluetoothRfcommSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int channel,
                 QBluetooth::SecurityOptions options = 0);

    QBluetoothAddress remoteAddress() const;
    int remoteChannel() const;
    QBluetoothAddress localAddress() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
