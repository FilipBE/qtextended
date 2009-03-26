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

#include <QStringList>
#include <QLabel>
#include <QTextEdit>

#include <QBluetoothRfcommServer>
#include <QBluetoothAddress>
#include <QBluetoothRfcommSocket>

#include "rfcommserver.h"

RfcommServer::RfcommServer(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    textArea = new QTextEdit(this);
    textArea->setReadOnly(true);
    setCentralWidget(textArea);

    server = new QBluetoothRfcommServer(this);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    if (server->listen(QBluetoothAddress::any, 14)) {
        textArea->append(QString(tr("Server listening on: %1, channel: %2"))
                .arg(server->serverAddress().toString()).arg(server->serverChannel()));
    } else {
        textArea->append(tr("Was not able to listen on channel:"));
        textArea->append(server->errorString());
    }

    setWindowTitle(tr("RFCOMM Server"));
}

RfcommServer::~RfcommServer()
{
    
}

void RfcommServer::readyRead()
{
    QObject *s = sender();
    if (!s)
        return;

    QBluetoothRfcommSocket *socket = qobject_cast<QBluetoothRfcommSocket *>(s);

    if (!socket)
        return;

    handleClient(socket);
}

void RfcommServer::handleClient(QBluetoothRfcommSocket *socket)
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        QByteArray reply("Server Echo: ");
        reply.append(line);
        socket->write(reply);

        line.chop(2);
        textArea->append(QString(tr("From client: %1, data: %2"))
                .arg(socket->remoteAddress().toString()).arg(QString(line)));
    }
}

void RfcommServer::disconnected()
{
    QObject *s = sender();
    if (!s)
        return;

    QBluetoothRfcommSocket *socket = qobject_cast<QBluetoothRfcommSocket *>(s);

    if (!socket)
        return;

    textArea->append(QString(tr("Client %1 disconnected.")).arg(socket->remoteAddress().toString()));
}

void RfcommServer::newConnection()
{
    while (server->hasPendingConnections()) {
        QBluetoothRfcommSocket *socket =
            qobject_cast<QBluetoothRfcommSocket *>(server->nextPendingConnection());
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

        textArea->append(QString(tr("Client %1 connected.")).arg(socket->remoteAddress().toString()));

        handleClient(socket);
    }
}
