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

#include "qtopianetwork.h"
#include "qtopianetworkinterface.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSettings>

#include <qvaluespace.h>
#include <qpluginmanager.h>
#include <qtopiaipcadaptor.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

/*!
  \class QtopiaNetwork
    \inpublicgroup QtBaseModule

  \brief The QtopiaNetwork class provides functions for starting/stopping of network devices/interfaces.

  In general it allows the managing of existing network configurations.
  The management functions include the starting and stopping of interfaces and
  other functions which may be useful for applications which want to influence the
  connectivity state of Qtopia.

  The QtopiaNetworkServer synchronizes network interface operations across all Qt Extended applications. Client applications can request network operations via the static methods
  provided by this class. The following functions are thin wrapper functions encapsulating
  IPC calls to the QtopiaNetworkServer which then acts on behalf of the caller:

  \list
    \o startInterface()
    \o stopInterface()
    \o setDefaultGateway()
    \o unsetDefaultGateway()
    \o shutdown()
  \endlist

  The second set of static functions relates to the management of network configuration
  files. By default Qt Extended saves network configurations in the directory returned by
  settingsDir(). Network interfaces are identified via a handle which is the absolute path
  and name of the network configuration file containing the parameters
  for that interface. The network server will choose and load the appropriate network plug-in
  based on the configuration handle. QNetworkState::deviceType() can be used to
  determine the type of a network handle. In order to obtain a list of network handles of
  a particular type availableNetworkConfigs() should be used.

  loadPlugin() finds and loads the Qt Extended network plug-in that is suitable for a given
  network handle. However to avoid multiple access to the same network interface the returned
  QtopiaNetworkInterface instance should be carefully managed. The most common reason
  for instanciating a QtopiaNetworkInterface from outside of the network server would be
  the need to allow the user to configure the interface via
  QtopiaNetworkInterface::configuration(). The internet application would be an example
  for such a use case.

  QtopiaNetwork only provides a minimum level of monitoring functionality via online().
  If more detailed monitoring is required one of the following classes should be used:

  \list
    \o QNetworkState - very generic connectivity information related to Qtopia
    \o QNetworkDevice - specific information about the state of a particular hardware device
    \o QNetworkConnection - information beyond the device level such as the remote partner
  \endlist

  \sa QtopiaNetworkServer
  \ingroup io
*/

/*!
  \enum QtopiaNetwork::TypeFlag

  This enum is used to describe the type of a network connection.

  \value LAN Ethernet based network.
  \value WirelessLAN Wireless network.
  \value Dialup The connection is established via a dial-up connection.
  \value GPRS The connection is established via GPRS/UMTS/EDGE.
  \value Bluetooth The network is based on Bluetooth.
  \value Hidden This interface is hidden from the user.
  \value Custom This network plugin must exactly match the network configuration. For more details
                see QtopiaNetworkFactoryIface::customID().
  \value Any A place holder for any arbitrary network type.

  These sub types are used in conjunction with QtopiaNetwork::GPRS and
  QtopiaNetwork::Dialup only:

  \value PhoneModem The network connection is established via the internal phone
             modem (this flag is defined for \l{QtCellModule}{Cell} module only).
  \value NamedModem A serial network connection using a specific device
             such as /dev/ttyS0 is established. The device name is given in the
             configuration file.

  These sub types specify the type of the external device which is used to
  establish the connection.

   \value PCMCIA The network device is an attached PCMCIA card.

  These sub types are used in conjunction with QtopiaNetwork::Bluetooth only.

  \value BluetoothDUN The network connection is established via a local bluetooth
            device. The remote Bluetooth device acts as Internet gateway as specified
            by the Dial-up Networking Profile (DNP).
  \value BluetoothPAN This value is reserved for future use in Qtopia.

*/

/*!
  Starts that the network interface identified by \a handle. If the interface is running already
  this function does nothing.

  \a options is used internally by some network plug-ins.
  It allows to specify additional parameters which are specific to particular network plug-ins.
  Otherwise this parameter can safely be ignored.
*/
void QtopiaNetwork::startInterface( const QString& handle, const QVariant& options )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(startInterface(const QString&,const QString&,const QVariant&)),
            qApp ? qApp->applicationName() : QString(), handle, options );
}

/*!
  Stops the network interface identified by \a handle. If \a deleteIface is true
  the server will delete all configuration information associated to this interface.
*/
void QtopiaNetwork::stopInterface( const QString& handle, bool deleteIface)
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(stopInterface(const QString&,const QString&,bool)),
            qApp ? qApp->applicationName() : QString(), handle, deleteIface );
}

/*!
  \internal

  This function allows the stopping of network connection w/o considering other applications
  which might have an interest in the same connection. This is achieved by circumventing
  the network session manager (and thus ignoring the extended life time flag). In general
  this should not be used by applications other than the global network manager. The Qt Extended
  netsetup application uses this function to allow the user to stop a network connection
  which could have been started by rogue applications running in the background.
  */
void QtopiaNetwork::privilegedInterfaceStop( const QString& handle )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(privilegedInterfaceStop(const QString&)), handle );
}

/*!
  Stops all active network interfaces.
*/
void QtopiaNetwork::shutdown()
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(shutdownNetwork()) );
}

/*!
  The network interface identified by \a handle becomes the new default gateway. If the
  device is connected to more than one network at a time this can be used to
  choose a preferred network for data transfers.
*/
void QtopiaNetwork::setDefaultGateway( const QString& handle )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setDefaultGateway(const QString&,bool)), handle, false );
}

/*!
  Qt Extended will set the default gateway to an interface that
  is not equal to \a handle. This is useful if \a handle goes offline and the
  caller doesn't care what interface should become the new default gateway.
  */
void QtopiaNetwork::unsetDefaultGateway( const QString& handle )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setDefaultGateway(const QString&,bool)), handle, true );
}

/*!
  \internal

  If \a isLocked is true the network server initiates shutdown() and prevents
  new network connections from being established.
  */
void QtopiaNetwork::lockdown( bool isLocked )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setLockMode(bool)), isLocked );
}

/*!
  \internal

  Sets the life time of the interface \a handle to \a isExtended. If \a isExtended is \c true
  \a handle will remain online indefinitly. Usually the session manager would keep track of interfaces
  started by applications and shuts the device down  when the application quits. However the life time flag
  prevents the session manager from doing so.

  The extended life time flag can only be set if \a handle (the associated network device) is in the state
  QtopiaNetworkInterface::Up, QtopiaNetworkInterfaces::Demand or QtopiaNetworkInterfaces::Pending. The flag
  immediately resets as soon as the device leaves one of these three states.

  Currently this function can only be called by the Qt Extended server and the netsetup application (enforced by SXE).
  */
void QtopiaNetwork::extendInterfaceLifetime( const QString& handle, bool isExtended )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setExtendedInterfaceLifetime(const QString&,bool)), handle, isExtended );
}

/*!
  Returns the default directory for network interface configurations;
  \c{$HOME/Applications/Network/config}.
*/
QString QtopiaNetwork::settingsDir()
{
    return Qtopia::applicationFileName("Network", "config");
}

/*!
  \internal
  Returns the type of network interface defined by \a handle.
*/
QtopiaNetwork::Type QtopiaNetwork::toType(const QString& handle)
{
    QSettings cfg( handle, QSettings::IniFormat );
    cfg.sync();
    const QString tp = cfg.value("Info/Type").toString();

    QtopiaNetwork::Type t;
    if ( tp == "dialup" )
    {
        if ( cfg.value("Serial/Type").toString() == "external" )
        {
            if ( cfg.value("Serial/SerialDevice").toString().isEmpty() )
                t |= QtopiaNetwork::PCMCIA;
            else
                t |= QtopiaNetwork::NamedModem;
        }
#ifdef QTOPIA_CELL
        else
        {
            t |= QtopiaNetwork::PhoneModem;
        }
#endif
        if ( cfg.value("Serial/GPRS").toString() == "y" )
            t |= QtopiaNetwork::GPRS;
        else
            t |= QtopiaNetwork::Dialup;
    }
    else if ( tp == "lan" )
    {
        t |= QtopiaNetwork::LAN;
    }
    else if ( tp == "wlan" )
    {
        t |= QtopiaNetwork::WirelessLAN;
    }
    else if ( tp == "pcmcialan" )
    {
        t |= QtopiaNetwork::PCMCIA;
        t |= QtopiaNetwork::LAN;
    }
    else if ( tp == "pcmciawlan" )
    {
        t |= QtopiaNetwork::PCMCIA;
        t |= QtopiaNetwork::WirelessLAN;
    }
    else if ( tp == "bluetooth" )
    {
        t |= QtopiaNetwork::Bluetooth;
        QByteArray a = cfg.value("Bluetooth/Profile").toByteArray();
        if ( a == "DUN" )
            t |= QtopiaNetwork::BluetoothDUN;
        else if ( a == "PAN" )
            t |= QtopiaNetwork::BluetoothPAN;
    }

    if ( cfg.value("Info/Visibility").toByteArray() == "hidden" ) {
        t |= QtopiaNetwork::Hidden;
    }

    if ( !cfg.value("Info/CustomID").toByteArray().isEmpty() )
        t |= QtopiaNetwork::Custom;

    return t;
}

/*!
  Returns a list of all known Qt Extended network interface handles which match \a type.
  A handle uniquely identifies a Qt Extended network interfaces and is the absolute
  path to the configuration file that contains the settings for a particular
  QtopiaNetworkInterface. \a path specifies the directory where the lookup takes place.

  Note that the lookup is independent of the state of an interface. It always returns all
  Qt Extended network interfaces which are online and offline.

  If \a type is set to \c{QtopiaNetwork::Any} it returns all known NetworkInterfaces. If \a path is
  empty the lookup will take place in settingsDir().
*/
QStringList QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::Type type,
        const QString& path)
{
    QStringList resultList;

    QString fileName = path;
    if ( path.isEmpty() )
        fileName = settingsDir();

    //qLog(Network) << "QN: Searching for configs in " << fileName;
    QDir configDir(fileName);
    if(!configDir.exists())
        configDir.mkdir(fileName);

    QStringList files = configDir.entryList(QStringList("*.conf"));
    foreach(QString entry, files)
    {
        entry = configDir.filePath(entry);
        QtopiaNetwork::Type configType = toType( entry );
        if ( ( configType & type) == type )
            resultList.append( entry );
    }

    if ( resultList.isEmpty() )
        qLog(Network) << "QN: no configuration available";
    else
        qLog(Network) << "QN: Found " << resultList;
    return resultList;
}

static QByteArray customID4Config( const QString& handle )
{
    QSettings cfg( handle, QSettings::IniFormat );
    cfg.sync();
    return cfg.value("Info/CustomID", QByteArray()).toByteArray();
}

static QHash<QString,QPointer<QtopiaNetworkInterface> > *loadedIfaces = 0;
static QHash<int,QtopiaNetworkFactoryIface*> *knownPlugins = 0;
static QPluginManager *pmanager = 0;

static void cleanup()
{
    if (loadedIfaces)
        delete loadedIfaces;
    if (knownPlugins)
        delete knownPlugins;
    if (pmanager)
        delete pmanager;
    loadedIfaces = 0;
    knownPlugins = 0;
    pmanager = 0;
}


/*!
     Loads the appropriate network plug-in for the interface with \a handle; or
     0 if no suitable plug-in can be found.

     Only use this function if you intend to configure the returned QtopiaNetworkInterface
     via QtopiaNetworkInterface::configuration(). Any other use may trigger
     undesired behaviour whereby the loaded network interface instance may
     overshadow the instance created by the QtopiaNetworkServer. For more details
     see the QtopiaNetworkInterface general class documentation.

*/
QPointer<QtopiaNetworkInterface> QtopiaNetwork::loadPlugin( const QString& handle)
{
    if ( handle.isEmpty() || !QFile::exists(handle) ) {
        qLog(Network) << "QN::loadPlugin(): Invalid settings file passed" << handle;
        return 0;
    }

#ifndef QT_NO_COMPONENT
    QtopiaNetworkFactoryIface *plugin = 0;
    QPointer<QtopiaNetworkInterface> impl = 0;

    if ( !loadedIfaces ) {
        loadedIfaces = new QHash<QString, QPointer<QtopiaNetworkInterface> >;
        qAddPostRoutine(cleanup);
    }
    if ( !knownPlugins )
        knownPlugins = new QHash<int,QtopiaNetworkFactoryIface*>;
    if ( !pmanager )
        pmanager = new QPluginManager( "network" ); //no tr

    //check that we do not already have an instance for this config
    QHash<QString, QPointer<QtopiaNetworkInterface> >::const_iterator iter =
        loadedIfaces->find( handle );
    if ( iter != loadedIfaces->end() )
    {
        if (iter.value() != 0) {
            impl = iter.value();
            //qLog(Network) << "QN::loadPlugin() : interface already in cache, returning instance";
        } else {
            //qLog(Network) << "QN::loadPlugin() : interface deleted, removing bogus reference";
            loadedIfaces->remove( handle ); //instance has been deleted
        }
    }

    const QByteArray customID = customID4Config( handle );
    if ( !impl )
    {
        Type t = toType(handle);
        //go through list of all known plugins
        int found = 0;
        foreach ( int key, knownPlugins->keys() )
        {
            if ( (key & t)==t ) {
                if ( (t & QtopiaNetwork::Custom) ) {
                    QByteArray id = knownPlugins->value( key )->customID();
                    if (  id == customID )
                    {
                        found = key;
                        break;
                    }
                } else if ( key & QtopiaNetwork::Custom ) {
                    continue;
                } else {
                    found = key;
                    break;
                }
            }
        }

        if ( found ) {
            //qLog(Network) << "QN::loadPlugin() : plugin in cache, creating new interface instance";
            plugin = knownPlugins->value(found);
            impl = plugin->network( handle );
            loadedIfaces->insert( handle, impl );
        } else {
            // the interface hasn't been instanciated nor do we have a
            // suitable plug-in yet. Go through all plug-ins
            //qLog(Network) << "QN::loadPlugin() : Extensive search mode";
            QStringList pluginList = pmanager->list();
            foreach(QString item, pluginList)
            {
                QObject *instance = pmanager->instance(item);
                plugin = qobject_cast<QtopiaNetworkFactoryIface*>(instance);
                if ( plugin && ((plugin->type() & t) == t) )
                {
                    if ( t & QtopiaNetwork::Custom ) {
                        if ( plugin->customID() != customID )
                            continue;
                    } else if ( plugin->type() & QtopiaNetwork::Custom ) {
                        continue;
                    }
                    qLog(Network) << "QN::loadPLugin() : plugin found,"
                                  << "loaded and new interface instanciated " ;
                    if ( qLogEnabled(Network) && t & QtopiaNetwork::Custom )
                        qLog(Network) << "Using custom id: " << customID;
                    knownPlugins->insert( plugin->type(), plugin );
                    impl = plugin->network( handle );
                    loadedIfaces->insert( handle, impl );
                    break;
                }
            }
        }
    }
    if (!impl)
        qLog(Network) << "QN::loadPlugin(): no suitable plugin found for ->" << handle << customID;
    return impl;
#else
    return 0;
#endif
}

/*!
    Returns true if any known network interface is online/connected.
    This can be used to check the general connectivity status of the Qt Extended environment.
    A device is considered to be online if it is in the state \c QtopiaNetwork::Up,
    \c QtopiaNetwork::Demand or \c QtopiaNetwork::Pending.

    More detailed state updates on a per device base can be obtained by using QNetworkDevice
    and overal state change notifications across all network devices are provided by QNetworkState.
*/
bool QtopiaNetwork::online()
{
    QValueSpaceItem netSpace( "/Network/Interfaces" );
    const QList<QString> ifaceList = netSpace.subPaths();
    foreach( QString iface, ifaceList ) {
        QtopiaNetworkInterface::Status state =
            (QtopiaNetworkInterface::Status) (netSpace.value( iface+QByteArray("/State"), 0 )).toInt();
        switch (state) {
            case QtopiaNetworkInterface::Up:
            case QtopiaNetworkInterface::Demand:
            case QtopiaNetworkInterface::Pending:
                return true;
                break;
            default:
                break;
        }
    }
    return false;
}
