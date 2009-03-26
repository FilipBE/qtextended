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

#include <qsimterminalresponse.h>
#include <qsmsmessage.h>

/*!
    \class QSimTerminalResponse
    \inpublicgroup QtTelephonyModule

    \brief The QSimTerminalResponse class specifies the contents of a SIM toolkit TERMINAL RESPONSE message.

    Applications that run within a SIM send commands to the host program
    to interact with the user.  These commands might entail choosing an
    item from a menu, asking if it is OK to dial a phone number, asking
    for a line of input, displaying text messages, etc.

    Once the user picks an action, a TERMINAL RESPONSE message is typically sent back
    to the SIM to communicate the user's intent.  The QSimTerminalResponse class
    encapsulates a TERMINAL RESPONSE message, containing all of the information about it.

    In Qtopia, the host program is \c simapp.

    \ingroup telephony
    \sa QSimToolkit, QSimCommand
*/

/*!
    \enum QSimTerminalResponse::Result
    This enum defines the primary response code for a TERMINAL RESPONSE message to a SIM,
    from 3GPP TS 11.14, section 12.12.

    \value Success Command performed successfully.
    \value PartialComprehension Command performed with partial comprehension.
    \value MissingInformation Command performed with missing information.
    \value RefreshPerformed Refresh performed with additional elementary files read.
    \value IconNotDisplayed Command performed, but requested icon could not be displayed.
    \value ModifiedCallControl Command performed, but modified by call control by SIM.
    \value LimitedService Command performed, limited service.
    \value WithModification Command performed with modification.
    \value SessionTerminated Proactive SIM session terminated by the user.
    \value BackwardMove Backward move in proactive SIM session requested by the user.
    \value NoResponseFromUser No response from the user.
    \value HelpInformationRequested Help information requested by the user.
    \value UssdOrSsTerminatedByUser USSD or SS transaction terminated by the user.
    \value MEUnableToProcess ME currently unable to process command.
    \value NetworkUnableToProcess Network currently unable to process command.
    \value UserDidNotAccept User did not accept the practive command.
    \value UserClearedDownCall User cleared down call before connection or network release.
    \value ActionInContradictionWithTimer Action in contradication with the current timer state.
    \value TemporaryCallControlProblem Interaction with call control by SIM, temporary problem.
    \value LaunchBrowserError Launch browser generic error code.
    \value BeyondMECapabilities Command beyond ME's capabilities.
    \value TypeNotUnderstood Command type not understood by ME.
    \value DataNotUnderstood Command data not understood by ME.
    \value NumberNotUnderstood Command number not understood by ME.
    \value SsReturnError SS Return Error.
    \value SmsRpError SMS RP-ERROR.
    \value RequiredValuesMissing Error, required values are missing.
    \value UssdReturnError USSD Return Error.
    \value MultipleCardError MultipleCard commands error.
    \value PermanentCallControlProblem Interaction with call control by SIM or MO short message control by SIM, permanent problem.
    \value BearerIndependentProtocolProblem Bearer Independent Protocol error.
*/

/*!
    \enum QSimTerminalResponse::Cause
    This enum defines the secondary cause code for a TERMINAL RESPONSE message to a SIM,
    from 3GPP TS 11.14, section 12.12.

    \value NoSpecificCause No specific cause can be given (applies to all results).
    \value ScreenIsBusy Screen is busy (applies to \c MEUnableToProcess).
    \value BusyOnCall ME currently busy on call (applies to \c MEUnableToProcess).
    \value BusyOnSsTransaction ME currently busy on SS transaction (applies to \c MEUnableToProcess).
    \value NoService No service (applies to \c MEUnableToProcess).
    \value AccessControlClassBar Access control class bar (applies to \c MEUnableToProcess).
    \value RadioResourceNotGranted Radio resource not granted (applies to \c MEUnableToProcess).
    \value NotInSpeechCall Not in speech call (applies to \c MEUnableToProcess).
    \value BusyOnUssdTransaction ME currently busy on USSD transaction (applies to \c MEUnableToProcess).
    \value BusyOnDtmf ME currently busy on SEND DTMF command (applies to \c MEUnableToProcess).
    \value ActionNotAllowed Action not allowed (applies to \c PermanentCallControlProblem).
    \value TypeOfRequestHasChanged The type of request has changed (applies to \c PermanentCallControlProblem).
    \value CardReaderRemovedOrNotPresent Card reader removed or not present (applies to \c MultipleCardError).
    \value CardRemovedOrNotPresent Card removed or not present (applies to \c MultipleCardError).
    \value CardReaderBusy Card reader busy (applies to \c MultipleCardError).
    \value CardPoweredOff Card powered off (applies to \c MultipleCardError).
    \value CAPDUFormatError C-APDU format error (applies to \c MultipleCardError).
    \value MuteCard Mute card (applies to \c MultipleCardError).
    \value TransmissionError Transmission error (applies to \c MultipleCardError).
    \value ProtocolNotSupported Protocol not supported (applies to \c MultipleCardError).
    \value SpecifiedReaderNotValid Specified reader not valid (applies to \c MultipleCardError).
    \value BearerUnavailable Bearer unavailable (applies to \c LaunchBrowserError).
    \value BrowserUnavailable Browser unavailable (applies to \c LaunchBrowserError).
    \value UnableToReadProvisioningData ME unable to read the provisioning data (applies to \c LaunchBrowserError).
    \value NoChannelAvailable No channel available (applies to \c BearerIndependentProtocolProblem).
    \value ChannelClosed Channel closed (applies to \c BearerIndependentProtocolProblem).
    \value ChannelIdentifierNotValid Channel identifier not valid (applies to \c BearerIndependentProtocolProblem).
    \value RequestedBufferSizeNotAvailable Requested buffer size not available (applies to \c BearerIndependentProtocolProblem).
    \value SecurityError Security error; i.e. unsuccessful authentication (applies to \c BearerIndependentProtocolProblem).
    \value RequestedTransportNotAvailable Required SIM/ME interface transport level not available (applies to \c BearerIndependentProtocolProblem).
*/

class QSimTerminalResponsePrivate
{
public:
    QSimTerminalResponsePrivate()
    {
        commandPdu = command.toPdu();
        sourceDevice = QSimCommand::ME;
        destinationDevice = QSimCommand::SIM;
        result = QSimTerminalResponse::Success;
        duration = 0;
        menuItem = 0;
        dataCodingScheme = -1;
    }

    QSimTerminalResponsePrivate( QSimTerminalResponsePrivate *other )
    {
        command = other->command;
        commandPdu = other->commandPdu;
        sourceDevice = other->sourceDevice;
        destinationDevice = other->destinationDevice;
        result = other->result;
        causeData = other->causeData;
        text = other->text;
        duration = other->duration;
        menuItem = other->menuItem;
        dataCodingScheme = other->dataCodingScheme;
        extensionData = other->extensionData;
    }

    template <typename Stream> int readInt( Stream &stream )
    {
        int value;
        stream >> value;
        return value;
    }

    template <typename Stream>  void read( Stream &stream )
    {
        stream >> command;
        stream >> commandPdu;
        sourceDevice = (QSimCommand::Device)readInt( stream );
        destinationDevice = (QSimCommand::Device)readInt( stream );
        result = (QSimTerminalResponse::Result)readInt( stream );
        stream >> causeData;
        stream >> text;
        stream >> duration;
        stream >> menuItem;
        stream >> dataCodingScheme;
        stream >> extensionData;
    }

    template <typename Stream> void write( Stream &stream )
    {
        stream << command;
        stream << commandPdu;
        stream << (int)sourceDevice;
        stream << (int)destinationDevice;
        stream << (int)result;
        stream << causeData;
        stream << text;
        stream << duration;
        stream << menuItem;
        stream << dataCodingScheme;
        stream << extensionData;
    }

    QSimCommand command;
    QByteArray commandPdu;
    QSimCommand::Device sourceDevice;
    QSimCommand::Device destinationDevice;
    QSimTerminalResponse::Result result;
    QByteArray causeData;
    QString text;
    uint duration;
    uint menuItem;
    int dataCodingScheme;
    QByteArray extensionData;
};

/*!
    Construct a new SIM terminal response object with default parameters.
*/
QSimTerminalResponse::QSimTerminalResponse()
{
    d = new QSimTerminalResponsePrivate();
}

/*!
    Construct a new SIM terminal response object as a copy of \a value.
*/
QSimTerminalResponse::QSimTerminalResponse( const QSimTerminalResponse& value )
{
    d = new QSimTerminalResponsePrivate( value.d );
}

/*!
    Destruct a SIM terminal response object.
*/
QSimTerminalResponse::~QSimTerminalResponse()
{
    delete d;
}

/*!
    Returns the SIM command that gave rise to this SIM terminal response.
    The default is a default-constructed QSimCommand.

    \sa setCommand(), commandPdu(), setCommandPdu()
*/
QSimCommand QSimTerminalResponse::command() const
{
    return d->command;
}

/*!
    Sets the SIM command that gave rise to this SIM terminal response to \a value.
    Calling this function will also affect the result of commandPdu().

    \sa command(), commandPdu(), setCommandPdu()
*/
void QSimTerminalResponse::setCommand( const QSimCommand& value )
{
    d->command = value;
    d->commandPdu = value.toPdu();
}

/*!
    Returns the PDU form of the SIM command that gave rise to this SIM terminal response.
    The default is a PDU corresponding to a default-constructed QSimCommand.

    \sa setCommandPdu(), command(), setCommand()
*/
QByteArray QSimTerminalResponse::commandPdu() const
{
    return d->commandPdu;
}

/*!
    Sets the PDU form of the SIM command that gave rise to this SIM terminal response
    to \a value.  Calling this function will also affect the result of command().

    \sa commandPdu(), command(), setCommand()
*/
void QSimTerminalResponse::setCommandPdu( const QByteArray& value )
{
    d->commandPdu = value;
    d->command = QSimCommand::fromPdu( value );
}

/*!
    Returns the source device that generated the response.  The default value is
    QSimCommand::ME.

    \sa setSourceDevice()
*/
QSimCommand::Device QSimTerminalResponse::sourceDevice() const
{
    return d->sourceDevice;
}

/*!
    Sets the source device that generated the response to \a value.

    \sa sourceDevice()
*/
void QSimTerminalResponse::setSourceDevice( QSimCommand::Device value )
{
    d->sourceDevice = value;
}

/*!
    Returns the destination device that will receive the response.  The default
    value is QSimCommand::SIM.

    \sa setDestinationDevice()
*/
QSimCommand::Device QSimTerminalResponse::destinationDevice() const
{
    return d->destinationDevice;
}

/*!
    Sets the destination device that will receive the response to \a value.

    \sa destinationDevice()
*/
void QSimTerminalResponse::setDestinationDevice( QSimCommand::Device value )
{
    d->destinationDevice = value;
}

/*!
    Returns the result code for this SIM terminal response.  The default value
    is \c Success.

    The result code may not be sufficient on its own to determine the cause of
    a failure.  The cause() and causeData() functions provides the additional information.

    \sa setResult(), cause(), causeData()
*/
QSimTerminalResponse::Result QSimTerminalResponse::result() const
{
    return d->result;
}

/*!
    Sets the result code for this SIM terminal response to \a value.

    The result code may not be sufficient on its own to determine the cause of
    a failure.  The setCause() and setCauseData() functions can be used to provide
    the additional information.

    \sa result(), setCause(), setCauseData()
*/
void QSimTerminalResponse::setResult( QSimTerminalResponse::Result value )
{
    d->result = value;
}

/*!
    Returns the additional cause information for this SIM terminal response.
    Returns \c NoSpecificCause if there is no additional cause information.

    The additional cause value is the first byte of causeData(), or
    \c NoSpecificCause if causeData() is empty.  Thus, cause() should be
    used only as a convenience function to quickly determine the cause of
    an error result().

    \sa setCause(), causeData(), result()
*/
QSimTerminalResponse::Cause QSimTerminalResponse::cause() const
{
    if ( d->causeData.isEmpty() )
        return NoSpecificCause;
    else
        return (QSimTerminalResponse::Cause)(d->causeData[0] & 0xFF);
}

/*!
    Sets the additional cause information for this SIM terminal response to \a value.
    This is equivalent to calling setCauseData() with a QByteArray containing a single
    byte, \a value.

    \sa cause(), setCauseData(), result()
*/
void QSimTerminalResponse::setCause( QSimTerminalResponse::Cause value )
{
    d->causeData = QByteArray(1, (char)value);
}

/*!
    Returns the additional cause data associated with this SIM terminal response.
    The default value is an empty QByteArray.

    \sa setCauseData(), cause(), result()
*/
QByteArray QSimTerminalResponse::causeData() const
{
    return d->causeData;
}

/*!
    Sets the additional cause data associated with this SIM terminal response to \a value.

    \sa causeData(), result()
*/
void QSimTerminalResponse::setCauseData( const QByteArray& value )
{
    d->causeData = value;
}

/*!
    Returns the text to be sent along with this terminal response.

    Applies to: \c GetInkey, \c GetInput

    \sa setText()
*/
QString QSimTerminalResponse::text() const
{
    return d->text;
}


/*!
    Sets the text to be sent along with this terminal response to \a value.

    Applies to: \c GetInkey, \c GetInput

    \sa text()
*/
void QSimTerminalResponse::setText( const QString& value )
{
    d->text = value;
}

/*!
    Returns the number of milliseconds for the duration of a poll interval.
    The default value is zero, indicating that the default duration should be used.

    Applies to: \c PollInterval.

    \sa setDuration()
*/
uint QSimTerminalResponse::duration() const
{
    return d->duration;
}

/*!
    Sets the duration of a poll interval to \a value.

    Applies to: \c PollInterval

    \sa duration()
*/
void QSimTerminalResponse::setDuration( uint value )
{
    d->duration = value;
}

/*!
    Returns the menu item to be selected.  The default value is zero.

    Applies to: \c SelectItem

    \sa setMenuItem()
*/
uint QSimTerminalResponse::menuItem() const
{
    return d->menuItem;
}

/*!
    Sets the menu item to be selected to \a value.

    Applies to: \c SelectItem

    \sa menuItem()
*/
void QSimTerminalResponse::setMenuItem( uint value )
{
    d->menuItem = value;
}

/*!
    Returns the recommended data coding scheme for encoding USSD text strings.
    The default value is -1, which indicates that the best scheme should be chosen
    based on the contents of the USSD text string.

    \sa setDataCodingScheme()
*/
int QSimTerminalResponse::dataCodingScheme() const
{
    return d->dataCodingScheme;
}

/*!
    Sets the recommended data coding scheme for encoding USSD text strings to \a value.
    The value -1 indicates that the best scheme should be chosen based on the contents
    of the USSD text string.

    \sa dataCodingScheme()
*/
void QSimTerminalResponse::setDataCodingScheme( int value )
{
    d->dataCodingScheme = value;
}

/*!
    Returns the extension data for this terminal response.  The extension data is
    appended after all other fields, and consists of zero or more BER tag-length-value
    field specifications.

    \sa setExtensionData(), extensionField()
*/
QByteArray QSimTerminalResponse::extensionData() const
{
    return d->extensionData;
}

/*!
    Sets the extension data for this terminal response to \a value.  The extension
    data is appended after all other fields, and consists of zero or more BER
    tag-length-value field specifications.

    \sa extensionData(), addExtensionField()
*/
void QSimTerminalResponse::setExtensionData( QByteArray value )
{
    d->extensionData = value;
}

// Imports from qsimcommand.cpp.
void _qtopiaphone_readBer( const QByteArray& binary, uint& posn, uint& tag, uint& length );
void _qtopiaphone_writeTextString( QByteArray& binary, const QString& str,
                                   QSimCommand::ToPduOptions options, int tag = 0x8D );
QString _qtopiaphone_decodeCodedString( const QByteArray& binary, uint posn, uint length );
void _qtopiaphone_writeDuration( QByteArray& data, uint time );
void _qtopiaphone_writeBerLength( QByteArray& binary, int length );
#define readBer _qtopiaphone_readBer
#define writeTextString _qtopiaphone_writeTextString
#define decodeCodedString _qtopiaphone_decodeCodedString
#define writeDuration _qtopiaphone_writeDuration
#define writeBerLength _qtopiaphone_writeBerLength

/*!
    Returns the contents of an extension field.  The \a tag is an 8-bit value,
    from 3GPP TS 11.14, that specifies the particular field the caller is
    interested in.  The most significant bit of \a tag is ignored when searching
    for the field, as it contains the "must comprehend" status from 3GPP TS 11.14,
    and is not important for locating the desired field.

    This is a simpler method than extensionData() for accessing values within
    the extension data.  As an example, the following code extracts the
    "answer to reset" information from the response to a \c PowerOnCard SIM command:

    \code
    QSimTerminalResponse resp;
    ...
    QByteArray atr = resp.extensionField(0xA1);
    \endcode

    \sa addExtensionField()
*/
QByteArray QSimTerminalResponse::extensionField( int tag ) const
{
    uint posn = 0;
    uint currentTag, length;
    while ( posn < (uint)( d->extensionData.length() ) ) {
        readBer( d->extensionData, posn, currentTag, length );
        if ( ( currentTag & 0x7F ) == (uint)( tag & 0x7F ) )
            return d->extensionData.mid( posn, length );
        posn += length;
    }
    return QByteArray();
}

/*!
    Adds an extension field to the data from extensionData().  The field will have
    the specified \a tag and contents given by \a value.

    \sa extensionField()
*/
void QSimTerminalResponse::addExtensionField( int tag, const QByteArray& value )
{
    d->extensionData += (char)tag;
    writeBerLength( d->extensionData, value.size() );
    d->extensionData += value;
}

/*!
    Returns a SIM terminal response object corresponding to the data in \a pdu.
    The data is decoded as described in 3GPP TS 11.14, section 6.8.

    \sa toPdu()
*/
QSimTerminalResponse QSimTerminalResponse::fromPdu( const QByteArray& pdu )
{
    QSimTerminalResponse resp;
    uint posn = 0;
    uint startPosn = 0;
    uint newPosn;
    uint tag, length;
    readBer( pdu, posn, tag, length );
    if ( ( tag & 0x7F ) != 0x01 ) {
        // Doesn't appear to be a valid TERMINAL RESPONSE.
        return resp;
    }
    for (;;) {
        if ( ( posn + length ) > (uint)pdu.size() )
            break;
        newPosn = posn + length;
        switch ( tag & 0x7F ) {

            case 0x01:
            {
                // Command details pdu blob.
                resp.setCommandPdu( pdu.mid( startPosn, posn + length - startPosn ) );
            }
            break;

            case 0x02:
            {
                // Device identities, GSM 11.14, section 12.7.
                if ( length >= 2 ) {
                    resp.setSourceDevice( (QSimCommand::Device)( pdu[posn] & 0xFF ) );
                    resp.setDestinationDevice( (QSimCommand::Device)( pdu[posn + 1] & 0xFF ) );
                }
            }
            break;

            case 0x03:
            {
                // Result information.
                if ( length < 1 )
                    break;
                resp.setResult( (QSimTerminalResponse::Result)(pdu[posn] & 0xFF) );
                resp.setCauseData( pdu.mid( posn + 1, length - 1 ) );
            }
            break;

            case 0x04:
            {
                // Duration for a PollInterval command, GSM 11.14, section 12.8.
                if ( length < 2 )
                    break;
                uint multiplier = 60000;            // Minutes.
                if ( pdu[posn] == 0x01 )         // Seconds.
                    multiplier = 1000;
                else if ( pdu[posn] == 0x02 )    // Tenths of a second.
                    multiplier = 100;
                resp.setDuration( multiplier * ( pdu[posn + 1] & 0xFF ) );
            }
            break;

            case 0x0D:
            {
                // Text string for terminal response, GSM 11.14, section 12.15.
                if ( pdu.length() >= 5 && pdu[3] == (int)QSimCommand::GetInkey &&
                     (pdu[4] & 0x04) != 0 ) {
                    // Processing Yes/No responses: convert 0x01/0x00 into Yes/No.
                    if ( length >= 2 && pdu[posn + 1] != (char)0x00 )
                        resp.setText( "Yes" );    // No tr
                    else
                        resp.setText( "No" );     // No tr
                } else {
                    if ( length > 0 )
                        resp.setDataCodingScheme( pdu[posn] & 0xFF );
                    resp.setText( decodeCodedString( pdu, posn, length ) );
                }
            }
            break;

            case 0x10:
            {
                // Menu item identifier, GSM 11.14, section 12.10.
                if ( length > 0 )
                    resp.setMenuItem( (uint)(pdu[posn] & 0xFF) );
            }
            break;

            default:
            {
                // Don't know what this is, so add it as an extension field.
                resp.addExtensionField( tag, pdu.mid( posn, length ) );
            }
            break;
        }
        posn = newPosn;
        if ( posn >= (uint)pdu.size() )
            break;
        startPosn = newPosn;
        readBer( pdu, posn, tag, length );
    }
    return resp;
}

/*!
    Returns the PDU form of this SIM terminal response, encoded as described in
    3GPP TS 11.14, section 6.8.

    \sa fromPdu()
*/
QByteArray QSimTerminalResponse::toPdu() const
{
    QByteArray data;

    // Extract the command details from the command PDU.
    QByteArray cmd = d->commandPdu;
    uint posn = 0;
    uint startPosn = 0;
    uint tag, length, qual;
    readBer( cmd, posn, tag, length );
    if ( tag == 0xD0 ) {
        // There appears to be a "Proactive SIM" wrapper on the front
        // of the command details.  Skip over it.
        startPosn = posn;
        readBer( cmd, posn, tag, length );
    }
    if ( (tag & 0x7F) == 0x01 ) {
        data += cmd.mid( startPosn, posn + length - startPosn );
        if ( length >= 3 ) {
            tag = (cmd[posn + 1] & 0x7F);
            qual = (cmd[posn + 2] & 0xFF);
        } else {
            tag = 0;
            qual = 0;
        }
    } else {
        // Could not find the command details, so output default details.
        data += (char)0x81;
        data += (char)0x03;
        data += (char)0x00;
        data += (char)0x00;
        data += (char)0x00;
        tag = 0;
        qual = 0;
    }

    // Add the device identity section (ME to SIM).
    data += (char)0x82;
    data += (char)0x02;
    data += (char)sourceDevice();
    data += (char)destinationDevice();

    // Add the result details.
    switch ( (QSimCommand::Type)tag ) {
        case QSimCommand::SendSS:
            if ( d->result == DataNotUnderstood )
                data += (char)0x83;
            else
                data += (char)0x03;
            break;
        default:                        data += (char)0x83; break;
    }
    data += (char)(d->causeData.length() + 1);
    data += (char)(d->result);
    data += d->causeData;

    // Add other information for specific command response types.
    switch ( (QSimCommand::Type)tag ) {

        case QSimCommand::GetInkey:
        {
            if ( d->result != Success && d->result != IconNotDisplayed )
                break;
            if ( ( qual & 0x04 ) != 0 ) {
                // Encode a Yes/No response.
                if ( text() == "Yes" ) {      // No tr
                    data += (char)0x8D;
                    data += (char)0x02;
                    data += (char)0x04;
                    data += (char)0x01;
                } else {
                    data += (char)0x8D;
                    data += (char)0x02;
                    data += (char)0x04;
                    data += (char)0x00;
                }
            } else if ( ( qual & 0x02 ) != 0 ) {
                // Use UCS-2 to encode the data.
                writeTextString( data, text(), QSimCommand::UCS2Strings );
            } else {
                // Use the unpacked 8-bit GSM encoding to encode the data.
                writeTextString( data, text(), QSimCommand::NoPduOptions );
            }
        }
        break;

        case QSimCommand::GetInput:
        {
            if ( d->result != Success && d->result != IconNotDisplayed )
                break;
            if ( ( qual & 0x02 ) != 0 ) {
                // Use UCS-2 to encode the data.
                writeTextString( data, text(), (QSimCommand::ToPduOptions)
                                                 (QSimCommand::UCS2Strings |
                                                  QSimCommand::EncodeEmptyStrings) );
            } else if ( ( qual & 0x08 ) != 0 ) {
                // Use the packed 7-bit GSM encoding to encode the data.
                writeTextString( data, text(), (QSimCommand::ToPduOptions)
                                                 (QSimCommand::PackedStrings |
                                                  QSimCommand::EncodeEmptyStrings) );
            } else {
                // Use the unpacked 8-bit GSM encoding to encode the data.
                writeTextString( data, text(), QSimCommand::EncodeEmptyStrings );
            }
        }
        break;

        case QSimCommand::PollInterval:
        {
            writeDuration( data, duration() );
        }
        break;

        case QSimCommand::SelectItem:
        {
            if ( menuItem() != 0 ) {
                data += (char)0x90;
                data += (char)0x01;
                data += (char)menuItem();
            }
        }
        break;

        case QSimCommand::SendUSSD:
        {
            if ( d->result != Success && d->result != IconNotDisplayed )
                break;
            int scheme = dataCodingScheme();
            QSimCommand::ToPduOptions options = QSimCommand::NoPduOptions;
            if ( scheme != -1 ) {
                if ( ( scheme & 0x0C ) == QSMS_DefaultAlphabet )
                    options = QSimCommand::PackedStrings;
                else if ( ( scheme & 0x0C ) == QSMS_UCS2Alphabet )
                    options = QSimCommand::UCS2Strings;
                scheme &= 0xF3;
            } else {
                scheme = 0;
            }
            writeTextString( data, text(), options, (scheme << 8) | 0x8D );
        }
        break;

        default: break;
    }

    // Add any extension data that is specified.
    data += extensionData();

    // Return the final SIM terminal response PDU.
    return data;
}

/*!
    Copy the QSimTerminalResponse object \a value.
*/
QSimTerminalResponse& QSimTerminalResponse::operator=( const QSimTerminalResponse &value )
{
    if ( d != value.d ) {
        delete d;
        d = new QSimTerminalResponsePrivate( value.d );
    }
    return *this;
}

/*!
    \fn void QSimTerminalResponse::deserialize(Stream &value)

    \internal

    Deserializes the QSimTerminalResponse instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimTerminalResponse::deserialize(Stream &stream)
{
    d->read( stream );
}


/*!
    \fn void QSimTerminalResponse::serialize(Stream &value) const

    \internal

    Serializes the QSimTerminalResponse instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimTerminalResponse::serialize(Stream &stream) const
{
    d->write( stream );
}

Q_IMPLEMENT_USER_METATYPE(QSimTerminalResponse)
