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


#ifndef QTOPIA_NO_SMS
#include "smsdecoder.h"

#include <qbuffer.h>
#include <qsmsmessage.h>
#include <qmailmessage.h>
#include <qwbxmlreader.h>
#include <qotareader.h>
#include <qwsppdu.h>


// Nokia-style application port numbers of interest.  Most based on the
// "Nokia Smart Messaging Specification 3.0.0" and the "Nokia Over
// the Air Settings Specification 7.0".
#define SMS_PORT_MIME_VCARD         226
#define SMS_PORT_MIME_VCALENDAR     228
#define SMS_PORT_WAP_PUSH           2948
#define SMS_PORT_RING_TONE          5505
#define SMS_PORT_OPERATOR_LOGO      5506
#define SMS_PORT_CLI_LOGO           5507
#define SMS_PORT_MULTIPART_MESSAGE  5514
#define SMS_PORT_WAP_VCARD          9204
#define SMS_PORT_WAP_VCALENDAR      9205
#define SMS_PORT_INTERNET_CONFIG    49999

SMSDecoder::SMSDecoder()
{
}

SMSDecoder::~SMSDecoder()
{
}

QString SMSDecoder::mimeTypeForPort( int port )
{
    switch ( port ) {

        case SMS_PORT_MIME_VCARD:
        case SMS_PORT_WAP_VCARD:
            return "text/x-vcard";

        case SMS_PORT_MIME_VCALENDAR:
        case SMS_PORT_WAP_VCALENDAR:
            return "text/x-vcalendar";

        case SMS_PORT_RING_TONE:
            return "audio/x-ota-ringtone";                  // FIXME
    }
    return QString();
}

SMSDecoder *SMSDecoder::decoder( int port )
{
    switch ( port ) {

        case SMS_PORT_MULTIPART_MESSAGE:
            return new SMSMultipartDecoder();

        case SMS_PORT_OPERATOR_LOGO:
            return new SMSLogoDecoder( true );

        case SMS_PORT_CLI_LOGO:
            return new SMSLogoDecoder( false );

        case SMS_PORT_INTERNET_CONFIG:
            return new SMSWbXmlDecoder
                ( new QOtaReader( QOtaReader::Nokia ),
                  "text/x-wap-prov.browser-settings", true );

        case SMS_PORT_WAP_PUSH:
            return new SMSWapPushDecoder();
    }
    return 0;
}

SMSMultipartDecoder::SMSMultipartDecoder()
{
}

SMSMultipartDecoder::~SMSMultipartDecoder()
{
}

// Type codes for multipart SMS messages.
#define SMS_MULTI_TEXT_LATIN1       0x00
#define SMS_MULTI_TEXT_UNICODE      0x01
#define SMS_MULTI_OTA_BITMAP        0x02
#define SMS_MULTI_RING_TONE         0x03
#define SMS_MULTI_PROFILE_NAME      0x04
#define SMS_MULTI_RESERVED          0x05
#define SMS_MULTI_SCREEN_SAVER      0x06

void SMSMultipartDecoder::decode( QMailMessage& mail, const QSMSMessage& msg )
{
    QByteArray data = msg.applicationData();
    int posn, type, len, index;
    QByteArray part;
    QString text;

    // Check the version number on the front of the message.
    if ( data.size() < 1 || data[0] != '0' )
        return;

    // Process each of the parts in turn.
    posn = 1;
    while ( ( posn + 3 ) <= data.size() ) {
        type = data[posn] & 0xFF;
        len = ((data[posn + 1] & 0xFF) << 8) | (data[posn + 2] & 0xFF);
        if ( ( posn + 3 + len ) > data.size() ) {
            qWarning("invalid sms multipart message length");
            break;
        }
        part.resize(len);
        memcpy( part.data(), data.data() + posn + 3, len );
        text = QString();

        switch ( type ) {

            case SMS_MULTI_TEXT_LATIN1:
            {
                text = QString::fromLatin1( part.data(), len );
            }
            break;

            case SMS_MULTI_TEXT_UNICODE:
            case SMS_MULTI_PROFILE_NAME:
            {
                for ( index = 0; (index + 1) < len; index += 2 ) {
                    text += QChar(((part.data()[index] & 0xFF) << 8) |
                                   (part.data()[index + 1] & 0xFF));
                }
            }
            break;

            case SMS_MULTI_RING_TONE:
            {
                QMailMessagePart mpart;
                QMailMessageContentType type( "audio/x-ota-ringtone" );
                mpart.setBody( QMailMessageBody::fromData( part, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );
                mail.appendPart( mpart );
            }
            break;

            case SMS_MULTI_OTA_BITMAP:
            case SMS_MULTI_SCREEN_SAVER:
            {
                QMailMessagePart mpart;
                QMailMessageContentType type( "image/x-ota-bitmap" );
                mpart.setBody( QMailMessageBody::fromData( part, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );
                mail.appendPart( mpart );
            }
            break;

            default:
                qWarning("unknown sms multipart message type %x", type );
                break;
        }
        if ( !text.isNull() ) {
            QMailMessagePart mpart;
            QMailMessageContentType type( "text/plain; charset=UTF-8" );
            mpart.setBody( QMailMessageBody::fromData( text, type, QMailMessageBody::QuotedPrintable ) );
            mail.appendPart( mpart );
        }
        posn += len + 3;
    }
}

SMSLogoDecoder::SMSLogoDecoder( bool operatorHeader )
{
    this->operatorHeader = operatorHeader;
}

SMSLogoDecoder::~SMSLogoDecoder()
{
}

void SMSLogoDecoder::decode( QMailMessage& mail, const QSMSMessage& msg )
{
    QByteArray data = msg.applicationData();
    uint start;
    int mcc, mnc;

    // Process the header (operator-style or CLI-style).
    if ( operatorHeader ) {

        // Check the header and version number.
      if ( data.size() <= 4 || data[0] != static_cast<char>(0x30) )
            return;

        // Extract the MCC and MNC, which are stored in BCD,
        // terminated with 0x0F.
    mcc = ( ( data[1] & 0x0F ) << 12 ) +
          ( ( data[1] & 0xF0 ) << 4 ) +
          ( ( data[2] & 0x0F ) << 4 ) +
          ( ( data[2] & 0xF0 ) >> 4 );
        while ( ( mcc & 0x0F ) == 0x0F )
            mcc >>= 4;
        mcc = ( ( mcc >> 12 ) & 0x0F ) * 1000 +
              ( ( mcc >> 8 ) & 0x0F ) * 100 +
              ( ( mcc >> 4 ) & 0x0F ) * 10 +
              ( mcc & 0x0F );
        mnc = ( ( data[3] & 0x0F ) << 4 ) +
              ( ( data[3] & 0xF0 ) >> 4 );
        while ( ( mnc & 0x0F ) == 0x0F )
            mnc >>= 4;
        mnc = ( ( mnc >> 4 ) & 0x0F ) * 10 +
              ( mnc & 0x0F );

        // Skip a line feed at the end of the header if one is present
        // (it may be omitted if the bitmap is large).
        start = 4;
    if ( data.size() > static_cast<int>(start) && data[start] == static_cast<char>(0x0A) ) {
            ++start;
        }

    } else {

        // Check the CLI header size and contents.
      if ( data.size() <= 1 || data[0] != static_cast<char>(0x30) )
            return;
        start = 1;
        mcc = 0;
        mnc = 0;

    }

    // Add the rest of the message as an OTA bitmap.
    QByteArray bitmap;
    bitmap.resize( data.size() - start );
    memcpy( bitmap.data(), data.data() + start, data.size() - start );

    QByteArray content;
    if ( !mcc && !mnc ) {
        content = "image/x-ota-bitmap";
    } else {
        content = QByteArray( "image/x-ota-bitmap; mcc=" ) +
                  QByteArray::number( mcc ) + "; mnc=" +
                  QByteArray::number( mnc );
    }

    QMailMessagePart mpart;
    QMailMessageContentType type( content );
    mpart.setBody( QMailMessageBody::fromData( bitmap, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );
    mail.appendPart( mpart );
}

SMSWbXmlDecoder::SMSWbXmlDecoder( QWbXmlReader *reader, const QString& mimeType, bool pushHeader )
{
    this->reader = reader;
    this->mimeType = mimeType;
    this->pushHeader = pushHeader;
}

SMSWbXmlDecoder::~SMSWbXmlDecoder()
{
    delete reader;
}

void SMSWbXmlDecoder::decode( QMailMessage& mail, const QSMSMessage& msg )
{
    QString xml;
    if ( !pushHeader ) {
        xml = reader->toXml( msg.applicationData());
    } else {
    QByteArray data = msg.applicationData();
        QBuffer buffer( &data,0);
        buffer.open( QIODevice::ReadOnly );
        QWspPduDecoder decoder( &buffer );
        QWspPush push = decoder.decodePush();
        xml = reader->toXml( push.data() );
    }

    QMailMessagePart mpart;
    QMailMessageContentType type( mimeType.toLatin1() );
    mpart.setBody( QMailMessageBody::fromData( xml, type, QMailMessageBody::EightBit ) );
    mail.appendPart( mpart );
}

SMSWapPushDecoder::SMSWapPushDecoder()
{
}

SMSWapPushDecoder::~SMSWapPushDecoder()
{
}

void SMSWapPushDecoder::decode( QMailMessage& mail, const QSMSMessage& msg )
{
    QMailMessagePart mpart;

    // Decode the push header.
    QByteArray data = msg.applicationData();
    QBuffer buffer(&data,0 );
    buffer.open( QIODevice::ReadOnly );
    QWspPduDecoder decoder( &buffer );
    QWspPush push = decoder.decodePush();

    // Recognise certain kinds of push messages.
    const QWspField *field = push.header( "Content-Type" );
    QString mimeType = ( field ? field->value : QString() );
    if ( mimeType.startsWith( "application/vnd.wap.connectivity-wbxml" ) ) {
        QString xml;
        QWbXmlReader *reader = new QOtaReader( QOtaReader::Wap );
        xml = reader->toXml( push.data() );
        delete reader;

        QMailMessageContentType type( "text/vnd.wap.connectivity-xml; charset=UTF-8" );
        mpart.setBody( QMailMessageBody::fromData( xml, type, QMailMessageBody::EightBit ) );
        mail.appendPart( mpart );
        return;
    }

    // Add the contents of the push message as a body part.
    QMailMessageContentType type( mimeType.toLatin1() );
    mpart.setBody( QMailMessageBody::fromData( push.data(), type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );
    mail.appendPart( mpart );
}

// "Enhanced Messaging Service" formatter, according to GSM 23.040.

class EmsPart;

class EmsFormatter
{
public:
    EmsFormatter() { _alignment = 0; _parts = 0; }
    ~EmsFormatter();

    QString closeTags() const { return _closeTags; }
    void setCloseTags( const QString& value ) { _closeTags = value; }

    QString closeAlignmentTags() const { return _closeAlignmentTags; }
    void setCloseAlignmentTags( const QString& value )
        { _closeAlignmentTags = value; }

    uint alignment() const { return _alignment; }
    void setAlignment( uint value ) { _alignment = value; }

    QString text() const { return _text; }
    void addText( const QString& value ) { _text += value; }

    void addPart( EmsPart *part );

    void flushParts( const QString& text, bool final=false );

private:
    QString _closeTags;
    QString _closeAlignmentTags;
    uint _alignment;
    QString _text;
    EmsPart *_parts;

    void addQuotedText( const QString& value, uint start, uint end );
};

class EmsPart
{
    friend class EmsFormatter;
public:
    EmsPart() { _start = 0; _length = 0; _next = 0; }
    virtual ~EmsPart();

    uint start() const { return _start; }
    void setStart( uint value ) { _start = value; }

    uint length() const { return _length; }
    void setLength( uint value ) { _length = value; }

    virtual void startFormat( EmsFormatter& formatter );
    virtual void endFormat( EmsFormatter& formatter );

private:
    uint _start;
    uint _length;
    EmsPart *_next;
};

class EmsTextFormattingPart : public EmsPart
{
public:
    EmsTextFormattingPart() { _format = 0; _colors = 0; _hasColors = false; }

    uint format() const { return _format; }
    void setFormat( uint value ) { _format = value; }

    uint colors() const { return _colors; }
    void setColors( uint value ) { _colors = value; _hasColors = true; }

    virtual void startFormat( EmsFormatter& formatter );
    virtual void endFormat( EmsFormatter& formatter );

private:
    uint _format;
    uint _colors;
    bool _hasColors;
};

class EmsImagePart : public EmsPart
{
public:
    QString source() const { return _source; }
    void setSource( const QString& value ) { _source = value; }

    virtual void startFormat( EmsFormatter& formatter );

private:
    QString _source;
};

class EmsAttachmentPart : public EmsPart
{
public:
    QString source() const { return _source; }
    void setSource( const QString& value ) { _source = value; }

    QString name() const { return _name; }
    void setName( const QString& value ) { _name = value; }

    virtual void startFormat( EmsFormatter& formatter );

private:
    QString _source;
    QString _name;
};

EmsFormatter::~EmsFormatter()
{
    delete _parts;
}

void EmsFormatter::addQuotedText( const QString& value, uint start, uint end )
{
    uint posn = start;
    while ( posn < end ) {
      switch ( (value[posn]).toAscii() ) {

            case '&':
            {
                addText( value.mid( start, posn - start ) );
                addText( "&amp;" );
                start = posn + 1;
            }
            break;

            case '<':
            {
                addText( value.mid( start, posn - start ) );
                addText( "&lt;" );
                start = posn + 1;
            }
            break;

            case '>':
            {
                addText( value.mid( start, posn - start ) );
                addText( "&gt;" );
                start = posn + 1;
            }
            break;

            case '\r':
            {
                addText( value.mid( start, posn - start ) );
                addText("<br>");
                if ( ( posn + 1 ) < end && value[posn + 1] == '\n' )
                    ++posn;
                start = posn + 1;
            }
            break;

            case '\n':
            {
                addText( value.mid( start, posn - start ) );
                addText("<br>");
                start = posn + 1;
            }
            break;

            default:    break;
        }
        ++posn;
    }
    if ( start < end ) {
        addText( value.mid( start, end - start ) );
    }
}

void EmsFormatter::addPart( EmsPart *part )
{
    EmsPart *current = _parts;
    EmsPart *prev = 0;
    while ( current != 0 ) {
        if ( current->_start > part->_start ) {
            break;
        }
        if ( current->_start == part->_start ) {
            // Insert zero-length image and sound parts after
            // text formatting parts that refer to the range.
            // This makes the formatting apply to the image or
            // sound part as well.
            if ( current->_length <= part->_length )
                break;
        }
        prev = current;
        current = current->_next;
    }
    if ( prev ) {
        part->_next = prev->_next;
        prev->_next = part;
    } else {
        part->_next = _parts;
        _parts = part;
    }
}

void EmsFormatter::flushParts( const QString& text, bool final )
{
    uint posn;
    uint last = 0;
    EmsPart *part;

    // Output parts that are interspersed with the text.
    for ( posn = 0; posn < static_cast<uint>(text.length()); ++posn ) {
        part = _parts;
        while ( part != 0 ) {
            if ( part->start() == posn ) {
                if ( last < posn ) {
                    addQuotedText( text, last, posn );
                    last = posn;
                }
                part->startFormat( *this );
            }
            if ( ( part->start() + part->length() ) == posn ) {
                if ( last < posn ) {
                    addQuotedText( text, last, posn );
                    last = posn;
                }
                part->endFormat( *this );
            }
            part = part->_next;
        }
    }

    // Output the remaining text.
    if ( last < static_cast<uint>(text.length()) ) {
        addQuotedText( text, last, text.length() );
        last = text.length();
    }

    // Output any parts that appear after the end of the text.
    part = _parts;
    while ( part != 0 ) {
        if ( part->start() >= last ) {
            part->startFormat( *this );
            part->endFormat( *this );
        }
        part = part->_next;
    }
    delete _parts;
    _parts = 0;

    // Flush the remaining close tags.
    if ( final ) {
        _text += _closeTags + _closeAlignmentTags;
        _closeTags = QString();
        _closeAlignmentTags = QString();
        _alignment = 0;
    } else {
        _text += _closeTags;
        _closeTags = QString();
    }
}

EmsPart::~EmsPart()
{
    if ( _next )
        delete _next;
}

void EmsPart::startFormat( EmsFormatter& )
{
    // Nothing to do here.
}

void EmsPart::endFormat( EmsFormatter& )
{
    // Nothing to do here.
}

void EmsTextFormattingPart::startFormat( EmsFormatter& formatter )
{
    QString pending;
    QString pendingAlignment;
    QString openTags;
    QString openAlignmentTags;
    QString closeTags;
    QString closeAlignmentTags;
    static const char *colorMap[16] = {
        "000000", "808080", "800000", "808000", "008000", "008080",
        "000080", "800080", "C0C0C0", "FFFFFF", "FF0000", "FFFF00",
        "00FF00", "00FFFF", "0000FF", "FF00FF"
    };

    // Get the pending close tags to reset the previous text format.
    pending = formatter.closeTags();
    pendingAlignment = formatter.closeAlignmentTags();

    // Build the new open and close tags for this range.
    if ( ( _format & 0x03 ) == 0x01 ) {
        openAlignmentTags = "<center>";
        closeAlignmentTags = "</center>";
    } else if ( ( _format & 0x03 ) == 0x02 ) {
        openAlignmentTags = "<p align=\"right\">";
        closeAlignmentTags = "</p>";
    }
    if ( _hasColors ) {
        openTags += "<font color=\"#" +
                    QString( colorMap[_colors & 0x0F] ) + "\">";
        closeTags = "</font>" + closeTags;
    }
    if ( ( _format & 0x0C ) == 0x04 ) {
        openTags += "<big>";
        closeTags = "</big>" + closeTags;
    } else if ( ( _format & 0x0C ) == 0x08 ) {
        openTags += "<small>";
        closeTags = "</small>" + closeTags;
    }
    if ( ( _format & 0x10 ) != 0x00 ) {
        openTags += "<b>";
        closeTags = "</b>" + closeTags;
    }
    if ( ( _format & 0x20 ) != 0x00 ) {
        openTags += "<i>";
        closeTags = "</i>" + closeTags;
    }
    if ( ( _format & 0x40 ) != 0x00 ) {
        openTags += "<u>";
        closeTags = "</u>" + closeTags;
    }
    if ( ( _format & 0x80 ) != 0x00 ) {
        openTags += "<strike>";
        closeTags = "</strike>" + closeTags;
    }

    // Update the current view of the tags.
    if ( ( _format & 0x03 ) != formatter.alignment() ) {
        pending += pendingAlignment;
        openTags = openAlignmentTags + openTags;
    }
    formatter.setAlignment( _format & 0x03 );
    formatter.setCloseTags( closeTags );
    formatter.setCloseAlignmentTags( closeAlignmentTags );

    // Add the formatting tags to output at this location.
    formatter.addText( pending + openTags );
}

void EmsTextFormattingPart::endFormat( EmsFormatter& formatter )
{
    // If the length is zero, then we have set a default and we don't
    // want to reset it until the next text formatting change.
    if ( length() == 0 ) {
        return;
    }

    // Add the accumulated format close tags, but not the alignment tags.
    formatter.addText( formatter.closeTags() );
    formatter.setCloseTags( QString() );
}

void EmsImagePart::startFormat( EmsFormatter& formatter )
{
    formatter.addText( "<img src=\"" + _source + "\">" );
}

void EmsAttachmentPart::startFormat( EmsFormatter& formatter )
{
    formatter.addText( "<a href=\"" + _source + "\">" + _name + "</a>" );
}

// Predefined EMS image types, from GSM 23.040, section 9.2.3.24.10.3.3.
static const char * const predefinedImages[15] = {
    "ironic",               // No tr
    "glad",                 // No tr
    "skeptical",            // No tr
    "sad",                  // No tr
    "wow",                  // No tr
    "crying",               // No tr
    "winking",              // No tr
    "laughing",             // No tr
    "indifferent",          // No tr
    "kissing",              // No tr
    "confused",             // No tr
    "tongue",               // No tr
    "angry",                // No tr
    "glasses",              // No tr
    "devil"                 // No tr
};

void SMSDecoder::formatMessage( QMailMessage& mail, const QSMSMessage& msg )
{
    QList<QSMSMessagePart> parts = msg.parts();
    uint part;
    bool hasDatagram = false;
    QString mimeType;
    uint position;
    uint partNum = 1;
    EmsFormatter formatter;
    QString id;

    for ( part = 0; part < static_cast<uint>(parts.count()); ++part ) {
        if ( parts[part].isText() ) {

            // Process the parts that relate to this text part.
            formatter.flushParts( parts[part].text() );

        } else {
            mimeType = parts[part].mimeType();
            position = parts[part].position();
            if ( mimeType.startsWith( "application/x-qtopia-wdp" ) ) {

                // The message contains a WDP datagram, which we
                // need to process separately below.
                hasDatagram = true;

            } else if ( mimeType.startsWith( "image/" ) ) {

                // Process an inline image.
                EmsImagePart *emsPart = new EmsImagePart();
                emsPart->setStart( position );
                id = "part" + QString::number( partNum ) + "@local";
                emsPart->setSource( "cid:" + id );
                formatter.addPart( emsPart );
                QMailMessagePart mpart;
                mpart.setContentID( "<" + id + ">" );
                QMailMessageContentType type( mimeType.toLatin1() );
                mpart.setBody( QMailMessageBody::fromData( parts[part].data(), type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );
                mail.appendPart( mpart );
                ++partNum;

            } else if ( mimeType == "application/x-qtopia-sms-10" ) {

                // EMS text formatting information.
                QByteArray contents = parts[part].data();
                if ( contents.count() >= 3 ) {
                    EmsTextFormattingPart *emsPart;
                    emsPart = new EmsTextFormattingPart();
                    emsPart->setStart( contents[0] & 0xFF );
                    emsPart->setLength( contents[1] & 0xFF );
                    emsPart->setFormat( contents[2] & 0xFF );
                    if ( contents.count() >= 4 ) {
                        emsPart->setColors( contents[3] & 0xFF );
                    }
                    formatter.addPart( emsPart );
                }

            } else if ( mimeType == "application/x-qtopia-predefined-animation" ) {

                // EMS predefined animation.
                if ( parts[part].data().count() > 0 ) {
                    int imageNum = parts[part].data()[0] & 0xFF;
                    if ( imageNum < 15 ) {
                        EmsImagePart *emsPart = new EmsImagePart();
                        emsPart->setStart( position );
                        emsPart->setSource
                            ( QString( "x-sms-predefined:" ) + // No tr
                              predefinedImages[imageNum] );
                        formatter.addPart( emsPart );
                    }
                }

            } else {

                // Everything else is inserted inline as-is.
                EmsAttachmentPart *emsPart = new EmsAttachmentPart();
                emsPart->setStart( position );
                id = "part" + QString::number( partNum ) + "@local";
                emsPart->setSource( "cid:" + id );
                emsPart->setName( QObject::tr("Attachment: ") + mimeType );
                formatter.addPart( emsPart );

                QMailMessageContentType type( mimeType.toLatin1() );
                type.setName( QByteArray( "part" ) + QByteArray::number( partNum ) );

                QMailMessageContentDisposition disposition( QMailMessageContentDisposition::Inline );

                QMailMessagePart mpart;
                mpart.setContentID( "<" + id + ">" );
                mpart.setContentDisposition( disposition );
                mpart.setBody( QMailMessageBody::fromData( parts[part].data(), type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );

                mail.appendPart( mpart );
                ++partNum;
            }
        }
    }

    // TODO: is it reasonable to use QP instead of B64 here?

    // Flush the remaining parts and get the resultant text/html.
    formatter.flushParts( "", true );
    if ( !formatter.text().isEmpty() ) {
        QMailMessagePart htmlPart;
        QMailMessageContentType type( "text/html; charset=UTF-8" );
        htmlPart.setBody( QMailMessageBody::fromData( formatter.text(), type, QMailMessageBody::QuotedPrintable ) );
        mail.setMultipartType( QMailMessage::MultipartRelated );
        mail.prependPart( htmlPart );
    }
    else if ( parts.count() > 1 ) {
        mail.setMultipartType( QMailMessage::MultipartMixed );
    }

    // If we saw a WDP datagram, then we need to process it now.
    if ( hasDatagram ) {
        int port = msg.destinationPort();
        QString type = SMSDecoder::mimeTypeForPort( port );
        SMSDecoder *decoder;
        QMailMessagePart mpart;
        if ( !type.isNull() ) {
            // We know the MIME type for this port number.
            QMailMessageContentType contentType( type.toLatin1() );
            QMailMessageBody::TransferEncoding encoding = 
                ( contentType.type().toLower() == "text" ? QMailMessageBody::QuotedPrintable 
                                                         : QMailMessageBody::Base64 );
            mpart.setBody( QMailMessageBody::fromData( msg.applicationData(), contentType, encoding, QMailMessageBody::RequiresEncoding ) );
            mail.appendPart( mpart );
        } else if ( ( decoder = SMSDecoder::decoder( port ) ) != 0 ) {
            // Use the decoder to turn the datagram into something useful.
            decoder->decode( mail, msg );
            delete decoder;
        } else {
            // We don't know what to do with this kind of datagram.
            QMailMessageContentType contentType( QByteArray("application/x-smsapp-") + QByteArray::number( port ) );
            mpart.setBody( QMailMessageBody::fromData( msg.applicationData(), contentType, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding ) );
            mail.appendPart( mpart );
        }
    }
}
#endif

