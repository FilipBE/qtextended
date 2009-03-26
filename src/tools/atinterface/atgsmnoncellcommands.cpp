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

#include "atgsmnoncellcommands.h"
#include "atcommands.h"
#include "atparseutils.h"

#include <QAtUtils>
#include <QPhoneCall>

#ifdef QTOPIA_CELL
#include <QCallSettings>
#endif

#include <QPhoneProfile>
#include <QCallVolume>
#include <QSoundControl>
#include <QTimer>
#include <QVibrateAccessory>
#include <QPhoneProfileManager>
#include <QGsm0710Multiplexer>

AtGsmNonCellCommands::AtGsmNonCellCommands( AtCommands * parent ) : QObject( parent )
{
    atc = parent;

    indicators = 0;
    netReg = new QNetworkRegistration( "modem" );   // No tr
    if ( !netReg->available() ) {
        // May be a VoIP-only phone, so use the default network reg object.
        delete netReg;
        netReg = new QNetworkRegistration();
    }
    requestingAvailableOperators = false;
    settingCurrentOperator = false;
    gqppm = new QPhoneProfileManager();
    activePhoneProfileId = gqppm->activeProfile().id();
    sendRelease = false;
    keyPressTime = 0;
    keyPauseTime = 0;

    serviceNumbers = new QServiceNumbers();
    pendingQuery = (QServiceNumbers::NumberId)(-1);
    pendingSet = (QServiceNumbers::NumberId)(-1);

    multiplexer = 0;
    underlying = 0;

    requestingCallWaiting = false;
    settingCallWaiting = false;
#ifdef QTOPIA_CELL
    callSettings = new QCallSettings();
    connect( callSettings, SIGNAL(callWaiting(QTelephony::CallClass)),
             this, SLOT(callWaitingState(QTelephony::CallClass)) );
    connect( callSettings, SIGNAL(setCallWaitingResult(QTelephony::Result)),
             this, SLOT(setCallWaitingResult(QTelephony::Result)) );
#endif

    callIndicatorsReady = false;
    connect( atc->manager()->callManager(), SIGNAL(callStateInitialized()),
             this, SLOT(callIndicatorsStatusReady()) );

    //-----------------------------------------

    connect( serviceNumbers,
             SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
             this,
             SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );

    connect( serviceNumbers,
             SIGNAL(setServiceNumberResult(QServiceNumbers::NumberId,QTelephony::Result)),
             this,
             SLOT(setServiceNumberResult(QServiceNumbers::NumberId,QTelephony::Result)) );

    connect( netReg, SIGNAL(registrationStateChanged()),
             this, SLOT(registrationStateChanged()) );
    connect( netReg, SIGNAL(availableOperators(QList<QNetworkRegistration::AvailableOperator>)),
             this, SLOT(availableOperators(QList<QNetworkRegistration::AvailableOperator>)) );
    connect( netReg, SIGNAL(setCurrentOperatorResult(QTelephony::Result)),
             this, SLOT(setCurrentOperatorResult(QTelephony::Result)) );

    //-----------------------------------------

    // Handle GSM 27.007 commands.
    atc->add( "+CALM",    this, SLOT(atcalm(QString)) );
    atc->add( "+CBC",     this, SLOT(atcbc(QString)) );
    atc->add( "+CBST",    this, SLOT(atcbst(QString)) );
    atc->add( "+CCLK",    this, SLOT(atcclk(QString)) );
    atc->add( "+CCWA",    this, SLOT(atccwa(QString)) );
    atc->add( "+CEER",    this, SLOT(atceer(QString)) );
    atc->add( "+CGDATA",  this, SLOT(atcgdata(QString)) );
    atc->add( "+CGDCONT", this, SLOT(atcgdcont(QString)) );
    atc->add( "+CGMI",    this, SLOT(atcgmi(QString)) );
    atc->add( "+CGMM",    this, SLOT(atcgmm(QString)) );
    atc->add( "+CGMR",    this, SLOT(atcgmr(QString)) );
    atc->add( "+CGSN",    this, SLOT(atcgsn(QString)) );
    atc->add( "+CHLD",    this, SLOT(atchld(QString)) );
    atc->add( "+CHUP",    this, SLOT(atchup(QString)) );
    atc->add( "+CIND",    this, SLOT(atcind(QString)) );
    atc->add( "+CKPD",    this, SLOT(atckpd(QString)) );
    atc->add( "+CLAN",    this, SLOT(atclan(QString)) );
    atc->add( "+CLCC",    this, SLOT(atclcc(QString)) );
    atc->add( "+CLIP",    this, SLOT(atclip(QString)) );
    atc->add( "+CLVL",    this, SLOT(atclvl(QString)) );
    atc->add( "+CMEC",    this, SLOT(atcmec(QString)) );
    atc->add( "+CMEE",    this, SLOT(atcmee(QString)) );
    atc->add( "+CMER",    this, SLOT(atcmer(QString)) );
    atc->add( "+CMOD",    this, SLOT(atcmod(QString)) );
    atc->add( "+CMUT",    this, SLOT(atcmut(QString)) );
    atc->add( "+CMUX",    this, SLOT(atcmux(QString)) );
    atc->add( "+CNUM",    this, SLOT(atcnum(QString)) );
    atc->add( "+COPS",    this, SLOT(atcops(QString)) );
    atc->add( "+CPAS",    this, SLOT(atcpas(QString)) );
    atc->add( "+CPIN",    this, SLOT(atcpin(QString)) );
    atc->add( "+CR",      this, SLOT(atcr(QString)) );
    atc->add( "+CRC",     this, SLOT(atcrc(QString)) );
    atc->add( "+CREG",    this, SLOT(atcreg(QString)) );
    atc->add( "+CRMC",    this, SLOT(atcrmc(QString)) );
    atc->add( "+CRMP",    this, SLOT(atcrmp(QString)) );
    atc->add( "+CSCS",    this, SLOT(atcscs(QString)) );
    atc->add( "+CSDF",    this, SLOT(atcsdf(QString)) );
    atc->add( "+CSGT",    this, SLOT(atcsgt(QString)) );
    atc->add( "+CSIL",    this, SLOT(atcsil(QString)) );
    atc->add( "+CSNS",    this, SLOT(atcsns(QString)) );
    atc->add( "+CSQ",     this, SLOT(atcsq(QString)) );
    atc->add( "+CSSN",    this, SLOT(atcssn(QString)) );
    atc->add( "+CSTA",    this, SLOT(atcsta(QString)) );
    atc->add( "+CSTF",    this, SLOT(atcstf(QString)) );
    atc->add( "+CSVM",    this, SLOT(atcsvm(QString)) );
    atc->add( "+CTFR",    this, SLOT(atctfr(QString)) );
    atc->add( "+CVHU",    this, SLOT(atcvhu(QString)) );
    atc->add( "+CVIB",    this, SLOT(atcvib(QString)) );
    atc->add( "+VTD",     this, SLOT(atvtd(QString)) );
    atc->add( "+VTS",     this, SLOT(atvts(QString)) );

    // Extension commands for features that should be in the
    // GSM specifications, but aren't.
    atc->add( "*QBC", this, SLOT(atqbc(QString)) );  // Battery charge notifications.
    atc->add( "*QCAM", this, SLOT(atqcam(QString)) );// Call status monitoring.
    atc->add( "*QSQ", this, SLOT(atqsq(QString)) );  // Signal quality notifications.
}

AtGsmNonCellCommands::~AtGsmNonCellCommands()
{
    delete netReg;
    delete gqppm;
    delete serviceNumbers;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CALM Alert Sound Mode}
    \compat

    This command is used to select the general alert sound mode of the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CALM=<mode>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CALM?} \o \list
                              \o \c{+CALM: <mode>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CALM=?} \o \c{+CALM: }(list of supported \c{<mode>}s)
    \endtable

    The list of \c{<mode>}s supported under qtopia are as follows:
    \table
    \row \o 0 \o "Normal" (callAlert and msgAlert set to "Once")
    \row \o 1 \o "Silent" (entire MT set to silent mode)
    \row \o 2 \o "CallSilent" (callAlert set to "Off")
    \row \o 3 \o "MsgSilent" (msgAlert set to "Off")
    \row \o 4 \o "AlertSilent" (callAlert and msgAlert set to "Off")
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcalm( const QString& params )
{
    switch ( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint alert = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || alert > 4 ) {
                    // too many parameters.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                QPhoneProfile tempProfile; 
                bool success = false;
                switch ( alert ) {
                    case 0:
                    {
                        success = gqppm->activateProfile( activePhoneProfileId );
                        tempProfile = gqppm->activeProfile();
                        tempProfile.setMsgAlert( QPhoneProfile::Once );
                        tempProfile.setCallAlert( QPhoneProfile::Once );
                    }
                    break;

                    case 1:
                    {
                        success = gqppm->activateProfile( getSilentPhoneProfileId() );
                    }
                    break;

                    case 2:
                    {
                        tempProfile = gqppm->activeProfile();
                        tempProfile.setMsgAlert( QPhoneProfile::Once );
                        tempProfile.setCallAlert( QPhoneProfile::Off );
                        success = true;
                    }
                    break;

                    case 3:
                    {
                        tempProfile = gqppm->activeProfile();
                        tempProfile.setCallAlert( QPhoneProfile::Once );
                        tempProfile.setMsgAlert( QPhoneProfile::Off );
                        success = true;
                    }
                    break;

                    case 4:
                    {
                        tempProfile = gqppm->activeProfile();
                        tempProfile.setMsgAlert( QPhoneProfile::Off );
                        tempProfile.setCallAlert( QPhoneProfile::Off );
                        success = true;
                    }
                    break;

                    default:
                    {
                        // error state!
                        return;
                    }
                    break;
                }

                gqppm->saveProfile( tempProfile );
                gqppm->sync();
                success ? atc->done() : atc->done( QAtResult::Error );

            } else {
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            if ( gqppm->activeProfile().id() == getSilentPhoneProfileId() ) {
                atc->send( "+CALM: 1" );
            } else {
                bool callAlertIsOff = true;
                bool msgAlertIsOff = true;
                if ( gqppm->activeProfile().callAlert() != QPhoneProfile::Off ) {
                    callAlertIsOff = false;
                }

                if ( gqppm->activeProfile().msgAlert() != QPhoneProfile::Off ) {
                    msgAlertIsOff = false;
                }

                if ( callAlertIsOff && msgAlertIsOff ) {
                    atc->send( "+CALM: 4" );
                } else if ( msgAlertIsOff ) {
                    atc->send( "+CALM: 3" );
                } else if ( callAlertIsOff ) {
                    atc->send( "+CALM: 2" );
                } else {
                    atc->send( "+CALM: 0" );
                }
            }

            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CALM: (0-4)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CBC Battery Charge}
    \compat

    The \c{AT+CBC} command can be used to query the current battery charge.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBC[?]} \o \c{+CBC: <bcs>, <bcl>}
    \row \o \c{AT+CBC=?} \o \c{+CBC: (0-3),(0-100)}
    \endtable

    Execution command returns battery connection status \c{<bcs>} and
    battery charge level \c{<bcl>} of the MT.

    \table
    \row \o \c{<bcs>}
         \o \list
                \o 0 MT is powered by the battery
                \o 1 MT has a battery connected, but is not powered by it
                \o 2 MT does not have a battery connected
                \o 3 Recognized power fault, calls inhibited
            \endlist
    \row \o \c{<bcl>}
         \o \list
                \o 0 battery is exhausted, or MT does not have a battery
                   connected.
                \o 1...100 battery has 1-100 percent of capacity remaining.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcbc( const QString& params )
{
    needIndicators();
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::CommandOnly:
        case AtParseUtils::Get:
        {
            // Report the current battery charge value.
            sendBatteryCharge( indicators->batteryCharge() );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            // Report the supported values.
            atc->send( "+CBC: (0-3),(0-100)" );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CBST Select Bearer Service Type}
    \compat

    The \c{AT+CBST} command is used to select the GSM data bearer.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBST=[<speed>[,<name>[,<ce>]]]} \o \c{OK}
    \row \o \c{AT+CBST?} \o \c{+CBST: <speed>,<name>,<ce>}
    \row \o \c{AT+CBST=?} \o \c{+CBST: (list of supported <speed>s), (list of supported <name>s), (list of supported <ce>s)}
    \endtable

    Set command selects bearer \c{<name>} with data rate \c{<speed>}, and
    the connection element \c{<ce>} to be used when data calls are
    originated.  See 3GPP TS 27.007 for more information on the valid
    values for these parameters.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcbst( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            if ( atc->options()->cbstSpeed != -1 ) {
                atc->send( "+CBST: " +
                      QString::number( atc->options()->cbstSpeed ) + "," +
                      QString::number( atc->options()->cbstName ) + "," +
                      QString::number( atc->options()->cbstCe ) );
            } else {
                // Hasn't been set yet, so return a reasonable set of defaults.
                atc->send( "+CBST: 0,0,1" );
            }
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            // Say that we support everything.  The modem will reject
            // values that aren't supported when the dial happens.
            atc->send( "+CBST: (0-134),(0-7),(0-3)" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint spd = (int)( QAtUtils::parseNumber( params, posn ) );
            uint name = (int)( QAtUtils::parseNumber( params, posn ) );
            uint ce = (int)( QAtUtils::parseNumber( params, posn ) );
            if ( spd > 134 || name > 7 || ce > 3 || posn < (uint)params.length() ) {
                // too many params or params too big.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            // Just save the information away.  It is up to the modem
            // to accept or reject the information when the dial is done.
            atc->options()->cbstSpeed = spd;
            atc->options()->cbstName = name;
            atc->options()->cbstCe = ce;
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CCLK Clock}
    \compat

    The \c{AT+CCLK} command sets the real-time clock of the MT.
    If setting fails in an MT error, \c{+CME ERROR: <err>}
    is returned.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CCLK=<time>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CCLK?} \o \list
                              \o \c{+CCLK: <time>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CCLK=?} \o
    \endtable

    The \c{<time>} parameter is of the form specified
    via \c{AT+CSDF}.

    In this implementation, we "fake" this command.
    When other time-related commands (such as
    \c{AT+CALA}) are performed, some operations are
    done to convert the presented time to the MT
    time, according to our fake time variable.

    It is recommended that the auxillary time format
    is set to 4-digit year format (AT+CSDF=,2)
    before using this command, as the two digit year
    format assumes a base date of 1900 AD.

    Conforms with: 3GPP TS 27.007.

    /sa AT+CSDF
*/
void AtGsmNonCellCommands::atcclk( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                QString timestr = QAtUtils::nextString( params, posn );
                if ( posn < (uint)params.length() || timestr.length() < 20) {
                    // too many params or invalid timestring format.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                // extract (and check) the timezone information
                QString tzInfo = timestr.right(3);
                if ( tzInfo.at(0) != '+' && tzInfo.at(0) != '-' ) {
                    // invalid timezone information.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                // first, we grab the current time.  We do it this way to avoid
                // an off-by-one error that is introduced due to milliseconds
                // not being recorded when using QDateTime::fromString().
                QDateTime currentTime = QDateTime::fromString( QDateTime::currentDateTime().toString("yyyy/MM/dd,hh:mm:ss"), "yyyy/MM/dd,hh:mm:ss" );

                // build the new time
                timestr = timestr.left( timestr.length() - 3 ); // don't parse the timezone information
                QDateTime mtDateTime;
                if ( atc->options()->auxDateFormat == 2 ) {
                    mtDateTime = QDateTime::fromString( timestr, "yyyy/MM/dd,hh:mm:ss" );
                } else {
                    mtDateTime = QDateTime::fromString( timestr, "yy/MM/dd,hh:mm:ss" );
                }

                // check that the date/time given was of the correct format.
                if ( !mtDateTime.isValid() ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                // Now, the fromString method assumes that it is in LocalTime
                // However, it is in an arbitrary time zone.  Subtract the offset.
                // We do it this way, because currentTime.toTime_t() == currentTime.toUTC().toTime_t()
                // which we _don't want_ - we need to get the difference in seconds.
                mtDateTime = mtDateTime.addSecs( currentTime.toTime_t() - 
                        QDateTime::fromString( currentTime.toUTC().toString("ddd MMM dd hh:mm:ss yyyy"), 
                                "ddd MMM dd hh:mm:ss yyyy" ).toTime_t() );

                // Now add the timezone offset given.
                bool ok = true;
                atc->options()->mtTimeZoneSeconds = tzInfo.toInt( &ok ) * 900;
                if ( !ok ) {
                    // error during conversion of timezone.
                    atc->done( QAtResult::Error );
                    break;
                }

                // timezone conversion successful - subtract the tz offset.
                mtDateTime = mtDateTime.addSecs( atc->options()->mtTimeZoneSeconds * -1 );

                // find the offset between this time and the real time
                atc->options()->mtDateTimeOffset = currentTime.toUTC().secsTo( mtDateTime.toUTC() );

                // and return response.
                atc->done();
            } else {
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }
        }
        break;

        case AtParseUtils::Get:
        {
            QString completedTime = "";

            // first, construct the faked date/time
            QDateTime mtDateTime;
            mtDateTime.setTime_t( QDateTime::currentDateTime().toTime_t() + 
                    atc->options()->mtDateTimeOffset + atc->options()->mtTimeZoneSeconds );

            // then, build the timezone offset
            QString tz = "";
            if ( atc->options()->mtTimeZoneSeconds >= 0 ) tz += "+";
            tz += QString::number( atc->options()->mtTimeZoneSeconds / 900 );
            if ( tz.length() < 3 ) tz += "0";

            // then, convert the date/time into the format specified by +CSDF
            // note that this is _not_ the true UTC time for this fake time
            // since we added the mtTimeZoneSeconds already.
            // We use the UTC conversion simply to avoid yet another
            // conversion from local time.
            if ( atc->options()->auxDateFormat == 2 ) {
                completedTime += mtDateTime.toUTC().toString( "yyyy/MM/dd,hh:mm:ss" );
            } else {
                completedTime += mtDateTime.toUTC().toString( "yy/MM/dd,hh:mm:ss" );
            }

            // add the timezone
            completedTime += tz;

            atc->send( "\"" + completedTime + "\""  );
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
    \ingroup ModemEmulator::SupplementaryServices
    \bold{AT+CCWA Call Waiting}
    \compat

    The \c{AT+CCWA} command allows control of the Call Waiting supplementary
    service according to 3GPP TS 22.083.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CCWA=[<n>[,<mode>[,<class>]]]}
         \o when \c{<mode>}=2 and command successful:
            \code
            +CCWA: <status>,<class1>
            +CCWA: <status>,<class2>
            ...
            \endcode
    \row \o \c{AT+CCWA?} \o \c{+CCWA: <n>}
    \row \o \c{AT+CCWA=?} \o \c{+CCWA: (0,1)}
    \endtable

    Activation, deactivation, and status query are supported.
    When querying the status of a network service (\c{<mode>}=2) the
    response line for the "not active" case (\c{<status>}=0) should be
    returned only if the service is not active for any \c{<class>}.

    Parameter \c{<n>} is used to disable/enable the presentation
    of an unsolicited result code \c{+CCWA: <number>,<type>,<class>,[<alpha>][,<CLI validity>[,<subaddr>,<satype>[,<priority>]]]} to the TE when the call
    waiting service is enabled.

    Test command returns the supported unsolicited presentation values.

    \table
    \row \o \c{<n>}
         \o Unsolicited presentation status value.
            \list
                \o 0 Disable
                \o 1 Enable
            \endlist
    \row \o \c{<mode>}
         \o Mode of call waiting operation to perform.
            \list
                \o 0 Disable call waiting
                \o 1 Enable call waiting
                \o 2 Query status
            \endlist
    \row \o \c{<class>}
         \o Sum of integers representing a class of information (default 7).
            \list
                \o 1 voice (telephony)
                \o 2 data (refers to all bearer services)
                \o 4 reserved for future use: fax
                \o 8 short message service
                \o 16 data circuit sync
                \o 32 data circuit async
                \o 64 dedicated packet access
                \o 128 dedicated PAD access
            \endlist
    \row \o \c{<status>}
         \o \list
                \o 0 not active
                \o 1 active
            \endlist
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<CLI validity>}
         \o \list
                \o 0 CLI valid
                \o 1 CLI has been withheld by the originator
                \o 2 CLI is not available due to interworking problems or
                     limitations of originating network.
            \endlist
    \row \o \c{<subaddr>}
         \o String type subaddress of format specified by \c{<satype>}.
    \row \o \c{<satype>}
         \o Type of subaddress octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<priority>}
         \o Digit indicating eMLPP priority level of incoming call
            (refer 3GPP TS 22.067).
    \endtable

    Conforms with: 3GPP TS 27.007.
*/

void AtGsmNonCellCommands::atccwa( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( atc->frontEnd()->options()->ccwa ? "+CCWA: 1" : "+CCWA: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CCWA: (0,1)" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint n = QAtUtils::parseNumber( params, posn );
            if ( n == 0 ) {
                atc->options()->ccwa = false;
            } else if ( n == 1 ) {
                atc->options()->ccwa = true;
            } else {
                // Invalid value for "n".
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }
            if ( posn < (uint)(params.length()) ) {
#ifdef QTOPIA_CELL
                uint mode = QAtUtils::parseNumber( params, posn );
                uint classx = 7;
                if ( posn < (uint)(params.length()) ) {
                    classx = QAtUtils::parseNumber( params, posn );
                }
                switch ( mode ) {
                   case 0:
                    {
                        // Disable call waiting for the specified call classes.
                        if ( callSettings->available() ) {
                            settingCallWaiting = true;
                            callSettings->setCallWaiting
                                ( false, (QTelephony::CallClass)classx );
                        } else {
                            // We don't have call settings support on this
                            // system at all, so report not supported.
                            atc->done( QAtResult::OperationNotSupported );
                        }
                        return;
                    }
                    // Not reached.

                    case 1:
                    {
                        // Enable call waiting for the specified call classes.
                        if ( callSettings->available() ) {
                            settingCallWaiting = true;
                            callSettings->setCallWaiting
                                ( true, (QTelephony::CallClass)classx );
                        } else {
                            // We don't have call settings support on this
                            // system at all, so report not supported.
                            atc->done( QAtResult::OperationNotSupported );
                        }
                        return;
                    }
                    // Not reached.

                    case 2:
                    {
                        // Query the current call waiting classes.
                        if ( callSettings->available() ) {
                            requestingCallWaiting = true;
                            callSettings->requestCallWaiting();
                            return;
                        } else {
                            // We don't have call settings support on this
                            // system at all, so report no classes enabled.
                            atc->send( "+CCWA: 0" );
                        }
                    }
                    break;

                    default:
                    {
                        // Invalid mode parameter.
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                    // Not reached.
                }
#else
                atc->done( QAtResult::OperationNotSupported );
                return;
#endif
            }
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CEER Extended error report}
    \compat

    The \c{AT+CEER} command provides extended error information.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CEER} \o \c{+CEER: <report>}
    \row \o \c{AT+CEER=?} \o
    \endtable

    Execution command causes the TA to return one or more lines of
    information text \c{<report>}, determined by the MT manufacturer,
    which should offer the user of the TA an extended report of the reason
    for the last failed operation.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atceer( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            if ( atc->extendedError == QAtResult::OK ) {
                atc->send( "+CEER: Error 0" );
            } else {
                QAtResult verbose;
                verbose.setResultCode( atc->extendedError );
                atc->send( "+CEER: " + verbose.verboseResult() );
            }
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
    \ingroup ModemEmulator::GPRS
    \bold{AT+CGDATA Enter Data State}
    \compat

    The \c{AT+CGDATA} command is used to enter the GPRS data state.
    In this implementation, it is identical to \c{ATD*99***1#}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGDATA=[<L2P>[,<cid>[,<cid>[,...]]]]}
         \o \c{CONNECT}, \c{ERROR}
    \row \o \c{AT+CGDATA=?} \o \c{+CGDCONT: }
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcgdata( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            if ( atc->options()->contextSet ) {
                atc->invokeCommand( "D", "*99***1#" );
            } else {
                // GPRS context hasn't been set yet.
                atc->done ( QAtResult::OperationNotSupported );
            }
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CGDATA: " );
            atc->done();
        }
        break;

        default:
        {
            atc->done ( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::GPRS
    \bold{AT+CGDCONT Define PDP context}
    \compat

    The \c{AT+CGDCONT} command is used to select the Packet Data Protocols
    and Packet Domain.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGDCONT=[<cid>[,<PDP_type>[,<APN>]]]} \o \list
                                                             \o \c{OK}
                                                             \o \c{ERROR}
                                                           \endlist
    \row \o \c{AT+CGDCONT?} \o \c{+CGDCONT: <cid>,<PDP_type>,<APN>}
    \row \o \c{AT+CGDCONT=?} \o \c{+CGDCONT: (1),"IP", }
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcgdcont( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Get:
            {
                atc->send( "+CGDCONT: 1,\"IP\",\"" + QAtUtils::quote( atc->options()->apn ) + "\"" );
                atc->done();
            }
            break;
        case AtParseUtils::Set:
            {
                uint posn = 1;
                if ( !( posn >= (uint)params.length() || QAtUtils::parseNumber( params, posn ) == 1 ) ) {
                    // Only 1 context id is supported; empty set defaults to cid = 1
                    atc->done( QAtResult::OperationNotSupported );
                    break;
                }
                if ( posn >= (uint)( params.length() ) ) {
                    // Special form that clears context information.
                    atc->options()->contextSet = false;
                    atc->done();
                    break;
                }

                QString pdp = QAtUtils::nextString( params, posn );
                if ( pdp != "IP" ) {
                    // We only support "IP" at present.
                    atc->done( QAtResult::OperationNotSupported );
                    break;
                }

                QString apn = "";
                if ( posn < (uint)params.length() ) {
                    apn = QAtUtils::nextString( params, posn );
                }

                if ( posn < (uint)params.length() ) {
                    // Too many parameters.
                    atc->done( QAtResult::OperationNotSupported );
                    break;
                }

                if ( apn.startsWith( QChar('"') ) )
                    apn = apn.mid( 1 );
                if ( apn.endsWith( QChar('"') ) )
                    apn = apn.left( apn.length()-1 );
                atc->options()->apn = apn;
                atc->options()->contextSet = true;
                atc->done();
            }
            break;
        case AtParseUtils::Support:
            {
                atc->send( "+CGDCONT: (1),\"IP\"," );
                atc->done();
            }
            break;
        default:
            {
                atc->done ( QAtResult::OperationNotAllowed );
            }
            break;
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGMI Read Manufacturer Information}
    \compat

    The \c{AT+CGMI} command is an alias for \c{AT+GMI}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGMI} \o \c{<manufacturer>}
    \row \o \c{AT+CGMI=?} \o
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcgmi( const QString& params )
{
    if ( !atc->invokeCommand( "+GMI", params ) ) {
        // this is "our version" of the command.
        // note - this should not ever get called!
        // however, it is "just in case" the v250
        // handlers haven't registered themselves.
        atc->send( QTOPIA_AT_MANUFACTURER );
        atc->done();
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGMM Read Model Information}
    \compat

    The \c{AT+CGMM} command returns information about the model
    of the phone.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGMM} \o \c{<model>}
    \row \o \c{AT+CGMM=?} \o
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcgmm( const QString& params )
{
    if ( !atc->invokeCommand( "+GMM", params ) ) {
        // this is "our version" of the command.
        // note - this should not ever get called!
        // however, it is "just in case" the v250
        // handlers haven't registered themselves.
        atc->send( QTOPIA_AT_MODEL );
        atc->done();
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGMR Read Revision Information}
    \compat

    The \c{AT+CGMR} command returns information about the revision
    of the phone's software.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGMR} \o \c{<revision>}
    \row \o \c{AT+CGMR=?} \o
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcgmr( const QString& params )
{
    if ( !atc->invokeCommand( "+GMR", params ) ) {
        // this is "our version" of the command.
        // note - this should not ever get called!
        // however, it is "just in case" the v250
        // handlers haven't registered themselves.
        atc->send( QTOPIA_AT_REVISION );
        atc->done();
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGSN Read Product Serial Number}
    \compat

    The \c{AT+CGSN} command returns information about the serial number
    of the phone.  Note: the return value may be a static string for all
    phones of a type, so it should not be relied upon to be unique.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGSN} \o \c{<sn>}
    \row \o \c{AT+CGSN=?} \o
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcgsn( const QString& params )
{
    if ( !atc->invokeCommand( "+GSN", params ) ) {
        // this is "our version" of the command.
        // note - this should not ever get called!
        // however, it is "just in case" the v250
        // handlers haven't registered themselves.
        atc->send( QTOPIA_AT_SERIAL_NUMBER );
        atc->done();
    }
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CLCC List Current Calls}
    \compat

    The \c{AT+CLCC} command lists the calls that are currently
    active within the system.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLCC}
         \o \code
            +CLCC: <id1>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>[,<priority>]]]
            +CLCC: <id2>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>[,<priority>]]]
            ...
            \endcode
    \endtable

    \table
    \row \o <id> \o Integer identifier for the call.
    \row \o <dir> \o Direction of the call: 0 = outgoing MO, 1 = incoming MT.
    \row \o <stat>
         \o State of the call:
            \list
                \o 0 active
                \o 1 held
                \o 2 dialing (MO call)
                \o 3 alerting (MO call)
                \o 4 incoming (MT call)
                \o 5 waiting (MT call)
            \endlist
    \row \o <mode>
         \o Bearer/teleservice:
            \list
                \o 0 voice
                \o 1 data
                \o 2 reserved for future use: fax
                \o 3 voice followed by data, voice mode
                \o 4 alternating voice/data, voice mode
                \o 5 reserved for future use: alternating voice/fax, voice mode
                \o 6 voice followed by data, data mode
                \o 7 alternating voice/data, data mode
                \o 8 reserved for future use: alternating voice/fax, fax mode
                \o 9 unknown - used to indicate video calls
            \endlist
    \row \o <mpty> \o Multiparty indicator: 1 = multi-party, 0 = no multi-party.
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<priority>}
         \o Digit indicating eMLPP priority level of incoming call
            (refer 3GPP TS 22.067).
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atclcc( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            QStringList list = atc->manager()->callManager()->formatCallList();
            foreach ( QString line, list ) {
                atc->send( line );
            }
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
    \ingroup ModemEmulator::SupplementaryServices
    \bold{AT+CLIP Calling line identification presentation}
    \compat

    The \c{AT+CLIP} command allows control of the calling line identification
    presentation supplementary service.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLIP=[<n>]} \o \c{OK}
    \row \o \c{AT+CLIP?} \o \c{+CLIP: <n>,<m>}
    \row \o \c{AT+CLIP=?} \o \c{+CLIP: (0,1)}
    \endtable

    Set command enables or disables the presentation of the CLI at the TE.
    It has no effect on the execution of the supplementary service in the
    network.  When \c{<n>} is 1, the unsolicited response
    \c{+CLIP: <number>,<type>[,<subaddr>,<satype>[,[<alpha>][,<CLI validitity>]]}
    is sent to the TE after every \c{RING} or \c{+CRING} response.

    Read command gives the status of \c{<n>}, plus the provision status
    of the CLIP service (\c{<m>}).

    \table
    \row \o \c{<n>}
         \o Unsolicited presentation status value.
            \list
                \o 0 Disable
                \o 1 Enable
            \endlist
    \row \o \c{<m>}
         \o Provision status of CLIP in the network:
            \list
                \o 0 CLIP not provisioned
                \o 1 CLIP provisioned
                \o 2 unknown (e.g. no network, etc.)
            \endlist
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<subaddr>}
         \o String type subaddress of format specified by \c{<satype>}.
    \row \o \c{<satype>}
         \o Type of subaddress octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<CLI validity>}
         \o \list
                \o 0 CLI valid
                \o 1 CLI has been withheld by the originator
                \o 2 CLI is not available due to interworking problems or
                     limitations of originating network.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atclip( const QString& params )
{
    // The ",1" indicates that CLIP is always provisioned, even if it isn't.
    // The underlying system handles the case of no CLIP better than us.
    flagCommand( "+CLIP: ", atc->options()->clip, params, ",1" );
    return;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CLVL Loudspeaker Volume Level}
    \compat

    This command is used to select the volume of the internal
    loudspeaker of the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLVL=<level>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CLVL?} \o \list
                              \o \c{+CLVL: <level>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CLVL=?} \o \list
                               \o \c{+CLVL: }(list of supported \c{<level>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atclvl( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint vol = QAtUtils::parseNumber( params, posn );
                QCallVolume cv;
                if ( posn < (uint)params.length() || 
                        vol > (uint)cv.maximumSpeakerVolume() || 
                        vol < (uint)cv.minimumSpeakerVolume() ) {
                    // too many params, or param not in range.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                cv.setSpeakerVolume(vol);
                atc->done();
            } else {
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            QCallVolume cv;
            atc->send( "+CLVL: " + QString::number( cv.speakerVolume() ) );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            QCallVolume cv;
            QString supportStr = "+CLVL: (" + 
                    QString::number( cv.minimumSpeakerVolume() ) + "," + 
                    QString::number( cv.maximumSpeakerVolume() ) + ")";
            atc->send( supportStr );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CHLD Call Hold And Multiparty}
    \compat

    The \c{AT+CHLD} command is used to control call hold, release,
    and multiparty states.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CHLD=[<n>]} \o \list
                                   \o \c{OK}
                                   \o \c{+CME ERROR: <err>}
                                 \endlist
    \row \o \c{AT+CHLD=?} \o \c{+CHLD: (0-4,11-19,21-29)}
    \endtable

    The parameter \c{<n>} is an integer value in accordance to
    3GPP TS 22.030:

    \table
    \header \o \c{<n>} \o Description
    \row \o 0 \o Release all held calls or set the busy state
                 for the waiting call.
    \row \o 1 \o Release all active calls.
    \row \o 1x \o Release only call \c{x}.
    \row \o 2 \o Put active calls on hold and activate the waiting or held call.
    \row \o 2x \o Put active calls on hold and activate call \c{x}.
    \row \o 3 \o Add the held calls to the active conversation.
    \row \o 4 \o Add the held calls to the active conversation, and then
                 detach the local subscriber from the conversation.
    \endtable

    GSM modems typically cannot support more than 7 or 8 calls at once.
    This implementation can support up to 99 calls using two-digit
    call identifiers for \c{x} between 10 and 99.  Test command only
    reports 9 call identifiers for backwards compatibility with
    existing client software.

    Conforms with: 3GPP TS 27.007, 22.030.
*/
void AtGsmNonCellCommands::atchld( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Support:
        {
            // Report all of the arguments that we support.  We actually
            // support more than the 9 call identifiers listed here, but
            // it isn't easy to express it in a GSM-compatible fashion.
            atc->send( "+CHLD: (0,1,1x,2,2x,3,4,11,12,13,14,15,16,17,18,19,21,22,23,24,25,26,27,28,29)" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint arg = QAtUtils::parseNumber( params, posn );
            if ( posn < (uint)params.length() ) {
                // too many params.
                arg = 5; // an invalid arg
            }

            uint callID = 0;
            if ( arg >= 11 && arg <= 19 ) {
                // Hang up a specific active call: single-digit call identifier.
                callID = arg - 10;
                arg = 1;
            } else if ( arg >= 110 && arg <= 199 ) {
                // Hang up a specific active call: two-digit call identifier.
                callID = arg - 100;
                arg = 1;
            } else if ( arg >= 21 && arg <= 29 ) {
                // Put current calls on hold and activate a specific call.
                callID = arg - 20;
                arg = 2;
            } else if ( arg >= 210 && arg <= 299 ) {
                // Put calls on hold and activate: two-digit call identifier.
                callID = arg - 200;
                arg = 2;
            }
            QAtResult::ResultCode result = AtCallManager::Defer;
            switch ( arg ) {

                // The following values are from GSM 22.030, section 6.5.5.1.

                case 0:
                {
                    // Release all held calls or set busy for a waiting call.
                    if ( atc->manager()->callManager()->ringing() )
                        result = atc->manager()->callManager()->hangupIncomingCall();
                    else
                        result = atc->manager()->callManager()->hangupHeldCalls();
                }
                break;

                case 1:
                {
                    // Hangup all active calls and accept the held or waiting
                    // call, or just hang up the selected call.
                    if ( !callID ) {
                        if ( atc->manager()->callManager()->callInProgress() )
                            atc->manager()->callManager()->hangup();
                        if ( atc->manager()->callManager()->ringing() )
                            result = atc->manager()->callManager()->accept();
                        else
                            result = atc->manager()->callManager()->activateHeldCalls();
                    } else {
                        result = atc->manager()->callManager()->hangup( callID );
                    }
                }
                break;

                case 2:
                {
                    // Place all calls on hold and activate one or more that
                    // were held or waiting.
                    if ( !callID ) {
                        atc->manager()->callManager()->activateHeldCalls();
                        if ( atc->manager()->callManager()->ringing() )
                            result = atc->manager()->callManager()->accept();
                        else
                            result = atc->manager()->callManager()->activateHeldCalls();
                    } else {
                        result = atc->manager()->callManager()->activate( callID );
                    }
                }
                break;

                case 3:
                {
                    // Adds the held calls to the current conversation (join).
                    result = atc->manager()->callManager()->join();
                }
                break;

                case 4:
                {
                    // Connect held and active calls, then hangup (transfer).
                    result = atc->manager()->callManager()->transfer();
                }
                break;

                default:
                {
                    result = QAtResult::OperationNotAllowed;
                }
                break;
            }
            if ( result != AtCallManager::Defer )
                atc->done( result );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CHUP Hangup Call}
    \compat

    The \c{AT+CHUP} command is an alias for \c{ATH}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CHUP} \o \list
                             \o \c{OK}
                             \o \c{ERROR}
                           \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atchup( const QString& params )
{
    if (! atc->invokeCommand( "H", params ) )
        atc->done( QAtResult::OperationNotSupported );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CIND Indicator Control}
    \compat

    The \c{AT+CIND} command is used to get the current indicator values.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CIND=[<ind>,[<ind>[,...]]]} \o \c{+CME ERROR: 3}
    \row \o \c{AT+CIND?} \o \c{+CIND: <ind>[,<ind>[,...]]}
    \row \o \c{AT+CIND=?} \o \c{+CIND: (<descr>,(list of <ind>'s))[,...]}
    \endtable

    Set command is used to set the indicator values.  In this implementation,
    indicators cannot be set and the set command will always return an error.

    Read command returns the current status of the indicators.

    Test command returns pairs, where the string value \c{<descr>} is a
    description of the indicator, and the compound value is the allowable
    values for the indicator.

    The following indicators are supported:

    \table
    \header \o Name \o Description \o Range
    \row \o \c{battchg} \o Battery charge level \o 0-5
    \row \o \c{signal} \o Signal quality \o 0-5
    \row \o \c{service} \o Service availability \o 0-1
    \row \o \c{message} \o Message received \o 0-1
    \row \o \c{call} \o Call in progress \o 0-1
    \row \o \c{roam} \o Roaming indicator \o 0-1
    \row \o \c{smsfull}
         \o SMS memory state \o
            \list
                \o 0 space is available
                \o 1 memory has just become full
                \o 2 memory is full and a message was just rejected
            \endlist
    \row \o \c{callsetup}
         \o Call setup state \o
            \list
                \o 0 no call setup in progress
                \o 1 incoming call setup in progress
                \o 2 outgoing call setup in progress
                \o 3 outgoing call setup in the "alerting" phase
            \endlist
    \row \o \c{callheld}
         \o Call hold state \o
            \list
                \o 0 no calls are held
                \o 1 there are both active and held calls
                \o 2 there are held calls, but no active calls
            \endlist
    \endtable

    The \c{callsetup} and \c{callheld} indicators are from the
    Bluetooth Hands-Free Profile version 1.5.  The rest are from
    3GPP TS 27.007.

    Conforms with: 3GPP TS 27.007, Bluetooth Hands-Free Profile 1.5.
*/
void AtGsmNonCellCommands::atcind( const QString& params )
{
    if (!callIndicatorsReady) {
        m_pendingAtCindParams.enqueue(params);
        return;
    }

    needIndicators();
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            QString result;
            for ( int ind = 0; ind < indicators->numIndicators(); ++ind ) {
                if ( !result.isEmpty() )
                    result += ",";
                result += QString::number( indicators->value( ind ) );
            }
            atc->send( "+CIND: " + result );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            QString result;
            for ( int ind = 0; ind < indicators->numIndicators(); ++ind ) {
                if ( !result.isEmpty() )
                    result += ",";
                result += "(\"" + QAtUtils::quote( indicators->name( ind ) )
                       + "\",(0-"
                       + QString::number( indicators->maxValue( ind ) )
                       + "))";
            }
            atc->send( "+CIND: " + result );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CKPD Keypad Control}
    \compat

    The \c{AT+CKPD} command is used to send keypad events to the phone.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CKPD=<keys>[,<time>[,<pause>]]} \o \list
                                                       \o \c{OK}
                                                       \o \c{+CME ERROR: 3}
                                                     \endlist
    \row \o \c{AT+CKPD=?} \o \c{+CKPD: (0123456789*#),(0-255),(0-255)}
    \endtable

    Execution command is used to send keypad events to the phone.
    The following \c{<keys>} are defined by 3GPP TS 27.007:

    \table
    \header \o Character \o Description
    \row \o \c{#} \o Hash (number sign)
    \row \o \c{%} \o Percent sign
    \row \o \c{*} \o Star
    \row \o \c{0} ... \c{9} \o Digit keys
    \row \o \c{:C} \o Send \c{C} as a literal character
    \row \o \c{;CCC...;} \o Send \c{CCC...} as literal characters
    \row \o \c{<} \o Left arrow key
    \row \o \c{>} \o Right arrow key
    \row \o \c{@} \o Alpha key for switching input method modes
    \row \o \c{C/c} \o Clear
    \row \o \c{D/d} \o Volume down
    \row \o \c{E/e} \o Connection end (END or hangup key)
    \row \o \c{F/f} \o Function key
    \row \o \c{L/l} \o Phone lock
    \row \o \c{M/m} \o Menu
    \row \o \c{Q/q} \o Quiet/mute
    \row \o \c{S/s} \o Connection start (SEND or call key)
    \row \o \c{U/u} \o Volume up
    \row \o \c{V/v} \o Down arrow key
    \row \o \c{W/w} \o Pause character
    \row \o \c{Y/y} \o Delete last character (same as \c{C/c})
    \row \o \c{[} \o Soft key 1 (same as \c{F/f})
    \row \o \c{]} \o Soft key 2 (same as \c{C/c})
    \row \o \c{^} \o Up arrow key
    \endtable

    The following keys from 3GPP TS 27.007 are not currently supported:

    \table
    \header \o Character \o Description
    \row \o \c{A/a} \o Channel A
    \row \o \c{B/b} \o Channel B
    \row \o \c{P/p} \o Power
    \row \o \c{R/r} \o Recall last number
    \row \o \c{T/t} \o Store/memory
    \row \o \c{X/x} \o Auxillary key
    \endtable

    The following additional non-27.007 keys are supported:

    \table
    \header \o Character \o Description
    \row \o \c{H/h} \o Key on headphones
    \endtable

    \table
    \row \o \c{time}
         \o Time that the key should be held down, in tenths of a second.
            The default is 1.
    \row \o \c{pause}
         \o The time to pause between keys, in tenths of a second.
            The default is 1.
    \endtable

    Conforms with: 3GPP TS 27.007
*/
void AtGsmNonCellCommands::atckpd( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            uint posn = 1;
            QString keys = "";
            if ( posn < (uint)params.length() ) {
                keys = QAtUtils::nextString( params, posn );
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            if ( posn < (uint)(params.length()) )
                keyPressTime = QAtUtils::parseNumber( params, posn );
            else
                keyPressTime = 1;
            if ( posn < (uint)(params.length()) )
                keyPauseTime = QAtUtils::parseNumber( params, posn );
            else
                keyPauseTime = 1;
            QList< QPair<int,int> > codes;
            for ( int index = 0; index < keys.length(); ++index ) {
                int ch = keys[index].unicode();
                int unicode, keycode;
                unicode = 0xFFFF;
                keycode = 0;
                switch ( ch ) {

                    // Keys with Unicode equivalents.
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                    case '*': case '#': case '%':
                    {
                        unicode = keycode = ch;
                    }
                    break;

                    // Primary mappings.
                    case '@':           keycode = Qt::Key_Mode_switch; break;
                    case '<':           keycode = Qt::Key_Left; break;
                    case '>':           keycode = Qt::Key_Right; break;
                    case 'c': case 'C': keycode = Qt::Key_Back; break;
                    case 'd': case 'D': keycode = Qt::Key_VolumeDown; break;
                    case 'e': case 'E': keycode = Qt::Key_Hangup; break;
                    case 'f': case 'F': keycode = Qt::Key_Context1; break;
                    case 'l': case 'L': keycode = Qt::Key_F29; break;
                    case 'm': case 'M': keycode = Qt::Key_Select; break;
                    case 'q': case 'Q': keycode = Qt::Key_VolumeMute; break;
                    case 's': case 'S': keycode = Qt::Key_Call; break;
                    case 'u': case 'U': keycode = Qt::Key_VolumeUp; break;
                    case 'v': case 'V': keycode = Qt::Key_Down; break;
                    case '^':           keycode = Qt::Key_Up; break;

                    // Aliases.
                    case 'y': case 'Y': keycode = Qt::Key_Back; break;
                    case '[':           keycode = Qt::Key_Context1; break;
                    case ']':           keycode = Qt::Key_Back; break;

                    // Key on headphones.
                    case 'h': case 'H': keycode = Qt::Key_F28; break;

                    // Literal key.
                    case ':':
                    {
                        ++index;
                        if ( index < keys.length() ) {
                            unicode = keycode = keys[index].unicode();
                        } else {
                            atc->done( QAtResult::OperationNotAllowed );
                            return;
                        }
                    }
                    break;

                    // Literal key sequence.
                    case ';':
                    {
                        ++index;
                        while ( index < keys.length() &&
                                keys[index] != QChar(';') ) {
                            unicode = keycode = keys[index].unicode();
                            codes.append( QPair<int,int>( unicode, keycode ) );
                            ++index;
                        }
                        if ( index >= keys.length() ) {
                            // No trailing ';' at the end of the sequence.
                            atc->done( QAtResult::OperationNotAllowed );
                            return;
                        }
                    }
                    continue;   // Keys have already been added to the list.
                }
                codes.append( QPair<int,int>( unicode, keycode ) );
            }
            this->keys = codes;
            sendRelease = false;
            sendNextKey();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CKPD: (0123456789*#),(0-255),(0-255)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CLAE Language Event}
    \compat

    This command is used to enable/disable unsolicited result code
    \c{+CLAV: <code>}, which is sent from the MT when the language
    in the MT is changed.

    \table
    \header \o Command \o Possible Values
    \row \o \c{AT+CLAE=<mode>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CLAE?} \o \list 
                              \o \c{+CLAE: <mode>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CLAE=?} \o \list 
                               \o \c{+CLAE: }(list of supported \c{<mode>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    The \c{<mode>} param can be either 0 (disable unsolicited result code),
    or 1 (enable unsolicited result code).  The \c{+CLAV=<code>}
    unsolicited result notification is disabled by default.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atclae( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || mode > 1 ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                atc->options()->clae = ( mode == 1 );
                atc->done();
            } else {
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            atc->options()->clae ? atc->send( "+CLAE: 1" )
                                 : atc->send( "+CLAE: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CLAE: (0,1)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CLAN Set Language}
    \compat

    This command sets the language in the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLAN=<code>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CLAN?} \o \list
                              \o \c{+CLAN: <code>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CLAN=?} \o \list
                               \o \c{+CLAN: }(list of supported \c{<code>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    The \c{<code>} parameter is a two letter abbreviation of the language.
    See ISO 639 for the complete list of codes.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atclan( const QString& params )
{
// obviously incomplete.  Need functions to access the available languages
// (the qtopia/4.3/src/settings/language program does it manually afaik
// by looking at the dict files in the filesystem...
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set: 
        {

        }
        break;

        case AtParseUtils::Get:
        {

        }
        break;

        case AtParseUtils::Support:
        {
            
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMEC Mobile Termination Control Mode}
    \compat

    The \c{AT+CMEC} command is used to select the modes that can
    control the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMEC=[<keyp>,[<disp>[,<ind>]]]} \o \list 
                                                       \o \c{OK}
                                                       \o \c{+CME ERROR: 3}
                                                     \endlist
    \row \o \c{AT+CMEC?} \o \c{+CMEC: 0,0,0}
    \row \o \c{AT+CMEC=?} \o \c{+CMEC: (0),(0),(0)}
    \endtable

    In this implementation, keypad and display control are not supported,
    and indicators can only be read, never written.  It is an error to use
    the set command with non-zero parameters.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcmec( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            // All three parameters must be zero, or the command will fail.
            // If a parameter is not supplied, parseNumber() defaults to zero.
            uint posn = 1;
            if ( QAtUtils::parseNumber( params, posn ) != 0 ||
                 QAtUtils::parseNumber( params, posn ) != 0 ||
                 QAtUtils::parseNumber( params, posn ) != 0 ||
                 posn < (uint)params.length() ) {
                atc->done( QAtResult::OperationNotAllowed );
            } else {
                atc->done();
            }
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CMEC: 0,0,0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMEC: (0),(0),(0)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMEE Report Mobile Termination Error}
    \compat

    The \c{AT+CMEE} command is used to select the error reporting mode
    for the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMEE=[<n>]} \o \list 
                                   \o \c{OK}
                                   \o \c{+CME ERROR: 4}
                                 \endlist
    \row \o \c{AT+CMEE?} \o \c{+CMEE: <n>}
    \row \o \c{AT+CMEE=?} \o \c{+CMEE: (0-2)}
    \endtable

    Set command disables or enables the use of result code
    \c{+CME ERROR: <err>} as an indication of an error relating
    to the functionality of the MT.  When enabled, MT related errors
    cause the \c{+CME ERROR: <err>} final result code instead of the
    regular \c{ERROR} result code.  \c{ERROR} is still returned
    normally when the error is related to syntax, invalid parameters,
    or TA functionality.

    \table
    \row \o \c{<n>}
         \o \list
                \o 0 disable \c{+CME ERROR: <err>} result code and use
                     \c{ERROR} instead.  This is the default value.
                \o 1 enable \c{+CME ERROR: <err>} result code and use
                     numeric \c{<err>} values.
                \o 2 enable \c{+CME ERROR: <err>} result code and use
                     verbose \c{<err>} values.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcmee( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( "+CMEE: " + QString::number( atc->options()->extendedErrors ) );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMEE: (0-2)" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint value = QAtUtils::parseNumber( params, posn );
            if ( posn < (uint)params.length() || value > 2 ) {
                atc->done( QAtResult::OperationNotSupported );
            } else {
                atc->options()->extendedErrors = value;
                atc->done();
            }
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotSupported );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMER Mobile Termination Event Reporting}
    \compat

    The \c{AT+CMER} command enables or disables unsolicited result
    codes related to MT events.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMER=[<mode>[,<keyp>[,<disp>[,<ind>[,<bfr>]]]]]}
         \o \list \o \c{OK} \o \c{+CME ERROR: 3} \endlist
    \row \o \c{AT+CMER?} \o \c{+CMER: 1,0,0,<ind>,0}
    \row \o \c{AT+CMER=?} \o \c{+CMER: (1,3),(0),(0),(0-2),(0)}
    \endtable

    Set command enables or disables unsolicited result codes for
    indicators.  This implementation does not support the other event
    types and \c{<mode>} must be 1 or 3, which are treated as identical.

    \table
    \row \o \c{<ind>}
         \o \list
                \o 0 no indicator event reporting.
                \o 1 indicator event reporting using result code
                     \c{+CIEV: <ind>,<value>}.  Only those indicator
                     changes that are not caused by \c{AT+CIND} set
                     command are reported.
                \o 2 indicator event reporting using result code
                     \c{+CIEV: <ind>,<value>}.  All indicator changes
                     are reported.
            \endlist
    \endtable

    In this implementation, \c{AT+CIND} set command is not supported,
    so \c{<ind>} values 1 and 2 are identical.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcmer( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( "+CMER: " +
                  QString::number( atc->options()->cmer ) + ",0,0," +
                  QString::number( atc->options()->cind ) + ",0" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            // Search for the indicator setting, but ignore everything else.
            uint posn = 1;
            int index = 0;
            int value;
            while ( posn < (uint)(params.length()) ) {
                value = (int)(QAtUtils::parseNumber( params, posn ));
                if ( index == 0 ) {
                    // We treat modes 1 and 3 as identical for now.
                    if ( value != 1 && value != 3 ) {
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                    atc->options()->cmer = value;
                } else if ( index == 1 || index == 2 || index == 4 ) {
                    if ( value != 0 ) {
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                } else if ( index == 3 ) {
                    needIndicators();
                    if ( value < 0 || value > 2 ) {
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                    atc->options()->cind = value;
                    break;
                } else {
                    atc->done( QAtResult::OperationNotAllowed );
                    return;
                }
                ++index;
            }
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMER: (1,3),(0),(0),(0-2),(0)" );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CMOD Call Mode}
    \compat

    The \c{AT+CMOD} command is used to select the call
    mode of further dialing commands (\c{D}) or for
    the next answering command (\c{A}).  The mode can
    be either single or alternating.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMOD=[<mode>]} \o
    \row \o \c{AT+CMOD?} \o \c{+CMOD: <mode>}
    \row \o \c{AT+CMOD=?} \o \c{CMOD: }(list of supported \c{<mode>}s)
    \endtable

    \c{+CMOD} shall be set to zero after a successfully
    completed alternating mode call.  It shall be set
    to zero also after a failed answering.  The power-up,
    factory (\c{&F}) and user-reset (\c{Z}) commands
    shall also set teh value to zero.  This reduces the
    possibility that an alternating mode call could be
    originated or answered accidentally.

    Possible values for \c{<mode>} are:
    \table
      \row \o 0 \o Single mode (default)
      \row \o 1 \o Reserved for future use: Alternating voice/fax (teleservice 61)
      \row \o 2 \o Alternating voice/data (bearer service 61)
      \row \o 3 \o Voice followed by data (bearer service 81)
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcmod( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint mode = 0;
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || mode > 3 ) {
                    // invalid mode or too many parameters.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            }

            atc->options()->cmod = mode;
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CMOD: " + QString::number( atc->options()->cmod ) );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMOD: (0-3)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMUT Mute Control}
    \compat

    This command is used to enable and disable the uplink voice muting
    during a voice call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMUT=<n>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CMUT?} \o \list
                              \o \c{+CMUT: <n>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CMUT=?} \o \c{+CMUT: }(list of supported \c{<n>}s)
    \endtable

    Where \c{<n>} is either \c{0} (mute off) or \c{1} (mute on)

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcmut( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint n = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || n > 1 ) {
                   atc->done( QAtResult::OperationNotAllowed );
                   break;
                }

                QCallVolume cv;
                ( n == 0 ) ? cv.setMicrophoneVolume( cv.maximumMicrophoneVolume() )
                           : cv.setMicrophoneVolume( cv.minimumMicrophoneVolume() );
                atc->done();
            } else {
                // not enough parameters
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            QCallVolume cv;
            if ( cv.microphoneVolume() == cv.minimumMicrophoneVolume() ) {
                atc->send( "+CMUT: 1" );
            } else {
                atc->send( "+CMUT: 0" );
            }

            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMUT: (0,1)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMUX Multiplexing Mode}
    \compat

    The \c{AT+CMUX} enables multiplexing according to 3GPP TS 27.010.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMUX=[<mode>[,<subset>[,<port_speed>[,<frame_size>[,...]]]]]}
         \o \list \o \c{OK} \o \c{+CME ERROR: 4} \endlist
    \row \o \c{AT+CMUX=?} \o \c{+CMUX: (0),(0),(1-6),(1-32768),(1-255),(0-100),(2-255),(1-255),(1-7)}
    \endtable

    Set command enables multiplexing mode according to the supplied
    parameters.  This implementation supports basic mode, UIH frames,
    any port speed, and any frame size.  Set command will report an
    error if some other mode is requested, or the command is used
    on a channel that already has multiplexing enabled.

    \table
    \row \o \c{<mode>}
         \o Multiplexer transparency mechanism:
            \list
                \o 0 Basic mode
                \o 1 Advanced mode; not supported
            \endlist
    \row \o \c{<subset>}
         \o Subset of \c{<mode>} to use:
            \list
                \o 0 UIH frames used only
                \o 1 UI frames used only; not supported
                \o 2 I frames used only; not supported
            \endlist
    \row \o \c{<port_speed>} \o Transmission rate; ignored.
    \row \o \c{<frame_size>} \o Maximum frame size, 1-32768.  Default is 31.
    \endtable

    Test command returns the parameter values that are supported,
    including additional parameters from 3GPP TS 27.007 which are
    ignored by this implementation.

    Conforms with: 3GPP TS 27.007, 27.010.
*/
void AtGsmNonCellCommands::atcmux( const QString& params )
{
    if ( ! atc->frontEnd()->canMux() ) {
        // Cannot use GSM 07.10 multiplexing on a multiplexed channel.
        atc->done( QAtResult::OperationNotSupported );
    } if ( params.startsWith( "=?" ) ) {
        // Indicate that we support Basic, UIH frames, all frame sizes.
        atc->send( "+CMUX: (0),(0),(1-6),(1-32768),(1-255),(0-100),"
              "(2-255),(1-255),(1-7)" );
        atc->done();
    } else if ( params.startsWith( "=" ) ) {
        // Parse the first four parameters and validate.  Ignore the rest.
        uint posn = 1;
        uint mode = QAtUtils::parseNumber( params, posn );
        uint subset = QAtUtils::parseNumber( params, posn );
        QAtUtils::parseNumber( params, posn );  // Skip port speed: not used.
        uint frameSize = QAtUtils::parseNumber( params, posn );
        if ( !frameSize )
            frameSize = 31;     // Use the default frame size if not set.
        if ( mode != 0 || subset != 0 || frameSize > 32768 ) {
            atc->done( QAtResult::OperationNotSupported );
            return;
        }

        // Send the OK response.  But make sure that atc->done() and
        // processNextCommand() are not called because we are about
        // to shut down the underlying device for use by this object.
        atc->frontEnd()->send( QAtResult::OK );
        atc->cmdsPosn = atc->cmds.size();

        // Create a multiplexing wrapper around the underlying device.
        underlying = atc->frontEnd()->device();
        atc->frontEnd()->setDevice( 0 );
        multiplexer = new QGsm0710MultiplexerServer
                ( underlying, frameSize, false, this );
        connect( multiplexer, SIGNAL(opened(int,QSerialIODevice*)),
                 this, SLOT(channelOpened(int,QSerialIODevice*)) );
        connect( multiplexer, SIGNAL(terminated()), this, SLOT(muxTerminated()) );
    } else {
        // Incorrectly formatted AT+CMUX command.
        atc->done( QAtResult::OperationNotSupported );
    }
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CNUM Subscriber Number}
    \compat

    The \c{AT+CNUM} command retrieves the MSISDN's related to the subscriber
    from the SIM.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CNUM}
         \o \code
            +CNUM: [<alpha>],<number>,<type>[,<speed>,<service>[,<itc>]]
            ...
            \endcode
    \row \o \c{AT+CNUM=?} \o
    \endtable

    Execution command returns the MSISDN's related to the subscriber from
    the SIM.  If the subscriber has different MSISDN's for different
    services, each MSISDN is returned on a separate line.

    \table
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<speed>}
         \o GSM bearer speed value.  See 3GPP TS 27.007 for more information.
    \row \o \c{<service>}
         \o \list
                \o Asynchronous modem
                \o Synchronous modem
                \o PAD access
                \o Packet access
                \o Voice
                \o Reserved for future use: Fax
            \endlist
    \row \o \c{<itc>}
         \o \list
                \o 0 3.1 kHz
                \o 1 UDI
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcnum( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            queryNumber( QServiceNumbers::SubscriberNumber );
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
    \ingroup ModemEmulator::Network
    \bold{AT+COPS Operator Selection}
    \compat

    The \c{AT+COPS} command is used to select operators, to request the
    current operator details, and to request a list of the available operators.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+COPS=[<mode>[,<format>[,<oper>[,<AcT>]]]]}
         \o \list \o \c{OK} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT+COPS?} \o \c{+COPS: <mode>[,<format>,<oper>[,<AcT>]]}
    \row \o \c{AT+COPS=?} \o \c{+COPS: [list of supported (<stat>,long <oper>,short <oper>,numeric <oper>[,<AcT>])][,,(list of supported <mode>s),(list of supported <format>s)]}
    \endtable

    Set command forces an attempt to select and register the network operator.
    \c{<mode>} is used to select whether the selection is done automatically
    by the MT or is forced by this command to use operator \c{<oper>}.
    If the selected operator is not available, no other operator shall
    be selected (except for \c{<mode>}=4).  The selected operator name
    format shall apply to further read commands.  \c{<mode>}=2 forces an
    attempt to deregister from the network.

    Read command returns the current mode, the currently selected operator
    and the current access technology (AcT).  If no operator is selected,
    \c{<format>}, \c{<oper>}, and \c{<AcT>} are omitted.

    Test command returns a set of five parameters, each representing an
    operator present in the network.  Each set consists of a \c{<stat>}
    integer indicating the availability of the operator, long and short
    alphanumeric names for the operator, the numeric name for the operator,
    and the access technology.

    \table
    \row \o \c{<mode>}
         \o \list
                \o 0 automatic
                \o 1 manual
                \o 2 deregister from the network
                \o 3 set only \c{<format>}
                \o 4 manual/automatic; if manual selection fails, use automatic
            \endlist
    \row \o \c{<format>}
         \o \list
                \o 0 long format alphanumeric \c{<oper>}
                \o 1 short format alphanumeric \c{<oper>}
                \o 2 numeric \c{<oper>}
            \endlist
    \row \o \c{<oper>} \o String type indicating the name of the operator
                          according to \c{<format>}.
    \row \o \c{<stat>}
         \o \list
                \o 0 unknown
                \o 1 available
                \o 2 current
                \o 3 forbidden
            \endlist
    \row \o \c{<AcT>}
         \o \list
                \o 0 GSM
                \o 1 GSM Compact
                \o 2 UTRAN
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcops( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            // Send the operator details, which we always send in
            // long alphanumeric format, regardless of the format setting.
            // If the technology type is "GSM", we omit the "AcT" parameter
            // as older software conformant with 3GPP TS 07.07 may not know
            // what to do with the extra parameter.
            QString status = "+COPS: ";
            status += QString::number
                ( (int)(netReg->currentOperatorMode()) );
            if ( !netReg->currentOperatorName().isEmpty() ) {
                status += ",0,\"";
                status += QAtUtils::quote( netReg->currentOperatorName() );
                status += "\"";
                QString tech = netReg->currentOperatorTechnology();
                if ( tech == "GSMCompact" )        // No tr
                    status += ",1";
                else if ( tech == "UTRAN" )        // No tr
                    status += ",2";
            }
            atc->send( status );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            // Request the available operators and report them.
            if ( netReg->available() ) {
                requestingAvailableOperators = true;
                netReg->requestAvailableOperators();
            } else {
                // No service, so no network operators to report.
                atc->send( "+COPS: ,,(0-4),(0-2)" );
                atc->done();
            }
        }
        break;

        case AtParseUtils::Set:
        {
            // Set the current operator details.
            uint posn = 1;
            uint mode = QAtUtils::parseNumber( params, posn );
            uint format = QAtUtils::parseNumber( params, posn );
            QString oper = QAtUtils::nextString( params, posn );
            uint tech = QAtUtils::parseNumber( params, posn );
            if ( posn < (uint)params.length() ) {
                // too many params.
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }

            if ( mode == 3 ) {
                // Set format for read command.  In this implementation,
                // we only support "long format alphanumeric" in read.
                // Ignore the format.  Since most users of this command
                // use AT+COPS=3,<format> to set long format alphanumeric
                // anyway, this is not expected to be a problem.
                atc->done();
            } else if ( mode <= 4 ) {
                // Determine the operator id and technology strings.
                QString id, technology;
                if ( !oper.isEmpty() ) {
                    id = QString::number( format ) + oper;
                    if ( tech == 0 )
                        technology = "GSM";             // No tr
                    else if ( tech == 1 )
                        technology = "GSMCompact";      // No tr
                    else if ( tech == 2 )
                        technology = "UTRAN";           // No tr
                    else {
                        // Unknown technology value.
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                }
                if ( netReg->available() ) {
                    settingCurrentOperator = true;
                    netReg->setCurrentOperator
                        ( (QTelephony::OperatorMode)mode, id, technology );
                } else {
                    // No service available, so cannot change operators.
                    atc->done( QAtResult::OperationNotSupported );
                }
            } else {
                // Invalid mode value.
                atc->done( QAtResult::OperationNotAllowed );
            }
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CPAS Phone Activity Status}
    \compat

    The \c{AT+CPAS} returns information about the mode the phone is
    currently operating in.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPAS} \o \c{+CPAS: <pas>}
    \row \o \c{AT+CPAS=?} \o \c{+CPAS: (0-5)}
    \endtable

    Execution command returns the activity status \c{<pas>} of the MT.

    It can be used to interrogate the MT before requesting action
    from the phone.

    Test command returns the values that are supported by the MT.

    \table
    \row \o \c{<pas>}
         \o \list
                \o 0 ready
                \o 1 unavailable
                \o 2 unknown
                \o 3 ringing
                \o 4 call in progress
                \o 5 asleep
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcpas( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::CommandOnly:
        {
            if ( atc->manager()->callManager()->ringing() )
                atc->send( "+CPAS: 3" );
            else if ( atc->manager()->callManager()->callInProgress() )
                atc->send( "+CPAS: 4" );
            else
                atc->send( "+CPAS: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CPAS: (0-5)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CPIN Enter PIN}
    \compat

    The \c{AT+CPIN} command allows for querying and entering the PIN.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPIN=<pin>[,<newpin]} \o \c{+CME ERROR: 3}
    \row \o \c{AT+CPIN?} \o \c{+CPIN: READY}
    \row \o \c{AT+CPIN=?} \o \c{OK}
    \endtable

    For security reasons, this implementation does not allow real access
    to the PIN through this interface.  It is assumed that the user has
    already entered the PIN directly on the phone using some other means.
    If the user hasn't, then requests to use restricted features will be
    denied or delayed.  Read command will always return \c{READY} and
    set command will always fail.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcpin( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            // Report that we are ready.  If we aren't, then the rest of the
            // system will deny or delay operations until it is actually ready.
            atc->send( "+CPIN: READY" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            // Nothing to do, according to GSM 27.007, section 8.3.
            atc->done();
        }
        break;

        case AtParseUtils::Set: // flow on
        default:
        {
            // Attempt to enter or change the pin: disallow it.  See above.
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CR Service Reporting Control}
    \compat

    This command controls whether or not the intermediate result code
    \c{+CR: <serv>} is returned from the TA to the TE.  If enabled,
    the intermediate result code is transmitted at the point during connect
    negotiation at which the TA has determined which speed and quality of
    service will be used, before any error control or data compression reports
    are transmitted, and before the intermediate result code \c{CONNECT} is
    transmitted.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CR=[<mode>]} \o
    \row \o \c{AT+CR?} \o \c{+CR: <mode>}
    \row \o \c{AT+CR=?} \o \c{+CR: }(list of supported \c{<mode>}s)
    \endtable

    The \c{<mode>} parameter can be 0 (disable reporting; default)
    or 1 (enable intermediate result code reporting).

    The \c{<serv>} parameter reports the type of service that the
    data call will use, and can take the following values:

    \table
    \row \o \c{ASYNC} \o Asynchronous Transparent
    \row \o \c{SYNC} \o Synchronous Transparent
    \row \o \c{REL ASYNC} \o Asynchronous Non-transparent
    \row \o \c{REL SYNC} \o Synchronous Non-transparent
    \row \o \c{GPRS} \o GRPS
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcr( const QString& params )
{
    flagCommand( "+CR: ", atc->options()->cr, params, "" );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CRC Cellular Result Codes}
    \compat

    The \c{AT+CRC} command enables or disables the \c{+CRING}
    unsolicited response.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CRC=[<mode>]} \o \list \o \c{OK} \o \c{+CME ERROR: 3} \endlist
    \row \o \c{AT+CRC?} \o \c{+CRC: <mode>}
    \row \o \c{AT+CRC=?} \o \c{+CRC: (0,1)}
    \endtable

    Set command controls whether or not the extended format of incoming
    call indication is used.  When enabled, an incoming call is indicated
    to the TE with unsolicited result code \c{+CRING: <type>} instead of
    the normal \c{RING}, where \c{<type>} is one of the following values:

    \table
    \row \o \c{ASYNC} \o Asynchronous data
    \row \o \c{FAX} \o Reserved for future use: Fascimile
    \row \o \c{VOICE} \o Normal voice
    \endtable

    GPRS network requests are not supported in this implementation.
    They are handled internally within the phone.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcrc( const QString& params )
{
    flagCommand( "+CRC: ", atc->options()->cring, params, "" );
}


// Convert an integer into a 4-four hex string, for lac and ci values.
static QString asHex( int value )
{
    static char const hexchars[] = "0123456789ABCDEF";
    return QString( QChar( hexchars[(value >> 12) & 0x0F] ) ) +
           QString( QChar( hexchars[(value >>  8) & 0x0F] ) ) +
           QString( QChar( hexchars[(value >>  4) & 0x0F] ) ) +
           QString( QChar( hexchars[ value        & 0x0F] ) );
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CREG Network Registration}
    \compat

    The \c{AT+CREG} command control the presentation of unsolicited
    network registration changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CREG=[<n>]} \o \list \o \c{OK} \o \c{+CME ERROR: 3} \endlist
    \row \o \c{AT+CREG?} \o \c{+CREG: <n>,<stat>[,<lac>,<ci>]}
    \row \o \c{AT+CREG=?} \o \c{+CREG: (0-2)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{+CREG: <stat>[,<lac>,<ci>]} when there is a change in the
    network registration status.

    Read command returns the presentation value and the current
    network registration status.
    \table
    \row \o \c{<n>}
         \o \list
                \o 0 disable network registration result code
                \o 1 enable network registration unsolicited result code
                     \c{+CREG: <stat>}
                \o 2 enable network registration unsolicited result code
                     \c{+CREG: <stat>[,<lac>,<ci>]}
            \endlist
    \row \o \c{<stat>}
         \o \list
                \o 0 not registered and not currently searching for a network
                \o 1 registered, home network
                \o 2 not registered and searching for a network
                \o 3 registration denied
                \o 4 unknown
                \o 5 registered, roaming
            \endlist
    \row \o \c{<lac>} \o String type, indicating the two byte location
                         area code in hexadecimal format.
    \row \o \c{<ci>} \o String type, indicating the two byte cell ID
                        in hexadecimal format.
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcreg( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            if ( atc->options()->creg == 2 &&
                 netReg->locationAreaCode() != -1 ) {
                QString hexlac = asHex( netReg->locationAreaCode() );
                QString hexci = asHex( netReg->cellId() );
                atc->send( "+CREG: " + QString::number( atc->options()->creg ) + "," +
                      QString::number
                        ( (int)(netReg->registrationState()) ) + "," +
                      hexlac + "," + hexci );
            } else {
                atc->send( "+CREG: " + QString::number( atc->options()->creg ) + "," +
                      QString::number
                        ( (int)(netReg->registrationState()) ) );
            }
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CREG: (0-2)" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint n = QAtUtils::parseNumber( params, posn );
            if ( posn < (uint)params.length() || n > 2 ) {
                // too many params, or param too big.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            atc->options()->creg = (int)n;
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CRMC Ring Melody Control}
    \compat

    The \c{AT+CRMC} command selects a specific ring
    melody and volume for the selected call type
    and profile.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CRMC=<index>,<volume>[,[<call_type>][,<profile>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CRMC?}
         \o \list
              \o \c{+CRMC: <index>,<volume>[,[<call_type1>][,<profile1>]][<CR><LF>
                    +CRMC: <index>,<volume>[,[<call_type2>][,<profile1>]][<CR><LF>
                    +CRMC: <index>,<volume>[,[<call_type1>][,<profile2>]][<CR><LF>
                    +CRMC: <index>,<volume>[,[<call_type2>][,<profile2>]] [...]]}
              \o \c{+CME ERROR: <err>}
            \endlist
    \row \o \c{AT+CRMC=?}
         \o \list
              \o \c{+CRMC: (list of supported <index>s), (list of supported <volume>s)[,[
                           (list of supported <call_type>s)][,(list of supported <profile>s)]]}
              \o \c{+CME ERROR: <err>}
            \endlist
    \endtable

    Defined Values:
    \table
    \row \o \c{<index>} \o Integer; identifies a certain ring melody
    \row \o \c{<volume>} \o Integer; range of sound levels
    \row \o \c{<call_type\i{x}>} \o Integer; identifies a call type
    \row \o \c{<profile\i{x}>} \o Integer; identifies a profile
    \endtable

    In this implementation, all of the parameters are ignored.
    Access to the ring tone settings is via the MT interface only.

    Conforms with: 3GPP TS 27.007.

    /sa AT+CRMP
*/
void AtGsmNonCellCommands::atcrmc( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint index = 0;
            uint volume = 0;
            uint callType = 0;
            uint profile = 0;

            uint posn = 1;
            index = QAtUtils::parseNumber( params, posn );
            if ( posn < (uint)params.length() ) {
                volume = QAtUtils::parseNumber( params, posn );
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            callType = QAtUtils::parseNumber( params, posn );
            profile = QAtUtils::parseNumber( params, posn );

            if ( posn < (uint)params.length() ||
                 index > 0 || volume > 5 || callType > 0 || profile > 0 ) {
                // too many or invalid params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            // if we get to here, the command was valid.
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            uint vol = gqppm->activeProfile().volume();
            atc->send( "+CRMC: 0," + QString::number( vol ) + ",0,0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CRMC: (0),(0-5),(0),(0)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CRMP Ring Melody Playback}
    \compat

    The \c{AT+CRMP} command causes the MT to play a specific
    ring type.  The default values for the optional
    parameters are those currently selected in the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CRMP=<call_type>[,<volume>[,<type>,<index>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CRMP=?}
         \o \list
              \o \c{+CRMP: (list of supported <call_type>s),(list of supported <volume>s),
                           <type0>,(list of supported <index>s)[<CR><LF>
                    +CRMP: (list of supported <call_type>s),(list of supported <volume>s),
                           <type1>,(list of supported <index>s)]}
              \o \c{+CME ERROR: <err>}
            \endlist
    \endtable

    Defined Values:
    \table
    \header \o Parameter \o Value / Meaning
    \row \o \c{<call_type>} \o Integer; defines the call type for which the melody is set
    \row \o \c{<volume>} \o Integer; defines the volume at which to play the melody
    \row \o \c{<type>} \o \list \o 0 Manufacturer Defined \o 1 User Defined \endlist
    \row \o \c{<index>} \o Integer; defines which of the melodies for the given <call_type> to play
    \endtable

    In this implementation, all parameters except for \c{<volume>}
    must be 0.  The currently active profile's ring melody will
    be played at the given volume, or at the current volume if
    the \c{<volume>} is not supplied.

    Conforms with: 3GPP TS 27.007.

    /sa AT+CRMC
*/
void AtGsmNonCellCommands::atcrmp( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint callType = 0;
            uint volume = gqppm->activeProfile().volume();
            uint type = 0;
            uint index = 0;

            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                callType = QAtUtils::parseNumber( params, posn );
            } else {
                // not enough params (call_type is mandatory)
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            if ( posn < (uint)params.length() ) {
                volume = QAtUtils::parseNumber( params, posn );
            }

            if ( posn < (uint)params.length() ) {
                type = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                     index = QAtUtils::parseNumber( params, posn );
                } else {
                    // not enough params (index is mandatory if type given)
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            }

            if ( posn < (uint)params.length() || 
                 callType > 0 || volume > 5 || type > 0 || index > 0 ) {
                // too many or invalid params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            // if we get to here, we have a valid command.
            QSoundControl *qsc = new QSoundControl( new QSound( gqppm->activeProfile().callTone().fileName() ) );
            qsc->setVolume( volume );
            qsc->setPriority( QSoundControl::RingTone );
            if ( qsc->sound()->isAvailable() ) qsc->sound()->play();
            delete qsc->sound();
            delete qsc;

            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CRMP: (0),(0-5),(0),(0)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CSCS Select TE Character Set}
    \compat

    The \c{AT+CSCS} command selects the character set to use in commands
    that take or return string arguments.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSCS=<chset>} \o \list \o \c{OK} \o \c{+CME ERROR: 4} \endlist
    \row \o \c{AT+CSCS?} \o \c{+CSCS: <chset>}
    \row \o \c{AT+CSCS=?} \o \c{+CSCS:} (list of supported \c{<chset>}s)
    \endtable

    Set command informs the TA which character set \c{<chset>} is used
    by the TE.  TA is then able to convert character strings correctly
    between TE and MT character sets.  \c{+CME ERROR: 4} will be returned
    if the character set is not supported.

    Read command shows the current setting, and test command displays
    the available character sets.

    The following character sets are supported by this implementation:

    \table
    \header \o Name \o Description
    \row \o \c{GSM} \o GSM 7-bit default alphabet from 3GPP TS 23.038.
                       This is the default value.
    \row \o \c{HEX} \o Hexadecimal encoding of GSM 7-bit default alphabet.
    \row \o \c{UCS2} \o 16-bit universal multiple-octet coded character set.
                        UCS2 character strings are converted to hexadecimal.
    \row \o \c{8859-1} \o ISO-8859 Latin 1 character set.
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtGsmNonCellCommands::atcscs( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            // Note: don't use AtGsmNonCellCommands::quote for this.  Charset changes
            // must always be in plain ASCII.
            atc->send( "+CSCS: \"" + QAtUtils::quote( atc->options()->charset ) + "\"" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSCS: (\"GSM\",\"HEX\",\"UCS2\",\"8859-1\")" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            // Note: don't use AtGsmNonCellCommands::nextString for this.
            // Charset changes must always be in plain ASCII.
            uint posn = 1;
            QString value = QAtUtils::nextString( params, posn ).toUpper();
            if ( value == "GSM" || value == "HEX" ||
                  value == "UCS2" || value == "8859-1" ) {
                atc->options()->setCharset( value );
                atc->done();
            } else {
                atc->done( QAtResult::OperationNotSupported );
            }
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CSDF Settings Date Format}
    \compat

    The \c{AT+CSDF} command sets the date format via MMI of the date information
    presented to the user.  It also sets the date format of the TE-TA interface,
    and therefore affects the \c{<time>} of +CCLK and +CALA.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSDF=[[<mode>][,<auxmode>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CSDF?} \o \list
                              \o \c{+CSDF:<mode>[,<auxmode>]}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CSDF=?} \o \c{+CSDF: }(list of supported \c{<mode>}s)
                             \c{[,}(list of supported \c{<auxmode>}s)\c{]}
    \endtable

    The \c{<mode>} parameter can take the following values:

    \table
    \header \o Value \o Date Format
    \row \o 1 \o DD-MMM-YYYY
    \row \o 2 \o DD-MM-YY
    \row \o 3 \o MM/DD/YY
    \row \o 4 \o DD/MM/YY
    \row \o 5 \o DD.MM.YY
    \row \o 6 \o YYMMDD
    \row \o 7 \o YY-MM-DD
    \row \o 8 \o DDD/MMM/YY
    \row \o 9 \o DDD/MMM/YYYY
    \endtable

    The \c{<auxmode>} parameter can take the following values:

    \list
      \o 1 yy/MM/dd (default)
      \o 2 yyyy/MM/dd
    \endlist

    Conforms with: 3GPP TS 27.007.

    /sa AT+CCLK, AT+CALA
*/
void AtGsmNonCellCommands::atcsdf( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                if ( params.at(posn) == ',' ) {
                    // the first param given is the <auxmode>
                    uint temp = QAtUtils::parseNumber( params, posn );
                    if ( posn < (uint)params.length() || temp > 2 || temp < 1 ) {
                        // too many or parameters, or parameters invalid.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }

                    atc->frontEnd()->options()->auxDateFormat = temp;
                    atc->frontEnd()->options()->dateFormat = 1;
                } else {
                    // the first param given is the <mode>
                    uint temp = QAtUtils::parseNumber( params, posn );
                    if ( posn < (uint)params.length() || temp > 9) {
                        uint temp2 = QAtUtils::parseNumber( params, posn );
                        if ( posn < (uint)params.length() || temp > 9 || temp2 > 2 || temp < 1 || temp2 < 1) {
                            // too many or parameters, or parameters invalid.
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }

                        atc->frontEnd()->options()->auxDateFormat = temp2;
                    } else {
                        atc->frontEnd()->options()->auxDateFormat = 1;
                    }

                    atc->frontEnd()->options()->dateFormat = temp;
                }

            } else {
                // set both to default values.
                atc->frontEnd()->options()->dateFormat = 1;
                atc->frontEnd()->options()->auxDateFormat = 1;
            }

            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CSDF: " + QString::number( atc->frontEnd()->options()->dateFormat ) +
                    "," + QString::number( atc->frontEnd()->options()->auxDateFormat ) );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSDF: (1-9),(1-2)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CSGT Set Greeting Text}
    \compat

    The \c{AT+CSGT} command sets and activates the greeting
    text in the MT.  The greeting text is shown in the MT
    display when the MT is turned on.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSGT=<mode>[,<text>]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CSGT?} \o \c{+CSGT: <text>, <mode>}
    \row \o \c{AT+CSGT=?} \o \list
                               \o \c{+CSGT: (}list of supported \c{<mode>}s\c{)},<ltext>}
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    \c{<ltext>}, the maximum length of the greeting text
    supported is 20 characters.

    The \c{<mode>} parameter may be 0 (off; default)
    or 1 (on).  In this implementation, the value
    of mode is ignored; the greeting text is always off.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcsgt( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    QString gt = QAtUtils::nextString( params, posn );
                    if ( posn < (uint)params.length() ||
                         gt.length() > 20 || mode > 1 ) {
                        // too many params, or invalid gt or mode.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }

                    atc->options()->greetingText = gt;
                }

                if ( mode <= 1 ) {
                    atc->options()->csgt = ( mode == 1 );
                } else {
                    // invalid mode
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            // if we reach here, we have successfully set the gt/mode.
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            // in this implementation, the greeting text is always off.
            atc->send( "+CSGT: \"" + atc->options()->greetingText + "\",0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSGT: (0,1),(20)" );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CSIL Silence Command}
    \compat

    The \c{AT+CSIL} command enables or disables silent mode.
    When the phone is in silent mode, all sounds from MT are
    suppressed except voice.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSIL=[<mode>]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CSIL?} \o \list
                              \o \c{+CSIL: <mode>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CSIL=?} \o \list 
                               \o \c{+CSIL: }(list of supported \c{<mode?}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    \c{<mode>} can be either 0 (silent mode off) or 1 (silent mode on).

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcsil( const QString& params )
{
// maybe use QPhoneProfile->setAudioProfile() instead...
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            bool success = false;
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint silent = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || silent > 1 ) {
                    // too many params
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                // valid command - either set silent or old active.
                if ( silent ) {
                    // check to see if we need to change the profile or not.
                    if ( gqppm->activeProfile().id() == getSilentPhoneProfileId() ) {
                        // the phone is already in silent mode.
                        success = true;
                    } else {
                        // save the current active profile id, set profile -> silent
                        activePhoneProfileId = ( gqppm->activeProfile() ).id();
                        success = gqppm->activateProfile( getSilentPhoneProfileId() );
                    }
                } else {
                    // check to see if we need to change the profile or not.
                    if ( ( gqppm->activeProfile() ).id() != getSilentPhoneProfileId() ) {
                        // this phone is not on silent.
                        success = true;
                    } else {
                        // the phone was on silent.  Set it to the previous active profile.
                        if ( activePhoneProfileId == getSilentPhoneProfileId() ) {
                            // if the active phone profile is the silent profile, we
                            // simply set it to the default general profile.
                            success = gqppm->activateProfile(1);
                        } else {
                            // set the phone to the previously saved active profile id.
                            success = gqppm->activateProfile( activePhoneProfileId );
                        }
                   }
                } 
            } else {
                // default is: set silent to off.
                // check to see if we need to change the profile or not.
                if ( ( gqppm->activeProfile() ).id() != getSilentPhoneProfileId() ) {
                    // this phone is not on silent.
                    success = true;
                } else {
                    // the phone was on silent.  Set it to the previous active profile.
                    if ( activePhoneProfileId == getSilentPhoneProfileId() ) {
                        // if the active phone profile is the silent profile, we
                        // simply set it to the default general profile.
                        success = gqppm->activateProfile(1);
                    } else {
                        // set the phone to the previously saved active profile id.
                        success = gqppm->activateProfile( activePhoneProfileId );
                    }
                }
            }

            gqppm->sync();
            success ? atc->done() : atc->done( QAtResult::Error );
         }
         break;

         case AtParseUtils::Get: 
         {
             ( ( gqppm->activeProfile() ).id() == getSilentPhoneProfileId() ) ? atc->send( "+CSIL: 1" ) : atc->send( "+CSIL: 0" ); 
             atc->done();
         }
         break;

         case AtParseUtils::Support:
         {
             atc->send( "+CSIL: (0,1)" );
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
    Used exclusively by AT+CSIL/AT+CALM to find the Profile ID of the silent profile.
*/
int AtGsmNonCellCommands::getSilentPhoneProfileId()
{
    int silentId = -1;
    QList<QPhoneProfile> profiles = gqppm->profiles();
    QList<QPhoneProfile>::ConstIterator it;
    for ( it = profiles.begin(); it != profiles.end(); ++it ) {
        if ( (*it).name() == "Silent" ) {
            silentId = (*it).id();
            break;
        }
    }

    return silentId;
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CSNS Single Number Scheme}
    \compat

    The \c{AT+CSNS} command queries or selects the
    bearer or teleservice to be used when a mobile
    terminated, single numbering scheme call is
    established.  If \c{<mode>} equals 4
    ("data service"), the parameter values set
    with \c{AT+CBST} command shall be used.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSNS=[<mode>]} \o
    \row \o \c{AT+CSNS?} \o \c{+CSNS: <mode>}
    \row \o \c{AT+CSNS=?} \o \c{+CSNS: }(list of supported \c{<mode>}s)
    \endtable

    The following values are defined for \c{<mode>}:
    \table
      \row \o 0 \o Voice (default)
      \row \o 1 \o Reserved for future use: Alternating voice/fax - voice first
      \row \o 2 \o Reserved for future use: Facsimile
      \row \o 3 \o Alternating voice/data - voice first
      \row \o 4 \o Data
      \row \o 5 \o Reserved for future use: Alternating voice/fax - fax first
      \row \o 6 \o Alternating voice/data - data first
      \row \o 7 \o Voice followed by Data
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcsns( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint mode = 0;
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || mode > 7 ) {
                    // invalid mode or too many parameters.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            }

            atc->options()->csns = mode;
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CSNS: " + QString::number( atc->options()->csns ) );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSNS: (0-7)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CSQ Signal Quality}
    \compat

    The \c{AT+CSQ} command can be used to query the current signal quality.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSQ[?]} \o \c{+CBC: <rssi>, <ber>}
    \row \o \c{AT+CSQ=?} \o \c{+CBC: (0-31),(0-7)}
    \endtable

    Execution command returns received signal strength indication
    \c{<rssi>} and channel bit error rate \c{<ber>} from the MT.

    \table
    \row \o \c{<rssi>}
         \o \list
                \o 0 -113 dBm or less
                \o 1 -111 dBm
                \o 2...30 -109 to -53 sBm
                \o 31 -51 dBm or greater
                \o 99 not known or not detectable.
            \endlist
    \row \o \c{<ber>}
         \o \list
                \o 0...7 RXQUAL values as defined in 3GPP TS 45.008.
                \o 99 not known or not detectable.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcsq( const QString& params )
{
    needIndicators();
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::CommandOnly:
        case AtParseUtils::Get:
        {
            // Report the current signal quality value.
            sendSignalQuality( indicators->signalQuality() );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            // Report the supported values.
            atc->send( "+CSQ: (0-31),(0-7)" );
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
    \ingroup ModemEmulator::SupplementaryServices
    \bold{AT+CSSN Supplementary Service Notifications}
    \compat

    The \c{AT+CSSN} command refers to supplementary services related
    network initiated notifications.  It controls the presentation
    of notification result codes from TA to TE.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSSN=[<n>[,<m>]]} \o
    \row \o \c{AT+CSSN?} \o \c{+CSSN: <n>,<m>}
    \row \o \c{AT+CSSN=?} \o \c{+CSSN: }(list of supported \c{<n>}s)\c{,}(list of supported \c{<m>}s)
    \endtable

    \c{<n>} parameter enables presentation of the +CSSI result code to the TE.
    \c{<m>} parameter enables presentation of the +CSSU result code to the TE.
    For each, values of 0 (disable; default) and 1 (enable) are supported.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcssn( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint n = 0;
            uint m = 0;
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                n = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    m = QAtUtils::parseNumber( params, posn );
                    if ( posn < (uint)params.length() ) {
                        // too many parameters.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }
                }
            }

            // check for invalid params.
            if ( n > 1 || m > 1 ) {
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            atc->options()->cssi = ( n == 1 );
            atc->options()->cssu = ( m == 1 );
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            QString status = "+CSSN: ";
            atc->options()->cssi ? status += "1," : status += "0,";
            atc->options()->cssu ? status += "1" : status += "0";
            atc->send( status );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSSN: (0,1),(0,1)" );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CSTA Select type of address}
    \compat

    The \c{AT+CSTA} command selects the type of number for dialing
    with the \c{ATD} command.  This implementation only supports 129.
    If a dial string starts with \c{+}, then 145 will be implicitly
    selected.  No other dial number types are supported.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSTA=[<type>]} \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CSTA?} \o \c{+CSTA: 129}
    \row \o \c{AT+CSTA=?} \o \c{+CSTA: (129)}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcsta( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            // The only value supported by this implementation is 129.
            if ( params != "=129" )
                atc->done( QAtResult::OperationNotSupported );
            else
                atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CSTA: 129" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSTA: (129)" );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+CSTF Settings Time Format}
    \compat

    The \c{AT+CSTF} command sets the time format of the time information
    presented to the user.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSTF=[<mode>]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CSTF?} \o \list
                              \o \c{+CSTF: <mode>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CSTF=?} \o \list 
                               \o \c{+CSTF: }(list of supported \c{<mode>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    This command has no effect in our current implementation.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcstf( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint temp = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ||  temp > 7 ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                atc->frontEnd()->options()->timeFormat = temp;
            } else {
                // set to default time format
                atc->frontEnd()->options()->timeFormat = 1;
            }

            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CSTF: " + QString::number( atc->frontEnd()->options()->timeFormat ) );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSTF: (1-7)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CSVM Set Voice Mail Number}
    \compat

    The \c{AT+CSVM} command is used to set the number to the
    voice mail server.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSVM=<mode>[,<number>[,<type>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CSVM?} \o \list
                              \o \c{+CSVM: <mode>,<number>,<type>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CSVM=?} \o \list
                               \o \c{+CSVM: }(list of supported \c{<mode>}s),
                                        (list of supported \c{<type>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    The \c{<mode>} parameter can take the value of 
    0 (disable voice mail; default) or 1 (enable voice mail).

    The \c{<number>} parameter is a string, <0..9,+>

    The \c{<type>} parameter is an integer type, default 129
    \table
        \row \o 129 \o ISDN/Telephony numbering, national/international unknown
        \row \o 145 \o ISDN/Telephony numbering, international number
        \row \o 161 \o ISDN/Telephony numbering, national number
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcsvm( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                QString number;
                uint type;
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() && mode <= 1 ) {
                    number = QAtUtils::nextString( params, posn );
                    if ( posn < (uint)params.length() ) {
                        type = QAtUtils::parseNumber( params, posn );
                        if ( posn < (uint)params.length() ) {
                            // too many parameters.
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }
                    }

                    atc->options()->csvm = ( mode == 1 );
                    pendingSet = QServiceNumbers::VoiceMail;
                    serviceNumbers->setServiceNumber( QServiceNumbers::VoiceMail, number );
                } else if ( mode == 0 ) {
                    // disable the voice mail number.
                    atc->options()->csvm = false;
                    atc->done();
                } else {
                    // not enough parameters, or mode too big.
                    atc->done( QAtResult::OperationNotAllowed );
                }
            } else {
                // not enough parameters.
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            pendingQuery = QServiceNumbers::VoiceMail;
            serviceNumbers->requestServiceNumber( QServiceNumbers::VoiceMail );
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSVM: (0,1),(129,145)" );
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
    \ingroup ModemEmulator::Network
    \bold{AT+CTFR Call Deflection}
    \compat

    The \c{AT+CTFR} command causes an incoming alerting
    call to be forwarded to a specific number.

    Call Deflection is only available to Teleservice 11.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CTFR=<number>[,<type>[,<subaddr>[,<satype>]]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CTFR=?} \o
    \endtable

    The \c{<type>},\c{<subaddr>} and \c{<satype>} parameters
    are ignored in this implementation.  The \c{<number>}
    parameter is a string type phone number.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atctfr( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn >= (uint)params.length() ) {
                // not enough parameters.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            QString number = QAtUtils::nextString( params, posn );
            (void)QAtUtils::parseNumber( params, posn );
            (void)QAtUtils::nextString( params, posn );
            (void)QAtUtils::parseNumber( params, posn );

            if ( posn < (uint)params.length() ) {
                // too many params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            // now check to see that the current state is "incoming"
            if ( !atc->manager()->callManager()->ringing() ) {
                // no incoming call to deflect.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            // and transfer the call.
            atc->done( atc->manager()->callManager()->transferIncoming( number ) );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CVHU Voice Hangup Control}
    \compat

    The \c{AT+CVHU} command controls the behaviour
    of the MT upon receiving ATH or "drop DTR".

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CVHU=[<mode>]} \o
    \row \o \c{AT+CVHU?} \o \c{+CVHU: <mode>}
    \row \o \c{AT+CVHU=?} \o \c{+CVHU: }(list of supported \c{<mode>}s)
    \endtable

    Values defined for \c{<mode>} are as follows:
    \table
    \row \o 0 (default) \o "Drop DTR" ignored but OK response given.  ATH disconnects.
    \row \o 1 \o "Drop DTR" and ATH ignored but OK response given.
    \row \o 2 \o "Drop DTR" behaviour according to &D setting.  ATH disconnects.
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcvhu( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    return;
                }

                switch ( mode ) {
                    case 0:
                    {
                        atc->options()->ignore_drop_dtr = true;
                        atc->options()->ignore_ath = false;
                    }
                    break;

                    case 1:
                    {
                        atc->options()->ignore_drop_dtr = true;
                        atc->options()->ignore_ath = true;
                    }
                    break;

                    case 2:
                    {
                        atc->options()->ignore_drop_dtr = false;
                        atc->options()->ignore_ath = false;
                    }
                    break;

                    default:
                    {
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                    break;
                }
            } else {
                atc->options()->ignore_drop_dtr = true;
                atc->options()->ignore_ath = false;
            }

            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            if ( atc->options()->ignore_ath ) {
                atc->send( "+CVHU: 1" );
            } else if ( !atc->options()->ignore_drop_dtr ) {
                atc->send( "+CVHU: 2" );
            } else {
                atc->send( "+CVHU: 0" );
            }

            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CVHU: (0-2)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CVIB Vibrator Mode}
    \compat

    This command is used to enable and disable the vibrator alert
    feature of the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CVIB=<mode>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CVIB?} \o \list
                              \o \c{+CVIB: <mode>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CVIB=?} \o \list 
                               \o \c{+CVIB: }(list of supported \c{<mode>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    The following values for \c{<mode>} apply:
    \table
    \row \o 0 \o Disable
    \row \o 1 \o Enable
    \row \o ...15 \o Reserved
    \row \o 16... \o Manufacturer Specific
    \endtable

    This implementation supports only values 0 and 1 for \c{<mode>}.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atcvib( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || mode > 1 ) {
                    atc->done( QAtResult::OperationNotAllowed ); 
                    break;
                }

                QPhoneProfile tempProfile = gqppm->activeProfile();
                tempProfile.setVibrate( mode == 1 );
                gqppm->saveProfile(tempProfile);
                gqppm->sync();

                atc->done();
            } else {
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            gqppm->activeProfile().vibrate() ? atc->send("+CVIB: 1") : atc->send("+CVIB: 0");
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CVIB: (0,1)" );
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
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT*QBC Enable Unsolicited Battery Charge Reporting}
    \compat

    The \c{AT*QBC} command can be used to enable or disable the
    unsolicited reporting of battery charge changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT*QBC=<n>} \o \list \o \c{OK} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT*QBC?} \o \c{*QBC: <n>}
    \row \o \c{AT*QBC=?} \o \c{*QBC: (0,1)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{*QBC: <bcs>,<bcl>} when \c{<n>}=1 and there is a change in the
    battery charge information.

    Read command returns the current state of the result code presentation
    value \c{<n>}.  The default value is 0.

    \table
    \row \o \c{<bcs>}
         \o \list
                \o 0 MT is powered by the battery
                \o 1 MT has a battery connected, but is not powered by it
                \o 2 MT does not have a battery connected
                \o 3 Recognized power fault, calls inhibited
            \endlist
    \row \o \c{<bcl>}
         \o \list
                \o 0 battery is exhausted, or MT does not have a battery
                   connected.
                \o 1...100 battery has 1-100 percent of capacity remaining.
            \endlist
    \endtable
*/
void AtGsmNonCellCommands::atqbc( const QString& params )
{
#ifndef QTOPIA_AT_STRICT
    needIndicators();
    flagCommand( "*QBC: ", atc->options()->qbc, params );
#else
    Q_UNUSED( params );
    atc->done( QAtResult::OperationNotAllowed );
#endif
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT*QCAM Call Status Monitoring}
    \compat

    The \c{AT*QCAM} command can be used to enable or disable the
    unsolicited reporting of call status changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT*QCAM=<n>} \o \list \o \c{OK} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT*QCAM?} \o \c{*QCAM: <n>}
    \row \o \c{AT*QCAM=?} \o \c{*QCAM: (0,1)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{*QCAV: <id>,<state>,<calltype>[,<number>,<type>]} when \c{<n>}=1
    and there is a change in the call status information.

    Read command returns the current state of the result code presentation
    value \c{<n>}.  The default value is 0.

    \table
    \row \o \c{<id>} \o Identifier for the call.
    \row \o \c{<state>}
         \o State of the call:
            \list
                \o 0 idle
                \o 1 calling
                \o 2 connecting
                \o 3 active
                \o 4 hold
                \o 5 waiting
                \o 6 alerting
                \o 7 busy
            \endlist
    \row \o \c{<calltype>}
         \o Type of call:
            \list
                \o 1 voice
                \o 2 data
                \o 4 reserved for future use: fax
                \o 32 video
            \endlist
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \endtable
*/
void AtGsmNonCellCommands::atqcam( const QString& params )
{
#ifndef QTOPIA_AT_STRICT
    flagCommand( "*QCAM: ", atc->options()->qcam, params );
#else
    Q_UNUSED( params );
    atc->done( QAtResult::OperationNotAllowed );
#endif
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT*QSQ Enable Unsolicited Signal Quality Reporting}
    \compat

    The \c{AT*QSQ} command can be used to enable or disable the
    unsolicited reporting of battery charge changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT*QSQ=<n>} \o \list \o \c{OK} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT*QSQ?} \o \c{*QBC: <n>}
    \row \o \c{AT*QSQ=?} \o \c{*QBC: (0,1)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{*QSQ: <rssi>,<ber>} when \c{<n>}=1 and there is a change in the
    battery charge information.

    Read command returns the current state of the result code presentation
    value \c{<n>}.  The default value is 0.

    \table
    \row \o \c{<rssi>}
         \o \list
                \o 0 -113 dBm or less
                \o 1 -111 dBm
                \o 2...30 -109 to -53 sBm
                \o 31 -51 dBm or greater
                \o 99 not known or not detectable.
            \endlist
    \row \o \c{<ber>}
         \o \list
                \o 0...7 RXQUAL values as defined in 3GPP TS 45.008.
                \o 99 not known or not detectable.
            \endlist
    \endtable
*/
void AtGsmNonCellCommands::atqsq( const QString& params )
{
#ifndef QTOPIA_AT_STRICT
    needIndicators();
    flagCommand( "*QSQ: ", atc->options()->qsq, params );
#else
    Q_UNUSED( params );
    atc->done( QAtResult::OperationNotAllowed );
#endif
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+VTD Tone Duration}
    \compat

    The \c{AT+VTD} command can be used to define the length of tones
    emitted as a result of the \c{AT+VTS} command.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VTD=<n>} \o \c{+CME ERROR: 3}
    \row \o \c{AT+VTD?} \o \c{+VTD: <n>}
    \row \o \c{AT+VTD=?} \o \c{+VTD: (0-255)}
    \endtable

    According to 3GPP TS 27.007, section C.2.12, the tone duration can only
    be queried, and never set.  This implementation always returns zero to
    indicate "manufacturer specific".

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atvtd( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( "+VTD: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+VTD: (0-255)" );
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
    \ingroup ModemEmulator::CallControl
    \bold{AT+VTS DTMF and Tone Generation}
    \compat

    The \c{AT+VTS} command can be used to generate DTMF tones during
    a voice call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VTS=<tones>} \o \c{OK}
    \row \o \c{AT+VTS=?} \o \c{+VTS: (0-9,*,#,A,B,C,D)}
    \endtable

    The \c{<tones>} parameter is a string containing the digits to be sent
    as DTMF tones.  The dual tone frequencies and tone duration parameters
    from 3GPP TS 27.007 are not supported.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmNonCellCommands::atvts( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            uint posn = 1;
            QString tones;
            if (params[posn] == QChar('"') )
                tones = QAtUtils::nextString( params, posn );
            else {
                tones = params.mid(posn, 1);
                ++posn;
            }
            if ( posn < (uint)(params.length()) ) {
                // We only support the single-argument form of tone generation.
                atc->done( QAtResult::OperationNotSupported );
            } else {
                atc->manager()->callManager()->tone( tones );
                atc->done();
            }
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+VTS: (0-9,*,#,A,B,C,D)" );
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

// This slot is called when the multiplexer opens a new channel.
void AtGsmNonCellCommands::channelOpened( int /*channel*/, QSerialIODevice *device )
{
    // Wrap the device in a new front end, and disallow GSM 07.10 multiplexing.
    AtFrontEnd *front = new AtFrontEnd( atc->options()->startupOptions, device );
    front->setDevice( device );
    front->setCanMux( false );

    // Let the session manager know about the front end so that
    // it can wrap it further with higher-level command handlers.
    emit atc->manager()->newSession( front );
}

// This slot is called when a mux is terminated or becomes invalid.
void AtGsmNonCellCommands::muxTerminated()
{
    // Restore the underlying device for AT command processing.
    atc->frontEnd()->setDevice( underlying );
    underlying = 0;

    // Destroy the multiplexer as it is no longer required.
    multiplexer->deleteLater();
    multiplexer = 0;
}

// Make sure that we have indicator support whenever someone
// uses a command that may be affected by indicators.
void AtGsmNonCellCommands::needIndicators()
{
    if ( indicators )
        return;
    indicators = new AtPhoneIndicators( this );
    connect( indicators, SIGNAL(indicatorChanged(int,int)),
             this, SLOT(indicatorChanged(int,int)) );
    connect( indicators, SIGNAL(signalQualityChanged(int)),
             this, SLOT(signalQualityChanged(int)) );
    connect( indicators, SIGNAL(batteryChargeChanged(int)),
             this, SLOT(batteryChargeChanged(int)) );
    connect( atc->manager()->callManager(), SIGNAL(setOnCall(bool)),
             indicators, SLOT(setOnCall(bool)) );
    connect( atc->manager()->callManager(),
             SIGNAL(setCallSetup(AtCallManager::CallSetup)),
             indicators, SLOT(setCallSetup(AtCallManager::CallSetup)) );
    connect( atc->manager()->callManager(),
             SIGNAL(setCallHold(AtCallManager::CallHoldState)),
             indicators, SLOT(setCallHold(AtCallManager::CallHoldState)) );

    // get initial values for indicators
    atc->manager()->callManager()->notifyCallStates();
}

// Report that an indicator has changed value.
void AtGsmNonCellCommands::indicatorChanged( int ind, int value )
{
    if ( atc->options()->cind != 0 ) {
        atc->send( "+CIEV: " + QString::number(ind + 1) + "," +
              QString::number(value) );
    }
}

// This function sends the solicited result code notification +CBC
void AtGsmNonCellCommands::sendBatteryCharge( int value )
{
    if ( value < 0 )
        atc->send( "+CBC: 0,100" );
    else
        atc->send( "+CBC: 0," + QString::number( value ) );
}

// This slot is called if the charge in the battery changes.
void AtGsmNonCellCommands::batteryChargeChanged( int value )
{
    if ( atc->options()->qbc )
        sendBatteryCharge( value );
}

// currently unused.  Stub for AT+CALA command.
void AtGsmNonCellCommands::alarmHandler( const QString & msg, int data )
{
    // we have received an alarm on this channel.  Make sure it is real.
    QList<alarm_info>::ConstIterator it;
    uint index = 0;
    for ( it = alarmList.begin(); it != alarmList.end(); ++it, index++ ) {
        if ( (*it).identifier == msg ) {
            // this is the message we want.
            atc->send( "+CALV: " + QString::number( (*it).nid ) );
            if ( !(*it).silent ) {
                // display the <message> and play the alarm <type>
                switch ( (*it).type ) {
                    case 0: // message only
                    case 1: // ringtone alarm
                    case 2: // sms alert
                    case 4: // vibrate
                    default: break;
                }
            }

            if ( (*it).recurrant == 128 ) {
                // this is a one-shot alarm.
                alarmList.removeAt( index );
            } else {
                // this is recurrant.  Find the next time and set it again.
                int currDayPosn = 0;
                QString currDayStr = QDateTime::currentDateTime().toString( "ddd" );
                if ( currDayStr == "Mon" ) currDayPosn = 0;
                else if ( currDayStr == "Tue" ) currDayPosn = 1;
                else if ( currDayStr == "Wed" ) currDayPosn = 2;
                else if ( currDayStr == "Thu" ) currDayPosn = 3;
                else if ( currDayStr == "Fri" ) currDayPosn = 4;
                else if ( currDayStr == "Sat" ) currDayPosn = 5;
                else if ( currDayStr == "Sun" ) currDayPosn = 6;

                int daysToAdd = 1;
                int bitsToShift = (currDayPosn + 1) % 7;
                while ( ! ( (1 << bitsToShift) & (*it).recurrant ) ) {
                    bitsToShift = (bitsToShift + 1) % 7;
                    daysToAdd += 1;
                }

                // and add the new alarm.
                //QDateTime newAlarmTime = QDateTime::currentDateTime().addDays( daysToAdd );
                //AlarmServerService::addAlarm ( newAlarmTime, "AtInterface", (*it).identifier, 0 );
            }

            // finished.
            Q_UNUSED( data );
            break;
        }
    }
}

// This slot is called when the QNetworkRegistration instance
// signals that the registration state has changed.
// It sends the +CREG solicited notification result code
// if required.
void AtGsmNonCellCommands::registrationStateChanged()
{
    // Report changes in the network registration state if necessary.
    if ( atc->options()->creg == 2 &&
         netReg->locationAreaCode() != -1 ) {
        QString hexlac = asHex( netReg->locationAreaCode() );
        QString hexci = asHex( netReg->cellId() );
        atc->send( "+CREG: " + QString::number( atc->options()->creg ) + "," +
              QString::number
                ( (int)(netReg->registrationState()) ) + "," +
              hexlac + "," + hexci );
    } else if ( atc->options()->creg != 0 ) {
        atc->send( "+CREG: " +
              QString::number( (int)(netReg->registrationState()) ) );
    }
}

// This slot is called when the QNetworkRegistration instance emits
// the a list of available operators due to a request.
// If we requested the list, we output each operator in turn.
void AtGsmNonCellCommands::availableOperators
        ( const QList<QNetworkRegistration::AvailableOperator>& opers )
{
    if ( requestingAvailableOperators ) {
        requestingAvailableOperators = false;
        QString status = "+COPS: ";
        QList<QNetworkRegistration::AvailableOperator>::ConstIterator it;
        for ( it = opers.begin(); it != opers.end(); ++it ) {
            if ( status.length() > 7 )
                status += ",";
            status += "(" + QString::number( (int)(*it).availability );
            status += ",\"" + QAtUtils::quote( (*it).name ) + "\",\"";
            status += QAtUtils::quote( (*it).shortName ) + "\",";
            if ( (*it).id.startsWith( "2" ) ) {
                // Back-end has supplied a numeric id for us to use.
                status += "\"" + (*it).id.mid(1) + "\"";
            } else {
                // No numeric id, so report zero.
                status += "\"0\"";
            }
            QString tech = (*it).technology;
            if ( tech == "GSMCompact" )        // No tr
                status += ",1";
            else if ( tech == "UTRAN" )        // No tr
                status += ",2";
            status += ")";
        }
        status += ",,(0-4),(0-2)";
        atc->send( status );
        atc->done();
    }
}

// This slot is called when the QNetworkRegistration instance emits
// the result of an attempt to set the current operator.
// If we have attempted to set the current operator, then
// we return this result code.
void AtGsmNonCellCommands::setCurrentOperatorResult( QTelephony::Result result )
{
    if ( settingCurrentOperator ) {
        settingCurrentOperator = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This function is used to query the QServiceNumbers service number for the specified ID.
// It is used within the AtGsmNonCellCommands class and the AtSmsCommands class.
void AtGsmNonCellCommands::queryNumber( QServiceNumbers::NumberId id )
{
    pendingQuery = id;
    serviceNumbers->requestServiceNumber( id );
}

// This function is used to set the QServiceNumbers service number for the specified ID.
// It is used within the AtGsmNonCellCommands class and the AtSmsCommands class.
void AtGsmNonCellCommands::setNumber( QServiceNumbers::NumberId id, const QString& value )
{
    pendingSet = id;
    serviceNumbers->setServiceNumber( id, value );
}

// This slot is called when the QServiceNumbers instance emits
// a service number due to a query.  This slot checks that we
// queried it, and outputs any necessary result code notifications.
void AtGsmNonCellCommands::serviceNumber
        ( QServiceNumbers::NumberId id, const QString& number )
{
    if ( id != pendingQuery )
        return;
    pendingQuery = (QServiceNumbers::NumberId)(-1);
    if ( id == QServiceNumbers::SubscriberNumber ) {
        atc->send( "+CNUM: ," + QAtUtils::encodeNumber( number ) );
    } else if ( id == QServiceNumbers::SmsServiceCenter ) {
        atc->send( "+CSCA: " + QAtUtils::encodeNumber( number ) );
    } else {
        QString status = "+CSVM: ";
        ( atc->options()->csvm ) ? status += "1" : status += "0";
        status += "," + number;
        ( number.contains('+') ) ? status += ",145" : status += ",129";
        atc->send( status );
    }
    atc->done();
}

// This slot is called when the QServiceNumbers instance emits
// the result of an attempt to set the service number for a 
// particular ID.  If we have attempted to set the service
// number, we return this result code.
void AtGsmNonCellCommands::setServiceNumberResult
        ( QServiceNumbers::NumberId id, QTelephony::Result result )
{
    if ( id != pendingSet )
        return;
    pendingSet = (QServiceNumbers::NumberId)(-1);
    atc->done( (QAtResult::ResultCode)result );
}

// This function prints the solicited notification result code +CSQ
void AtGsmNonCellCommands::sendSignalQuality( int value )
{
    if ( value < 0 )
        atc->send( "+CSQ: 99,99" );
    else
        atc->send( "+CSQ: " + QString::number( value * 31 / 100 ) + ",99" );
}

// This slot is called when the signal quality to the device changes.
void AtGsmNonCellCommands::signalQualityChanged( int value )
{
    if ( atc->options()->qsq )
        sendSignalQuality( value );
}

void AtGsmNonCellCommands::callIndicatorsStatusReady()
{
    callIndicatorsReady = true;
    while (m_pendingAtCindParams.size() > 0)
        atcind(m_pendingAtCindParams.dequeue());
}

// Used by the AT+CKPD command to send the given key code to the keypad VxD.
void AtGsmNonCellCommands::sendNextKey()
{
    if ( keys.size() == 0 ) {
        atc->done();
        return;
    }
    if ( sendRelease ) {
        QtopiaServiceRequest release
            ( "VirtualKeyboard", "processKeyEvent(int,int,int,bool,bool)" );
        release << keys[0].first;
        release << keys[0].second;
        release << (int)0;
        release << false;
        release << false;
        release.send();
        keys = keys.mid(1);
        sendRelease = false;
        QTimer::singleShot( keyPauseTime * 100, this, SLOT(sendNextKey()) );
    } else {
        QtopiaServiceRequest press
            ( "VirtualKeyboard", "processKeyEvent(int,int,int,bool,bool)" );
        press << keys[0].first;
        press << keys[0].second;
        press << (int)0;
        press << true;
        press << false;
        press.send();
        sendRelease = true;
        QTimer::singleShot( keyPressTime * 100, this, SLOT(sendNextKey()) );
    }
}

// This slot is called when the QCallWaiting instance emits
// the call waiting state.  If we requested the call waiting
// state, we send the solicited result code notification
// and return.
void AtGsmNonCellCommands::callWaitingState( QTelephony::CallClass cls )
{
    if ( requestingCallWaiting ) {
        requestingCallWaiting = false;
        if ( cls == QTelephony::CallClassNone ) {
            // The only time we report status=0 is for no call classes enabled.
            atc->send( "+CCWA: 0,7" );
        } else {
            // Break the class mask up into individual bits and report them.
            int bit = 1;
            while ( bit <= 65536 ) {
                if ( (((int)cls) & bit) != 0 ) {
                    atc->send( "+CCWA: 1," + QString::number(bit) );
                }
                bit <<= 1;
            }
        }
        atc->done();
    }
}

// This slot is called when the QCallWaiting instance emits
// the result of the an attempt to set call waiting.
// If we attempted to set call waiting, we return this
// result code.
void AtGsmNonCellCommands::setCallWaitingResult( QTelephony::Result result )
{
    if ( settingCallWaiting ) {
        settingCallWaiting = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// Generic form that matches several commands.
void AtGsmNonCellCommands::flagCommand( const QString& prefix, bool& flag,
                              const QString& params, const QString& extra )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( prefix + ( flag ? "1" : "0" ) + extra );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            if ( params == "=" || params == "=0" )
                flag = false;
            else if ( params == "=1" )
                flag = true;
            else {
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( prefix + "(0,1)" );
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

// returns a pointer to the network registration member.
// Used by (external) ATD command, and various internal commands.
QNetworkRegistration* AtGsmNonCellCommands::networkRegistration()
{
    return netReg;
}

