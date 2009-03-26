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

#include "networkserver.h"

#include "systemsuspend.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTimer>

#include <qtopianetworkinterface.h>
#include <qvaluespace.h>
#include <qtopialog.h>
#include <qnetworkstate.h>

class NetworkServerShutdownHandler : public SystemShutdownHandler
{
    Q_OBJECT
public:
    NetworkServerShutdownHandler( QtopiaNetworkServer* s )
        : server( s )
    {
    }

    bool systemRestart()
    {
        return doShutdown();
    }

    bool systemShutdown()
    {
        return doShutdown();
    }

    bool doShutdown()
    {
        qLog(Network) << "Initiating shutdown of QtopiaNetworkServer";
        QNetworkState* state = new QNetworkState( this );
        QStringList onlineList = state->interfacesOnline();
        if ( onlineList.isEmpty() ) {
            return true;
        }
        connect( state, SIGNAL(disconnected()), this, SLOT(finished()) );
        server->shutdownNetwork();

        //make sure that we don't block the shutdown process when our network interfaces don't come down
        //the timeout should be a couple of seconds as the shutdown requires the execution of
        //shell scripts
        QTimer::singleShot( SystemShutdownHandler::timeout(), this, SLOT(finished()) );//just in case
        return false;
    }

private slots:
    void finished()
    {
        static bool m_finished = false;
        if ( m_finished )
            return;
        m_finished = true;
        qLog(Network) << "Shutdown of QtopiaNetworkServer complete" ;
        emit proceed();
    }

private:
    QtopiaNetworkServer* server;
};

//QtopiaNetworkSession
QtopiaNetworkSession::QtopiaNetworkSession( const QByteArray& ifaceHandle, QObject* parent )
    : QObject( parent ), m_handle( ifaceHandle ), m_extended( false ),
         m_sessionVso( 0 )
{
    QByteArray hash;
    hash.setNum(qHash(m_handle));
    m_sessionVso = new QValueSpaceObject( "/Network/Sessions/"+hash, this);
    m_sessionVso->setAttribute( "ExtendedLifeTime", m_extended );

    //depends on QtopiaApplicationLifeCycle
    m_appVsi = new QValueSpaceItem( "/System/Applications", this );
    connect( m_appVsi, SIGNAL(contentsChanged()), this, SLOT(runningAppsChanged()) );

    m_ifaceVsi = new QValueSpaceItem( "/Network/Interfaces/"+hash+"/State", this );
    connect( m_ifaceVsi, SIGNAL(contentsChanged()), this, SLOT(interfaceStateChanged()) );

}

/*!
  \internal
  \a appName requested the network for this session.
  */
void QtopiaNetworkSession::addApplication( const QString& appName )
{
    if ( !m_apps.contains( appName ) ) {
        m_apps.insert( appName );
        m_sessionVso->setAttribute( "Subscriber", QStringList(m_apps.toList()).join( " " ) );
    }
}

void QtopiaNetworkSession::removeApplication( const QString& appName )
{
    if ( m_apps.contains( appName ) ) {
        m_apps.remove( appName );
        m_sessionVso->setAttribute( "Subscriber", QStringList(m_apps.toList()).join(" ") );
        //we don't have to emit the expiry as we assume that the session manager will
        //automatically cleanup such sessions
        /*if ( !m_extended && m_apps.isEmpty() )
            emit sessionExpired( m_handle );*/
    }
}

bool QtopiaNetworkSession::applicationIsRegistered( const QString& appName )
{
    return m_apps.contains( appName );
}

int QtopiaNetworkSession::subscribedApplications()
{
    return m_apps.count();
}

void QtopiaNetworkSession::setExtendedLife( bool isExtended ) {
    m_extended = isExtended;
    m_sessionVso->setAttribute( "ExtendedLifeTime", m_extended );
    if ( !m_extended && m_apps.isEmpty() )
        emit sessionExpired( m_handle );
}

bool QtopiaNetworkSession::extendedLife() {
    return m_extended;
}

void QtopiaNetworkSession::runningAppsChanged()
{
    QSet<QString> allApps = QSet<QString>::fromList(m_appVsi->subPaths());
    QSet<QString> remaining = m_apps;
    remaining.subtract( allApps );
    if ( !remaining.isEmpty() ) {
        //application(s) just quit or crashed w/o unregistering its network session
        if ( qLogEnabled(Network) && !m_extended )
            qLog(Network) << remaining.toList() << "didn't cleanup network session";
        m_apps.subtract( remaining );
        m_sessionVso->setAttribute( "Subscriber", QStringList(m_apps.toList()).join(" ") );
    }

    if ( m_apps.isEmpty() && !m_extended ) {
        emit sessionExpired( m_handle );
    }
}

//this serves as fallback in case the interface goes away and nobody bothered to inform the session
//manager
void QtopiaNetworkSession::interfaceStateChanged()
{
    QVariant v = m_ifaceVsi->value();
    if ( !v.isValid() )
        return;

    QtopiaNetworkInterface::Status s =
        (QtopiaNetworkInterface::Status)(v.toInt());
    switch( s ) {
        case QtopiaNetworkInterface::Down:
        case QtopiaNetworkInterface::Unavailable:
        case QtopiaNetworkInterface::Unknown:
            emit sessionObsolete( m_handle );
            break;
        default:
            break;
    }
}

/*!
  \internal
  \class QtopiaNetworkSession
  \inpublicgroup QtConnectivityModule
  \inpublicgroup QtBluetoothModule
  \brief This class keeps track of requested network sessions.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
  \internal
  Constrcuts a new network session manager for Qtopia.
  */
QtopiaSessionManager::QtopiaSessionManager( QObject* parent )
    : QObject( parent )
{
}


QtopiaSessionManager::~QtopiaSessionManager()
{
}

/*!
  \internal

  Registers \a appName as a subscriber to the network session associated with \a ifaceHandle.
  */
void QtopiaSessionManager::registerSession( const QByteArray& ifaceHandle, const QString& appName )
{
    QtopiaNetworkSession* s = 0;
    if ( !sessions.contains( ifaceHandle ) ) {
        qLog(Network) << "Creating network session for" << appName << "on" << ifaceHandle;
        s = new QtopiaNetworkSession( ifaceHandle, this );
        s->addApplication( appName );
        connect( s, SIGNAL(sessionExpired(QByteArray)), this, SLOT(sessionChanged(QByteArray)) );
        connect( s, SIGNAL(sessionObsolete(QByteArray)), this, SLOT(sessionObsolete(QByteArray)) );
        sessions.insert( ifaceHandle, s );
    } else {
        qLog(Network) << "Adding" << appName << "to the network session associated with" << ifaceHandle;
        s = sessions.value( ifaceHandle );
        if ( !s->applicationIsRegistered( appName ) )
            s->addApplication( appName );
    }
}

/*!
  \internal

  Unregisters \a appName from the network session associated with \a ifaceHandle.
  If the session doesn't have any subscriber left the session is removed unless the session
  has been marked as extended.

  If \a appName is not a subscriber to the session running on \a ifaceHandle, the session
  is stopped as well. This allows other applications to stop an interface that wasn't started
  by themselves. The only exception are again extended sessions.
  */
void QtopiaSessionManager::stopSession( const QByteArray& ifaceHandle, const QString& appName )
{
    if ( !sessions.contains( ifaceHandle ) )
        return;

    bool thirdPartyQuit = false;
    QtopiaNetworkSession* s = sessions.value( ifaceHandle );
    if ( s->applicationIsRegistered( appName ) ) {
        qLog(Network) << "Removing" << appName << "from network session on" << ifaceHandle;
        s->removeApplication( appName );
    } else {
        //another application explicitly requested the stopping of this session
        thirdPartyQuit = true;
    }

    if ( s->extendedLife() )
       return;

    if ( !thirdPartyQuit && s->subscribedApplications() > 0 )
        return;

    qLog(Network) << "Closing network session associated with" << ifaceHandle;
    sessions.remove( ifaceHandle );
    delete s;
}

/*!
    \internal

    \fn void QtopiaSessionManager::quitInterface( const QString& handle );

    This signal is emitted when the session manager detected that an interface
    is obsolete. This situation occurs when an application requested/started a
    network interface and didn't close the connection or crashed. In this case
    the session manager can assist in the process of cleaning up network connections
    when the application that requested the connection doesn't exist anymore.
*/


/*!
  \internal

  Sets the life time of the network session associated with \a ifaceHandle to \a isExtended.
  An extended life time means that the network server won't stop a network connection
  that was started by an application which is not running anymore.

  Unsetting the life time extension implies that the attached session
  expires (no matter how many subscribers exist). This is equivalent to a hard stop of the interface.
  */
void QtopiaSessionManager::setExtendedLifetime( const QByteArray& ifaceHandle, bool isExtended )
{
    if ( !sessions.contains( ifaceHandle ) ) {
        qWarning() << ifaceHandle << "is doesn't have an attached session. Cannot extend life time.";
        return;
    }
    QtopiaNetworkSession* s = sessions.value( ifaceHandle );
    s->setExtendedLife( isExtended );

}

/*!
  \internal

  Returns true if \a ifaceHandle is associated to a valid network session. Valid means that
  there is either an application left which started a network session via \a ifaceHandle
  or the associated session has an extended life time.
  */
bool QtopiaSessionManager::isValidSession( const QByteArray& ifaceHandle ) const
{
    if ( !sessions.contains( ifaceHandle ) )
        return false;

    QtopiaNetworkSession* netSession = sessions.value( ifaceHandle );
    if ( netSession->subscribedApplications() || netSession->extendedLife() )
        return true;

    return false;
}

/*!
  \internal

  Emits the quitInterface signal when ifaceHandle's network session expired.
  The session object detected that the application which requested the network
  went offline/crashed. This will notify the network server to take this interface
  down.
  */
void QtopiaSessionManager::sessionChanged( const QByteArray& ifaceHandle )
{
    if ( !sessions.contains( ifaceHandle ) )
        return;

    QtopiaNetworkSession* netSession = sessions.value( ifaceHandle );
    if ( netSession->extendedLife() || netSession->subscribedApplications() )
        return;

    qLog(Network) << "Network session w/o client detected. Initiating shutdown of interface" << ifaceHandle;
    sessions.remove( ifaceHandle );
    netSession->deleteLater(); //netsession may have triggered this slot

    emit quitInterface( QString(ifaceHandle) );
}

/*!
  \internal

  A session discovered that it became obsolete (the associated interface went offline ).
  This means the network server didn't have any time to inform the session manager or the
  interface timed out and disconnected.

  */
void QtopiaSessionManager::sessionObsolete( const QByteArray& ifaceHandle )
{
    if ( !sessions.contains( ifaceHandle ) )
        return;

    qLog(Network) << "Obsolete network session detected on" << ifaceHandle;
    QtopiaNetworkSession* netSession = sessions.value( ifaceHandle );
    if ( qLogEnabled(Network) && netSession->extendedLife() )
            qLog(Network) << "Obsolete session had extended life time";
    sessions.remove( ifaceHandle );
    netSession->deleteLater(); //netSession triggered this slot
}

/*!
  \internal

  This invalidates (and removes) the session attached to ifaceHandle. Note that
  it is the callers responsibility to stop the interface identified by \a ifaceHandle.
  */
void QtopiaSessionManager::invalidateSession( const QByteArray& ifaceHandle )
{
    qLog(Network) << "Invalidating network session on" << ifaceHandle;
    sessionObsolete( ifaceHandle );
}


/* -------------------------------------------------------*/
/* network server implementation */

/*!
  \class QtopiaNetworkServer
  \inpublicgroup QtBluetoothModule
  \inpublicgroup QtConnectivityModule
  \ingroup QtopiaServer::Task
  \brief The QtopiaNetworkServer class manages all network interfaces.

  This server task synchronizes the network access across all Qt Extended applications.
  Client applications request network functions via QtopiaNetwork which
  forwards the requests to this network server. The network server creates 
  and keeps the references to the various QtopiaNetworkInterface instance. All
  forwarded network requests are directly executed by this server class. 

  The network server ensures the automated start-up of network interfaces during 
  the system start phase, stops all network interfaces when the system shuts down and 
  manages the network sessions for all network devices.
  External network devices such as PCMCIA cards are automatically detected and initialized 
  by the network server when they are plugged into the Qt Extended device.

  The QtopiaNetworkServer is a Qt Extended server task and is automatically started by the server.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa QtopiaNetwork, QtopiaNetworkInterface
*/

/*!
  \enum QtopiaNetworkServer::StartCode

  \internal
*/


/*!
  Constructs the network server.
*/
QtopiaNetworkServer::QtopiaNetworkServer()
    : QtopiaIpcAdaptor( "QPE/Network" ), tidAutoStart( 0 ), lockdown( false )
{
    NetworkServerShutdownHandler* ss = new NetworkServerShutdownHandler( this );
    QtopiaServerApplication::addAggregateObject( this, ss );

    SystemSuspend *suspend = qtopiaTask<SystemSuspend>();
    Q_ASSERT(suspend);
    QObject::connect(suspend, SIGNAL(systemActive()), this, SLOT(updateNetwork()));

    sessionManager = new QtopiaSessionManager( this );
    QObject::connect( sessionManager, SIGNAL(quitInterface(QString)),
            this, SLOT(sessionManagerQuitRequest(QString)) );

    QtopiaIpcAdaptor* stab = new QtopiaIpcAdaptor("QPE/Card", this);
    QtopiaIpcAdaptor::connect(stab, MESSAGE(stabChanged()),
                        this, SLOT(cardMessage()), QtopiaIpcAdaptor::SenderIsChannel);

    nwState = new QtopiaIpcAdaptor( "QPE/NetworkState", this );
    QtopiaIpcAdaptor::connect( nwState, MESSAGE(updateNetwork()),
                         this, SLOT(updateNetwork()), QtopiaIpcAdaptor::SenderIsChannel );

    publishAll( Slots );
    updateNetwork();

    QSettings cfg( "Trolltech", "Network" );
    lockdown = cfg.value("Info/Lockdown", false ).toBool();

    gatewaySpace = new QValueSpaceObject( "/Network/Gateway", this );
    gatewaySpace->setAttribute( "Default", QString() );

    if ( lockdown ) {
        qLog(Network) << "Network is in lock down mode. Auto start of network interfaces not possible.";
    } else {
        //setup autostart
        QStringList configList = QtopiaNetwork::availableNetworkConfigs();
        foreach( QString config, configList ) {
            QSettings cfg( config, QSettings::IniFormat );
            bool autoStart = cfg.value("Properties/Autostart","n").toString() == "y";
            if ( autoStart )
                autoStartConfigs.append( config );
        }
        if ( autoStartConfigs.count() > 0)
            tidAutoStart = startTimer( 10000 );
    }
}

/*!
  Destroys the QtopaNetworkServer instance.
*/
QtopiaNetworkServer::~QtopiaNetworkServer()
{
    shutdownNetwork();
}

/*!
  \internal

  Stops any kind of networking. This function works similar to privilegedInterfaceStop by circumventing
  the session manager.
*/
void QtopiaNetworkServer::shutdownNetwork()
{
    qLog(Network) << "QNS: Shutting down all running network interfaces";
    QStringList configList = QtopiaNetwork::availableNetworkConfigs();
    foreach( QString config, configList )
    {

        QSettings netConfig(config, QSettings::IniFormat);
        if (netConfig.value("Properties/DebugInterface", false).toBool()) {
            qLog(Network) << "Not stopping network interface used for debugging" << config;
            continue;
        }

        privilegedInterfaceStop( config );
    }

    updateNetwork();
}

/*!
  Returns true if \a ref is associated to a device that is already used
  by another interface instance, otherwise false.
*/
bool QtopiaNetworkServer::interfaceDeviceBusy( const QPointer<QtopiaNetworkInterface> ref)
{
    QStringList configList = QtopiaNetwork::availableNetworkConfigs();
    foreach( QString config, configList )
    {
        QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( config );
        if ( !iface || iface == ref)
            continue;
        switch (iface->status())  {
            default:
                continue;
            case QtopiaNetworkInterface::Up:
            case QtopiaNetworkInterface::Pending:
            case QtopiaNetworkInterface::Demand:
                if ( ref->device() == iface->device() )
                    return true;
                break;
        }
    }

    return false;
}

/*!
  \internal

  Starts the network interface identified by \a handle.
*/
void QtopiaNetworkServer::startInterface(const QString& appName, const QString& handle, const QVariant& options)
{
    activateInterface( appName, handle, options );
}

/*!
  \internal

  Starts the network interface identified by \a handle. Returns \c{NotReady} if
  the iface refused the start request.
*/
QtopiaNetworkServer::StartCode QtopiaNetworkServer::activateInterface( const QString& appName,
        const QString& handle, const QVariant& options )
{
    if (handle.isEmpty() || appName.isEmpty() )
        return Unavailable;

    if ( lockdown ) {
        qLog(Network) << handle << "can't be started due to network lockdown.";
        return Unavailable;
    }
    qLog(Network) << "Starting interface ->" << handle;

    QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( handle );
    if ( iface ) {
        if (iface->status() == QtopiaNetworkInterface::Unknown)
            iface->initialize();
        QtopiaNetworkInterface::Status state = iface->status();
        if (state == QtopiaNetworkInterface::Unavailable)
            return Unavailable;

        switch ( state ) {
            case QtopiaNetworkInterface::Down:
            {
                if ( !interfaceDeviceBusy( iface ) ) {
                    qLog(Network) << "QNS::startInterface: starting " << handle;
                    bool success = iface->start( options );
                    if ( success ) {
                        sessionManager->registerSession( handle.toLatin1().constData(), appName );
                        return Success;
                    } else {
                        qLog(Network) << "QNS::startInterface: interface start was not successful";
                        return NotReady;
                    }
                } else {
                    qLog(Network) << "QNS::startInterface: the device associated with"
                        << handle << "is in use/online already.";
                }
                break;
            }
            case QtopiaNetworkInterface::Demand:
            case QtopiaNetworkInterface::Pending:
            case QtopiaNetworkInterface::Up:
                qLog(Network) << handle << "already running.";
                sessionManager->registerSession( handle.toLatin1().constData(), appName );
                break;
            default:
                break;

        }
    } else {
        qLog(Network) << "QNS::startInterface: no plugin available for" <<
            handle;
    }

    return Unavailable;
}

/*!
  \internal

  Stops the network interface specified by \a handle. \a appName is the name of the application
  that requested this action.
  If \a deleteIface is true and there is no pedning session on anymore, the interface will be
  stopped, its cleanup function called, the associated configuration file and the
  plugin instance deleted.
*/
void QtopiaNetworkServer::stopInterface( const QString& appName, const QString& handle, bool deleteIface )
{
    if ( handle.isEmpty() || appName.isEmpty() )
        return;

    qLog(Network) << "QNS::stopInterface -> " << handle;
    QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( handle );

    if (iface ) {
        switch(iface->status())
        {
            case QtopiaNetworkInterface::Unknown:
                delete iface;
                break;
            case QtopiaNetworkInterface::Pending:
            case QtopiaNetworkInterface::Demand:
            case QtopiaNetworkInterface::Up:
                sessionManager->stopSession( handle.toLatin1().constData(), appName );
                if ( sessionManager->isValidSession( handle.toLatin1().constData() ) )
                    return;

                qLog(Network) << "QNS::stopInterface: stopping " << handle;
                //if this interface is the default gateway reset it to sth new before we go ahead
                if ( gateway == handle ) {
                    //set new gateway and don't use handle at all
                    setDefaultGateway( handle, true );
                }

                iface->stop();
            case QtopiaNetworkInterface::Unavailable:
            case QtopiaNetworkInterface::Down:
                break;
        }
    }

    if ( deleteIface ) {
        qLog(Network) << "QNS::stopInterface: deleting " << handle;
        if ( iface ) {
            iface->cleanup();
            delete iface;
        }
        QFile::remove( handle );
    }

    updateNetwork();
}

/*!
  \internal

  Stops the network interface identified by \a handle w/o considering the network session manager.
  This function must have restricted access as it could potentially allow a rogue application to
  shutdown the network.
  */
void QtopiaNetworkServer::privilegedInterfaceStop( const QString& handle )
{
    if ( handle.isEmpty() )
        return;

    QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( handle );
    if (!iface)
        return;
    QtopiaNetworkInterface::Status state = iface->status();
    if (state == QtopiaNetworkInterface::Unknown) {
        delete iface;
        return;
    }

    switch(state) {
        case QtopiaNetworkInterface::Unavailable:
        case QtopiaNetworkInterface::Down:
            return;
        default:
            qLog(Network) << "QNS: Privileged stop of interface: " << iface->device();
            //if this interface is the default gateway reset it to sth new before we go ahead
            if ( gateway == handle ) {
                //set new gateway and don't use handle at all
                setDefaultGateway( handle, true );
            }
            sessionManager->invalidateSession( handle.toLatin1().constData() );
            iface->stop();
            break;
    }
}

/*!
  \internal

  Handles incoming messages on QPE/Card
*/
void QtopiaNetworkServer::cardMessage()
{
    qLog(Network) << "QNS: stab changed ( updating interfaces )";
    updateNetwork();
}

/*!
  \internal

  Updates the currently known and active network interfaces
*/
void QtopiaNetworkServer::updateNetwork()
{
    qLog(Network) << "###### General network update ######";
    QStringList configList = QtopiaNetwork::availableNetworkConfigs();
    foreach( QString config, configList )
    {
        QPointer<QtopiaNetworkInterface> iface =
            QtopiaNetwork::loadPlugin( config );
        if ( !iface )
            continue;
        QtopiaNetworkInterface::Status state = iface->status();
        if ( state == QtopiaNetworkInterface::Up
                || state == QtopiaNetworkInterface::Pending
                || state == QtopiaNetworkInterface::Demand  )
        {
            qLog(Network) << "QNS::updateNetwork(): " << config
                        << " ONLINE on " << iface->device();
        } else if ( state == QtopiaNetworkInterface::Unknown) {
                iface->initialize();
        }
    }
}

/*!
  \internal

  Activates the network interface identified by \a handle as default gateway.
  If the device is connected to more than one network at a time this can be used to
  choose a preferred network for data transfers.
  If \a excludeInterface is true, this function essentially chooses the next best
  interface that is online and is not the same as handle. Essentially \a handle will be
  unset from being the gateway.
*/
void QtopiaNetworkServer::setDefaultGateway( const QString& handle, bool excludeInterface )
{
    qLog(Network) << "QNS::setDefaultGateway -> " << handle << excludeInterface;
    if ( handle.isEmpty() ) {
        qWarning() << "Empty gateway config passed";
        return;
    }

    if ( excludeInterface ) {
        if ( handle != gateway ) //handle is not gateway ->nothing to do
            return;

        // find next interface that is online and set it as default gateway
        //this could be a bit smarter by evaluating what interface to use
        // give online interfaces a higher priority than interfaces in demand state
        QStringList configList = QtopiaNetwork::availableNetworkConfigs();
        foreach( QString config, configList ) {
            if ( config == handle )
                continue; //exclude this interface

            QPointer<QtopiaNetworkInterface> iface =
                QtopiaNetwork::loadPlugin( config );
            if ( !iface )
                continue;

            QtopiaNetworkInterface::Status state = iface->status();
            if ( state == QtopiaNetworkInterface::Up ) {
                //use first interface
                iface->setDefaultGateway();
                gateway = config;
                gatewaySpace->setAttribute( "Default", gateway );
                qLog(Network) << gateway << "becomes new default gateway";
                return;
            }
       }

        //we couldn't find anything -> unset gateway
        gateway = QLatin1String("");
        gatewaySpace->setAttribute( "Default", gateway );
        qLog(Network) << "Cannot find an interface that could become new default gateway";

        return;
    }

    //we were given a particular config
    QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( handle );
    if ( iface ) {
        QtopiaNetworkInterface::Status state = iface->status();
        if ( state == QtopiaNetworkInterface::Up ) {
            //only a running iface can be configured
            iface->setDefaultGateway();
            gateway = handle;
            gatewaySpace->setAttribute( "Default", gateway );
        } else {
            qLog(Network) << "Not possible to set default gateway to an interface that isn't online";
        }
    }
}

/*!
  \internal

  This function marks \a handle as a permanent interface. A permanent interface is not stopped
  by network session management if the application which started the interface crashes
  or quits.

  Currently only applications in SXE's netconfig domain are able to call this function. This feature
  is used by netsetup in order to enforce the global start of an interface.

  Unsetting the life time property of a network session that is extended but doesn't have any
  subscribed applications, will cause the stop of this interface. Applications can stop
  privileged network devices by calling \l QtopiaNetwork::privilegedInterfaceStop(). 
  */
void QtopiaNetworkServer::setExtendedInterfaceLifetime( const QString& handle, bool isExtended )
{
    QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( handle );
    if ( iface ) {
        QtopiaNetworkInterface::Status state = iface->status();
        if ( state == QtopiaNetworkInterface::Up
                || state == QtopiaNetworkInterface::Pending
                || state == QtopiaNetworkInterface::Demand ) {
            sessionManager->setExtendedLifetime( handle.toLatin1().constData(), isExtended );
            qLog(Network) << "Setting extended life time for" << handle << "to" << isExtended;
            return;
        }
    }
    qLog(Network) << "Cannot change life time for network interface that is offline" << handle;
}

/*!
  \internal

  If \a isLocked is true the network server prevents new connections and initiates the shutdown
  of network connections.
  */
void QtopiaNetworkServer::setLockMode( bool isLocked )
{
    qLog(Network) << "QNS: Setting lock down mode to" << isLocked;
    lockdown = isLocked;
    QSettings cfg( "Trolltech", "Network" );
    cfg.setValue("Info/Lockdown", lockdown );
    if ( lockdown )
        shutdownNetwork();
}

void QtopiaNetworkServer::sessionManagerQuitRequest( const QString& handle )
{
   qLog(Network) << "Stopping" << handle << "on request by session manager";
   stopInterface( qApp->applicationName(), handle, false );
}
/*!
  \internal

  Start interfaces automatically if requested by configuration.
*/
void QtopiaNetworkServer::timerEvent(QTimerEvent* /*tEvent*/)
{
    static int attempts = 0;

    if ( autoStartConfigs.count() == 0 || attempts > 20 ) {
        killTimer( tidAutoStart );
        tidAutoStart = 0;
        attempts = 0;
        autoStartConfigs.clear();
        return;
    }

    attempts++;

    QMutableListIterator<QString> iter( autoStartConfigs );
    while ( iter.hasNext() ) {
        QString config = iter.next();
        QPointer<QtopiaNetworkInterface> iface = QtopiaNetwork::loadPlugin( config );
        if ( !iface ) {
            iter.remove();
            continue;
        }
        
        
        //updateNetwork() has been called at least once already
        //hence we can assume that the interface has been initialized
        if ( iface->status() == QtopiaNetworkInterface::Down ) {
            qLog(Network) << "Initiate automatic start of" << config;
            StartCode code = activateInterface( qApp->applicationName(), config );
            //automatically started interfaces always have extendedLifeTime
            if ( code == Success )
                sessionManager->setExtendedLifetime( config.toLatin1().constData(), true );
            if ( code != NotReady )
                iter.remove();
        } else if ( iface->status() == QtopiaNetworkInterface::Up )
        {
            qLog(Network) << "Automatic start not necessary for" << config;
            iter.remove();
        }
    }
}

QTOPIA_TASK(QtopiaNetworkServer,QtopiaNetworkServer);
QTOPIA_TASK_PROVIDES(QtopiaNetworkServer, SystemShutdownHandler);

#include "networkserver.moc"
