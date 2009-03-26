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

#include "simapplication.h"
#include <qatutils.h>
#include <qdebug.h>

class SimApplicationPrivate
{
public:
    SimApplicationPrivate()
    {
        rules = 0;
        expectedType = QSimCommand::NoCommand;
        target = 0;
        slot = 0;
        inResponse = false;
    }

    SimRules   *rules;
    QSimCommand::Type expectedType;
    QByteArray  currentCommand;
    QObject    *target;
    const char *slot;
    bool inResponse;
};

SimApplication::SimApplication( QObject *parent )
    : QObject( parent )
{
    d = new SimApplicationPrivate();
}

SimApplication::~SimApplication()
{
    delete d;
}

/*!
    Sends a proactive SIM command, \a cmd, to the ME, and instructs
    SimApplication to invoke \a slot on \a target when the response
    arrives back.  The \a options provides extra information about
    how the command should be transmitted to the ME in PDU form.

    For \c SetupMenu commands, \a target and \a slot should be null.
    When the user responds to the main menu, mainMenuSelection() or
    mainMenuHelpRequest() will be invoked.

    \sa mainMenuSelection(), mainMenuHelpRequest()
*/
void SimApplication::command( const QSimCommand& cmd,
                              QObject *target, const char *slot,
                              QSimCommand::ToPduOptions options )
{
    // Record the command details, together with the type of
    // TERMINAL RESPONSE or ENVELOPE that we expect in answer.
    d->currentCommand = cmd.toPdu( options );
    d->expectedType = cmd.type();
    d->target = target;
    d->slot = slot;

    // Send an unsolicited notification to indicate that a new
    // proactive SIM command is available.  If we are already in
    // the middle of processing a TERMINAL RESPONSE or ENVELOPE,
    // then delay the unsolicited notification until later.
    if ( d->rules && !d->inResponse ) {
        d->rules->unsolicited
            ( "*TCMD: " + QString::number( d->currentCommand.size() ) );
    }
}

/*!
    Sends a call control \a event to the ME.
*/
void SimApplication::controlEvent( const QSimControlEvent& event )
{
    if ( d->rules ) {
        d->rules->unsolicited
            ( "*TCC: " + QString::number( (int)(event.type()) ) +
              "," + QAtUtils::toHex( event.toPdu() ) );
    }
}

/*!
    Starts the SIM application.  The default implementation calls mainMenu().

    \sa abort(), mainMenu()
*/
void SimApplication::start()
{
    mainMenu();
}

/*!
    Aborts the SIM application and forces it to return to the main menu.
    The default implementation clears the pending command for FETCH
    and then calls mainMenu().

    This function is called whenever a \c{TERMINAL PROFILE} command is
    received from the ME.

    \sa start(), mainMenu()
*/
void SimApplication::abort()
{
    d->expectedType = QSimCommand::NoCommand;
    d->currentCommand = QByteArray();
    d->target = 0;
    d->slot = 0;
    mainMenu();
}

/*!
    \fn void SimApplication::mainMenu()

    Builds and sends the main menu to the ME using the command() method.

    \sa mainMenuSelection(), mainMenuHelpRequest()
*/

/*!
    Indicates that the main menu item with the identifier \a id
    has been selected.  The default implementation re-sends
    the main menu.

    \sa mainMenu(), mainMenuHelpRequest()
*/
void SimApplication::mainMenuSelection( int id )
{
    Q_UNUSED(id);
    mainMenu();
}

/*!
    Indicates that help has been requested for the main menu item
    with the identifier \a id.  The default implementation
    re-sends the main menu.

    \sa mainMenu(), mainMenuSelection()
*/
void SimApplication::mainMenuHelpRequest( int id )
{
    Q_UNUSED(id);
    mainMenu();
}

void SimApplication::setSimRules( SimRules *rules )
{
    d->rules = rules;
}

bool SimApplication::execute( const QString& cmd )
{
    // Process SIM toolkit begin and end commands by forcing the app back to the main menu.
    if ( cmd == "AT*TSTB" || cmd == "AT*TSTE") {
        d->rules->respond( "OK" );
        abort();
        return true;
    }

    // If not AT+CSIM, then this is not a SIM toolkit command.
    if ( !cmd.startsWith( "AT+CSIM=" ) )
        return false;

    // Extract the binary payload of the AT+CSIM command.
    int comma = cmd.indexOf( QChar(',') );
    if ( comma < 0 )
        return false;
    QByteArray param = QAtUtils::fromHex( cmd.mid(comma + 1) );
    if ( param.length() < 5 || param[0] != (char)0xA0 )
        return false;

    // Check for TERMINAL PROFILE, FETCH, TERMINAL RESPONSE,
    // and ENVELOPE packets.
    if ( param[1] == (char)0x10 ) {
        // Download of a TERMINAL PROFILE.  We respond with a simple OK,
        // on the assumption that what we were sent was valid.
        d->rules->respond( "+CSIM: 4,9000\nOK" );

        // Abort the SIM application and force it to return to the main menu.
        abort();
    } else if ( param[1] == (char)0x12 ) {
        // Fetch the current command contents.
        QByteArray resp = d->currentCommand;
        if ( resp.isEmpty() ) {
            // We weren't expecting a FETCH.
            d->rules->respond( "+CSIM: 4,6F00\nOK" );
            return true;
        }
        resp += (char)0x90;
        resp += (char)0x00;
        d->rules->respond( "+CSIM: " + QString::number( resp.size() * 2 ) + "," +
                           QAtUtils::toHex( resp ) + "\nOK" );
    } else if ( param[1] == (char)0x14 ) {
        // Process a TERMINAL RESPONSE message.
        QSimTerminalResponse resp;
        resp = QSimTerminalResponse::fromPdu( param.mid(5) );
        if ( resp.command().type() != QSimCommand::NoCommand &&
             resp.command().type() != d->expectedType ) {
            // Response to the wrong type of command.
            d->rules->respond( "+CSIM: 4,6F00\nOK" );
            return true;
        }
        response( resp );
    } else if ( param[1] == (char)0xC2 ) {
        // Process a menu selection ENVELOPE message.  We turn it into a
        // QSimTerminalResponse to make it easier to process.
        QSimEnvelope env;
        env = QSimEnvelope::fromPdu( param.mid(5) );
        if ( env.type() == QSimEnvelope::EventDownload ) {
            d->rules->respond( "+CSIM: 4,9000\nOK" );
            return true;
        }
        if ( env.type() != QSimEnvelope::MenuSelection )
            return false;
        if ( d->expectedType != QSimCommand::SetupMenu ) {
            // Envelope sent for the wrong type of command.
            d->rules->respond( "+CSIM: 4,6F00\nOK" );
            return true;
        }
        d->rules->respond( "+CSIM: 4,9000\nOK" );
        d->expectedType = QSimCommand::NoCommand;
        d->currentCommand = QByteArray();
        d->target = 0;
        d->slot = 0;
        if ( env.requestHelp() )
            mainMenuHelpRequest( env.menuItem() );
        else
            mainMenuSelection( env.menuItem() );
    } else {
        // This SIM command is not related to SIM toolkit - ignore it.
        return false;
    }

    return true;
}

void SimApplication::response( const QSimTerminalResponse& resp )
{
    // Save the target information.
    QObject *target = d->target;
    const char *slot = d->slot;

    // Clear the command details, in preparation for a new command.
    if ( resp.command().type() != QSimCommand::SetupMenu ) {
        d->expectedType = QSimCommand::NoCommand;
        d->currentCommand = QByteArray();
    }
    d->target = 0;
    d->slot = 0;

    // Process the response.
    d->inResponse = true;
    if ( target && slot ) {
        // Invoke the slot and pass "resp" to it.
        QByteArray name( slot + 1 );
        name = QMetaObject::normalizedSignature( name.constData() );
        int index = target->metaObject()->indexOfMethod( name.constData() );
        if ( index != -1 ) {
            void *args[2];
            args[0] = 0;
            args[1] = (void *)&resp;
            target->qt_metacall
                ( QMetaObject::InvokeMetaMethod, index, args );
        }
    }
    d->inResponse = false;

    // Answer the AT+CSIM command and send notification of the new command.
    if ( !d->rules )
        return;
    if ( d->currentCommand.isEmpty() || resp.command().type() == QSimCommand::SetupMenu ) {
        // No new command, so respond with a simple OK.
        d->rules->respond( "+CSIM: 4,9000\nOK" );
    } else {
        // There is a new command, so send back 91XX to the TERMINAL RESPONSE
        // or ENVELOPE request to indicate that we have another command to
        // be fetched.  Then send the unsolicited "*TCMD" notification.
        QByteArray data;
        data += (char)0x91;
        data += (char)(d->currentCommand.size());
        d->rules->respond( "+CSIM: " + QString::number( data.size() * 2 ) + "," +
                           QAtUtils::toHex( data ) + "\nOK" );
        d->rules->unsolicited
            ( "*TCMD: " + QString::number( d->currentCommand.size() ) );
    }
}

DemoSimApplication::DemoSimApplication( QObject *parent )
    : SimApplication( parent )
{
}

DemoSimApplication::~DemoSimApplication()
{
}

#define MainMenu_News       1
#define MainMenu_Sports     2
#define MainMenu_Time       3
#define MainMenu_SticksGame 4
#define MainMenu_Tones      5
#define MainMenu_Icons      6
#define MainMenu_IconsSE    7
#define MainMenu_Finance    8
#define MainMenu_Browser    9

#define SportsMenu_Chess        1
#define SportsMenu_Painting     2
#define SportsMenu_Snakes       3
#define SportsMenu_Main         4

void DemoSimApplication::mainMenu()
{
    QSimCommand cmd;
    QSimMenuItem item;
    QList<QSimMenuItem> items;

    cmd.setType( QSimCommand::SetupMenu );

    item.setIdentifier( MainMenu_News );
    item.setLabel( "News" );
    items += item;

    item.setIdentifier( MainMenu_Sports );
    item.setLabel( "Sports" );
    items += item;

    item.setIdentifier( MainMenu_Time );
    item.setLabel( "Time" );
    items += item;

    item.setIdentifier( MainMenu_SticksGame );
    item.setLabel( "Sticks Game" );
    items += item;

    item.setIdentifier( MainMenu_Tones );
    item.setLabel( "Tones" );
    items += item;

    item.setIdentifier( MainMenu_Icons );
    item.setLabel( "Icons (not self-explanatory)" );
    items += item;

    item.setIdentifier( MainMenu_IconsSE );
    item.setLabel( "Icons (self-explanatory)" );
    items += item;

    item.setIdentifier( MainMenu_Finance );
    item.setLabel( "Finance" );
    items += item;

    item.setIdentifier( MainMenu_Browser );
    item.setLabel( "Web Browser" );
    items += item;

    cmd.setMenuItems( items );

    command( cmd, 0, 0 );
}

void DemoSimApplication::sendDisplayText()
{
    // Display a text string and then go back to the main menu once the
    // text is accepted by the user.
    QSimCommand cmd;
    cmd.setType( QSimCommand::DisplayText );
    cmd.setDestinationDevice( QSimCommand::Display );
    cmd.setClearAfterDelay(false);
    cmd.setImmediateResponse(true);
    cmd.setHighPriority(false);
    immediateResponse = true;
    cmd.setText( "Police today arrested a man on suspicion "
            "of making phone calls while intoxicated.  Witnesses claimed "
            "that they heard the man exclaim \"I washent dwinkn!\" as "
            "officers escorted him away." );
    command( cmd, this, SLOT(displayTextResponse(QSimTerminalResponse)) );
}

void DemoSimApplication::mainMenuSelection( int id )
{
    QSimCommand cmd;

    switch ( id ) {

        case MainMenu_News:
        {
            QTimer::singleShot( 0, this, SLOT(sendDisplayText()) );
        }
        break;

        case MainMenu_Sports:
        {
            sendSportsMenu();
        }
        break;

        case MainMenu_Time:
        {
            cmd.setType( QSimCommand::SetupCall );
            cmd.setNumber( "1194" );
            cmd.setText( "Dialing the Time Guy ..." );
            command( cmd, this, SLOT(mainMenu()) );
        }
        break;

        case MainMenu_SticksGame:
        {
            startSticksGame();
        }
        break;

        case MainMenu_Tones:
        {
            sendToneMenu();
        }
        break;

        case MainMenu_Icons:
        {
            sendIconMenu();
        }
        break;

        case MainMenu_IconsSE:
        {
            sendIconSEMenu();
        }
        break;

        case MainMenu_Finance:
        {
            cmd.setType( QSimCommand::GetInput );
            cmd.setText( "Enter code" );
            cmd.setWantDigits( true );
            cmd.setMinimumLength( 3 );
            cmd.setHasHelp( true );
            command( cmd, this, SLOT(getInputLoop(QSimTerminalResponse)) );
        }
        break;

        case MainMenu_Browser:
        {
            sendBrowserMenu();
        }
        break;

        default:
        {
            // Don't know what this item is, so just re-display the main menu.
            mainMenu();
        }
        break;
    }
}

void DemoSimApplication::sendSportsMenu()
{
    QSimCommand cmd;
    QSimMenuItem item;
    QList<QSimMenuItem> items;

    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( "Sports" );

    item.setIdentifier( SportsMenu_Chess );
    item.setLabel( "Chess" );
    items += item;

    item.setIdentifier( SportsMenu_Painting );
    item.setLabel( "Finger Painting" );
    items += item;

    item.setIdentifier( SportsMenu_Snakes );
    item.setLabel( "Snakes and Ladders" );
    items += item;

    item.setIdentifier( SportsMenu_Main );
    item.setLabel( "Return to main menu" );
    items += item;

    cmd.setMenuItems( items );

    command( cmd, this, SLOT(sportsMenu(QSimTerminalResponse)) );
}

void DemoSimApplication::sportsMenu( const QSimTerminalResponse& resp )
{
    QSimCommand cmd;

    if ( resp.result() == QSimTerminalResponse::Success ) {
        // Item selected.
        switch ( resp.menuItem() ) {

            case SportsMenu_Chess:
            {
                cmd.setType( QSimCommand::DisplayText );
                cmd.setDestinationDevice( QSimCommand::Display );
                cmd.setText( "Kasparov 3, Deep Blue 4" );
                command( cmd, this, SLOT(sendSportsMenu()) );
            }
            break;

            case SportsMenu_Painting:
            {
                cmd.setType( QSimCommand::DisplayText );
                cmd.setDestinationDevice( QSimCommand::Display );
                cmd.setText( "Little Johnny 4, Little Sally 6" );
                command( cmd, this, SLOT(sendSportsMenu()) );
            }
            break;

            case SportsMenu_Snakes:
            {
                cmd.setType( QSimCommand::DisplayText );
                cmd.setDestinationDevice( QSimCommand::Display );
                cmd.setText( "Little Johnny 0, Little Sally 2" );
                cmd.setClearAfterDelay( true );
                command( cmd, this, SLOT(sendSportsMenu()) );
            }
            break;

            default:    mainMenu(); break;
        }
    } else if ( resp.result() == QSimTerminalResponse::BackwardMove ) {
        // Request to move backward.
        mainMenu();
    } else {
        // Unknown response - just go back to the main menu.
        mainMenu();
    }
}

void DemoSimApplication::startSticksGame()
{
    sticksLeft = 21;
    sticksGameShow();
}

void DemoSimApplication::sticksGameShow()
{
    QSimCommand cmd;
    if ( sticksLeft == 1 ) {
        cmd.setType( QSimCommand::GetInkey );
        cmd.setText( "There is only 1 stick left.  You lose.  Play again?" );
        cmd.setWantYesNo( true );
        command( cmd, this, SLOT(sticksGamePlayAgain(QSimTerminalResponse)) );
    } else {
        cmd.setType( QSimCommand::GetInkey );
        cmd.setText( "There are 21 sticks left.  How many do you take (1, 2, or 3)?" );
        cmd.setWantDigits( true );
        if ( sticksLeft == 21 )
            cmd.setHasHelp( true );
        command( cmd, this, SLOT(sticksGameLoop(QSimTerminalResponse)) );
    }
}

void DemoSimApplication::sticksGameLoop( const QSimTerminalResponse& resp )
{
    QSimCommand cmd;
    if ( resp.result() == QSimTerminalResponse::Success ) {
        // User has selected the number of sticks they want.
        int taken = 0;
        if ( resp.text() == "1" ) {
            taken = 1;
        } else if ( resp.text() == "2" ) {
            taken = 2;
        } else if ( resp.text() == "3" ) {
            taken = 3;
        } else {
            cmd.setType( QSimCommand::GetInkey );
            cmd.setText( "Must be 1, 2, or 3.  There are " + QString::number( sticksLeft ) +
                         " sticks left.  How many sticks do you take?" );
            cmd.setWantDigits( true );
            command( cmd, this, SLOT(sticksGameLoop(QSimTerminalResponse)) );
            return;
        }
        cmd.setType( QSimCommand::DisplayText );
        cmd.setDestinationDevice( QSimCommand::Display );
        cmd.setText( "I take " + QString::number( 4 - taken ) + " sticks." );
        cmd.setClearAfterDelay( true );
        sticksLeft -= 4;
        command( cmd, this, SLOT(sticksGameShow()) );
    } else if ( resp.result() == QSimTerminalResponse::HelpInformationRequested ) {
        // Display help for the game.
        cmd.setType( QSimCommand::DisplayText );
        cmd.setDestinationDevice( QSimCommand::Display );
        cmd.setText( "Starting with 21 sticks, players pick up 1, 2, or 3 sticks at a time.  "
                     "The loser is the player who has to pick up the last stick." );
        command( cmd, this, SLOT(startSticksGame()) );
    } else {
        // Probably aborted.
        mainMenu();
    }
}

void DemoSimApplication::sticksGamePlayAgain( const QSimTerminalResponse& resp )
{
    if ( resp.text() == "Yes" )
        startSticksGame();
    else
        mainMenu();
}

void DemoSimApplication::getInputLoop( const QSimTerminalResponse& resp )
{
    QSimCommand cmd;
    if ( resp.result() == QSimTerminalResponse::HelpInformationRequested ) {
        // Display help for the game.
        cmd.setType( QSimCommand::DisplayText );
        cmd.setDestinationDevice( QSimCommand::Display );
        cmd.setText("Enter code of the company." );
        command( cmd, this, SLOT(mainMenu()) );
    } else {
        mainMenu();
    }
}

void DemoSimApplication::sendToneMenu()
{
    QSimCommand cmd;
    QSimMenuItem item;
    QList<QSimMenuItem> items;

    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( "Tones" );

    item.setIdentifier( (uint)QSimCommand::ToneDial );
    item.setLabel( "Dial" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneBusy );
    item.setLabel( "Busy" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneCongestion );
    item.setLabel( "Congestion" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneRadioAck );
    item.setLabel( "Radio Ack" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneDropped );
    item.setLabel( "Dropped" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneError );
    item.setLabel( "Error" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneCallWaiting );
    item.setLabel( "Call Waiting" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneGeneralBeep );
    item.setLabel( "General Beep" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::TonePositiveBeep );
    item.setLabel( "Positive Beep" );
    items += item;

    item.setIdentifier( (uint)QSimCommand::ToneNegativeBeep );
    item.setLabel( "Negative Beep" );
    items += item;

    cmd.setMenuItems( items );

    command( cmd, this, SLOT(toneMenu(QSimTerminalResponse)) );
}

void DemoSimApplication::toneMenu( const QSimTerminalResponse& resp )
{
    QSimCommand cmd;

    if ( resp.result() == QSimTerminalResponse::Success ) {
        // Item selected.
        cmd.setType( QSimCommand::PlayTone );
        cmd.setDestinationDevice( QSimCommand::Earpiece );
        cmd.setTone( (QSimCommand::Tone)( resp.menuItem() ) );
        if ( cmd.tone() == QSimCommand::ToneDial )
            cmd.setDuration( 5000 );
        else if ( cmd.tone() == QSimCommand::ToneGeneralBeep ||
                  cmd.tone() == QSimCommand::TonePositiveBeep ||
                  cmd.tone() == QSimCommand::ToneNegativeBeep )
            cmd.setDuration( 1000 );
        else
            cmd.setDuration( 3000 );
        command( cmd, this, SLOT(sendToneMenu()) );
    } else if ( resp.result() == QSimTerminalResponse::BackwardMove ) {
        // Request to move backward.
        mainMenu();
    } else {
        // Unknown response - just go back to the main menu.
        mainMenu();
    }
}

void DemoSimApplication::sendIconMenu()
{
    QSimCommand cmd;
    QSimMenuItem item;
    QList<QSimMenuItem> items;

    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( "Icons" );

    item.setIdentifier( 1 );
    item.setLabel( "Basic Icon" );
    item.setIconId( 1 );
    items += item;

    item.setIdentifier( 2 );
    item.setLabel( "Color Icon" );
    item.setIconId( 2 );
    items += item;

    item.setIdentifier( 3 );
    item.setLabel( "Bad Icon" );
    item.setIconId( 70 );
    items += item;

    cmd.setMenuItems( items );

    command( cmd, this, SLOT(iconMenu(QSimTerminalResponse)) );
}

void DemoSimApplication::sendIconSEMenu()
{
    QSimCommand cmd;
    QSimMenuItem item;
    QList<QSimMenuItem> items;

    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( "Icons SE" );

    item.setIdentifier( 1 );
    item.setLabel( "Basic Icon" );
    item.setIconId( 1 );
    item.setIconSelfExplanatory( true );
    items += item;

    item.setIdentifier( 2 );
    item.setLabel( "Color Icon" );
    item.setIconId( 2 );
    item.setIconSelfExplanatory( true );
    items += item;

    item.setIdentifier( 3 );
    item.setLabel( "Bad Icon" );
    item.setIconId( 70 );
    item.setIconSelfExplanatory( true );
    items += item;

    cmd.setMenuItems( items );

    command( cmd, this, SLOT(iconSEMenu(QSimTerminalResponse)) );
}

void DemoSimApplication::iconMenu( const QSimTerminalResponse& resp )
{
    if ( resp.result() == QSimTerminalResponse::Success )
        sendIconMenu();
    else
        mainMenu();
}

void DemoSimApplication::iconSEMenu( const QSimTerminalResponse& resp )
{
    if ( resp.result() == QSimTerminalResponse::Success )
        sendIconSEMenu();
    else
        mainMenu();
}

void DemoSimApplication::displayTextResponse( const QSimTerminalResponse& resp )
{
    QSimCommand cmd;

    if ( resp.result() == QSimTerminalResponse::Success ) {
        if ( immediateResponse )
            return;
        mainMenu();
    } else if ( resp.result() == QSimTerminalResponse::BackwardMove ) {
        // Request to move backward.
        mainMenu();
    } else {
        // Unknown response - just go back to the main menu.
        mainMenu();
    }
}

void DemoSimApplication::sendBrowserMenu()
{
    QSimCommand cmd;
    QSimMenuItem item;
    QList<QSimMenuItem> items;

    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( "Web Browser" );

    item.setIdentifier( 1 );
    item.setLabel( "Qt Extended" );
    items += item;

    item.setIdentifier( 2 );
    item.setLabel( "Google (normal)" );
    items += item;

    item.setIdentifier( 3 );
    item.setLabel( "Google (if browser not in use)" );
    items += item;

    item.setIdentifier( 4 );
    item.setLabel( "Google (clear history)" );
    items += item;

    item.setIdentifier( 5 );
    item.setLabel( "Default Home Page" );
    items += item;

    cmd.setMenuItems( items );

    command( cmd, this, SLOT(browserMenu(QSimTerminalResponse)) );
}

void DemoSimApplication::browserMenu( const QSimTerminalResponse& resp )
{
    QSimCommand cmd;

    if ( resp.result() == QSimTerminalResponse::Success ) {
        // Item selected.
        switch ( resp.menuItem() ) {

            case 1:
            {
                cmd.setType( QSimCommand::LaunchBrowser );
                cmd.setText( "Qt Extended" );
                cmd.setBrowserLaunchMode( QSimCommand::UseExisting );
                cmd.setUrl( "http://www.qtextended.org/" );
                command( cmd, this, SLOT(sendBrowserMenu()) );
            }
            break;

            case 2:
            {
                cmd.setType( QSimCommand::LaunchBrowser );
                cmd.setText( "Google" );
                cmd.setBrowserLaunchMode( QSimCommand::UseExisting );
                cmd.setUrl( "http://www.google.com/" );
                command( cmd, this, SLOT(sendBrowserMenu()) );
            }
            break;

            case 3:
            {
                cmd.setType( QSimCommand::LaunchBrowser );
                cmd.setText( "Google" );
                cmd.setBrowserLaunchMode( QSimCommand::IfNotAlreadyLaunched );
                cmd.setUrl( "http://www.google.com/" );
                command( cmd, this, SLOT(sendBrowserMenu()) );
            }
            break;

            case 4:
            {
                cmd.setType( QSimCommand::LaunchBrowser );
                cmd.setText( "Google" );
                cmd.setBrowserLaunchMode( QSimCommand::CloseExistingAndLaunch );
                cmd.setUrl( "http://www.google.com/" );
                command( cmd, this, SLOT(sendBrowserMenu()) );
            }
            break;

            case 5:
            {
                cmd.setType( QSimCommand::LaunchBrowser );
                cmd.setText( "Default Home Page" );
                cmd.setBrowserLaunchMode( QSimCommand::UseExisting );
                cmd.setUrl( "" );
                command( cmd, this, SLOT(sendBrowserMenu()) );
            }
            break;

            default:    mainMenu(); break;
        }
    } else if ( resp.result() == QSimTerminalResponse::BackwardMove ) {
        // Request to move backward.
        mainMenu();
    } else {
        // Unknown response - just go back to the main menu.
        mainMenu();
    }
}
