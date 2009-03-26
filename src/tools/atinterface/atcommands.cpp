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

#include "atcommands.h"
#include "atparseutils.h"

#include <QAtUtils>
#include <QTimer>
#include <qtopialog.h>
#include <QNetworkRegistration>

#include "atgsmnoncellcommands.h"
#include "atv250commands.h"

#ifdef QTOPIA_CELL
#	include <QSimInfo>
#	include "atgsmcellcommands.h"
#	include "atsmscommands.h"
#endif

#ifdef QTOPIA_BLUETOOTH
#	include "atbluetoothcommands.h"
#endif

AtCommands::AtCommands( AtFrontEnd *frontEnd, AtSessionManager *manager )
    : QObject( frontEnd )
{
    qLog(ModemEmulator) << "AT command session started";

    atFrontEnd = frontEnd;
    atManager = manager;

    m_atgnc = new AtGsmNonCellCommands( this );
#ifdef QTOPIA_CELL
    m_atgcc = new AtGsmCellCommands( this );
    m_atsms = new AtSmsCommands( this );
#endif
#ifdef QTOPIA_BLUETOOTH
    m_atbtc = new AtBluetoothCommands( this );
#endif
    m_atv250c = new AtV250Commands( this );

    // network registration for ATD()
    dataCallRequested = false;

    cmdsPosn = 0;
    result = QAtResult::OK;
    extendedError = QAtResult::OK;

    connect( frontEnd, SIGNAL(commands(QStringList)),
             this, SLOT(commands(QStringList)) );

    AtCallManager *calls = manager->callManager();
    connect( calls, SIGNAL(stateChanged(int,AtCallManager::CallState,QString,QString)),
             this, SLOT(stateChanged(int,AtCallManager::CallState,QString,QString)) );
    connect( calls, SIGNAL(deferredResult(AtCommands*,QAtResult::ResultCode)),
             this, SLOT(deferredResult(AtCommands*,QAtResult::ResultCode)) );
    connect( calls, SIGNAL(ring(QString,QString)),
             this, SLOT(ring(QString,QString)) );
    connect( calls, SIGNAL(dialingOut(bool,bool,bool)),
             this, SLOT(dialingOut(bool,bool,bool)) );
    connect( calls, SIGNAL(outgoingConnected(QString)),
             this, SLOT(outgoingConnected(QString)) );
    connect( calls, SIGNAL(callWaiting(QString,QString)),
             this, SLOT(callWaiting(QString,QString)) );
    connect( calls, SIGNAL(noCarrier()), this, SLOT(noCarrier()) );

    // register those AT commands that this class defines.
    add( "D",        this, SLOT(atd(QString)) );
    add( "+CDIS",    this, SLOT(notAllowed()) );
    add( "+CFUN",    this, SLOT(atcfun(QString)) );
    add( "+CIMI",    this, SLOT(atcimi(QString)) );
    add( "+CMAR",    this, SLOT(ignore()) );
}

AtCommands::~AtCommands()
{
    qLog(ModemEmulator) << "AT command session stopped";
    delete m_atgnc;
#ifdef QTOPIA_CELL
    delete m_atgcc;
    delete m_atsms;
#endif
#ifdef QTOPIA_BLUETOOTH
    delete m_atbtc;
#endif
    delete m_atv250c;
}

AtFrontEnd *AtCommands::frontEnd() const
{
    return atFrontEnd;
}

AtSessionManager *AtCommands::manager() const
{
    return atManager;
}

AtOptions *AtCommands::options() const
{
    return frontEnd()->options();
}

AtGsmNonCellCommands *AtCommands::atgnc() const
{
    return m_atgnc;
}

/*
    Add \a name to the list of commands that are processed by this
    AT interface.  When the command arrives, \a slot on \a target
    will be invoked, with a QString argument corresponding to the
    command parameters.

    The slot should perform whatever processing is necessary and
    then call done() to terminate the results.  If the command
    will be sending lines of data, it should call send() before done().

    The command name should be in upper case and should not start
    with \c AT.  For example, \c{+CGMI}, \c{S0}, \c{D} etc.
*/
void AtCommands::add( const QString& name, QObject *target, const char *slot )
{
    invokers.insert( name, new QSlotInvoker( target, slot, this ) );
}

void AtCommands::send( const QString& line )
{
    frontEnd()->send( line );
}

void AtCommands::done( QAtResult::ResultCode result )
{
    // Record the final result code for the command we just executed.
    this->result = result;

    // Arrange for the next command in sequence to be processed.
    // Do it upon the next event loop entry to prevent the
    // stack from growing too much.
    QTimer::singleShot( 0, this, SLOT(processNextCommand()) );
}

void AtCommands::doneWithExtendedError( QAtResult::ResultCode result )
{
    extendedError = result;
    done( result );
}

void AtCommands::commands( const QStringList& newcmds )
{
    // Start by assuming that the final result will be OK.
    // If it is ever set to non-OK, then the commands stop.
    result = QAtResult::OK;

    // Save the commands.
    cmds = newcmds;
    cmdsPosn = 0;

    // Process the first command.
    processNextCommand();
}

bool AtCommands::invokeCommand( const QString& cmd, const QString& params )
{
    QMap<QString, QSlotInvoker *>::Iterator iter;
    iter = invokers.find( cmd );

    // check to see that the command has been registered.
    if ( iter == invokers.end() ) {
        // no.  return error.
        return false;
    }

    // Notify the call manager which handler will be talking to it
    // to properly deliver deferred results back to this handler.
    manager()->callManager()->setHandler( this );

    // Dispatch the command to the associated slot.
    QList<QVariant> args;
    args += QVariant( params );
    iter.value()->invoke( args );

    return true;
}

void AtCommands::processNextCommand()
{
    // If we have a non-OK result, or no further commands, then stop now.
    if ( result != QAtResult::OK || cmdsPosn >= cmds.size() ) {
        cmdsPosn = cmds.size();
        frontEnd()->send( result );
        return;
    }

    // Remove one command from the queue.
    QString name = cmds[(cmdsPosn)++];
    QString params = cmds[(cmdsPosn)++];

    // Print the full command to the debug output stream.
    qLog(ModemEmulator) << "AT" + name + params;

    // Determine how to dispatch the command.
    QMap<QString, QSlotInvoker *>::Iterator iter;
    iter = invokers.find( name );
    if ( iter == invokers.end() ) {
        // Process the AT+CLAC command according to GSM 27.007, section 8.37.
        // We also supply AT* as a synonym for quicker debugging.
        if ( name == "+CLAC"
        #ifndef QTOPIA_AT_STRICT
            || name == "*"
        #endif
            ) {
            for ( iter = invokers.begin();
                  iter != invokers.end(); ++iter ) {
                send( "AT" + iter.key() );
            }
            send( "AT+CLAC" );
        #ifndef QTOPIA_AT_STRICT
            send( "AT*" );
        #endif
            done( QAtResult::OK );
            return;
        }

        // We don't know how to process this command, so error out.
        // This will stop any further commands from being processed.
        cmdsPosn = cmds.size();
        frontEnd()->send( QAtResult::Error );
        return;
    }

    // Notify the call manager which handler will be talking to it
    // to properly deliver deferred results back to this handler.
    manager()->callManager()->setHandler( this );

    // Dispatch the command to the associated slot.
    QList<QVariant> args;
    args += QVariant( params );
    iter.value()->invoke( args );
}

/*
    Handler that ignores the command and just responds \c OK.
*/
void AtCommands::ignore()
{
    done();
}

/*
    Handler that errors out the command with "not allowed".
*/
void AtCommands::notAllowed()
{
    done( QAtResult::OperationNotAllowed );
}

/*!
    \page modem-emulator.html
    \title Modem Emulator

    The modem emulator component in Qt Extended allows external devices, such
    as laptops and Bluetooth hands-free kits, to send AT commands to a
    Qt Extended Phone to cause it to perform operations on the external device's
    behalf.

    The following sections describe the AT commands that are supported
    by Qt Extended via the Modem Emulator interface.

    \list
        \o \l{Modem Emulator - Call Control}{Call Control Commands}
        \o \l{Modem Emulator - Control and Status}{Control and Status Commands}
        \o \l{Modem Emulator - Phonebook Operations}{Phonebook Commands}
        \o \l{Modem Emulator - Identification}{Identification Commands}
        \o \l{Modem Emulator - Network}{Network Commands}
        \o \l{Modem Emulator - Supplementary Services}{Supplementary Service Commands}
        \o \l{Modem Emulator - GPRS}{GPRS Commands}
        \o \l{Modem Emulator - Short Message Service}{Short Message Service (SMS) Commands}
        \o \l{Modem Emulator - Ignored Commands}{Ignored Commands}
    \endlist

    The modem emulator is accessed by sending QCop service messages to
    the \l{ModemEmulatorService}{ModemEmulator} service as devices that
    need AT command support are connected and disconnected.

    For phones with a standard serial cable for external devices to
    access the phone, the \c{ExternalAccessDevice} option can be
    set in the \c Phone.conf file to specify this device.  See
    \l{GSM Modem Integration#modem-emulator}{GSM Modem Integration: Modem Emulator}
    for more information.

    \nextpage{Modem Emulator - Call Control}
*/

/*!
    \group ModemEmulator::CallControl
    \title Modem Emulator - Call Control

    The AT commands in this section are used for dialing, accepting,
    and manipulating voice and data calls.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Control and Status}
    \previouspage{Modem Emulator}
*/

/*!
    \group ModemEmulator::ControlAndStatus
    \title Modem Emulator - Control and Status
    \section1 Control and Status

    The AT commands in this section are used for controlling state
    information in the modem and for reporting the current status
    of the state information.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Phonebook Operations}
    \previouspage{Modem Emulator - Call Control}
*/

/*!
    \group ModemEmulator::PhoneBook
    \title Modem Emulator - Phonebook Operations
    \section1 Phonebook Operations

    The AT commands in this section are used for accessing the SIM phone book.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Identification}
    \previouspage{Modem Emulator - Control and Status}

*/

/*!
    \group ModemEmulator::Identification
    \title Modem Emulator - Identification

    The AT commands in this section are used for retrieving identification
    information for the modem.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Network}
    \previouspage{Modem Emulator - Phonebook Operations}
*/

/*!
    \group ModemEmulator::Network
    \title Modem Emulator - Network

    The AT commands in this section are used for accessing network
    services on GSM and similar networks.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Supplementary Services}
    \previouspage{Modem Emulator - Identification}
*/

/*!
    \group ModemEmulator::SupplementaryServices
    \title Modem Emulator - Supplementary Services

    The AT commands in this section are used for accessing GSM supplementary
    services such as call waiting, caller-id, etc.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - GPRS}
    \previouspage{Modem Emulator - Network}
*/

/*!
    \group ModemEmulator::GPRS
    \title Modem Emulator - GPRS

    The AT commands in this section are used for setting up GPRS connections.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Short Message Service}
    \previouspage{Modem Emulator - Supplementary Services}
*/

/*!
    \group ModemEmulator::ShortMessageService
    \title Modem Emulator - Short Message Service

    The AT commands in this section are used for accessing the Short Message
    Service (SMS) subsystem.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Ignored Commands}
    \previouspage{Modem Emulator - GPRS}
*/

/*!
    \group ModemEmulator::Ignored
    \title Modem Emulator - Ignored Commands

    The following AT commands are ignored and will always return \c{OK}.
    They exist for compatibility with Recommendation V.250.

    \list
        \o \c{ATL} Monitor speaker loudness.
        \o \c{ATM} Monitor speaker mode.
        \o \c{ATP} Select pulse dialing.
        \o \c{ATS0} Automatic answer.
        \o \c{ATS6} Pause before blind dialing.
        \o \c{ATS7} Connection completion timeout.
        \o \c{ATS8} Comma dial modifier time.
        \o \c{ATS10} Automatic disconnect delay.
        \o \c{ATT} Select tone dialing.
        \o \c{ATX} Result code selection and call progress monitoring control.
        \o \c{AT&C} Circuit 109 (Received line signal detector) behavior.
        \o \c{AT&D} Circuit 108 (Data terminal ready) behavior.
        \o \c{AT+GCI} Country of installation.
        \o \c{AT+GOI} Request global object identification.
    \endlist

    \previouspage{Modem Emulator - Short Message Service}
*/


/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATD Dial Call}
    \compat

    The \c{ATD} command initiates a dial for a voice or data call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATDdialstring[i][;]}
         \o \list
               \o \c{CONNECT [<rate>]}: Data call has connected at \c rate.
               \o \c{OK}: Voice call has connected.
               \o \c{NO CARRIER}: Connection could not be established.
               \o \c{BUSY}: Called party is busy.
               \o \c{ERROR}: Command issues when already connected.
            \endlist
    \endtable

    The command name is followed by a string of digits and control
    characters, according to the following table:

    \table
    \row \o \c{0-9, *, #, +, A, B, C, D}
         \o Digits to be dialed.  In GSM networks, \c{D} is ignored,
            in accordance with 3GPP TS 27.007.
    \row \o \c{,} \o Insert a pause into the dialing sequence.
    \row \o \c{T}, \c{P} \o Select tone or pulse dialing.  These are ignored.
    \row \o \c{!}
         \o Insert a hook flash into the dialing sequence.  This is ignored
            in GSM networks.
    \row \o \c{W} \o Wait for dial tone.  This is ignored in GSM networks.
    \row \o \c{@} \o Wait for quiet answer.  This is ignored in GSM networks.
    \row \o \c{;}
         \o This must be last character in the dialing string, and indicates
            a voice call rather than a data call.  The system will issue
            \c{OK} and immediately return to command mode.
    \row \o \c{i}
         \o Allow the local user's caller ID information to be presented
            to the called party.
    \row \o \c{I}
         \o Suppress the local user's caller ID information from being
            presented to the called party.
    \endtable

    If the dialing sequence begins with \c{>}, then the rest of the
    sequence, up until the semi-colon, is interpreted as a name in a
    direct-dialing phonebook.  This will only work with GSM modems that
    support the feature.  Dialing by explicit number is recommended.

    If the system has been configured with support for VoIP, then VoIP
    calls can be placed with the \c{ATD} command by using the full
    URI of the called party.  For example: \c{ATDsip:fred@example.com;}.

    Conforms with: Recommendation V.250, 3GPP TS 27.007.
*/
void AtCommands::atd( const QString& params )
{
    // first, check that the only semicolon is the last character.
    if ( (params.indexOf(';') != -1) && 
         (params.indexOf(';') != (params.length()-1)) ) {
        done( QAtResult::OperationNotAllowed );
        return;
    }

    bool isDataCall = false;
    if ( !params.endsWith( ";" ) ) {
        if ( atgnc()->networkRegistration()->registrationState() != QTelephony::RegistrationHome &&
                atgnc()->networkRegistration()->registrationState() != QTelephony::RegistrationRoaming ) {
            //no network -> cannot make data calls
            done( QAtResult::NoDialtone );
            return;
        }
        isDataCall = true;
    }
    QAtResult::ResultCode result = manager()->callManager()->dial( params );
    if ( result != AtCallManager::Defer )
        done( result );
    else if ( isDataCall ) //deferred data call
        dataCallRequested = true; //remember that we are in data call
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CFUN Set Phone Functionality}
    \compat

    The \c{AT+CFUN} command returns information about the current
    functionality level of the phone, and allows the level to be
    modified.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CFUN=<fun>[,<rst>]} \o \c{OK}, \c{ERROR}
    \row \o \c{AT+CFUN?} \o \c{+CFUN: <fun>}
    \row \o \c{AT+CFUN=?} \o \c{+CFUN: (0-4),(0-1)}
    \endtable

    Set command selects the level of functionality \c{<fun>} in the MT.
    Read command returns the current level.  Test command returns the
    values that are supported by the MT.

    \table
    \row \o \c{<fun>}
         \o \list
                \o 0 minimum functionality
                \o 1 full functionality
                \o 2 disable phone transmit RF circuits only
                \o 3 disable phone receive RF circuits only
                \o 4 disable phone both transmit and receive RF circuits
            \endlist
    \row \o \c{<rst>}
         \o \list
                \o 0 do not reset the MT before setting it to \c{<fun>}
                \o 1 reset the MT before setting it to \c{<fun>} (not supported at present)
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcfun( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
        #ifdef QTOPIA_CELL
            send( "+CFUN: " + QString::number( (int)(m_atgcc->phonerf->level()) ) );
        #else
            send( "+CFUN: 1" );
        #endif
            done();
        }
        break;

        case AtParseUtils::Set:
        {
        #ifdef QTOPIA_CELL
            // Process the <fun> parameter.  We ignore the <rst> parameter.
            uint posn = 1;
            uint fun = QAtUtils::parseNumber( params, posn );
            if ( m_atgcc->phonerf->available() && fun <= 4 ) {
                m_atgcc->settingPhoneRf = true;
                m_atgcc->phonerf->setLevel( (QPhoneRfFunctionality::Level)fun );
            } else {
                done( QAtResult::OperationNotAllowed );
            }
        #else
            done( QAtResult::OperationNotAllowed );
        #endif
        }
        break;

        case AtParseUtils::Support:
        {
            send( "+CFUN: (0-4),(0-1)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}


/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CIMI Request International Mobile Subscriber Identity}
    \compat

    The \c{AT+CIMI} command retrieves the IMSI value from the SIM.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CIMI} \o \list \o \c{<IMSI>} \o \c{+CME ERROR: 10} \endlist
    \row \o \c{AT+CIMI=?} \o
    \endtable

    An error will be returned if there is no SIM present, or the SIM
    is still being initialized and the IMSI is not available yet.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcimi( const QString& params )
{
    switch( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
#ifdef QTOPIA_CELL
            QSimInfo simInfo;
            QString imsi = simInfo.identity();
            if ( imsi.isEmpty() ) {
                done( QAtResult::SimNotInserted );
            } else {
                send( imsi );
                done();
            }
#else
            done( QAtResult::SimNotInserted );
#endif
        }
        break;

        case AtParseUtils::Support:
        {
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

// State of a phone call has changed.
void AtCommands::stateChanged
        ( int callID, AtCallManager::CallState state,
          const QString& number, const QString& type )
{
    if ( options()->qcam ) {
        QString line;
        line = "*QCAV: " + QString::number( callID ) + "," +
               QString::number( (int)state ) + "," +
               QString::number( AtCallManager::numCallType( type ) );
        if ( state != AtCallManager::CallIdle && !number.isEmpty() ) {
            line += "," + QAtUtils::encodeNumber( number );
        }
        send( line );
    }
}

// Deferred result code from the call manager.
void AtCommands::deferredResult
        ( AtCommands *handler, QAtResult::ResultCode result )
{
    if ( handler != this )
        return;

    //if we have a connect we forward the raw data traffic
    if ( dataCallRequested && result == QAtResult::Connect ) {
        dataCallRequested = false;
        frontEnd()->setState( AtFrontEnd::OnlineData );
        if (cmdsPosn >= cmds.size()) {
            this->result = result;
            cmdsPosn = cmds.size();
            frontEnd()->send( result );
            return;
        } else {
            qLog(ModemEmulator) << "Data call: Connected but more commands pending";
        }
    }
    done( result );
}


// Ring indication for an incoming call.
void AtCommands::ring( const QString& number, const QString& type )
{
    if ( options()->cring ) {
        // The client wants more information about the call type.
        QString typeString = AtCallManager::strCallType( type );
        send( "+CRING: " + typeString );
    } else {
        // Send an ordinary ring indication.
        send( "RING" );
    }
    if ( options()->clip && !number.isEmpty() ) {
        // Send the caller id information.
        send( "+CLIP: " + QAtUtils::encodeNumber( number ) );
    }
}

// Dialing indication for an outgoing call.
void AtCommands::dialingOut( bool asynchronous, bool transparent, bool gprs )
{
    // send the Service Reporting intermediate result code.
    // note that if the implementation of AtCallManager changes,
    // we may have to add in a check for type == "Data" here...
    if ( options()->cr ) {
        QString status = "+CR: ";
        if ( transparent )
            status += "REL ";
        if ( asynchronous )
            status += "A";

        status += "SYNC";

        if ( gprs )
            status = "GPRS";

        send( status );
    }
}

// Connected indication for an outgoing call.
void AtCommands::outgoingConnected( const QString& number )
{
    if ( options()->colp && !number.isEmpty() ) {
        // send the caller id information.
        send( "+COLP: " + QAtUtils::encodeNumber( number ) );
    }
}

// Call waiting indication for an incoming call.
void AtCommands::callWaiting( const QString& number, const QString& type )
{
    if ( options()->ccwa ) {
        QString classId = QString::number
            ( AtCallManager::numCallType( type ) );
        send( "+CCWA: " + QAtUtils::encodeNumber( number ) + "," + classId );
    }
}

// Report "NO CARRIER" for the active call.
void AtCommands::noCarrier()
{
    send( "NO CARRIER" );
}

