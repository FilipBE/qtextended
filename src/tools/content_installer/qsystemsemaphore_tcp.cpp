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
#include "qsystemsemaphore.h"

#include <QTcpServer>
#include <QTimerEvent>
#include <QApplication>
#include <QTime>
#include <QTcpSocket>
#include <qdebug.h>
#include <unistd.h>

static quint32 content_installer_getPort(const QString &path)
{
    quint32 tcp_port = 0;
    // Set the TCP port for TCP semaphores
    QByteArray port = qgetenv("CONTENT_INSTALLER_PORT");
    if ( !port.isEmpty() )
        tcp_port = port.toInt();
    if ( (tcp_port <= 1024 || tcp_port > 65535) ) {
        tcp_port = 0;
        for ( int i = 0; i < path.length(); i++ ) {
            tcp_port += path.at(i).unicode(); // yes, this will probably overflow
        }
        tcp_port = (tcp_port % 64510) + 1025; // gives a range from 1025 to 65535
    }
    if ( tcp_port <= 1024 || tcp_port > 65535 )
        qFatal("BUG! The TCP port must be between 1025 and 65535 but it is %d", tcp_port);

    return tcp_port;
}

class QSystemSemaphorePrivate : public QObject
{
public:
    QSystemSemaphorePrivate()
        : QObject(),
        timerId( 0 )
    {
    }

    void timerEvent(QTimerEvent *e)
    {
        Q_ASSERT(e->timerId() == timerId);
        Q_UNUSED(e);
        killTimer(timerId);
        timerId = 0;
    }

    QTcpServer socket;
    int timerId;
    quint32 port;
};

QSystemSemaphore::QSystemSemaphore(const QString &fpath, int n)
{
    Q_UNUSED(n);
    d = new QSystemSemaphorePrivate;
    d->port = ::content_installer_getPort(fpath);
}

QSystemSemaphore::~QSystemSemaphore()
{
    d->socket.close();
    delete d;
}

void QSystemSemaphore::acquire()
{
    tryAcquire(-1);
}

void QSystemSemaphore::release()
{
    d->socket.close();
}

int QSystemSemaphore::available()
{
    return 0;
}

bool QSystemSemaphore::tryAcquire()
{
    return d->socket.listen(QHostAddress::LocalHost, d->port);
}

bool QSystemSemaphore::tryAcquire(int timeout)
{
    QTime t;
    t.start();
    while ( 1 ) {
        if ( tryAcquire() )
            return true;

        QTcpSocket socket;
        socket.connectToHost(QHostAddress::LocalHost, d->port);
        if ( socket.waitForConnected(timeout-t.elapsed()) )
            socket.waitForDisconnected(timeout-t.elapsed());

        if (timeout > 0 && t.elapsed() > timeout && !tryAcquire()) {
            qWarning() << "*********************************************************";
            qWarning() << "content_installer needs a unique port but it cannot open";
            qWarning() << "port" << d->port << ". Please kill any stale content_installer";
            qWarning() << "instances or use a different port.";
            qWarning() << "You can instruct content_installer to use a different port by";
            qWarning() << "setting the CONTENT_INSTALLER_PORT environment variable.";
            qWarning() << "*********************************************************";
            ::exit(1);
        }
    }
}

bool QSystemSemaphore::clear(const QString &fpath)
{
    quint32 tcp_port = ::content_installer_getPort(fpath);
    QTcpServer socket;
    if ( socket.listen(QHostAddress::LocalHost, tcp_port) ) {
        socket.close();
        return true;
    }
    qWarning() << "*********************************************************";
    qWarning() << "content_installer needs a unique port but it cannot open";
    qWarning() << "port" << tcp_port << ". Please kill any stale content_installer";
    qWarning() << "instances or use a different port.";
    qWarning() << "You can instruct content_installer to use a different port by";
    qWarning() << "setting the CONTENT_INSTALLER_PORT environment variable.";
    qWarning() << "*********************************************************";
    ::exit(1);
}

bool QSystemSemaphore::isValid()
{
    return true;
}

