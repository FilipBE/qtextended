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
#include "connectionmanager.h"
#include <trace.h>
#include <qdplugin.h>
#include "pluginmanager.h"
#include <DesktopSettings>

QD_LOG_OPTION(ConnectionManager)

#ifndef RETRY_INTERVAL
#define RETRY_INTERVAL (DesktopSettings::debugMode()?DesktopSettings("settings").value("retry_interval",5000).toInt():500)
#endif

#ifndef WAIT_INTERVAL
#define WAIT_INTERVAL (DesktopSettings::debugMode()?DesktopSettings("settings").value("wait_interval",10000).toInt():10000)
#endif

class ConnectionManagerPrivate
{
public:
    int timerId;
    ConnectionManager::TimerAction timerAction;
    QDLinkPlugin *currentLink;
    QDConPlugin *currentConnection;
    QDDevPlugin *currentDevice;
    int state;
};

ConnectionManager::ConnectionManager( QObject *parent )
    : QObject( parent )
{
    d = new ConnectionManagerPrivate;
    d->currentLink = 0;
    d->currentConnection = 0;
    d->currentDevice = 0;
    d->state = QDConPlugin::Disconnected;

    foreach ( QDPlugin *p, qdPluginManager()->linkPlugins() )
        connect( p, SIGNAL(setState(QDLinkPlugin*,int)), this, SLOT(setLinkState(QDLinkPlugin*,int)) );
    foreach ( QDPlugin *p, qdPluginManager()->conPlugins() )
        connect( p, SIGNAL(setState(QDConPlugin*,int)), this, SLOT(setConnectionState(QDConPlugin*,int)) );

    d->timerId = startTimer( RETRY_INTERVAL );
    d->timerAction = TryConnect;
}

ConnectionManager::~ConnectionManager()
{
    delete d;
}

void ConnectionManager::stop()
{
    if ( d->timerId ) {
        killTimer( d->timerId );
        if ( d->timerAction == WaitForCon )
            setConnectionState( d->currentConnection, QDConPlugin::Disconnected );
        if ( d->timerAction == WaitForLink )
            setLinkState( d->currentLink, QDLinkPlugin::Down );
    } else {
        if ( d->currentConnection )
            setConnectionState( d->currentConnection, QDConPlugin::Disconnected );
        if ( d->currentLink )
            setLinkState( d->currentLink, QDLinkPlugin::Down );
    }

    // The actions above might cause the timer to get re-registered.
    if ( d->timerId )
        killTimer( d->timerId );
}

void ConnectionManager::timerEvent( QTimerEvent * /*e*/ )
{
    TRACE(ConnectionManager) << "ConnectionManager::timerEvent";

    killTimer( d->timerId );
    d->timerId = 0;

    if ( d->timerAction == WaitForCon ) {
        setConnectionState( d->currentConnection, QDConPlugin::Disconnected );
        return;
    }

    if ( d->timerAction == WaitForLink ) {
        setLinkState( d->currentLink, QDLinkPlugin::Down );
        return;
    }

    if ( d->timerAction == TryConnect ) {
        bool waiting = false;
        while ( !waiting ) {
            nextLink();
            if ( d->currentLink == 0 ) {
                LOG() << "You must have at least 1 link plugin enabled.";
                return;
            }
            nextConnection();
            if ( d->currentConnection == 0 ) {
                LOG() << "You must have at least 1 connection plugin enabled.";
                return;
            }
            waiting = d->currentLink->tryConnect( d->currentConnection );
            break;
        }
        if ( waiting ) {
            d->timerId = startTimer( WAIT_INTERVAL );
            d->timerAction = WaitForLink;
        } else {
            d->timerId = startTimer( RETRY_INTERVAL );
            d->timerAction = TryConnect;
        }
    }
}

void ConnectionManager::nextLink()
{
    TRACE(ConnectionManager) << "ConnectionManager::nextLink";
    bool next = false;
    foreach ( QDLinkPlugin *plugin, qdPluginManager()->linkPlugins() ) {
        if ( d->currentLink == 0 || next ) {
            d->currentLink = plugin;
            next = false;
            break;
        }
        if ( plugin == d->currentLink ) {
            next = true;
            continue;
        }
    }
    if ( next )
        d->currentLink = qdPluginManager()->linkPlugins().at(0);
    LOG() << (d->currentLink?d->currentLink->displayName():QString("NULL"));
}

void ConnectionManager::nextConnection()
{
    TRACE(ConnectionManager) << "ConnectionManager::nextConnection";
    bool next = false;
    foreach ( QDConPlugin *plugin, qdPluginManager()->conPlugins() ) {
        if ( d->currentConnection == 0 || next ) {
            d->currentConnection = plugin;
            next = false;
            break;
        }
        if ( plugin == d->currentConnection ) {
            next = true;
            continue;
        }
    }
    if ( next )
        d->currentConnection = qdPluginManager()->conPlugins().at(0);
    LOG() << (d->currentConnection?d->currentConnection->displayName():QString("NULL"));
}

void ConnectionManager::setLinkState( QDLinkPlugin *link, int state )
{
    TRACE(ConnectionManager) << "ConnectionManager::setLinkState" << "link" << link->displayName() << "state" << state;
    if ( link != d->currentLink ) {
        LOG() << link->displayName() << "sent state" << state << "when it wasn't the current link";
        return;
    }

    if ( d->timerId != 0 && d->timerAction == WaitForLink ) {
        killTimer( d->timerId );
        d->timerId = 0;
    }

    if ( state == QDLinkPlugin::Up ) {
        if ( d->currentConnection->tryConnect( d->currentLink ) ) {
            d->timerId = startTimer( WAIT_INTERVAL );
            d->timerAction = WaitForCon;
            return;
        }
    }

    d->currentLink->stop();

    if ( d->timerId == 0 ) {
        d->timerId = startTimer( RETRY_INTERVAL );
        d->timerAction = TryConnect;
    }
}

void ConnectionManager::setConnectionState( QDConPlugin *connection, int state )
{
    TRACE(ConnectionManager) << "ConnectionManager::setConnectionState" << "connection" << connection->displayName() << "state" << state;
    if ( connection != d->currentConnection ) {
        LOG() << connection->displayName() << "sent state" << state << "when it wasn't the current connection";
        return;
    }

    if ( d->timerId != 0 && d->timerAction == WaitForCon ) {
        killTimer( d->timerId );
        d->timerId = 0;
    }

    if ( state == QDConPlugin::Disconnected ) {
        // Notify the system that we're disconnected
        d->state = state;
        d->currentDevice = 0;
        emit setConnectionState( state );
        d->currentConnection->stop();
        d->currentLink->stop();

        if ( d->timerId == 0 ) {
            d->timerId = startTimer( RETRY_INTERVAL );
            d->timerAction = TryConnect;
        }
        return;
    }

    if ( state == QDConPlugin::Matching ) {
        // Notify the system that we're matching
        d->state = state;
        emit setConnectionState( state );
        foreach ( QDDevPlugin *p, qdPluginManager()->devPlugins() ) {
            p->probe( connection );
            if ( connection->device() == p ) {
                LOG() << "probed" << p->displayName();
                break;
            }
        }
    } else {
        if ( state == QDConPlugin::Connected )
            d->currentDevice = connection->device();
        // Notify the system that we're connected
        d->state = state;
        emit setConnectionState( state );
    }
}

QDDevPlugin *ConnectionManager::currentDevice()
{
    return d->currentDevice;
}

int ConnectionManager::state()
{
    return d->state;
}

