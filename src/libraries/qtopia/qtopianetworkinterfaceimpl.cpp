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

#include <qtopianetworkinterface.h>
#include <qtopialog.h>

/*!
    \class QtopiaNetworkInterface
    \inpublicgroup QtBaseModule

    \brief The QtopiaNetworkInterface class describes the minimum interface that
    a network plug-in must provide.

    The QtopiaNetworkServer interacts with Linux network devices via network
    plug-ins. Each plug-in implements this interface. It allows to start() and stop()
    network devices, changes to the routing table via setDefaultGateway() and provides
    access to the status() and Linux device() name.

    Note that applications should usually not handle QtopiaNetworkInterface instances directly.
    This interface is only of interest to device integrators intending to implement their
    own network plug-ins. Most functions of this interface have a public equivalent which can be used
    by application developer wanting to utilise network facilities:

    \table
    \header
        \o QtopiaNetworkInterface call
        \o Public replacement call
    \row
        \o \c cleanup()
        \o none (internally used by network server only)
    \row
        \o \c configuration()
        \o can be savely called from any Qt Extended application that intends to configure the interface.
    \row
        \o \c device()
        \o use \l QNetworkDevice::interfaceName()
    \row
        \o \c initialize()
        \o none (internally used by network server only)
    \row
        \o \c setDefaultGateway()
        \o use \l QtopiaNetwork::setDefaultGateway() and \l QtopiaNetwork::unsetDefaultGateway()
    \row
        \o \c setProperties()
        \o \c no equivalent available (use configuration())
    \row
        \o \c start()
        \o use \l QtopiaNetwork::startInterface()
    \row
        \o \c status()
        \o use \l QNetworkDevice::state() and QNetworkDevice::stateChanged()
    \row
        \o \c stop()
        \o use \l QtopiaNetwork::stopInterface()
    \row
        \o \c type()
        \o use \l QNetworkState::deviceType()
    \endtable

    Each instance publishes various internal states to the network value space.
    The network value space can be found under \c{/Network/Interfaces/<config hash>}.
    The hash value uniquely identifies each interface and is generated via:

    \code
        QtopiaNetworkInterface* iface = ...
        QString hash = qHash( iface->configuration()->configFile() );

    \endcode

    Each QtopiaNetworkInterface instance must publish the following values:

    \list
        \o \c Config - the configuration file associated to this interface
        \o \c Error - the last QtopiaNetworkInterface::Error value converted to an integer
        \o \c ErrorString - the human-readable string describing the last error
        \o \c NetDevice - the Linux network interface that this interface is managing e.g., eth0
        \o \c State - the current state (QtopiaNetworkInterface::Status enum converted to integer)
    \endlist

    There is no default implementation available for this interface.

    \sa QtopiaNetworkServer, QtopiaNetworkConfiguration
    \ingroup io
*/

/*!
    \enum QtopiaNetworkInterface::Status

    The status provides information about the state of the network interface.

    \value Unknown this is the initial state of a network interface
        that has not been initialized yet (\c{initialize()}). The interface
        must never return to this state after initialization.
    \value Down the interface is ready to be started
    \value Up the interface is up and running
    \value Pending temporary state of the interface
    \value Demand Initiate the link only on demand when data traffic is present
    \value Unavailable the interface cannot be started because
        the network device does not exist and/or cannot be recognised
*/

/*!
    \enum QtopiaNetworkInterface::Error

    This enum identifies the error that occurred.

    \value NoError No error occurred.
    \value NotConnected The device could not connect to the network or in case of
        of a PPP connection to the peer server.
    \value NotInitialized The network plug-in has not been initialized yet. The interface can
        be initialized by calling QtopiaNetworkInterface::initialize().
    \value NotAvailable The device is not available (e.g. the PCMCIA card is not plugged in).
    \value UnknownError An error other than those specified above occurred.
*/

/*!
    \fn bool QtopiaNetworkInterface::start( const QVariant options = QVariant() )

    Starts the network service, returning true on success.
    \a options is used to pass optional parameter to the interface. The actual value of \a options is plug-in specific and
    plug-in developer may use it for any arbitrary value that might be of importance to the startup code.

    Note: This function may only be called by the qpe server. Qt Extended applications should start an interface via
    QtopiaNetwork::startInterface().
*/

/*!
    \fn bool QtopiaNetworkInterface::stop()

    Stops the network service,
    returning true on success.

    Note: This function may only be called by the qpe server. Qt Extended applications should stop a specific interface via
    QtopiaNetwork::stopInterface() or QtopiaNetwork::shutdown()
*/

/*!
    \fn Status QtopiaNetworkInterface::status()

    Updates and returns the current status of the interface. At any given time
    a call to this function should always perform a status check before
    returning the results.

    \sa QtopiaNetworkInterface::Status
*/

/*!
    \fn void QtopiaNetworkInterface::initialize()

    Initializes the network interface configuration.
    The implementation must change the interface state
    to a value that is not equal to QtopiaNetworkInterface::Unknown.
*/

/*!
    \fn void QtopiaNetworkInterface::cleanup()

    Removes all system and config files (e.g., the dialup plug-in deletes
    the peer and connect/disconnect chat file) created for this interface. However this function must not remove the
    network configuration file associated with this plug-in instance as this will be
    done by the network server.
*/

/*!
    \fn bool QtopiaNetworkInterface::setDefaultGateway()

    This interface becomes the default gateway for the device. This function returns false
    if the default gateway could not be set.
*/

/*!
    \fn QString QtopiaNetworkInterface::device() const

    Returns the name of the network device that is used for connections.
    (e.g., eth0, wlan1, ppp0). This name is only valid if the interface
    status is not QtopiaNetworkInterface::Unknown or QtopiaNetworkInterface::Unavailable
*/

/*!
    \fn QtopiaNetwork::Type QtopiaNetworkInterface::type() const

    Returns the type of network interface that this particular interface can handle.
*/

/*!
    \fn QtopiaNetworkConfiguration *QtopiaNetworkInterface::configuration()

    Returns a pointer to allow access to the configuration for this interface.
    This function must not change the internal state of the current network interface instance.
    Its sole purpose is to allow applications to change the underlying configuration.
*/

/*!
    \fn void QtopiaNetworkInterface::setProperties( const QtopiaNetworkProperties& properties)

    The content of \a properties is written to the network configuration for this interface.
*/

/*!
    \internal

    \fn QtopiaNetworkMonitor QtopiaNetworkInterface::monitor()

    This function call is a placeholder for future releases of Qtopia. The returned Qt Extended network monitor
    will perform operations such as traffic and cost monitoring.
*/

/*----------------------------------------------------------*/
// QtopiaNetworkMonitor

/*!
    \internal
    \class QtopiaNetworkMonitor
    \inpublicgroup QtBaseModule

    \brief The QtopiaNetworkMonitor class allows the monitoring of
    data traffic and can calculate the associated costs

    \warning This class merely serves as a placeholder for future
    releases of Qtopia.
*/

/*----------------------------------------------------------*/
// QtopiaNetworkConfiguration

/*!
    \class QtopiaNetworkConfiguration
    \inpublicgroup QtBaseModule

    \brief The QtopiaNetworkConfiguration class defines the interface to
    network interface configurations.

    Network interface parameter are stored in configFile(). They can be edited via a
    combination of writeProperties(),  getProperties() and property() or by calling configure()
    which returns one or more  user interface dialogs. The dialogs depend on
    the various configuration types() supported by the interface.
    The WLAN plug-in e.g., provides two configuration dialogs. The first dialog supports
    general interface parameter whereas the second dialog allows the configuration through
    an WLAN scanner interface.

    There is no default implementation available for this interface.

    \sa QtopiaNetworkInterface
    \ingroup io
*/

/*!
    \fn QString QtopiaNetworkConfiguration::configFile() const

    Returns the absolute path and filename of the network configuration file that defines
    the current interface. The return string can also be used as interface handle.
*/

/*!
    \fn QVariant QtopiaNetworkConfiguration::property(const QString& key) const

    This is a quick way of accessing the configuration value for \a key. The following example
    quickly determines the human-readable name of this interface/configuration:
    \code
        QtopiaNetworkConfiguration* config = ...
        QString value = config->property( "Info/Name" ).toString();
    \endcode
*/

/*!
    \fn QStringList QtopiaNetworkConfiguration::types() const

    Returns a list of all user interfaces. Each list entries are localised strings and can by used as a
    short description for the interface dialogs. The same strings are also used as identifiers
    for configure() to request the user interface.

    The first entry in the list is the default configuration interface.
    The returned list must contain at least one entry (being the default interface).
*/

/*!
    \fn QDialog* QtopiaNetworkConfiguration::configure(QWidget *parent, const QString& type)

    Returns the default configuration dialog that is shown to the user when
    he configures this interface. \a parent is the parent widget for the dialog.
    The valid values to \a type can be obtained by using types().
    If \a type is unknown this function returns 0. The default configuration interface is returned
    if no type is passed.

    \bold{Note:} A plug-in can have additional configuration interfaces which are relevant to
    the plug-in but not essential. For example, the LAN plug-in has an additional user interfaces
    for WLAN scanning.

    \sa types()
*/

/*!
    Destroys the QtopaNetworkConfiguration instance.
*/
QtopiaNetworkConfiguration::~QtopiaNetworkConfiguration()
{
}

/*!
    \fn QtopiaNetworkProperties QtopiaNetworkConfiguration::getProperties() const

    Returns all properties of this interface. If only a single property is needed
    property() should be used.

    \sa property()
*/

/*!
    \fn void QtopiaNetworkConfiguration::writeProperties( const QtopiaNetworkProperties& properties)

    Writes \a properties out to the network configuration file.
*/

/*----------------------------------------------------------*/
// QtopiaNetworkProperties

/*!
    \class QtopiaNetworkProperties
    \inpublicgroup QtBaseModule

    \brief The QtopiaNetworkProperties class reflects the content of a network interface configuration.

    This helper class is used to access network configurations sourced from
    settings files by mapping keys to values. The following example demonstrates the mapping:

    \code
        QtopiaNetworkProperties prop;
        //populate with content of Configuration.conf
        ...

        QSettings s("Configuration", QSettings::IniFormat);
        s.beginGroup( "Group" );
        QString value = s.value("Key").toString();
        ...

        //Is true
        value == prop.value("Group/Key").toString();
    \endcode

    \sa QMap, QSettings
    \ingroup io
*/

/*!
    \fn QtopiaNetworkProperties::QtopiaNetworkProperties()

    Constructs an empty property map.
*/

/*!
    \fn QtopiaNetworkProperties::QtopiaNetworkProperties( const QtopiaNetworkProperties & other)

    Constructs a copy of \a other.
*/

/*!
    \fn QtopiaNetworkProperties::~QtopiaNetworkProperties()

    Destroys the object.
*/

QtopiaNetworkProperties::QtopiaNetworkProperties()
{
}

QtopiaNetworkProperties::QtopiaNetworkProperties( const QtopiaNetworkProperties& list )
    : QMap<QString, QVariant> ( list )
{
}

QtopiaNetworkProperties::~QtopiaNetworkProperties()
{
}

/*!
    This function dumps the content to the Network log stream if
    Qt Extended is build in debug mode; otherwise does nothing.
*/
void QtopiaNetworkProperties::debugDump() const
{
    if (qLogEnabled(Network)) {
        QString key;
        QString value;

        QtopiaNetworkProperties::const_iterator i = constBegin();
        qLog(Network) << "Dumping network settings";
        while ( i != constEnd() )
        {
            qLog(Network) << i.key() << ": " << i.value().toString();
            i++;
        }
    }
}

/*----------------------------------------------------------*/
//QtopiaNetworkPlugin

/*!
    \class QtopiaNetworkPlugin
    \inpublicgroup QtBaseModule

    \brief The QtopiaNetworkPlugin class provides an abstract base class for all network plug-ins.

    \ingroup plugins
    \ingroup io

    The plug-in should override at least type() and network(). The Qt Extended network
    plug-ins can be found under \c{QPEDIR/src/plugins/network}.

    \sa QtopiaNetworkFactoryIface, QtopiaNetworkInterface
*/

/*!
    Constructs a QtopiaNetworkPlugin instance with the given \a parent.
*/
QtopiaNetworkPlugin::QtopiaNetworkPlugin(QObject* parent)
    :QObject(parent)
{
}

/*!
    Destroys the QtopiaNetworkPlugin instance.
*/
QtopiaNetworkPlugin::~QtopiaNetworkPlugin()
{
}

/*!
    Destroys the QtopiaNetworkInterface instance.
*/
QtopiaNetworkInterface::~QtopiaNetworkInterface()
{
}

QtopiaNetworkMonitor* QtopiaNetworkInterface::monitor()
{
    return 0;
}

/*!
    \class QtopiaNetworkFactoryIface
    \inpublicgroup QtBaseModule

    \ingroup io
    \brief The QtopiaNetworkFactoryIface class defines the interface to network plug-ins.

    The QtopiaNetworkFactoryIface class defines the interface to network plug-ins. Plug-ins will
    typically inherit from QtopiaNetworkPlugin rather than this class.

    \sa QtopiaNetworkPlugin, QtopiaNetworkInterface
*/

/*!
    \fn QtopiaNetwork::Type QtopiaNetworkFactoryIface::type() const

    Returns the type of network interfaces that the current plug-in can handle.
    This function is usually called by the QtopiaNetworkServer to check whether this plug-in
    is appropriate for the given network interface.

    \sa QtopiaNetworkServer
*/

/*!
    \fn QPointer<QtopiaNetworkInterface> QtopiaNetworkFactoryIface::network(const QString& handle)

    Returns the instance of the QtopiaNetworkInterface provided by the plug-in.
    Multiple calls to this function will return several instances. \a handle is
    the configuration file that this interface should be initialized with.
*/

/*!
    \fn QByteArray QtopiaNetworkFactoryIface::customID() const

    The network server automatically matches network plug-ins and network configurations by comparing
    QtopiaNetworkFactoryIface::type() and QNetworkState::deviceType(). This may mean that in some situations
    several plug-ins match the same network configuration. If these plug-ins provide different functionality
    or user interfaces the user may become confused because the matching process may cause a race condition.

    To resolve such problems the QtopiaNetwork::Custom flag
    can be used in combination with this function to uniquely match a network configuration with a particular plug-in.
    If a plug-in sets the custom flag the network server will compare the custom ID of the current network configuration with
    the value returned by this function. Only if they match the plug-in will be allowed to manage this
    configuration. The custom ID string can be any string as long as it is unique and not empty. The custom
    flag is not raised when the configuration doesn't set the customID field.
    Note that the remaining type flags must still match.

    For more details about the network configuration file format refer to the
    \l{Network Services#Network configuration file format}{Network Services} documentation.
*/

/*!
    \internal
*/
QtopiaNetworkFactoryIface::~QtopiaNetworkFactoryIface()
{
}

