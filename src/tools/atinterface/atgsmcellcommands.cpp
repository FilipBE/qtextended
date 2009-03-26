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

#include "atgsmcellcommands.h"
#include "atcommands.h"
#include "atparseutils.h"

#include <qtopialog.h>
#include <QAtUtils>
#include <QCallSettings>
#include <QTimer>
#include <QAdviceOfCharge>
#include <QSimInfo>
#include <QContactModel>
#include <QSimGenericAccess>
#include <QPinManager>
#include <QTextCodec>

AtGsmCellCommands::AtGsmCellCommands( AtCommands * parent ) : QObject( parent )
{
    // we need a reference to our parent, so we can call "send", "done" etc.
    atc = parent;

    // initialise all of our instance variables.
    supplementaryServices = new QSupplementaryServices( "modem" );
    sendingUnstructuredServiceData = false;
    pinManager = new QPinManager();
    settingPinLockType = "";
    requestingPinLockType = "";
    changingPinType = "";
    callBarring = new QCallBarring();
    settingBarringStatus = false;
    requestingBarringStatusType = QCallBarring::OutgoingAll;
    settingBarringPassword = false;
    callSettings = new QCallSettings();
    requestingConnectedIdPresentation = false;
    callForwarding = new QCallForwarding( "modem" ); // no tr
    settingCallForwardingReason = -1;
    requestingCallForwardingStatusReason = -1;
    adviceOfCharge = new QAdviceOfCharge( "modem" );
    requestingCurrentCallMeter = false;
    requestingAccumulatedCallMeter = false;
    resettingAccumulatedCallMeter = false;
    requestingAccumulatedCallMeterMaximum = false;
    settingAccumulatedCallMeterMaximum = false;
    requestingPricePerUnit = false;
    settingPricePerUnit = false;

    prefNetOps = new QPreferredNetworkOperators( "modem" ); // no tr
    settingPreferredOperator = false;
    requestingOperatorNames = false;
    requestingPreferredOperatorsFromList = -1;
    testingPreferredOperators = false;

    sga = new QSimGenericAccess( "modem" );

    phonerf = new QPhoneRfFunctionality( "modem" ); // No tr
    settingPhoneRf = false;
    phoneBook = new QPhoneBook( "modem" );
    phoneBookQueried = false;
    limitsReqBy = "";
    contactModel = 0;
    availableMemoryContacts = 50000; //we set an upper limit for contact

    // ---------------------------------------

    connect( supplementaryServices,
             SIGNAL(incomingNotification(QSupplementaryServices::IncomingNotification,
                     int, const QString &)),
             this,
             SLOT(incomingSupplementaryServicesNotification(QSupplementaryServices::IncomingNotification,
                     int, const QString &)) );
    connect( supplementaryServices,
             SIGNAL(outgoingNotification
                     (QSupplementaryServices::OutgoingNotification, int)),
             this,
             SLOT(outgoingSupplementaryServicesNotification
                     (QSupplementaryServices::OutgoingNotification, int)) );
    connect( supplementaryServices,
             SIGNAL(unstructuredNotification(
                     QSupplementaryServices::UnstructuredAction, const QString &)),
             this,
             SLOT(unstructuredSupplementaryServicesNotification(QSupplementaryServices::UnstructuredAction,
                     const QString &)) );
    connect( supplementaryServices, SIGNAL(unstructuredResult(QTelephony::Result)),
             this, SLOT(unstructuredSupplementaryServicesResult(QTelephony::Result)) );

    connect( pinManager, SIGNAL(setLockStatusResult(QString,bool)),
             this, SLOT(setPinLockStatusResult(QString,bool)) );
    connect( pinManager, SIGNAL(lockStatus(QString,bool)),
             this, SLOT(pinLockStatus(QString,bool)) );
    connect( pinManager, SIGNAL(changePinResult(QString,bool)),
             this, SLOT(changePinResult(QString,bool)) );

    connect( callBarring, SIGNAL(setBarringStatusResult(QTelephony::Result)),
             this, SLOT(setBarringStatusResult(QTelephony::Result)) );
    connect( callBarring, SIGNAL(barringStatus(QCallBarring::BarringType,QTelephony::CallClass)),
             this, SLOT(barringStatus(QCallBarring::BarringType,QTelephony::CallClass)) );
    connect( callBarring, SIGNAL(changeBarringPasswordResult(QTelephony::Result)),
             this, SLOT(changeBarringPasswordResult(QTelephony::Result)) );

    connect( callSettings, SIGNAL(callerIdRestriction(QCallSettings::CallerIdRestriction,
                     QCallSettings::CallerIdRestrictionStatus)),
             this, SLOT(callerIdRestriction(QCallSettings::CallerIdRestriction,
                     QCallSettings::CallerIdRestrictionStatus)) );
    connect( callSettings, SIGNAL(setCallerIdRestrictionResult(QTelephony::Result)),
             this, SLOT(setCallerIdRestrictionResult(QTelephony::Result)) );
    connect( callSettings, SIGNAL(connectedIdPresentation(QCallSettings::PresentationStatus)),
             this, SLOT(connectedIdPresentation(QCallSettings::PresentationStatus)) );

    connect( adviceOfCharge, SIGNAL(currentCallMeter(int,bool)),
             this, SLOT(currentCallMeter(int,bool)) );
    connect( adviceOfCharge, SIGNAL(accumulatedCallMeter(int)),
             this, SLOT(accumulatedCallMeter(int)) );
    connect( adviceOfCharge, SIGNAL(accumulatedCallMeterMaximum(int)),
             this, SLOT(accumulatedCallMeterMaximum(int)) );
    connect( adviceOfCharge, SIGNAL(resetAccumulatedCallMeterResult(QTelephony::Result)),
             this, SLOT(resetAccumulatedCallMeterResult(QTelephony::Result)) );
    connect( adviceOfCharge, SIGNAL(setAccumulatedCallMeterMaximumResult(QTelephony::Result)),
             this, SLOT(setAccumulatedCallMeterMaximumResult(QTelephony::Result)) );
    connect( adviceOfCharge, SIGNAL(pricePerUnit(QString,QString)),
             this, SLOT(pricePerUnit(QString,QString)) );
    connect( adviceOfCharge, SIGNAL(setPricePerUnitResult(QTelephony::Result)),
             this, SLOT(setPricePerUnitResult(QTelephony::Result)) );
    connect( adviceOfCharge, SIGNAL(callMeterMaximumWarning()),
             this, SLOT(callMeterMaximumWarning()) );

    connect( prefNetOps, SIGNAL(operatorNames(QList<QPreferredNetworkOperators::NameInfo>)),
             this, SLOT(operatorNames(QList<QPreferredNetworkOperators::NameInfo>)) );
    connect( prefNetOps,
             SIGNAL(preferredOperators(QPreferredNetworkOperators::List,
                     QList<QPreferredNetworkOperators::Info>)),
             this,
             SLOT(preferredOperators(QPreferredNetworkOperators::List,
                     QList<QPreferredNetworkOperators::Info>)) );
    connect( prefNetOps, SIGNAL(writePreferredOperatorResult(QTelephony::Result)),
             this, SLOT(writePreferredOperatorResult(QTelephony::Result)) );

    connect( callForwarding, SIGNAL(setForwardingResult(QCallForwarding::Reason,QTelephony::Result)),
             this, SLOT(setForwardingResult(QCallForwarding::Reason,QTelephony::Result)) );
    connect( callForwarding, SIGNAL(forwardingStatus(QCallForwarding::Reason,QList<QCallForwarding::Status>)),
             this, SLOT(forwardingStatus(QCallForwarding::Reason,QList<QCallForwarding::Status>)) );

    connect( sga,
             SIGNAL(response(QString,QTelephony::Result,QByteArray)),
             this,
             SLOT(simGenericAccessResponse(QString,QTelephony::Result,QByteArray)) );

    connect( phonerf,
             SIGNAL(setLevelResult(QTelephony::Result)),
             this,
             SLOT(setLevelResult(QTelephony::Result)) );

    connect( phoneBook, SIGNAL(limits(QString,QPhoneBookLimits)),
             this, SLOT(phoneBookLimits(QString,QPhoneBookLimits)) );
    connect( phoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
             this, SLOT(phoneBookEntries(QString,QList<QPhoneBookEntry>)) );

    //------------------------------------------

    // register the 27.007 AT commands that this class provides functionality for.
    atc->add( "+CACM", this, SLOT(atcacm(QString)) );
    atc->add( "+CAMM", this, SLOT(atcamm(QString)) );
    atc->add( "+CAOC", this, SLOT(atcaoc(QString)) );
    atc->add( "+CCFC", this, SLOT(atccfc(QString)) );
    atc->add( "+CCWE", this, SLOT(atccwe(QString)) );
    atc->add( "+CLCK", this, SLOT(atclck(QString)) );
    atc->add( "+CLIR", this, SLOT(atclir(QString)) );
    atc->add( "+COLP", this, SLOT(atcolp(QString)) );
    atc->add( "+COPN", this, SLOT(atcopn(QString)) );
    atc->add( "+CPBF", this, SLOT(atcpbf(QString)) );
    atc->add( "+CPBR", this, SLOT(atcpbr(QString)) );
    atc->add( "+CPBS", this, SLOT(atcpbs(QString)) );
    atc->add( "+CPBW", this, SLOT(atcpbw(QString)) );
    atc->add( "+CPLS", this, SLOT(atcpls(QString)) );
    atc->add( "+CPOL", this, SLOT(atcpol(QString)) );
    atc->add( "+CPUC", this, SLOT(atcpuc(QString)) );
    atc->add( "+CPWD", this, SLOT(atcpwd(QString)) );
    atc->add( "+CRES", this, SLOT(atcres()) );
    atc->add( "+CSAS", this, SLOT(atcsas()) );
    atc->add( "+CSCA", this, SLOT(atcsca(QString)) );
    atc->add( "+CSDH", this, SLOT(atcsdh(QString)) );
    atc->add( "+CSIM", this, SLOT(atcsim(QString)) );
    atc->add( "+CSMP", this, SLOT(atcsmp(QString)) );
    atc->add( "+CSMS", this, SLOT(atcsms(QString)) );
    atc->add( "+CUSD", this, SLOT(atcusd(QString)) );
}

AtGsmCellCommands::~AtGsmCellCommands()
{
    delete supplementaryServices;
    delete pinManager;
    delete callBarring;
    delete callSettings;
    delete callForwarding;
    delete adviceOfCharge;
    delete prefNetOps;
    delete sga;
    delete phonerf;
    delete phoneBook;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CACM Accumulated Call Meter}
    \compat

    The \c{AT+CACM} command is used to reset or query the
    accumulated call meter advice of charge.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CACM=[<passwd>]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CACM?} \o \list
                              \o \c{+CACM: <acm>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CACM=?} \o
    \endtable

    The \c{<passwd>} parameter is equal to the SIM PIN2
    locking password, and is usually required to reset
    the accumulated call meter value.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcacm( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        case AtParseUtils::Set:
        {
            QString passwd = "";
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                passwd = QAtUtils::nextString( params, posn );
                if ( posn < (uint)params.length() ) {
                    // too many parameters
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            }

            resettingAccumulatedCallMeter = true;
            adviceOfCharge->resetAccumulatedCallMeter( passwd );
        }
        break;

        case AtParseUtils::Get:
        {
            requestingAccumulatedCallMeter = true;
            adviceOfCharge->requestAccumulatedCallMeter();
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
    \bold{AT+CAMM Accumulated Call Meter Maximum}
    \compat

    The \c{AT+CAMM} command is used to set or query
    the Advice Of Charge related accumulated call
    meter maximum value in the SIM or active
    application in the UICC.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CAMM=[<acmmax>[,<passwd>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CAMM?} \o \list
                              \o \c{+CAMM: <acmmax>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CAMM=?} \o
    \endtable

    The \c{<acmmax>} parameter contains the maximum
    number of home units allowed to be consumed by
    the subscriber.  When ACM (see \c{AT+CACM})
    reaches \c{<acmmax>}, calls are prohibited.
    SIM PIN2 is usually required to set the value.

    A zero or non-present value of \c{<acmmax>}
    disables the ACMmax feature.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcamm( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly: // flow on
        case AtParseUtils::Set:
        {
            QString passwd = "";
            uint acmmax = 0;
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                acmmax = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    passwd = QAtUtils::nextString( params, posn );
                    if ( posn < (uint)params.length() ) {
                        // too many parameters.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }
                }
            }

            settingAccumulatedCallMeterMaximum = true;
            adviceOfCharge->setAccumulatedCallMeterMaximum( acmmax, passwd );
        }
        break;

        case AtParseUtils::Get:
        {
            requestingAccumulatedCallMeterMaximum = true;
            adviceOfCharge->requestAccumulatedCallMeterMaximum();
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
    \bold{AT+CAOC Advice of Charge}
    \compat

    The \c{AT+CAOC} command enables the subscriber
    to get information about the cost of calls.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CAOC[=<mode>]} \o \list
                                      \o \c{[+CAOC: <ccm>]}
                                      \o \c{+CME ERROR: <err>}
                                    \endlist
    \row \o \c{AT+CAOC?} \o  \c{+CAOC: <mode>}
    \row \o \c{AT+CAOC=?} \o \c{[+CAOC: }(list of supported \c{<mode>}s)
    \endtable

    The \c{<mode>} parameter can take the following values:
    \list
       \o 0 Query CCM value
       \o 1 Deactivate the unsolicited reporting of CCM value
       \o 2 Activate the unsolicited reporting of CCM value
    \endlist

    The unsolicited result code \c{+CCCM: <ccm>} is sent when
    the CCM value changes, but not more than every ten seconds,
    if enabled.

    \c{<ccm>} is a string type; 3 bytes of the current call meter
    value in hexadecimal format.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcaoc( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            // assume <mode> == 0
            requestingCurrentCallMeter = true;
            adviceOfCharge->requestCurrentCallMeter();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || mode > 2 ) {
                    // too many params, or mode too big.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                if ( mode == 0 ) {
                    requestingCurrentCallMeter = true;
                    adviceOfCharge->requestCurrentCallMeter();
                } else {
                    atc->options()->cccm = ( mode == 2 );
                    atc->done();
                }
            } else {
                // mode is not an optional parameter.
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            ( atc->options()->cccm ) ? atc->send( "+CAOC: 2" ) : atc->send( "+CAOC: 1" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CAOC: (0,1,2)" );
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
    \bold{AT+CCFC Call Forwarding}
    \compat

    The \c{AT+CCFC} command allows control of the call forwarding supplementary
    service according to 3GPP TS 22.082.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CCFC=<reason>,<mode>[,<number>[,<type>[,<class>[,<subaddr>[,<satype>[,<time>]]]]]]}
         \o \list \o \c{+CME ERROR: <err>}
                  \o \bold{when <mode>=2 and command successful:}
                     \c{+CCFC: <status>,<class1>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]]
                     [<CR><LF>+CCFC: <status>,<class2>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]]
                     [...]]}
            \endlist
    \row \o \c{AT+CCFC=?} \o \c{+CCFC: }(list of supported \c{<reason>}s)
    \endtable

    Defined Values:

    \table
    \header \o Parameter \o Values
    \row \o \c{<reason>} \o \list
                              \o 0  Unconditional
                              \o 1  Mobile Busy
                              \o 2  No Reply
                              \o 3  Not Reachable
                              \o 4  All Call Forwarding
                              \o 5  All Conditional Call Forwarding
                            \endlist
    \row \o \c{<mode>} \o \list
                            \o 0  Disable
                            \o 1  Enable
                            \o 2  Query Status
                            \o 3  Registration
                            \o 4  Erasure
                          \endlist
    \row \o \c{<number>} \o string type phone number of forwarding address in format specified by \c{type}
    \row \o \c{<type>} \o type of address octet in integer format (see TS 24.008 [8]);
                          default 145 when dialling string includes international access code character `+', else 129
    \row \o \c{<subaddr>} \o string type subaddress of the format specified by \c{<satype>}
    \row \o \c{<satype>} \o type of subaddress octet in integer format (see TS 24.008 [8]);
                            default 128
    \row \o \c{<class>} \o sum of integers representing a class of information (default 7):
                            \list
                              \o 1   voice (telephony)
                              \o 2   data (refers to all bearer services)
                              \o 4   reserved for future use: fax (facsimile services)
                              \o 8   short message service
                              \o 16  data circuit sync
                              \o 32  data circuit async
                              \o 64  dedicated packet access
                              \o 128 dedicated PAD access
                              \o 7   default (voice, data and fax)
                            \endlist
    \row \o \c{<time>} \o 1..30; when "no reply" is enabled or queried, this gives the time in seconds to wait before the
                          call is forwarded.  Default value of 20
    \row \o \c{<status>} \o \list
                              \o 0  Not Active
                              \o 1  Active
                            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atccfc( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            QCallForwarding::Status status;
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint reason = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    uint mode = QAtUtils::parseNumber( params, posn );
                    switch ( mode ) {
                        case 0: // disable - flow on.
                        case 1: // enable
                        {
                            if ( posn < (uint)params.length() ) {
                                status.number = QAtUtils::nextString( params, posn );
                                if ( posn < (uint)params.length() ) {
                                    // in the current QCallForwarding, type is unused.
                                    (void)QAtUtils::parseNumber( params, posn );
                                    if ( posn < (uint)params.length() ) {
                                        status.cls = (QTelephony::CallClass)QAtUtils::parseNumber( params, posn );
                                        if ( posn < (uint)params.length() ) {
                                            // subaddress is also unused
                                            (void)QAtUtils::nextString( params, posn );
                                            if ( posn < (uint)params.length() ) {
                                                (void)QAtUtils::nextString( params, posn );
                                                if ( posn < (uint)params.length() ) {
                                                    status.time = QAtUtils::parseNumber( params, posn );
                                                } else {
                                                    if ( (QCallForwarding::Reason)reason != QCallForwarding::NoReply ) {
                                                        status.time = 0; // see QCallForwarding::Status description.
                                                    } else {
                                                        status.time = 20; // default
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        // default class = 7
                                        status.cls = QTelephony::CallClassDefault;
                                    }
                                } else {
                                    // default class = 7
                                    status.cls = QTelephony::CallClassDefault;
                                }
                            }

                            // set the forwarding rule.
                            settingCallForwardingReason = reason;
                            callForwarding->setForwarding( (QCallForwarding::Reason)reason, status, mode == 1 );
                        }
                        break;

                        case 2: // query
                        {
                            if ( posn < (uint)params.length() ) {
                                // too many params for a query.
                                atc->done( QAtResult::OperationNotAllowed );
                            } else {
                                requestingCallForwardingStatusReason = reason;
                                callForwarding->requestForwardingStatus( QCallForwarding::Reason( reason ) );
                            }
                        }
                        break;

                        case 3: // registration
                        {
                            // at the moment, the backend does not provide for this.
                            atc->done( QAtResult::OperationNotAllowed );
                        }
                        break;

                        case 4: // erasure
                        {
                            // at the moment, the backend does not provide for this.
                            atc->done( QAtResult::OperationNotAllowed );
                        }
                        break;

                        default:
                        {
                            // invalid mode.
                            atc->done( QAtResult::OperationNotAllowed );
                        }
                        break;
                    }
                } else {
                    atc->done( QAtResult::OperationNotSupported );
                }
            } else {
                atc->done( QAtResult::OperationNotSupported );
            }
        }
        break;

        case AtParseUtils::Support:
        {
            // from QCallForwarding, all 6 reasons are supported.
            atc->send( "+CCFC: (0-5)" );
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
    \bold{AT+CLCK Facility Lock}
    \compat

    This command enables locking, unlocking and interrogation of a
    MT or network facility.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLCK=<fac>,<mode>[,<passwd>[,<class>]]}
         \o \list
              \o \c{+CME ERROR: <err>}
              \o \bold{when mode = 2 and command is successful:}
                 \c{+CLCK: <status>[,<class1>}
                 \c{[<CR><LF>+CLCK: <status>,<class2>}
                 \c{[...]]}
            \endlist
    \row \o \c{AT+CLCK=?} \o \list
                               \o \c{+CLCK: }(list of supported \c{<fac>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    Defined values for the facility \c{<fac>} are as follows:

    \table
    \header \o Value \o Meaning
    \row \o "CS" \o CNTRL (lock CoNTRoL surface (e.g. phone keyboard)
    \row \o "PS" \o PH-SIM (lock PHone to SIM/UICC card)
    \row \o "PF" \o PH-FSIM (lock PHone to the very First inserted SIM/UICC card)
    \row \o "SC" \o SIM (lock SIM/UICC card)
    \row \o "AO" \o BAOC (Barr All Outgoing Calls)
    \row \o "OI" \o BOIC (Barr Outgoing International Calls)
    \row \o "OX" \o BOIC-exHC (Barr Outgoing International Call except to Home Country)
    \row \o "AI" \o BAIC (Barr All Incoming Calls)
    \row \o "IR" \o BIC-Roam (Barr Incoming Calls when Roaming outside the home country)
    \row \o "NT" \o barr incoming calls from numbers Not stored in TA memory
    \row \o "NM" \o barr incoming calls from numbers Not stored to MT memory
    \row \o "NS" \o barr incoming calls from numbers not stored to SIM/UICC memory
    \row \o "NA" \o barr incoming calls from numbers Not stored in Any memory
    \row \o "AB" \o All Barring services (applicable for \c{<mode>} = 0)
    \row \o "AG" \o All outGoing barring services (applicable for \c{<mode>} = 0)
    \row \o "AC" \o All inComing barring services (applicable for \c{<mode>} = 0)
    \row \o "FD" \o SIM card or active application in the UICC fixed dialing memory feature
    \row \o "PN" \o Network Personalisation
    \row \o "PU" \o network sUbset Personalisation
    \row \o "PP" \o service Provider Personalisation
    \row \o "PC" \o Corporate Personalisation
    \endtable

    Defined values for \c{<mode>} are 0 (unlock), 1 (lock) and 2 (query status).
    Defined values for \c{<status>} are 0 (not active) and 1 (active).

    \c{<classx>} is a sum of integers that each represent a class of information (default 7)
    Defined values for \c{<classx>} are as follows:
    \table
    \row \o 1 \o voice (telephony)
    \row \o 2 \o data (refers to all bearer services)
    \row \o 4 \o reserved for future use: fax (facsimile services)
    \row \o 8 \o sms (short message service)
    \row \o 16 \o data circuit sync
    \row \o 32 \o data circuit async
    \row \o 64 \o dedicated packet access
    \row \o 128 \o dedicated PAD access
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atclck( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                QString fac = QAtUtils::nextString( params, posn );
                if ( posn < (uint)params.length() ) {
                    uint mode = QAtUtils::parseNumber( params, posn );

                    if ( mode > 2 ) {
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }

                    QString pwd = "";
                    uint classx = 255; // barr all classes by default.

                    if ( posn < (uint)params.length() ) {
                        pwd = QAtUtils::nextString( params, posn );
                        if ( posn < (uint)params.length() ) {
                            classx = QAtUtils::parseNumber( params, posn );
                            if ( posn < (uint)params.length() || classx > 255 ) {
                                // invalid classx or too many params
                                atc->done( QAtResult::OperationNotAllowed );
                                break;
                            }
                        }
                    }

                    if ( fac == "CS" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "CNTRL PIN";
                                pinManager->setLockStatus( "CNTRL PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "CNTRL PIN";
                                pinManager->setLockStatus( "CNTRL PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "CNTRL PIN";
                                pinManager->requestLockStatus( "CNTRL PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "PS" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "PH-SIM PIN";
                                pinManager->setLockStatus( "PH-SIM PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "PH-SIM PIN";
                                pinManager->setLockStatus( "PH-SIM PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "PH-SIM PIN";
                                pinManager->requestLockStatus( "PH-SIM PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "PF" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "PH-FSIM PIN";
                                pinManager->setLockStatus( "PH-FSIM PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "PH-FSIM PIN";
                                pinManager->setLockStatus( "PH-FSIM PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "PH-FSIM PIN";
                                pinManager->requestLockStatus( "PH-FSIM PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "SC" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "SIM PIN";
                                pinManager->setLockStatus( "SIM PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "SIM PIN";
                                pinManager->setLockStatus( "SIM PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "SIM PIN";
                                pinManager->requestLockStatus( "SIM PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "AO" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::OutgoingAll, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::OutgoingAll, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::OutgoingAll;
                                callBarring->requestBarringStatus( QCallBarring::OutgoingAll );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "OI" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::OutgoingInternational, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::OutgoingInternational, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::OutgoingInternational;
                                callBarring->requestBarringStatus( QCallBarring::OutgoingInternational );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "OX" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::OutgoingInternationalExceptHome, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::OutgoingInternationalExceptHome, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::OutgoingInternationalExceptHome;
                                callBarring->requestBarringStatus( QCallBarring::OutgoingInternationalExceptHome );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "AI" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingAll, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingAll, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::IncomingAll;
                                callBarring->requestBarringStatus( QCallBarring::IncomingAll );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "IR" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingWhenRoaming, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingWhenRoaming, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::IncomingWhenRoaming;
                                callBarring->requestBarringStatus( QCallBarring::IncomingWhenRoaming );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "NT" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonTA, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonTA, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::IncomingNonTA;
                                callBarring->requestBarringStatus( QCallBarring::IncomingNonTA );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "NM" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonMT, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonMT, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::IncomingNonMT;
                                callBarring->requestBarringStatus( QCallBarring::IncomingNonMT );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "NS" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonSIM, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonSIM, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::IncomingNonSIM;
                                callBarring->requestBarringStatus( QCallBarring::IncomingNonSIM );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "NA" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonMemory, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::IncomingNonMemory, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::IncomingNonMemory;
                                callBarring->requestBarringStatus( QCallBarring::IncomingNonMemory );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "AB" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::AllBarringServices, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::AllBarringServices, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::AllBarringServices;
                                callBarring->requestBarringStatus( QCallBarring::AllBarringServices );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "AG" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::AllOutgoingBarringServices, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::AllOutgoingBarringServices, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::AllOutgoingBarringServices;
                                callBarring->requestBarringStatus( QCallBarring::AllOutgoingBarringServices );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "AC" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::AllIncomingBarringServices, pwd, (QTelephony::CallClass)classx, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingBarringStatus = true;
                                callBarring->setBarringStatus( QCallBarring::AllIncomingBarringServices, pwd, (QTelephony::CallClass)classx, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingBarringStatusType = QCallBarring::AllIncomingBarringServices;
                                callBarring->requestBarringStatus( QCallBarring::AllIncomingBarringServices );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "FD" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingFixedDialingState = true;
                                phoneBook->setFixedDialingState( false, pwd );
                            }
                            break;

                            case 1: // lock
                            {
                                settingFixedDialingState = true;
                                phoneBook->setFixedDialingState( true, pwd );
                            }
                            break;

                            case 2: // query
                            {
                                requestingFixedDialingState = true;
                                phoneBook->requestFixedDialingState();
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "PN" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "PH-NET PIN";
                                pinManager->setLockStatus( "PH-NET PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "PH-NET PIN";
                                pinManager->setLockStatus( "PH-NET PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "PH-NET PIN";
                                pinManager->requestLockStatus( "PH-NET PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "PU" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "PH-NETSUB PIN";
                                pinManager->setLockStatus( "PH-NETSUB PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "PH-NETSUB PIN";
                                pinManager->setLockStatus( "PH-NETSUB PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "PH-NETSUB PIN";
                                pinManager->requestLockStatus( "PH-NETSUB PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "PP" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "PH-SP PIN";
                                pinManager->setLockStatus( "PH-SP PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "PH-SP PIN";
                                pinManager->setLockStatus( "PH-SP PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "PH-SP PIN";
                                pinManager->requestLockStatus( "PH-SP PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else if ( fac == "PC" ) {
                        switch ( mode ) {
                            case 0: // unlock
                            {
                                settingPinLockType = "PH-CORP PIN";
                                pinManager->setLockStatus( "PH-CORP PIN", pwd, false );
                            }
                            break;

                            case 1: // lock
                            {
                                settingPinLockType = "PH-CORP PIN";
                                pinManager->setLockStatus( "PH-CORP PIN", pwd, true );
                            }
                            break;

                            case 2: // query
                            {
                                requestingPinLockType = "PH-CORP PIN";
                                pinManager->requestLockStatus( "PH-CORP PIN" );
                            }
                            break;

                            default: // error
                            {
                                atc->done( QAtResult::OperationNotSupported );
                                return;
                            }
                            break;
                        }
                    } else {
                        // invalid facility.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }
                } else {
                    // not enough parameters.
                    atc->done( QAtResult::OperationNotAllowed );
                }
            } else {
                // not enough parameters.
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CLCK: (\"CS\",\"PS\",\"PF\",\"SC\",\"AO\",\"OI\",\"OX\"," \
                    "\"AI\",\"IR\",\"NT\",\"NM\",\"NS\",\"NA\",\"AB\",\"AG\"," \
                    "\"AC\",\"FD\",\"PN\",\"PU\",\"PP\",\"PC\")" );
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
    \bold{AT+CCWE Call Meter Maximum Event}
    \compat

    The \c{AT+CCWE} command is used to enable the
    \c{+CCWV} unsolicited notification result.

    If enabled, the \c{+CCWV} unsolicited notification
    result will be returned when approximately 30
    seconds of call time remains, or if less than
    30 seconds of call time remains when a new
    call is started.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CCWE=<mode>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CCWE?} \o \list
                              \o \c{+CCWE: <mode>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CCWE=?} \o \list
                               \o \c{+CCWE: }(list of supported \c{<mode>}s)
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    The \c{<mode>} parameter can be:
    \table
      \row \o 0 \o Disable call meter warning event (default)
      \row \o 1 \o Enable call meter warning event
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atccwe( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || mode > 1 ) {
                    // invalid mode or too many params.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }

                atc->options()->ccwv = ( mode == 1 );
                atc->done();
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        case AtParseUtils::Get:
        {
            atc->options()->ccwv ? atc->send( "+CCWE: 1" ) : atc->send( "+CCWE: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CCWE: (0,1)" );
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
    \bold{AT+CLIR Calling Line Identification Restriction}
    \compat

    This command allows a calling subscriber to enable or disable
    the presentation of the CLI to the called party when originating
    a call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLIR=[<n>]} \o
    \row \o \c{AT+CLIR?} \o \c{+CLIR: <n>,<m>}
    \row \o \c{AT+CLIR=?} \o \c{+CLIR: }(list of supported \c{<n>}s)
    \endtable

    The \c{<n>} parameter sets the adjustment for outgoing calls,
    and can be 0 (default; according to CLIR subscription),
    1 (invocation), or 2 (suppression).

    The \c{<m>} parameter shows the subscriber CLIR service status
    in the network.  It can be one of:
    \table
    \row \o 0 \o CLIR not provisioned
    \row \o 1 \o CLIR provisioned in permanent mode
    \row \o 2 \o unknown (no network, etc)
    \row \o 3 \o CLIR temporary mode presentation restricted
    \row \o 4 \o CLIR temporary mode presentation allowed
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atclir( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint temp = QCallSettings::Subscription; // default
            if ( posn < (uint)params.length() ) {
                temp = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || temp > 2 ) {
                    // too many params.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            }

            settingCallerIdRestriction = true;
            callSettings->setCallerIdRestriction( (QCallSettings::CallerIdRestriction)temp );
        }
        break;

        case AtParseUtils::Get:
        {
            requestingCallerIdRestriction = true;
            callSettings->requestCallerIdRestriction();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CLIR: (0-2)" );
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
    \bold{AT+COLP Connected Line Identification Presentation}
    \compat

    This command enables a calling subscriber to get the line
    identity of the called party after setting up a mobile
    originated call.  The command enables or disables the
    presentation of the connected line identity at the TE.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+COLP=[<n>]} \o
    \row \o \c{AT+COLP?} \o \c{+COLP: <n>,<m>}
    \row \o \c{AT+COLP=?} \o \c{+COLP: }(list of supported \c{<n>}s)
    \endtable

    In this implementation of \c{AT+COLP} the line identity
    information is presented after the establishment of any
    type of call.

    The \c{<n>} parameter can be 0 (default; disable) or 1.
    The \c{<m>} parameter shows the subscriber COLP service
    status in the network.  See 27.007 for complete details.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcolp( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint n = 0; // default is 0 (disabled)
            if ( posn < (uint)params.length() ) {
                n = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() || n > 1 ) {
                    // too many params.
                    atc->done( QAtResult::OperationNotAllowed );
                    break;
                }
            }

            atc->frontEnd()->options()->colp = n;
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            requestingConnectedIdPresentation = true;
            callSettings->requestConnectedIdPresentation();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+COLP: (0,1)" );
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
    \bold{AT+COPN Read Operator Names}
    \compat

    This command returns the list of operator names from the MT.
    Each operator code that has an alphanumeric equivalent in
    the MT memory shall be returned.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+COPN} \o \c{+COPN: <numeric1>,<alpha1>}
                           \c{[<CR><LF>+COPN: <number2><alpha2>}
                           \c{[...]]}
    \row \o \c{AT+COPN=?} \o
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcopn( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly:
        {
            requestingOperatorNames = true;
            prefNetOps->requestOperatorNames();
        }
        break;

        case AtParseUtils::Support:
        {
            // null output.
            atc->done();
        }
        break;

        default:
        {
            // unknown mode.
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CPLS Selection of Preferred PLMN List}
    \compat

    The \c{AT+CPLS} command can be used to select one PLMN Selector With
    Access Technology list from the SIM card or active application in the
    UICC, which is used by the \c{AT+CPOL} command.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{+CPLS=<list>} \o \c{+CME ERROR: <err>}
    \row \o \c{+CPLS?} \o  \list
                             \o \c{+CPLS: <list>}
                             \o \c{+CME ERROR: <err>}
                           \endlist
    \row \o \c{+CPLS=?} \o \list
                             \o \c{+CPLS: } (list of supported \c{<list>}s)
                             \o \c{+CME ERROR: <err>}
                           \endlist
    \endtable

    The \c{<list>} parameter can take the following values:
    \table
    \row \o 0 \o User controlled PLMN Selector With Access Technology list
    \row \o 1 \o Operator controlled PLMN Selector With Access Technology list
    \row \o 2 \o HPLMN Selector With Access Technology list
    \endtable

    Conforms with: 3GPP TS 27.007.

    /sa AT+CPOL
*/
void AtGsmCellCommands::atcpls( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint newCPLS = QAtUtils::parseNumber( params, posn );
            if ( posn < (uint)params.length() || newCPLS > 2 ) {
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            atc->frontEnd()->options()->cpls = (int)newCPLS;
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CPLS: (0-2)" );
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            atc->send( "+CPLS: " + QString::number( atc->frontEnd()->options()->cpls ) );
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
    \bold{AT+CPOL Preferred PLMN List }
    \compat

    The \c{AT+CPOL} command is used to edit the PLMN Selector With Access
    Technology lists in the SIM card or active application in the UICC.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{+CPOL=[<index>][,<format>[,<oper>[,<GSM_AcT>,
         <GSM_Compact_AcT>,<UTRAN_AcT>]]]} \o \c{+CME ERROR: <err>}
    \row \o \c{+CPOL?} \o
         \list \o \c{+CPOL: <index1>,<format>,<oper1>[,<GSM_AcT1>,<GSM_Compact_AcT1>,<UTRAN_AcT1>]}
                  \c{[<CR><LF>+CPOL: <index2>,<format>,<oper2>[,<GSM_AcT2>,<GSM_Compact_AcT2>,<UTRAN_AcT2>]}
               \o \c{+CME ERROR: <err>}
         \endlist
    \row \o \c{+CPOL=?} \o
         \list \o \c{+CPOL: }(list of supported \c{<index>}s),list of supported \c{<format>}s)
               \o \c{+CME ERROR: <err>}
         \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.

    /sa AT+CPLS
*/
void AtGsmCellCommands::atcpol( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            QPreferredNetworkOperators::Info newOper;
            uint posn = 1;

            if ( posn < (uint)params.length() ) {
                if ( params.at(posn) != ',' ) {
                    // first param is the index.
                    newOper.index = QAtUtils::parseNumber( params, posn );
                    if ( newOper.index == 0 ) {
                        // we don't allow 0 index to be set manually.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }
                } else {
                    newOper.index = 0;
                }
            } else {
                // empty set.  not allowed.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            if ( posn < (uint)params.length() ) {
                newOper.format = QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    // fetch the numeric id or alphanumeric name of the oper
                    if ( newOper.format != 2 ) {
                        newOper.name = QAtUtils::nextString( params, posn );
                    } else {
                        bool ok = true;
                        if ( params.at(posn+1) == '"' || params.at(posn+1) == ' ' && params.at(posn+1) == '"' ) {
                            newOper.id = QAtUtils::nextString( params, posn ).toInt( &ok );
                        } else {
                            QString idStr = "";
                            posn += 1;
                            while ( posn < (uint)params.length() && params.at(posn) != ',' ) {
                                idStr += params.at( posn++ );
                            }

                            newOper.id = idStr.toInt( &ok );
                        }

                        // check to make sure that the conversion went ok.
                        if ( !ok ) {
                            // invalid opern given.
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }
                    }

                    // check for technologies.
                    if ( posn < (uint)params.length() ) {
                        // does the operator support GSM, Compact, UTRAN ?
                        uint gsm = QAtUtils::parseNumber( params, posn );
                        if ( gsm == 1 ) {
                            newOper.technologies.insert( newOper.technologies.size(), "GSM" );
                        } else if ( gsm > 1 ) {
                            // gsm param too big.
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }

                        if ( posn < (uint)params.length() ) {
                            uint gsmc = QAtUtils::parseNumber( params, posn );
                            if ( gsmc == 1 ) {
                                newOper.technologies.insert( newOper.technologies.size(), "GSMCompact" );
                            } else if ( gsmc > 1 ) {
                                // gsmc param too big.
                                atc->done( QAtResult::OperationNotAllowed );
                                break;
                            }

                            if ( posn < (uint)params.length() ) {
                                uint utran = QAtUtils::parseNumber( params, posn );
                                if ( utran == 1 ) {
                                    newOper.technologies.insert( newOper.technologies.size(), "UTRAN" );
                                } else if ( utran > 1 ) {
                                    // utran param too big.
                                    atc->done( QAtResult::OperationNotAllowed );
                                    break;
                                }

                                if ( posn < (uint)params.length() ) {
                                    // too many params.
                                    atc->done( QAtResult::OperationNotAllowed );
                                    break;
                                }
                            } else {
                                // not enough params.
                                atc->done( QAtResult::OperationNotAllowed );
                                break;
                            }

                        } else {
                            // too many / not enough params to command.
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }
                    }

                    // add the oper to the list.
                    settingPreferredOperator = true;
                    prefNetOps->writePreferredOperator(
                            (QPreferredNetworkOperators::List)(atc->frontEnd()->options()->cpls + 1), newOper );
                } else {
                    if ( newOper.format <= 2 ) {
                        if ( newOper.index != 0 ) {
                            // remove the oper at this index.
                            settingPreferredOperator = true;
                            newOper.name = "";
                            newOper.format = 0;
                            newOper.id = 0;
                            prefNetOps->writePreferredOperator(
                                    (QPreferredNetworkOperators::List)(atc->frontEnd()->options()->cpls + 1), newOper );
                        } else {
                            // the format of the <oper> in the read command is changed:
                            atc->frontEnd()->options()->cpolFormat = newOper.format;
                            atc->done();
                        }
                    } else {
                        atc->done( QAtResult::OperationNotAllowed );
                    }
                }
            } else {
                // remove the oper at this index from the currently selected list.
                settingPreferredOperator = true;
                newOper.name = "";
                newOper.format = 0;
                newOper.id = 0;
                prefNetOps->writePreferredOperator(
                        (QPreferredNetworkOperators::List)(atc->frontEnd()->options()->cpls + 1), newOper );
            }
        }
        break;

        case AtParseUtils::Support:
        {
            testingPreferredOperators = true;
	}
        // flow on into Get

        case AtParseUtils::Get:
        {
            requestingPreferredOperatorsFromList = atc->frontEnd()->options()->cpls+1;
            prefNetOps->requestPreferredOperators( (QPreferredNetworkOperators::List)(atc->frontEnd()->options()->cpls+1) );
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
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBF Find phonebook entries}
    \compat

    The \c{AT+CPBF} command allows for searching the phonebook entries
    from the current phonebook memory storage selected with \c{+CPBS}.

    \table
    \header \o Command \o Possible Responses
    \row    \o \c{AT+CPBF=<findtext>}   \o \c{[+CPBF: <index1>,<number>,<type>,<text>[<hidden>][[...]<CR><LF>
                                               +CPBF: <index2>,<number>,<type>,<text>[<hidden>]]]}, \c{+CME ERROR: <err>}
    \row    \o \c{AT+CPBF=?}            \o \c{+CPBF: [<nlength>],[<tlength>]}, \c{+CME ERROR: <err>}
    \endtable

    Test command returns the maximum length of \c{<number>} and \c{<text>} fields.

    The set command returns a set of four parameters, each representing an entry in the phonebook memory. If the memory
    has several entries which alphanumeric field start with \c{<findtext>} each entry is returned on a separate line.
    The search is caseinsensitive.

    \table
    \row \o \c{<index1>, <index2>} \o integer number indicating the location number of a phonebook entry
    \row \o \c{<number>} \o string type phone number of format \c{<type>}
    \row \o \c{<type>}  \o string type, phone number of format \c{<type>}; default 145
    when dialling string includes "+", otherwise 129
    \row \o \c{<text>} \o string type, character set as specified by selected character set \c{+CSCS}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcpbf( const QString& params )
{
    switch( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::Support:
        {
            QString status;
            if ( atc->options()->phoneStore == "ME" ) {
                status = "+CPBF: 60,60"; //fixed values
                atc->send( status );
                atc->done();
            } else {
                limitsReqBy = "cpbfSupport";
                phoneBookQueried = true;
                phoneBook->requestLimits( atc->options()->phoneStore );
            }
        }
        break;
        case AtParseUtils::Set:
        {
            uint pos = 1;
            if ( pos >= (uint)params.length() || params[1] != QChar('"') ) {
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }

            pbSearchText = atc->options()->nextString( params, pos);

            if ( atc->options()->phoneStore == "ME" ) {
                if ( !contactModel )
                    initializeMemoryPhoneBook();
                //find contact with given name
                QModelIndex start = contactModel->index(0, 0);
                QModelIndexList list = contactModel->match( start,
                        0 /*don't care what role*/,
                        QVariant( pbSearchText ), -1 );

                if ( list.count() == 0 ) {
                    atc->done( QAtResult::NotFound );
                    return;
                }
                QSet<QUniqueId> foundIds;
                foreach( QModelIndex i, list )
                    foundIds.insert( contactModel->id( i ) );

                QString status = "+CPBF: %1,\"%2\",%3,\"%4\"";
                QString temp;
                QList<int> usedIndices = contactIndices.uniqueKeys();
                QList<int>::const_iterator iter;
                for ( iter = usedIndices.constBegin();
                        iter!= usedIndices.constEnd();
                        iter++ )
                {
                    ContactRecord rec = contactIndices.value( *iter );
                    if ( foundIds.contains( rec.id ) ) {
                        QContact cnt = contactModel->contact( rec.id );
                        //QContactModel::match() matches beginning of firstname and lastname
                        //whereas we need a matching on label()
                        if ( !cnt.label().startsWith( pbSearchText, Qt::CaseInsensitive ) )
                            continue;
                        temp = status.arg( *iter );
                        QString number = cnt.phoneNumber( rec.type ).left( 60 );
                        temp = temp.arg( number );
                        if ( number[0] == QChar('+') )
                            temp = temp.arg( 145 );
                        else
                            temp = temp.arg( 129 );
                        temp = temp.arg( QAtUtils::quote( QString::fromLatin1( atc->options()->codec->fromUnicode( ( cnt.label().left( 60 )) ) ) ) );
                        atc->send( temp );
                    }
                }
                atc->done();
            } else {
                limitsReqBy = "cpbfSet";
                phoneBookQueried = true;
                phoneBook->getEntries( atc->options()->phoneStore );
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
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBR Read phonebook entries}
    \compat

    The \c{AT+CPBR} command allows for reading the phonebook entries
    from the current phonebook memory storage selected with \c{+CPBS}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBPR=<index1>[,<index2>]} \o \c{[+CPBR: <index1>,<number>,<type>,<text>[[...]<CR><LF>
                                                   +CPBR: <index2>,<number>,<type>,<text>]]}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBR=?} \o \c{+CPBR (list of supported <index>s), [<nlength>], [<tlength>]}, \c{+CME ERROR: <err>}
    \endtable

    Test command returns the available location indices, the maximum length of phone number
    fields and the maximum length of text fields.

    The set command returns a set of four parameters, each representing an entry in the phonebook memory. If the memory
    has several entries each entry is returned on a separate line.

    \table
    \row \o \c{<index>, <index1>, <index2>} \o integer number indicating the location number of a phonebook entry
    \row \o \c{<number>} \o string type phone number of format \c{<type>}
    \row \o \c{<type>}  \o string type, phone number of format \c{<type>}; default 145
    when dialling string includes "+", otherwise 129
    \row \o \c{<text>} \o string type, character set as specified by selected character set \c{+CSCS}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcpbr( const QString& params )
{
    if ( !contactModel && atc->options()->phoneStore == "ME" )
        initializeMemoryPhoneBook();
    switch( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::Set:
            {
                uint pos = 1;
                uint index1 = QAtUtils::parseNumber( params, pos);
                uint index2 = index1;
                if ( pos < (uint)params.length() ) //second param
                    index2 = QAtUtils::parseNumber( params, pos); //0 if not passed
                if ( index1 < 1 || index1 > index2 ) {
                    atc->done( QAtResult::InvalidIndex );
                    return;
                }

                if ( atc->options()->phoneStore == "ME" ) {
                    //get limits so that we can check for invalid indices
                    if ( 1 > index1 || availableMemoryContacts < (int) index2 )
                    {
                       atc->done( QAtResult::InvalidIndex );
                       return;
                    }

                    const QString raw = "+CPBR: %1,\"%2\",%3,\"%4\"";
                    QString status;
                    QString name;
                    QString number;
                    for ( uint i = index1; i<=index2; i++ ) {
                        if ( !contactIndices.contains( i ) )
                            continue;
                        status = raw.arg( i );
                        QContact c = contactModel->contact( contactIndices.value( i ).id );
                        //number/text field length: 60 chars
                        number = c.phoneNumber( contactIndices.value( i ).type ).left( 60 );
                        status = status.arg( number );
                        if ( number[0] == QChar('+') )
                            status = status.arg( 145 );
                        else
                            status = status.arg( 129 );
                        name = c.label();
                        name = name.left(60);
                        status = status.arg( QAtUtils::quote( QString::fromLatin1( atc->options()->codec->fromUnicode( name ) ) ) );
                        atc->send( status );
                    }
                    atc->done();

                } else {
                    limitsReqBy = "cpbrSet";
                    phoneBookIndex.first = index1;
                    phoneBookIndex.second = index2;
                    phoneBookQueried = true;
                    phoneBook->requestLimits( atc->options()->phoneStore );
                }
            }
            break;
        case AtParseUtils::Support:
            {
                if ( atc->options()->phoneStore == "ME" ) {
                    QString status = "+CPBR: (1,%1),60,60";
                    status = status.arg( availableMemoryContacts );
                    atc->send( status );
                    atc->done();
                } else {
                    limitsReqBy = "cpbrSupport";
                    phoneBookQueried = true;
                    phoneBook->requestLimits( atc->options()->phoneStore );
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
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBS Select phonebook memory storage}
    \compat

    The \c{AT+CPBS} command allows for querying and selecting the
    phone book storage.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBPS=<storage>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBS?} \o \c{+CPBS: <storage>,[<used>,<total>]}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBS=?} \o \c{+CPBS (list of <storage>s)}
    \endtable

    The set command sets the selected phonebook memroy to <storage>. A list of available phonebook memories
    can be obtained when using the test command.

    The Read command returns the currently selected storage and the used/total number of locations in the selected
    memory.

    This implementation is not entirely conform with 3GPP TS 27.007. It is not
    possible to set a password for phone book stores.
*/
void AtGsmCellCommands::atcpbs( const QString& params )
{
    switch( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::CommandOnly:
        case AtParseUtils::Get:
        {
            if ( atc->options()->phoneStore == "ME" ) {
                if ( !contactModel && atc->options()->phoneStore == "ME" )
                    initializeMemoryPhoneBook();
                QString status = "+CPBS: \"ME\",%1,%2";
                status = status.arg( contactIndices.size() ).arg( availableMemoryContacts );
                atc->send( status );
                atc->done();
            } else {
                /*if ( options()->phoneStorePw.isEmpty() )
                    phoneBook->clearPassword( options()->phoneStore );
                else
                    phoneBook->setPassword( options()->phoneStore, options()->phoneStorePw );*/
                limitsReqBy = "cpbs";
                phoneBookQueried = true;
                phoneBook->requestLimits( atc->options()->phoneStore );
            }
        }
        break;
        case AtParseUtils::Support:
        {
            QString result = "+CPBS: (";
            QStringList storages = phoneBook->storages();
            storages.removeAll( "MT" ); //no support for MT atm
            if ( !storages.contains( "ME" ) )
                storages.append( "ME" );
            for( int i = 0; i<storages.count(); i++ ) {
                result.append( "\"" + QAtUtils::quote(storages[i]) + "\"," );
            }

            if ( storages.count() )
                result.chop( 1 );//remove last comma
            result.append( QLatin1String(")") );

            atc->send( result );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            uint posn = 1;
            QString store = QAtUtils::nextString( params, posn );
            if ( !phoneBook->storages().contains( store ) ) {
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }
            if ( !QAtUtils::nextString( params, posn ).isEmpty() )
            {
                //QPhoneBook cannot report an error when the user provides a wrong password
                atc->done( QAtResult::OperationNotSupported );
                return;
            }
            atc->options()->phoneStore = store;
            atc->done();
            if ( store == "ME" ) {
                QTimer::singleShot( 5, this, SLOT(initializeMemoryPhoneBook()) );
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
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBW Write phonebook entry}
    \compat

    The \c{AT+CPBW} command allows for writing of the phonebook entries
    from the current phonebook memory storage selected with \c{+CPBS}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBPW=[<index>][,<number>[,<type>,<text>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBW=?} \o \list
                               \o \c{+CPBW (list of supported <index>s), [<nlength>], (list of supported <type>s), [<tlength>]}
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    Test command returns the available location indices, the maximum length of phone number
    fields, the supported number formats of the storage and the maximum length of text fields.

    The set command expects a set of four parameters. \c{<index>} determines the location where the current write operation takes place.
    In addition the entry field contains the phone number \c{<number>} (in the format \c{<type>}),
        and the text associated with the given number. If these fields are omitted the phonebook entry at \c{<index>} is deleted. If
    \c{<index>} is omitted but \c{<number>} is given the entry is saved at the first free location in the phonebook.

    \table
    \row \o \c{<index>} \o integer number indicating the location number of a phonebook memory entry
    \row \o \c{<number>} \o string type phone number of format \c{<type>}
    \row \o \c{<type>}  \o string type, phone number of format \c{<type>}; default 145
    when dialling string includes "+", otherwise 129
    \row \o \c{<text>} \o string type, character set as specified by selected character set \c{+CSCS}
    \endtable

    \table
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcpbw( const QString& params )
{
    if ( !contactModel && atc->options()->phoneStore == "ME" )
        initializeMemoryPhoneBook();
    QStringList notSupportedStores;
    notSupportedStores << "DC" << "EN" << "MC" << "RC" << "LD" << "MT";
    switch( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::Set:
            {
                if ( notSupportedStores.contains(atc->options()->phoneStore) ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    return;
                }
                uint pos = 1;
                //number of parameters determines operation
                QStringList pList = params.split( "," );
                entryToWrite = QPhoneBookEntry();
                switch( pList.count() ) {
                    case 1:
                        {
                            //just index passed ->delete entry - AT+CPBW=3
                            uint index = QAtUtils::parseNumber( params, pos );
                            if ( atc->options()->phoneStore == "ME" ) {
                                writeMemoryPhoneBookEntry( true, index );
                                return;
                            } else {
                                limitsReqBy = "cpbwSet-Remove";
                                entryToWrite.setIndex( index );
                            }
                        }
                        break;
                    case 2:
                        {
                            //add number w/o text at index  - AT+CPBW=2,"0544444"
                            uint index = QAtUtils::parseNumber( params, pos );
                            QString number = QAtUtils::nextString( params, pos );
                            //test for letters in dial string
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9')
                                        && number.at(i) != '+' ) {
                                    atc->done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }

                            if ( atc->options()->phoneStore == "ME" ) {
                                writeMemoryPhoneBookEntry( false, index, number );
                                return;
                            } else {
                                limitsReqBy = "cpbwSet-Number@Index";
                                entryToWrite.setIndex( index );
                                entryToWrite.setNumber( number );
                            }
                        }
                        break;
                    case 3:
                        {
                            //add entry at next available spot - AT+CPBW="4444",129,"text"
                            QString number = QAtUtils::nextString( params, pos );
                            //test for letters in dial string
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9')
                                        && number.at(i) != '+' ) {
                                    atc->done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }

                            uint type = QAtUtils::parseNumber( params, pos );
                            if ( type != 129 && type != 145 ) {
                                atc->done( QAtResult::OperationNotAllowed );
                                return;
                            }
                            QString text = atc->options()->nextString( params, pos );
                            //test for letters
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9')
                                        && number.at(i) != '+' ) {
                                    atc->done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }

                            if ( atc->options()->phoneStore == "ME" ) {
                                //find next available phone book index
                                uint index = 0;
                                for( int i = 1; i <= availableMemoryContacts; i++ )
                                {
                                    if ( !contactIndices.contains( i ) ) {
                                        index = i;
                                        break;
                                    }
                                }
                                if ( index == 0 ) { //could not find free index
                                    atc->done( QAtResult::MemoryFull );
                                    return;
                                }
                                writeMemoryPhoneBookEntry( false, index, number, text );
                                return;
                            } else {
                                limitsReqBy = "cpbwSet-Number@nextIndex";
                                entryToWrite.setNumber( number );
                                entryToWrite.setText( text );
                            }
                        }
                        break;
                    case 4:
                        {
                            //add entry incl text at index - AT+CPBW=1,"4444",129,"text"
                            uint index = QAtUtils::parseNumber( params, pos );
                            QString number = QAtUtils::nextString( params, pos );
                            //test for letters in dial string
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9')
                                        && number.at(i) != '+' ) {
                                    atc->done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }
                            uint type = QAtUtils::parseNumber( params, pos );
                            if ( type != 129 && type != 145 ) {
                                atc->done( QAtResult::OperationNotAllowed );
                                return;
                            }
                            QString text = atc->options()->nextString( params, pos );

                            if ( atc->options()->phoneStore == "ME" ) {
                                writeMemoryPhoneBookEntry( false, index, number, text );
                                return;
                            } else {
                                limitsReqBy = "cpbwSet-NumberText@Index";
                                entryToWrite.setIndex( index );
                                entryToWrite.setNumber( number );
                                entryToWrite.setText( text );
                            }
                        }
                        break;
                    default:
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                }

                phoneBookQueried = true;
                phoneBook->requestLimits( atc->options()->phoneStore );

            }
            break;
        case AtParseUtils::Support:
            {
                if ( notSupportedStores.contains(atc->options()->phoneStore) ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    return;
                }
                if ( atc->options()->phoneStore == "ME" ) {
                    QString status = "+CPBW: (1-%1),60,(129,145),60";
                    status = status.arg( availableMemoryContacts );
                    atc->send( status );
                    atc->done();
                } else {
                    limitsReqBy = "cpbwSupport";
                    phoneBookQueried = true;
                    phoneBook->requestLimits( atc->options()->phoneStore );
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
    \bold{AT+CPUC Price Per Unit and Currency Table}
    \compat

    The \c{AT+CPUC} command is used to set the parameters
    of the Advice Of Charge related price per unit and
    currency table in the SIM card or active application
    in the UICC.  PUCT information can be used to convert
    the home units (as used in \c{AT+CAOC},\c{AT+CACM}
    and \c{AT+CAMM}) into currency units.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPUC=<currency>,<ppu>[,<passwd>]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CPUC?} \o \list
                              \o \c{+CPUC: <currency>,<ppu>}
                              \o \c{+CME ERROR: <err>}
                            \endlist
    \row \o \c{AT+CPUC=?} \o
    \endtable

    The SIM PIN2 password is usually required to set the
    Advice Of Charge price per unit parameters.

    The \c{<currency>} parameter is a 3-character string
    currency code (e.g. "GBP", "DEM").

    The \c{<ppu>} parameter is a string type price per
    unit; dot is used as a decimal separator
    (e.g. "2.66").

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcpuc( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                QString currency = QAtUtils::nextString( params, posn );
                if ( posn < (uint)params.length() ) {
                    QString passwd = "";
                    QString ppu = QAtUtils::nextString( params, posn );
                    if ( posn < (uint)params.length() ) {
                        passwd = QAtUtils::nextString( params, posn );
                        if ( posn < (uint)params.length() ) {
                            // too many parameters.
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }
                    }

                    settingPricePerUnit = true;
                    adviceOfCharge->setPricePerUnit( currency, ppu, passwd );
                    break;
                }
            }

            // if we get here, we had an error parsing parameters.
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;

        case AtParseUtils::Get:
        {
            requestingPricePerUnit = true;
            adviceOfCharge->requestPricePerUnit();
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
    \bold{AT+CPWD Change Password}
    \compat

    The command allows a new password to be set for the facility lock function
    defined by the \c{AT+CLCK} command.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPWD=<fac>,<oldpwd>,<newpwd>} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CPWD=?} \o \list
                               \o \c{+CPWD: }list of supported \c{(<fac>,<pwdlength>)}s
                               \o \c{+CME ERROR: <err>}
                             \endlist
    \endtable

    All of the \c{<fac>}'s from \c{AT+CLCK} plus "P2" (SIM Pin 2) are defined for this command.
    \c{<oldpwd>},\c{<newpwd>} are strings; the previous and new passwords for the facility.
    \c{<pwdlength>} is the maximum allowed length of a password for the facility.

    Conforms with: 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcpwd( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                QString fac = QAtUtils::nextString( params, posn );
                if ( posn < (uint)params.length() ) {
                    QString oldpwd = QAtUtils::nextString( params, posn );
                    if ( posn < (uint)params.length() ) {
                        QString newpwd = QAtUtils::nextString( params, posn );

                        // have our three parameters.  Pass to the required facility handler.
                        if ( fac == "CS" ) {
                            changingPinType = "CNTRL PIN";
                            pinManager->changePin( "CNTRL PIN", oldpwd, newpwd );
                        } else if ( fac == "PS" ) {
                            changingPinType = "PH-SIM PIN";
                            pinManager->changePin( "PH-SIM PIN", oldpwd, newpwd );
                        } else if ( fac == "PF" ) {
                            changingPinType = "PH-FSIM PIN";
                            pinManager->changePin( "PH-FSIM PIN", oldpwd, newpwd );
                        } else if ( fac == "SC" ) {
                            changingPinType = "SIM PIN";
                            pinManager->changePin( "SIM PIN", oldpwd, newpwd );
                        } else if ( fac == "AO" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::OutgoingAll, oldpwd, newpwd );
                        } else if ( fac == "OI" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::OutgoingInternational, oldpwd, newpwd );
                        } else if ( fac == "OX" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::OutgoingInternationalExceptHome, oldpwd, newpwd );
                        } else if ( fac == "AI" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::IncomingAll, oldpwd, newpwd );
                        } else if ( fac == "IR" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::IncomingWhenRoaming, oldpwd, newpwd );
                        } else if ( fac == "NT" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::IncomingNonTA, oldpwd, newpwd );
                        } else if ( fac == "NM" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::IncomingNonMT, oldpwd, newpwd );
                        } else if ( fac == "NS" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::IncomingNonSIM, oldpwd, newpwd );
                        } else if ( fac == "NA" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::IncomingNonMemory, oldpwd, newpwd );
                        } else if ( fac == "AB" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::AllBarringServices, oldpwd, newpwd );
                        } else if ( fac == "AG" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::AllOutgoingBarringServices, oldpwd, newpwd );
                        } else if ( fac == "AC" ) {
                            settingBarringPassword = true;
                            callBarring->changeBarringPassword( QCallBarring::AllIncomingBarringServices, oldpwd, newpwd );
                        } else if ( fac == "FD" ) {
                            // not sure.  The fixed dialing password in QPhoneBook appears to be the same as "P2" (SIM Pin2)
                        } else if ( fac == "PN" ) {
                            changingPinType = "PH-NET PIN";
                            pinManager->changePin( "PH-NET PIN", oldpwd, newpwd );
                        } else if ( fac == "PU" ) {
                            changingPinType = "PH-NETSUB PIN";
                            pinManager->changePin( "PH-NETSUB PIN", oldpwd, newpwd );
                        } else if ( fac == "PP" ) {
                            changingPinType = "PH-SP PIN";
                            pinManager->changePin( "PH-SP PIN", oldpwd, newpwd );
                        } else if ( fac == "PC" ) {
                            changingPinType = "PH-CORP PIN";
                            pinManager->changePin( "PH-CORP PIN", oldpwd, newpwd );
                        } else if ( fac == "P2" ) {
                            changingPinType = "SIM PIN2";
                            pinManager->changePin( "SIM PIN2", oldpwd, newpwd );
                        } else {
                            // invalid facility
                            atc->done( QAtResult::OperationNotAllowed );
                        }

                        // we let the "setPasswordResult" handlers deal with the response.
                        return;
                    }
                }
            }

            // if we get to here, the user has not entered enough params.
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;

        case AtParseUtils::Support:
        {
            // unfinished - need some functions to get max pin size for each.
            atc->send( "+CLCK: (\"CS\",pin_size),(\"PS\",pin_size),(\"PF\",pin_size),(\"SC\",pin_size)" \
                    ",(\"AO\",pin_size),(\"OI\",pin_size),(\"OX\",pin_size),(\"AI\",pin_size)" \
                    ",(\"IR\",pin_size),(\"NT\",pin_size),(\"NM\",pin_size),(\"NS\",pin_size)" \
                    ",(\"NA\",pin_size),(\"AB\",pin_size),(\"AG\",pin_size),(\"AC\",pin_size)" \
                    ",(\"FD\",pin_size),(\"PN\",pin_size),(\"PU\",pin_size),(\"PP\",pin_size)" \
                    ",(\"PC\",pin_size),(\"P2\",pin_size)" );
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
    \bold{AT+CSIM General SIM Access}
    \compat

    This command allows a direct control of the SIM by a distant application
    on the TE.

    \table
    \header \o Comand \o Possible Responses
    \row \o \c{AT+CSIM=<length>,<command>}
         \o \list
              \o \c{+CSIM: <length>,<response>}
              \o \c{+CME ERROR: <err>}
            \endlist
    \row \o \c{AT+CSIM=?} \o
    \endtable

    Conforms with 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcsim( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            if ( posn < (uint)params.length() ) {
                // we disregard the length parameter - use QString to allocate.
                (void)QAtUtils::parseNumber( params, posn );
                if ( posn < (uint)params.length() ) {
                    simRequestId = sga->command( ( QAtUtils::nextString( params, posn ) ).toAscii() );
                } else {
                    atc->done( QAtResult::OperationNotAllowed );
                }
            } else {
                atc->done( QAtResult::OperationNotAllowed );
            }
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
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSMP Set Text Mode Parameters}
    \compat

    The \c{AT+CSMP} command sets the additional parameters that are passed
    in the header of an SMS message when sending messages in text mode
    (\c{AT+CMGF=1}).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSMP=[<fo>[,<vp>[,<pid>[,<dcs>]]]]} \o \c{OK}
    \row \o \c{AT+CSMP?} \o \c{+CSMP: 1,167,0,0}
    \row \o \c{AT+CSMP=?} \o \c{OK}
    \endtable

    This implementation always sends SMS messages with the default
    GSM values, irrespective of what is set with \c{AT+CSMP}.
    Applications should use PDU mode for sending messages with
    specific header values.

    Conforms with 3GPP TS 27.005.
*/
void AtGsmCellCommands::atcsmp( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            // Return the standard defaults.
            atc->send( "+CSMP: 1,167,0,0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            // The spec says to just respond with OK, so that is all we do.
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            // Accept anything and then just ignore it.
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
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSMS Select Message Service}
    \compat

    The \c{AT+CSMS} command selects the short message service to use.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSMS=<service>} \o \c{+CSMS: <mt>,<mo>,<bm>}
    \row \o \c{AT+CSMS?} \o \c{+CSMS: <service>,<mt>,<mo>,<bm>}
    \row \o \c{AT+CSMS=?} \o \c{+CSMS:} (list of supported \c{<service>}s)
    \endtable

    This implementation says that it supports service 0 and 1,
    mobile-terminated messages, mobile-originated messages, and
    cell broadcast messages, irrespective of the underlying
    modem's message support.  The underlying modem will select the
    correct service automatically.

    Conforms with 3GPP TS 27.005.
*/
void AtGsmCellCommands::atcsms( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( "+CSMS: " + QString::number( atc->options()->smsService ) +
                  ",1,1,1" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CSMS: (0,1)" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            if ( params == "=0" )
                atc->options()->smsService = 0;
            else if ( params == "=1" )
                atc->options()->smsService = 1;
            else {
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }
            atc->send( "+CSMS: 1,1,1" );
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
    \bold{AT+CUSD Unstructured Supplementary Service Data}
    \compat

    The \c{AT+CUSD} command allows control of the Unstructured
    Supplementary Service Data (USSD).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CUSD=[<n>[,<str>[,<dcs>]]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CUSD?} \o \c{+CUSD: <n>}
    \row \o \c{AT+CUSD=?} \o \c{+CUSD: }(list of supported \c{<n>}s)
    \endtable

    The \c{<n>} parameter can take the following values:
    \list
      \o 0 Disable result code presentation to TE (default)
      \o 1 Enable result code presentation to TE
      \o 2 Cancel session (not applicable to read command response)
    \endlist

    When \c{<str>} is given, a mobile initiated USSD string
    is sent to the network.  The response USSD string from
    the network is returned in a subsequent unsolicited
    +CUSD result code.

    The \c{<dcs>} parameter defines the Cell Broadcast Data
    Coding Scheme in integer format (default 0).

    The unstructured supplementary service data response
    is of the form \c{+CUSD: <m>[,<str>,<dcs>]} where
    \c{<m>} can take the following values:
    \list
      \o 0 No further user action required
      \o 1 Further user action required
      \o 2 USSD terminated by network
      \o 3 Other local client has responded
      \o 4 Operation not supported
      \o 5 Network time-out
    \endlist

    Conforms with 3GPP TS 27.007.
*/
void AtGsmCellCommands::atcusd( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            bool networkResponse = false;
            uint posn = 1;
            uint n = 0;
            if ( posn < (uint)params.length() ) {
                n = QAtUtils::parseNumber( params, posn );

                // set or cancel.
                if ( n > 2 ) {
                    atc->done( QAtResult::OperationNotAllowed );
                    return;
                } else if ( n == 2 ) {
                    networkResponse = true;
                    sendingUnstructuredServiceData = true;
                    supplementaryServices->cancelUnstructuredSession();
                    break;
                } else {
                    atc->options()->cusd = ( n == 1 );
                }

                // send any USS data we need to.
                if ( posn < (uint)params.length() ) {
                    QString data = QAtUtils::nextString( params, posn );
                    uint dcs = 0;

                    if ( posn < (uint)params.length() ) {
                        dcs = QAtUtils::parseNumber( params, posn );

                        if ( posn < (uint)params.length() ) {
                            atc->done( QAtResult::OperationNotAllowed );
                            break;
                        }
                    }

                    if ( data.length() == 0 ) {
                        // null USSD string.
                        atc->done( QAtResult::OperationNotAllowed );
                        break;
                    }

                    // send the USSD to the network.
                    networkResponse = true;
                    sendingUnstructuredServiceData = true;
                    supplementaryServices->sendUnstructuredData( QAtUtils::decode( data, atc->options()->codec ) );
                    // not 100% certain about the above line of code... Decoding may be unnecessary.
                }
            } else {
                // by default, disable CUSD notification
                atc->options()->cusd = false;
            }

            // return result code if required.
            if ( !networkResponse ) {
                atc->done();
            }
        }
        break;

        case AtParseUtils::Get:
        {
            atc->options()->cusd ? atc->send( "+CUSD: 1" ) : atc->send( "+CUSD: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CUSD: (0-2)" );
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

// This slot is called when the QSupplementaryServices instance
// emits a notification of incoming data.  If we have incoming
// notifications set to `on', we send the +CSSU unsolicited
// result code notification.
void AtGsmCellCommands::incomingSupplementaryServicesNotification
        ( QSupplementaryServices::IncomingNotification type,
          int groupIndex, const QString & number )
{
    if ( atc->options()->cssu ) {
        QString status = "+CSSU: " + QString::number( (uint)type );
        if ( groupIndex != 0 ) {
            status += "," + QString::number( groupIndex );
        }

        if ( type == QSupplementaryServices::MT_ExplicitTransfer ) {
            status += "," + number;
        }

        atc->send( status );
    }
}

// This slot is called when the QSupplementaryServices instance
// emits a notification of outgoing data.  If we have outgoing
// notifications set to `on', we send the +CSSI unsolicited
// result code notification.
void AtGsmCellCommands::outgoingSupplementaryServicesNotification
        ( QSupplementaryServices::OutgoingNotification type, int groupIndex )
{
    if ( atc->options()->cssi ) {
        QString status = "+CSSI: " + QString::number( (uint)type );
        if ( groupIndex != 0 ) {
            status += "," + QString::number( groupIndex );
        }

        atc->send( status );
    }
}

// This slot is called when the QSupplementaryServices instance
// emits a notification from the network.  If we have USSD
// notifications set to `on', we send the +CUSD unsolicited
// result code notification.
void AtGsmCellCommands::unstructuredSupplementaryServicesNotification
        ( QSupplementaryServices::UnstructuredAction action, const QString & data )
{
    if ( atc->options()->cusd ) {
        QString status = "+CUSD: ";
        status += QString::number( (uint)action );
        if ( data.length() > 0 ) {
            // assume default <dcs> of 0
            status += "," + data + ",0";
        }

        atc->send( status );
    }
}

// This slot is called when the QSupplementaryServices instance
// emits the result of an attempt to send USSD to the network.
// If we attempted to send unstructured service data,
// we return this result code.
void AtGsmCellCommands::unstructuredSupplementaryServicesResult
        ( QTelephony::Result result )
{
    if ( sendingUnstructuredServiceData ) {
        sendingUnstructuredServiceData = false;
        atc->done( (QAtResult::ResultCode)result );
    } else {

    }
}

// This slot is called when the QPinManager instance emits
// the result of an attempt to set the status of a PIN lock.
// If we attempted to set the status, we return this
// result code.
void AtGsmCellCommands::setPinLockStatusResult( const QString& type, bool valid )
{
    if ( settingPinLockType == type ) {
        settingPinLockType = "";
        valid ? atc->done() : atc->done( QAtResult::Error );
    }
}

// This slot is called when the QPinManager instance emits
// the status of a particular PIN lock.
// If we requested a PIN lock of this type, we return
// the status of the PIN lock.
void AtGsmCellCommands::pinLockStatus( const QString& type, bool enable )
{
    if ( requestingPinLockType == type ) {
        requestingPinLockType = "";
        enable ? atc->send( "+CLCK: 1" ) : atc->send( "+CLCK: 0" );
    }
}

// This slot is called when the QPinManager instance emits
// the result of an attempt to change a pin.
// If we attempted to change a pin of the given type,
// we return this result code.
void AtGsmCellCommands::changePinResult( const QString& type, bool valid )
{
    if ( changingPinType == type ) {
        changingPinType = "";
        valid ? atc->done() : atc->done( QAtResult::Error );
    }
}

// This slot is called when the QCallBarring instance emits
// the result of an attempt to set the barring status.
// If we attempted to set the barring status, we return
// this result code.
void AtGsmCellCommands::setBarringStatusResult( QTelephony::Result result )
{
    if ( settingBarringStatus ) {
        settingBarringStatus = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the QCallBarring instance emits
// the barring status for a particular type of call.
// If we requested the barring status for this type of call,
// we send the solicited result code notification of
// which call classes are barred for this call type,
// and return.
void AtGsmCellCommands::barringStatus( QCallBarring::BarringType type, QTelephony::CallClass cls )
{
    if ( requestingBarringStatusType == type ) {
        requestingBarringStatusType = QCallBarring::OutgoingAll;
        bool needsNewLine = false;
        uint classes = (uint)cls;

        // first, check for no barring.
        if ( classes == 0 ) {
            // this is returned only if call barring is inactive for every class.
            atc->send( "+CLCK: 0" );
        }

        // then, check for other classes
        if ( classes >= (uint)QTelephony::CallClassDedicatedPADAccess ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassDedicatedPADAccess ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassDedicatedPADAccess;
        }

        if ( classes >= (uint)QTelephony::CallClassDedicatedPacketAccess ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassDedicatedPacketAccess ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::QTelephony::CallClassDedicatedPacketAccess;
        }

        if ( classes >= (uint)QTelephony::CallClassDataCircuitAsync ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassDataCircuitAsync ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassDataCircuitAsync;
        }

        if ( classes >= (uint)QTelephony::CallClassDataCircuitSync ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassDataCircuitSync ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassDataCircuitSync;
        }

        if ( classes >= (uint)QTelephony::CallClassSMS ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassSMS ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassSMS;
        }

        if ( classes >= (uint)QTelephony::CallClassFax ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassFax ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassFax;
        }

        if ( classes >= (uint)QTelephony::CallClassData ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassData ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassData;
        }

        if ( classes >= (uint)QTelephony::CallClassVoice ) {
            if ( needsNewLine ) {
                atc->send( "\r\n" );
            }
            atc->send( "+CLCK: 1," + QString::number( QTelephony::CallClassVoice ) );
            needsNewLine = true;
            classes -= (uint)QTelephony::CallClassVoice;
        }

        atc->done();
    }
}

// This slot is called when the QCallBarring instance emits
// the result of an attempt to change the barring password.
// If we attempted to set the barring password, we return
// this result code.
void AtGsmCellCommands::changeBarringPasswordResult( QTelephony::Result result )
{
    if ( settingBarringPassword ) {
        settingBarringPassword = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the QCallSettings instance emits
// the caller Id Restriction status.  If we requested this
// status, we send the solicited result code notification
// and return.
void AtGsmCellCommands::callerIdRestriction( QCallSettings::CallerIdRestriction clir,
        QCallSettings::CallerIdRestrictionStatus status )
{
    if ( requestingCallerIdRestriction ) {
        requestingCallerIdRestriction = false;
        atc->send( "+CLIR: " + QString::number( clir ) + "," + QString::number( status ) );
        atc->done();
    }
}

// This slot is called when the QCallSettings instance emits
// the result of an attempt to set Caller Id restrictions.
// If we attempted to set caller ID restrictions, we return
// this result code.
void AtGsmCellCommands::setCallerIdRestrictionResult( QTelephony::Result result )
{
    if ( settingCallerIdRestriction ) {
        settingCallerIdRestriction = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the CCM.  If we requested the CCM, we send
// the solicited result code notification and return.
// If we did not request the CCM, but have have advice
// of charge reporting set to `on', the +CCCM
// unsolicited result code notification is returned.
void AtGsmCellCommands::currentCallMeter( int value, bool explicitRequest )
{
    if ( requestingCurrentCallMeter && explicitRequest ) {
        requestingCurrentCallMeter = false;
        atc->send( "+CAOC: " + QString::number( value, 16 ) );
        atc->done();
    } else if ( !requestingCurrentCallMeter && !explicitRequest && atc->options()->cccm ) {
        QDateTime currTime = QDateTime::currentDateTime();
        if ( atc->options()->lastTimeCCCM.secsTo( currTime ) > 10 ) {
            atc->send( "+CCCM: " + QString::number( value, 16 ) );
            atc->done();
        }
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the ACM.  If we requested the ACM, we send
// the solicited result code notification and return.
void AtGsmCellCommands::accumulatedCallMeter( int value )
{
    if ( requestingAccumulatedCallMeter ) {
        requestingAccumulatedCallMeter = false;
        atc->send( "+CACM: " + QString::number( value, 16 ) );
        atc->done();
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the result of an attempt to reset the ACM.
// If we attempted to reset the ACM, we return this
// result code.
void AtGsmCellCommands::resetAccumulatedCallMeterResult( QTelephony::Result result )
{
    if ( resettingAccumulatedCallMeter ) {
        resettingAccumulatedCallMeter = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the accumulated call meter maximum.  If we
// requested the ACMM, we send the solicited result
// code notification and return.
void AtGsmCellCommands::accumulatedCallMeterMaximum( int value )
{
    if ( requestingAccumulatedCallMeterMaximum ) {
        requestingAccumulatedCallMeterMaximum = false;
        atc->send( "+CAMM: " + QString::number( value, 16 ) );
        atc->done();
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the result of an attempt to set the accumulated
// call meter maximum.  If we attempted to set it, we
// return this result code.
void AtGsmCellCommands::setAccumulatedCallMeterMaximumResult( QTelephony::Result result )
{
    if ( settingAccumulatedCallMeterMaximum ) {
        settingAccumulatedCallMeterMaximum = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the PPU of calls.  If we requested the PPU, we
// send the solicited result code notification and return.
void AtGsmCellCommands::pricePerUnit( const QString& currency, const QString& unitPrice )
{
    if ( requestingPricePerUnit ) {
        requestingPricePerUnit = false;
        atc->send( "+CPUC: " + currency + "," + unitPrice );
        atc->done();
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits the result of an attempt to set the price per unit
// of calls.  If we attempted to set the PPU, we return
// this result code.
void AtGsmCellCommands::setPricePerUnitResult( QTelephony::Result result )
{
    if ( settingPricePerUnit ) {
        settingPricePerUnit = false;
        atc->done( (QAtResult::ResultCode) result );
    }
}

// This slot is called when the QAdviceOfCharge instance
// emits a callMeterMaximumWarning (30 seconds before
// time runs out, or if a new call is connected and
// less than 30 seconds of paid time remains).
void AtGsmCellCommands::callMeterMaximumWarning()
{
    if ( atc->options()->ccwv ) {
        atc->send( "+CCWV" );
    }
}

// This slot is called when a list of operator names is emitted
// by the QPreferredNetworkOperators instance.  If we requested
// the list of operator names, we return it, after performing
// the necessary formatting.
void AtGsmCellCommands::operatorNames
        (const QList<QPreferredNetworkOperators::NameInfo>& names)
{
    // send the list of operator names.
    if ( requestingOperatorNames ) {
        requestingOperatorNames = false;
        QString status = "+COPN: ";
        int namesSeen = 1;
        QList<QPreferredNetworkOperators::NameInfo>::ConstIterator it;
        for ( it = names.begin(); it != names.end(); ++it, ++namesSeen ) {
            status += QString::number( (*it).id );
            status += ",";
            status += "\"" + (*it).name + "\"";
            if ( namesSeen != names.size() ) {
                status += "\r\n+COPN: ";
            }
        }

        atc->send( status );
        atc->done();
    }
}

// This slot is called when a list of preferred operators
// is emitted by the QPreferredNetworkOperators instance.
// If we requested the list of operators, we return it,
// after performing the necessary formatting.
void AtGsmCellCommands::preferredOperators
        (QPreferredNetworkOperators::List list, const QList<QPreferredNetworkOperators::Info>& opers)
{
    // get the range of index numbers of pref. opers from the sim.
    if ( requestingPreferredOperatorsFromList == list ) {
        requestingPreferredOperatorsFromList = -1;

        if ( testingPreferredOperators ) {
            // must have been a "test" (support) cpol command.
            testingPreferredOperators = false;

            // note : this is the _wrong_ way to do this.  We need a command
            // that returns the entire index range supported by the SIM.
            atc->send( "+CPOL: (1-" + QString::number( opers.size() ) + "),(0-2)" );
            atc->done();
        } else {
            // must have been a "read" (get) command.
            QString status = "+CPOL: ";
            int operatorCount = 1;
            QList<QPreferredNetworkOperators::Info>::ConstIterator it;
            for ( it = opers.begin(); it != opers.end(); ++it ) {
                uint index = (*it).index;
                uint id = (*it).id;
                QString name = (*it).name;

                // first, check that it is a valid entry
                if ( index < 1 || id == 0 )
                    continue;

                // second, if this is a new entry, add a new line.
                if ( operatorCount > 1)
                    status += "\r\n+CPOL: ";

                // then output the data from this entry in the list.
                operatorCount += 1;
                status += QString::number( index );
                status += ",";
                status += QString::number( atc->frontEnd()->options()->cpolFormat );
                status += ",";
                if ( atc->frontEnd()->options()->cpolFormat != 2 ) {
                    if ( name != 0 ) {
                        status += name;
                    }
                } else {
                    if ( id != 0 ) {
                        status += QString::number( id );
                    }
                }
                status += ",";

                if ( (*it).technologies.contains("GSM") ) {
                    status += "1,";
                } else {
                    status += "0,";
                }

                if ( (*it).technologies.contains("GSMCompact") ) {
                    status += "1,";
                } else {
                    status += "0,";
                }

                if ( (*it).technologies.contains("UTRAN") ) {
                    status += "1";
                } else {
                    status += "0";
                }
            }
            atc->send( status );
            atc->done();
        }
    }
}

// This slot is called when the result of an attempt to write
// to a preferred operators list is emitted by the
// QPreferredNetworkOperators instance.  If we had attempted
// to write to a list, then we return this result code.
void AtGsmCellCommands::writePreferredOperatorResult( QTelephony::Result result )
{
    if ( settingPreferredOperator ) {
        settingPreferredOperator = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the result of an attempt to set
// call forwarding is emitted by the QCallForwarding instance.
// If we attempted to set call forwarding, then we return
// this result code.
void AtGsmCellCommands::setForwardingResult( QCallForwarding::Reason reason, QTelephony::Result result )
{
    if ( settingCallForwardingReason == reason ) {
        settingCallForwardingReason = -1;
        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the forwarding status is emitted
// by the QCallForwarding instance.  If the reason that the
// status was emitted corresponds to our request for the
// forwarding status, then we return any solicited
// result code notifications and return.
void AtGsmCellCommands::forwardingStatus( QCallForwarding::Reason reason, const QList<QCallForwarding::Status>& status )
{
    // the mode = 2 (query).
    if ( requestingCallForwardingStatusReason == reason ) {
        requestingCallForwardingStatusReason = -1;
        QString sendStr = "+CCFC: ";
        bool firstEntry = true;
        QList<QCallForwarding::Status>::ConstIterator it;
        for ( it = status.begin(); it != status.end(); ++it ) {
            if ( !firstEntry ) {
                sendStr += "\r\nCCFC: ";
            } else {
                firstEntry = false;
            }

            // at the moment, status is always active
            sendStr += "1,";
            sendStr += QString::number( (uint)((*it).cls) );
            if ( (*it).number != 0) { // check for valid number.
                sendStr += ",";
                sendStr += "\"" + (*it).number + "\"";
                if ( (*it).number.contains('+') ) {
                    sendStr += ",145";
                } else {
                    sendStr += ",129";
                }

                if ( (*it).time != 0 ) {
                    sendStr += ",";
                    sendStr += (*it).time;
                }
            }
        }

        // check to see that we had at least one in the list
        if ( firstEntry ) {
            // no, we didn't.  output default.
            sendStr += QString::number( (uint)QCallForwarding::Unconditional ) + "," +
                    QString::number( (uint)QTelephony::CallClassDefault );
        }

        // output.
        atc->send( sendStr );
        atc->done();
    }
}

// This slot is called when the presentation status of a connected line
// is emitted by the QCallSettings instance.  If we requested the status,
// we send an unsolicited result code notification, and return.
void AtGsmCellCommands::connectedIdPresentation( QCallSettings::PresentationStatus status )
{
    if ( requestingConnectedIdPresentation ) {
        requestingConnectedIdPresentation = false;
        QString sendStr = "+COLP: ";
        if ( atc->frontEnd()->options()->colp ) {
            sendStr += "1,";
        } else {
            sendStr += "0,";
        }

        sendStr += QString::number(status);
        atc->send( sendStr );
        atc->done();
    }
}

// This slot is called when the QSimGenericAccess instance emits
// a response to an attempt to access the SIM.
// If the associated requestID identifies an attempt that we
// have initiated, we send any unsolicited result code notifications
// and return the result code.
void AtGsmCellCommands::simGenericAccessResponse( const QString & reqid, QTelephony::Result result, const QByteArray & data )
{
    if ( reqid == simRequestId ) {
        if ( result == QTelephony::OK ) {
            simRequestId.clear();
            atc->send( "+CSIM: " + QString::number( data.length() ) + "," );
            for ( int i = 0; i < data.length(); i++ ) {
                atc->send( "" + data.at( i ) );
            }
        }

        atc->done( (QAtResult::ResultCode)result );
    }
}

// This slot is called when the QPhoneRf device emits a result code
// due to an attempt to set the level.  If we attempted to set
// the level, we return this result code.
void AtGsmCellCommands::setLevelResult( QTelephony::Result result )
{
    if ( settingPhoneRf ) {
        settingPhoneRf = false;
        atc->done( (QAtResult::ResultCode)result );
    }
}


void AtGsmCellCommands::phoneBookLimits( const QString& /*store*/, const QPhoneBookLimits& limits )
{
    if (!phoneBookQueried)
        return;
    phoneBookQueried = false;
    QString status;
    if ( limitsReqBy == "cpbs" )
    {
        status += "+CPBS: ";
        status += "\"" + atc->options()->phoneStore + "\",";
        status += QString::number(limits.used())+","
            +QString::number(limits.lastIndex()-limits.firstIndex()+1);
        atc->send( status );
        atc->done();
    }
    else if ( limitsReqBy == "cpbrSupport" )
    {
        status = "+CPBR: ";
        status += "("+QString::number(limits.firstIndex()) + "," +
                      QString::number(limits.lastIndex())+")," +
                      QString::number(limits.numberLength()) + "," +
                      QString::number(limits.textLength());
        atc->send( status );
        atc->done();
    }
    else if ( limitsReqBy == "cpbrSet" )
    {
        //get limits so that we can check for invalid indices
        if ( limits.firstIndex() > phoneBookIndex.first ||
                limits.lastIndex() < phoneBookIndex.second )
        {
           atc->done( QAtResult::InvalidIndex );
           return;
        }
        phoneBookQueried = true;
        phoneBook->getEntries( atc->options()->phoneStore );
        return;
    }
    else if ( limitsReqBy == "cpbwSupport" )
    {
        status = "+CPBW: (%1-%2),%3,(129,145),%4";
        status = status.arg(limits.firstIndex()).arg(limits.lastIndex());
        status = status.arg(limits.numberLength()).arg(limits.textLength());
        atc->send( status );
        atc->done();
    }
    else if ( limitsReqBy == "cpbwSet-Remove" )
    {
        if ( entryToWrite.index() < limits.firstIndex()
                || entryToWrite.index() > limits.lastIndex() ) {
            atc->done( QAtResult::InvalidIndex );
            return;
        }
        phoneBook->remove( entryToWrite.index(), atc->options()->phoneStore );
        atc->done();
    }
    else if ( limitsReqBy == "cpbwSet-Number@Index"
            || limitsReqBy == "cpbwSet-NumberText@Index" )
    {
        if ( entryToWrite.index() < limits.firstIndex()
                || entryToWrite.index() > limits.lastIndex() ) {
            atc->done( QAtResult::InvalidIndex );
            return;
        }

        if ( (uint)entryToWrite.number().length() > limits.numberLength() ) {
            atc->done( QAtResult::DialStringTooLong );
            return;
        } else if ( (uint)entryToWrite.text().length() > limits.textLength() ) {
            atc->done( QAtResult::TextStringTooLong );
            return;
        }
        phoneBook->update( entryToWrite, atc->options()->phoneStore );
        atc->done();
    }
    else if ( limitsReqBy == "cpbwSet-Number@nextIndex" )
    {
        if ( (uint)entryToWrite.number().length() > limits.numberLength() ) {
            atc->done( QAtResult::DialStringTooLong );
            return;
        } else if ( (uint)entryToWrite.text().length() > limits.textLength() ) {
            atc->done( QAtResult::TextStringTooLong );
            return;
        }
        phoneBook->add( entryToWrite, atc->options()->phoneStore );
        atc->done();
    }
    else if ( limitsReqBy == "cpbfSupport" )
    {
        status = "+CPBF: %1,%2";
        status = status.arg(limits.numberLength()).arg(limits.textLength());
        atc->send( status );
        atc->done();
    }
}

/*!
   \internal

   Read/search the phone book entries of non memory phone storages (e.g. SM).
   */
void AtGsmCellCommands::phoneBookEntries( const QString& /*store*/, const QList<QPhoneBookEntry>& entries )
{
    if (!phoneBookQueried)
        return;
    phoneBookQueried = false;

    QString prefix;
    bool isSearch = false;
    bool hasEntry = false; //have we found a matching entry for +CPBF
    if ( limitsReqBy == "cpbfSet" ) {
        prefix = "+CPBF: ";
        isSearch = true;
    } else if ( limitsReqBy == "cpbrSet" ) {
        prefix = "+CPBR: ";
    }


    //entries are not always sorted but all modems out there seem to
    //return the results in a sorted order
    QMap<int,QPhoneBookEntry> sortedEntries;
    for ( int i = 0; i<entries.count(); i++ ) {
        sortedEntries.insert( entries[i].index(), entries[i] );
    }

    QMap<int,QPhoneBookEntry>::const_iterator iter;
    for ( iter = sortedEntries.constBegin();
            iter != sortedEntries.constEnd(); iter++ )
    {
        if ( !isSearch &&
                ((*iter).index() < phoneBookIndex.first
                 || (*iter).index() > phoneBookIndex.second )
           )
            continue;
        else if ( isSearch )
            if ((*iter).text().startsWith( pbSearchText, Qt::CaseInsensitive ) )
                hasEntry = true;
            else
                continue;


        QString status = prefix + "%1,\"%2\",%3,\"%4\"";
        status = status.arg( (*iter).index() );
        status = status.arg( (*iter).number() );
        if ( (*iter).number().contains( "+" ) )
            status = status.arg(145);
        else
            status = status.arg( 129 );
        status = status.arg( QAtUtils::quote( QString::fromLatin1( atc->options()->codec->fromUnicode( ((*iter).text()) ) ) ) );//use +CSCS encoding
        atc->send( status );
    }

    if ( isSearch && !hasEntry )
        atc->done( QAtResult::NotFound );
    else
        atc->done();
}

void AtGsmCellCommands::initializeMemoryPhoneBook()
{
    if ( contactModel )
        return;

    QSet<QContact::PhoneType> typeRef;
    contactModel = new QContactModel( this );
    contactIndices.reserve( contactModel->rowCount() + 100 );
    //build ContactToIndex mapping for this session
    int index = 1;
    QUniqueId id;
    for ( int i = 0; i<contactModel->rowCount(); i++ ) {
        id = contactModel->id( i );
        if ( !contactModel->isSimCardContact( id ) ) {
            QContact c = contactModel->contact( id );
            QMap<QContact::PhoneType,QString> numbers = c.phoneNumbers();
            QList<QContact::PhoneType> types = numbers.keys();
            QList<QContact::PhoneType>::const_iterator iter;
            for ( iter = types.constBegin() ;iter != types.constEnd(); iter++ ) {
                //don't show pager or fax
                if ( (*iter) & QContact::Fax  || (*iter) & QContact::Pager )
                    continue;

                ContactRecord store;
                store.type = *iter;
                store.id = id;
                contactIndices.insert( index++, store );
            }
        }

    }
    //contactmodel doesn't really have entry limit
    //set artifical limits for the sake of at+cpbs
    availableMemoryContacts = qMin( availableMemoryContacts,
                index-(index%50)+100 );

}

void AtGsmCellCommands::writeMemoryPhoneBookEntry( bool isDeletion, uint index, const QString& number, const QString& text )
{
    if ( index < 1 || (int) index > availableMemoryContacts ) {
        atc->done( QAtResult::InvalidIndex );
        return;
    }
    if ( number.length() > 60 ) {
        atc->done( QAtResult::DialStringTooLong );
        return;
    }

    if ( text.length() > 60 ) {
        atc->done( QAtResult::TextStringTooLong );
        return;
    }

    //index already used by other contact
    if ( contactIndices.contains( index ) ) {
        QContact c = contactModel->contact(
                contactIndices.value( index ).id );

        QString name = c.label();
        name = name.left( 60 ); //we display the first 60 chars only

        if ( isDeletion ) {
            //delete entry at given index
            c.setPhoneNumber( contactIndices.value(index).type, QString() );
            contactModel->updateContact( c );
            contactIndices.remove( index );
        } else if ( QString::compare(name, text, Qt::CaseInsensitive) == 0 ) {
            //same name -> replace phone number
            c.setPhoneNumber( contactIndices.value(index).type, number );
            contactModel->updateContact( c );
        } else {
            //not same name
            //remove old phone number
            c.setPhoneNumber( contactIndices.value(index).type, QString() );
            contactModel->updateContact( c );
            contactIndices.remove( index );
            //add new contact
            QContact newContact;
            newContact.setPhoneNumber( QContact::HomePhone, number );
            if ( !text.isEmpty() ) {
                int space = text.indexOf( QChar(' ') );
                if ( space == -1 ) {  //not found
                    newContact.setFirstName( text );
                } else {
                    newContact.setFirstName( text.left( space ) );
                    newContact.setLastName( text.mid( space+1 ) );
                }
            }
            ContactRecord store;
            store.type = QContact::HomePhone;
            store.id = contactModel->addContact( newContact );
            if ( store.id.isNull() )
                qLog(ModemEmulator) << "Cannot add new memory contact.";
            else
                contactIndices.insert( index, store );
        }
    } else {
        QContact newContact;
        newContact.setPhoneNumber( QContact::HomePhone, number );
        if ( !text.isEmpty() ) {
            int space = text.indexOf( QChar(' ') );
            if ( space == -1 ) {  //not found
                newContact.setFirstName( text );
            } else {
                newContact.setFirstName( text.left( space ) );
                newContact.setLastName( text.mid( space+1 ) );
            }
        }
        ContactRecord store;
        store.type = QContact::HomePhone;
        store.id = contactModel->addContact( newContact );
        if ( store.id.isNull() )
            qLog(ModemEmulator) << "Cannot add new memory contact.";
        else
            contactIndices.insert( index, store );

    }
    atc->done();
}


