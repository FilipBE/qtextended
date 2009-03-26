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

#ifndef QIRSERVER_H
#define QIRSERVER_H

#include <QObject>

#include <qirglobal.h>
#include <qirnamespace.h>

#include <qirsocket.h>

class QIrServerPrivate;

class QIR_EXPORT QIrServer : public QObject
{
    Q_OBJECT
    friend class QIrServerPrivate;

public:
    explicit QIrServer(QObject *parent = 0);
    ~QIrServer();

    void close();

    bool listen(const QByteArray &service, QIr::DeviceClasses classes = QIr::OBEX);

    QByteArray serverService() const;
    QIr::DeviceClasses serverDeviceClasses() const;

    bool isListening() const;

    QIrSocket::SocketError serverError() const;
    QString errorString() const;

    int maxPendingConnections() const;
    void setMaxPendingConnections(int max);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    bool hasPendingConnections() const;
    QIrSocket *nextPendingConnection();

    int socketDescriptor() const;

signals:
    void newConnection();

private:
    Q_DISABLE_COPY(QIrServer)
    QIrServerPrivate *m_data;
};

#endif
