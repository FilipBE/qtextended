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

#include <stdlib.h>

#include <qsmsmessage.h>
#include "qsmsmessage_p.h"
#include <qatutils.h>
#include <qgsmcodec.h>
#ifdef Q_WS_QWS
#include <qtopialog.h>
#else
#include <qdebug.h>
#define qLog(dbgcat) if(1); else qDebug()
#endif
#include <qstringlist.h>

#include <qtextcodec.h>

const int Unit = 1;

class QSMSMessagePartPrivate
{
public:
    QSMSMessagePartPrivate( const QString& text )
    {
        this->isText = true;
        this->text = text;
        this->position = 0;
    }
    QSMSMessagePartPrivate( const QString& mimeType, const QByteArray& data )
    {
        this->isText = false;
        this->mimeType = mimeType;
        this->data = data;
        this->position = 0;
    }
    QSMSMessagePartPrivate( const QString& mimeType, const QByteArray& data, uint position )
    {
        this->isText = false;
        this->mimeType = mimeType;
        this->data = data;
        this->position = position;
    }
    QSMSMessagePartPrivate( const QSMSMessagePartPrivate& part )
    {
        this->isText = part.isText;
        this->text = part.text;
        this->mimeType = part.mimeType;
        this->data = part.data;
        this->position = part.position;
    }

    bool isText;
    QString text;
    QString mimeType;
    QByteArray data;
    uint position;
};

class QSMSMessagePrivate
{
public:
    QSMSMessagePrivate()
    {
        ref = 1;
        mValidity = 2 * 24 * 60;        // 2 days
        mReplyRequest = false;
        mStatusReportRequested = false;
        mMessageType = QSMSMessage::Normal;
        mCodec = 0;
        mForceGsm = false;
        mBestScheme = (QSMSDataCodingScheme)(-1);
        mDataCodingScheme = -1;
        mMessageClass = -1;
        mProtocol = 0;
    }

    ~QSMSMessagePrivate()
    {
    }

    /*  Convert from minutes to GSM 03.40 specification (section 9.2.3.12).
         - 0-143   = 0 to 12 hours in 5 minute increments (0 = 5 minutes).
         - 144-167 = 12hrs30min to 24hrs in 30 minute increments.
         - 168-196 = 2days to 30days in 1 day increments.
         - 197-255 = 5weeks to 63weeks in 1 week increments.
    */
    uint gsmValidityPeriod()
    {
        uint mins = mValidity;

        if ( mins < 5 )
            return 0;

        if ( mins <= 12 * 60 )
            return ((mins / 5) - 1);

        if ( mins <= 24 * 60 )
            return ((mins + 29 - (12 * 60 + 30)) / 30) + 144;

        uint days = (mins + (24 * 60 - 1)) / (24 * 60);

        if ( days <= 30 )
            return days - 2 + 168;

        if ( days <= 63 * 7 )
            return ((days + 6) / 7) - 5 + 197;

        return 255;
    }

    void setGsmValidityPeriod(uint value)
    {
        if ( value <= 143 )
            mValidity = (value + 1) * 5;
        else if ( value <= 167 )
            mValidity = (value - 143) * 30 + 12 * 60;
        else if ( value <= 196 )
            mValidity = (value - 166) * 24 * 60;
        else
            mValidity = (value - 192) * 7 * 24 * 60;
    }

    template <typename Stream> void readFromStream( Stream &s )
    {
        QString codec;
        s >> codec;
        if ( codec.length() > 0)
            mCodec = QTextCodec::codecForName( codec.toLatin1() );
        else
            mCodec = 0;
        s >> mServiceCenter;
        s >> mRecipient;
        s >> mSender;
        s >> mTimestamp;
        s >> mValidity;
        int val;
        s >> val;
        mReplyRequest = (val != 0);
        s >> val;
        mStatusReportRequested = (val != 0);
        s >> val;
        mForceGsm = (val != 0);
        s >> val;
        mMessageType = (QSMSMessage::MessageType)val;
        s >> val;
        mBestScheme = (QSMSDataCodingScheme)val;
        s >> mHeaders;
        s >> mParts;
        mCachedBody = QString();
        s >> mDataCodingScheme;
        s >> mMessageClass;
        s >> mProtocol;
    }

    template <typename Stream> void writeToStream( Stream &s )
    {
        if ( mCodec )
            s << QString( mCodec->name() );
        else
            s << QString( "" );
        s << mServiceCenter;
        s << mRecipient;
        s << mSender;
        s << mTimestamp;
        s << mValidity;
        s << (int)mReplyRequest;
        s << (int)mStatusReportRequested;
        s << (int)mForceGsm;
        s << (int)mMessageType;
        s << (int)mBestScheme;
        s << mHeaders;
        s << mParts;
        s << mDataCodingScheme;
        s << mMessageClass;
        s << mProtocol;
    }

    void copy( QSMSMessagePrivate *from )
    {
        *this = *from;
        mHeaders = from->mHeaders;
    }

    QAtomicInt ref;
    QTextCodec *mCodec;
    QString mServiceCenter;
    QString mRecipient;
    QString mSender;
    QDateTime mTimestamp;
    uint mValidity;
    bool mReplyRequest;
    bool mStatusReportRequested;
    bool mForceGsm;
    QSMSMessage::MessageType mMessageType;
    QSMSDataCodingScheme mBestScheme;
    QByteArray mHeaders;
    QList<QSMSMessagePart> mParts;
    QString mCachedBody;
    int mDataCodingScheme;
    int mMessageClass;
    int mProtocol;
};

/*!
    \enum QSMSMessage::MessageType
    Defines the type of an SMS message.

    \value Normal The message is a normal SMS message.
    \value CellBroadCast The message is a cell broadcast message.
    \value StatusReport The message is an SMS status report message.
*/

/*!
    \enum QSMSDataCodingScheme
    Define the data coding scheme to use to encode SMS message text.

    \value QSMS_Compressed Flag that indicates that the data is compressed
           (not used at present).
    \value QSMS_MessageClass Flag that indicates the message class
           (not used at present).
    \value QSMS_DefaultAlphabet Use the default 7-bit GSM alphabet.
    \value QSMS_8BitAlphabet Use the locale-specific 8-bit alphabet.
    \value QSMS_UCS2Alphabet Use the UCS-2 16-bit alphabet.
    \value QSMS_ReservedAlphabet Use the reserved alphabet
           (not used at present).
*/

/*!
    \class QSMSMessagePart
    \inpublicgroup QtTelephonyModule

    \brief The QSMSMessagePart class specifies a part within an SMS message.

    \ingroup telephony
    \sa QSMSMessage

    QSMSMessage objects contain zero or more "parts", which describe
    the contents of the SMS message.  A part may be plain text,
    or a binary stream tagged by a MIME type.
*/

/*!
    Constructs an empty SMS message part.
*/
QSMSMessagePart::QSMSMessagePart()
{
    d = new QSMSMessagePartPrivate( QString() );
}

/*!
    Constructs a new plain text SMS message part from the
    string \a text.
*/
QSMSMessagePart::QSMSMessagePart( const QString& text )
{
    d = new QSMSMessagePartPrivate( text );
}

/*!
    Constructs a new binary SMS message part with the specified
    \a mimeType and \a data.
*/
QSMSMessagePart::QSMSMessagePart( const QString& mimeType, const QByteArray& data )
{
    d = new QSMSMessagePartPrivate( mimeType, data );
}

/*!
    Constructs a new binary SMS message part with the specified
    \a mimeType and \a data.  The part is intended to be displayed
    at \a position within a subsequent text part.
*/
QSMSMessagePart::QSMSMessagePart( const QString& mimeType, const QByteArray& data, uint position )
{
    d = new QSMSMessagePartPrivate( mimeType, data, position );
}

/*!
    Constructs a copy of \a part.
*/
QSMSMessagePart::QSMSMessagePart( const QSMSMessagePart& part )
{
    d = new QSMSMessagePartPrivate( *(part.d) );
}

/*!
    Destructs the QSMSMessagePart.
*/
QSMSMessagePart::~QSMSMessagePart()
{
    delete d;
}

/*!
    Assigns a copy of \a part to this object.
*/
QSMSMessagePart& QSMSMessagePart::operator=( const QSMSMessagePart& part )
{
    if ( &part != this ) {
        delete d;
        d = new QSMSMessagePartPrivate( *(part.d) );
    }
    return *this;
}

/*!
    Returns true if this SMS message part is plain text; or false if binary.
*/
bool QSMSMessagePart::isText() const
{
    return d->isText;
}

/*!
    Returns the plain text contents of this SMS message part,
    or QString() if it is not a text part.
*/
QString QSMSMessagePart::text() const
{
    return d->text;
}

/*!
    Returns the MIME type associated with this SMS message part,
    or QString() if it is not a binary part.
*/
QString QSMSMessagePart::mimeType() const
{
    return d->mimeType;
}

/*!
    Returns the binary data associated with this SMS message part.
    Returns and empty QByteArray if it is not a binary part.
*/
const QByteArray& QSMSMessagePart::data() const
{
    return d->data;
}

/*!
    Returns the text position to display this SMS message part at.
*/
uint QSMSMessagePart::position() const
{
    return d->position;
}

/*!
    \internal
    \fn void QSMSMessagePart::deserialize(Stream &stream)
*/
template <typename Stream> void QSMSMessagePart::deserialize(Stream &stream)
{
    int flag;
    stream >> flag;
    d->isText = (flag != 0);
    stream >> d->text;
    stream >> d->mimeType;
    stream >> d->data;
    stream >> d->position;
}

/*!
    \internal
    \fn void QSMSMessagePart::serialize(Stream &stream) const
*/
template <typename Stream> void QSMSMessagePart::serialize(Stream &stream) const
{
    stream << (int)(d->isText);
    stream << d->text;
    stream << d->mimeType;
    stream << d->data;
    stream << d->position;
}

/*!
    \class QSMSMessage
    \inpublicgroup QtTelephonyModule

    \brief The QSMSMessage class specifies the contents of an SMS message.

    This class is intended for use with QSMSReader and QSMSSender to process
    SMS messages according to 3GPP TS 03.40 and 23.040.

    An incoming SMS message from QSMSReader will typically have text() and
    sender() set.  Other fields such as destinationPort() and
    applicationData() may be set if the message is a WAP Push or SMS datagram
    message rather than plain text.

    An outgoing SMS message sent via QSMSSender will need to have at least
    recipient() and text() set.  If the message is a WAP Push or SMS datagram
    message rather than plain text, then destinationPort() and applicationData()
    should also be set.

    Special header fields in SMS messages can be accessed with serviceCenter(),
    replyRequest(), statusReportRequested(), validityPeriod(), timestamp(),
    dataCodingScheme(), and protocol().

    \ingroup telephony
*/

/*!
    Constructs an empty QSMSMessage.
*/
QSMSMessage::QSMSMessage()
{
    d = new QSMSMessagePrivate;
}

/*!
    Constructs an QSMSMessage that is a copy of \a msg.
*/
QSMSMessage::QSMSMessage(const QSMSMessage &msg)
{
    d = msg.d;
    d->ref.ref();
}

/*!
    Destructs the QSMSMessage.
*/
QSMSMessage::~QSMSMessage()
{
    if ( !d->ref.deref() )
        delete d;
}

QSMSMessagePrivate *QSMSMessage::dwrite()
{
    // If we are the only user of the private object, return it as-is.
    if ( d->ref == 1 )
        return d;

    // Create a new private object and copy the current contents into it.
    QSMSMessagePrivate *newd = new QSMSMessagePrivate();
    newd->copy( d );
    if ( !d->ref.deref() )
        delete d;
    d = newd;
    return newd;
}

/*!
    Assigns a copy of \a msg to this object.
*/
QSMSMessage& QSMSMessage::operator=( const QSMSMessage &msg)
{
    if ( d == msg.d )
        return *this;

    if ( !d->ref.deref() )
        delete d;

    d = msg.d;
    d->ref.ref();

    return *this;
}

/*!
    Sets the contents of this QSMSMessage to a single plain text
    body containing \a str.  This is for simple messages only.
    Complex messages should be constructed part by part using
    the addPart() method.

    \sa text()
*/
void QSMSMessage::setText(const QString &str)
{
    clearParts();
    addPart( QSMSMessagePart( str ) );
}

/*!
    Returns the contents of this QSMSMessage as a single plain text string.
    If the message contains binary parts, they will not appear
    in the result.  This is for simple message viewers only.
    Complex message viewers should iterate over the list returned
    by parts().

    \sa setText()
*/
QString QSMSMessage::text() const
{
    // Handle the easy cases first.
    if ( d->mParts.count() == 0 ) {
        return QString();
    } else if ( d->mParts.count() == 1 && d->mParts[0].isText() ) {
        return d->mParts[0].text();
    } else if ( d->mCachedBody.length() > 0 ) {
        return d->mCachedBody;
    }

    // We need the private structure to be writable to cache the body.
    const_cast<QSMSMessage *>(this)->dwrite();

    // Append all text parts to get the complete body.
    QString body = QString();
    for ( int posn = 0; posn < d->mParts.count(); ++posn ) {
        if ( d->mParts[posn].isText() ) {
            body += d->mParts[posn].text();
        }
    }
    d->mCachedBody = body;
    return body;
}

/*!
    If the message consists solely of characters in the 7-bit GSM
    encoding, then the message will be transmitted that way.
    Otherwise \a codec is used to convert the characters into an
    appropriate language-specific 8-bit encoding.  If \a codec is
    set to NULL, then the default UCS-2 encoding for GSM messages
    is used.

    \sa textCodec()
*/
void QSMSMessage::setTextCodec(QTextCodec *codec)
{
    dwrite()->mCodec = codec;
}

/*!
    Returns the current 8-bit text codec, or NULL if none has
    been set.

    \sa setTextCodec()
*/
QTextCodec *QSMSMessage::textCodec() const
{
    return d->mCodec;
}

/*!
    If \a force is true, then the codec set by QSMSMessage::setTextCodec
    is ignored and the 7-bit GSM encoding is always used.  Setting this
    flag increases the number of characters that can be sent in any
    given SMS message, but may lose information.

    \sa forceGsm()
*/
void QSMSMessage::setForceGsm(bool force)
{
    dwrite()->mForceGsm = force;
}

/*!
    Returns true if the 7-bit GSM encoding has been forced.

    \sa setForceGsm()
*/
bool QSMSMessage::forceGsm() const
{
    return d->mForceGsm;
}

/*!
    Sets the SMS data coding \a scheme to use for this message.
    Normally you won't need to call this unless the user has
    somehow explicitly requested an override.  By default,
    the QSMSMessage class will choose the best scheme for you.
    Should be set to one of \c QSMS_DefaultAlphabet,
    \c QSMS_8BitAlphabet, or \c QSMS_UCS2Alphabet.

    \sa bestScheme()
*/
void QSMSMessage::setBestScheme(QSMSDataCodingScheme scheme)
{
    dwrite()->mBestScheme = scheme;
}


/*!
    Returns the best SMS data coding scheme to use for this
    message, determined by an inspection of the plain text body parts.

    \sa setBestScheme()
*/
QSMSDataCodingScheme QSMSMessage::bestScheme() const
{
    QTextCodec *codec = QAtUtils::codec( "gsm-noloss" );
    QString body = text();
    uint len = body.length();
    bool gsmSafe;

    // Did the user provide a scheme override?
    if ( d->mBestScheme != (QSMSDataCodingScheme)(-1) )
        return d->mBestScheme;

    // Encode zero-length bodies in the default alphabet.
    if ( len == 0 )
        return QSMS_DefaultAlphabet;

    // Always use GSM if we are forced to do so.
    if ( d->mForceGsm )
        return QSMS_DefaultAlphabet;

    // Check the body for non-GSM characters.
    gsmSafe = codec->canEncode( body );

    // Use the default alphabet if everything is GSM-compatible.
    if ( gsmSafe )
        return QSMS_DefaultAlphabet;

    // See if we can convert to 8-bit using the codec
    // without losing any information.
    if ( d->mCodec && d->mCodec->canEncode( body ) )
        return QSMS_8BitAlphabet;

    // Default to the UCS-2 alphabet.
    return QSMS_UCS2Alphabet;
}

/*!
    Sets the recipient's telephone number to \a txt.

    \sa recipient()
*/
void QSMSMessage::setRecipient(const QString &txt)
{
    dwrite()->mRecipient = txt;
}

/*!
    Returns the recipient's telephone number.  Normally QString()
    for an incoming message.

    \sa setRecipient()
*/
QString QSMSMessage::recipient() const
{
    return d->mRecipient;
}

/*!
    Sets the sender's telephone number to \a txt.

    \sa sender()
*/
void QSMSMessage::setSender(const QString &txt)
{
    dwrite()->mSender = txt;
}

/*!
    Returns the sender's telephone number.  Normally QString()
    for an outgoing message.

    \sa setSender()
*/
QString QSMSMessage::sender() const
{
    return d->mSender;
}

/*!
    Sets the service center to use for transmitting this SMS message,
    or QString() for the default service center, to \a str.

    \sa serviceCenter()
*/
void QSMSMessage::setServiceCenter(const QString &str)
{
    dwrite()->mServiceCenter = str;
}

/*!
    Returns the service center.

    \sa setServiceCenter()
*/
QString QSMSMessage::serviceCenter() const
{
    return d->mServiceCenter;
}

/*!
    Enable or disable the "reply request" flag for this SMS message,
    according to the value of \a on.

    \sa replyRequest()
*/
void QSMSMessage::setReplyRequest(bool on )
{
    dwrite()->mReplyRequest = on;
}

/*!
    Returns the "reply request" flag.

    \sa setReplyRequest()
*/
bool QSMSMessage::replyRequest() const
{
    return d->mReplyRequest;
}

/*!
    Sets the status report requested flag to \a on.

    \sa statusReportRequested()
*/
void QSMSMessage::setStatusReportRequested(bool on)
{
    dwrite()->mStatusReportRequested = on;
}

/*!
    Returns true if status report requested flag is currently set;otherwise returns false.

    \sa setStatusReportRequested()
*/
bool QSMSMessage::statusReportRequested() const
{
    return d->mStatusReportRequested;
}

/*!
    Sets the validity period to \a minutes.  The default is 2 days.  If the value
    is set to \c{(uint)(-1)}, it indicates that the message should have no
    validity period specified.

    \sa validityPeriod(), gsmValidityPeriod()
*/
void QSMSMessage::setValidityPeriod(uint minutes)
{
    dwrite()->mValidity = minutes;
}

/*!
    Returns the validity period in minutes for this message.  The default is 2 days.
    If the value is \c{(uint)(-1)}, it indicates that the message should have no
    validity period specified.

    \sa setValidityPeriod(), setGsmValidityPeriod()
*/
uint QSMSMessage::validityPeriod() const
{
    return d->mValidity;
}

/*!
    Sets the GSM validity period to \a value, which must be between
    0 and 255, inclusive.  The setValidity() method
    is a friendlier way to set the validity value.

    0 to 143 indicates 0 to 12 hours in 5 minute increments (0 = 5 minutes).
    144 to 167 indicates 12 hrs 30 min to 24 hrs in 30 minute increments.
    168 to 196 indicates 2 days to 30 days in 1 day increments.
    197 to 255 indicates 5 weeks to 63 weeks in 1 week increments.

    \sa gsmValidityPeriod(), validityPeriod()
*/
void QSMSMessage::setGsmValidityPeriod(uint value)
{
    dwrite()->setGsmValidityPeriod(value);
}

/*!
    Returns the GSM validity period value, between 0 and 255, inclusive.

    \sa setGsmValidityPeriod(), setValidityPeriod()
*/
uint QSMSMessage::gsmValidityPeriod() const
{
    return d->gsmValidityPeriod();
}

/*!
    Sets the SMS message's \a timestamp.

    \sa timestamp()
*/
void QSMSMessage::setTimestamp(const QDateTime& timestamp)
{
    dwrite()->mTimestamp = timestamp;
}

/*!
    Returns the SMS message's timestamp, which will be null if the
    message does not have a timestamp.

    \sa setTimestamp()
*/
QDateTime QSMSMessage::timestamp() const
{
    return d->mTimestamp;
}

/*!
    Returns the SMS message type.

    \sa setMessageType()
*/
QSMSMessage::MessageType QSMSMessage::messageType() const
{
    return d->mMessageType;
}

/*!
    Sets the SMS message type to \a m.  There is rarely any need to
    set this to something other than QSMSMessage::Normal.

    \sa messageType()
*/
void QSMSMessage::setMessageType(MessageType m)
{
    dwrite()->mMessageType  = m;
}

/*!
    Sets the SMS message's user data headers to \a value.

    \sa headers()
*/
void QSMSMessage::setHeaders(const QByteArray& value)
{
    dwrite()->mHeaders = value;
}

/*!
    Returns the SMS message's user data headers.

    \sa setHeaders()
*/
const QByteArray& QSMSMessage::headers() const
{
    return d->mHeaders;
}

/*!
    Clear all body parts from this SMS message.

    \sa addPart(), addParts(), parts()
*/
void QSMSMessage::clearParts()
{
    dwrite()->mParts.clear();
    dwrite()->mCachedBody = QString();
}

/*!
    Add a new body \a part to this SMS message.

    \sa clearParts(), addParts(), parts()
*/
void QSMSMessage::addPart( const QSMSMessagePart& part )
{
    // We need a writable copy.
    dwrite();

    // Clear the part list if we have one empty text part.
    if ( d->mParts.count() == 1 &&
         d->mParts[0].isText() &&
         d->mParts[0].text().length() == 0 ) {
        d->mParts.clear();
    }

    // Append the new part and clear the cached text.
    d->mParts.append( part );
    d->mCachedBody = QString();
}

/*!
    Add a list of body \a parts to this SMS message.

    \sa clearParts(), addPart(), parts()
*/
void QSMSMessage::addParts( const QList<QSMSMessagePart>& parts )
{
    // We need a writable copy.
    dwrite();

    if ( d->mParts.count() == 1 &&
         d->mParts[0].isText() &&
         d->mParts[0].text().length() == 0 ) {
        d->mParts.clear();
    }
    d->mParts += parts;
    d->mCachedBody = QString();
}

/*!
    Returns a list of all body parts in this SMS message.

    \sa clearParts(), addParts(), addPart()
*/
QList<QSMSMessagePart> QSMSMessage::parts() const
{
    return d->mParts;
}

/*!
    Compute an estimate for the number of messages that will need
    to be used to send this SMS message (\a numMessages), and the
    number of spare characters that are left in the last message
    before it overflows (\a spaceLeftInLast).

    This function may be useful in user interfaces to indicate to
    the user that an SMS message needs to be sent in multiple pieces,
    costing the user more money.
*/
void QSMSMessage::computeSize( uint& numMessages, uint& spaceLeftInLast ) const
{
    int part = findPart( "application/x-qtopia-wdp-ports" );
    if ( part != -1 ) {
        // This is an application datagram, which has its size
        // computed using a different algorithm.
        uint headerLen = 3 + d->mParts[(uint)part].data().size();
        uint dataLen = applicationData().size();
        if ( ( headerLen + dataLen ) <= 140 ) {
            numMessages = 1;
            spaceLeftInLast = 134 - headerLen - dataLen;
        } else {
            uint partSize = ( 134 - headerLen );
            numMessages = ( dataLen + partSize - 1 ) / partSize;
            spaceLeftInLast = dataLen - numMessages * partSize;
        }
        return;
    }

    QSMSDataCodingScheme scheme = bestScheme();
    uint len;
    QString body = text();

    if ( scheme == QSMS_DefaultAlphabet ) {

        // Encode the message using 7-bit GSM.
        len = body.length();
        if ( len <= 160 ) {
            numMessages = 1;
            spaceLeftInLast = 160 - len;
        } else {
            // 153 = 160 - fragment_header_size (7).
            numMessages = ( len + 152 ) / 153;
            len %= 153;
            if ( len != 0 )
                spaceLeftInLast = 153 - len;
            else
                spaceLeftInLast = 0;
        }

    } else if ( scheme == QSMS_8BitAlphabet ) {

        // Encode the message using an 8-bit character set.
        QByteArray converted = d->mCodec->fromUnicode( body );
        len = converted.length();
        if ( len <= 140 ) {
            numMessages = 1;
            spaceLeftInLast = 140 - len;
        } else {
            // 134 = 140 - fragment_header_size (6).
            numMessages = ( len + 133 ) / 134;
            len %= 134;
            if ( len != 0 )
                spaceLeftInLast = 134 - len;
            else
                spaceLeftInLast = 0;
        }

    } else {

        // Encode the message with unicode.
        len = body.length();
        if ( len <= 70 ) {
            numMessages = 1;
            spaceLeftInLast = 70 - len;
        } else {
            // 67 = 70 - fragment_header_size (3).
            numMessages = ( len + 66 ) / 67;
            len %= 67;
            if ( len != 0 )
                spaceLeftInLast = 67 - len;
            else
                spaceLeftInLast = 0;
        }
    }
}

/*!
    Returns the destination port number if this SMS message contains
    an application datagram, or -1 if not an application datagram.

    When an SMS message is received that has a destination port
    number, Qt Extended will attempt to find a QDS service that can handle it.
    Qt Extended first looks for a QDS service named \c push for the MIME type
    \c T from the WAP push header.  Next, it looks for a service named
    \c push for the MIME type \c{application/x-smsapp-N} where
    \c N is the port number in decimal.

    If a matching service is found, then the SMS message is
    sent to the corresponding application via QDS.  The QCop
    message is that specified in the QDS service definition.
    Thus, applications can register to receive special SMS messages.

    The following QDS definition, in \c{etc/qds/ContactsPhone} will
    direct vcard's that are received via WAP to the \c ContactsPhone
    service.  The \c ContactsPhone service is normally implemented
    by the \c addressbook program.

    \code
    [Translation]
    File=QtopiaServices
    Context=ContactsPhone
    [pushVCard]
    RequestDataType=text/x-vcard
    ResponseDataType=
    Attributes="push"
    Description[]=Receive a vcard via WAP push
    \endcode

    The \c Attributes must contain \c push and the \c RequestDataType
    must be the MIME type to be dispatched.  The QCop message that is
    delivered to the application will have the name
    \c pushVCard(QDSActionRequest).  The data in the action request
    will be the payload of the push message.

    The auxilary data in the action request will be a QByteArray
    containing the full QSMSMessage object, from which the application
    can extra header information if it needs it.  Use the QSMSMessage
    datastream operators to extract the QSMSMessage object.

    If the application fails to process an SMS message that is sent to
    it via QCop, then it will be lost.  It is important that such
    applications take steps to save the message if necessary.

    If a matching service is not found, then the SMS message
    will be delivered to the \c qtmail application normally,
    and then saved by that application.

    \sa setDestinationPort(), sourcePort()
*/
int QSMSMessage::destinationPort() const
{
    int part = findPart( "application/x-qtopia-wdp-ports" );
    if ( part == -1 )
        return -1;

    QByteArray data = d->mParts[(uint)part].data();
    if ( data.size() == 4 ) {

        return ((((int)(data[0])) & 0xFF) << 8) |
                (((int)(data[1])) & 0xFF);

    } else if ( data.size() == 2 ) {

        return (((int)(data[0])) & 0xFF);
    }

    return -1;
}

/*!
    Sets the destination port number for an SMS message that contains
    an application datagram to \a value.

    \sa destinationPort(), sourcePort()
*/
void QSMSMessage::setDestinationPort(int value)
{
    QByteArray data;
    int source;
    int part = findPart( "application/x-qtopia-wdp-ports" );
    if ( part == -1 ) {
        data.resize(4);
        data[0] = (char)(value >> 8);
        data[1] = (char)value;
        data[2] = (char)0;
        data[3] = (char)0;
    } else {
        source = sourcePort();
        data.resize(4);
        data[0] = (char)(value >> 8);
        data[1] = (char)value;
        data[2] = (char)(source >> 8);
        data[3] = (char)source;
    }
    removeParts( "application/x-qtopia-wdp-ports" );
    addPart( QSMSMessagePart( "application/x-qtopia-wdp-ports", data ) );
}

/*!
    Returns the source port number if this SMS message contains
    an application datagram, or -1 if not an application datagram.

    \sa setSourcePort(), destinationPort()
*/
int QSMSMessage::sourcePort() const
{
    int part = findPart( "application/x-qtopia-wdp-ports" );
    if ( part == -1 )
        return -1;

    QByteArray data = d->mParts[(uint)part].data();
    if ( data.size() == 4 ) {

        return ((((int)(data[2])) & 0xFF) << 8) |
                (((int)(data[3])) & 0xFF);

    } else if ( data.size() == 2 ) {

        return (((int)(data[1])) & 0xFF);
    }

    return -1;
}

/*!
    Sets the source port number for an SMS message that contains
    an application datagram to \a value.

    \sa sourcePort(), destinationPort()
*/
void QSMSMessage::setSourcePort(int value)
{
    QByteArray data;
    int dest;
    int part = findPart( "application/x-qtopia-wdp-ports" );
    if ( part == -1 ) {
        data.resize(4);
        data[0] = (char)0;
        data[1] = (char)0;
        data[2] = (char)(value >> 8);
        data[3] = (char)value;
    } else {
        dest = destinationPort();
        data.resize(4);
        data[0] = (char)(dest >> 8);
        data[1] = (char)dest;
        data[2] = (char)(value >> 8);
        data[3] = (char)value;
    }
    removeParts( "application/x-qtopia-wdp-ports" );
    addPart( QSMSMessagePart( "application/x-qtopia-wdp-ports", data ) );
}

/*!
    Returns the data if this SMS message contains an application datagram.
    Returns an empty byte array if this message is not a datagram.

    \sa setApplicationData()
*/
QByteArray QSMSMessage::applicationData() const
{
    QByteArray data;
    QList<QSMSMessagePart>::ConstIterator iter;
    uint size;
    for ( iter = d->mParts.begin(); iter != d->mParts.end(); ++iter ) {
        if ( (*iter).mimeType() == "application/x-qtopia-wdp" ) {
            size = data.size();
            data.resize( size + (*iter).data().size() );
            memcpy( data.data() + size, (*iter).data().data(),
                    (*iter).data().size() );
        }
    }
    return data;
}

/*!
    Sets the data within an SMS message that contains an application
    datagram to \a value.

    \sa applicationData()
*/
void QSMSMessage::setApplicationData(const QByteArray& value)
{
    removeParts( "application/x-qtopia-wdp" );
    addPart( QSMSMessagePart( "application/x-qtopia-wdp", value ) );
}

/*!
    Sets the data coding scheme to use within an SMS message to \a value.
    If \a value is -1, then the system chooses the best data coding scheme
    based on the content.

    This method is mainly of use with application datagrams, not text
    SMS messages.

    \sa dataCodingScheme()
*/
void QSMSMessage::setDataCodingScheme(int value)
{
    dwrite()->mDataCodingScheme = value;
}

/*!
    Returns the data coding scheme to use within an SMS message.
    If the value is -1, then the system chooses the best data coding scheme
    based on the content.

    This method is mainly of use with application datagrams, not text
    SMS messages.

    \sa setDataCodingScheme()
*/
int QSMSMessage::dataCodingScheme() const
{
    return d->mDataCodingScheme;
}

/*!
    Sets the message class for this message to \a value.  The \value should
    be -1 if the system should choose a default message class.  The message
    class should otherwise be 0, 1, 2, or 3, according to 3GPP TS 03.38.
*/
void QSMSMessage::setMessageClass(int value)
{
    dwrite()->mMessageClass = value;
}

/*!
    Get the message class for this message, or -1 if the system should
    choose a default message class.  The message class should otherwise
    be 0, 1, 2, or 3, according to 3GPP TS 03.38.
*/
int QSMSMessage::messageClass() const
{
    return d->mMessageClass;
}

/*!
    Sets the SMS message's protocol field to \a value.

    \sa protocol()
*/
void QSMSMessage::setProtocol(int value)
{
    dwrite()->mProtocol = value;
}

/*!
    Returns the SMS message's protocol field.

    \sa setProtocol()
*/
int QSMSMessage::protocol() const
{
    return d->mProtocol;
}

/*!
    Returns true if this message needs to be split into multiple messages
    before being transmitted over a GSM network; otherwise returns false.

    \sa split()
*/
bool QSMSMessage::shouldSplit() const
{
    uint numMessages, spaceLeftInLast;
    this->computeSize( numMessages, spaceLeftInLast );
    return ( numMessages <=1 ? false : true );
}

/*!
    Split this message into several messages of smaller size for
    transmission over a GSM network.

    \sa shouldSplit()
*/
QList<QSMSMessage> QSMSMessage::split() const
{
    QList<QSMSMessage> list;
    uint numMessages, spaceLeftInLast;
    static uint fragmentCounter =0;

    computeSize( numMessages, spaceLeftInLast );
    if ( numMessages <= 1 ) {
        // Splitting is not necessary, so return a list with one message.
        list += *this;
        return list;
    }

    // Get the number of characters to transmit in each fragment.
    int split;
    QSMSDataCodingScheme scheme = bestScheme();
    switch ( scheme ) {
        case QSMS_DefaultAlphabet:  split = 153; break;
        case QSMS_8BitAlphabet:     split = 134; break;
        default:                    split = 67; break;
    }

    // Split the message to create sub-messages and transmit them.
    int posn = 0;
    int len;
    uint number;
    QSMSMessage tmp;
    number = 1;
    if ( destinationPort() == -1 ) {
        // Splitting a simple text message.
        QString txt = text();
        while ( posn < txt.length() ) {
            tmp = *this;
            len = txt.length() - posn;
            if ( len > split ) {
                len = split;
            }
            tmp.setText( txt.mid( posn, len ) );
            tmp.setFragmentHeader( fragmentCounter, number++,
                                   numMessages, scheme );
            posn += len;
            list.append(tmp);
        }
    } else {
        // Splitting a datagram message.
        QByteArray data = applicationData();
        QByteArray part;
        uint partSize = 134 - 6;
        while ( posn < data.size() ) {
            tmp = *this;
            if ( ( posn + partSize ) <= (uint)data.size() ) {
                part.resize(partSize);
                memcpy(part.data(), data.data() + posn, partSize );
            } else {
                part.resize(data.size() - posn);
                memcpy(part.data(), data.data() + posn, data.size() - posn);
            }
            tmp.setDestinationPort( destinationPort() );// Force 16-bit ports.
            tmp.setFragmentHeader( fragmentCounter, number++,
                                   numMessages, QSMS_8BitAlphabet );
            tmp.setApplicationData( part );
            list.append(tmp);
            posn += partSize;
        }
    }

    // Increase the fragment counter for the next multi-part SMS message.
    fragmentCounter = ( fragmentCounter + 1 ) & 0xFF;

    return list;
}

/*!
    Convert this SMS message into its binary PDU form, according to
    3GPP TS 03.40 and 3GPP TS 23.040.  If the message has a recipient,
    then a SUBMIT message will be constructed.  If the message does not
    have a recipient, then a DELIVER message will be constructed.

    \sa fromPdu()
*/
QByteArray QSMSMessage::toPdu() const
{
    QSMSSubmitMessage submit( *this, recipient().isEmpty() );
    return submit.toByteArray();
}

/*!
    Convert a binary \a pdu into an SMS message, according to
    3GPP TS 03.40 and 3GPP TS 23.040.

    \sa toPdu()
*/
QSMSMessage QSMSMessage::fromPdu( const QByteArray& pdu )
{
    QSMSDeliverMessage pdumsg( pdu );
    return pdumsg.unpack();
}

/*!
    Returns the length of the service center address on the start of \a pdu.
    This is typically used with AT-based GSM modems that need to transmit
    the length of the pdu, excluding the service center address,
    along with the \c{AT+CMGS} command.
*/
int QSMSMessage::pduAddressLength( const QByteArray& pdu )
{
    if( pdu.length() > 0 )
        return (pdu[0] & 0xFF) + 1;
    else
        return 0;
}

int QSMSMessage::findPart( const QString& mimeType ) const
{
    QList<QSMSMessagePart>::ConstIterator iter;
    int posn = 0;
    for ( iter = d->mParts.begin(); iter != d->mParts.end(); ++iter ) {
        if ( (*iter).mimeType() == mimeType )
            return posn;
        ++posn;
    }
    return -1;
}

void QSMSMessage::removeParts( const QString& mimeType )
{
    dwrite();
    QList<QSMSMessagePart>::Iterator iter;
    for ( iter = d->mParts.begin(); iter != d->mParts.end(); ) {
        if ( (*iter).mimeType() == mimeType ) {
            iter = d->mParts.erase( iter );
        } else {
            ++iter;
        }
    }
}

void QSMSMessage::setFragmentHeader( uint refNum, uint part, uint numParts,
                                    QSMSDataCodingScheme scheme )
{
    dwrite()->mBestScheme = scheme;
    uint len = d->mHeaders.size();
    d->mHeaders.resize( len + 5 );
    d->mHeaders[len++] = 0;         // Type for concatenated short messages.
    d->mHeaders[len++] = 3;         // Length of header information.
    d->mHeaders[len++] = (char)refNum;
    d->mHeaders[len++] = (char)numParts;
    d->mHeaders[len++] = (char)part;
}


void QSMSMessage::unpackHeaderParts()
{
    QByteArray headers = dwrite()->mHeaders;
    uint posn = 0;
    uint tag, len;
    uint temp;
    QString type;
    while ( ( posn + 2 ) <= (uint)(headers.size()) ) {
        tag = (unsigned char)(headers[posn]);
        len = (unsigned char)(headers[posn + 1]);
        if ( ( posn + len + 2 ) > (uint)(headers.size()) )
            break;
        switch ( (SMSHeaderKind)tag ) {

            case SMS_HK_Predefined_Sound:
            {
                // Predefined sound type.
                if ( len >= 2 ) {
                    QByteArray data( len - 1, 0 );
                    memcpy( data.data(), headers.data() + posn + 3, len - 1 );
                    addPart( QSMSMessagePart
                        ( "application/x-qtopia-predefined-sound", data,
                          headers[posn + 2] & 0xFF ) );
                }
            }
            break;

            case SMS_HK_User_Defined_Sound:
            {
                // User defined sound type.
                if ( len >= 2 ) {
                    QByteArray data( len - 1, 0 );
                    memcpy( data.data(), headers.data() + posn + 3, len - 1 );
                    addPart( QSMSMessagePart
                        ( "audio/imelody", data, headers[posn + 2] & 0xFF ) );
                }
            }
            break;

            case SMS_HK_Predefined_Animation:
            {
                // Predefined animation type.
                if ( len >= 2 ) {
                    QByteArray data( len - 1, 0 );
                    memcpy( data.data(), headers.data() + posn + 3, len - 1 );
                    addPart( QSMSMessagePart
                        ( "application/x-qtopia-predefined-animation", data,
                          headers[posn + 2] & 0xFF ) );
                }
            }
            break;

            case SMS_HK_Large_Animation:
            case SMS_HK_Large_Picture:
            {
                // 32x32 monochrome animation or image - turn it into a WBMP.
                if ( len >= 1 ) {
                    QByteArray data( len + 3, 0 );
                    data[0] = 0x00;
                    data[1] = 0x00;
                    data[2] = 0x20;
                    data[3] = 0x20;
                    for ( temp = 0; temp < (len - 1); ++temp ) {
                        data[4 + temp] = (char)(~(headers[posn + 3 + temp]));
                    }
                    addPart( QSMSMessagePart
                        ( "image/vnd.wap.wbmp", data,
                          headers[posn + 2] & 0xFF ) );
                }
            }
            break;

            case SMS_HK_Small_Animation:
            case SMS_HK_Small_Picture:
            {
                // 16x16 monochrome animation or image - turn it into a WBMP.
                if ( len >= 1 ) {
                    QByteArray data( len + 3, 0 );
                    data[0] = 0x00;
                    data[1] = 0x00;
                    data[2] = 0x10;
                    data[3] = 0x10;
                    for ( temp = 0; temp < (len - 1); ++temp ) {
                        data[4 + temp] = (char)(~(headers[posn + 3 + temp]));
                    }
                    addPart( QSMSMessagePart
                        ( "image/vnd.wap.wbmp", data,
                          headers[posn + 2] & 0xFF ) );
                }
            }
            break;

            case SMS_HK_Variable_Picture:
            {
                // Variable-sized monochrome image - turn it into a WBMP.
                if ( len >= 3 ) {
                    QByteArray data( len - 1, 0 );
                    data[0] = 0x00;
                    data[1] = 0x00;
                    data[2] = headers[posn + 3];
                    data[3] = headers[posn + 4];
                    for ( temp = 2; temp < (len - 1); ++temp ) {
                        data[2 + temp] = (char)(~(headers[posn + 3 + temp]));
                    }
                    addPart( QSMSMessagePart
                        ( "image/vnd.wap.wbmp", data,
                          headers[posn + 2] & 0xFF ) );
                }
            }
            break;

            case SMS_HK_Concat_8Bit:    break;
            case SMS_HK_Concat_16Bit:   break;

            case SMS_HK_AppPort_8Bit:
            case SMS_HK_AppPort_16Bit:
            {
                QByteArray data( len, 0 );
                memcpy( data.data(), headers.data() + posn + 2, len );
                addPart( QSMSMessagePart
                    ( "application/x-qtopia-wdp-ports", data ) );
            }
            break;

            default:
            {
                // Add the unknown part as "application/x-qtopia-sms-N".
                // Maybe qtmail will know what to do with it.
                QByteArray data( len, 0 );
                memcpy( data.data(), headers.data() + posn + 2, len );
                addPart( QSMSMessagePart( "application/x-qtopia-sms-" +
                                         QString::number( tag ), data ) );
            }
            break;
        }
        posn += 2 + len;
    }
}

/*!
    \internal
    \fn void QSMSMessage::deserialize(Stream &stream)
*/
template <typename Stream> void QSMSMessage::deserialize(Stream &stream)
{
    dwrite()->readFromStream( stream );
}

/*!
    \internal
    \fn void QSMSMessage::serialize(Stream &stream) const
*/
template <typename Stream> void QSMSMessage::serialize(Stream &stream) const
{
    d->writeToStream( stream );
}

QPDUMessage::QPDUMessage()
{
    mPosn = 0;
    mBits = 0;
}

QPDUMessage::QPDUMessage(const QByteArray &data)
{
    mBuffer = data;
    mPosn = 0;
    mBits = 0;
}

QPDUMessage::~QPDUMessage()
{
}

QPDUMessage::QPDUMessage(const QPDUMessage &msg)
{
    mBuffer = msg.mBuffer;
    mPosn = msg.mPosn;
    mBits = msg.mBits;
}

void QPDUMessage::skipOctet()
{
    if ( mPosn < mBuffer.size() )
        ++mPosn;
}

QByteArray QPDUMessage::getOctets( uint len )
{
    QByteArray result;
    if ( ( mBuffer.size() - mPosn ) < (int)len ) {
        result = mBuffer.mid( mPosn );
        abort();
    } else {
        result = mBuffer.mid( mPosn, (int)len );
        mPosn += len;
    }
    return result;
}

void QPDUMessage::setBit(int b, bool on)
{
    if ( on )
        mBits |= (char)(Unit << b);
    else
        mBits &= (char)(~(Unit << b));
}

void QPDUMessage::setBits(int offset, int len, int val)
{
    uint mask = ((Unit << len) - 1) << offset;
    mBits = (char)((mBits & ~mask) | ((val << offset) & mask));
}

void QPDUMessage::commitBits()
{
    mBuffer += mBits;
    mBits = 0;
}

bool QPDUMessage::bit(int b)
{
    if ( needOctets(1) )
        return (mBuffer[mPosn] & (Unit << b));
    else
        return 0;
}

unsigned char QPDUMessage::bits(int offset, int len)
{
    if ( needOctets(1) )
        return (unsigned char)
            ((mBuffer[mPosn] >> offset) & ((Unit << len) - 1));
    else
        return 0;
}

unsigned char QPDUMessage::getOctet()
{
    if ( needOctets(1) )
        return (unsigned char)(mBuffer[mPosn++]);
    else
        return 0;
}

unsigned char QPDUMessage::peekOctet() const
{
    if ( needOctets(1) )
        return (unsigned char)(mBuffer[mPosn]);
    else
        return 0;
}

// Collapse 8-bit-aligned GSM data to its 7-bit form.
static QByteArray collapse7Bit( const QByteArray& in )
{
    QByteArray out;
    int byte = 0;
    int size = 0;
    for ( int posn = 0; posn < in.length(); ++posn ) {
        for ( int bit = 0; bit < 7; ++bit ) {
            if ( ( in[posn] & (1 << bit) ) != 0 )
                byte |= (1 << size);
            ++size;
            if ( size >= 8 ) {
                out += (char)byte;
                byte = 0;
                size = 0;
            }
        }
    }
    if ( size != 0 ) {
        out += (char)byte;
    }
    return out;
}

void QPDUMessage::setAddress(const QString &strin, bool SCAddress)
{
    SMSAddressType at;
    int len, digit, octet;
    int posn;
    const int maxPhoneNumberLen = 15;
    QString str( strin );
    str.truncate( maxPhoneNumberLen );

    // Determine the address type and length.
    at = SMS_Address_Unknown;
    len = 0;
    for ( posn = 0; posn < str.length(); ++posn ) {
        switch ( str[posn].unicode() ) {
            case '+':
                at = SMS_Address_International;
                break;

            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
            case '8': case '9': case '*': case '#':
            case 'A': case 'B': case 'C': case 'a':
            case 'b': case 'c':
                ++len;
                break;

            default:
                // Probably an alpha-numeric address.
                ++len;
                at = SMS_Address_AlphaNumeric;
                break;
        }
    }
    if ( SCAddress )
        len = (len + 1) / 2;

    // Bail out early if the address is zero-length.
    if ( !len ) {
        appendOctet(0);
        return;
    }

    // Handle alphanumeric address fields.
    if ( at == SMS_Address_AlphaNumeric ) {
        // Convert the address and calculate its encoded length.
        QTextCodec *codec = QAtUtils::codec( "gsm-noloss" );
        QByteArray bytes = collapse7Bit( codec->fromUnicode( strin ) );
        len = bytes.size();

        // Need an extra byte for SCAddress fields.
        if ( SCAddress )
            len++;
        else
            len *= 2;

        // Output the length of the encoded address.
        appendOctet(len);

        // Output the type of number information.
        setBits(0, 4, SMS_NumberId_Unknown);
        setBits(4, 3, at);
        setBit(7, true);
        commitBits();

        // Output the encoded address and exit.
        mBuffer += bytes;
        return;
    }

    // SCAddress len = octets + type specifier
    if ( SCAddress )
        len++;

    appendOctet(len);

    setBits(0, 4, SMS_Phone);
    setBits(4, 3, at);
    setBit(7, true);
    commitBits();

    bool upper4 = false;
    octet = 0;
    for ( posn = 0; posn < str.length(); posn++ ) {
        switch ( str[posn].unicode() ) {
            case '0':           digit = 0; break;
            case '1':           digit = 1; break;
            case '2':           digit = 2; break;
            case '3':           digit = 3; break;
            case '4':           digit = 4; break;
            case '5':           digit = 5; break;
            case '6':           digit = 6; break;
            case '7':           digit = 7; break;
            case '8':           digit = 8; break;
            case '9':           digit = 9; break;
            case '*':           digit = 10; break;
            case '#':           digit = 11; break;
            case 'A': case 'a': digit = 12; break;
            case 'B': case 'b': digit = 13; break;
            case 'C': case 'c': digit = 14; break;
            default:            digit = -1; break;
        }
        if ( digit != -1 ) {
            if ( !upper4 ) {
                octet = digit;
            } else {
                octet |= (digit << 4);
                appendOctet( octet );
            }
            upper4 = !upper4;
        }
    }
    if ( upper4 ) {
        appendOctet( octet | 0xF0 );
    }
}

// Expand the 7-bit GSM data to 8-bit-aligned characters.
static QByteArray expand7Bit( const QByteArray& in )
{
    QByteArray out;
    int byte = 0;
    int size = 0;
    for ( int posn = 0; posn < in.length(); ++posn ) {
        for ( int bit = 0; bit < 8; ++bit ) {
            if ( ( in[posn] & (1 << bit) ) != 0 )
                byte |= (1 << size);
            ++size;
            if ( size >= 7 ) {
                out += (char)byte;
                byte = 0;
                size = 0;
            }
        }
    }
    return out;
}

QString QPDUMessage::address(bool SCAddress)
{
    QString str = "";

    // Get the address length and validate it.
    if ( !needOctets(1) )
        return str;
    uint len = (((uint)(getOctet())) & 0xFF);
    if ( !needOctets(len) ) {
        abort();
        return str;
    }

    if ( len ) {
        SMSAddressType at = (SMSAddressType) bits(4, 3);

        if ( at == SMS_Address_International )
            str += "+";

        skipOctet();
        if ( !SCAddress ) {
                len = len / 2 + (len % 2);
        } else {
            len--;
        }

        if ( at != SMS_Address_AlphaNumeric ) {
            unsigned char c;
            for (int i = 0; i < (int)len; i++) {
                c = peekOctet() & 0x0F;
                str += (char) ('0' + c);
                c = (peekOctet() & 0xF0) >> 4;
                if ( c != 0xf )
                    str += (char) ('0' + c);
                skipOctet();
            }
        } else {
            // Recognize an alphanumeric address in the 7-bit GSM encoding.
            QTextCodec *codec = QAtUtils::codec( "gsm" );
            QByteArray bytes = expand7Bit( getOctets( len ) );
            str += codec->toUnicode( bytes );
        }
    }

    return str;
}

// Return the length of the service centre address.
uint QPDUMessage::addressLength() const
{
    if( mPosn < mBuffer.size() )
        return (((uint)peekOctet()) & 0xFF) + 1;
    else
        return 0;
}

void QPDUMessage::setTimeStamp(const QDateTime &dt)
{
    QDate date = dt.date();
    int tArr[6];
    tArr[0] = date.year();
    tArr[1] = date.month();
    tArr[2] = date.day();

    QTime t = dt.time();
    tArr[3] = t.hour();
    tArr[4] = t.minute();
    tArr[5] = t.second();

    for ( int i = 0; i < 6; i++) {
        appendOctet((unsigned char) ( (((tArr[i]/10)%10) & 0x0F) | (((tArr[i]%10) & 0x0F)<<4) ) );
    }

    appendOctet(0x04); //arbitrary random timezone
}

QDateTime QPDUMessage::timeStamp()
{
    QDateTime d;
    unsigned char c, c4, date[7];

    if ( !needOctets(7) ) {
        abort();
        return d;
    }
    for (int i = 0; i < 7; i++) {
        c = (peekOctet() & 0x0F);
        c4 = (peekOctet() & 0xF0) >> 4;
        date[i] = c*10 + c4;
        skipOctet();
    }

    if ( date[0] < 80 ) {
        d.setDate( QDate(2000 + date[0], date[1], date[2]) );
    } else {
        d.setDate( QDate(1900 + date[0], date[1], date[2]) );
    }
    d.setTime( QTime(date[3], date[4], date[5]) );

    return d;
}

// Get the length of a string when encoded in the GSM 7-bit alphabet.
static uint getEncodedLength( const QString& txt, uint size )
{
    uint len = 0;
    for ( int u = 0; u < (int)size; u++ ) {
        if ( QGsmCodec::twoByteFromUnicode( txt[u].unicode() ) >= 256 )
            len += 2;
        else
            ++len;
    }
    return len;
}

void QPDUMessage::setUserData(const QString &txt, QSMSDataCodingScheme scheme, QTextCodec *codec, const QByteArray& headers, bool implicitLength)
{
    uint len = txt.length();
    uint u;
    uint encodedLen;
    uint headerLen = (uint)(headers.size());
    if ( headerLen )
        ++headerLen;

    // Strip off everything except the alphabet bits.
    scheme = (QSMSDataCodingScheme)(scheme & 0x0C);

    if ( scheme == QSMS_DefaultAlphabet ) {

        // Encode the text using the 7-bit GSM alphabet.
        if ( len > 160 )
            len = 160;
        encodedLen = getEncodedLength( txt, len );
        while ( encodedLen > 160 ) {
            // Chop off some more characters until it is <= 160.
            --len;
            encodedLen = getEncodedLength( txt, len );
        }
        if (!implicitLength)
            appendOctet( encodedLen + ( headerLen * 8 + 6 ) / 7 );
        int bitCount = 0;
        unsigned short c;
        if ( headerLen > 0 ) {
            // Output the header and align on a septet boundary.
            bitCount = headerLen * 8;
            if ((bitCount % 7) != 0)
                bitCount = 7 - (bitCount % 7);
            else
                bitCount = 0;
            appendOctet( headerLen - 1 );
            for ( u = 0; u < headerLen - 1; u++ ) {
                appendOctet( headers[u] );
            }
        }
        for ( u = 0; u < len; u++ ) {
            c = QGsmCodec::twoByteFromUnicode( txt[u].unicode() );
            if ( c >= 256 ) {
                // Encode a two-byte sequence.
                for ( int i = 0; i < 7; i++ ) {
                    if ( bitCount == 8 ) {
                        bitCount = 0;
                        commitBits();
                    }
                    setBit( bitCount++, (Unit << (i+8)) & c );
                }
            }
            for ( int i = 0; i < 7; i++ ) {
                if ( bitCount == 8 ) {
                    bitCount = 0;
                    commitBits();
                }
                setBit( bitCount++, (Unit << i) & c );
            }
        }
        if ( bitCount != 0 ) {
            commitBits();
        }

    } else if ( scheme == QSMS_8BitAlphabet ) {
        // Encode the text using the codec's 8-bit alphabet.
        if ( !codec )
            codec = QAtUtils::codec( "iso-8859-1" );
        QByteArray converted = codec->fromUnicode( txt );
        len = converted.length();
        if ( len > 140 )
            len = 140;
        if (!implicitLength)
            appendOctet( len + headerLen );
        if ( headerLen > 0 ) {
            appendOctet( headerLen - 1 );
            for ( u = 0; u < headerLen - 1; u++ ) {
                appendOctet( headers[u] );
            }
        }
        const char *s = (const char *)converted;
        for ( u = 0; u < len; u++ ) {
            appendOctet( (unsigned char)(s[u]) );
        }

    } else {

        // Encode the text using the 16-bit UCS2 alphabet.
        if ( len > 70 )
            len = 70;
        if (!implicitLength)
            appendOctet( len * 2 + headerLen );
        if ( headerLen > 0 ) {
            appendOctet( headerLen - 1 );
            for ( u = 0; u < headerLen - 1; u++ ) {
                appendOctet( headers[u] );
            }
        }
        for ( u = 0; u < len; u++ ) {
            appendOctet( (unsigned char)(txt[u].unicode() >> 8) );
            appendOctet( (unsigned char)(txt[u].unicode() & 0xFF) );
        }

    }
}

// Determine if the headers in an SMS frame indicate that it
// is an application datagram destinated for a particular port.
static bool isSMSDatagram( const QByteArray& headers )
{
    uint posn = 0;
    uint tag, len;
    while ( ( posn + 2 ) <= (uint)(headers.size()) ) {
        tag = (unsigned char)(headers[posn]);
        len = (unsigned char)(headers[posn + 1]);
        if ( ( posn + len + 2 ) > (uint)(headers.size()) )
            break;
        if ( tag == (uint)SMS_HK_AppPort_8Bit ||
             tag == (uint)SMS_HK_AppPort_16Bit ) {
            return true;
        }
        posn += len + 2;
    }
    return false;
}

QString QPDUMessage::userData(QSMSDataCodingScheme scheme, QTextCodec *codec, QByteArray *& headers, bool hasHeaders, bool implicitLength)
{
    QString str = "";
    uint len, headerLen;
    uint u;
    uint ch;

    // Reset the header return.
    headers = 0;

    // Get the length of the user data payload.
    if ( implicitLength ) {
        len = mBuffer.size() - mPosn;
    } else {
        if ( !needOctets(1) )
            return str;
        len = (uint)getOctet();
    }

    // Strip off everything except the alphabet bits.
    scheme = (QSMSDataCodingScheme)(scheme & 0x0C);

    if ( scheme == QSMS_DefaultAlphabet ) {

        // Process a sequence in the default 7-bit GSM character set.
        int bitCount = 0;
        int startBit = 0;
        ch = 0;
        if ( implicitLength )
            len = len * 8 / 7;      // Convert 8-bit bytes to 7-bit characters.
        if ( hasHeaders ) {
            u = ( len * 7 + 7 ) / 8;
            if ( !u || !needOctets(u) )
                return str;
            headerLen = getOctet();
            if ( headerLen >= u )
                return str;
            headers = new QByteArray( getOctets( headerLen ) );
            if ( isSMSDatagram( *headers ) )
                return str;
            u = ((headerLen + 1) * 8 + 6) / 7;
            len -= u;
            startBit = (headerLen + 1) * 8;
            if ((startBit % 7) != 0)
                startBit = 7 - (startBit % 7);
            else
                startBit = 0;
        }
        bool prefixed = false;
        while ( len > 0 ) {
            if ( !needOctets(1) )
                return str;
            for ( int i = startBit; len > 0 && i < 8; i++ ) {
                // test the bit and shift it down again, as i doesn't mark
                // where we are in the current char, but bitCount does
                ch |= ( ( peekOctet() & (Unit << i) ) >> i) << bitCount;
                bitCount++;
                if ( bitCount == 7 ) {
                    bitCount = 0;
                    if ( ch == 0x1B ) {     // Start of a two-byte encoding.
                        prefixed = true;
                    } else if ( prefixed ) {
                        str += QChar
                            ( QGsmCodec::twoByteToUnicode( 0x1B00 | ch ) );
                        prefixed = false;
                    } else {
                        str += QGsmCodec::singleToUnicode( (unsigned char)ch );
                    }
                    ch = 0;
                    --len;
                }
            }
            startBit = 0;
            skipOctet();
        }

    } else if ( scheme == QSMS_8BitAlphabet && codec ) {

        // Process an 8-bit sequence using the supplied codec.
        if ( !needOctets(len) ) {
            abort();
            return str;
        }
        if ( hasHeaders ) {
            if ( !len || len < (uint)( (peekOctet() & 0xFF) + 1 ) ) {
                abort();
                return str;
            }
            headerLen = getOctet();
            headers = new QByteArray( getOctets( headerLen ) );
            if ( isSMSDatagram( *headers ) )
                return str;
            len -= headerLen + 1;
        }
        str = codec->toUnicode( getOctets( len ) );

    } else if ( scheme == QSMS_UCS2Alphabet ) {

        // Process a UCS2 sequence.
        if ( !needOctets(len) ) {
            abort();
            return str;
        }
        if ( hasHeaders ) {
            if ( !len || len < (uint)( (peekOctet() & 0xFF) + 1 ) ) {
                abort();
                return str;
            }
            headerLen = getOctet();
            headers = new QByteArray( getOctets( headerLen ) );
            len -= headerLen + 1;
        }
        len /= 2;
        for ( u = 0; u < len; ++u ) {
            ch = (((uint)(getOctet() & 0xFF)) << 8);
            ch |= ((uint)(getOctet() & 0xFF));
            str += (QChar)ch;
        }

    } else {

        // Assume 8-bit for any other coding scheme value.

        // Process an 8-bit sequence using the default Latin1 codec.
        if ( len > (uint)(mBuffer.size() - mPosn) ) {
            // The length field is invalid - use the actual size.
            len = mBuffer.size() - mPosn;
        }
        if ( hasHeaders ) {
            if ( !len || len < (uint)( (peekOctet() & 0xFF) + 1 ) ) {
                abort();
                return str;
            }
            headerLen = getOctet();
            headers = new QByteArray( getOctets( headerLen ) );
            if ( isSMSDatagram( *headers ) )
                return str;
            len -= headerLen + 1;
        }
        for ( u = 0; u < len; ++u ) {
            str += (QChar)(getOctet() & 0xFF);
        }

    }

    return str;
}

/*  Note that the meaning of messageType is also dependant on
    the direction of the message (orig. location) */
SMSMessageType QPDUMessage::messageType()
{
    if ( mBuffer.size() >= 1 &&
         mBuffer.size() >= ((mBuffer[0] & 0xFF) + 2) ) {

        const char *ptr = mBuffer.constData();
        ptr += (*ptr & 0xFF) + 1;
        unsigned char c = *ptr & 3;
        return (SMSMessageType) c;

    }
    return (SMSMessageType)0;
}

QSMSSubmitMessage::QSMSSubmitMessage(const QSMSMessage &m, bool isDeliver)
    : QPDUMessage()
{
    // Clear the pdu before we start.
    mBuffer = QByteArray();
    mPosn = 0;
    mBits = 0;

    setAddress( m.serviceCenter(), true );

    // If there is port and application data information, then
    // create header parts for them.  This is for sending datagram
    // based messages.
    QByteArray headers = m.headers();
    QSMSDataCodingScheme scheme = m.bestScheme();
    int part = m.findPart( "application/x-qtopia-wdp-ports" );
    if ( part != -1 ) {
        QByteArray portData = m.parts()[(uint)part].data();
        uint size = headers.size();
        headers.resize(size + portData.size() + 2);
        if ( portData.size() == 4 ) {
            headers[size++] = (char)SMS_HK_AppPort_16Bit;
        } else {
            headers[size++] = (char)SMS_HK_AppPort_8Bit;
        }
        headers[size++] = (char)(portData.size());
        memcpy(headers.data() + size, portData.data(), portData.size());
        int dataScheme = m.dataCodingScheme(); // User-supplied override.
        if ( dataScheme == -1 )
            dataScheme = 0xF5;      // Special value for datagrams.
        scheme = (QSMSDataCodingScheme)dataScheme;
    } else if ( m.messageClass() != -1 ) {
        scheme = (QSMSDataCodingScheme)
                    (scheme | QSMS_MessageClass | m.messageClass());
        int dataScheme = m.dataCodingScheme(); // User-supplied override.
        if ( dataScheme != -1 )
            scheme = (QSMSDataCodingScheme)dataScheme;
    } else {
        int dataScheme = m.dataCodingScheme(); // User-supplied override.
        if ( dataScheme != -1 )
            scheme = (QSMSDataCodingScheme)dataScheme;
    }

    if ( !isDeliver )
        setBits(0, 2, SMS_Submit);
    else
        setBits(0, 2, SMS_Deliver);

    setBit(2, false);                   // TP-Reject-Duplicates
    if ( !isDeliver && m.validityPeriod() == (uint)(-1) )
        setBits(3, 2, SMS_VF_NoPresent);
    else
        setBits(3, 2, SMS_VF_Relative);
    setBit(5, m.statusReportRequested());// TP-Status-Report-Requested;
    setBit(6, headers.size() != 0);     // TP-User-Data Header
    setBit(7, m.replyRequest());        // TP-Reply-Path

    commitBits();       // first octet done

    //second octet TP-MR (Message reference
    if( !isDeliver )
        appendOctet(0);

    if ( !isDeliver ) {
        //third octet TP-DA (Destination Address)
        //len must be done later
        setAddress( m.recipient(), false );
    } else {
        setAddress( m.sender(), false);
    }


    //nth octet, TP-PID (protocol identifier)
    if ( !isDeliver )
        appendOctet(m.protocol());
    else
        appendOctet(1); //arbitrary protocol for deliver

    // TP-DCS ( Data coding scheme )
    appendOctet(scheme);

    if ( !isDeliver ) {
        // TP-VP ( Validity Period )
        if ( m.validityPeriod() != (uint)(-1) )
            appendOctet(m.gsmValidityPeriod());
    }  else {
        setTimeStamp(m.timestamp());
    }

    // Set the user data field.
    if ( part == -1 ) {
        setUserData(m.text(), scheme, m.textCodec(), headers);
    } else {
        // The applicationData() is the text to be sent in the datagram.
        QByteArray appData = m.applicationData();
        uint len = appData.size();
        if ( ( len + headers.size() + 1 ) > 140 )
            len = 140 - headers.size() - 1;
        appendOctet( len + headers.size() + 1 );
        appendOctet( headers.size() );
        uint u;
        for ( u = 0; u < (uint)headers.size(); u++ ) {
            appendOctet( headers[u] );
        }
        for ( u = 0; u < len; u++ ) {
            appendOctet( appData[u] );
        }
    }
}

QSMSDeliverMessage::QSMSDeliverMessage(const QByteArray &pdu)
    : QPDUMessage(pdu)
{
}

// Unpack a message that has a "//SCKL" header in its text body.
// This is a standard to interoperate with networks and phones
// that do not support user data headers properly.
static void unpackSckl( QSMSMessage& m, const QString& text )
{
    // Split into header and body, separated by a space.
    int index = text.indexOf( QChar(' ') );
    if ( index == -1 ) {
        m.addPart( QSMSMessagePart( text ) );
        return;
    }
    QString head = text.mid( 6, index - 6 );
    QString body = text.mid( index + 1 );

    // Convert the header from hex into raw binary and then
    // copy its details into the real message header fields.
    QByteArray header = QAtUtils::fromHex( head );
    int len;
    if ( header.size() < 4 ) {
        len = header.size();
    } else {
        len = 4;
    }
    QByteArray ports( len, '\0' );
    memcpy( ports.data(), header.data(), len );
    m.addPart( QSMSMessagePart( "application/x-qtopia-wdp-ports", ports ) );
    if ( header.size() > 4 ) {
        QByteArray fragments( header.size() - 4 + 2, '\0' );
        fragments[0] = (char)( SMS_HK_Concat_8Bit );
        fragments[1] = (char)( header.size() - 4 );
        memcpy( fragments.data() + 2, header.data() + 4, header.size() - 4 );
        m.setHeaders( fragments );
    }

    // Is the body text or binary?
    int port = m.destinationPort();
    if ( port == 226 || port == 9204 ||     // vCard port numbers.
         port == 228 || port == 9205 ) {    // vCalendar port numbers.
        m.setApplicationData( body.toLatin1() );
    } else {
        m.setApplicationData( QAtUtils::fromHex( body ) );
    }
}

QSMSMessage QSMSDeliverMessage::unpack(QTextCodec *codec)
{
    QSMSMessage m;
    bool moreMessages;
    bool statusReport;
    bool userDataHeader;
    bool replyPath;
    bool rejectDuplicates;
    unsigned char protocol;
    unsigned char scheme;
    uint msgType;
    uint validityFormat;

    // Start from the beginning of the PDU.
    reset();

    // Extract the service center address.
    m.setServiceCenter( address(true) );

    // Pull apart the message header.  We handle both deliver and
    // submit messages, because we may have pulled a submit out
    // of the phone's outgoing SMS queue.
    if ( !needOctets(1) )
        return m;
    msgType = bits(0, 2);
    if ( msgType == SMS_Deliver ) {
        moreMessages = bit(2);
        // Bits 3 and 4 are unused for deliver messages.
        statusReport = bit(5);
        userDataHeader = bit(6);
        replyPath = bit(7);
        rejectDuplicates = false;
        validityFormat = SMS_VF_NoPresent;
    } else if ( msgType == SMS_Submit ) {
        moreMessages = false;
        rejectDuplicates = bit(2);
        validityFormat = bits(3, 2);
        statusReport = bit(5);
        userDataHeader = bit(6);
        replyPath = bit(7);
    } else {
        // Probably a delivery report, which we don't process.
        return m;
    }
    skipOctet();

    // Remember the status and reply bits.
    m.setStatusReportRequested(statusReport);
    m.setReplyRequest(replyPath);

    // Skip the message reference (submit messages only).
    if ( msgType == SMS_Submit ) {
        if ( !needOctets(1) )
            return m;
        skipOctet();
    }

    // Get the address of the sender (delivery) or recipient (submit).
    if ( msgType == SMS_Deliver ) {
        m.setSender( address(false) );
    } else {
        m.setRecipient( address(false) );
    }

    // Get the protocol identifier and data coding scheme.
    if ( !needOctets(2) )
        return m;
    protocol = getOctet();
    scheme = getOctet();
    m.setProtocol( protocol );
    m.setDataCodingScheme( scheme );
    if ( ( scheme & QSMS_MessageClass ) != 0 )
        m.setMessageClass( scheme & 0x03 );
    else
        m.setMessageClass( -1 );

    // Get the timestamp (deliver) or validity period (submit) information.
    if ( msgType == SMS_Deliver ) {
        m.setTimestamp( timeStamp() );
    } else {
        if ( validityFormat == SMS_VF_Relative ) {
            if ( !needOctets(1) )
                return m;
            m.setGsmValidityPeriod( bits(0, 8) );
            skipOctet();
        } else if (validityFormat == SMS_VF_Absolute ) {
            m.setTimestamp( timeStamp() );
        } else if (validityFormat == SMS_VF_Enhanced ) {
            // Not supported yet - skip the octets.
            if ( !needOctets(7) )
                return m;
            mPosn += 7;
        } else {
            m.setValidityPeriod( (uint)(-1) );
        }
    }

    // Read the user data field.
    QByteArray *headers = 0;
    QString text;
    text = userData( (QSMSDataCodingScheme)scheme, codec,
                     headers, userDataHeader, false );
    if ( !headers && text.startsWith( "//SCKL" ) ) {
        unpackSckl( m, text );
        return m;
    }
    if ( headers ) {
        m.setHeaders( *headers );
        delete headers;
        m.unpackHeaderParts();
        if ( isSMSDatagram( m.d->mHeaders ) && mPosn <= mBuffer.size() ) {
            // The rest of the PDU is assumed to be the application payload.
            QByteArray array = mBuffer.mid( mPosn );
            m.addPart( QSMSMessagePart( "application/x-qtopia-wdp", array ) );
        }
    }
    if ( text.length() > 0 ) {
        m.addPart( QSMSMessagePart( text ) );
    }

    // Return the completed message to the caller.
    return m;
}


QCBSDeliverMessage::QCBSDeliverMessage()
    : QPDUMessage()
{
    // Nothing to do here.
}


QCBSDeliverMessage::QCBSDeliverMessage(const QByteArray &pdu)
    : QPDUMessage(pdu)
{
    // Nothing to do here.
}


QCBSMessage QCBSDeliverMessage::unpack(QTextCodec *codec)
{
    QCBSMessage m;
    unsigned char scheme;
    uint len;

    // Start from the beginning of the PDU.
    reset();

    // Extract the header fields.
    if ( !needOctets(6) )
        return m;

    const char *mOffset = mBuffer.constData() + mPosn;
    m.setMessageCode( ((mOffset[0] & 0xFC) << 2) | (mOffset[1] & 0x0F) );
    m.setScope( (QCBSMessage::GeographicalScope)(mOffset[0] & 0x03) );
    m.setUpdateNumber( (mOffset[1] >> 4) & 0x0F );
    m.setChannel( ((mOffset[2] & 0xFF) << 8) | (mOffset[3] & 0xFF) );
    scheme = (unsigned char)((mOffset[4] >> 4) & 0x0F);
    m.setLanguage( (QCBSMessage::Language)(mOffset[4] & 0x0F) );
    m.setNumPages( (uint)((mOffset[5] >> 4) & 0x0F) );
    m.setPage( (uint)(mOffset[5] & 0x0F) );
    mPosn += 6;

    // Read the user data field and strip CR's, LF's, and NUL's from the end.
    QByteArray *headers = 0;
    QString text = userData
        ( (QSMSDataCodingScheme)scheme, codec, headers, false, true );
    len = text.length();
    while ( len > 0 && ( text[len - 1] == '\r' || text[len - 1] == '\n' ||
                         text[len - 1] == '\0' ) ) {
        --len;
    }
    m.setText( text.left( len ) );

    // Return the completed message to the caller.
    return m;
}

void QCBSDeliverMessage::pack(const QCBSMessage &m, QSMSDataCodingScheme scheme)
{
    // Clear the pdu before we start.
    mBuffer = QByteArray();
    mPosn = 0;
    mBits = 0;

    scheme = QSMS_8BitAlphabet; // Only 8-bit works at present.
    QByteArray data;
    mBuffer.append( (char)( ((m.messageCode() & 0x000003F0)>>2) | (m.scope() & 0x03)) );
    mBuffer.append( (char)((m.messageCode() & 0x0000000F) | (m.updateNumber() & 0x0000000F)<<4) );
    mBuffer.append( (char)((m.channel() & 0x0000FF00) >> 8) );
    mBuffer.append( (char)(m.channel() & 0x000000FF) );
    mBuffer.append( (char)( ((scheme & 0x0F)<<4) | (m.language() & 0x0F)) );
    mBuffer.append( (char)( ((m.numPages() & 0x0F)<<4) | (m.page() & 0x0F)) );

    QTextCodec *codec = QAtUtils::codec( "gsm" );
    QByteArray header;
    setUserData(m.text(), scheme, codec, header,true);
    int numPad = 88 - (m.text().length()  + data.size());

    for ( int i=0; i<numPad; i++ )
        appendOctet(0x0D);
}

Q_IMPLEMENT_USER_METATYPE(QSMSMessage)
Q_IMPLEMENT_USER_METATYPE(QSMSMessagePart)
