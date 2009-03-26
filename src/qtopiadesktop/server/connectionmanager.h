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
#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <qdplugindefs.h>

#include <QObject>

class ConnectionManagerPrivate;

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    ConnectionManager( QObject *parent = 0 );
    ~ConnectionManager();

    QDDevPlugin *currentDevice();

    enum TimerAction {
        TryConnect,
        WaitForLink,
        WaitForCon,
    };

    void stop();
    int state();

signals:
    void setConnectionState( int state );

private slots:
    void setConnectionState( QDConPlugin *connection, int state );
    void setLinkState( QDLinkPlugin *link, int state );

private:
    void timerEvent( QTimerEvent *e );
    void nextLink();
    void nextConnection();

    ConnectionManagerPrivate *d;
};

#endif
