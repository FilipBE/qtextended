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

#ifndef QBLUETOOTHSCOSERVER_H
#define QBLUETOOTHSCOSERVER_H

#include <qbluetoothabstractserver.h>

class QBluetoothScoServerPrivate;
class QBluetoothAddress;
class QBluetoothScoSocket;

class QBLUETOOTH_EXPORT QBluetoothScoServer : public QBluetoothAbstractServer
{
public:
    explicit QBluetoothScoServer(QObject *parent = 0);
    ~QBluetoothScoServer();

    void close();

    bool listen(const QBluetoothAddress &local);
    QBluetoothAddress serverAddress() const;

private:
    QBluetoothAbstractSocket * createSocket();

    Q_DISABLE_COPY(QBluetoothScoServer)
};

#endif
