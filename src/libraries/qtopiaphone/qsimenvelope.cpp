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

#include <qsimenvelope.h>

/*!
    \class QSimEnvelope
    \inpublicgroup QtTelephonyModule

    \brief The QSimEnvelope class specifies the contents of a SIM ENVELOPE message.

    Envelopes are specified by 3GPP TS 11.14 and are used for a variety of purposes
    when communicating with SIM's, including:

    \list
        \o Menu selections for SIM toolkit applications.
        \o Downloading items from the network into the SIM.
        \o Call control and SMS control by the SIM.
        \o Notifying the SIM of events of interest in the system.
    \endlist

    The particular type of envelope is specified by the type() function.  Other
    functions, such as menuItem(), requestHelp(), etc, are used to specify
    type-specific parameters for the envelope.

    \ingroup telephony
    \sa QSimToolkit, QSimCommand, QSimTerminalResponse
*/

class QSimEnvelopePrivate
{
public:
    QSimEnvelopePrivate()
    {
        type = QSimEnvelope::NoEnvelope;
        event = QSimEnvelope::NoEvent;
        sourceDevice = QSimCommand::ME;
        destinationDevice = QSimCommand::SIM;
        menuItem = 0;
        requestHelp = false;
    }

    QSimEnvelopePrivate( QSimEnvelopePrivate *other )
    {
        type = other->type;
        event = other->event;
        sourceDevice = other->sourceDevice;
        destinationDevice = other->destinationDevice;
        menuItem = other->menuItem;
        requestHelp = other->requestHelp;
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
        type = (QSimEnvelope::Type)readInt( stream );
        event = (QSimEnvelope::Event)readInt( stream );
        sourceDevice = (QSimCommand::Device)readInt( stream );
        destinationDevice = (QSimCommand::Device)readInt( stream );
        stream >> menuItem;
        requestHelp = ( readInt( stream ) != 0 );
        stream >> extensionData;
    }

    template <typename Stream> void write( Stream &stream )
    {
        stream << (int)type;
        stream << (int)event;
        stream << (int)sourceDevice;
        stream << (int)destinationDevice;
        stream << menuItem;
        stream << (int)requestHelp;
        stream << extensionData;
    }

    QSimEnvelope::Type type;
    QSimEnvelope::Event event;
    QSimCommand::Device sourceDevice;
    QSimCommand::Device destinationDevice;
    uint menuItem;
    bool requestHelp;
    QByteArray extensionData;
};

/*!
    \enum QSimEnvelope::Type
    This enum defines the type of QSimEnvelope request to be sent to a SIM.

    \value NoEnvelope The envelope type has not yet been set.
    \value SMSPPDownload The envelope is being used for an SMS-PP download.
    \value CellBroadcastDownload The envelope is being used for a cell broadcast download.
    \value MenuSelection The envelope is being used for a SIM toolkit menu selection.
    \value CallControl The envelope is being used for call control.
    \value MOSMSControl The envelope is being used for mobile-originated short message control.
    \value EventDownload The envelope is being used for event download.
    \value TimerExpiration The envelope is being used to report timer expiration.
*/

/*!
    \enum QSimEnvelope::Event
    This enum defines the type of event for a \c EventDownload envelope, according
    to 3GPP TS 11.14, section 12.25.

    \value NoEvent No event type defined.
    \value MTCall Mobile-terminated call event.
    \value CallConnected Call connected event.
    \value CallDisconnected Call disconnected event.
    \value LocationStatus Location status event.
    \value UserActivity User activity event.
    \value IdleScreenAvailable Idle screen available event.
    \value CardReaderStatus Additional card reader status event.
    \value LanguageSelection Language selection event.
    \value BrowserTermination Browser termination event.
    \value DataAvailable Channel data available event.
    \value ChannelStatus Channel status event.
*/

/*!
    Construct a new envelope object with default parameters.
*/
QSimEnvelope::QSimEnvelope()
{
    d = new QSimEnvelopePrivate();
}

/*!
    Construct a new envelope object as a copy of \a value.
*/
QSimEnvelope::QSimEnvelope( const QSimEnvelope& value )
{
    d = new QSimEnvelopePrivate( value.d );
}

/*!
    Destruct an envelope object.
*/
QSimEnvelope::~QSimEnvelope()
{
    delete d;
}

/*!
    Returns the type of this envelope.

    \sa setType()
*/
QSimEnvelope::Type QSimEnvelope::type() const
{
    return d->type;
}

/*!
    Sets the type of this envelope to \a value.

    \sa type()
*/
void QSimEnvelope::setType( QSimEnvelope::Type value )
{
    d->type = value;
}

/*!
    Returns the source device that generated the envelope.  The default value is
    QSimCommand::ME.

    Applies to: all commands.

    \sa setSourceDevice()
*/
QSimCommand::Device QSimEnvelope::sourceDevice() const
{
    return d->sourceDevice;
}

/*!
    Sets the source device that generated the envelope to \a value.

    Applies to: all commands.

    \sa sourceDevice()
*/
void QSimEnvelope::setSourceDevice( QSimCommand::Device value )
{
    d->sourceDevice = value;
}

/*!
    Returns the destination device that will receive the envelope.  The default
    value is QSimCommand::SIM.

    Applies to: all commands.

    \sa setDestinationDevice()
*/
QSimCommand::Device QSimEnvelope::destinationDevice() const
{
    return d->destinationDevice;
}

/*!
    Sets the destination device that will receive the envelope to \a value.

    Applies to: all commands.

    \sa destinationDevice()
*/
void QSimEnvelope::setDestinationDevice( QSimCommand::Device value )
{
    d->destinationDevice = value;
}

/*!
    Returns the menu item to be selected.  The default value is zero.

    Applies to: \c MenuSelection

    \sa setMenuItem()
*/
uint QSimEnvelope::menuItem() const
{
    return d->menuItem;
}

/*!
    Sets the menu item to be selected to \a value.

    Applies to: \c MenuSelection

    \sa menuItem()
*/
void QSimEnvelope::setMenuItem( uint value )
{
    d->menuItem = value;
}

/*!
    Returns true if help has been requested for this menu selection; false if the
    menu item is being selected normally.  The default value is false.

    Applies to: \c MenuSelection

    \sa setRequestHelp()
*/
bool QSimEnvelope::requestHelp() const
{
    return d->requestHelp;
}

/*!
    Sets the help request flag to \a value.  If \a value is true, then help has been
    requested for this menu selection.  If \a value is false, then the menu item
    is being selected normally.

    Applies to: \c MenuSelection

    \sa requestHelp()
*/
void QSimEnvelope::setRequestHelp( bool value )
{
    d->requestHelp = value;
}

/*!
    Returns the event type for \c EventDownload envelopes.  The default value is \c NoEvent.

    Applies to: \c EventDownload

    \sa setEvent()
*/
QSimEnvelope::Event QSimEnvelope::event() const
{
    return d->event;
}

/*!
    Sets the event type for \c EventDownload envelopes to \a value.

    Applies to: \c EventDownload

    \sa event()
*/
void QSimEnvelope::setEvent( QSimEnvelope::Event value )
{
    d->event = value;
}

/*!
    Returns the extension data for this envelope.  The extension data is
    appended after all other fields, and consists of zero or more BER tag-length-value
    field specifications.

    \sa setExtensionData(), extensionField()
*/
QByteArray QSimEnvelope::extensionData() const
{
    return d->extensionData;
}

/*!
    Sets the extension data for this envelope to \a value.  The extension
    data is appended after all other fields, and consists of zero or more BER
    tag-length-value field specifications.

    \sa extensionData(), addExtensionField()
*/
void QSimEnvelope::setExtensionData( QByteArray value )
{
    d->extensionData = value;
}

// Imports from qsimcommand.cpp.
void _qtopiaphone_readBer( const QByteArray& binary, uint& posn, uint& tag, uint& length );
void _qtopiaphone_writeBerLength( QByteArray& binary, int length );
#define readBer _qtopiaphone_readBer
#define writeBerLength _qtopiaphone_writeBerLength

/*!
    Returns the contents of an extension field.  The \a tag is an 8-bit value,
    from 3GPP TS 11.14, that specifies the particular field the caller is
    interested in.  The most significant bit of \a tag is ignored when searching
    for the field, as it contains the "must comprehend" status from 3GPP TS 11.14,
    and is not important for locating the desired field.

    This is a simpler method than extensionData() for accessing values within
    the extension data.

    \sa addExtensionField()
*/
QByteArray QSimEnvelope::extensionField( int tag ) const
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
void QSimEnvelope::addExtensionField( int tag, const QByteArray& value )
{
    d->extensionData += (char)tag;
    writeBerLength( d->extensionData, value.size() );
    d->extensionData += value;
}

/*!
    Returns an envelope object corresponding to the data in \a pdu.
    The data is decoded as described in 3GPP TS 11.14.

    \sa toPdu()
*/
QSimEnvelope QSimEnvelope::fromPdu( const QByteArray& pdu )
{
    QSimEnvelope env;
    QByteArray content;
    uint posn = 0;
    uint startPosn;
    uint newPosn;
    uint tag, length;
    readBer( pdu, posn, tag, length );
    if ( ( tag & 0xF0 ) != 0xD0 ) {
        // Doesn't appear to be a valid ENVELOPE.
        return env;
    }
    env.setType( (QSimEnvelope::Type)tag );
    content = pdu.mid( posn, length );
    posn = 0;
    startPosn = 0;
    readBer( content, posn, tag, length );
    for (;;) {
        if ( ( posn + length ) > (uint)content.size() )
            break;
        newPosn = posn + length;
        switch ( tag & 0x7F ) {

            case 0x02:
            {
                // Device identities, GSM 11.14, section 12.7.
                if ( length >= 2 ) {
                    env.setSourceDevice
                        ( (QSimCommand::Device)( content[posn] & 0xFF ) );
                    env.setDestinationDevice
                        ( (QSimCommand::Device)( content[posn + 1] & 0xFF ) );
                }
            }
            break;

            case 0x10:
            {
                // Menu item identifier, GSM 11.14, section 12.10.
                if ( length > 0 )
                    env.setMenuItem( (uint)(content[posn] & 0xFF) );
            }
            break;

            case 0x15:
            {
                // Help requested, GSM 11.14, section 12.21.
                env.setRequestHelp( true );
            }
            break;

            case 0x19:
            {
                // Event list, GSM 11.14, section 12.25.
                if ( length > 0 )
                    env.setEvent( (QSimEnvelope::Event)( content[posn] & 0xFF ) );
            }
            break;

            default:
            {
                // Don't know what this is, so add it as an extension field.
                env.addExtensionField( tag, content.mid( posn, length ) );
            }
            break;
        }
        posn = newPosn;
        if ( posn >= (uint)content.size() )
            break;
        startPosn = newPosn;
        readBer( content, posn, tag, length );
    }
    return env;
}

/*!
    Returns the PDU form of this envelope, encoded as described in 3GPP TS 11.14.

    \sa fromPdu()
*/
QByteArray QSimEnvelope::toPdu() const
{
    QByteArray data;

    // Output the event list before the device identities.
    if ( d->type == QSimEnvelope::EventDownload ) {
        data += (char)0x99;
        data += (char)0x01;
        data += (char)(d->event);
    }

    // Add the device identity section (ME/Keypad/... to SIM).
    // According to 3GPP TS 51.010-4, the tag should be 0x02 for
    // MO-SMS control by SIM.
    if ( d->type == QSimEnvelope::MOSMSControl )
        data += (char)0x02;
    else
        data += (char)0x82;
    data += (char)0x02;
    data += (char)sourceDevice();
    data += (char)destinationDevice();

    // Add parameters specific to this type of envelope.
    switch ( type() ) {

        case MenuSelection:
        {
            data += (char)0x90;
            data += (char)0x01;
            data += (char)menuItem();
            if ( requestHelp() ) {
                data += (char)0x15;
                data += (char)0x00;
            }
        }
        break;

        default: break;

    }

    // Add any extension data that is specified.
    data += extensionData();

    // Add the outermost envelope tag layer and return.
    QByteArray env;
    env += (char)type();
    writeBerLength( env, data.size() );
    env += data;
    return env;
}

/*!
    Copy the QSimEnvelope \a value.
*/
QSimEnvelope& QSimEnvelope::operator=( const QSimEnvelope &value )
{
    if ( d != value.d ) {
        delete d;
        d = new QSimEnvelopePrivate( value.d );
    }
    return *this;
}

/*!
    \fn void QSimEnvelope::deserialize(Stream &value)

    \internal

    Deserializes the QSimEnvelope instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimEnvelope::deserialize(Stream &stream)
{
    d->read( stream );
}

/*!
    \fn void QSimEnvelope::serialize(Stream &value) const

    \internal

    Serializes the QSimEnvelope instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimEnvelope::serialize(Stream &stream) const
{
    d->write( stream );
}

Q_IMPLEMENT_USER_METATYPE(QSimEnvelope)
