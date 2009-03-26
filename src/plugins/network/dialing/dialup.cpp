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

#include "dialup.h"
#include "config.h"
#include "dialstring.h"

#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>

#include <QDebug>
#include <QFile>

#include <errno.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define PPPD_BINARY QString("/usr/sbin/pppd")
static const QString pppScript = Qtopia::qtopiaDir()+"bin/ppp-network";

DialupImpl::DialupImpl( const QString& confFile)
    : state( Initialize ), configIface(0), ifaceStatus(Unknown), tidStateUpdate(0)
#ifdef QTOPIA_CELL
    , regState(QTelephony::RegistrationUnknown), callManager( 0 ), 
    netReg( 0 ), pppdProcessBlocked( false )
#endif
    , netSpace( 0 ), delayedGatewayInstall( false )
{
    qLog(Network) << "Creating DialupImpl instance";
    configIface = new DialupConfig( confFile );

    //update state of this interface after each script execution
    connect( &thread, SIGNAL(scriptDone()), this, SLOT(updateState()));
}

DialupImpl::~DialupImpl()
{
    qLog(Network) << "Deleting DialupImpl instance";
    if (configIface)
        delete configIface;
    configIface = 0;
}

QtopiaNetworkInterface::Status DialupImpl::status()
{
    if ( ifaceStatus == QtopiaNetworkInterface::Unknown) {
        return ifaceStatus;
    }

    QtopiaNetworkInterface::Status status = QtopiaNetworkInterface::Unavailable;
    if ( isAvailable() ) {
        status = QtopiaNetworkInterface::Down;

        if ( ifaceStatus == QtopiaNetworkInterface::Pending ||
                ifaceStatus == QtopiaNetworkInterface::Demand )
            // these states are updated by timerEvent
            status = ifaceStatus;
        else if ( isActive() )
            status = QtopiaNetworkInterface::Up;
    }

    ifaceStatus = status;
    netSpace->setAttribute( "State", ifaceStatus );
    updateTrigger();
    return ifaceStatus;
}

void DialupImpl::initialize()
{
    if ( !netSpace ) {
        const uint ident = qHash( configIface->configFile() );
        QString path = QString("/Network/Interfaces/%1").arg(ident);
        netSpace = new QValueSpaceObject( path, this );
        netSpace->setAttribute( "Config", configIface->configFile() );
        netSpace->setAttribute( "State", QtopiaNetworkInterface::Unknown );
        netSpace->setAttribute( "Error", QtopiaNetworkInterface::NotInitialized );
        netSpace->setAttribute( "ErrorString", tr("Interface hasn't been initialized yet.") );
        netSpace->setAttribute( "NetDevice", QVariant() );
        netSpace->setAttribute( "UpdateTrigger", 0 );
    }

    if ( isAvailable() ) {
        ifaceStatus = QtopiaNetworkInterface::Down;
        qLog(Network) << "DialupImpl: Using serial device: " << device();
    } else {
        ifaceStatus = QtopiaNetworkInterface::Unavailable;
        qLog(Network) << "DialupImpl: interface not available";
    }

    netSpace->setAttribute( "State", ifaceStatus );
    updateTrigger();
#ifdef QTOPIA_CELL
    QtopiaNetwork::Type t = type();
    if ( t & QtopiaNetwork::PhoneModem ) {
        //we cannot start an internal GPRS connection if we don't have a registered network
        //initiate monitoring of registration changes
        commManager = new QCommServiceManager( this );
        connect( commManager, SIGNAL(servicesChanged()), this, SLOT(registrationStateChanged()) );
        registrationStateChanged(); //ask for first update
    }
#endif

}

void DialupImpl::cleanup()
{
    if ( ifaceStatus != QtopiaNetworkInterface::Unknown ) {
        ifaceStatus = QtopiaNetworkInterface::Unknown;
        netSpace->setAttribute( "State", ifaceStatus );
        updateTrigger();
    } else {
        return;
    }

    //delete peer file
    qLog(Network) << "Deleting peer file";
    QStringList params;
    params << "cleanup";
    params << "peer";
    const QString peerName = configIface->property("Serial/PeerID").toString();
    if ( peerName.isEmpty() )
        return;
    params << peerName;
    thread.addScriptToRun( pppScript, params );

    //reversing to standard dns server
    qLog(Network) << "deinstalling DNS server";
    params.clear();
    params << "cleanup";
    params << "dns";
    thread.addScriptToRun( pppScript, params );

    //delete chat files
    const QString path = Qtopia::applicationFileName("Network","chat");
    QString chat = path + "/connect-" + peerName;
    qLog(Network) << QString("Deleting connect file (%1)").arg(chat);
    QFile::remove(chat);
    chat = path + "/disconnect-" + peerName;
    qLog(Network) << QString("Deleting disconnect file (%1)").arg(chat);
    QFile::remove(chat);

    //config file will be deleted by qtopia network server
}

bool DialupImpl::setDefaultGateway()
{
    if ( pppIface.isEmpty() ) {
        updateTrigger( QtopiaNetworkInterface::UnknownError,
                tr("Cannot set default gateway.") );
        qLog(Network) << "Cannot set default gateway";
        return false;
    }

    QStringList params;
    params << "install";
    params << "dns";
    if ( configIface->property("Serial/UsePeerDNS").toString() == "n" ) {
        params << configIface->property("Properties/DNS_1").toString();
        params << configIface->property("Properties/DNS_2").toString();
    }
    thread.addScriptToRun( pppScript, params );


    qLog(Network) << "Settings default gateway to" <<configIface->configFile();
    QStringList args;
    args << "route";
    args << pppIface;
    thread.addScriptToRun( pppScript, args );

    return true;
}

bool DialupImpl::start( const QVariant /*options*/ )
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

    QtopiaNetworkProperties prop = configIface->getProperties();
    QStringList args;

    const bool demand = prop.value("Properties/Autostart").toString() == "y";
    const QtopiaNetwork::Type t = type();
#ifdef QTOPIA_CELL
    if ( t & QtopiaNetwork::PhoneModem ) {
        if ( regState != QTelephony::RegistrationHome &&
                regState != QTelephony::RegistrationRoaming ) {
            //we want to start the network but we don't have a network
            //registration yet. wait till registration is done then go ahead
            //and start iface
            qLog(Network) << "Cannot start dialup yet due to missing"
                << "network registration. try again later";
            registrationStateChanged(); //ask for first update
            updateTrigger( QtopiaNetworkInterface::NotConnected,
                    tr("Missing network registration. Try again later.") );
            return false;

        }
    }
#endif
    if ( t & QtopiaNetwork::NamedModem || t & QtopiaNetwork::PCMCIA ) {
        QString dev = device();
        if ( dev.isEmpty() || dev == "internal") {
            qLog(Network) << "No external device";
            updateTrigger( QtopiaNetworkInterface::NotConnected,
                    tr("Missing external serial device.") );
            return false;
        }
        if ( dev.startsWith("/dev/") )
            dev = dev.mid(5);
        args << dev;
        args << "updetach";
    }
#ifdef QTOPIA_CELL
    else {
        args << "nodetach";
    }
#endif

    args << "debug";
    args << "call";
    const QString peerID = prop.value("Serial/PeerID").toString();
    if ( peerID.isEmpty() ) {
        qLog(Network) << "Missing peer ID";
        updateTrigger( QtopiaNetworkInterface::NotConnected,
                tr("Missing peer ID. Please reconfigure device.") );
        return false;
    }
    args << peerID;

    QString password = prop.value("Properties/Password").toString();
    if (!password.isEmpty()) {
        args << "password";
        args << password;
    }

    const QString logfile = Qtopia::tempDir() + "qpe-pppd-log-"+peerID;
    args << "logfile";
    args << logfile;
    QFile::remove( logfile ) ;

    if ( t & QtopiaNetwork::NamedModem || t & QtopiaNetwork::PCMCIA ) {
        qLog(Network) << "###################################";
        qLog(Network) << "Starting ppp using external modem: " <<
            args.join(" ");
        args.prepend("start"); //cmd for network script

        thread.addScriptToRun( pppScript, args );
    } else { //QtopiaNetwork::PhoneModem
#ifdef QTOPIA_CELL
        const QString path = Qtopia::applicationFileName("Network", "chat");
        const QString connectF = path+"/connect-"+peerID;
        const QString disconnectF = path+"/disconnect-"+peerID;


        //Create a data call
        qLog(Network) << "###################################";
        qLog(Network) << "Starting ppp using internal modem: " <<
            args.join(" ");
        qLog(Network) << "connect: " << connectF;
        qLog(Network) << "disconnect: " << disconnectF;
        qLog(Network) << "demand dialing: " << (demand ? "yes" : "no");

        QDialOptions pppd;
        pppd.setUseIpModule( true );
        pppd.setIpProgramName( PPPD_BINARY );

        pppd.setIpConnectScript( connectF );
        pppd.setIpDisconnectScript( disconnectF );
        pppd.setIpDemandDialing( demand );
        pppd.setIpArgs( args );

        //Make sure that an existing call is stopped.
        if ( !dataCall.isNull() ) {
            dataCall.hangup();
        }

        if ( !callManager )
            callManager = new QPhoneCallManager( this );
        //Dial the new call.
        if ( callManager ) {
            dataCall = callManager->create( "IP" );
            dataCall.connectNotification( this, 
                    SLOT(connectNotification(QPhoneCall,QPhoneCall::Notification,QString)));
            dataCall.connectStateChanged( this, SLOT(phoneCallStateChanged(QPhoneCall)) );

            dataCall.dial( pppd );
        } else {
            qLog(Network) << "No call manager created";
        }
#endif
    }

    state = Initialize;
    logIndex = 0;
    tidStateUpdate = startTimer( 1000 );
    if ( demand )
        ifaceStatus = QtopiaNetworkInterface::Demand;
    else
        ifaceStatus = QtopiaNetworkInterface::Pending;
    netSpace->setAttribute( "State", ifaceStatus );
    updateTrigger();
    delayedGatewayInstall = true;
    return true;
}

bool DialupImpl::stop()
{
    switch( ifaceStatus ) {
        case QtopiaNetworkInterface::Pending:
        case QtopiaNetworkInterface::Demand:
        case QtopiaNetworkInterface::Up:
            break;
        //case QtopiaNetworkInterface::Unknown:
        //case QtopiaNetworkInterface::Unavailable:
        //case QtopiaNetworkInterface::Down:
        default:
            updateTrigger( QtopiaNetworkInterface::UnknownError,
                    tr("Device is not active.") );
            return true;
    }
    const QtopiaNetwork::Type t = type();
    if ( t & QtopiaNetwork::NamedModem || t & QtopiaNetwork::PCMCIA ) {
        QStringList args;
        args << "stop";
        args << pppIface;
        thread.addScriptToRun( pppScript, args );
    } else {
#ifdef QTOPIA_CELL
        qLog(Network) << "stopping data call on phone line";
        if ( ! dataCall.isNull() ) {
            dataCall.hangup();
        }
#endif
    }

    if ( tidStateUpdate ) {
        killTimer( tidStateUpdate );
        tidStateUpdate = 0;
        state = Initialize;
        logIndex = 0;
    }
    pppIface = QString();
    netSpace->setAttribute( "NetDevice", QString() );
    ifaceStatus = QtopiaNetworkInterface::Down;
#ifdef QTOPIA_CELL
    if ( t & QtopiaNetwork::PhoneModem )  //internal phone device
        pppdProcessBlocked = true;
#endif

    status();

    updateTrigger();
    return true;
}

QString DialupImpl::device() const
{
    const QtopiaNetwork::Type t = type();
    if ( t & QtopiaNetwork::NamedModem || t & QtopiaNetwork::PCMCIA )
        return deviceName; //e.g. /dev/ttyS0, /dev/ircomm
    else
        return QString("internal"); //no tr
}

QtopiaNetwork::Type DialupImpl::type() const
{
    return QtopiaNetwork::toType( configIface->configFile() );
}

bool DialupImpl::isAvailable() const
{
    const QtopiaNetwork::Type t = type();

#ifdef QTOPIA_CELL
    if ( t & QtopiaNetwork::PhoneModem ) { //use internal phone device
        qLog(Network) << "DialupImpl: Using internal serial device";
        deviceName = QString();
        if ( (regState == QTelephony::RegistrationHome ||
             regState == QTelephony::RegistrationRoaming) && !pppdProcessBlocked ) {
            return true;
        }
        if ( pppdProcessBlocked )
            qLog(Network) << "pppd manager blocked";
        return false;
    }
#endif

    if ( t & QtopiaNetwork::NamedModem ) {
        const QString d = configIface->property("Serial/SerialDevice").toString();
        if ( QFile::exists(d) ) {
            deviceName = d;
            return true;
        }
        deviceName = QString();
        return false;
    }

    if ( t & QtopiaNetwork::PCMCIA ) {
        //check serial pcmcia cards
        /*****************************/
        const QString cardType = "serial"; //pcmcia modem/serial card

        FILE* f = fopen("/var/run/stab", "r");
        if (!f) f = fopen("/var/state/pcmcia/stab", "r");
        if (!f) f = fopen("/var/lib/pcmcia/stab", "r");

        if ( f ) {
            char line[1024];
            char devtype[1024];
            qLog(Network) << "DialupImpl: Searching for pcmcia serial/modem card...";
            while ( fgets( line, 1024, f ) ) {
                if ( sscanf(line,"%*d %s %*s", devtype )==1 )
                {
                    if ( cardType == devtype)
                    {
                        QString l(line);
                        QStringList list = l.split("\t");

                        if (!deviceName.isEmpty()) { // we know this device already
                            if ( QString("/dev/%1").arg(list[4].simplified()) == deviceName) {
                                qLog(Network) << "DialupImpl: Using serial device on "<< deviceName;
                                fclose(f);
                                return true;
                            }
                        } else { // new device - remember name
                            //Assumption: this device is the only device of
                            //its type
                            fclose(f);
                            deviceName = QString("/dev/%1").arg(list[4].simplified());
                            if ( QFile::exists( deviceName ) ) {
                                qLog(Network) << "DialupImpl: Found new serial device on /dev/" << deviceName;
                                return true;
                            }
                        }
                    }
                }
            }
            fclose(f);
        }
    }

    deviceName = QString();
    qLog(Network) << "DialupImpl: No serial/modem device found";
    return false;
}

bool DialupImpl::isActive() const
{

    if ( pppIface.isEmpty() ||  device().isEmpty() ) {
        qLog(Network) << "DialupImpl::isActive: no PPP connection active";
        return false;
    }

    //test IPv4 and IPv6
    int sockets[2];
    sockets[0] = socket( PF_INET, SOCK_DGRAM, 0 );
    sockets[1] = socket( PF_INET6, SOCK_DGRAM, 0 );

    for ( int i = 0; i<2; i++ ) {
        struct ifreq ifreqst;
        strcpy( ifreqst.ifr_name, pppIface.toLatin1().data() );
        int ret = ioctl( sockets[i], SIOCGIFFLAGS, &ifreqst );
        if ( ret == -1 ) {
            int error = errno;
            qLog(Network) << "DialupImpl::isActive: ioctl: " << strerror( error );
            continue;
        }



        int flags = ifreqst.ifr_flags;
        if ( ( flags & IFF_UP ) == IFF_UP  &&
                (flags & IFF_LOOPBACK) != IFF_LOOPBACK &&
                (flags & IFF_POINTOPOINT) == IFF_POINTOPOINT ) {
            qLog(Network) << "DialupImpl::isActive: " <<pppIface<<" is up and running";
            ::close( sockets[0] );
            ::close( sockets[1] );
            return true;
        }
    }

    ::close( sockets[0] );
    ::close( sockets[1] );
    qLog(Network) << "DialupImpl::isActive: interface " << pppIface <<" is offline" ;
    return false;
}

QtopiaNetworkConfiguration * DialupImpl::configuration()
{
    return configIface;
}

void DialupImpl::setProperties( const QtopiaNetworkProperties& properties )
{
    configIface->writeProperties(properties);
}

#ifdef QTOPIA_CELL
/*!
  This function handles registration queries and notifications
*/
void DialupImpl::registrationStateChanged()
{
    if ( commManager->supports<QNetworkRegistration>().contains( "modem" ) ) {
        if ( !netReg ) {
            netReg = new QNetworkRegistration( "modem", this );
            connect( netReg, SIGNAL(registrationStateChanged()),
                     this, SLOT(registrationStateChanged()) );
        }
    } else if ( netReg ){
        qLog(Network) << "lost modem network registration";
        delete netReg;
        netReg = 0;
        regState = QTelephony::RegistrationUnknown;
        status();
        return;
    } else {
        qLog(Network) << "No modem network registration available yet";
        return;
    }

    QTelephony::RegistrationState oldState = regState;
    const QtopiaNetwork::Type t = this->type();
    if ( t & QtopiaNetwork::PhoneModem  && netReg ) {
        regState = netReg->registrationState();
        switch ( regState ) {
            case QTelephony::RegistrationHome:
            case QTelephony::RegistrationRoaming:
                // we have network registration and can start dialup
                if ( regState != oldState ) {
                    qLog(Network) << "Network registered - Dialup can be started";
                    status();
                }
                break;
            case QTelephony::RegistrationDenied:
            default:
            case QTelephony::RegistrationNone:
            case QTelephony::RegistrationUnknown:
            case QTelephony::RegistrationSearching:
                if ( regState != oldState ) {
                    status();
                    qLog(Network) << "Missing network registration for Dialup";
                }
                break;
        }
    }
}
#endif


void DialupImpl::timerEvent( QTimerEvent* /*e*/)
{
    const QString peerID = configIface->property("Serial/PeerID").toString();
    const QString logfile = Qtopia::tempDir() + "qpe-pppd-log-"+peerID;
    const bool demand = configIface->property("Properties/Autostart").toString() == "y";

    if (state == Disappearing) {

        if ( tidStateUpdate )
            killTimer( tidStateUpdate );
        tidStateUpdate = 0;
        state = Initialize;
        logIndex = 0;
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
                    || log.indexOf(QRegExp("Connect script failed"), logIndex) >= logIndex )
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
                    state = Disappearing;
                    qLog(Network) << "Connect chat not responding";
                } else if ( (i=log.indexOf(QRegExp("Using interface ppp.*\n"), logIndex))>= logIndex ) {
                    //determine iface name;
                    int end = log.indexOf(QRegExp("\n"), i);
                    int begin = log.indexOf(QRegExp("ppp"), i);
                    pppIface = log.mid(begin, end-begin);
                    netSpace->setAttribute( "NetDevice", pppIface );
                    qLog(Network) << "using interface: " << pppIface;
                    logIndex = end;
                    if ( !demand )
                       state = Connect;
                } else if ( demand ) {
                    if ( (i=log.indexOf(QRegExp("Starting link\n"), logIndex))>=logIndex ) {
                        ifaceStatus = QtopiaNetworkInterface::Pending;
                        netSpace->setAttribute( "State", ifaceStatus );
                        updateTrigger();
                        logIndex = i++;
                        state = Connect;
                    }
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
                        pppIface = QString();
                        netSpace->setAttribute( "NetDevice", QString() );
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
                        QtopiaNetwork::setDefaultGateway( configIface->configFile() );
                    } else if ( (i=log.indexOf(QRegExp("\nConnect script failed"), logIndex)) >= logIndex )
                    {
                        logIndex = i;
                        if ( demand ) {
                            state = Initialize;
                            ifaceStatus = QtopiaNetworkInterface::Demand;
                            netSpace->setAttribute( "State", ifaceStatus );
                            updateTrigger();
                        } else {
                            state = Disappearing;
                            pppIface = QString();
                            netSpace->setAttribute( "NetDevice", QString() );
                            ifaceStatus = QtopiaNetworkInterface::Unavailable;
                            netSpace->setAttribute( "State", ifaceStatus );
                            updateTrigger( QtopiaNetworkInterface::UnknownError,
                                    tr("Connect failed.") );
                            qLog(Network) << "Connect failed";
                        }
                    }

                }
                break;
            case Monitoring:
                {
                    int i;
                    if ( (i=log.indexOf(QRegExp("\nConnection terminated"), logIndex)) >= logIndex ) {
                        logIndex = i++;
                        qLog(Network) << "Connection terminated";
                        if ( demand ) {
                            state = Initialize;
                            ifaceStatus = QtopiaNetworkInterface::Demand;
                            netSpace->setAttribute( "State", ifaceStatus );
                            updateTrigger();
                        } else {
                            pppIface = QString();
                            ifaceStatus = QtopiaNetworkInterface::Unavailable;
                            netSpace->setAttribute( "NetDevice", QString() );
                            netSpace->setAttribute( "State", ifaceStatus );
                            state = Disappearing;
                            updateTrigger();
                            QtopiaNetwork::unsetDefaultGateway( configIface->configFile() );
                        }
                    }
                }
                break;
            default:
                break;
        }

    }
}

void DialupImpl::updateTrigger( QtopiaNetworkInterface::Error code, const QString& desc )
{
    if ( !netSpace )
        return;
    trigger = (++trigger)%256;
    if ( !desc.isEmpty() ) //do not override old error string if nothing to report
        netSpace->setAttribute( "ErrorString", desc );
    netSpace->setAttribute( "Error", code );
    netSpace->setAttribute( "UpdateTrigger", trigger );
}

void DialupImpl::updateState()
{
    status();
    if ( delayedGatewayInstall ) {
        if ( ifaceStatus == QtopiaNetworkInterface::Up ) {
            QtopiaNetwork::setDefaultGateway( configIface->configFile() );
            delayedGatewayInstall = false;
        } else if ( ifaceStatus == QtopiaNetworkInterface::Down
                || ifaceStatus == QtopiaNetworkInterface::Unavailable ) {
            // do not update gateway when we suddenly drop out during the startup of the network
            delayedGatewayInstall = false;
        } // else { //wait until we are online }
    }
}

#ifdef QTOPIA_CELL
void DialupImpl::connectNotification( const QPhoneCall&,
        QPhoneCall::Notification notification, const QString& value)
{
    if ( notification == QPhoneCall::DataStateUpdate )
    {

        QPhoneCall::DataState dState = QPhoneCall::parseDataState( value );
        if ( dState & QPhoneCall::PPPdStopped ) {
            pppdProcessBlocked = false;
            status();
        }
        if ( qLogEnabled( Network ) ) {
            QString s = "DataCall ";
            if ( dState & QPhoneCall::PPPdStarted )
                s+="started ";
            if ( dState & QPhoneCall::PPPdStopped )
                s+="stopped ";
            if ( dState & QPhoneCall::PPPdFailed )
                s+="failed ";
            if ( dState & QPhoneCall::DataActive )
                s+="dataActive ";
            if ( dState & QPhoneCall::DataInactive )
                s+="dataInactive ";
            if ( dState & QPhoneCall::Connecting )
                s+="connecting ";
            if ( dState & QPhoneCall::ConnectFailed )
                s+="connectFailed ";
            qLog(Network) << "Data state: " << s;
        }
    } 
}

void DialupImpl::phoneCallStateChanged( const QPhoneCall& call)
{
    if ( (int)call.state()  >= (int) QPhoneCall::HangupLocal  && !pppdProcessBlocked ) {
        //if the call hangs up/aborts w/o being stopped manually by the user we need to cleanup 
        //in order to prevent that pppdProcessBlocked blocks the interface forever
        if ( tidStateUpdate ) {
            killTimer( tidStateUpdate );
            tidStateUpdate = 0;
            state = Initialize;
            logIndex = 0;
        }
        pppIface = QString();
        netSpace->setAttribute( "NetDevice", QString() );
        ifaceStatus = QtopiaNetworkInterface::Down;
        status();
    }
    qLog(Network) << "Call state: " << call.state();
}
#endif
