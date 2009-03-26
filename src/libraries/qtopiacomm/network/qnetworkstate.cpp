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

#include "qnetworkstate.h"

#include <QHash>
#include <QSettings>
#include <QStringList>

#include <qtopianetwork.h>
#include <qnetworkdevice.h>
#include <qvaluespace.h>

class QNetworkStatePrivate : public QObject
{
    Q_OBJECT
public:
    QNetworkStatePrivate( QObject* parent )
        : QObject( parent )
    {
        netSpace = new QValueSpaceItem( "/Network/Interfaces", this );
        update();
        connect( netSpace, SIGNAL(contentsChanged()), this, SLOT(update()) );
        gwSpace = new QValueSpaceItem( "/Network/Gateway" , this );
        connect( gwSpace, SIGNAL(contentsChanged()), this, SLOT(gatewayChanged()) );
    }

    virtual ~QNetworkStatePrivate()
    {
    }

    QString gateway() const
    {
        return gwSpace->value( "Default", QString() ).toString();
    }

    QList<QString> interfacesOnline;

Q_SIGNALS:
    void defaultGatewayChanged( const QString& iface );
    void connected();
    void disconnected();

private slots:

    void gatewayChanged()
    {
        emit defaultGatewayChanged( gateway() );
    }

    void update()
    {
        const bool isOnline = interfacesOnline.count();
        interfacesOnline.clear();
        const QStringList ifaceList = netSpace->subPaths();
        foreach( QString iface, ifaceList ) {
            QString config = netSpace->value( iface+QString("/Config") ).toString();
            if ( config.isEmpty() )
                continue;
            QtopiaNetworkInterface::Status state =
                (QtopiaNetworkInterface::Status) (netSpace->value(iface+QString("/State"), 0 ).toInt());
            if ( state == QtopiaNetworkInterface::Up
                || state == QtopiaNetworkInterface::Demand
                || state == QtopiaNetworkInterface::Pending ) {
                interfacesOnline << config;
            }
        }

        if ( isOnline && !interfacesOnline.count() )
            emit disconnected();
        else if ( !isOnline && interfacesOnline.count() )
            emit connected();
    }
private:
    QValueSpaceItem* netSpace;
    QValueSpaceItem* gwSpace;
};

/*!
  \class QNetworkState
    \inpublicgroup QtBaseModule

  \brief The QNetworkState class provides very generic information about the connectivity state of the Qt Extended device.

  In addition to QtopiaNetwork::online() which returns the current connectivity state
  QNetworkState provides the connected() and disconnected() signals which are emitted 
  when the connectivity changes. If more specific information about a particular 
  device is required QNetworkDevice should be used.

  interfacesOnline() returns the list of interfaces which are online. An network interface 
  is identified via its device handle which is the configuration file that specifies the 
  device parameter. If it is required to enumerate all or a particular type of network devices
  (regardless of their state) availableNetworkDevices() can be used. 
  The reverse operation is provided by deviceType().

  QNetworkState also provides gateway/routing information. The gateway() function returns the
  devHandle belonging to the network device that currently provides the default route. 
  The defaultGatewayChanged() signal is emitted when the default route changes. The default 
  WAP configuration is returned via defaultWapAccount().

  \ingroup io
  \sa QtopiaNetwork, QNetworkDevice, QNetworkConnection
*/

/*!
  Constructs a QNetworkState object with the specified \a parent.
*/
QNetworkState::QNetworkState( QObject* parent )
    : QObject( parent )
{
    d = new QNetworkStatePrivate( this );
    connect( d, SIGNAL(connected()), this, SIGNAL(connected()) );
    connect( d, SIGNAL(disconnected()), this, SIGNAL(disconnected()) );
    connect( d, SIGNAL(defaultGatewayChanged(QString)),
             this, SLOT(gatewayChanged(QString)) );
}

/*!
  Destructs the QNetworkState object.
*/
QNetworkState::~QNetworkState()
{
}

/*!
  Returns the handle to the network interface that is currently used as default gateway for
  network packages. If QtopiaNetwork::online() returns \c{false} this function returns
  an empty string.
*/
QString QNetworkState::gateway() const
{
    return d->gateway();
}

/*!
  This function returns the list of all interface which are online at the time
  of the function call. The returned list contains the device handles for these interfaces.
*/
QList<QString> QNetworkState::interfacesOnline() const
{
    return d->interfacesOnline;
}

/*!
  Returns the list of known network devices of \a type.
  If \a type is \c{QtopiaNetwork::Any} it returns all known devices. A device is considered
  to be known if a configuration file exists for it. The returned Qt Extended network interface handles
  are equivalent to the full qualified path to the configuration file.

 \sa QtopiaNetwork::availableNetworkConfigs()
*/
QList<QString> QNetworkState::availableNetworkDevices( QtopiaNetwork::Type type )
{
    return QtopiaNetwork::availableNetworkConfigs( type );
}

/*!
  Returns the type of \a devHandle.
*/
QtopiaNetwork::Type QNetworkState::deviceType( const QString& devHandle )
{
    return QtopiaNetwork::toType( devHandle );
}


/*!
  Returns the default WAP account. If no default account has been set this function returns
  an empty string. The returned account can be accessed via QWapAccount.

  \code
    QNetworkState state;
    QString wapConf = state.defaultWapAccount();
    if ( !wapConf.isEmpty() ) {
        QWapAccount acc( state.defaultWapAccount() );
        QString name = acc.name();  //returns user visibile name
    }
  \endcode

  \sa QWapAccount
*/
QString QNetworkState::defaultWapAccount() const
{
    QSettings cfg( "Trolltech", "Network" );
    cfg.beginGroup("WAP");
    QString r = cfg.value("DefaultAccount", QString()).toString();
    cfg.endGroup();
    return r;
}

/*!
  \fn void QNetworkState::connected()

  This signal is emitted when Qt Extended changes from offline to online.
  If the connectivity state of a particular device is required \l QNetworkDevice::state()
  should be used.

  \sa QtopiaNetwork::online()
*/

/*!
  \fn void QNetworkState::disconnected()

  This signal is emitted when Qt Extended changes from online to offline.
  If the connectivity state of a particular device is required \l QNetworkDevice::state()
  should be used.

  \sa QtopiaNetwork::online()
*/

/*!
  \fn void QNetworkState::defaultGatewayChanged( QString handle, const QNetworkInterface& local )

  This signal is emitted when the default gateway for network packages
  changes. Such a change usually implies that a new route and new DNS information
  have been set. \a handle is the new default interface and \a local contains the IP
  details for \a handle. An empty \a handle implies that no new default gateway is available. This
  may happen when the last online interface changes its state to offline.

  \sa gateway()
  */

/*!
  \internal
*/
void QNetworkState::gatewayChanged( const QString& newGateway )
{
    if ( newGateway.isEmpty() ) {
        QNetworkInterface iface;
        emit defaultGatewayChanged( QString(), iface );
    } else {
        QNetworkDevice dev( newGateway );
        if ( dev.state() == QtopiaNetworkInterface::Up ) {
            QNetworkInterface i = dev.address();
            emit defaultGatewayChanged( newGateway, i );
            return;
        }
        QNetworkInterface iface;
        emit defaultGatewayChanged( QString(), iface );
    }
}

#include "qnetworkstate.moc"
