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

#include <qatresult.h>

/*!
    \class QAtResult
    \inpublicgroup QtBaseModule

    \brief The QAtResult class provides access to the results of AT modem commands and unsolicited notifications
    \ingroup telephony::serial

    AT commands that are sent to a modem with QAtChat::chat() result in a QAtResult
    object being made available to describe the result of the command when it completes.

    The resultCode() method can be used to determine the exact cause of an AT
    modem command failure.  The content() method can be used to access the
    response content that was returned from the command.  For complex response
    contents, the QAtResultParser class can be used to decode the response.

    \sa QAtChat, QAtResultParser
*/

class QAtResultPrivate
{
public:
    QAtResultPrivate()
    {
        result = "OK";
        resultCode = QAtResult::OK;
        verbose = true;
        userData = 0;
    }
    ~QAtResultPrivate()
    {
        if ( userData )
            delete userData;
    }

    QString result;
    QString content;
    QAtResult::ResultCode resultCode;
    bool verbose;
    QAtResult::UserData *userData;
};

/*!
    \enum QAtResult::ResultCode
    Result codes for AT modem commands.

    \value OK Command responded with \c{OK}
    \value Connect Command responded with \c{CONNECT}
    \value NoCarrier Command responded with \c{NO CARRIER}
    \value Error Command responded with \c{ERROR}
    \value NoDialtone Command responded with \c{NO DIALTONE}
    \value Busy Command responded with \c{BUSY}
    \value NoAnswer Command responded with \c{NO ANSWER}
    \value Dead The link is dead
    \value PhoneFailure GSM 27.07 code 0: phone failure
    \value NoConnectionToPhone GSM 27.07 code 1: no connection to phone
    \value PhoneAdapterLinkReserved GSM 27.07 code 2: phone-adaptor link reserved
    \value OperationNotAllowed GSM 27.07 code 3: operation not allowed
    \value OperationNotSupported GSM 27.07 code 4: operation not supported
    \value PhSimPinRequired GSM 27.07 code 5: PH-SIM PIN required
    \value PhFSimPinRequired GSM 27.07 code 6: PH-FSIM PIN required
    \value PhFSimPukRequired GSM 27.07 code 7: PH-FSIM PUK required
    \value SimNotInserted GSM 27.07 code 10: SIM not inserted
    \value SimPinRequired GSM 27.07 code 11: SIM PIN required
    \value SimPukRequired GSM 27.07 code 12: SIM PUK required
    \value SimFailure GSM 27.07 code 13: SIM failure
    \value SimBusy GSM 27.07 code 14: SIM busy
    \value SimWrong GSM 27.07 code 15: SIM wrong
    \value IncorrectPassword GSM 27.07 code 16: incorrect password
    \value SimPin2Required GSM 27.07 code 17: SIM PIN2 required
    \value SimPuk2Required GSM 27.07 code 18: SIM PUK2 required
    \value MemoryFull GSM 27.07 code 20: memory full
    \value InvalidIndex GSM 27.07 code 21: invalid index
    \value NotFound GSM 27.07 code 22: not found
    \value MemoryFailure GSM 27.07 code 23: memory failure
    \value TextStringTooLong GSM 27.07 code 24: text string too long
    \value InvalidCharsInTextString GSM 27.07 code 25: invalid characters in text string
    \value DialStringTooLong GSM 27.07 code 26: dial string too long
    \value InvalidCharsInDialString GSM 27.07 code 27: invalid characters in dial string
    \value NoNetworkService GSM 27.07 code 30: no network service
    \value NetworkTimeout GSM 27.07 code 31: network timeout
    \value NetworkNotAllowed GSM 27.07 code 32: network not allowed - emergency calls only
    \value NetPersPinRequired GSM 27.07 code 40: network personalization PIN required
    \value NetPersPukRequired GSM 27.07 code 41: network personalization PUK required
    \value NetSubsetPersPinRequired GSM 27.07 code 42: network subset personalization PIN required
    \value NetSubsetPersPukRequired GSM 27.07 code 43: network subset personalization PUK required
    \value ServProvPersPinRequired GSM 27.07 code 44: service provider personalization PIN required
    \value ServProvPersPukRequired GSM 27.07 code 45: service provider personalization PUK required
    \value CorpPersPinRequired GSM 27.07 code 46: corporate personalization PIN required
    \value CorpPersPukRequired GSM 27.07 code 47: corporate personalization PUK required
    \value HiddenKeyRequired GSM 27.07 code 48: hidden key required
    \value Unknown GSM 27.07 code 100: unknown
    \value IllegalMS GSM 27.07 code 103: Illegal MS
    \value IllegalME GSM 27.07 code 106: Illegal ME
    \value GPRSServicesNotAllowed GSM 27.07 code 107: GPRS services not allowed
    \value PLMNNotAllowed GSM 27.07 code 111: PLMN not allowed
    \value LocationAreaNotAllowed GSM 27.07 code 112: Location area not allowed
    \value RoamingNotAllowed GSM 27.07 code 113: Roaming not allowed in this location area
    \value ServiceOptionNotSupported GSM 27.07 code 132: service option not supported
    \value ServiceOptionNotSubscribed GSM 27.07 code 133: service option not subscribed
    \value ServiceOptionOutOfOrder GSM 27.07 code 134: service option temporarily out of order
    \value UnspecifiedGPRSError GSM 27.07 code 148: unspecified GPRS error
    \value PDPAuthenticationFailure GSM 27.07 code 149: PDP authentication failure
    \value InvalidMobileClass GSM 27.07 code 150: invalid mobile class
    \value VBSVGCSNotSupported GSM 27.07 code 151: VBS/VGCS not supported by the network
    \value NoServiceSubscriptionOnSim GSM 27.07 code 152: No service subscription on SIM
    \value NoSubscriptionForGroupId GSM 27.07 code 153: No subscription for group ID
    \value GroupIdNotActivatedOnSim GSM 27.07 code 154: Group Id not activated on SIM
    \value NoMatchingNotification GSM 27.07 code 155: No matching notification
    \value VBSVGCSCallAlreadyPresent GSM 27.07 code 156: VBS/VGCS call already present
    \value Congestion GSM 27.07 code 157: Congestion
    \value NetworkFailure GSM 27.07 code 158: Network failure
    \value UplinkBusy GSM 27.07 code 159: Uplink busy
    \value NoAccessRightsForSimFile GSM 27.07 code 160: No access rights for SIM file
    \value NoSubscriptionForPriority GSM 27.07 code 161: No subscription for priority
    \value OperationNotApplicable GSM 27.07 code 162: operation not applicable or not possible
    \value MEFailure GSM 27.05 code 300: ME failure
    \value SMSServiceOfMEReserved GSM 27.05 code 301: SMS service of ME reserved
    \value SMSOperationNotAllowed GSM 27.05 code 302: operation not allowed
    \value SMSOperationNotSupported GSM 27.05 code 303: operation not supported
    \value InvalidPDUModeParameter GSM 27.05 code 304: invalid PDU mode parameter
    \value InvalidTextModeParameter GSM 27.05 code 305: invalid text mode parameter
    \value USimNotInserted GSM 27.05 code 310: (U)SIM not inserted
    \value USimPinRequired GSM 27.05 code 311: (U)SIM PIN required
    \value PHUSimPinRequired GSM 27.05 code 312: PH-(U)SIM PIN required
    \value USimFailure GSM 27.05 code 313: (U)SIM failure
    \value USimBusy GSM 27.05 code 314: (U)SIM busy
    \value USimWrong GSM 27.05 code 315: (U)SIM wrong
    \value USimPukRequired GSM 27.05 code 316: (U)SIM PUK required
    \value USimPin2Required GSM 27.05 code 317: (U)SIM PIN2 required
    \value USimPuk2Required GSM 27.05 code 318: (U)SIM PUK2 required
    \value SMSMemoryFailure GSM 27.05 code 320: memory failure
    \value InvalidMemoryIndex GSM 27.05 code 321: invalid memory index
    \value SMSMemoryFull GSM 27.05 code 322: memory full
    \value SMSCAddressUnknown GSM 27.05 code 330: SMSC address unknown
    \value SMSNoNetworkService GSM 27.05 code 331: no network service
    \value SMSNetworkTimeout GSM 27.05 code 332: network timeout
    \value NoCNMAAckExpected GSM 27.05 code 340: no +CNMA acknowledgment expected
    \value UnknownError GSM 27.05 code 500: unknown error
*/

/*!
    \class QAtResult::UserData
    \inpublicgroup QtBaseModule
    \ingroup telephony::serial

    \brief The UserData class provides a mechanism to add user data to a result object via QAtChat::chat().

    Usually this class will be inherited, with extra fields added to hold the user data.

    \sa QAtChat::chat()
*/

/*!
    \fn QAtResult::UserData::~UserData()

    Destructs a user data object.
*/

/*!
    Construct a new QAtResult object.  The result() will be \c{OK},
    and the content() empty.
*/
QAtResult::QAtResult()
{
    d = new QAtResultPrivate();
}

/*!
    Construct a copy of \a other.
*/
QAtResult::QAtResult( const QAtResult& other )
{
    d = new QAtResultPrivate();
    *this = other;
}

/*!
    Destruct this QAtResult object.
*/
QAtResult::~QAtResult()
{
    delete d;
}

/*!
    Assign the contents of \a other to this object.
*/
QAtResult& QAtResult::operator=( const QAtResult& other )
{
    if ( this != &other ) {
        d->result = other.d->result;
        d->content = other.d->content;
        d->resultCode = other.d->resultCode;
    }
    return *this;
}

/*!
    Returns the result line that terminated the command's response.
    This is usually a string such as \c{OK}, \c{ERROR}, \c{+CME ERROR: N},
    and so on.

    The resultCode() function is a better way to determine why a command
    failed, but sometimes it is necessary to parse the text result line.
    For example, for \c{CONNECT baudrate}, the caller may be interested in
    the baud rate.

    \sa resultCode(), ok()
*/
QString QAtResult::result() const
{
    return d->result;
}

/*!
    Sets the result line that terminated the command's response to \a value.
    This will also update resultCode() to reflect the appropriate code.

    \sa result(), resultCode()
*/
void QAtResult::setResult( const QString& value )
{
    d->result = value;
    resultToCode( value );
}

/*!
    Returns the content that was returned with an AT command's result.

    \sa setContent(), append()
*/
QString QAtResult::content() const
{
    return d->content;
}

/*!
    Sets the content that was returned with an AT command's result to \a value.

    \sa content(), append()
*/
void QAtResult::setContent( const QString& value )
{
    d->content = value;
}

/*!
    Append \a value to the current content, after a line terminator.

    \sa content(), setContent()
*/
void QAtResult::append( const QString& value )
{
    if ( d->content.isEmpty() )
        d->content = value;
    else
        d->content = d->content + "\n" + value;
}

/*!
    Returns the numeric result code associated with result().

    Extended error codes are only possible if the appropriate modem error
    mode has been enabled (e.g. with the \c{AT+CMEE=1} command).  Otherwise
    most errors will simply be reported as QAtResult::Error.

    \sa setResultCode(), result(), ok()
*/
QAtResult::ResultCode QAtResult::resultCode() const
{
    return d->resultCode;
}

/*!
    Sets the numeric result code to \a value, and update result()
    to reflect the value.

    \sa resultCode(), result()
*/
void QAtResult::setResultCode( QAtResult::ResultCode value )
{
    d->resultCode = value;
    d->result = codeToResult( QString() );
    d->verbose = true;
}

/*!
    Returns true if this result indicates a successful command; false otherwise.
    Success is indicated when resultCode() returns either
    QAtResult::OK or QAtResult::Connect.  All other result
    codes indicate failure.

    \sa resultCode()
*/
bool QAtResult::ok() const
{
    return ( d->resultCode == QAtResult::OK ||
             d->resultCode == QAtResult::Connect );
}

/*!
    Returns a more verbose version of result(), suitable for debug output.
    Many modems report extended errors by number (e.g. \c{+CME ERROR: 4}),
    which can be difficult to use when diagnosing problems.  This function
    returns a string that is more suitable for diagnostic output than result().
    If result() is already verbose, it will be returned as-is.

    \sa result()
*/
QString QAtResult::verboseResult() const
{
    if ( d->verbose )
        return d->result;
    else
        return codeToResult( d->result );
}

/*!
    Returns the user data associated with this result object.

    \sa setUserData()
*/
QAtResult::UserData *QAtResult::userData() const
{
    return d->userData;
}

/*!
    Sets the user data associated with this result object to \a value.

    \sa userData()
*/
void QAtResult::setUserData( QAtResult::UserData *value )
{
    if ( d->userData && d->userData != value )
        delete d->userData;
    d->userData = value;
}

// Table of result codes.  Note: these strings are not translatable,
// as they are used to parse values on the wire.
struct QAtCodeInfo
{
    QAtResult::ResultCode   code;
    const char             *name;
};
static QAtCodeInfo const basic_codes[] = {
    {QAtResult::OK,                         "OK"},      // no tr
    {QAtResult::OK,                         "0"},       // no tr
    {QAtResult::Connect,                    "CONNECT"},     // no tr
    {QAtResult::Connect,                    "1"},       // no tr
    {QAtResult::NoCarrier,                  "NO CARRIER"},      // no tr
    {QAtResult::NoCarrier,                  "3"},       // no tr
    {QAtResult::Error,                      "ERROR"},       // no tr
    {QAtResult::Error,                      "4"},       // no tr
    {QAtResult::NoDialtone,                 "NO DIALTONE"},     // no tr
    {QAtResult::NoDialtone,                 "6"},       // no tr
    {QAtResult::Busy,                       "BUSY"},        // no tr
    {QAtResult::Busy,                       "7"},       // no tr
    {QAtResult::NoAnswer,                   "NO ANSWER"},       // no tr
    {QAtResult::NoAnswer,                   "8"},        // no tr
    {QAtResult::OK,                         "VCON"}        // no tr
};
static QAtCodeInfo const ext_codes[] = {
    {QAtResult::PhoneFailure,               "phone failure"},       // no tr
    {QAtResult::NoConnectionToPhone,        "no connection to phone"},      // no tr
    {QAtResult::PhoneAdapterLinkReserved,   "phone-adaptor link reserved"},     // no tr
    {QAtResult::OperationNotAllowed,        "operation not allowed"},       // no tr
    {QAtResult::OperationNotSupported,      "operation not supported"},     // no tr
    {QAtResult::PhSimPinRequired,           "PH-SIM PIN required"},     // no tr
    {QAtResult::PhFSimPinRequired,          "PH-FSIM PIN required"},        // no tr
    {QAtResult::PhFSimPukRequired,          "PH-FSIM PUK required"},        // no tr
    {QAtResult::SimNotInserted,             "SIM not inserted"},        // no tr
    {QAtResult::SimPinRequired,             "SIM PIN required"},        // no tr
    {QAtResult::SimPukRequired,             "SIM PUK required"},        // no tr
    {QAtResult::SimFailure,                 "SIM failure"},     // no tr
    {QAtResult::SimBusy,                    "SIM busy"},        // no tr
    {QAtResult::SimWrong,                   "SIM wrong"},       // no tr
    {QAtResult::IncorrectPassword,          "incorrect password"},      // no tr
    {QAtResult::SimPin2Required,            "SIM PIN2 required"},       // no tr
    {QAtResult::SimPuk2Required,            "SIM PUK2 required"},       // no tr
    {QAtResult::MemoryFull,                 "memory full"},     // no tr
    {QAtResult::InvalidIndex,               "invalid index"},       // no tr
    {QAtResult::NotFound,                   "not found"},   // no tr
    {QAtResult::MemoryFailure,              "memory failure"},  // no tr
    {QAtResult::TextStringTooLong,          "text string too long"},    // no tr
    {QAtResult::InvalidCharsInTextString,   "invalid characters in text string"},   // no tr
    {QAtResult::DialStringTooLong,          "dial string too long"},    // no tr
    {QAtResult::InvalidCharsInDialString,   "invalid characters in dial string"},   // no tr
    {QAtResult::NoNetworkService,           "no network service"},  // no tr
    {QAtResult::NetworkTimeout,             "network timeout"}, // no tr
    {QAtResult::NetworkNotAllowed,          "network not allowed - emergency calls only"},  // no tr
    {QAtResult::NetPersPinRequired,         "network personalization PIN required"},    // no tr
    {QAtResult::NetPersPukRequired,         "network personalization PUK required"},    // no tr
    {QAtResult::NetSubsetPersPinRequired,   "network subset personalization PIN required"}, // no tr
    {QAtResult::NetSubsetPersPukRequired,   "network subset personalization PUK required"}, // no tr
    {QAtResult::ServProvPersPinRequired,    "service provider personalization PIN required"},   // no tr
    {QAtResult::ServProvPersPukRequired,    "service provider personalization PUK required"},   // no tr
    {QAtResult::CorpPersPinRequired,        "corporate personalization PIN required"},  // no tr
    {QAtResult::CorpPersPukRequired,        "corporate personalization PUK required"},  // no tr
    {QAtResult::HiddenKeyRequired,          "hidden key required"}, // no tr
    {QAtResult::Unknown,                    "unknown"}, // no tr

    {QAtResult::IllegalMS,                  "Illegal MS"},  // no tr
    {QAtResult::IllegalME,                  "Illegal ME"},  // no tr
    {QAtResult::GPRSServicesNotAllowed  ,   "GPRS services not allowed"},   // no tr
    {QAtResult::PLMNNotAllowed,             "PLMN not allowed"},    // no tr
    {QAtResult::LocationAreaNotAllowed,     "Location area not allowed"},   // no tr
    {QAtResult::RoamingNotAllowed,          "Roaming not allowed in this location area"},   // no tr
    {QAtResult::ServiceOptionNotSupported,  "service option not supported"},    // no tr
    {QAtResult::ServiceOptionNotSubscribed, "requested service option not subscribed"}, // no tr
    {QAtResult::ServiceOptionOutOfOrder,    "service option temporarily out of order"}, // no tr
    {QAtResult::UnspecifiedGPRSError,       "unspecified GPRS error"},  // no tr
    {QAtResult::PDPAuthenticationFailure,   "PDP authentication failure"},  // no tr
    {QAtResult::InvalidMobileClass,         "invalid mobile class"},    // no tr
    // no tr
    {QAtResult::VBSVGCSNotSupported,        "VBS/VGCS not supported by the network"},   // no tr
    {QAtResult::NoServiceSubscriptionOnSim, "No service subscription on SIM"},  // no tr
    {QAtResult::NoSubscriptionForGroupId,   "No subscription for group ID"},    // no tr
    {QAtResult::GroupIdNotActivatedOnSim,   "Group Id not activated on SIM"},   // no tr
    {QAtResult::NoMatchingNotification,     "No matching notification"},    // no tr
    {QAtResult::VBSVGCSCallAlreadyPresent,  "VBS/VGCS call already present"},   // no tr
    {QAtResult::Congestion,                 "Congestion"},  // no tr
    {QAtResult::NetworkFailure,             "Network failure"}, // no tr
    {QAtResult::UplinkBusy,                 "Uplink busy"}, // no tr
    {QAtResult::NoAccessRightsForSimFile,   "No access rights for SIM file"},   // no tr
    {QAtResult::NoSubscriptionForPriority,  "No subscription for priority"},    // no tr
    {QAtResult::OperationNotApplicable,     "operation not applicable or not possible"},    // no tr

    {QAtResult::MEFailure,                  "ME failure"},  // no tr
    {QAtResult::SMSServiceOfMEReserved,     "SMS service of ME reserved"},  // no tr
    {QAtResult::SMSOperationNotAllowed,     "operation not allowed"},   // no tr
    {QAtResult::SMSOperationNotSupported,   "operation not supported"}, // no tr
    {QAtResult::InvalidPDUModeParameter,    "invalid PDU mode parameter"},  // no tr
    {QAtResult::InvalidTextModeParameter,   "invalid text mode parameter"}, // no tr
    {QAtResult::USimNotInserted,            "(U)SIM not inserted"}, // no tr
    {QAtResult::USimPinRequired,            "(U)SIM PIN required"}, // no tr
    {QAtResult::PHUSimPinRequired,          "PH-(U)SIM PIN required"},  // no tr
    {QAtResult::USimFailure,                "(U)SIM failure"},  // no tr
    {QAtResult::USimBusy,                   "(U)SIM busy"}, // no tr
    {QAtResult::USimWrong,                  "(U)SIM wrong"},    // no tr
    {QAtResult::USimPukRequired,            "(U)SIM PUK required"}, // no tr
    {QAtResult::USimPin2Required,           "(U)SIM PIN2 required"},    // no tr
    {QAtResult::USimPuk2Required,           "(U)SIM PUK2 required"},    // no tr
    {QAtResult::SMSMemoryFailure,           "memory failure"},  // no tr
    {QAtResult::InvalidMemoryIndex,         "invalid memory index"},    // no tr
    {QAtResult::MemoryFull,                 "memory full"}, // no tr
    {QAtResult::SMSCAddressUnknown,         "SMSC address unknown"},    // no tr
    {QAtResult::SMSNoNetworkService,        "no network service"},  // no tr
    {QAtResult::SMSNetworkTimeout,          "network timeout"}, // no tr
    {QAtResult::NoCNMAAckExpected,          "no +CNMA acknowledgement expected"},   // no tr
    {QAtResult::UnknownError,               "unknown error"}    // no tr
};
#define num_basic_codes   ((int)( sizeof(basic_codes) / sizeof(QAtCodeInfo) ))
#define num_ext_codes     ((int)( sizeof(ext_codes) / sizeof(QAtCodeInfo) ))

// Extract a numeric error code from the front of "value", or -1 if not.
static int numeric( const QString& value )
{
    int posn = 0;
    if ( posn >= value.length() || value[posn] < '0' || value[posn] > '9' )
        return -1;
    int number = 0;
    while ( posn < value.length() && value[posn] >= '0' && value[posn] <= '9' )
        number = number * 10 + (int)(value[posn++].unicode() - '0');
    return number;
}

// Determine if we have a prefix match, ignoring case.
static bool match( const QString& value, const char *prefix )
{
    int posn = 0;
    while ( posn < value.length() && *prefix != '\0' ) {
        int ch1 = value[posn++].unicode();
        int ch2 = *prefix++;
        if ( ch1 >= 'A' && ch1 <= 'Z' )
            ch1 = ch1 - 'A' + 'a';
        if ( ch2 >= 'A' && ch2 <= 'Z' )
            ch2 = ch2 - 'A' + 'a';
        if ( ch1 != ch2 )
            return false;
    }
    if ( *prefix != '\0' )
        return false;
    return ( posn >= value.length() || value[posn] == ' ' );
}

void QAtResult::resultToCode( const QString& value )
{
    QString val;
    int index;

    // Determine what kind of error report we have.
    if ( value.startsWith( "+CME ERROR:", Qt::CaseInsensitive ) ||
         value.startsWith( "+EXT ERROR:", Qt::CaseInsensitive ) ) {

        // Extended or GPRS error report.
        val = value.mid( 11 ).trimmed();
        index = numeric( val );
        if ( index >= 0 ) {
            d->resultCode = (QAtResult::ResultCode)index;
            d->verbose = false;
            return;
        }

    } else if ( value.startsWith( "+CMS ERROR:", Qt::CaseInsensitive ) ) {

        // Check the SMS codes before the main codes, as there is
        // some overlap in the message names.
        val = value.mid( 11 ).trimmed();
        index = numeric( val );
        if ( index >= 0 ) {
            d->resultCode = (QAtResult::ResultCode)index;
            d->verbose = false;
            return;
        }
        index = 0;
        while ( ext_codes[index].code != QAtResult::MEFailure )
            ++index;
        while ( index < num_ext_codes ) {
            if ( match( val, ext_codes[index].name ) ) {
                d->resultCode = ext_codes[index].code;
                d->verbose = true;
                return;
            }
            ++index;
        }

    } else {

        // Probably something like OK, ERROR, etc.  Scan the basic codes only.
        for ( index = 0; index < num_basic_codes; ++index ) {
            if ( match( value, basic_codes[index].name ) ) {
                d->resultCode = basic_codes[index].code;
                d->verbose = true;
                return;
            }
        }
        d->resultCode = QAtResult::UnknownError;
        d->verbose = true;

    }

    // Scan the extended code list for a match.
    for ( index = 0; index < num_ext_codes; ++index ) {
        if ( match( val, ext_codes[index].name ) ) {
            d->resultCode = ext_codes[index].code;
            d->verbose = true;
            return;
        }
    }
    d->resultCode = QAtResult::UnknownError;
    d->verbose = true;
}

QString QAtResult::codeToResult( const QString& defaultValue ) const
{
    int index;
    for ( index = 0; index < num_basic_codes; ++index ) {
        if ( basic_codes[index].code == d->resultCode ) {
            return basic_codes[index].name;
        }
    }
    for ( index = 0; index < num_ext_codes; ++index ) {
        if ( ext_codes[index].code == d->resultCode ) {
            if ( d->resultCode >= 300 && d->resultCode <= 500 )
                return QString( "+CMS ERROR: " ) + ext_codes[index].name;
            else
                return QString( "+CME ERROR: " ) + ext_codes[index].name;
        }
    }
    if ( defaultValue.isEmpty() ) {
        if ( ((int)d->resultCode) >= 300 &&
             ((int)d->resultCode) <= 500 ) {
            return "+CMS ERROR: " + QString::number( d->resultCode );
        } else {
            return "+CME ERROR: " + QString::number( d->resultCode );
        }
    } else {
        return defaultValue;
    }
}
