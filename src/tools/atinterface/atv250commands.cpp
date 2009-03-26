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

#include "atv250commands.h"
#include "atcommands.h"
#include "atparseutils.h"

#include <QAtUtils>
#include <QCommServiceManager>

AtV250Commands::AtV250Commands( AtCommands * parent ) : QObject( parent )
{
    atc = parent;

    // Add handlers for the common V.250 commands.  Many of these are
    // ignored because they have no meaning for us, or they will be
    // handled by other settings on the phone, not AT commands.
    atc->add( "A",     this, SLOT(ata()) );
    atc->add( "E",     this, SLOT(ate(QString)) );
    atc->add( "H",     this, SLOT(ath(QString)) );
    atc->add( "I",     this, SLOT(ati(QString)) );
    atc->add( "L",     atc,  SLOT(ignore()) );
    atc->add( "M",     atc,  SLOT(ignore()) );
    atc->add( "O",     this, SLOT(ato()) );
    atc->add( "P",     atc,  SLOT(ignore()) );
    atc->add( "Q",     this, SLOT(atq(QString)) );
    atc->add( "S0",    atc,  SLOT(ignore()) );
    atc->add( "S3",    this, SLOT(ats3(QString)) );
    atc->add( "S4",    this, SLOT(ats4(QString)) );
    atc->add( "S5",    this, SLOT(ats5(QString)) );
    atc->add( "S6",    atc,  SLOT(ignore()) );
    atc->add( "S7",    atc,  SLOT(ignore()) );
    atc->add( "S8",    atc,  SLOT(ignore()) );
    atc->add( "S10",   atc,  SLOT(ignore()) );
    atc->add( "T",     atc,  SLOT(ignore()) );
    atc->add( "V",     this, SLOT(atv(QString)) );
    atc->add( "X",     atc,  SLOT(ignore()) );
    atc->add( "Z",     this, SLOT(atz()) );
    atc->add( "&C",    atc,  SLOT(ignore()) );
    atc->add( "&D",    atc,  SLOT(ignore()) );
    atc->add( "&F",    this, SLOT(atampf()) );
    atc->add( "&W",    this, SLOT(atampw()) );
    atc->add( "+GCAP", this, SLOT(atgcap()) );
    atc->add( "+GCI",  atc,  SLOT(ignore()) );
    atc->add( "+GMI",  this, SLOT(atgmi(QString)) );
    atc->add( "+GMM",  this, SLOT(atgmm(QString)) );
    atc->add( "+GMR",  this, SLOT(atgmr(QString)) );
    atc->add( "+GOI",  atc,  SLOT(ignore()) );
    atc->add( "+GSN",  this, SLOT(atgsn(QString)) );
}

AtV250Commands::~AtV250Commands()
{

}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATA Answer Incoming Call}
    \compat

    The \c{ATA} command answers an incoming voice or data call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATA}
         \o \list
               \o \c{CONNECT [<rate>]}: Data call has connected at \c rate.
               \o \c{OK}: Voice call has connected.
               \o \c{NO CARRIER}: Connection could not be established.
               \o \c{ERROR}: Command issues when already connected.
            \endlist
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ata()
{
    QAtResult::ResultCode result = atc->manager()->callManager()->accept();
    if ( result != AtCallManager::Defer )
        atc->done( result );
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATE Command Echo}
    \compat

    The \c{ATE} command can be used to turn command echo on (\c{ATE1})
    or off (\c{ATE0}).  If no parameter is supplied (i.e. \c{ATE}),
    it is the same as \c{ATE0}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATE[<n>]} \o \list
                              \o \c{OK}
                              \o \c{ERROR}
                            \endlist
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ate( const QString& params )
{
    if ( params.isEmpty() || params == "0" ) {
        atc->options()->echo = false;
        atc->done();
    } else if ( params == "1" ) {
        atc->options()->echo = true;
        atc->done();
    } else {
        atc->done( QAtResult::Error );
    }
}


/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATH Hangup Call}
    \compat

    The \c{ATH} command hangs up the current connected, dialing, or
    incoming call.  The \c{AT+CHLD} command is recommended instead for
    voice calls.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATH[<n>]} \o \list
                              \o \c{OK}
                              \o \c{ERROR}
                            \endlist
    \endtable

    If the parameter \c{<n>} is present, it must be zero.

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ath( const QString& params )
{
    // check that the 
    if ( params.length() > 0 && params != "0" ) {
        atc->done( QAtResult::OperationNotAllowed );
        return;
    }

    if ( atc->options()->ignore_ath ) {
        atc->done();
        return;
    }

    QAtResult::ResultCode result = atc->manager()->callManager()->hangup();
    if ( result != AtCallManager::Defer )
        atc->done( result );
}


/*!
    \ingroup ModemEmulator::Identification
    \bold{ATI Identification Information}
    \compat

    The \c{ATI} command returns identification information about the
    modem's manufacturer and revision.  \c{ATI0} can also be used for
    this purpose.  All other \c{ATIn} commands will return an empty string.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATI[<n>]} \o \c{<manufacturer> <revision>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ati( const QString& params )
{
    // Only ATI and ATI0 should return the identification.
    // All other values should return an empty string.
    if ( params.isEmpty() || params == "0" )
        atc->send( QString(QTOPIA_AT_MANUFACTURER) + " " + QTOPIA_AT_REVISION );
    atc->done();
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATO Return to Online Data State}
    \compat

    The \c{ATO} command returns to the online data state if a data call
    is currently in progress.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATO[<n>]} \o \list
                              \o \c{OK}
                              \o \c{ERROR}
                            \endlist
    \endtable

    The parameter \c{<n>} is ignored in this implementation.  If it is
    present, it should be zero according to Recommendation V.250.

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ato()
{
    QAtResult::ResultCode result = atc->manager()->callManager()->online();
    if ( result != AtCallManager::Defer )
        atc->done( result );
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATQ Result Code Suppression}
    \compat

    The \c{ATQ} command can be used to suppress (\c{ATQ1})
    or not suppress (\c{ATQ0}) the reporting of result codes.
    If no parameter is supplied (i.e. \c{ATQ}), it is the
    same as \c{ATQ0}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATQ[<n>]} \o \list
                              \o \c{OK}
                              \o \c{ERROR}
                            \endlist
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atq( const QString& params )
{
    if ( params.isEmpty() || params == "0" ) {
        atc->options()->suppressResults = false;
        atc->done();
    } else if ( params == "1" ) {
        atc->options()->suppressResults = true;
        atc->done();
    } else {
        atc->done( QAtResult::Error );
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATS3 Command Line Termination Character}
    \compat

    The \c{ATS3} command can be used to query or alter the character
    character used to terminate AT command lines.  There is usually no
    reason to set this to something other than the default of 13.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATS3=<n>]} \o \c{OK}
    \row \o \c{ATS3?} \o \c{<n><CR><LF>OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ats3( const QString& params )
{
    int value = soption( params, atc->options()->terminator, 0, 127 );
    if ( value != -1 )
        atc->options()->terminator = (char)value;
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATS4 Response Formatting Character}
    \compat

    The \c{ATS4} command can be used to query or alter the response
    formatting (line feed) character used to format AT response lines.
    There is usually no reason to set this to something other than the
    default of 10.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATS4=<n>]} \o \c{OK}
    \row \o \c{ATS4?} \o \c{<n><CR><LF>OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ats4( const QString& params )
{
    int value = soption( params, atc->options()->response, 0, 127 );
    if ( value != -1 )
        atc->options()->response = (char)value;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATS5 Command Line Editing Character (Backspace)}
    \compat

    The \c{ATS5} command can be used to query or alter the backspace
    character that is used to edit AT command lines.  There is
    usually no reason to set this to something other than the
    default of 8.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATS5=<n>]} \o \c{OK}
    \row \o \c{ATS5?} \o \c{<n><CR><LF>OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::ats5( const QString& params )
{
    int value = soption( params, atc->options()->backspace, 0, 127 );
    if ( value != -1 )
        atc->options()->backspace = (char)value;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATV Verbose Result Codes}
    \compat

    The \c{ATV} command can be used to turn on (\c{ATV1})
    or off (\c{ATV0}) the use of verbose result codes such
    as \c{OK}, \c{ERROR}, \c{NO CARRIER}, etc.  If no parameter
    is supplied (i.e. \c{ATV}), it is the same as \c{ATV0}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATV[<n>]} \o \list
                              \o \c{OK}
                              \o \c{ERROR}
                            \endlist
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atv( const QString& params )
{
    if ( params.isEmpty() || params == "0" ) {
        atc->options()->verboseResults = false;
        atc->done();
    } else if ( params == "1" ) {
        atc->options()->verboseResults = true;
        atc->done();
    } else {
        atc->done( QAtResult::Error );
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATZ Initialize Modem}
    \compat

    The \c{ATZ} command initializes the modem and returns all status
    settings to their default values.  In the current implementation,
    this command is identical to \c{AT&F}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATZ[<n>]} \o \c{OK}
    \endtable

    The parameter \c{<n>} is ignored in this implementation.

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atz()
{
    atc->options()->factoryDefaults();
    atc->options()->load();
    atc->done();
}



/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT&F Reset to Factory Defaults}
    \compat

    The \c{AT&F} command resets all status settings to their factory
    default values.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT&F} \o \c{OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atampf()
{
    atc->options()->factoryDefaults();
    atc->done();
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT&W Write Settings}
    \compat

    The \c{AT&W} command saves all status settings so they can be restored the
    next time the modem is initialized.  In the current implementation,
    this command is ignored.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT&W} \o \c{OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atampw()
{
    atc->options()->save();
    atc->done();
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+GCAP Request Capabilities}
    \compat

    The \c{AT+GCAP} command requests the capabilities of the modem.
    It responds with a list of functionality areas, such as GSM,
    Fax, etc, that the modem supports.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GCAP} \o \c{+GCAP: <functionality-list>}
    \endtable

    The following functionality values may be returned:

    \table
    \header \o Value \o Description
    \row \o \c{+CGSM} \o GSM commands according to 3GPP TS 27.007 and 27.005.
    \row \o \c{+FCLASS} \o Fax commands.
    \row \o \c{+VOIP} \o VoIP calls can be dialed with \c{ATD<uri>;}.
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atgcap()
{
    QCommServiceManager mgr;
    QStringList services = mgr.services();
    QStringList values;
    if ( services.contains( "modem" ) )
        values += "+CGSM";
    if ( services.contains( "voip" ) )
        values += "+VOIP";
    // Detect fax support here when it is done and add "+FCLASS".
    if ( values.size() > 0 )
        atc->send( "+GCAP: " + values.join(", ") );
    atc->done();
}


/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GMI Read Manufacturer Information}
    \compat

    The \c{AT+CGMI} command returns information about the manufacturer
    of the phone.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GMI} \o \c{<manufacturer>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atgmi( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            atc->send( QTOPIA_AT_MANUFACTURER );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GMM Read Model Information}
    \compat

    \bold{AT+GMM Read Model Information}

    The \c{AT+GMM} command is an alias for \c{AT+CGMM}.

    \table
   \header \o Command \o Possible Responses
    \row \o \c{AT+GMM} \o \c{<model>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atgmm( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            atc->send( QTOPIA_AT_MODEL );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GMR Read Revision Information}
    \compat

    The \c{AT+GMR} command is an alias for \c{AT+CGMR}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GMR} \o \c{<revision>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atgmr( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            atc->send( QTOPIA_AT_REVISION );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GSN Read Model Information}
    \compat

    The \c{AT+GSN} command is an alias for \c{AT+CGSN}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GSN} \o \c{<sn>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtV250Commands::atgsn( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            atc->send( QTOPIA_AT_SERIAL_NUMBER );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

// Process an S option.
int AtV250Commands::soption( const QString& params, int prev, int min, int max )
{
    if ( params == "?" ) {
        // Report the current value as a three digit decimal value.
        QString result;
        prev &= 0xFF;
        result = QString( QChar( ( ( prev / 100 ) % 10 ) + '0' ) ) +
                 QString( QChar( ( ( prev /  10 ) % 10 ) + '0' ) ) +
                 QString( QChar( (   prev         % 10 ) + '0' ) );
        atc->send( result );
        atc->done();
    } else if ( params == "=" ) {
        // Set the value to zero unless zero is not an allowable value.
        if ( min != 0 ) {
            atc->done( QAtResult::Error );
            return -1;
        }
        atc->done();
        return 0;
    } else if ( params.startsWith( "=" ) ) {
        // Set the value to the parameter.
        int value = params.mid(1).toInt();
        if ( value < min || value > max ) {
            atc->done( QAtResult::Error );
            return -1;
        }
        atc->done();
        return value;
    } else if ( params.isEmpty() ) {
        // Do nothing.
        atc->done();
    } else {
        // Syntax error in option.
        atc->done( QAtResult::Error );
    }
    return -1;
}

