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

#include "vendor_wavecom_p.h"
#include <qatresultparser.h>
#include <qatutils.h>

// Time to wait between seeing a "+STIN" notification and sending
// the "+STGI" command to get the command information.
#define STGI_TIMEOUT        2000

WavecomCallProvider::WavecomCallProvider( WavecomModemService *service )
    : QModemCallProvider( service )
{
    connect( service, SIGNAL(callReleased(uint)),
             this, SLOT(callReleased(uint)) );
    connect( service, SIGNAL(callingPartyAlerting()),
             this, SLOT(callingPartyAlerting()) );
}

WavecomCallProvider::~WavecomCallProvider()
{
}

QModemCallProvider::AtdBehavior WavecomCallProvider::atdBehavior() const
{
    // The wavecom reports OK to ATD when the call has connected.
    return AtdOkIsConnect;
}

void WavecomCallProvider::abortDial( uint , QPhoneCall::Scope )
{
    // We need to use "ATH" to abort outgoing calls on wavecom.
    atchat()->send( "ATH" );
}

void WavecomCallProvider::callReleased( uint index )
{
    // Handle the call end notification: "+WIND: 6,<index>"
    QModemCall *call = callForIdentifier( index );
    if ( !call )
        return;
    if ( call->state() == QPhoneCall::Dialing ||
         call->state() == QPhoneCall::Alerting ) {

        // We never managed to connect.
        hangupRemote( call );

    } else if ( call->state() == QPhoneCall::Connected ||
                call->state() == QPhoneCall::Hold ) {

        // This is an indication that the connection has been lost.
        hangupRemote( call );

    } else if ( call->state() == QPhoneCall::Incoming ) {

        // This is an indication that an incoming call was missed.
        call->setState( QPhoneCall::Missed );

    } else {

        // Call ended for some other reason - assume remote hangup.
        hangupRemote( call );

    }
}

void WavecomCallProvider::callingPartyAlerting()
{
    // Handle the call alerting notification: "+WIND: 2"
    QModemCall *call = dialingCall();
    if ( call )
        call->setState( QPhoneCall::Alerting );
}

WavecomSimToolkit::WavecomSimToolkit( QModemService *service )
    : QModemSimToolkit( service )
{
    inputFormat = 0;
    lastCommand.setType( QSimCommand::NoCommand );
    pendingCommand = 0;     // Wavecom SetupMenu
    supportsStk = false;
    seenBegin = false;
    awaitingCommand = false;
    getTimeout = STGI_TIMEOUT;
    getTimer = new QTimer( this );
    getTimer->setSingleShot( true );
    QObject::connect( getTimer, SIGNAL(timeout()),
                      this, SLOT(getCommandTimeout()) );
    service->primaryAtChat()->registerNotificationType
        ( "+STIN:", this, SLOT(stinNotification(QString)) );
}

WavecomSimToolkit::~WavecomSimToolkit()
{
}

void WavecomSimToolkit::initialize()
{
    // Send our list of supported facilities to the SIM and
    // attempt to activate the application contained therein.
    initializationStarted();
    atchat()->chat( "AT+STSF=2,\"5FFFFFFF7F\",6",
                    this, SLOT(configureDone(bool)) );
    lastCommand.setType( QSimCommand::NoCommand );
    pendingCommand = 0;     // Wavecom SetupMenu
    getTimeout = STGI_TIMEOUT;
}

void WavecomSimToolkit::begin()
{
    if ( !supportsStk ) {

        // SIM toolkit functionality is not available.
        emit beginFailed();

    } else if ( lastCommand.type() == QSimCommand::SetupMenu ) {

        // We just fetched the main menu, so return what we fetched.
        emit command( lastCommand );

    } else if ( !awaitingCommand ) { // Don't fetch again if already fetching.

        // Force the current session, if any, to exit.
        if ( lastCommand.type() != QSimCommand::NoCommand )
            atchat()->chat( "AT+STGR=99" );

        // Fetch the main menu anew.
        getTimer->stop();
        pendingCommand = 0;     // Wavecom SetupMenu
        getCommandTimeout();
    }
    seenBegin = true;
}

void WavecomSimToolkit::end()
{
    // Stop the timeout logic from re-getting the main menu over and over.
    seenBegin = false;
}

static QString toUCS2Hex( const QString& str )
{
    static char const hexchars[] = "0123456789ABCDEF";
    QString result = "";
    int posn = 0;
    uint ch;
    while ( posn < str.length() ) {
        ch = str[posn++].unicode();
        result += (QChar)(hexchars[(ch >> 12) & 0x0F]);
        result += (QChar)(hexchars[(ch >> 8) & 0x0F]);
        result += (QChar)(hexchars[(ch >> 4) & 0x0F]);
        result += (QChar)(hexchars[ch & 0x0F]);
    }
    return result;
}

void WavecomSimToolkit::sendResponse( const QSimTerminalResponse& resp )
{
    QSimCommand::Type cmd = resp.command().type();

    switch ( resp.result() ) {

        case QSimTerminalResponse::Success:
        {
            switch ( cmd ) {

                case QSimCommand::DisplayText:
                {
                    atchat()->chat( "AT+STGR=1" );
                }
                break;

                case QSimCommand::GetInkey:
                {
                    if ( !resp.command().ucs2Input() ) {
                        atchat()->chat
                            ( "AT+STGR=2,1,\"" + QAtUtils::quote( resp.text() ) + "\"" );
                    } else {
                        atchat()->chat
                            ( "AT+STGR=2,1,\"" + toUCS2Hex( resp.text() ) + "\"" );
                    }
                }
                break;

                case QSimCommand::GetInput:
                {
                    if ( !resp.command().ucs2Input() ) {
                        atchat()->chatPDU
                            ( "AT+STGR=3,1", QAtUtils::fromHex( resp.text() ), 0, 0 );
                    } else {
                        atchat()->chatPDU
                            ( "AT+STGR=3,1",
                              QAtUtils::fromHex( toUCS2Hex( resp.text() ) ), 0, 0 );
                    }
                }
                break;

                case QSimCommand::SetupCall:
                {
                    atchat()->chat( "AT+STGR=4,1" );
                }
                break;

                case QSimCommand::SelectItem:
                {
                    atchat()->chat( "AT+STGR=6,1," + QString::number( resp.menuItem() ) );
                }
                break;

                default: break;     // Don't know what to do here.  Probably just an ack.
            }
        }
        break;

        case QSimTerminalResponse::HelpInformationRequested:
        {
            switch ( cmd ) {

                case QSimCommand::GetInkey:
                {
                    atchat()->chat( "AT+STGR=2,2" );
                }
                break;

                case QSimCommand::GetInput:
                {
                    atchat()->chat( "AT+STGR=3,2" );
                }
                break;

                case QSimCommand::SelectItem:
                {
                    atchat()->chat( "AT+STGR=6,2," + QString::number( resp.menuItem() ) );
                }
                break;

                default: break;     // Don't know what to do here.
            }
        }
        break;

        case QSimTerminalResponse::UserDidNotAccept:
        {
            // Call setup denied.
            atchat()->chat( "AT+STGR=4,0" );
        }
        break;

        case QSimTerminalResponse::BackwardMove:
        {
            if ( cmd == QSimCommand::SelectItem )
                atchat()->chat( "AT+STGR=6,3" );
            else
                atchat()->chat( "AT+STGR=95" );
        }
        break;

        case QSimTerminalResponse::BeyondMECapabilities:
        {
            atchat()->chat( "AT+STGR=96" );
        }
        break;

        case QSimTerminalResponse::MEUnableToProcess:
        {
            atchat()->chat( "AT+STGR=97" );
        }
        break;

        case QSimTerminalResponse::NoResponseFromUser:
        {
            atchat()->chat( "AT+STGR=98" );
        }
        break;

        case QSimTerminalResponse::SessionTerminated:
        {
            if ( lastCommand.type() == QSimCommand::GetInkey )
                atchat()->chat( "AT+STGR=2,0" );
            else if ( lastCommand.type() == QSimCommand::GetInput )
                atchat()->chat( "AT+STGR=3,0" );
            else if ( lastCommand.type() == QSimCommand::SelectItem )
                atchat()->chat( "AT+STGR=6,0" );
            else
                atchat()->chat( "AT+STGR=99" );
        }
        break;

        default:
        {
            // Don't know what to do, so just say "unable to process".
            atchat()->chat( "AT+STGR=97" );
        }
        break;
    }
}

void WavecomSimToolkit::sendEnvelope( const QSimEnvelope& env )
{
    if ( env.type() == QSimEnvelope::MenuSelection ) {
        if ( env.requestHelp() )
            atchat()->chat( "AT+STGR=0,2," + QString::number( env.menuItem() ) );
        else
            atchat()->chat( "AT+STGR=0,1," + QString::number( env.menuItem() ) );
    } else if ( env.type() == QSimEnvelope::EventDownload ) {
        if ( env.event() == QSimEnvelope::IdleScreenAvailable )
            atchat()->chat( "AT+STGR=11,1" );
        else if ( env.event() == QSimEnvelope::UserActivity )
            atchat()->chat( "AT+STGR=11,2" );
    }
}

void WavecomSimToolkit::stinNotification( const QString& msg )
{
    // The SIM is notifying us that there is a command available.
    int cmd = msg.mid(7).toInt();
    if ( cmd != 98 /*Wavecom Timeout*/ && cmd != 99 /*Wavecom EndSession*/ ) {

        // Notification of a new command that we need to fetch.
        pendingCommand = cmd;
        getTimer->start( getTimeout );

    } else if ( lastCommand.type() == QSimCommand::SetupMenu &&
                !seenBegin ) {

        // AT+STIN=99 when main menu displayed, but before the first
        // time we've run "simapp".  Don't reget the main menu yet.
        lastCommand.setType( QSimCommand::NoCommand );

    } else if ( lastCommand.type() != QSimCommand::NoCommand ) {

        // Something else was aborted, so re-fetch the main menu.
        pendingCommand = 0;     // Wavecom SetupMenu
        getTimer->start( getTimeout );

    }
}

void WavecomSimToolkit::configureDone( bool ok )
{
    if ( ok ) {
        atchat()->chat( "AT+STSF=1", this, SLOT(activateDone(bool)) );
    } else {
        supportsStk = false;
        initializationDone();
    }
}

void WavecomSimToolkit::activateDone( bool ok )
{
    if ( ok ) {
        atchat()->chat( "AT+CFUN?", this, SLOT(cfunQueryDone(bool,QAtResult)) );
        supportsStk = true;
    } else {
        supportsStk = false;
        initializationDone();
    }
}

void WavecomSimToolkit::cfunQueryDone( bool, const QAtResult& result )
{
    QAtResultParser cmd( result );
    cmd.next( "+CFUN:" );
    atchat()->chat( "AT+CFUN=" + cmd.line() );
    initializationDone();
}

void WavecomSimToolkit::getCommandDone( bool ok, const QAtResult& result )
{
    QAtResultParser cmd( result );

    // No longer waiting for a command to be fetched.
    awaitingCommand = false;

    // Bail out if the command failed for some reason.
    if ( !ok ) {
        if ( pendingCommand == 0 /*Wavecom SetupMenu*/ ) {
            // Probably an indication that the SIM does not have
            // an application embedded in it.
            emit beginFailed();
        }
        return;
    }

    // Now that we have a positive response, reset the timeout to zero.
    getTimeout = 0;

    // Retrieve the command type and map it from Wavecom numbers to standard GSM numbers.
    QSimCommand::Type type;
    switch ( pendingCommand ) {

        case 0:             type = QSimCommand::SetupMenu; break;
        case 1:             type = QSimCommand::DisplayText; break;
        case 2:             type = QSimCommand::GetInkey; break;
        case 3:             type = QSimCommand::GetInput; break;
        case 4:             type = QSimCommand::SetupCall; break;
        case 5:             type = QSimCommand::PlayTone; break;
        case 6:             type = QSimCommand::SelectItem; break;
        case 7:             type = QSimCommand::Refresh; break;
        case 8:             type = QSimCommand::SendSS; break;
        case 9:             type = QSimCommand::SendSMS; break;
        case 10:            type = QSimCommand::SendUSSD; break;
        case 11:            type = QSimCommand::SetupEventList; break;
        case 98:            type = QSimCommand::Timeout; break;
        case 99:            type = QSimCommand::EndSession; break;
        default:            type = QSimCommand::EndSession; break;
    }

    // Parse the command details, based on the type.
    QSimCommand scmd;
    uint timeval, timeformat;
    scmd.setType( type );
    cmd.next( "+STGI:" );
    switch ( type ) {

        case QSimCommand::SetupMenu:
            scmd.setTitle( cmd.readString() );
            readMenu( cmd, scmd );
            break;

        case QSimCommand::DisplayText:
            scmd.setDestinationDevice( QSimCommand::Display );
            scmd.setHighPriority( cmd.readNumeric() != 0 );
            scmd.setText( cmd.readString() );
            scmd.setClearAfterDelay( cmd.readNumeric() == 0 );
            break;

        case QSimCommand::GetInkey:
            inputFormat = cmd.readNumeric();
            scmd.setWantDigits( inputFormat == 0 );
            scmd.setHasHelp( cmd.readNumeric() != 0 );
            scmd.setText( cmd.readString() );
            break;

        case QSimCommand::GetInput:
            inputFormat = cmd.readNumeric();
            scmd.setWantDigits( inputFormat == 0 );
            scmd.setEcho( cmd.readNumeric() != 0 );
            scmd.setMinimumLength( cmd.readNumeric() );
            scmd.setMaximumLength( cmd.readNumeric() );
            scmd.setHasHelp( cmd.readNumeric() != 0 );
            scmd.setText( cmd.readString() );
            break;

        case QSimCommand::SetupCall:
            scmd.setDestinationDevice( QSimCommand::Network );
            scmd.setDisposition
                ( (QSimCommand::Disposition)( cmd.readNumeric() ) );
            scmd.setNumber( cmd.readString() );
            scmd.setSubAddress( cmd.readString() );
            scmd.setCallClass
                ( (QSimCommand::CallClass)( cmd.readNumeric() ) );
            break;

        case QSimCommand::PlayTone:
            scmd.setDestinationDevice( QSimCommand::Earpiece );
            switch ( cmd.readNumeric() ) {

                // Wavecom uses non-standard tone numbers: convert them.
                case 0:     scmd.setTone( QSimCommand::ToneDial ); break;
                case 1:     scmd.setTone( QSimCommand::ToneBusy ); break;
                case 2:     scmd.setTone( QSimCommand::ToneCongestion ); break;
                case 3:     scmd.setTone( QSimCommand::ToneRadioAck ); break;
                case 4:     scmd.setTone( QSimCommand::ToneDropped ); break;
                case 5:     scmd.setTone( QSimCommand::ToneError ); break;
                case 6:     scmd.setTone( QSimCommand::ToneCallWaiting ); break;
                case 7:     scmd.setTone( QSimCommand::ToneRinging ); break;
                case 8:     scmd.setTone( QSimCommand::ToneGeneralBeep ); break;
                case 9:     scmd.setTone( QSimCommand::TonePositiveBeep ); break;
                case 10:    scmd.setTone( QSimCommand::ToneNegativeBeep ); break;
                default:    scmd.setTone( QSimCommand::ToneGeneralBeep ); break;

            }
            timeformat = cmd.readNumeric();
            timeval = cmd.readNumeric();
            if ( timeformat == 0 )
                timeval *= 60 * 1000;           // minutes
            else if ( timeformat == 1 )
                timeval *= 1000;                // seconds
            else if ( timeformat == 2 )
                timeval *= 100;                 // tenths of a second
            scmd.setToneTime( timeval );
            scmd.setText( cmd.readString() );
            break;

        case QSimCommand::SelectItem:
            scmd.setDefaultItem( cmd.readNumeric() );
            scmd.setTitle( cmd.readString() );
            readMenu( cmd, scmd );
            break;

        case QSimCommand::Refresh:
            scmd.setRefreshType
                ( (QSimCommand::RefreshType)( cmd.readNumeric() ) );
            break;

        case QSimCommand::SendSS:
            scmd.setDestinationDevice( QSimCommand::Network );
            scmd.setText( cmd.readString() );
            break;

        case QSimCommand::SendSMS:
            scmd.setDestinationDevice( QSimCommand::Network );
            scmd.setText( cmd.readString() );
            break;

        case QSimCommand::SendUSSD:
            scmd.setDestinationDevice( QSimCommand::Network );
            scmd.setText( cmd.readString() );
            break;

        case QSimCommand::SetupEventList:
            scmd.setEvents( (QSimCommand::Event)( cmd.readNumeric() ) );
            break;

        case QSimCommand::Timeout:
            break;

        case QSimCommand::EndSession:
            break;

        default:
            break;
    }

    // Send the command to the upper layers.
    lastCommand = scmd;
    emit command( scmd );
}

void WavecomSimToolkit::getCommandTimeout()
{
    atchat()->chat( "AT+STGI=" + QString::number( (uint)pendingCommand ),
                    this, SLOT(getCommandDone(bool,QAtResult)) );
    awaitingCommand = true;
}

void WavecomSimToolkit::readMenu( QAtResultParser& cmd, QSimCommand& scmd )
{
    QList<QSimMenuItem> items;
    QSimMenuItem item;
    while ( cmd.next( "+STGI:" ) ) {
        item.setIdentifier( cmd.readNumeric() );
        cmd.readNumeric();  // skip "NbItems" which isn't useful to us.
        item.setLabel( cmd.readString() );
        item.setHasHelp( cmd.readNumeric() != 0 );
        item.setNextAction( cmd.readNumeric() );    // NextActionId
        items.append( item );
    }
    scmd.setMenuItems( items );
}

WavecomPhoneBook::WavecomPhoneBook( WavecomModemService *service )
    : QModemPhoneBook( service )
{
    // Flush all phone book caches when the SIM is removed (+WIND: 0).
    connect( service, SIGNAL(simRemoved()), this, SLOT(flushCaches()) );

    // When +WIND: 10 arrives, tell QModemPhoneBook that the phone books
    // are ready to be used.
    connect( service, SIGNAL(phoneBooksReady()),
             this, SLOT(phoneBooksReady()) );
}

WavecomPhoneBook::~WavecomPhoneBook()
{
}

bool WavecomPhoneBook::hasModemPhoneBookCache() const
{
    return true;
}

WavecomSimInfo::WavecomSimInfo( WavecomModemService *service )
    : QModemSimInfo( service )
{
    connect( service, SIGNAL(simInserted()), this, SLOT(simInserted()) );
    connect( service, SIGNAL(simRemoved()), this, SLOT(simRemoved()) );
}

WavecomSimInfo::~WavecomSimInfo()
{
}

WavecomRfFunctionality::WavecomRfFunctionality( WavecomModemService *service )
    : QModemRfFunctionality( service )
{
    this->service = service;
    this->planeMode = false;
}

WavecomRfFunctionality::~WavecomRfFunctionality()
{
}

class WavecomCFunUserData : public QAtResult::UserData
{
public:
    WavecomCFunUserData( QPhoneRfFunctionality::Level level )
    { this->level = level; }

    QPhoneRfFunctionality::Level level;
};

void WavecomRfFunctionality::forceLevelRequest()
{
    service->primaryAtChat()->chat
        ( "AT+CFUN?", this, SLOT(cfun(bool,QAtResult)) );
}

void WavecomRfFunctionality::setLevel( QPhoneRfFunctionality::Level level )
{
    // Change DisableTransmitAndReceive into Minimum, because wavecom
    // doesn't support DisableTransmitAndReceive.
    QPhoneRfFunctionality::Level newLevel;
    if ( level == DisableTransmitAndReceive )
        newLevel = Minimum;
    else
        newLevel = level;
    service->primaryAtChat()->chat
        ( "AT+CFUN=" + QString::number( (int)newLevel ),
          this, SLOT(cfunSet(bool,QAtResult)),
          new WavecomCFunUserData( level ) );
}

void WavecomRfFunctionality::cfun( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CFUN:" ) ) {
        Level level = (Level)parser.readNumeric();
        if ( planeMode && level == Minimum ) {
            // If we are actually in plane mode, then we aren't "really"
            // in Minimum, but DisableTransmitAndReceive.
            level = DisableTransmitAndReceive;
        }
        setValue( "level", qVariantFromValue( level ) );
        emit levelChanged();
    }
}

void WavecomRfFunctionality::cfunSet( bool ok, const QAtResult& result )
{
    Level level = ((WavecomCFunUserData *)result.userData())->level;
    planeMode = ( level == DisableTransmitAndReceive );

    // Report the results to the client applications.
    if ( ok ) {
        setValue( "level", qVariantFromValue( level ) );
        emit levelChanged();
    }
    emit setLevelResult( (QTelephony::Result)result.resultCode() );

    // Force a PIN query at this point, because we may need to supply it again.
    if ( level != Minimum )
        service->post( "pinquery" );
}

WavecomConfiguration::WavecomConfiguration( WavecomModemService *service )
    : QModemConfiguration( service )
{
    this->service = service;
}

WavecomConfiguration::~WavecomConfiguration()
{
}

void WavecomConfiguration::request( const QString& name )
{
    if ( name == "extraVersion" ) {
        info = QString();
        service->primaryAtChat()->chat
            ( "AT+WHWV", this, SLOT(whwv(bool,QAtResult)) );
        service->primaryAtChat()->chat
            ( "AT+WSSV", this, SLOT(wssv(bool,QAtResult)) );
        service->primaryAtChat()->chat
            ( "AT+WDOP", this, SLOT(wdop(bool,QAtResult)) );
    } else {
        QModemConfiguration::request( name );
    }
}

void WavecomConfiguration::whwv( bool, const QAtResult& result )
{
    info = result.content().trimmed();
}

void WavecomConfiguration::wssv( bool, const QAtResult& result )
{
    QString trimmed = result.content().trimmed();
    if ( !trimmed.isEmpty() )
        info += "\n" + trimmed;
}

void WavecomConfiguration::wdop( bool, const QAtResult& result )
{
    QString trimmed = result.content().trimmed();
    if ( !trimmed.isEmpty() )
        info += "\n" + trimmed;
    emit notification( "extraVersion", info );
}

WavecomModemService::WavecomModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent )
    : QModemService( service, mux, parent )
{
    // Listen for +WIND notifications, which are dispatched to signals
    // by windNotification() below.
    primaryAtChat()->chat( "AT+WIND=1023" );
    primaryAtChat()->registerNotificationType
        ( "+WIND:", this, SLOT(windNotification(QString)) );
}

WavecomModemService::~WavecomModemService() {}

void WavecomModemService::initialize()
{
    // Create our Wavecom-specific overrides for the service interfaces.
    if ( !supports<QSimInfo>() )
        addInterface( new WavecomSimInfo( this ) );

    if ( !supports<QSimToolkit>() )
        addInterface( new WavecomSimToolkit( this ) );

    if ( !supports<QPhoneBook>() )
        addInterface( new WavecomPhoneBook( this ) );

    if ( !supports<QPhoneRfFunctionality>() )
        addInterface( new WavecomRfFunctionality( this ) );

    if ( !supports<QTelephonyConfiguration>() )
        addInterface( new WavecomConfiguration( this ) );

    if ( !callProvider() )
        setCallProvider( new WavecomCallProvider( this ) );

    // Call QModemService to create other interfaces that we didn't override.
    QModemService::initialize();
}

void WavecomModemService::windNotification( const QString& msg )
{
    // Parse the +WIND notification parameters and dispatch them
    // to the relevant signals.  The other Wavecom classes
    // hook onto these signals to be informed of these events.
    uint posn = 6;
    uint event = QAtUtils::parseNumber( msg, posn );
    uint index = QAtUtils::parseNumber( msg, posn );
    switch ( event ) {

        case 0:         emit simRemoved(); break;
        case 1:         emit simInserted(); break;
        case 2:         emit callingPartyAlerting(); break;
        case 3:         emit basicCommandsReady(); break;
        case 4:         emit allCommandsReady(); break;
        case 5:         emit callCreated( index ); break;
        case 6:         emit callReleased( index ); break;
        case 7:         emit emergencyCallsPossible(); break;
        case 8:         emit networkLost(); break;
        case 9:         emit audioOn(); break;
        case 10:        emit phoneBooksReady(); break;

    }
}
