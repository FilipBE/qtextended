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

#ifndef QBLUETOOTHRFCOMMSERVER_H
#define QBLUETOOTHRFCOMMSERVER_H

#include <QObject>

#include <qbluetoothnamespace.h>
#include <qbluetoothabstractserver.h>

class QBluetoothRfcommServerPrivate;
class QBluetoothAddress;
class QBluetoothRfcommSocket;

class QBLUETOOTH_EXPORT QBluetoothRfcommServer : public QBluetoothAbstractServer
{
public:
    explicit QBluetoothRfcommServer(QObject *parent = 0);
    ~QBluetoothRfcommServer();

    virtual void close();

    bool listen(const QBluetoothAddress &local, int channel);

    int serverChannel() const;
    QBluetoothAddress serverAddress() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

private:
    QBluetoothAbstractSocket * createSocket();

    Q_DISABLE_COPY(QBluetoothRfcommServer)
};

#endif
