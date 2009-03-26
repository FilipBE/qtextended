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

#include <qsimcontrolevent.h>
#include <qsimcommand.h>

/*!
    \class QSimControlEvent
    \inpublicgroup QtTelephonyModule

    \brief The QSimControlEvent class specifies the contents of a \c{CALL CONTROL} or \c{MO SHORT MESSAGE CONTROL} result event.

    When the user initiates phone calls or sends SMS messages on a
    SIM-equipped mobile phone, the SIM can make modifications to
    the request before it is sent to the network, or deny the request
    outright.  This process is described in section 9 of 3GPP TS 11.14.

    Once the SIM has determined whether it will allow the operation to
    proceed or not, an event is sent to the modem indicating the result.
    This event may include a text() string to be displayed to the user.

    The particular type of SIM control event is specified by the type()
    function.  Other functions, such as result(), text(), etc, are used
    to specify type-specific parameters for the event.

    Events are delivered to client applications via the
    QSimToolkit::controlEvent() signal.

    \ingroup telephony
    \sa QSimToolkit::controlEvent()
*/

/*!
    \enum QSimControlEvent::Type
    This enum defines the type of SIM control event for QSimControlEvent.

    \value Call The control event refers to a regular call setup or supplementary service.
    \value Sms The control event refers to a mobile-originated SMS message.
*/

/*!
    \enum QSimControlEvent::Result
    This enum defines the result of a SIM control event.

    \value Allowed The SIM allowed the call setup or SMS message to be processed normally.
    \value NotAllowed This SIM disallowed the call setup or SMS message.
    \value AllowedWithModifications The SIM allowed the call setup or SMS message,
           but made some modifications to the request.
*/

class QSimControlEventPrivate
{
public:
    QSimControlEventPrivate()
    {
        type = QSimControlEvent::Call;
        result = QSimControlEvent::Allowed;
    }

    QSimControlEventPrivate( QSimControlEventPrivate *other )
    {
        type = other->type;
        result = other->result;
        text = other->text;
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
        type = (QSimControlEvent::Type)readInt( stream );
        result = (QSimControlEvent::Result)readInt( stream );
        stream >> text;
        stream >> extensionData;
    }

    template <typename Stream> void write( Stream &stream )
    {
        stream << (int)type;
        stream << (int)result;
        stream << text;
        stream << extensionData;
    }

    QSimControlEvent::Type type;
    QSimControlEvent::Result result;
    QString text;
    QByteArray extensionData;
};

/*!
    Construct a new SIM control object with default parameters.
*/
QSimControlEvent::QSimControlEvent()
{
    d = new QSimControlEventPrivate();
}

/*!
    Construct a new SIM control object as a copy of \a value.
*/
QSimControlEvent::QSimControlEvent( const QSimControlEvent& value )
{
    d = new QSimControlEventPrivate( value.d );
}

/*!
    Destruct a SIM control object.
*/
QSimControlEvent::~QSimControlEvent()
{
    delete d;
}

/*!
    Returns the type of SIM control event: \c Call or \c Sms.  The default value is \c Call.

    \sa setType()
*/
QSimControlEvent::Type QSimControlEvent::type() const
{
    return d->type;
}

/*!
    Sets the type of this SIM control event to \a value.

    \sa type()
*/
void QSimControlEvent::setType( QSimControlEvent::Type value )
{
    d->type = value;
}

/*!
    Returns the result of this SIM control event.

    \sa setResult()
*/
QSimControlEvent::Result QSimControlEvent::result() const
{
    return d->result;
}

/*!
    Sets the result of this SIM control event to \a value.  The default value is \c Allowed.

    \sa result()
*/
void QSimControlEvent::setResult( QSimControlEvent::Result value )
{
    d->result = value;
}

/*!
    Returns the text string associated with this SIM control event.  If this string
    is not empty, it should be displayed to the user to inform them of the SIM
    control operation.

    \sa setText()
*/
QString QSimControlEvent::text() const
{
    return d->text;
}

/*!
    Sets the text string associated with this SIM control event to \a value.

    \sa text()
*/
void QSimControlEvent::setText( const QString& value )
{
    d->text = value;
}

/*!
    Returns the extension data for this SIM control event.  The extension data is
    appended after all other fields, and consists of zero or more BER tag-length-value
    field specifications.

    \sa setExtensionData(), extensionField()
*/
QByteArray QSimControlEvent::extensionData() const
{
    return d->extensionData;
}

/*!
    Sets the extension data for this SIM control event to \a value.  The extension
    data is appended after all other fields, and consists of zero or more BER
    tag-length-value field specifications.

    \sa extensionData(), addExtensionField()
*/
void QSimControlEvent::setExtensionData( QByteArray value )
{
    d->extensionData = value;
}

// Imports from qsimcommand.cpp.
void _qtopiaphone_readBer( const QByteArray& binary, uint& posn, uint& tag, uint& length );
void _qtopiaphone_writeBerLength( QByteArray& binary, int length );
QString _qtopiaphone_decodeEFADN( const QByteArray& binary, uint posn, uint length );
bool _qtopiaphone_extractAndWriteExtField( QByteArray& data, QByteArray& extData, int tag );
void _qtopiaphone_writeEFADN( QByteArray& binary, const QString& str,
                              QSimCommand::ToPduOptions options, int tag = 0x85 );
#define readBer _qtopiaphone_readBer
#define writeBerLength _qtopiaphone_writeBerLength
#define decodeEFADN _qtopiaphone_decodeEFADN
#define extractAndWriteExtField _qtopiaphone_extractAndWriteExtField
#define writeEFADN _qtopiaphone_writeEFADN

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
QByteArray QSimControlEvent::extensionField( int tag ) const
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
void QSimControlEvent::addExtensionField( int tag, const QByteArray& value )
{
    d->extensionData += (char)tag;
    writeBerLength( d->extensionData, value.size() );
    d->extensionData += value;
}

/*!
    Returns a SIM control event object corresponding to the data in \a pdu.
    The \a type indicates the type of SIM control event that is being decoded
    from \a pdu: \c Call or \c Sms.  The data is decoded as described in 3GPP TS 11.14.

    \sa toPdu()
*/
QSimControlEvent QSimControlEvent::fromPdu
        ( QSimControlEvent::Type type, const QByteArray& pdu )
{
    QSimControlEvent ev;
    QByteArray content;
    uint posn = 0;
    uint startPosn;
    uint newPosn;
    uint tag, length;
    ev.setType( type );
    if ( pdu.size() < 2 ) {
        // Doesn't appear to be a valid control event.
        return ev;
    }
    readBer( pdu, posn, tag, length );
    ev.setResult( (QSimControlEvent::Result)tag );
    content = pdu.mid( posn, length );
    posn = 0;
    startPosn = 0;
    if ( posn >= (uint)content.size() )
        return ev;
    readBer( content, posn, tag, length );
    for (;;) {
        if ( ( posn + length ) > (uint)content.size() )
            break;
        newPosn = posn + length;
        switch ( tag & 0x7F ) {

            case 0x05:
            {
                // Alpha identifier, GSM 11.14, section 12.2.
                ev.setText( decodeEFADN( content, posn, length ) );
            }
            break;

            default:
            {
                // Don't know what this is, so add it as an extension field.
                ev.addExtensionField( tag, content.mid( posn, length ) );
            }
            break;
        }
        posn = newPosn;
        if ( posn >= (uint)content.size() )
            break;
        startPosn = newPosn;
        readBer( content, posn, tag, length );
    }
    return ev;
}

/*!
    Returns the PDU form of this SIM control event, encoded as described in 3GPP TS 11.14.

    \sa fromPdu()
*/
QByteArray QSimControlEvent::toPdu() const
{
    QByteArray data;
    QByteArray extData = extensionData();

    // Extract parameters that need to appear before the text string,
    // according to 3GPP TS 11.14, sections 9.1.6 and 9.2.2.
    if ( type() == Call ) {
        extractAndWriteExtField( data, extData, 0x06 );     // Address
        extractAndWriteExtField( data, extData, 0x09 );     // SS string
        extractAndWriteExtField( data, extData, 0x0A );     // USSD string
        extractAndWriteExtField( data, extData, 0x07 );     // Capability configuration 1
        extractAndWriteExtField( data, extData, 0x08 );     // Subaddress
    } else {
        extractAndWriteExtField( data, extData, 0x06 );     // Address 1
        extractAndWriteExtField( data, extData, 0x06 );     // Address 2
    }

    // Output the text string.
    if ( !text().isEmpty() )
        writeEFADN( data, text(), QSimCommand::NoPduOptions );

    // Add any remaining extension data that is specified.
    data += extData;

    // Add the outermost tag layer and return.
    QByteArray outer;
    outer += (char)result();
    writeBerLength( outer, data.size() );
    outer += data;
    return outer;
}

/*!
    Copy the QSimControlEvent \a value.
*/
QSimControlEvent& QSimControlEvent::operator=( const QSimControlEvent &value )
{
    if ( d == value.d )
        return *this;
    delete d;
    d = new QSimControlEventPrivate( value.d );
    return *this;
}

/*!
    \fn void QSimControlEvent::deserialize(Stream &value)

    \internal

    Deserializes the QSimControlEvent instance out to a template
    type \c{Stream} \a stream.
 */
template <typename Stream> void QSimControlEvent::deserialize(Stream &stream)
{
    d->read( stream );
}

/*!
    \fn void QSimControlEvent::serialize(Stream &value) const

    \internal

    Serializes the QSimControlEvent instance out to a template
    type \c{Stream} \a stream.
 */
template <typename Stream> void QSimControlEvent::serialize(Stream &stream) const
{
    d->write( stream );
}

Q_IMPLEMENT_USER_METATYPE(QSimControlEvent)
