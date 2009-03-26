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

#include <qtelephonynamespace.h>

/*!
  \class QTelephony
    \inpublicgroup QtTelephonyModule

  \brief The QTelephony namespace contains telephony-related enumerated type definitions.

  \ingroup telephony
*/

/*!
    \enum QTelephony::RegistrationState
    Indicates the current network registration state.

    \value RegistrationNone Not registered and not searching for a new operator to register to.
    \value RegistrationHome Registered to the user's home network.
    \value RegistrationSearching Not registered, but currently searching for a new operator to register to.
    \value RegistrationDenied Registration was denied, usually because of invalid access credentials.
    \value RegistrationUnknown Registered, but it is not known whether the network is home or roaming.
    \value RegistrationRoaming Registered to a roaming network.
*/

/*!
    \enum QTelephony::OperatorMode
    Mode that was requested when the operator was selected.

    \value OperatorModeAutomatic Select any operator that is available.
    \value OperatorModeManual Select a particular operator by name.
    \value OperatorModeDeregister Deregister from the network.
    \value OperatorModeManualAutomatic Try to select the user's preferred
           operator by name, and if that fails select any operator that
           is available.
*/

/*!
    \enum QTelephony::OperatorAvailability
    Availability of a network operator within
    QNetworkRegistration::AvailableOperator.

    \value OperatorUnavailable Network operator is unavailable for use.
    \value OperatorAvailable Network operator is available for use.
    \value OperatorCurrent Network operator is currently being used.
    \value OperatorForbidden Network operator is forbidden for use.
*/

/*!
    \enum QTelephony::CallClass
    Class of call to effect with a call forwarding, call barring,
    or call waiting change.

    \value CallClassNone No call class specified.
    \value CallClassVoice Voice calls.
    \value CallClassData Data calls, all bearer services.
    \value CallClassFax Fax calls.
    \value CallClassSMS Short message service.
    \value CallClassDataCircuitSync Data calls, circuit-switched
           synchronous only.
    \value CallClassDataCircuitAsync Data calls, circuit-switched
           asynchronous only.
    \value CallClassDedicatedPacketAccess Data calls, dedicated packet access
           only.
    \value CallClassDedicatedPADAccess Data calls, dedicated PAD access only.
    \value CallClassDefault Default call class; includes CallClassVoice,
           CallClassData, and CallClassFax.
*/

/*!
    \enum QTelephony::Result
    Result codes for telephony operations, based on the GSM specifications.

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
    \enum QTelephony::SimFileType
    Type of files within a SIM that are accessible via QSimFiles.

    \value SimFileRootDirectory Root directory on the SIM (also known
           as the \i{master file}}.
    \value SimFileDirectory Non-root directory on the SIM (also known
           as a \i{dedicated file}).
    \value SimFileTransparent Transparent elementary file (binary).
    \value SimFileLinearFixed Linear fixed elementary file (record-based).
    \value SimFileCyclic Cyclic elementary file (record-based with cyclic
           reuse of old records).
    \value SimFileUnknown Unknown file type.
*/

/*!
    \enum QTelephony::SimFileError
    Error codes for SIM file operations via QBinarySimFile and QRecordBasedSimFile.

    \value SimFileServiceUnavailable Sim file access is not available on any of the services in the system.
    \value SimFileSimUnavailable Sim file access is available, but the SIM is not present or ready.
    \value SimFileNotFound Sim file access is available, but the file does not exist on the SIM.
    \value SimFileInvalidRead Attempt to perform a read operation with invalid parameters.
    \value SimFileInvalidWrite Attempt to perform a write operation with invalid parameters.
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::RegistrationState)
Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::OperatorMode)
Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::OperatorAvailability)
Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::CallClass)
Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::Result)
Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::SimFileType)
Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephony::SimFileError)
