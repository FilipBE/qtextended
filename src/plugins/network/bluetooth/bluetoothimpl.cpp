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

#include "bluetoothimpl.h"
#include "config.h"
#include "btdialupdevice.h"

#include <QByteArray>
#include <QProcess>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopianetwork.h>
#include <qvaluespace.h>

#include <qbluetoothremotedevicedialog.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothrfcommserialport.h>
#include <qbluetoothremotedevice.h>
#include <QCommDeviceSession>

#include <errno.h>
//#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define PPPD_BINARY "/usr/sbin/pppd"
static const QString pppScript = Qtopia::qtopiaDir()+"bin/btdun-network";

BluetoothImpl::BluetoothImpl( const QString& confFile )
    : state( Initialize ), configIface( 0 ), ifaceStatus( Unknown ), netSpace( 0 ), trigger( 0 ),
        dialupDev( 0 ), session(0), tidStateUpdate( 0 )
{
    qLog(Network) << "Creating BluetoothImpl instance";
    configIface = new BluetoothConfig( confFile );

    //update state of this interface after each script execution
    connect( &thread, SIGNAL(scriptDone()), this, SLOT(updateState()));
}

BluetoothImpl::~BluetoothImpl()
{
    if ( configIface )
        delete configIface;
    configIface = 0;
    qLog(Network) << "Deleting BluetoothImpl instance";

    if ( session )
        delete session;
}

QtopiaNetworkInterface::Status BluetoothImpl::status()
{
    if ( ifaceStatus == Unknown ) //not initialized
        return ifaceStatus;

    QtopiaNetworkInterface::Status status = Unavailable;
    if ( isAvailable() ) {
        status = Down;
        if ( ifaceStatus == Demand || ifaceStatus == Pending )
            status = ifaceStatus;
        else if ( isActive() )
            status = QtopiaNetworkInterface::Up;
    }

    ifaceStatus = status;
    netSpace->setAttribute( QLatin1String("State"), ifaceStatus );
    updateTrigger();
    return ifaceStatus;
}

void BluetoothImpl::initialize()
{
    if ( !netSpace ) { //initialize the value space for this interface
        QByteArray path("/Network/Interfaces/"+ QByteArray::number(qHash(configIface->configFile())) );
        netSpace = new QValueSpaceObject( path, this );
        netSpace->setAttribute( QLatin1String("Config"), configIface->configFile() );
        netSpace->setAttribute( QLatin1String("State"), QtopiaNetworkInterface::Unknown );
        netSpace->setAttribute( QLatin1String("Error"), QtopiaNetworkInterface::NotInitialized );
        netSpace->setAttribute( QLatin1String("ErrorString"), tr("Interface hasn't been initialized yet.") );
        netSpace->setAttribute( QLatin1String("NetDevice"), QVariant() );
        netSpace->setAttribute( QLatin1String("BtDevice"), QVariant() );
        netSpace->setAttribute( QLatin1String("UpdateTrigger"), 0 );
    }
    if ( !dialupDev ) {
        dialupDev = new BluetoothDialupDevice( this );
        connect( dialupDev, SIGNAL(deviceStateChanged()),
                this, SLOT(updateState()) );
        connect( dialupDev, SIGNAL(connectionEstablished()),
                this, SLOT(serialPortConnected()) );
    }

    if ( isAvailable() ) {
        ifaceStatus = Down;
    } else {
        ifaceStatus = Unavailable;
    }
    netSpace->setAttribute( QLatin1String("State"), ifaceStatus );
    updateTrigger();
}

void BluetoothImpl::cleanup()
{
    if ( ifaceStatus != QtopiaNetworkInterface::Unknown ) {
        ifaceStatus = QtopiaNetworkInterface::Unknown;
        netSpace->setAttribute( "State", ifaceStatus );
        updateTrigger();
    } else {
        return;
    }

    const QString peerName = configIface->property("Serial/PeerID").toString();
    if ( peerName.isEmpty() )
        return;

    //delete peer file
    qLog(Network) << "Deleting DUN peer file";
    QFile::remove( "/etc/ppp/peers/"+peerName );

    //delete chat files
    qLog(Network) << "Deleting connect chat script";
    const QString path = Qtopia::applicationFileName("Network","chat");
    QString chat = path + "/connect-" + peerName;
    qLog(Network) << QString("Deleting connect file (%1)").arg(chat);
    QFile::remove(chat);
}

bool BluetoothImpl::start( const QVariant /*options*/ )
{
     if ( ifaceStatus != QtopiaNetworkInterface::Down ) {
        switch ( ifaceStatus )
        {
            case QtopiaNetworkInterface::Unknown:
                updateTrigger( QtopiaNetworkInterface::NotInitialized,
                        tr("Interface hasn't been initialized yet.") );
                break;
            case QtopiaNetworkInterface::Unavailable:
                updateTrigger( QtopiaNetworkInterface::NotAvailable,
                        tr("Interface is not available.") );
                break;
            case QtopiaNetworkInterface::Up:
            case QtopiaNetworkInterface::Pending:
            case QtopiaNetworkInterface::Demand:
                updateTrigger( QtopiaNetworkInterface::NotConnected,
                        tr("Interface already started/active.") );
                break;
            default:
                break;
        }
        qLog(Network) << "ppp interface cannot be started "
            <<configIface->configFile();
        return false;
    }

    bool result = false;
    QBluetoothAddress addr( configIface->property(QLatin1String("Serial/PartnerDevice")).toString() );
    if ( addr.isValid() ) {
        session = QCommDeviceSession::session(QBluetoothLocalDevice().deviceName().toLatin1());
        if (!session) {
            updateTrigger( QtopiaNetworkInterface::UnknownError,
                           tr("Bluetooth Session failed.") );
            return false;
        }
        qLog(Network) << "Connecting to" << addr.toString();
        dialupDev->connectToDUNService( addr );

        result = true;

        ifaceStatus = QtopiaNetworkInterface::Pending;
        netSpace->setAttribute( "State", ifaceStatus );
        updateTrigger();
    } else {
        updateTrigger( QtopiaNetworkInterface::UnknownError,
                tr("No remote Bluetooth partner selected.") );
    }

    return result;
}

bool BluetoothImpl::stop()
{
    switch( ifaceStatus ) {
        case QtopiaNetworkInterface::Pending:
        case QtopiaNetworkInterface::Demand:
        case QtopiaNetworkInterface::Up:
            break;
        default:
            updateTrigger( QtopiaNetworkInterface::UnknownError,
                    tr("Interface is not active.") );
            return false;
    }

    //stop pppd
    QStringList args;
    args << "stop";
    args << pppIface;
    thread.addScriptToRun( pppScript, args );

    dialupDev->releaseDUNConnection();
    if (session) {
        session->endSession();
        delete session;
        session = 0;
    }

    if ( tidStateUpdate ) {
        killTimer( tidStateUpdate );
        tidStateUpdate = 0;
        state = Initialize;
        logIndex = 0;
    }

    ifaceStatus = QtopiaNetworkInterface::Down;
    netSpace->setAttribute( "State", ifaceStatus );
    pppIface = "";
    netSpace->setAttribute( "NetDevice", QByteArray() );
    updateTrigger();

    return true;
}

QString BluetoothImpl::device() const
{
    return pppIface;
}

bool BluetoothImpl::setDefaultGateway()
{
    if ( pppIface.isEmpty() ) {
        updateTrigger( QtopiaNetworkInterface::UnknownError,
                tr("Cannot set default gateway.") );
        qLog(Network) << "Cannot set default gateway";
        return false;
    }

    QStringList args;
    args << "route";
    args << pppIface;
    thread.addScriptToRun( pppScript, args );

    QStringList dns;
    dns << "install";
    dns << "dns";
    thread.addScriptToRun( pppScript, dns );
    return true;
}

QtopiaNetwork::Type BluetoothImpl::type() const
{
    return QtopiaNetwork::toType( configIface->configFile() );
}

QtopiaNetworkConfiguration * BluetoothImpl::configuration()
{
    return configIface;
}

void BluetoothImpl::setProperties( const QtopiaNetworkProperties& properties)
{
    configIface->writeProperties( properties );
}

bool BluetoothImpl::isAvailable() const
{
    if ( !dialupDev )
        return false;

    if ( dialupDev->isAvailable( deviceName ) )
        return true;

    deviceName = dialupDev->name();
    netSpace->setAttribute( "BtDevice", deviceName );
    if ( !deviceName.isEmpty()
            && dialupDev->isAvailable( deviceName ) ) {
        qLog(Network) << "BluetoothImpl: Using" << deviceName << "for DUN client";
        return true;
    }
    qLog(Network) << "BluetoothImpl: Cannot find bluetooth device. device:"
        <<deviceName << dialupDev->isAvailable( deviceName );
    return false;
}

bool BluetoothImpl::isActive() const
{
    if ( !dialupDev )
        return false;

    if ( !dialupDev->hasActiveConnection() ) {
        qLog(Network) << "BluetoothImpl::isActive: no rfcomm serial device";
        return false;
    }

    if ( pppIface.isEmpty() ) {
        qLog(Network) << "BluetoothImpl::isActive: no PPP connection active";
        return false;
    }

    //test IPv4 and IPv6
    int sockets[2] ;
    sockets[0] = socket( PF_INET, SOCK_DGRAM, 0 );
    sockets[1] = socket( PF_INET6, SOCK_DGRAM, 0 );

    for ( int i = 0; i<2; i++ ) {
        struct ifreq ifreqst;
        strcpy( ifreqst.ifr_name, pppIface.data() );
        int ret = ioctl( sockets[i], SIOCGIFFLAGS, &ifreqst );
        if ( ret == -1 ) {
            int error = errno;
            qLog(Network) << "BluetoothImpl::isActive: ioctl: " << strerror( error );
            continue;
        }

        int flags = ifreqst.ifr_flags;
        if ( ( flags & IFF_UP ) == IFF_UP  &&
                (flags & IFF_LOOPBACK) != IFF_LOOPBACK &&
                (flags & IFF_POINTOPOINT) == IFF_POINTOPOINT ) {
            qLog(Network) << "BluetoothImpl::isActive: " <<pppIface<<" is up and running";

            ::close( sockets[0] );
            ::close( sockets[1] );
            return true;
        }
    }

    qLog(Network) << "BluetoothImpl::isActive: interface " << pppIface <<" is offline" ;
    ::close( sockets[0] );
    ::close( sockets[1] );
    return false;
}

void BluetoothImpl::updateTrigger( QtopiaNetworkInterface::Error error, const QString& desc )
{
    if ( !netSpace )
        return;
    //we have to use a trigger value because a new error (of same type) wouldn't trigger the contentsChanged()
    //signal
    trigger = (trigger+1)%256;
    if ( !desc.isEmpty() )
        netSpace->setAttribute( QLatin1String("ErrorString"), desc );
    netSpace->setAttribute( QLatin1String("Error"), error );
    netSpace->setAttribute( QLatin1String("UpdateTrigger"), trigger );
}

void BluetoothImpl::updateState()
{
    status();
}

/*!
  This function is part of the startup procedure and is called once the
  rfcomm device has been created.
  */
void BluetoothImpl::serialPortConnected( )
{
    const QByteArray tty = dialupDev->rfcommDevice();
    if ( tty.isEmpty() || !QFile::exists( tty ) ) {
        if ( !tty.isEmpty() ) {
            qLog(Network) << "Invalid rfcomm device:" << tty;
            dialupDev->releaseDUNConnection();
        } else {
            qLog(Network) << "Empty rfcomm device";
        }

        if (session) {
            session->endSession();
            delete session;
            session = 0;
        }

        ifaceStatus = QtopiaNetworkInterface::Down;
        netSpace->setAttribute( "State", ifaceStatus );
        updateTrigger(QtopiaNetworkInterface::UnknownError,
                tr("Cannot bind Bluetooth DUN to serial port.") );
        status();
        return;
    }

    const QString peerName = configIface->property("Serial/PeerID").toString();
    QStringList params;

    if ( tty.startsWith("/dev/") )
        params << tty.mid( 5 /*strlen("/dev/")*/ );
    else
        params << tty;
    params << QLatin1String("updetach");
    params << QLatin1String("debug"); //enable debugging output

    const QString logfile = Qtopia::tempDir() + "qpe-pppd-log-"+ peerName;
    params << "logfile";
    params << logfile;
    QFile::remove( logfile ) ;

    params << "call";
    params << peerName;

    qLog(Network) << "###################################";
    qLog(Network) << "Starting ppp using " << tty;
    qLog(Network) << "using parameter:" << params.join(QString(" ") );
    params.prepend( QLatin1String("start") );
    thread.addScriptToRun( pppScript, params );

    state = Initialize;
    logIndex = 0;
    tidStateUpdate = startTimer( 1000 );
}

void BluetoothImpl::timerEvent( QTimerEvent* /*e*/)
{
    const QString peerID = configIface->property("Serial/PeerID").toString();
    const QString logfile = Qtopia::tempDir() + "qpe-pppd-log-"+peerID;

    if (state == Disappearing) {

        if ( tidStateUpdate )
            killTimer( tidStateUpdate );
        tidStateUpdate = 0;
        state = Initialize;
        logIndex = 0;
        dialupDev->releaseDUNConnection();

        if (session) {
            session->endSession();
            delete session;
            session = 0;
        }

        updateState();
        qLog(Network) << "Stopping ppp script monitor";
        return;
    }

    QFile logf( logfile );
    if ( logf.open(QIODevice::ReadOnly) ) {
        QString log = logf.readAll();
        log.replace(QRegExp("\r"),"");
        if ( logIndex > log.length() )
            logIndex = 0;
        switch ( state ) {
            case Initialize:
            {
                int i = 0;
                if ( log.indexOf(QRegExp("\nFailed"), logIndex)>=logIndex
                    || log.indexOf(QRegExp("is locked by pid"), logIndex) >= logIndex
                    || log.indexOf(QRegExp("Connect script failed"), logIndex) >= logIndex
                    || log.indexOf(QRegExp("Connection refused"), logIndex) >= logIndex )
                {
                    state = Disappearing;
                    ifaceStatus = QtopiaNetworkInterface::Unavailable;
                    netSpace->setAttribute( "State", ifaceStatus );
                    updateTrigger( QtopiaNetworkInterface::UnknownError,
                            tr("Connect chat failed.") );
                    qLog(Network) << "Connect chat failed";
                    return;
                }
                if ( log.indexOf(QRegExp("\nNO DIALTONE"), logIndex)>=logIndex
                    || log.indexOf(QRegExp("\nNO CARRIER"), logIndex)>=logIndex
                    || log.indexOf(QRegExp("\nERROR"), logIndex)>=logIndex
                    || log.indexOf(QRegExp("\nBUSY"), logIndex)>=logIndex )
                {
                    ifaceStatus = QtopiaNetworkInterface::Unavailable;
                    netSpace->setAttribute( "State", ifaceStatus );
                    updateTrigger( QtopiaNetworkInterface::UnknownError,
                            tr("Connect chat not responding.") );
                    qLog(Network) << "Connect chat not responding";
                    state = Disappearing;
                } else if ( (i=log.indexOf(QRegExp("Using interface ppp.*\n"), logIndex))>= logIndex ) {
                    //determine iface name;
                    int end = log.indexOf(QRegExp("\n"), i);
                    int begin = log.indexOf(QRegExp("ppp"), i);
                    pppIface = log.mid(begin, end-begin).toLatin1();
                    netSpace->setAttribute( "NetDevice", pppIface );
                    qLog(Network) << "using interface: " << pppIface;
                    logIndex = end;
                    state = Connect;
                }
            }
                break;
            case Connect:
                {
                    int i = 0;
                    if ( (i=log.indexOf(QRegExp("\nPAP authentication failed"), logIndex))>=logIndex
                        || (i=log.indexOf(QRegExp("\nCHAP authentication failed"), logIndex))>=logIndex
                        || (i=log.indexOf(QRegExp("\nConnection terminated."), logIndex)) >=logIndex)
                    {
                        state = Disappearing;
                        pppIface = QByteArray();
                        netSpace->setAttribute( "NetDevice", pppIface );
                        ifaceStatus = QtopiaNetworkInterface::Unavailable;
                        netSpace->setAttribute( "State", ifaceStatus );
                        updateTrigger( QtopiaNetworkInterface::UnknownError,
                                tr("Authentication failed.") );
                        qLog(Network) << "Authentication failed";
                    } else if ( (i=log.indexOf(QRegExp("\nLocal IP address changed"), logIndex)) >= logIndex
                            || (i=log.indexOf(QRegExp("\nlocal  IP address"), logIndex)) >= logIndex )
                    {
                        logIndex = i;
                        ifaceStatus = QtopiaNetworkInterface::Up;
                        netSpace->setAttribute( "State", ifaceStatus );
                        updateTrigger();
                        state = Monitoring;
                        QStringList dns;
                        dns << "install";
                        dns << "dns";
                        thread.addScriptToRun( pppScript, dns );
                    } else if ( (i=log.indexOf(QRegExp("\nConnect script failed"), logIndex)) >= logIndex )
                    {
                        logIndex = i;
                        state = Disappearing;
                        pppIface = QByteArray();
                        netSpace->setAttribute( "NetDevice", pppIface );
                        ifaceStatus = QtopiaNetworkInterface::Unavailable;
                        netSpace->setAttribute( "State", ifaceStatus );
                        updateTrigger( QtopiaNetworkInterface::UnknownError,
                                tr("Connect failed.") );
                        qLog(Network) << "Connect failed";
                    }

                }
                break;
            case Monitoring:
                {
                    int i;
                    if ( (i=log.indexOf(QRegExp("\nConnection terminated"), logIndex)) >= logIndex ) {
                        logIndex = i++;
                        qLog(Network) << "Connection terminated";
                        pppIface = QByteArray();
                        netSpace->setAttribute( "NetDevice", pppIface );
                        ifaceStatus = QtopiaNetworkInterface::Unavailable;
                        netSpace->setAttribute( "State", ifaceStatus );
                        state = Disappearing;
                        updateTrigger();
                        QtopiaNetwork::unsetDefaultGateway( configIface->configFile() );
                    }
                }
                break;
            default:
                break;
        }
    }
}

