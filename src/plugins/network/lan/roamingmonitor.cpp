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

#include "roamingmonitor.h"
#include <QTimer>

#ifndef NO_WIRELESS_LAN

#include <qvaluespace.h>
#include <qtopialog.h>
#include <qnetworkdevice.h>
#include <QSignalSourceProvider>

#include <QEventLoop>
#include <QSettings>

#include "wirelessscan.h"

/*!
  \internal
  \class RoamingMonitor
  \inpublicgroup QtConnectivityModule
  \brief This class assists the LAN plugin in the process of manual and automatic roaming from
  one WLAN to another.

  The class is based on a WLAN scanner which periodically scans the environment for changes. The monitor
  can send a signal when it is appropriate to change the network. The configuration of the roaming behavior
  is done via the LAN plug-in's general configure dialog.

  The scanner requires a Linux kernel with Wireless Extensions v14 or above.
*/
RoamingMonitor::RoamingMonitor( QtopiaNetworkConfiguration* cfg, QObject* parent )
    :QObject( parent ), configIface( cfg ), activeHop( false )
{
    // some interfaces need to be up
    QSettings config(configIface->configFile(), QSettings::IniFormat);
    const bool scanWhileDown = config.value("Properties/ScanWhileDown", true).toBool();
    scanner = new WirelessScan( QNetworkDevice(configIface->configFile()).interfaceName(),
                                scanWhileDown );
    connect( scanner, SIGNAL(scanningFinished()), this, SLOT(newScanResults()) );
    rescanTimer = new QTimer( this );
    connect( rescanTimer, SIGNAL(timeout()), this, SLOT(scanTimeout()) );

    const int ident = qHash( configIface->configFile() );
    netSpace = new QValueSpaceItem( QString("/Network/Interfaces/%1/NetDevice").arg(ident), this );
    deviceName = netSpace->value().toString();
    connect( netSpace, SIGNAL(contentsChanged()), this, SLOT(deviceNameChanged()) );
    //keep track of signal strength
    signalProvider = new QSignalSourceProvider( "wlan", QString::number(qHash( configIface->configFile() )), this );
#if WIRELESS_EXT > 11
    signalProvider->setAvailability( QSignalSource::NotAvailable );
    //polling is bad but there is no alternative to it right now.
    signalTimer = new QTimer( this );
    signalTimer->setInterval( 60000 ); 
    connect( signalTimer, SIGNAL(timeout()), this, SLOT(updateSignalStrength()) );
#else
    //we need WE 11+ to detect signal strength
    signalProvider->setAvailability( QSignalSource::Invalid );
#endif
}

RoamingMonitor::~RoamingMonitor()
{
    if (scanner) {
        delete scanner;
    }
}

/*!
  \internal

  Returns the index of \a essid in the list of known networks. If \a essid is empty
  we connect to the next network in the list of known networks.

  This function returns 0 if there is no network with name \a essid or none
  of the known networks is available/online.

  This function performs an active scan and will block until the scan has completed.
*/
int RoamingMonitor::selectWLAN( const QString& essid )
{
    QNetworkDevice dev( configIface->configFile() );
    const QVariant numNets = configIface->property("WirelessNetworks/size");

    if ( !numNets.isValid() ) {
        qLog(Network) << "Cannot connect on" << dev.interfaceName() << ". WLAN configuration missing.";
        return 0; //we don't have a valid configuration. no point in going on
    }

    //if an essid was passed we directly connect to the network
    //otherwise we go through the list of known networks and connect to the first
    //wlan that is online

    //get list of known networks
    //NOTE: if two nets use the same essid we only select the one that comes first
    QStringList knownEssids;
    QStringList knownMACs;
    for( int i=1; i<= numNets.toInt(); i++ ) {
        QString keyPrefix = "WirelessNetworks/"+QString::number(i);
        QString id = configIface->property( keyPrefix+"/ESSID").toString();
        knownEssids.append( id );
        knownMACs.append( configIface->property(keyPrefix+"/AccessPoint").toString() );
        if ( !essid.isEmpty() && essid == id ) {
            qLog(Network) << "Trying to directly connect to" << essid << "on interface"
                << dev.interfaceName();
            return i;
        }
    }
    if ( !essid.isEmpty() )
        return 0; //didn't find the given essid among known networks;

#if WIRELESS_EXT > 13
    //get list of all networks around us
    if ( !scanner ) {
        //we don't have scanning facilities
        //connect to first entry in list
        return 1; //there has to be at least one network ( numNets.toInt() > 0 )
    }

    //do active scan and wait until we have new scan result
    QEventLoop* loop = new QEventLoop();
    if ( !scanner->isScanning() ) {
        const bool hasStarted = scanner->startScanning();
        if ( !hasStarted ) {
            qLog(Network) << "Can not start scan. Attempting to start first configured network.";
            delete loop;
            return 1;
        }
    } // else { //already scanning}
    connect( scanner, SIGNAL(scanningFinished()), loop, SLOT(quit()) );
    loop->exec();
    delete loop;
    
    const QList<WirelessNetwork> results = scanner->results();
    if ( !results.count() ) {
        qLog(Network) << "Could not find any known WLAN that surrounds us";
        return 0;
    }
    QStringList onlineMacs;
    QStringList onlineEssids;
    foreach( WirelessNetwork net, results ) {
        onlineMacs.append( net.data( WirelessNetwork::AP ).toString() );
        onlineEssids.append( net.data( WirelessNetwork::ESSID ).toString() );
    }

    //use a MAC based lookup first because the essid could be hidden
    //find index of next WLAN that is online ( use list of MACs for lookup )
    QString mac;
    for (int i = 0; i<knownMACs.count(); i++ ) {
        mac = knownMACs[i];
        if ( mac.isEmpty() )
            continue;
        if ( onlineMacs.contains( mac ) ) {
                qLog(Network) << "Automatic attempt to connect to" << knownEssids[i]
                    << " - MAC=" << knownMACs[i];
                return i+1;
        }
    }

    //use an ESSID based lookup
    //find index of next WLAN that is online ( use list of ESSIDs for lookup )
    QString id;
    for (int i = 0; i<knownEssids.count(); i++ ) {
        id = knownEssids[i];
        if ( id.isEmpty() )
            continue;
        if ( onlineEssids.contains( id ) ) {
                qLog(Network) << "Automatic attempt to connect to" << knownEssids[i];
                return i+1;
        }
    }

    //we could have a case where the essid is hidden and we don't have 
    //the MAC for the hidden network
    if ( onlineEssids.contains("<hidden>") ) {
        qLog(Network) << "Can not match any configured WLAN with environment. "
                        "However there are hidden networks which might match anyway. "
                        "Attempting to connect to first configured network.";
        return 1;
    }

#else
    //we don't have a scanner and we haven't got a essid
    //just try to connect to the first wlan in list
    return 1; //there has to be at least one network ( numNets.toInt() > 0 )
#endif //WIRLESS_EXT>13

    qLog(Network) << "Could not find any known WLAN that surrounds the device";
    return 0;
}

void RoamingMonitor::scanTimeout()
{
#if WIRELESS_EXT > 13
    const bool autoConnect = configIface->property("WirelessNetworks/AutoConnect").toBool();
    if ( !autoConnect ) 
        return;

    //the scanner is attached to certain interface name. if the interface name changes
    //we have to reset the scanner
    if ( scanner && scanner->attachedInterface() != deviceName ) {
        delete scanner;
        scanner = 0;
    }

    if ( deviceName.isEmpty() )
       return;

    if ( !scanner ) {
        // some interfaces need to be up
        QSettings cfg(configIface->configFile(), QSettings::IniFormat);
        const bool scanWhileDown = cfg.value("Properties/ScanWhileDown", true).toBool();
        scanner = new WirelessScan( QNetworkDevice(configIface->configFile()).interfaceName(),
                                    scanWhileDown );
        connect( scanner, SIGNAL(scanningFinished()), this, SLOT(newScanResults()) );
    }
    if ( !scanner->isScanning() )
        scanner->startScanning();
#endif
}

void RoamingMonitor::newScanResults()
{
#if WIRELESS_EXT > 13
    if ( !activeHop )
        return;

    const bool autoConnect = configIface->property("WirelessNetworks/AutoConnect").toBool();
    if ( !autoConnect || !scanner )
        return;

    const QList<WirelessNetwork> results = scanner->results();
    if ( !results.count() ) {
        qLog(Network) << "No accessable WLAN's within range";
        return;
    }

    //get all networks in our know networks list
    const int numNets = configIface->property("WirelessNetworks/size").toInt();
    QStringList knownMACs;
    QStringList knownEssids;
    QString keyPrefix;
    for( int i=1; i<= numNets; i++ ) {
        keyPrefix = "WirelessNetworks/"+QString::number(i);
        knownEssids.append( configIface->property( keyPrefix+"/ESSID" ).toString() );
        knownMACs.append( configIface->property(keyPrefix+"/AccessPoint").toString() );
    }

    QStringList availMACs;
    QStringList availEssids;
    for( int i = 0; i<results.count(); ++i ) {
        availMACs.append( results[i].data(WirelessNetwork::AP).toString() );
        availEssids.append( results[i].data(WirelessNetwork::ESSID).toString() );
    }

    const QString cEssid = scanner->currentESSID();
    //const QString cMAC = scanner->currentAccessPoint();
    if ( availEssids.contains( cEssid ) ) {
        return;
    } else {
        for( int i = 0; i< knownEssids.count(); ++i ) {
            int idx = availEssids.indexOf( knownEssids[i] );
            if ( idx < 0 ) //not in surrounding area
                continue;
            if ( knownMACs[i].isEmpty() ) { //this known net is not bound to specific AP
                qLog(Network) << "Emitting changeNetwork signal";
                emit changeNetwork();
                return;
            } else if ( knownMACs[i] == availMACs[idx] ) {
                //MAC and Essid are known and match
                //->we are within range of one of our specified nets
                qLog(Network) << "Emitting changeNetwork signal";
                emit changeNetwork();
                return;
            }
        }
    }
#endif
}

void RoamingMonitor::updateSignalStrength()
{
#if WIRELESS_EXT > 11
    if ( scanner ) {
        signalProvider->setSignalStrength( scanner->currentSignalStrength() );
    }
#endif
}

void RoamingMonitor::deviceNameChanged()
{
#if WIRELESS_EXT > 13
    deviceName = netSpace->value().toString();
#endif
}

QString RoamingMonitor::currentEssid() const
{
#if WIRELESS_EXT > 13
    if ( scanner )
        return scanner->currentESSID();
#endif
    return QString();
}

QString RoamingMonitor::currentMAC() const
{
#if WIRELESS_EXT > 13
    if ( scanner )
        scanner->currentAccessPoint();
#endif
    return QString();
}

void RoamingMonitor::activeNotification( bool enabled )
{
    activeHop = enabled;
    const bool autoConnect = configIface->property("WirelessNetworks/AutoConnect").toBool();
    if ( activeHop ) {
#if WIRELESS_EXT > 11
        signalTimer->start();
        signalProvider->setAvailability( QSignalSource::Available );
        updateSignalStrength();
#endif
        const int interval = configIface->property("WirelessNetworks/Timeout").toInt()*1000;
        if ( autoConnect )
            rescanTimer->start( interval );
    } else {
#if WIRELESS_EXT > 11
        signalTimer->stop();
        signalProvider->setAvailability( QSignalSource::NotAvailable );
#endif
        rescanTimer->stop();
    }
}
#endif // NO_WIRELESS_LAN
