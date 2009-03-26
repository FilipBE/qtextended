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

#include "qnetworkconnection.h"

#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <qtopialog.h>
#include <QSettings>
#include <QStringList>
#include <QValueSpaceItem>

#include <QTranslatableSettings>
#include "qnetworkdevice.h"
#include "qwlanregistration.h"
#include "qnetworkstate.h"

/*!
  \class QNetworkConnection::Identity
    \inpublicgroup QtBaseModule
  \brief The Identity class provides an identifier for a network connection.


  As a physical network device can potentially
  connect to a variety of remote networks (usually just determined by a set of
  configuration parameter such as a WLAN ESSID), QNetworkConnection::Identity
  allows to uniquely distinguish network connections
  in a device independent manor.

  Most network connections such as LAN and all types of
  dial-up connections do not support more than one network connection per network device.
  The two types of network devices that support multiple network connections are:

  \list
    \o WLAN
    \o Bluetooth PAN (not supported iby Qt Extended yet)
  \endlist

  Each network connection has a user visible name(), is associated to a deviceHandle()
  which identifies the network device and a type(). The following example deomstrates the
  most common way how network identities are obtain:

  \code
        QNetworkConnectionManager manager;
        QNetworkConnection::Identities idents = manager.connections();
        QNetworkDevice dev( idents.at(0) ); //pick first
        QtopiaNetworkInterface::Status state = dev.state();
  \endcode

  For more information about the Qt Extended network API see \l QNetworkDevice.

  \ingroup io
  \sa QNetworkConnection
*/

/*!
  Creates an invalid identity.
*/
QNetworkConnection::Identity::Identity()
    : vNetId( QUuid() ), devHandle( QLatin1String("") )
{
}

/*!
  \internal
  Constructs an identity with the given \a id and \a handle. This
  is how QNetworkConnectionManager creates new identities.
 */
QNetworkConnection::Identity::Identity( const QString& handle, const QUuid& id )
  :  vNetId( id ), devHandle( handle )
{
}

/*!
  Constructs a copy of \a other.
  */
QNetworkConnection::Identity::Identity( const Identity& other )
    : vNetId( other.vNetId ), devHandle( other.devHandle )
{
}

/*!
  Destroys this QNetworkConnection::Identity instance.
  */
QNetworkConnection::Identity::~Identity()
{
}

/*!
  Assings \a other to this
  */
QNetworkConnection::Identity& QNetworkConnection::Identity::operator=(const Identity& other)
{
    vNetId = other.vNetId;
    devHandle = other.devHandle;
    return (*this);
}

/*!
  Returns true if this identityis equal to \a other. Equality means all
  the fields are equivalent.
  */
bool QNetworkConnection::Identity::operator==(const Identity& other) const
{
    return vNetId == other.vNetId
            && devHandle == other.devHandle;
}

/*!
  Returns true if this identityis is not equal to \a other. Equality means all
  the fields are equivalent.
  */
bool QNetworkConnection::Identity::operator!=(const Identity& other) const
{
    return !( *this == other );
}

/*!
  Returns the handle for this network connection. The handle allows the identification of
  the network device associated to this identity.

  Note: Several identities can have the same handle such as wireless LAN or Bluetooth PAN
  connections which can be managed by the same network device. This means
  that no two network connections with the same identity handle can be
  online/connected at the same time.
  */
QString QNetworkConnection::Identity::deviceHandle() const
{
    return devHandle;
}

/*!
  Returns true if this Identity object can be mapped to a valid network configuration.

  A network connection identity becomes invalid when the associated network
  configuration is deleted.

  Note: note The network connection identity remains the same if the configuration has been
  edited only. This means that the actual target network and/or the name
  (as returned by \l QNetworkConnection::Identity::name() ) may change during the life time
  of this object.
*/
bool QNetworkConnection::Identity::isValid() const
{
    if ( devHandle.isEmpty() || !QFile::exists( devHandle ) ) {
        devHandle = "";
        return false;
    }

    //devices which support multiple connections must have a valid uuid
    QtopiaNetwork::Type type = QNetworkState::deviceType( devHandle );
    if ( (type & QtopiaNetwork::WirelessLAN) || (type & QtopiaNetwork::BluetoothPAN) ) {
        if ( vNetId.isNull() ) {//once the identity is invalid it remains so
            return false;
            devHandle = "";
        }

        QSettings s( devHandle, QSettings::IniFormat );
        if ( s.status() != QSettings::NoError )
            return false;

        int size = 0;
        if ( type & QtopiaNetwork::BluetoothPAN )
            size = s.beginReadArray( QLatin1String("BluetoothNetworks") );
        else
            size = s.beginReadArray( QLatin1String("WirelessNetworks") );

        for (int i = 0; i< size; i++ ) {
            s.setArrayIndex( i );
            QString uuid = s.value( QLatin1String("Uuid"), QString() ).toString();
            if ( vNetId.toString() == uuid )
                return true;
        }
        s.endArray();
        vNetId = QUuid(); //this connection doesn't exist anymore
        devHandle = "";

        return false;
    } else {
        return true;
    }
}

/*!
  Returns the user visibile name of the network connection. The returned
  name is empty if this identity is invalid.

  \sa isValid()
  */
QString QNetworkConnection::Identity::name() const
{
    if ( !isValid() ) {
        return QString();
    }

    QTranslatableSettings cfg( devHandle, QSettings::IniFormat );
    if ( cfg.status() != QSettings::NoError )
        return QString();

    QtopiaNetwork::Type t = type();
    if ( (t & QtopiaNetwork::WirelessLAN) || (t & QtopiaNetwork::BluetoothPAN)  ) {
        int size = 0;
        if ( t & QtopiaNetwork::BluetoothPAN )
            size = cfg.beginReadArray( QLatin1String( "BluetoothNetworks" ) );
        else
            size = cfg.beginReadArray( QLatin1String( "WirelessNetworks" ) );

        for ( int i = 0; i<size; i++ )
        {
            cfg.setArrayIndex( i );
            if ( vNetId.toString() == cfg.value( QLatin1String("Uuid") ).toString() ) {
                if ( t & QtopiaNetwork::BluetoothPAN )
                    return cfg.value( QLatin1String("PANNetName") ).toString();
                else
                    return tr("WLAN: %1", "%1 name of WLAN ESSID").arg(
                            cfg.value( QLatin1String("ESSID") ).toString() );
            }
        }
        cfg.endArray();
    } else {
        return cfg.value( "Info/Name" ).toString();
    }

    return QString();
}

/*!
  Returns the network connection type. An invalid identity returns QtopiaNetwork::Any;
  */
QtopiaNetwork::Type QNetworkConnection::Identity::type() const
{
    if ( !isValid() )
        return QtopiaNetwork::Any;

    return QNetworkState::deviceType( devHandle );
}

/*!
  \typedef  QNetworkConnection::Identities

  This is a convenience typedef to encapsulate a list of QNetworkConnection::Identity objects.
  The exact type is:

  \code
    QList<QNetwork::Identity>
  \endcode

  */

class QNetworkConnectionPrivate {
public:
    QNetworkConnectionPrivate()
        : lastPanNetwork( -1 )
    {
        device = 0;
        lastState = QtopiaNetworkInterface::Unknown;
    }

    ~QNetworkConnectionPrivate()
    {
        if ( device )
            delete device;
    }

    bool isConnected() const
    {
        if ( !identity.isValid()/* || !d->device*/ )
            return false;

        QtopiaNetwork::Type type = identity.type();
        if ( type & QtopiaNetwork::WirelessLAN ) {
            return ( lastState == QtopiaNetworkInterface::Up && !lastEssid.isEmpty() );
        } else if ( type & QtopiaNetwork::BluetoothPAN ) {
            return ( lastState == QtopiaNetworkInterface::Up  && lastPanNetwork > 0);
        } else {
            return (lastState == QtopiaNetworkInterface::Up );
        }
    }

#ifndef NO_WIRELESS_LAN
    /*
       Return null if the identity associated to this connection
       is not of type WirelessLAN and/or is invalid. Otherwise returns
       the essid of identity.
       */
    QString essidForIdentity() const
    {
        if ( !identity.isValid() )
            return QString();

        if ( identity.type() & QtopiaNetwork::WirelessLAN ) {
            QSettings cfg( identity.devHandle, QSettings::IniFormat );
            if ( cfg.status() != QSettings::NoError )
                return QString();
            int size = cfg.beginReadArray( QLatin1String( "WirelessNetworks" ) );
            for ( int i = 0; i<size; i++ )
            {
                cfg.setArrayIndex( i );
                if ( identity.vNetId.toString() == cfg.value( QLatin1String("Uuid") ).toString() )
                    return cfg.value( QLatin1String("ESSID") ).toString();
            }
            cfg.endArray();
        }

        return QString();
    }
#endif

#ifdef QTOPIA_BLUETOOTH
    /*
       Return -1 if the identity associated to this connection
       is not of type BluetoothPAN or is invalid.

       Otherwise returns the index of the Bluetooth network that this device is
       connected to.
       */
    int panNetworkForIdentity() const
    {
        if ( !identity.isValid() )
            return -1;

        if ( identity.type() & QtopiaNetwork::BluetoothPAN ) {
            QSettings cfg( identity.devHandle, QSettings::IniFormat );
            if ( cfg.status() != QSettings::NoError )
                return -1;
            int size = cfg.beginReadArray( QLatin1String( "BluetoothNetworks" ) );
            for ( int i = 0; i<size; i++ )
            {
                cfg.setArrayIndex( i );
                if ( identity.vNetId.toString() == cfg.value( QLatin1String("Uuid") ).toString() )
                    return i+1;
            }
            cfg.endArray();
        }
        return -1;
    }
#endif

    void _q_deviceStateChanged( QtopiaNetworkInterface::Status s, bool /*error*/ )
    {
        QtopiaNetworkInterface::Status oldState = lastState;
        lastState = s;
        switch( s ) {
            case QtopiaNetworkInterface::Unknown:
                if ( !identity.isValid() ) {
                    //this state change was triggered by removal of this connection;
                    if ( device ){
                        delete device;
                        device = 0;
                    }
                }
                break;
            case QtopiaNetworkInterface::Unavailable:
            case QtopiaNetworkInterface::Down:
            case QtopiaNetworkInterface::Pending:
            case QtopiaNetworkInterface::Demand:
                {
                    if ( oldState == QtopiaNetworkInterface::Up ) {
                        if ( identity.type() & QtopiaNetwork::WirelessLAN ) {
#ifndef NO_WIRELESS_LAN
                            if ( !lastEssid.isEmpty() ) {
                                lastEssid = QString();
                                emit q->connectivityChanged( false );
                            }
#endif
                        } else if ( identity.type() & QtopiaNetwork::BluetoothPAN ) {
#ifdef QTOPIA_BLUETOOTH
                            if ( lastPanNetwork > 0 ) {
                                lastPanNetwork = -1;
                                emit q->connectivityChanged( false );
                            }
#endif
                        } else {
                            emit q->connectivityChanged( false );
                        }
                    }
                }
                break;
            case QtopiaNetworkInterface::Up:
                if ( oldState != QtopiaNetworkInterface::Up ) {
                    if ( identity.type() & QtopiaNetwork::WirelessLAN ) {
#ifndef NO_WIRELESS_LAN
                        QWlanRegistration wlanReg( QString::number(qHash(identity.devHandle) ) );
                        if ( wlanReg.currentESSID() == essidForIdentity() )
                        {
                            lastEssid = wlanReg.currentESSID();
                            emit q->connectivityChanged( true );
                        }
#endif
                    } else if ( identity.type() & QtopiaNetwork::BluetoothPAN ) {
#ifdef QTOPIA_BLUETOOTH
                        QValueSpaceItem item( "/Network/Interfaces/"+QByteArray::number(qHash(identity.deviceHandle())) );
                        int currentPANIndex = item.value( QLatin1String("ConnectedPartnerNetwork"), -1 ).toInt();
                        if ( currentPANIndex == panNetworkForIdentity() )
                        {
                            lastPanNetwork = currentPANIndex;
                            emit q->connectivityChanged( true );
                        }
#endif
                    } else {
                        emit q->connectivityChanged( true );
                    }
                }
                break;
            default:
                qWarning("Unknown network device state.");
        }
    }

    QNetworkConnection::Identity identity;
    QNetworkDevice* device;
    QtopiaNetworkInterface::Status lastState;
    QString lastEssid;
    int lastPanNetwork;

    QNetworkConnection* q;
};



/*!
  \class QNetworkConnection
    \inpublicgroup QtBaseModule

  \brief The QNetworkConnection class provides monitoring of network connections.

  Each QNetworkDevice can only be connected to one remote network at a time.
  However some devices can potentially connect to different remote networks without
  the need to reconfigure. An example are WLAN devices which maintain an internal
  list of known WLAN access points. QNetworkConnection exposes the connection
  state based on the connection partner. Thus a single WLAN QNetworkDevice may actually
  have several QNetworkConnections with distinct identities associated to it .
  Consequently if the QNetworkDevice is connected only one associated QNetworkConnection
  is online. Use QNetworkConnection::Identity to distinguish network connections.

  In order to keep track of network connections the QNetworkConnectionManager should be used.

  \code
    QNetworkConnectionManager manager;
    QNetworkConnection::Identities idents = manager.connections();
    QNetworkConnection connection( idents.at(0) ); //just pick the first

  \endcode

  The validity of a QNetworkConnection object depends on the validity of its network
  connection identity. The validity can be checked with isValid(). Once a connection is invalid
  it can never become valid anymore. This can occur if the user deletes a network
  interface or he removes a WLAN access point configuration. It is then required to
  reacquire a new identity via QNetworkConnectionManager::connections().

  \ingroup io
  \sa QNetworkConnectionManager
*/

/*!
  Constrcuts a new QNetworkConnection based on \a ident with the given \a parent.
  References to Network identities can be obtained via QNetworkConnectionManager.
  */
QNetworkConnection::QNetworkConnection( const Identity& ident, QObject* parent )
    : QObject( parent )
{
    d = new QNetworkConnectionPrivate();
    d->q = this;
    d->identity = ident;

    if ( !ident.isValid() )
        return;

    d->device = new QNetworkDevice( ident.deviceHandle(), this );
    connect( d->device, SIGNAL(stateChanged(QtopiaNetworkInterface::Status,bool)),
            this, SLOT(_q_deviceStateChanged(QtopiaNetworkInterface::Status,bool)) );
    d->_q_deviceStateChanged( d->device->state(), false );
}

/*!
  Destroys a QNetworkConnection instance.
  */
QNetworkConnection::~QNetworkConnection()
{
    delete d;
}

/*!
  Returns the idenity object associated to this network connection. The identity object
  provides further details about this network connection object.
  */
QNetworkConnection::Identity QNetworkConnection::identity() const
{
    return d->identity;
}

/*!
  \fn void QNetworkConnection::connectivityChanged( bool isConnected )

  This signal is emitted when the connection changes from offline to online and vice versa.

  \a isConnected  is \c {true} when the associated network device's state is equal to QtopiaNetworkInterface::Up.
  If it is required to keep track of the precise device state QNetworkDevice should be used.
  */

/*!
  This function returns true if the connection is established.
  A connection is considered to be established if the underlying device
  state is equal to QtopiaNetworkInterface::Up.

  If it is necessary to distinguish the various types of connectivity \l QNetworkDevice::state()
  should be used.
*/
bool QNetworkConnection::isConnected() const
{
    return d->isConnected();
}

/*!
  Returns \c true if this network connection is valid; otherwise \c false.

  An invalid QNetworkConnection has an invalid identity.
  This may happen when initialising this object with an invalid
  identity or when the underlying network configuration has been deleted (e.g.
  the user removed the configuration for WLAN with ESSID "xyz").

  A transition from a valid to an invalid state can be noticed by
  connecting to the \l QNetworkConnectionManager::connectionRemoved() signal.
*/
bool QNetworkConnection::isValid() const
{
    return d->identity.isValid();
}


class QNetworkConnectionManagerPrivate
{
public:
    QNetworkConnectionManagerPrivate( QNetworkConnectionManager* parent )
        : firstUpdate( true )
    {
        q = parent;
        fsWatcher = new QFileSystemWatcher( QStringList() << QtopiaNetwork::settingsDir() );
        QObject::connect( fsWatcher, SIGNAL(directoryChanged(QString)),
                q, SLOT(_q_accountsAddedRemoved()));
        QObject::connect( fsWatcher, SIGNAL(fileChanged(QString)),
                q, SLOT(_q_accountChanged(QString)));

        //initial update for all device
        _q_accountsAddedRemoved( );
        firstUpdate = false;
    }

    ~QNetworkConnectionManagerPrivate()
    {
        delete fsWatcher;
    }

    void _q_accountChanged( const QString& path )
    {
        //network devices which support more than one connection
        //requires check for changes in config files
        bool emitConnectionAdded = false;
        bool emitConnectionRemoved = false;

        QtopiaNetwork::Type type = QNetworkState::deviceType( path );
        if ( !QFile::exists( path ) )
        {
            //this device cfg has just been removed
            if ( knownConnections.contains( path ) ) {
                emitConnectionRemoved = true;
                knownConnections.remove( path );
            }
        }
        else if ( (type & QtopiaNetwork::WirelessLAN) || (type & QtopiaNetwork::BluetoothPAN) )
        {
            //multi connection device
            QList<QString> knownUuids = knownConnections.values( path );
            knownConnections.remove( path );//remove for now -> read again later
            QStringList newUuids;
            QSettings s( path, QSettings::IniFormat );
            int size = 0;
            if ( type & QtopiaNetwork::BluetoothPAN )
                size = s.beginReadArray( "BluetoothNetworks" );
            else
                size = s.beginReadArray( "WirelessNetworks" );

            for ( int i = 0; i<size; i++ ) {
                s.setArrayIndex( i );
                QString uid = s.value( QLatin1String("Uuid") ).toString();
                if ( uid.isEmpty() ) {
                    qWarning("Network without uuid in %s (index %d)", path.toLatin1().constData(), i );
                    continue;
                }

                newUuids.append( uid );
                if ( knownUuids.contains( uid ) )
                    knownUuids.removeAll( uid );
                else
                    emitConnectionAdded = true;
            }

            if ( !knownUuids.isEmpty() ) {
                emitConnectionRemoved = true;
            }
            for ( int i = 0; i < newUuids.count(); i++ )
                knownConnections.insert( path, newUuids.at(i) );
        }
        else
        {
            //device with one connection only has been edited
            if ( !knownConnections.contains( path ) ) {
                emitConnectionAdded = true;
                knownConnections.replace( path, QString() );
            } // else { //do nothing }
        }

        if ( emitConnectionAdded ) {
           emit q->connectionAdded();
        }
        if ( emitConnectionRemoved ) {
           emit q->connectionRemoved();
        }
    }

    void _q_accountsAddedRemoved( /*const QString& path*/ )
    {
        const QStringList configs = QtopiaNetwork::availableNetworkConfigs();
        QStringList devHandles = knownConnections.keys();
        bool emitConnectionAdded = false;
        bool emitConnectionRemoved = false;

        foreach( QString cfg, configs ) {
            if ( devHandles.contains( cfg ) ) {
                //already seen this device
                devHandles.removeAll( cfg );
            } else {
                //new device
                fsWatcher->addPath( cfg );
                QtopiaNetwork::Type type = QNetworkState::deviceType( cfg );
                if ( (type & QtopiaNetwork::WirelessLAN) || (type & QtopiaNetwork::BluetoothPAN) ) {
                    //multi-connection device
                    QSettings s(cfg, QSettings::IniFormat);
                    if ( s.status() == QSettings::NoError ) {
                        int size = 0;
                        if ( type & QtopiaNetwork::BluetoothPAN )
                            size = s.beginReadArray( "BluetoothNetworks" );
                        else
                            size = s.beginReadArray( "WirelessNetworks" );
                        for (int i=0; i< size; i++ )
                        {
                            s.setArrayIndex( i );
                            knownConnections.insert( cfg, s.value( QLatin1String("Uuid") ).toString() );
                            emitConnectionAdded = true;
                        }
                    }
                } else {
                    //single connection device
                    knownConnections.replace( cfg, QString() );
                    emitConnectionAdded = true;
                }
            }
        }

        //remove deleted devices
        for ( int h=0; h<devHandles.count(); h++ )
        {
            fsWatcher->removePath( devHandles[h] );
            knownConnections.remove( devHandles[h] );
            emitConnectionRemoved = true;
        }

        if ( firstUpdate ) {
            return;
        }

        if ( emitConnectionAdded ) {
            emit q->connectionAdded();
        }
        if ( emitConnectionRemoved ) {
            emit q->connectionRemoved();
        }
    }

private:
    QNetworkConnectionManager* q;
    QFileSystemWatcher* fsWatcher;
    bool firstUpdate;
    QMultiHash<QString,QString> knownConnections;
};

/*!
  \class QNetworkConnectionManager
    \inpublicgroup QtBaseModule

  \brief The QNetworkConnectionManager class allows applications to receive notifications
  when network connections are added and/or removed from Qtopia.

  The QNetworkConnectionManager acts as a factory for new QNetworkConnection::Identity
  object. These object can then be used to construct QNetworkConnection objects.


  The connectionAdded() and connectionRemoved() signals are emitted when the user adds or
  removes network connections. New connections are usually added by configuring new
  network interfaces or adding access point parameter to an already existing WLAN device.

  \code
    QNetworkConnectionManager* manager = new QNetworkConnectionManager();
    connect( manager, SIGNAL(connectionAdded()),
                this, SLOT(newConnection()) );
  \endcode

  \ingroup io
  \sa QNetworkConnection
*/


/*!
  Constructs a new QNetworkConnectionManager with the specified \a parent.
  */
QNetworkConnectionManager::QNetworkConnectionManager( QObject* parent )
    : QObject( parent )
{
    d = new QNetworkConnectionManagerPrivate( this );
}

/*!
  Destroys the QNetworkConnectionManager instance.
  */
QNetworkConnectionManager::~QNetworkConnectionManager()
{
}

/*!
  \fn void QNetworkConnectionManager::connectionAdded()

  Emitted whenever a new network connection is added to Qtopia. The most likely reason for
  a new connection is a new network configuration created by the user.
  */

/*!
  \fn void QNetworkConnectionManager::connectionRemoved()

  Emitted whenever an existing network connection was removed. This is usually caused by
  the user who deleted a network configuration. Any QNetworkConnection object which was based
  on the deleted configuration becomes invalid.
  */

/*!
  Returns a list of all network connection identities known to Qtopia. These identities
  can be used to create QNetworkConnection instances.
  */
QNetworkConnection::Identities QNetworkConnectionManager::connections()
{
    QNetworkConnection::Identities result;
    QStringList configs = QtopiaNetwork::availableNetworkConfigs();
    foreach( QString cfg, configs )
    {
        QtopiaNetwork::Type type = QNetworkState::deviceType( cfg );
        if ( (type & QtopiaNetwork::WirelessLAN) | (type & QtopiaNetwork::BluetoothPAN) ) {
#if defined (QTOPIA_BLUETOOTH) || !defined (NO_WIRELESS_LAN)
            QSettings s( cfg, QSettings::IniFormat );
            int size = 0;
            if ( type & QtopiaNetwork::BluetoothPAN ) {
                size = s.beginReadArray( QLatin1String("BluetoothNetworks") );
            } else {
                size = s.beginReadArray( QLatin1String("WirelessNetworks") );
            }

            for (int i = 0; i< size ; i++ ) {
                s.setArrayIndex( i );
                QUuid uid;
                QString stringId = s.value( QLatin1String("Uuid"), QString() ).toString();
                if ( !stringId.isEmpty() )
                    uid = QUuid( stringId );
                QNetworkConnection::Identity ident( cfg, uid );
                qLog(Network) << "Found net connection:" << cfg << ident.name();
                result.append( ident );
            }
            s.endArray();
#endif
        } else {
            //all other configurations support one network connections per device only
            QNetworkConnection::Identity ident( cfg, QUuid() );
            qLog(Network) << "Found net connection:" << cfg << ident.name();

            result.append( QNetworkConnection::Identity( ident ) );
        }
    }
    return result;
}

#include "moc_qnetworkconnection.cpp"
