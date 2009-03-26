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

#include "vendor_greenphone_p.h"
#include <qatresultparser.h>
#include <qatutils.h>
#include <qmodemindicators.h>
#include <qnetworkregistration.h>
#include <qtopianamespace.h>
#include <QTimer>
#include <QtopiaChannel>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <qtopialog.h>

GreenphoneCall::GreenphoneCall
        ( QModemCallProvider *provider, const QString& identifier,
          const QString& callType )
    : QModemCall( provider, identifier, callType )
{
}

GreenphoneCall::~GreenphoneCall()
{
}

void GreenphoneCall::tone( const QString& tones )
{
    // The greenphone needs AT*MKEYTONE to be sent in addition
    // to AT+VTS, or the tone won't sound in the local speaker.
    if ( state() == QPhoneCall::Connected && tones.length() > 0 ) {
        for ( int posn = 0; posn < tones.length(); ++posn ) {
            provider()->service()->primaryAtChat()->chat
                ( "AT*MKEYTONE=\"" + QString( tones[posn] ) + "\",25" );
            provider()->service()->primaryAtChat()->chat
                ( "AT+VTS=\"" + QString( tones[posn] ) + "\"" );
        }
    }
}

GreenphoneCallProvider::GreenphoneCallProvider
        ( GreenphoneModemService *service )
    : QModemCallProvider( service )
{
    this->service = service;
    service->primaryAtChat()->registerNotificationType
        ( "*MCAM:", this, SLOT(mcamNotification(QString)), true );
    callingId = 0;
    alertingId = 0;
    abortingId = 0;
    sawMrdy3 = true;        // Reset only if we see *MRDY: 4/5.
    needClipSet = false;
    prevActiveCall = false;
    connect( service, SIGNAL(allCommandsReady()),
             this, SLOT(allCommandsReady()) );
    connect( service, SIGNAL(simInserted()),
             this, SLOT(simChanged()) );
    connect( service, SIGNAL(simRemoved()),
             this, SLOT(simChanged()) );
    connect( this, SIGNAL(callStatesChanged()),
             this, SLOT(callsChanged()) );
}

GreenphoneCallProvider::~GreenphoneCallProvider()
{
}

QPhoneCallImpl *GreenphoneCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    // We need to override QModemCall::tone() to do proper key tones.
    if ( callType == "Voice" )      // No tr
        return new GreenphoneCall( this, identifier, callType );
    else
        return QModemCallProvider::create( identifier, callType );
}

QModemCallProvider::AtdBehavior GreenphoneCallProvider::atdBehavior() const
{
    // When ATD responds with OK, the call has connected.
    return AtdOkIsConnect;
}

void GreenphoneCallProvider::abortDial
    ( uint modemIdentifier, QPhoneCall::Scope scope )
{
    // The greenphone cannot abort an ATD request until the call
    // transitions from "calling" to "alerting/pre-connected".
    // It will ignore all attempts to abort the call before that.
    // The following code exists to work around this problem.
    atchat()->abortDial();
    if ( modemIdentifier == callingId ) {
        // If we have already transitioned to the alerting state,
        // then we can send ATH immediately to abort the call.
        // Otherwise we have to wait until we see the alerting state.
        if ( callingId == alertingId ) {
            doAbortDial();
        } else {
            abortingId = callingId;
        }
    } else {
        // Some other call.  Shouldn't happen, but do something useful.
        QModemCallProvider::abortDial( modemIdentifier, scope );
    }
}

QString GreenphoneCallProvider::setBusyCommand() const
{
    // Greenphone needs to use ATH to reject incoming calls.
    return "ATH";
}

void GreenphoneCallProvider::resetModem()
{
    // Don't call the parent class because we don't want AT+CLIP
    // turned on until we are sure that the modem is ready for it.
    // Otherwise it will sit and block for a long period of time.

    service->primaryAtChat()->chat( "AT+CRC=1" );
    needClipSet = true;
    if ( sawMrdy3 )
        allCommandsReady();
}

void GreenphoneCallProvider::allCommandsReady()
{
    sawMrdy3 = true;
    if ( needClipSet ) {
        needClipSet = false;

        // Turn on caller-ID presentation for incoming calls.
        service->retryChat( "AT+CLIP=1" );

        // Turn on unsolicited notifications for incoming calls.
        service->retryChat( "AT+CCWA=1" );
    }
}

void GreenphoneCallProvider::simChanged()
{
    // We need to wait for *MRDY: 3 before sending AT+CLIP.
    sawMrdy3 = false;
}

void GreenphoneCallProvider::mcamNotification( const QString& msg )
{
    // Greenphone-style call monitoring notification.  The first
    // two parameters are the call identifier and the status.
    // 0 = IDLE, 1 = CALLING, 2 = PRE-CONNECTED, 3 = CONNECT,
    // 4 = HOLD, 5 = WAITING, 6 = ALERTING, 7 = BUSY, 8 = DISCONNECT
    // The third parameter (not used here) is the call type:
    // 1 = VOICE, 2 = DATA, 3 = FAX, 10 = GPRS
    uint posn = 6;
    uint identifier = QAtUtils::parseNumber( msg, posn );
    uint status = QAtUtils::parseNumber( msg, posn );
    QModemCall *call = callForIdentifier( identifier );

    if ( status == 3 && call &&
         ( call->state() == QPhoneCall::Dialing ||
           call->state() == QPhoneCall::Alerting ) ) {

        // This is an indication that a "Dialing" connection
        // is now in the "Connected" state.  If there was a pending
        // abort, then we had a race condition between the local
        // user hanging up the call and the remote user accepting.
        // In that case, obey the local user and hang up the call.
        if ( abortingId == identifier ) {
            doAbortDial();
            call->setState( QPhoneCall::HangupLocal );
        } else {
            call->setConnected();
        }

    } else if ( call && call->callType() != "IP" && status == 8 &&
                ( call->state() == QPhoneCall::Dialing ||
                  call->state() == QPhoneCall::Alerting ) ) {

        // We never managed to connect.
        
        // We cannot hangup during an IP data call. IP data calls
        // have a secondary report type based in the state of the pppd process (see QPhoneCall::DataState).
        // If we'd hangup at this stage the modem data call would be deleted and subsequent 
        // pppd data state updates would never be received. The hangup will be done by QModemDataCall.
        hangupRemote( call );

    } else if ( call && call->callType() != "IP" && status == 8 &&
                ( call->state() == QPhoneCall::Connected ||
                  call->state() == QPhoneCall::Hold ) ) {

        // This is an indication that the connection has been lost.
        
        // We cannot hangup during an IP data call. IP data calls
        // have a secondary report type based in the state of the pppd process (see QPhoneCall::DataState).
        // If we'd hangup at this stage the modem data call would be deleted and subsequent 
        // pppd data state updates would never be received. The hangup will be done by QModemDataCall.
        hangupRemote( call );

    } else if ( status == 8 && call &&
                call->state() == QPhoneCall::Incoming ) {

        // This is an indication that an incoming call was missed.
        call->setState( QPhoneCall::Missed );

        // Let the rest of QModemCallProvider know that a remote hangup occurred.
        hangupRemote(call);

    } else if ( ( status == 6 || status == 2 ) &&
                callingId == identifier ) {

        // An outgoing call has transitioned to alerting/pre-connected.
        // Check to see if we have a pending abort waiting.
        bool aborting = false;
        if ( abortingId != 0 && callingId == abortingId ) {
            // There is a pending abort, so send the ATH now.
            doAbortDial();
            aborting = true;
        } else if ( abortingId == 0 ) {
            // Transitioned to alerting before the abort was received.
            // Record that we saw the alert so that abortDial() can
            // send the ATH itself when requested to do so.
            alertingId = callingId;
        }

        // Report the Dialing -> Alerting transition.
        if ( !aborting && call && call->state() == QPhoneCall::Dialing ) {
            call->setState( QPhoneCall::Alerting );
        }

    } else if ( status == 1 && call &&
                ( call->state() == QPhoneCall::Dialing ||
                  call->state() == QPhoneCall::Alerting ) ) {

        // This is the currently outgoing dial request.
        callingId = identifier;

    }

    // Clear the abortDial() identifiers if we are no longer dialing.
    if ( callingId == identifier ) {
        if ( status != 1 && status != 2 && status != 6 ) {
            callingId = 0;
            alertingId = 0;
            abortingId = 0;
        }
    }
}

void GreenphoneCallProvider::callsChanged()
{
    QList<QPhoneCallImpl *> list = calls();
    bool activeCall = false;
    foreach ( QPhoneCallImpl *call, list ) {
        QPhoneCall::State state = call->state();
        if ( state == QPhoneCall::Connected ||
             state == QPhoneCall::Dialing ||
             state == QPhoneCall::Alerting ) {
            activeCall = true;
            break;
        }
    }
    if ( activeCall != prevActiveCall ) {
        prevActiveCall = activeCall;
    }
}

void GreenphoneCallProvider::doAbortDial()
{
    callingId = 0;
    alertingId = 0;
    abortingId = 0;
    atchat()->send( "ATH" );
}

GreenphoneSimToolkit::GreenphoneSimToolkit( GreenphoneModemService *service )
    : QModemSimToolkit( service )
{
    registeredMenuItems = 0;
    expectedMenuItems = 0;
    lastCommand.setType( QSimCommand::NoCommand );
    responseType = -1;
    unicodeStrings = false;
    hasHelp = false;
    fetchingMainMenu = false;

    service->primaryAtChat()->registerNotificationType
        ( "*MTSMENU:", this, SLOT(mtsmenu(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTITEM:", this, SLOT(mtsmenu(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MSTKEV:", this, SLOT(mstkev()) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTDISP:", this, SLOT(mtdisp(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTKEY:", this, SLOT(mtkey(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTGIN:", this, SLOT(mtgin(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTTONE:", this, SLOT(mttone(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTRSH:", this, SLOT(mtrsh(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTSMS:", this, SLOT(mtsms(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTSS:", this, SLOT(mtss(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTUSSD:", this, SLOT(mtussd(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTDTMF:", this, SLOT(mtdtmf(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTCALL:", this, SLOT(mtcall(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTITXT:", this, SLOT(mtitxt(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTLANG:", this, SLOT(mtlang()) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTLANGNT:", this, SLOT(mtlangnt(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTLBR:", this, SLOT(mtlbr(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTRUNAT:", this, SLOT(mtrunat(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTCHEVT:", this, SLOT(mtchevt(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "*MTSTKCC:", this, SLOT(mtstkcc(QString)) );

    connect( service, SIGNAL(simInserted()), this, SLOT(simInserted()) );
    connect( service, SIGNAL(simRemoved()), this, SLOT(simRemoved()) );
}

GreenphoneSimToolkit::~GreenphoneSimToolkit()
{
}

void GreenphoneSimToolkit::groupInitialized( QAbstractIpcInterfaceGroup *group )
{
    // Hook onto the QSimInfo::inserted() signal because sometimes
    // "*MRDY: 4" will not be received if the modem has already gone
    // through main initialisation before Qtopia starts up.  The
    // QModemSimInfo class uses AT+CIMI to detect SIM already inserted.
    QSimInfo *simInfo = group->interface<QSimInfo>();
    if ( simInfo )
        connect( simInfo, SIGNAL(inserted()), this, SLOT(simInserted()) );
}

void GreenphoneSimToolkit::initialize()
{
    // Nothing special needs to be done to initialize STK on this modem.
    // It will just happen once regular SIM initialization is performed.
    QModemSimToolkit::initialize();
}

void GreenphoneSimToolkit::begin()
{
    if ( lastCommand.type() == QSimCommand::SetupMenu ) {

        // We just fetched the main menu, so return what we fetched.
        emit command( lastCommand );

    } else {

        // Clear the last-cached command.
        clearLastCommand();

        // Force the current session to exit, if necessary.
        if ( responseType != -1 ) {
            atchat()->chat
                ( "AT*MTRES=" + QString::number( responseType ) + ",10" );
            responseType = -1;
        } else if ( mainMenu.type() != QSimCommand::SetupMenu ) {
            // We don't know where we are.  We may be stuck in a sub-menu
            // from a previous instance of Qtopia.
            atchat()->chat( "AT*MTRES=4,10" );
        } else {
            // We know that we are back at the cached main menu.
            emitCommand( 8, mainMenu );
            return;
        }

        // We just issued a command to abort a session.  We have to wait
        // a little bit for the "*MSTKEV: 1" notification and then we
        // can fetch the menu.  If "*MSTKEV: 1" does not arrive, then
        // force the menu to be fetched with "AT*MTMENU?".
        QTimer::singleShot( 1000, this, SLOT(fetchMenuIfNecessary()) );

    }
}

void GreenphoneSimToolkit::end()
{
    // Force begin() to refetch the main menu next time.
    clearLastCommand();
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

void GreenphoneSimToolkit::sendResponse( const QSimTerminalResponse& resp )
{
    QString cmd = "AT*MTRES=";

    // Add the command type to the string.
    switch ( resp.command().type() ) {

        case QSimCommand::DisplayText:              cmd += "0"; break;
        case QSimCommand::GetInkey:                 cmd += "1"; break;
        case QSimCommand::GetInput:                 cmd += "2"; break;
        case QSimCommand::PlayTone:                 cmd += "3"; break;
        case QSimCommand::SelectItem:               cmd += "4"; break;
        case QSimCommand::SendSS:                   cmd += "5"; break;
        case QSimCommand::SendUSSD:                 cmd += "6"; break;
        case QSimCommand::SetupCall:                cmd += "7"; break;
        case QSimCommand::SetupMenu:                cmd += "8"; break;
        case QSimCommand::Refresh:                  cmd += "10"; break;
        case QSimCommand::SendDTMF:                 cmd += "12"; break;
        case QSimCommand::LaunchBrowser:            cmd += "13"; break;
        case QSimCommand::SetupIdleModeText:        cmd += "14"; break;
        case QSimCommand::ProvideLocalInformation:  cmd += "15"; break;
        case QSimCommand::OpenChannel:
        case QSimCommand::CloseChannel:
        case QSimCommand::ReceiveData:
        case QSimCommand::SendData:                 cmd += "16"; break;

        default:
        {
            // Don't know how to respond to this on greenphone, so use the default behaviour.
            QModemSimToolkit::sendResponse( resp );
        }
        break;
    }

    // Add the result code, in hex, to the string.
    cmd += "," + QString::number( (int)resp.result(), 16 );

    // Add the cause code, in hex, to the string.
    bool needExtraComma;
    if ( resp.cause() != QSimTerminalResponse::NoSpecificCause ) {
        cmd += "," + QString::number( (int)resp.cause(), 16 );
        needExtraComma = false;
    } else {
        needExtraComma = true;
    }

    // Add additional parameters as necessary.
    QString extra;
    switch ( resp.command().type() ) {

        case QSimCommand::SelectItem:
        {
            if ( resp.result() != QSimTerminalResponse::Success &&
                 resp.result() != QSimTerminalResponse::HelpInformationRequested )
                break;
            extra = ",," + QString::number( resp.menuItem() );
        }
        break;

        case QSimCommand::GetInkey:
        case QSimCommand::GetInput:
        {
            if ( resp.result() != QSimTerminalResponse::Success )
                break;
            if ( resp.command().wantYesNo() ) {
                if ( resp.text() == "Yes" )     // No tr
                    extra += ",\"YES\"";
                else
                    extra += ",\"NO\"";
            } else if ( resp.command().ucs2Input() ) {
                extra += ",\"" + toUCS2Hex( resp.text() ) + "\"";
            } else {
                extra += ",\"" + QAtUtils::quote( resp.text() ) + "\"";
            }
        }
        break;

        case QSimCommand::OpenChannel:
        {
            // The cause code is replaced with chanId for the OpenChannel command.
            int chanId = (int)(resp.command().destinationDevice());
            if ( chanId >= 0x21 && chanId <= 0x2F )
                chanId -= 0x20;
            else
                chanId = 0;
            extra = "," + QString::number( chanId );
            needExtraComma = false;
        }
        break;

        default: break;
    }
    if ( !extra.isEmpty() ) {
        if ( needExtraComma )
            cmd += ",";
        cmd += extra;
    }

    // Issue the command to the modem.
    atchat()->chat( cmd );
}

void GreenphoneSimToolkit::sendEnvelope( const QSimEnvelope& env )
{
    if ( env.type() == QSimEnvelope::MenuSelection ) {
        if ( env.requestHelp() )
            atchat()->chat( "AT*MTMENU=" + QString::number( env.menuItem() ) + ",1" );
        else
            atchat()->chat( "AT*MTMENU=" + QString::number( env.menuItem() ) );
    } else {
        QModemSimToolkit::sendEnvelope( env );
    }
}

void GreenphoneSimToolkit::simInserted()
{
    // Wait two seconds and then force the menu to be fetched.
    clearLastCommand();
    clearMainMenu();
    QTimer::singleShot( 2000, this, SLOT(begin()) );
}

void GreenphoneSimToolkit::simRemoved()
{
    // SIM was removed, so clear cached information.
    clearLastCommand();
    clearMainMenu();
    responseType = -1;
}

void GreenphoneSimToolkit::fetchMenu()
{
    // We may have gotten here after aborting an existing session in begin().
    // In that case, we have already requested the main menu.
    if ( !fetchingMainMenu ) {
        clearLastCommand();
        responseType = -1;
        fetchingMainMenu = true;
        atchat()->chat( "AT*MTMENU?", this, SLOT(mtmenu(bool)) );
    }
}

void GreenphoneSimToolkit::fetchMenuIfNecessary()
{
    // Fetch the main menu if we didn't already get it during startup.
    if ( lastCommand.type() != QSimCommand::SetupMenu )
        fetchMenu();
}

// Process *MTSMENU and *MTITEM notifications from the modem.
void GreenphoneSimToolkit::mtsmenu( const QString& msg )
{
    // Determine where to start parsing and the type of command.
    if ( msg.startsWith( "*MTITEM:" ) ) {
        firstLine = msg.mid( 8 );
        menuType = QSimCommand::SelectItem;
    } else {
        firstLine = msg.mid( 9 );
        menuType = QSimCommand::SetupMenu;
    }

    // Record the first line for later and parse information we need now.
    uint posn = 0;
    QAtUtils::nextString( firstLine, posn );        // Skip title.
    unicodeStrings = ( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    uint numItems = QAtUtils::parseNumber( firstLine, posn );
    hasHelp = ( QAtUtils::parseNumber( firstLine, posn ) != 0 );

    // Start receiving the menu items.
    items.clear();
    expectedMenuItems = numItems;
    if ( !expectedMenuItems )
        menuDone();
    else
        atchat()->requestNextLine( this, SLOT(menuItem(QString)) );
}

void GreenphoneSimToolkit::mstkev()
{
    // Session has ended, so we are back at the main menu.
    if ( mainMenu.type() == QSimCommand::SetupMenu ) {
        // Emit the cached main menu.
        emitCommand( 8, mainMenu );
    } else {
        // We don't have the main menu yet, so fetch it.
        fetchMenu();
    }
}

// Process a menu item for a *MTSMENU command.
void GreenphoneSimToolkit::menuItem( const QString& msg )
{
    // If we are not expecting menu items, then ignore the line.
    if ( !expectedMenuItems )
        return;

    // Read the menu item identifier from the front of the message.
    uint posn = 0;
    uint value = 0;
    while ( ((int)posn) < msg.length() && msg[posn] != ':' ) {
        value = value * 10 + (msg[posn].unicode() - '0');
        ++posn;
    }
    ++posn;         // Skip the colon.

    // Parse the menu item's label.
    QString label = QAtUtils::nextString( msg, posn );

    // Parse the icon information.
    uint iconId = 0;
    bool iconSelfExplanatory = false;
    if ( ((int)posn) < msg.length() ) {
        QAtUtils::parseNumber( msg, posn );     // Skip iconExist flag.
        iconSelfExplanatory = ( QAtUtils::parseNumber( msg, posn ) != 0 );
        iconId = QAtUtils::parseNumber( msg, posn );
    }

    // Add the item to the list.
    QSimMenuItem item;
    item.setIdentifier( value );
    item.setLabel( decodeString( label ) );
    item.setIconId( iconId );
    item.setIconSelfExplanatory( iconSelfExplanatory );
    item.setHasHelp( hasHelp );
    if ( ((int)posn) < msg.length() &&
         QAtUtils::parseNumber( msg, posn ) != 0 ) {
        item.setNextAction( QAtUtils::parseNumber( msg, posn ) );
    }
    items.append( item );

    // Stop if we have seen all of the menu items.
    --expectedMenuItems;
    if ( expectedMenuItems == 0 )
        menuDone();
    else
        atchat()->requestNextLine( this, SLOT(menuItem(QString)) );
}

void GreenphoneSimToolkit::menuDone()
{
    // Parse the first menu line and populate the command block.
    QSimCommand cmd;
    uint posn = 0;
    QString menuTitle = QAtUtils::nextString( firstLine, posn );
    unicodeStrings = ( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    QAtUtils::parseNumber( firstLine, posn );       // Skip number of items.
    cmd.setType( menuType );
    cmd.setMenuItems( items );
    cmd.setTitle( decodeString( menuTitle ) );
    cmd.setHasHelp( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    parseIconInfo( cmd, firstLine, posn );

    // Send the command to the application layer.
    if ( menuType == QSimCommand::SetupMenu ) {
        emitCommand( 8, cmd );

        // Cache the main menu so we can reuse it later.
        mainMenu = cmd;

        // Tell the SIM that we have accepted the menu.
        atchat()->chat( "AT*MTRES=8,0" );
    } else {
        emitCommand( 4, cmd );
    }

    // Clear the menu items to save memory.
    items.clear();
}

// Process the response to AT*MTMENU?  If the menu was fetched,
// it will have already been processed by mtsmenu() above.
void GreenphoneSimToolkit::mtmenu( bool ok )
{
    fetchingMainMenu = false;
    if ( !ok || lastCommand.type() != QSimCommand::SetupMenu ) {

        // We were unable to get the main menu, so there probably
        // is no STK functionality on this SIM or it isn't ready yet.
        emit beginFailed();

    }
}

QString GreenphoneSimToolkit::decodeString( const QString& value )
{
    QModemPhoneBook *phoneBook =
        qobject_cast<QModemPhoneBook *>( service()->interface<QPhoneBook>() );
    if ( !unicodeStrings && phoneBook )
        return QAtUtils::decode( value, phoneBook->stringCodec() );
    else if ( !unicodeStrings )
        return value;
    else
        return QAtUtils::decode( value, QAtUtils::codec( "ucs2" ) );
}

void GreenphoneSimToolkit::clearLastCommand()
{
    lastCommand = QSimCommand();
    lastCommand.setType( QSimCommand::NoCommand );
}

void GreenphoneSimToolkit::clearMainMenu()
{
    mainMenu = QSimCommand();
    mainMenu.setType( QSimCommand::NoCommand );
}

void GreenphoneSimToolkit::mtdisp( const QString& msg )
{
    firstLine = msg;
    atchat()->requestNextLine( this, SLOT(mtdispData(QString)) );
}

void GreenphoneSimToolkit::parseIconInfo
        ( QSimCommand& cmd, const QString& msg, uint& posn )
{
    if ( QAtUtils::parseNumber( msg, posn ) != 0 ) {
        // Icon information is available.
        cmd.setIconSelfExplanatory
            ( QAtUtils::parseNumber( msg, posn ) != 0 );
        cmd.setIconId( QAtUtils::parseNumber( msg, posn ) );
    }
}

void GreenphoneSimToolkit::mtdispData( const QString& msg )
{
    uint posn = 8;
    QSimCommand cmd;
    cmd.setType( QSimCommand::DisplayText );
    cmd.setDestinationDevice( QSimCommand::Display );
    cmd.setHighPriority( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    cmd.setClearAfterDelay( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    unicodeStrings = ( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    cmd.setText( decodeString( msg ) );
    parseIconInfo( cmd, firstLine, posn );
    emitCommand( 0, cmd );
}

void GreenphoneSimToolkit::mtkey( const QString& msg )
{
    firstLine = msg;
    atchat()->requestNextLine( this, SLOT(mtkeyData(QString)) );
}

void GreenphoneSimToolkit::mtkeyData( const QString& msg )
{
    mtkeygin( msg, QString() );
}

void GreenphoneSimToolkit::mtgin( const QString& msg )
{
    firstLine = msg;
    atchat()->requestNextLine( this, SLOT(mtginData(QString)) );
}

void GreenphoneSimToolkit::mtginData( const QString& msg )
{
    secondLine = msg;
    atchat()->requestNextLine( this, SLOT(mtginDefaultInput(QString)) );
}

void GreenphoneSimToolkit::mtginDefaultInput( const QString& msg )
{
    mtkeygin( secondLine, msg );
}

void GreenphoneSimToolkit::mtkeygin
        ( const QString& data, const QString& defaultInput )
{
    uint posn = 7;
    QSimCommand cmd;
    if ( firstLine.startsWith( "*MTKEY:" ) )
        cmd.setType( QSimCommand::GetInkey );
    else
        cmd.setType( QSimCommand::GetInput );
    uint keyType = QAtUtils::parseNumber( firstLine, posn );
    unicodeStrings = ( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    if ( keyType == 0 )
        cmd.setWantYesNo( true );
    else if ( keyType == 1 )
        cmd.setWantDigits( true );
    else if ( keyType == 3 )
        cmd.setUcs2Input( true );
    if ( cmd.type() == QSimCommand::GetInput ) {
        cmd.setEcho( QAtUtils::parseNumber( firstLine, posn ) != 0 );
        cmd.setMinimumLength( QAtUtils::parseNumber( firstLine, posn ) );
        cmd.setMaximumLength( QAtUtils::parseNumber( firstLine, posn ) );
    }
    cmd.setHasHelp( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    parseIconInfo( cmd, firstLine, posn );
    if ( cmd.type() == QSimCommand::GetInkey ) {
        cmd.setText( decodeString( data ) );
        emitCommand( 1, cmd );
    } else {
        cmd.setText( decodeString( data ) );
        cmd.setDefaultText( decodeString( defaultInput ) );
        emitCommand( 2, cmd );
    }
}

void GreenphoneSimToolkit::mttone( const QString& msg )
{
    QSimCommand cmd;
    uint posn = 8;
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setType( QSimCommand::PlayTone );
    cmd.setDestinationDevice( QSimCommand::Earpiece );
    cmd.setText( decodeString( text ) );
    uint tone = QAtUtils::parseNumber( msg, posn );
    switch ( tone ) {

        case 1:     cmd.setTone( QSimCommand::ToneDial ); break;
        case 2:     cmd.setTone( QSimCommand::ToneBusy ); break;
        case 3:     cmd.setTone( QSimCommand::ToneCongestion ); break;
        case 4:     cmd.setTone( QSimCommand::ToneRadioAck ); break;
        case 5:     cmd.setTone( QSimCommand::ToneDropped ); break;
        case 6:     cmd.setTone( QSimCommand::ToneError ); break;
        case 7:     cmd.setTone( QSimCommand::ToneCallWaiting ); break;
        case 8:     cmd.setTone( QSimCommand::ToneRinging ); break;

        // The modem spec says that the tone numbers 10, 11, and 12 are the ME beeps,
        // but GSM says that the values should be 16, 17, and 18.  Either the modem
        // is renumbering the values, or it is sending the values as hex and not decimal.
        // We recognize both sets of values just to be sure.
        case 10:    cmd.setTone( QSimCommand::ToneGeneralBeep ); break;
        case 11:    cmd.setTone( QSimCommand::TonePositiveBeep ); break;
        case 12:    cmd.setTone( QSimCommand::ToneNegativeBeep ); break;
        case 16:    cmd.setTone( QSimCommand::ToneGeneralBeep ); break;
        case 17:    cmd.setTone( QSimCommand::TonePositiveBeep ); break;
        case 18:    cmd.setTone( QSimCommand::ToneNegativeBeep ); break;

        // Some other proprietry tone.  Use general beep.
        default:    cmd.setTone( QSimCommand::ToneGeneralBeep ); break;
    }
    cmd.setToneTime( QAtUtils::parseNumber( msg, posn ) );
    parseIconInfo( cmd, msg, posn );
    emitCommand( 3, cmd );
}

void GreenphoneSimToolkit::mtrsh( const QString& msg )
{
    // Parse the command information and send it.
    QSimCommand cmd;
    uint posn = 7;
    cmd.setType( QSimCommand::Refresh );
    uint type = QAtUtils::parseNumber( msg, posn );
    cmd.setRefreshType( (QSimCommand::RefreshType)type );
    emitCommandNoResponse( cmd );

    // Acknowledge the command to the SIM.  Higher level action not needed.
    atchat()->chat( "AT*MTRES=10,0" );

    // We need to discard the EF list if one is present.
    if ( type <= 2 )
        atchat()->requestNextLine( this, SLOT(mtrshEFList(QString)) );
}

void GreenphoneSimToolkit::mtrshEFList( const QString& )
{
    // Nothing to do here - just discard it.
}

void GreenphoneSimToolkit::mtsms( const QString& msg )
{
    QSimCommand cmd;
    uint posn = 7;
    cmd.setType( QSimCommand::SendSMS );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    parseIconInfo( cmd, msg, posn );
    emitCommandNoResponse( cmd );
}

void GreenphoneSimToolkit::mtss( const QString& msg )
{
    QSimCommand cmd;
    uint posn = 6;
    cmd.setType( QSimCommand::SendSS );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    cmd.setNumber( QAtUtils::nextString( msg, posn ) ); // SSstring value.
    parseIconInfo( cmd, msg, posn );
    emitCommandNoResponse( cmd );
}

void GreenphoneSimToolkit::mtussd( const QString& msg )
{
    QSimCommand cmd;
    uint posn = 8;
    cmd.setType( QSimCommand::SendUSSD );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    cmd.setNumber( QAtUtils::nextString( msg, posn ) ); // USSDstring value.
    parseIconInfo( cmd, msg, posn );
    emitCommandNoResponse( cmd );
}

void GreenphoneSimToolkit::mtdtmf( const QString& msg )
{
    QSimCommand cmd;
    uint posn = 8;
    cmd.setType( QSimCommand::SendDTMF );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    cmd.setNumber( QAtUtils::nextString( msg, posn ) ); // DTMF value.
    parseIconInfo( cmd, msg, posn );
    emitCommandNoResponse( cmd );
}

void GreenphoneSimToolkit::mtcall( const QString& msg )
{
    QSimCommand cmd;
    uint posn = 8;
    cmd.setType( QSimCommand::SetupCall );
    cmd.setQualifier( QAtUtils::parseNumber( msg, posn ) );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    cmd.setNumber( QAtUtils::nextString( msg, posn ) );
    QAtUtils::parseNumber( msg, posn );     // Skip phase value.
    parseIconInfo( cmd, msg, posn );
    emitCommand( 7, cmd );
}

void GreenphoneSimToolkit::mtitxt( const QString& msg )
{
    firstLine = msg;
    atchat()->requestNextLine( this, SLOT(mtitxtData(QString)) );
}

void GreenphoneSimToolkit::mtitxtData( const QString& msg )
{
    // Parse the command details and send it.
    QSimCommand cmd;
    uint posn = 8;
    cmd.setType( QSimCommand::SetupIdleModeText );
    unicodeStrings = ( QAtUtils::parseNumber( firstLine, posn ) != 0 );
    cmd.setText( decodeString( msg ) );
    parseIconInfo( cmd, firstLine, posn );
    emitCommandNoResponse( cmd );

    // Acknowledge the command back to the SIM.  No higher level
    // intervention is needed at this point.
    atchat()->chat( "AT*MTRES=14,0" );
}

void GreenphoneSimToolkit::mtlang()
{
    QStringList languages = Qtopia::languageList();
    QString lang;
    if ( languages.size() >= 1 && languages[0].length() >= 2 ) {
        lang = languages[0].left(2);
    } else {
        // Something wrong with the language list - assume English.
        lang = "en";
    }
    atchat()->chat( "AT*MTRES=15,0,,\"" + QAtUtils::quote( lang ) + "\"" );
}

void GreenphoneSimToolkit::mtlangnt( const QString& msg )
{
    // SIM is sending us a language notification event.
    uint posn = 10;
    QSimCommand cmd;
    cmd.setType( QSimCommand::LanguageNotification );
    QByteArray lang = QAtUtils::nextString( msg, posn ).toLatin1();
    if ( !lang.isEmpty() )
        cmd.addExtensionField( 0xAD, lang );
    emitCommandNoResponse( cmd );
}

void GreenphoneSimToolkit::mtlbr( const QString& msg )
{
    firstLine = msg;
    atchat()->requestNextLine( this, SLOT(mtlbrText(QString)) );
}

void GreenphoneSimToolkit::mtlbrText( const QString& )
{
    // Discard the text, which is the proxy information, and get the alpha id.
    atchat()->requestNextLine( this, SLOT(mtlbrAlpha(QString)) );
}

void GreenphoneSimToolkit::mtlbrAlpha( const QString& msg )
{
    uint posn = 7;
    QSimCommand cmd;
    cmd.setType( QSimCommand::LaunchBrowser );
    cmd.setBrowserLaunchMode
        ( (QSimCommand::BrowserLaunchMode)QAtUtils::parseNumber( firstLine, posn ) );
    cmd.setUrl( QAtUtils::nextString( firstLine, posn ) );
    posn = 0;
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( QAtUtils::nextString( msg, posn ) ) );
    parseIconInfo( cmd, msg, posn );
    emitCommand( 13, cmd );
}

void GreenphoneSimToolkit::mtrunat( const QString& msg )
{
    // The spec is confusing.  It says <atCmd>,<codeType>,<iconInfo>, when it probably
    // means <alphaId>,<codeType>,<atCmd>,<iconInfo> for consistency with the other
    // commands.  We will assume the latter until we discover otherwise.
    uint posn = 9;
    QSimCommand cmd;
    cmd.setType( QSimCommand::RunATCommand );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    QByteArray atcmd = QAtUtils::nextString( msg, posn ).toLatin1();
    if ( !atcmd.isEmpty() )
        cmd.addExtensionField( 0xA8, atcmd );
    parseIconInfo( cmd, msg, posn );
    emitCommandNoResponse( cmd );
}

void GreenphoneSimToolkit::mtchevt( const QString& msg )
{
    uint posn = 9;
    QSimCommand cmd;
    uint chanId = QAtUtils::parseNumber( msg, posn );
    uint servType = QAtUtils::parseNumber( msg, posn );
    if ( servType == 1 )
        cmd.setType( QSimCommand::OpenChannel );
    else if ( servType == 2 )
        cmd.setType( QSimCommand::SendData );
    else if ( servType == 3 )
        cmd.setType( QSimCommand::ReceiveData );
    else if ( servType == 4 )
        cmd.setType( QSimCommand::CloseChannel );
    else
        return;
    cmd.setDestinationDevice( (QSimCommand::Device)( chanId + 0x20 ) );
    QString text = QAtUtils::nextString( msg, posn );
    unicodeStrings = ( QAtUtils::parseNumber( msg, posn ) != 0 );
    cmd.setText( decodeString( text ) );
    parseIconInfo( cmd, msg, posn );
    if ( cmd.type() == QSimCommand::OpenChannel ) {
        emitCommand( 16, cmd );
    } else {
        emitCommandNoResponse( cmd );
        atchat()->chat( "AT*MTRES=16,0," + QString::number( chanId ) );
    }
}

void GreenphoneSimToolkit::mtstkcc( const QString& msg )
{
    QSimControlEvent ev;
    uint posn = 8;
    uint resultMode, errCode;
    uint oldType, newType;
    QString alpha;
    resultMode = QAtUtils::parseNumber( msg, posn );
    if ( resultMode != 4 ) {
        errCode = 0;
        oldType = QAtUtils::parseNumber( msg, posn );
        newType = QAtUtils::parseNumber( msg, posn );
        if ( resultMode == 2 )
            ev.setResult( QSimControlEvent::AllowedWithModifications );
        else if ( resultMode == 2 )
            ev.setResult( QSimControlEvent::NotAllowed );
        else
            ev.setResult( QSimControlEvent::Allowed );
    } else {
        errCode = QAtUtils::parseNumber( msg, posn );
        oldType = QAtUtils::parseNumber( msg, posn );
        newType = oldType;
        if ( errCode == 5 )
            ev.setResult( QSimControlEvent::AllowedWithModifications );
        else
            ev.setResult( QSimControlEvent::NotAllowed );
    }
    alpha = QAtUtils::nextString( msg, posn );
    QModemPhoneBook *phoneBook =
        qobject_cast<QModemPhoneBook *>( service()->interface<QPhoneBook>() );
    if ( phoneBook )
        alpha = QAtUtils::decode( alpha, phoneBook->stringCodec() );
    ev.setText( alpha );
    if ( oldType == 4 )
        ev.setType( QSimControlEvent::Sms );
    else
        ev.setType( QSimControlEvent::Call );
    emit controlEvent( ev );
}

void GreenphoneSimToolkit::emitCommand
        ( int responseType, const QSimCommand& cmd )
{
    this->responseType = responseType;
    this->lastCommand = cmd;
    emit command( cmd );
}

// Emit a command to which we do not expect a response from the higher levels.
void GreenphoneSimToolkit::emitCommandNoResponse( const QSimCommand& cmd )
{
    this->responseType = -1;
    this->lastCommand = cmd;
    emit command( cmd );
}

GreenphonePhoneBook::GreenphonePhoneBook( GreenphoneModemService *service )
    : QModemPhoneBook( service )
{
    // Flush the phone book caches when the SIM is removed (*MRDY: 5).
    connect( service, SIGNAL(simRemoved()), this, SLOT(flushCaches()) );

    // Start up the phone books once we receive *MRDY: 3.
    connect( service, SIGNAL(allCommandsReady()),
             this, SLOT(phoneBooksReady()) );
}

GreenphonePhoneBook::~GreenphonePhoneBook()
{
}

bool GreenphonePhoneBook::hasModemPhoneBookCache() const
{
    // We have to wait for "*MRDY: 3" before accessing the SIM phone books.
    return true;
}

GreenphoneSimInfo::GreenphoneSimInfo( GreenphoneModemService *service )
    : QModemSimInfo( service )
{
    connect( service, SIGNAL(simInserted()), this, SLOT(simInserted()) );
    connect( service, SIGNAL(simRemoved()), this, SLOT(simRemoved()) );
}

GreenphoneSimInfo::~GreenphoneSimInfo()
{
}

GreenphoneSimFiles::GreenphoneSimFiles( GreenphoneModemService *service )
    : QModemSimFiles( service )
{
}

GreenphoneSimFiles::~GreenphoneSimFiles()
{
}

bool GreenphoneSimFiles::useCSIM() const
{
    // The greenphone needs AT+CSIM instead of AT+CRSM to access SIM files.
    return true;
}

typedef struct
{
    const char *name;
    int         value;

} BandInfo;
static BandInfo const bandInfo[] = {
    {"GSM & EGSM",          0},
    {"GSM 1800",            1},
    {"Dualband 900/1800",   2},
    {"PCS 1900",            3},
    {"GSM 850",             5},
    {"Dualband 1900/850",   6}
};
#define numBands    ((int)(sizeof(bandInfo) / sizeof(BandInfo)))

GreenphoneBandSelection::GreenphoneBandSelection
        ( GreenphoneModemService *service )
    : QBandSelection( service->service(), service, Server )
{
    this->service = service;
}

GreenphoneBandSelection::~GreenphoneBandSelection()
{
}

void GreenphoneBandSelection::requestBand()
{
    service->primaryAtChat()->chat
        ( "AT*MBSEL?", this, SLOT(mbsel(bool,QAtResult)) );
}

void GreenphoneBandSelection::requestBands()
{
    QStringList list;
    for ( int index = 0; index < numBands; ++index ) {
        list += QString( bandInfo[index].name );
    }
    emit bands( list );
}

void GreenphoneBandSelection::setBand
        ( QBandSelection::BandMode mode, const QString& value )
{
    int bandValue;
    if ( mode == Automatic ) {
        bandValue = 4;
    } else {
        bandValue = -1;
        for ( int index = 0; index < numBands; ++index ) {
            if ( value == bandInfo[index].name ) {
                bandValue = bandInfo[index].value;
                break;
            }
        }
        if ( bandValue == -1 ) {
            // The band value is not valid.
            emit setBandResult( QTelephony::OperationNotSupported );
            return;
        }
    }
    service->primaryAtChat()->chat
        ( "AT*MBSEL=" + QString::number( bandValue ),
          this, SLOT(mbselSet(bool,QAtResult)) );
}

void GreenphoneBandSelection::mbsel( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    int bandValue;
    if ( parser.next( "*MBSEL:" ) ) {
        bandValue = (int)parser.readNumeric();
    } else {
        // Command did not work, so assume "Auto".
        bandValue = 4;
    }
    for ( int index = 0; index < numBands; ++index ) {
        if ( bandValue == bandInfo[index].value ) {
            emit band( Manual, bandInfo[index].name );
            return;
        }
    }
    emit band( Automatic, QString() );
}

void GreenphoneBandSelection::mbselSet( bool, const QAtResult& result )
{
    emit setBandResult( (QTelephony::Result)result.resultCode() );
}

GreenphoneVibrateAccessory::GreenphoneVibrateAccessory
        ( QModemService *service )
    : QVibrateAccessoryProvider( service->service(), service )
{
    setSupportsVibrateNow( true );
}

GreenphoneVibrateAccessory::~GreenphoneVibrateAccessory()
{
}

#define OMEGA_VIBRATE_ON        0xc0043901
#define OMEGA_VIBRATE_OFF       0xc0043902

void GreenphoneVibrateAccessory::setVibrateNow( const bool value )
{
    int fd = ::open( "/dev/omega_vibrator", O_RDWR );
    if ( fd < 0 ) {
        perror( "/dev/omega_vibrator" );
        return;
    }
    int result;
    if ( value ) {
        result = ::ioctl( fd, OMEGA_VIBRATE_ON, 0 );
    } else {
        result = ::ioctl( fd, OMEGA_VIBRATE_OFF, 0 );
    }
    if ( result < 0 ) {
        perror( "/dev/omega_vibrator: ioctl" );
    }
    ::close( fd );
    QVibrateAccessoryProvider::setVibrateNow( value );
}

GreenphoneServiceNumbers::GreenphoneServiceNumbers( QModemService *service )
    : QModemServiceNumbers( service )
{
    this->service = service;
}

GreenphoneServiceNumbers::~GreenphoneServiceNumbers()
{
}

void GreenphoneServiceNumbers::requestServiceNumber
        ( QServiceNumbers::NumberId id )
{
    if ( id == QServiceNumbers::VoiceMail ) {
        // Note: neither AT*MSMCS nor AT+CSVM seem to work on
        // the greenphone's modem, so we use a conf file instead.
        QModemServiceNumbers::requestServiceNumberFromFile( id );
        //service->primaryAtChat()->chat
        //    ( "AT*MVMSC=1,0", this, SLOT(vmQueryDone(bool,QAtResult)) );
    } else {
        QModemServiceNumbers::requestServiceNumber( id );
    }
}

void GreenphoneServiceNumbers::setServiceNumber
        ( QServiceNumbers::NumberId id, const QString& number )
{
    if ( id == QServiceNumbers::VoiceMail ) {
        // Note: neither AT*MSMCS nor AT+CSVM seem to work on
        // the greenphone's modem, so we use a conf file instead.
        QModemServiceNumbers::setServiceNumberInFile( id, number );
        //if ( number.isEmpty() ) {
        //    service->primaryAtChat()->chat
        //        ( "AT*MVMSC=1,1,\"\"",
        //          this, SLOT(vmModifyDone(bool,QAtResult)) );
        //} else {
        //    service->primaryAtChat()->chat
        //        ( "AT*MVMSC=1,1," + QAtUtils::encodeNumber( number ),
        //          this, SLOT(vmModifyDone(bool,QAtResult)) );
        //}
    } else {
        QModemServiceNumbers::setServiceNumber( id, number );
    }
}

void GreenphoneServiceNumbers::vmQueryDone( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "*MVMSC:" ) && parser.readNumeric() != 0 ) {
        emit serviceNumber( QServiceNumbers::VoiceMail,
                            QAtUtils::decodeNumber( parser ) );
    } else {
        emit serviceNumber( QServiceNumbers::VoiceMail, QString() );
    }
}

void GreenphoneServiceNumbers::vmModifyDone( bool, const QAtResult& result )
{
    emit setServiceNumberResult( QServiceNumbers::VoiceMail,
                                 (QTelephony::Result)result.resultCode() );
}

GreenphoneCellBroadcast::GreenphoneCellBroadcast( QModemService *service )
    : QModemCellBroadcast( service )
{
    this->service = service;
}

GreenphoneCellBroadcast::~GreenphoneCellBroadcast()
{
}

bool GreenphoneCellBroadcast::isRegistered() const
{
    QNetworkRegistration *netReg = service->interface<QNetworkRegistration>();
    QTelephony::RegistrationState state = netReg->registrationState();
    switch ( state ) {

        case QTelephony::RegistrationHome:
        case QTelephony::RegistrationUnknown:
        case QTelephony::RegistrationRoaming:
            return true;

        default: break;
    }
    return false;
}

static QString scsbCommand( int mode, const QList<int>& channels )
{
    QString cmd = "AT+CSCB=" + QString::number( mode ) + ",\"";
    bool first = true;
    foreach ( int channel, channels ) {
        if ( !first )
            cmd += ",";
        else
            first = false;
        cmd += QString::number( channel );
    }
    cmd += "\"";
    return cmd;
}

void GreenphoneCellBroadcast::suspend()
{
    if ( isRegistered() && channels().contains(50) ) {
        // Turn off the cell location channel.  Leave the others on for now.
        service->secondaryAtChat()->chat( "AT+CSCB=1,\"50\"" );
    }
}

void GreenphoneCellBroadcast::resume()
{
    if ( isRegistered() && channels().contains(50) ) {
        // Turn the cell broadcast channels back on.
        service->secondaryAtChat()->chat( scsbCommand( 0, channels() ) );
    }
}

bool GreenphoneCellBroadcast::removeBeforeChange() const
{
    // We have to remove existing channels before we can change to
    // the new ones, because AT+CSCB=0 adds to the existing list
    // rather than replacing it with a new list.
    return true;
}

bool GreenphoneCellBroadcast::removeOneByOne() const
{
    // The AT+CSCB=1 command needs to remove channels one at a time.
    return true;
}

GreenphoneConfiguration::GreenphoneConfiguration
        ( GreenphoneModemService *service )
    : QModemConfiguration( service )
{
    this->service = service;
}

GreenphoneConfiguration::~GreenphoneConfiguration()
{
}

void GreenphoneConfiguration::request( const QString& name )
{
    if ( name == "extraVersion" ) {
        service->primaryAtChat()->chat
            ( "AT*MVERS", this, SLOT(mvers(bool,QAtResult)) );
    } else {
        QModemConfiguration::request( name );
    }
}

void GreenphoneConfiguration::mvers( bool, const QAtResult& result )
{
    QString value = result.content().trimmed();
    value.replace( QChar('/'), QChar('\n') );
    int index = value.indexOf( QChar('\n') );
    if ( index >= 0 && value.startsWith( QChar('R') ) &&
         value.indexOf( QChar(':') ) > index ) {
        // Remove the first line, which duplicates the revision information.
        value = value.mid( index + 1 );
    }
    emit notification( "extraVersion", value );
}

GreenphoneCallVolume::GreenphoneCallVolume( GreenphoneModemService *service )
    : QModemCallVolume( service )
{
    this->service = service;

    connect( service, SIGNAL(allCommandsReady()),
             this, SLOT(callVolumesReady()) );
}

GreenphoneCallVolume::~GreenphoneCallVolume()
{
}

bool GreenphoneCallVolume::hasDelayedInit() const
{
    return true;
}

GreenphoneModemService::GreenphoneModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent )
    : QModemService( service, mux, parent )
{
    smsRetryCount = 0;

    // Greenphone uses octal escapes in strings instead of the standard hex.
    QAtUtils::setOctalEscapes( true );

    primaryAtChat()->registerNotificationType
        ( "*MRDY:", this, SLOT(mrdyNotification(QString)) );
    primaryAtChat()->registerNotificationType
        ( "*MCSQ:", this, SLOT(mcsqNotification(QString)), true );
    primaryAtChat()->registerNotificationType
        ( "*MTZ: 2,", this, SLOT(mtz2Notification(QString)), true );
    primaryAtChat()->registerNotificationType
        ( "+CMS ERROR: 322", this, SLOT(cmsError322()) );

    connect( this, SIGNAL(resetModem()), this, SLOT(reset()) );

    // Turn on Greenphone-style call monitoring so that we can
    // distinguish between call in progress and call connected.
    chat( "AT*MCAM=1" );

    // Make sure command echo is enabled, and then turn on the
    // retry on non-echo timer.  This is needed because sometimes
    // the greenphone stalls and the only way to wake it up is
    // to retry the last command.  30 seconds is a long time,
    // but the greenphone also has a habit of not emitting the
    // echo until it has results to report - and some commands
    // like AT+COPS=? can take a long time.
    primaryAtChat()->chat( "ATE1" );
    secondaryAtChat()->chat( "ATE1" );
    primaryAtChat()->setRetryOnNonEcho( 30000 );
    secondaryAtChat()->setRetryOnNonEcho( 30000 );

    // Enable switching of the Modem Input from Normal to Auxiliary (Mono) Input / Output
    QtopiaIpcAdaptor *adaptor
            = new QtopiaIpcAdaptor( "QPE/GreenphoneModem", this );
    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setOutput(int)), this, SLOT(setOutput(int)) );
    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setNoiseSuppression(int)), this, SLOT(setNoiseSuppression(int)) );
    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setEchoCancellation(int)), this, SLOT(setEchoCancellation(int)) );
}

GreenphoneModemService::~GreenphoneModemService()
{
}

void GreenphoneModemService::initialize()
{
    if ( !supports<QSimInfo>() )
        addInterface( new GreenphoneSimInfo( this ) );

    if ( !supports<QSimToolkit>() )
        addInterface( new GreenphoneSimToolkit( this ) );

    if ( !supports<QPhoneBook>() )
        addInterface( new GreenphonePhoneBook( this ) );

    if ( !supports<QSimFiles>() )
        addInterface( new GreenphoneSimFiles( this ) );

    if ( !supports<QBandSelection>() )
        addInterface( new GreenphoneBandSelection( this ) );

    if ( !supports<QVibrateAccessory>() )
        addInterface( new GreenphoneVibrateAccessory( this ) );

    if ( !supports<QServiceNumbers>() )
        addInterface( new GreenphoneServiceNumbers( this ) );

    if ( !supports<QModemCellBroadcast>() ) {
        cb = new GreenphoneCellBroadcast( this );
        addInterface( cb );
    } else {
        cb = 0;
    }

    if ( !supports<QTelephonyConfiguration>() )
        addInterface( new GreenphoneConfiguration( this ) );

    if ( !supports<QCallVolume>() )
        addInterface( new GreenphoneCallVolume( this ) );

    if ( !callProvider() )
        setCallProvider( new GreenphoneCallProvider( this ) );

    QModemService::initialize();
}

void GreenphoneModemService::mrdyNotification( const QString& msg )
{
    uint posn = 6;
    uint event = QAtUtils::parseNumber( msg, posn );
    switch ( event ) {

        case 1:                 emit moduleIsReady(); break;
        case 2:                 emit emergencyCallsPossible(); break;

        case 3:                 
        {
            // Force the modem to wake up when we receive "*MRDY: 3".
            QtopiaChannel::send("QPE/GreenphoneModem", "forceWakeup()");

            // Now tell interested parties that AT commands are ready to use.
            emit allCommandsReady();
        }
        break;

        case 4:                 emit simInserted(); break;
        case 5:                 emit simRemoved(); break;
        case 6:                 emit noNetworkService(); break;
        case 7:                 emit emergencyCallsOnly(); break;

    }
}

void GreenphoneModemService::mcsqNotification( const QString& msg )
{
    // Automatic signal quality update from the greenphone.
    uint posn = 7;
    uint rssi = QAtUtils::parseNumber( msg, posn );
    indicators()->setSignalQuality( (int)rssi, 31 );
}

void GreenphoneModemService::mtz2Notification( const QString& msg )
{
    // Timezone information from the network.
    uint posn = 8;
    QString time = QAtUtils::nextString( msg, posn );
    int dst = ((int)QAtUtils::parseNumber( msg, posn )) * 60;
    int zoneIndex = time.length();
    while ( zoneIndex > 0 && time[zoneIndex - 1] != QChar('-') &&
            time[zoneIndex - 1] != QChar('+') )
        --zoneIndex;
    int zoneOffset;
    if ( zoneIndex > 0 && time[zoneIndex - 1] == QChar('-') ) {
        zoneOffset = time.mid(zoneIndex - 1).toInt() * 15;
    } else if ( zoneIndex > 0 && time[zoneIndex - 1] == QChar('+') ) {
        zoneOffset = time.mid(zoneIndex).toInt() * 15;
    } else {
        // Unknown timezone information.
        return;
    }
    QString timeString;
    if (zoneIndex > 0)
        timeString = time.mid(0, zoneIndex - 1);
    else
        timeString = time;
    QDateTime t = QDateTime::fromString(timeString, "MM/dd/yyyy, HH:mm:ss");
    if ( t.isValid() ) {
        QDateTime utc = QDateTime(t.date(), t.time(), Qt::UTC);
        utc = utc.addSecs(-zoneOffset * 60);
        indicators()->setNetworkTime( utc.toTime_t(), zoneOffset, dst );
    }
}

void GreenphoneModemService::reset()
{
    // AT*MCNFG=1,1 needs to be sent before AT*MCAM or it won't work.
    // This command also turns on incoming SMS events, SMS memory full
    // events, and phone book ready events.
    chat( "AT*MCNFG=1,1" );

    // Configure the SMS system so that messages will automatically
    // change from UNREAD to READ when the +CMGL list command is used.
    // This is the expected behaviour for the higher phone library layers.
    chat( "AT*MCNFG=2,1" );

    // Make sure that "AT*MCAM" is re-enabled after a reset.
    chat( "AT*MCAM=1" );

    // Turn on unsolicited signal quality notifications.
    chat( "AT*MCSQ=1" );

    setNoiseSuppression(1);
    setEchoCancellation(1);
}

void GreenphoneModemService::setOutput(int output)
{
    // Can also use *MVCHN? to find the current mode
    // and *MVCHN=? to list the modes
    // mode 0 = use handset
    // mode 1 = use headset / btheadset

    qLog(AudioState) << "Setting AT*MVCHN=" << output;

    if (output == 0) {
        chat ( "AT*MVCHN=0" );
    }
    else {
        chat( "AT*MVCHN=1" );
    }
}

void GreenphoneModemService::setNoiseSuppression(int val)
{
    if (val == 0) {
        chat( "AT*MECNS=2,0" );
    }
    else {
        chat( "AT*MECNS=2,1" );
    }
}

void GreenphoneModemService::setEchoCancellation(int val)
{
    if (val == 0) {
        chat( "AT*MECNS=1,0" );
    }
    else {
        chat( "AT*MECNS=1,1" );
    }
}

void GreenphoneModemService::sendSuspendDone()
{
    QtopiaChannel::send("QPE/GreenphoneModem", "sleep()");
    suspendDone();
}

void GreenphoneModemService::mcsqOff()
{
    // We have to wait a little bit because there is probably a
    // pending AT+COPS? command in the secondary atchat queue
    // due to changing the AT+CREG notification mode.  We want
    // it to complete before putting the system to sleep or
    // we will wake straight back up again.
    QTimer::singleShot( 500, this, SLOT(sendSuspendDone()) );
}

void GreenphoneModemService::mcsqOn()
{
    wakeDone();
}

void GreenphoneModemService::cmsError322()
{
    // The greenphone sends an unsolicited "+CMS ERROR: 322" when
    // an SMS message arrives, but the SIM memory was full.
    indicators()->setSmsMemoryFull
        ( QModemIndicators::SmsMemoryMessageRejected );
}

void GreenphoneModemService::cmgfDone( bool ok )
{
    if ( ok ) {
        // AT+CMGF succeeded - switch to sending AT+CPMS="SM".
        sendCPMS();
    } else if ( --smsRetryCount > 0 ) {
        QTimer::singleShot( 1000, this, SLOT(sendCMGF()) );
    }
}

void GreenphoneModemService::cpmsDone( bool ok )
{
    if ( ok ) {
        smsRetryCount = 0;
        post( "smsready" );
    } else if ( --smsRetryCount > 0 ) {
        QTimer::singleShot( 1000, this, SLOT(sendCPMS()) );
    }
}

void GreenphoneModemService::sendCMGF()
{
    chat( "AT+CMGF=0", this, SLOT(cmgfDone(bool)) );
}

void GreenphoneModemService::sendCPMS()
{
    chat( "AT+CPMS=\"SM\"", this, SLOT(cpmsDone(bool)) );
}

void GreenphoneModemService::needSms()
{
    // There are some cases where the greenphone will respond with
    // "OK" to AT+CMGF=0, but will fail AT+CPMS="SM" with "SIM busy".
    // Therefore, the AT+CMGF=0 poll logic in the base class won't work.
    // Here, we poll AT+CMGF=0 until it succeeds, and then switch to
    // polling AT+CPMS="SM" until it succeeds.

    // Update the final timeout if there is a ready check in progress,
    // but otherwise let it continue.
    if ( smsRetryCount > 0 ) {
        smsRetryCount = 15;
        return;
    }

    // Set the initial retry count and send the first check command.
    smsRetryCount = 15;
    sendCMGF();
}

void GreenphoneModemService::suspend()
{
    // Turn off cell id information on +CREG and +CGREG as it will
    // cause unnecessary wakeups when moving between cells.
    chat( "AT+CREG=1" );
    chat( "AT+CGREG=1" );

    // Turn off cell broadcast location messages.
    if ( cb )
        cb->suspend();

    // Turn off signal quality notifications while the system is suspended.
    chat( "AT*MCSQ=0", this, SLOT(mcsqOff()) );
}

void GreenphoneModemService::wake()
{
    // Turn cell id information back on.
    chat( "AT+CREG=2" );
    chat( "AT+CGREG=2" );

    // Turn cell broadcast location messages back on again.
    if ( cb )
        cb->resume();

    // Re-enable signal quality notifications when the system wakes up again.
    chat( "AT*MCSQ=1", this, SLOT(mcsqOn()) );
}
