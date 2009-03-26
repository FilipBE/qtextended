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

#include "atsessionmanager.h"
#include "atcallmanager.h"
#include "modememulatorservice.h"
#include <qserialport.h>
#include <qserialsocket.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qmap.h>

class AtSessionManagerPrivate
{
public:
    ~AtSessionManagerPrivate()
    {
        QMap<QString, QSerialIODevice *>::Iterator it1;
        for ( it1 = serialPorts.begin(); it1 != serialPorts.end(); ++it1 ) {
            delete it1.value();
        }

        QMap<int, QSerialSocketServer *>::Iterator it2;
        for ( it2 = tcpPorts.begin(); it2 != tcpPorts.end(); ++it2 ) {
            delete it2.value();
        }
    }

    QMap<QString, QSerialIODevice *> serialPorts;
    QMap<int, QSerialSocketServer *> tcpPorts;
    QMap<int, QString> tcpPortOptions;
    AtCallManager *callManager;
    bool taskRegistered;
    int nextTaskId;
};

AtSessionManager::AtSessionManager( QObject *parent )
    : QObject( parent )
{
    d = new AtSessionManagerPrivate();
    d->taskRegistered = false;
    d->nextTaskId = 0;
    d->callManager = new AtCallManager( this );

    // Start up the "ModemEmulator" service on this object.
    new ModemEmulatorService( this );
}

AtSessionManager::~AtSessionManager()
{
    delete d;
}

bool AtSessionManager::addSerialPort
        ( const QString& deviceName, const QString& options )
{
    // Bail out if the device is already bound.
    if ( d->serialPorts.contains( deviceName ) )
        return true;

    // Attempt to open the device.
    QSerialPort *port = QSerialPort::create( deviceName, 115200, true );
    if ( !port )
        return false;
    connect( port, SIGNAL(destroyed()), this, SLOT(serialPortDestroyed()) );

    // Zero reads on RFCOMM sockets should cause a close to occur.
    port->setKeepOpen( false );

    // Add the device to our list.
    d->serialPorts.insert( deviceName, port );
    registerTaskIfNecessary();

    // Wrap the device in a new session handler.  We hang it off the
    // serial port object so it will get cleaned up automatically
    // when removeSerialPort is called.
    AtFrontEnd *session = new AtFrontEnd( options, port );
    session->setDevice( port );
    emit newSession( session );
    return true;
}

bool AtSessionManager::addTcpPort
        ( int tcpPort, bool localHostOnly, const QString& options )
{
    // Bail out if the port is already bound.
    if ( d->tcpPorts.contains( tcpPort ) )
        return true;

    // Attempt to bind to the port.
    QSerialSocketServer *socket =
        new QSerialSocketServer( tcpPort, localHostOnly );
    if ( !socket->isListening() ) {
        delete socket;
        return false;
    }

    // Handle incoming connections.
    connect( socket, SIGNAL(incoming(QSerialSocket*)),
             this, SLOT(incoming(QSerialSocket*)) );

    // Add the port server to our list.
    d->tcpPorts.insert( tcpPort, socket );
    d->tcpPortOptions.insert( tcpPort, options );
    registerTaskIfNecessary();
    return true;
}

void AtSessionManager::removeSerialPort( const QString& deviceName )
{
    if ( d->serialPorts.contains( deviceName ) ) {
        delete d->serialPorts[deviceName];
        d->serialPorts.remove( deviceName );
    }
    unregisterTaskIfNecessary();
}

void AtSessionManager::removeTcpPort( int tcpPort )
{
    if ( d->tcpPorts.contains( tcpPort ) ) {
        delete d->tcpPorts[tcpPort];
        d->tcpPorts.remove( tcpPort );
        d->tcpPortOptions.remove( tcpPort );
    }
    unregisterTaskIfNecessary();
}

QStringList AtSessionManager::serialPorts() const
{
    return d->serialPorts.keys();
}

QList<int> AtSessionManager::tcpPorts() const
{
    return d->tcpPorts.keys();
}

AtCallManager *AtSessionManager::callManager() const
{
    return d->callManager;
}

void AtSessionManager::incoming( QSerialSocket *socket )
{
    // Arrange for the socket object to be destroyed when it is closed.
    connect( socket, SIGNAL(closed()), socket, SLOT(deleteLater()) );

    // Hang the socket from this object so that it will get cleaned
    // up when the manager is destroyed because the destructor for
    // AtSessionManagerPrivate cannot see such sockets normally.
    socket->setParent( this );

    // Find the options for the server that sent us this socket.
    QSerialSocketServer *server
        = qobject_cast<QSerialSocketServer *>( sender() );
    int port = 0;
    if ( server )
        port = server->port();
    QString startupOptions;
    if ( d->tcpPortOptions.contains( port ) )
        startupOptions = d->tcpPortOptions[port];

    // Wrap the socket in a new session handler.  We hang it off the
    // socket so it will get cleaned up automatically upon closing.
    AtFrontEnd *session = new AtFrontEnd( startupOptions, socket );
    session->setDevice( socket );
    emit newSession( session );

    // Register the socket as a task so that atinterface stays alive
    // while the socket is active even if removeTcpPort() is called
    // on the main listening port.
    QString name = "AtInterfaceSocket" + QString::number( d->nextTaskId++ );
    ((QtopiaApplication *)qApp)->registerRunningTask( name, session );
}

void AtSessionManager::serialPortDestroyed()
{
    QSerialIODevice *device = (QSerialIODevice *)sender();
    if ( device ) {
        QMap<QString, QSerialIODevice *>::Iterator it;
        for ( it = d->serialPorts.begin(); it != d->serialPorts.end(); ++it ) {
            if ( it.value() == device ) {
                d->serialPorts.erase( it );
                break;
            }
        }
        unregisterTaskIfNecessary();
    }
}

void AtSessionManager::registerTaskIfNecessary()
{
    if ( !d->taskRegistered ) {
        d->taskRegistered = true;
        ((QtopiaApplication *)qApp)->registerRunningTask( "AtInUse", this );
    }
    emit devicesChanged();
}

void AtSessionManager::unregisterTaskIfNecessary()
{
    if ( d->taskRegistered && !d->serialPorts.size() && !d->tcpPorts.size() ) {
        d->taskRegistered = false;
        ((QtopiaApplication *)qApp)->unregisterRunningTask( "AtInUse" );
    }
    emit devicesChanged();
}
