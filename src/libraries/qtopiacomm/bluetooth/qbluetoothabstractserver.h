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

#ifndef QBLUETOOTHABSTRACTSERVER_H
#define QBLUETOOTHABSTRACTSERVER_H

#include <QObject>

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothabstractsocket.h>

class QBluetoothAbstractServerPrivate;
struct sockaddr;

class QBLUETOOTH_EXPORT QBluetoothAbstractServer : public QObject
{
    Q_OBJECT
    friend class QBluetoothAbstractServerPrivate;

public:
    ~QBluetoothAbstractServer();

    virtual void close();

    bool isListening() const;

    QBluetoothAbstractSocket::SocketError serverError() const;
    QString errorString() const;

    int maxPendingConnections() const;
    void setMaxPendingConnections(int max);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    bool hasPendingConnections() const;
    QBluetoothAbstractSocket *nextPendingConnection();

    int socketDescriptor() const;

signals:
    void newConnection();

protected:
    explicit QBluetoothAbstractServer(QBluetoothAbstractServerPrivate *data,
                                        QObject *parent = 0);
    void setListening();
    virtual QBluetoothAbstractSocket * createSocket() = 0;

    QBluetoothAbstractServerPrivate *m_data;

private:
    Q_DISABLE_COPY(QBluetoothAbstractServer)
};

#endif
