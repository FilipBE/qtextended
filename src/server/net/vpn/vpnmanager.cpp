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

#include "vpnmanager.h"
#include "qtopiaserverapplication.h"


#include <QVPNFactory>
#include <QVPNClient>

#include <QDebug>

#include <qcategorymanager.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

/*!
  \class QtopiaVpnManager
  \inpublicgroup QtConnectivityModule
  \ingroup QtopiaServer::Task
  \brief The QtopiaVpnManager class provides management functionality for virtual private network sessions regardless of their type.

  Similar to the QtopiaNetworkServer the VPN manager synchronizes the VPN requests 
  throughout Qtopia. Client applications request VPN functions via QVPNClient which
  forwards the requests to this network server. All
  forwarded network requests are directly executed by this server class. 
  
  The QtopiaVPNManager class provides the \c {QtopiaVPNManager} task.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa QVPNClient, QVPNFactory

  */

/*!
  Creates a QtopiaVpnManager instance with the given \a parent.
  */
QtopiaVpnManager::QtopiaVpnManager( QObject* parent )
    : QtopiaIpcAdaptor( QLatin1String("QPE/VPNManager"), parent )
{
    publishAll( QtopiaIpcAdaptor::Slots );
    QString certificateString=QLatin1String("Certificate"),
            securityKeyString=QLatin1String("Security Key");

    // Ensure these system categories exist
    QCategoryManager cats("Documents");
    // For new code a more unique id should be used instead of using the untranslated text
    // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
    cats.ensureSystemCategory(certificateString, certificateString);
    cats.ensureSystemCategory(securityKeyString, securityKeyString);

    vpnFactory = new QVPNFactory();
    vpnFactory->setServerMode( true );

    qLog(VPN) << "Starting VPN Manager";
}

/*!
  Destroys the QtopiaVpnManager instance.
  */
QtopiaVpnManager::~QtopiaVpnManager()
{
    if ( idToVPN.count() > 0 ) {
        QMutableHashIterator<uint,QVPNClient*> i(idToVPN);
        QVPNClient* vpn = 0;
        while( i.hasNext() ) {
            i.next();
            vpn = i.value();
            if ( vpn && vpn->state() != QVPNClient::Disconnected ) {
                qLog(VPN) << "Stopping VPN" << vpn->name();
                vpn->disconnect();
                i.remove();
                delete vpn;
                vpn = 0;
            }
        }
    }

    delete vpnFactory;
    vpnFactory = 0;
}

/*!
  Starts VPN with the given \a vpnID.
  \internal
  */
void QtopiaVpnManager::connectVPN( uint vpnID )
{
    qLog(VPN) << "VPNManager received request to start VPN" << vpnID;
    QVPNClient* vpn = 0;
    if ( idToVPN.contains(vpnID) ) {
        vpn = idToVPN[ vpnID ];
    } else {
        vpn = vpnFactory->instance( vpnID, this );
        idToVPN[vpnID] = vpn;
    }

    if ( vpn->state() == QVPNClient::Disconnected ) {
        qLog(VPN) << "Starting VPN connection" << vpn->name();
        vpn->connect();
    } else {
        qLog(VPN) << vpn->name() << "is already running.";
    }

}

/*!
  Stops VPN with the given \a vpnID.
  \internal
  */
void QtopiaVpnManager::disconnectVPN( uint vpnID )
{
    qLog(VPN) << "VPNManager received request to stop VPN" << vpnID;
    if ( idToVPN.contains(vpnID) ) {
        QVPNClient* vpn = idToVPN[vpnID ];
        if ( !vpn )
            return;
        vpn->disconnect();
        qLog(VPN) << "Stopping VPN connection" << vpn->name();
    } else {
        qLog(VPN) << "VPN" << vpnID << "is not running.";
    }
}

/*!
  Delete VPN with the given \a vpnID.
  \internal
  */
void QtopiaVpnManager::deleteVPN( uint vpnID )
{
    qLog(VPN) << "VPNManager received request to delete VPN" << vpnID;
    QVPNClient* vpn = 0;
    if ( idToVPN.contains(vpnID) )
        vpn = idToVPN.take( vpnID );
    else
        return;

    if ( !vpn )
        return;

    if ( vpn->state() != QVPNClient::Disconnected ) {
        qLog(VPN) << "VPN is still running. Stop VPN and try again";
        return;
    }

    qLog(VPN) << "VPN deleted" ;
    delete vpn;
    vpn = 0;
}

QTOPIA_TASK(QtopiaVpnManager,QtopiaVpnManager);
