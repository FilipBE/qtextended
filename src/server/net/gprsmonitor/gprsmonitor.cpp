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

#include "gprsmonitor.h"

#include "qtopiaserverapplication.h"

#include <QFileSystemWatcher>
#include <QtopiaNetwork>
#include <QNetworkDevice>
#include <QValueSpaceObject>

#include <QCommServiceManager>
#include <QNetworkRegistration>

/*!
  \class GPRSMonitor
    \inpublicgroup QtCellModule
  \brief The GPRSMonitor class keeps track of the state of GPRS accounts.
  \ingroup QtopiaServer

  This task monitors the types of internet connections available to the user.
  The information is used by the home screen title bar which indicates the 
  GPRS state to the user.

  The results are posted into the value space under:
  \code
        /Network/GPRSEnabled
        /Network/GPRSConnected
        /Network/EDGEEnabled
        /Network/EDGEConnected
        /Network/UMTSEnabled
        /Network/UMTSConnected
  \endcode

  GPRS is considered to be enabled if Qt Extended has at least one GPRS account which
  could be started/is configured. GPRS is connected when the IP connection to the 
  operator network has been established. The EDGE indicator always takes precedence over the GPRS indicator and the UMTS 
  indicator has precedence over GPRS and EDGE. GPRS, EDGE and UMTS are 
  mutually exclusive options.

  Note: The EDGE field is not enabled as Qt Extended cannot detect EDGE as bearer yet.
  
  The GPRSMonitor is a Qt Extended server task and is automatically started by the server.
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
  Creates a GPRS monitor whereby \a parent is the standard Qt parent object.
  */
GPRSMonitor::GPRSMonitor( QObject* parent )
    : QObject( parent ), netReg( 0 ), umts( false )
{
    watcher = new QFileSystemWatcher( this );
    watcher->addPath( QtopiaNetwork::settingsDir() );
    connect( watcher, SIGNAL(directoryChanged(QString)), this, SLOT(dataAccountsChanged()) );

    commManager = new QCommServiceManager( this );
    connect( commManager, SIGNAL(servicesChanged()), this, SLOT(servicesAdded()) );
    
    vso = new QValueSpaceObject( QByteArray("/Network"), this );
    vso->setAttribute( "GPRSEnabled", false );
    vso->setAttribute( "GPRSConnected", false );
    vso->setAttribute( "EDGEEnabled", false );
    vso->setAttribute( "EDGEConnected", false );
    vso->setAttribute( "UMTSEnabled", false );
    vso->setAttribute( "UMTSConnected", false );

    dataAccountsChanged(); //find all GPRS accounts
    servicesAdded(); //check network technology ( 3G, GSM )
}


/*!
  \internal 

  Destroys the GPRSMonitor
*/
GPRSMonitor::~GPRSMonitor()
{
}

/*!
  \internal

  This slot is called whenever a comm service was removed/added. This way
  we keep track of QNetworkRegistrations
  */
void GPRSMonitor::servicesAdded()
{
    if ( commManager->supports<QNetworkRegistration>().contains("modem") ) {
        if ( !netReg ) {
            netReg = new QNetworkRegistration( "modem", this );
            connect( netReg, SIGNAL(currentOperatorChanged()), 
                    this, SLOT(currentOperatorChanged()) );
            currentOperatorChanged();
        }
    } else {
        if ( netReg )  {
            delete netReg;
            netReg = 0;
            umts = false;
        }
    }
}

/*!
  \internal
  This slot is called whenever the current network operator changes. Each operator
  change my go hand in hand with a technology change (UMTS vs. GSM).
  */
void GPRSMonitor::currentOperatorChanged()
{
    if ( !netReg ) //just making sure...
       return;
  
    const bool oldUmtsState = umts; 
    umts = (netReg->currentOperatorTechnology() == "UTRAN"); //no tr
    if ( oldUmtsState != umts )
        gprsStateChanged();
    
}

/*!
  This slot is called whenever a network data account was created and/or deleted.
*/
void GPRSMonitor::dataAccountsChanged()
{
    bool updateState = false;
    QStringList configs = QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::GPRS );
    foreach( QNetworkDevice* dev, knownGPRSDevices )
    {
        if ( configs.contains( dev->handle() ) ) {
            configs.removeAll( dev->handle() );
        } else {
            knownGPRSDevices.removeAll( dev );    
            delete dev;
            dev = 0;
            updateState = true;
        }
    }

    foreach( QString cfg, configs ) {
        QNetworkDevice* dev = new QNetworkDevice( cfg, this );
        knownGPRSDevices.append( dev );
        connect( dev, SIGNAL(stateChanged(QtopiaNetworkInterface::Status,bool)),
                this, SLOT(gprsStateChanged()) );
        updateState = true;
    }

    if ( updateState )
        gprsStateChanged();
}

/*!
  This slot is called when a monitored GPRS account changes its state

*/
void GPRSMonitor::gprsStateChanged()
{
    bool gprsEnabled = false;
    bool gprsRunning = false;
    foreach( QNetworkDevice* dev, knownGPRSDevices ) {
        switch( dev->state() ) 
        {
            case QtopiaNetworkInterface::Unknown:
            case QtopiaNetworkInterface::Unavailable:
                break;    
            case QtopiaNetworkInterface::Down:
                gprsEnabled = true;
                break; 
            case QtopiaNetworkInterface::Pending:
            case QtopiaNetworkInterface::Demand:
            case QtopiaNetworkInterface::Up:
                gprsEnabled = gprsRunning = true;
                break;
        }

        if ( gprsRunning ) {
            vso->setAttribute( "GPRSEnabled", !umts );
            vso->setAttribute( "GPRSConnected", !umts );
            vso->setAttribute( "UMTSEnabled", umts );
            vso->setAttribute( "UMTSConnected", umts );
            return;
        }
    }

    vso->setAttribute( "GPRSEnabled", gprsEnabled && !umts );
    vso->setAttribute( "GPRSConnected", gprsRunning && !umts );
    vso->setAttribute( "UMTSEnabled", gprsEnabled && umts );
    vso->setAttribute( "UMTSConnected", gprsRunning && umts );
}

QTOPIA_TASK(GPRSMonitor,GPRSMonitor);
