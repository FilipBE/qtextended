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

#include "qvpnclient.h"
#include "qvpnclientprivate_p.h"

#include <QHash>
#include <QProcess>

#include <qtopianamespace.h>
#include <qtopialog.h>

/*!
  \class QVPNClient

  \brief The QVPNClient class abstracts data and state of a virtual private network.

  An instance of a QVPNClient can be created by using \l QVPNFactory. Qt Extended currently
  supports OpenVPN only. New VPN implementations must subclass this abstract class.

  Each VPNClient instance operates in one of the follwoing two modes:
  \list
    \o Client mode -
        This mode is the most common way of using a QVPNClient object. Any Qt Extended application
        that obtains a QVPNClient instance via QVPNFactory::create() will create such an object.
        Essentially the return instance acts as a thin wrapper that forwards the VPN requests
        to the QtopiaVpnManager.
    \o Server mode -
        Only the QtopiaVpnManager can create such a QVPNObject instance. It performs the actual
        VPN operations by making the appropriate calls to the root file system.
  \endlist

  Every VPN can be identified via a unique identifier returned by id(), has a name() and a type().
  The VPN configuration can be changed via configure(). connect() will establish
  the VPN connection. Once the VPN has been started its
  state() will change from \c{QVPNCLient::Disconnected} to \c{QVPNClient::Pending}
  and eventually to \c{QVPNClient::Connected}. Each state change is indicated via the
  connectionStateChanged() signal. If an error occurs during the startup period the user
  can obtain a human-readable string describing the nature of the error.

  If the user decides to delete a VPN connection cleanup() should be called to remove any
  configuration file that has been created previously. By default Qt Extended saves VPN
  configuration files in \c{$HOME/Applications/Netork/vpn}.

  \sa QVPNFactory
  \ingroup io
*/

/*!
  \enum QVPNClient::Type

  This enum is used to describe the type of a VPN connection.

  \value OpenVPN VPN solution based on OpenVPN (for details see \l http://openvpn.net)
  \value IPSec VPN solution based on IPSec (not yet implemented in
                Qt Extended and serves as place holder for future implementation)
*/

/*!
  \enum QVPNClient::State

  This enum is used to describe the state of a VPN connection.

  \value Disconnected The VPN connection is offline.
  \value Pending The VPN connection is in a transitional phase between Disconnected and Connected.
  \value Connected The VPN connection is active and can be used.
*/



/*!
  Creates a new QVPNClient instance with the given \a parent. QVPNClient instances
  can only be created via QVPNFactory::create().

  \a serverMode determines whether this VPN object operates in server mode.
  Qt Extended applications usually obtain client mode instances of QVPNClient. In this mode
  all request are forwarded to the QtopiaVpnManager which keeps a reference to a
  VPNClient instance running in server mode. This allows the synchronization of
  multiple VPN requests to the same VPN.

  This constructor is used internally by QVPNFactory whenever the user creates a new
  VPN that doesn't exist yet.

  \sa QtopiaVpnManager, QVPNFactory

  */
QVPNClient::QVPNClient( bool serverMode, QObject* parent )
    : QObject( parent )
{
    d = new QVPNClientPrivate();
    d->serverMode = serverMode;

}

/*!
  Creates a new QVPNClient instance with the given \a parent. QVPNClient instances
  can only be created via QVPNFactory::create().

  \a serverMode determines whether this VPN object operates in server mode.
  Qt Extended applications usually obtain client mode instances of QVPNClient. In this mode
  all request are forwarded to the QtopiaVpnManager which keeps a reference to a
  VPNClient instance running in server mode. This allows the synchronization of
  multiple VPN requests to the same VPN.

  \a vpnID acts as an identifier for a particular VPN.

  \sa QtopiaVpnManager, QVPNFactory
  */
QVPNClient::QVPNClient( bool serverMode, uint vpnID, QObject* parent )
    : QObject( parent )
{
    d = new QVPNClientPrivate();
    d->serverMode = serverMode;

    const QString path = Qtopia::applicationFileName( "Network", "vpn" );
    QDir dir(path);
    dir.mkdir( path );

    const QStringList files = dir.entryList( QStringList("*.conf") );
    QString file;
    foreach( QString entry, files ) {
        file = dir.filePath(entry);
        if ( qHash(file) == vpnID ) {
            d->config = file;
            break;
        }
    }
    if ( qLogEnabled(VPN) && d->config.isEmpty() )
        qLog(VPN) << "Unknown VPN id: "<< vpnID;
}

/*!
  Destroys the virtual private network.
*/
QVPNClient::~QVPNClient()
{
    delete d;
    d = 0;
}

/*!
  Returns the unique ID of this virtual private network.
*/
uint QVPNClient::id() const
{
    return qHash( d->config );
}

/*!
  Returns the user set name for this VPN.
  */
QString QVPNClient::name() const
{
    QSettings cfg( d->config, QSettings::IniFormat );
    return cfg.value( "Info/Name", QString() ).toString();
}

/*!
  Returns a human-readable description of the last error that occurred. This is useful for
  presenting an error message to the user when receiving a connectionStateChanged() signal
  with the error argument set to true;
  */
QString QVPNClient::errorString() const
{
    return d->errorString;
}

/*!
  \fn void QVPNClient::connect()

  This function starts the VPN connection.
  */

/*!
  \fn void QVPNClient::disconnect()

  This function stops the active connection.
  */

/*!
  \fn QDialog* QVPNClient::configure( QWidget* parent )

  Returns the default configuration dialog that is shown to the user
  when he configures this interface. \a parent is the parent widget for
  the returned dialog.
  */

/*!
  \fn State QVPNClient::state() const

  Returns the current state of this VPN client.

  \sa QVPNClient::State
  */

/*!
  \fn Type QVPNClient::type() const

  Returns the VPN client type.

  \sa QVPNClient::Type
  */

/*!
  \fn void QVPNClient::cleanup()

  Deletes the VPN client and all files associated to it. This function
  does nothing if the client is still connected.
  */

/*!
  \fn void QVPNClient::connectionStateChanged( bool error )

  This signal is emitted when the state of the VPN connection changes.

  \a error will be set to \c true if an error occurred during the last state transition.
  A human-readable string of the error can be obtained via errorString()

  This signal must be emitted by subclasses of QVPNClient when state transitions occur.
  */
