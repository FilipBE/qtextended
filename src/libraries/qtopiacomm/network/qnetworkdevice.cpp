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

#include <qnetworkdevice.h>
#include <qtranslatablesettings.h>

#include <QDebug>
#include <QHash>
#include <qvaluespace.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>


class QNetworkDevicePrivate: public QObject
{
    Q_OBJECT
public:
    QNetworkDevicePrivate( const QString& handle, QObject* parent = 0 )
        : QObject( parent ), iface( handle ),
        error( QtopiaNetworkInterface::NoError ),
        state( QtopiaNetworkInterface::Unknown ),
        firstTime( true )
    {
        devHash = QString::number( qHash( iface ) );
        devSpace = new QValueSpaceItem( "/Network/Interfaces/" + devHash, this );
        devStateChanged();
        updateTrigger = new QValueSpaceItem( "/Network/Interfaces/"+devHash +"/UpdateTrigger" );
        connect( updateTrigger, SIGNAL(contentsChanged()), this, SLOT(devStateChanged()) );
    }

    ~QNetworkDevicePrivate()
    {
    }

    QString errorString() const
    {
        return devSpace->value( "/ErrorString", QString() ).toString();
    }

    QString deviceName() const
    {
        return devSpace->value( "/NetDevice", QString() ).toString();
    }

public:
    const QString iface;
    QtopiaNetworkInterface::Error error;
    QtopiaNetworkInterface::Status state;

Q_SIGNALS:
    void deviceStateChanged( QtopiaNetworkInterface::Status newState, bool error );

private Q_SLOTS:
    void devStateChanged( )
    {
        QtopiaNetworkInterface::Status newState =
            (QtopiaNetworkInterface::Status) (devSpace->value("/State", 0 ).toInt());

        state = newState;
        error = (QtopiaNetworkInterface::Error) ( devSpace->value("/Error", 0 ).toInt() );
        if ( firstTime ) 
            firstTime = false;
        else 
            emit deviceStateChanged( state, error );
    }

private:
    QString devHash;
    QValueSpaceItem* devSpace;
    QValueSpaceItem* updateTrigger;
    bool firstTime;
};


/*!
  \class QNetworkDevice
    \inpublicgroup QtBaseModule

  \brief The QNetworkDevice class provides information about the connectivity state of
  a network device.

  \section1 Architecture

  Network devices are always associated with a configuration file which serves as an 
  identifier/handle for a particular device. The configuration file is device specific 
  and is saved in \c{$HOME/Applications/Network/config/}.
  The user can start start/stop/configure network devices via the Internet application. 
  Network related application can start and stop the device via 
  \l QtopiaNetwork::startInterface() and \l QtopiaNetwork::stopInterface() respectively. 
  If the Qt Extended device has more than one device online at a time the default gateway 
  can be set via \l QtopiaNetwork::setDefaultGateway().

  The following UML class diagram displays the general interaction of the Qt Extended network API.

  \image NetworkAPI.png
  
  Qt Extended keeps track of network devices with the help of a session manager. 
  If an application starts a device the session manager records the identity of the 
  application and will close the device if the application doesn't stop the device 
  before it quits. If several applications requested the same device the device 
  is closed when the last remaining application closes the device. Hence every 
  application should call \l QtopiaNetwork::startInterface() 
  no matter whether the device is active already as this call creates a new session 
  for this application. Note that a device which has been configured to use
  an internal timeout (e.g. timeouts of dial-up connections) may still stop when 
  the timeout is triggered. 
 
  Any network related application may stop a particular interface which has 
  been started by another application. This may be necessary in use cases such as 
  when the phone receives a notification about a pending MMS. If the dial-up 
  connection is already running but the APN is not the required one for MMS download 
  the mail application may stop the dial-up connection and start another dial-up interface.
  
  Since a QNetworkDevice is directly related to hardware it does not support use 
  cases whereby e.g. an application wants to know to what WLAN the device is 
  connected to. In order to obtain such information it is required to use 
  QNetworkConnection which allows network identification on a much finer level of granularity.

  \image netdevice-netconnection.png
  
  \sa QtopiaNetwork, QNetworkState, QNetworkConnection, QtopiaNetworkInterface
  \ingroup io
*/



/*!
  Constructs a QNetworkDevice object for the network device which is defined by
  \a handle. \a parent is the standard Qt parent object .
  */
QNetworkDevice::QNetworkDevice( const QString& handle, QObject* parent )
    : QObject( parent )
{
    d = new QNetworkDevicePrivate( handle, this );
    connect( d, SIGNAL(deviceStateChanged(QtopiaNetworkInterface::Status,bool)),
            this, SIGNAL(stateChanged(QtopiaNetworkInterface::Status,bool)) );
}

/*!
  Deconstructs the object.
  */
QNetworkDevice::~QNetworkDevice()
{
}

/*!
  Returns the interface handle which is the full path of the associated configuration file.
  */
QString QNetworkDevice::handle() const
{
    return d->iface;
}

/*!
  This function returns the connectivity state of this device.
*/
QtopiaNetworkInterface::Status QNetworkDevice::state() const
{
    return d->state;
}

/*!
  Returns the last error that occured. This is useful to find out what went wrong
  when receiving a stateChanged() signal with the \c error argument set to \c true.

  Note: There could be various reasons why a device may report \c QtopiaNetworkInterface::NotConnected.
  A detailed description for such an error can be obtained by using errorString().
  */
QtopiaNetworkInterface::Error QNetworkDevice::error() const
{
    return d->error;
}

/*!
  Returns a human-readable description of the last error that occurred.
  This is useful for presenting an error message to the user.
*/
QString QNetworkDevice::errorString() const
{
    return d->errorString();
}

/*!
  Returns the associated QNetworkInterface.
  */
QNetworkInterface QNetworkDevice::address() const
{
    return QNetworkInterface::interfaceFromName( d->deviceName() );
}

/*!
  \fn void QNetworkDevice::stateChanged( QtopiaNetworkInterface::Status newState, bool error )

  This signal is emitted when the state of this device changes. \a newState is the new state.
  If \a error is true an error has occurred during the last transition.

  \sa errorString()

  \sa state()
*/

/*!
  Returns the name of the associated *nix network interface, e.g. \c eth0 or \c ppp2. The name should not be used to identify
  the network interface as it can change during the life time of the device. The returned name is empty if the interface
  hasn't been initialized yet or is not available.

  \sa state()
*/
QString QNetworkDevice::interfaceName() const
{
    return d->deviceName();
}

/*!
  Returns the human-readable name of the device (e.g. Vodafone GPRS). The name can be set by the user when creating
  and/or configuring the device.
  */
QString QNetworkDevice::name() const
{
    QTranslatableSettings cfg( d->iface, QSettings::IniFormat );
    return cfg.value( "Info/Name" ).toString();
}
#include "qnetworkdevice.moc"
