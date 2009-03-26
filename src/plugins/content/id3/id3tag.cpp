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

#include "id3tag.h"

#include <QTextStream>
#include <QStringList>
#include <QImage>
#include <QTextCodec>
#include <QtDebug>

static const char *codecForEncoding( char e )
{
    switch( e )
    {
    case 0x00:
        return "ISO 8859-1";
    case 0x01:
        return "UTF-16";
    case 0x02:
        return "UTF-16E";
    case 0x03:
        return "UTF-8";
    default:
        return "UTF-8";
    };
}

bool Id3Tag::isValidFrame(const FrameHeader &header) const
{
    return header.size != 0
            && (header.idBytes[0] >= '0' && header.idBytes[0] <= 'Z' || header.idBytes[0] == '\0')
            && (header.idBytes[1] >= '0' && header.idBytes[1] <= 'Z')
            && (header.idBytes[2] >= '0' && header.idBytes[2] <= 'Z')
            && (header.idBytes[3] >= '0' && header.idBytes[3] <= 'Z' || header.idBytes[3] == '\0');
}

static quint32 convertSyncSafeInteger( quint32 syncSafe )
{
    return ((syncSafe & 0x7F000000) >> 0x03) |
           ((syncSafe & 0x007F0000) >> 0x02) |
           ((syncSafe & 0x00007F00) >> 0x01) |
           ((syncSafe & 0x0000007F)        );
}

static QByteArray unsynchronise( const QByteArray &data )
{
    if( data.size() == 0 )
        return data;

    int shift = 0;

    QByteArray unsynced = data;

    for( int i = 1; i < data.size(); i++ )
    {
        if( data[ i - 1 ] == (char)0xFF && data[ i ] == (char)0x00 )
            shift++;
        else if( shift )
            unsynced[ i - shift ] = data[ i ];
    }

    if( shift )
        unsynced.chop( shift );

    return unsynced;
}

/*!
    \class Id3Tag
    \internal
*/
Id3Tag::Id3Tag( QIODevice *device )
    : m_stream( device )
    , m_frameParsed( true )
    , m_unsynchroniseFrames( false )
    , m_isSyncSafe(false)
    , m_isNotSyncSafe(false)
{
    {
        QDataStream stream( device );

        stream.setByteOrder( QDataStream::BigEndian );

        stream.readRawData( m_tagHeader.id, 3 );

        stream >> m_tagHeader.version;
        stream >> m_tagHeader.flags;
        stream >> m_tagHeader.size;
    }

    m_stream.setByteOrder( QDataStream::BigEndian );

    m_tagHeader.size = convertSyncSafeInteger( m_tagHeader.size );

    if( !isValid() )
        return;

    QByteArray tagData = device->read( m_tagHeader.size );

    if( m_tagHeader.majorVersion == 3 && m_tagHeader.flags & TagUnsynchronisation )
        tagData = unsynchronise( tagData );

    m_tagBuffer.setData( tagData );

    if( !m_tagBuffer.open( QIODevice::ReadOnly ) )
        return;

    m_stream.setDevice( &m_tagBuffer );

    m_unsynchroniseFrames = m_tagHeader.majorVersion == 4 && m_tagHeader.flags & TagUnsynchronisation;

    if( m_tagHeader.flags & TagHasExtendedHeader )
        readExtendedHeaders();
}

Id3Tag::~Id3Tag()
{
    if( m_tagBuffer.isOpen() )
        m_tagBuffer.close();
}

quint32 Id3Tag::size() const
{
    return m_tagHeader.size;
}

bool Id3Tag::isValid() const
{
    return m_tagHeader.id[ 0 ] == 'I' &&
           m_tagHeader.id[ 1 ] == 'D' &&
           m_tagHeader.id[ 2 ] == '3' &&
           m_tagHeader.size > 0       &&
           m_tagHeader.majorVersion >= 2 &&
           m_tagHeader.majorVersion <= 4;
}

bool Id3Tag::nextFrame()
{
    if (!m_frameParsed) {
        if (m_stream.device()->pos() + m_frameHeader.size > m_tagHeader.size)
            return false;
        else
            m_stream.device()->seek(m_stream.device()->pos() + m_frameHeader.size);
    }

    if (m_stream.atEnd()) {
        m_frameParsed = true;
        m_frameValues.clear();

        m_frameHeader.id    = 0;
        m_frameHeader.size  = 0;
        m_frameHeader.flags = 0;

        return false;
    }

    m_frameParsed = false;

    if (!readFrameHeader(&m_frameHeader)) {
        return false;
    } else if (m_isSyncSafe) {
        m_frameHeader.size = convertSyncSafeInteger(m_frameHeader.size);
    } else if (!m_isNotSyncSafe && m_tagHeader.majorVersion == 4 && m_frameHeader.size >= 0x80) {
        if ((m_isSyncSafe = m_stream.device()->pos() + m_frameHeader.size > m_tagHeader.size)) {
            m_frameHeader.size = convertSyncSafeInteger(m_frameHeader.size);
        } else if (!(m_isNotSyncSafe = m_frameHeader.size & 0x80808080)) {
            qint64 pos = m_stream.device()->pos();

            quint32 syncSafeSize = convertSyncSafeInteger(m_frameHeader.size);

            m_stream.device()->seek(m_stream.device()->pos() + syncSafeSize);

            FrameHeader header;

            if ((m_isSyncSafe = readFrameHeader(&header))) {
                m_frameHeader.size = convertSyncSafeInteger(m_frameHeader.size);
            } else {
                m_stream.device()->seek(
                        m_stream.device()->pos() + m_frameHeader.size - 10 - syncSafeSize);

                m_isNotSyncSafe = readFrameHeader(&header);
            }
            m_stream.device()->seek(pos);
        }
    }

    return true;
}

bool Id3Tag::readFrameHeader(FrameHeader *header)
{
    if( m_tagHeader.majorVersion == 2 )
    {
        m_stream.readRawData( header->idBytes, 3 );
        header->idBytes[3] = '\0';

        union
        {
            quint32 size;
            char sizeBytes[ 4 ];
        };

        sizeBytes[ 0 ] = '\0';
        m_stream.readRawData( &(sizeBytes[ 1 ]), 3 );

        header->size = qFromBigEndian( size );
        header->flags = 0;
    }
    else
    {
        if( m_stream.readRawData( header->idBytes, 4 ) != 4 )
            return false;

        m_stream >> header->size;
        m_stream >> header->flags;
    }

    header->id = qFromBigEndian(header->id);

    return isValidFrame(*header);
}

int Id3Tag::frameId() const
{
    return m_frameHeader.id;
}

QVariantList Id3Tag::frameValues() const
{
    if( m_frameParsed )
        return m_frameValues;

    m_frameParsed = true;

    m_frameValues = QVariantList();

    QByteArray data = m_stream.device()->read( m_frameHeader.size );

    if( m_frameHeader.flags & FrameUnsynchronisation || m_unsynchroniseFrames )
        data = unsynchronise( data );

    if( m_frameHeader.flags & FrameCompression )
        data = qUncompress( data );

    if( m_frameHeader.flags & FrameDataLengthIndicator )
        data = data.right( data.size() - 4 );

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    if( m_frameHeader.idBytes[ 0 ] == 'T' ) {
#else
    if( m_frameHeader.idBytes[ 3 ] == 'T' ) {
#endif
            QBuffer buffer( &data );

            buffer.open( QIODevice::ReadOnly );

            const char *codec = codecForEncoding( buffer.read( 1 )[ 0 ] );

            QTextStream textStream( &buffer );

            textStream.setCodec( codec );

            QString all = textStream.readAll();

            if( all.endsWith( QChar( '\0' ) ) )
                all.chop( 1 );

            foreach( QString string, all.split( m_tagHeader.majorVersion == 2 ? '\\' : '\0' ) )
                m_frameValues.append( string );
    }
    else
    {
        switch( m_frameHeader.id )
        {
        case PIC:
        case APIC:
            {
                QByteArray mimeType;
                int pos = 1;
                int length = 0;

                if( m_tagHeader.majorVersion == 2 )
                {
                    mimeType = data.mid( 1, 3 );

                    pos = 4;
                }
                else
                {
                    length = data.indexOf( '\0', pos ) - pos;

                    mimeType = data.mid( pos, length );

                    pos += length + 1;
                }

                char pictureType = data[ pos ];

                pos++;

                switch( data[ 0 ] )
                {
                case 0:
                case 3:
                    length = data.indexOf( '\0', pos ) - pos;
                break;
                case 1:
                case 2:
                    length = data.indexOf( QByteArray::fromRawData( "\0\0", 2 ), pos ) - pos  + 1;
                default:
                    return m_frameValues;
                }

                QString description;
                QTextCodec *codec = QTextCodec::codecForName( codecForEncoding( data[ 0 ] ) );
                if( codec )
                    description = codec->toUnicode( data.mid( pos, length ) );

                pos += length + 1;

                QImage picture = QImage::fromData( data.mid( pos ) );;

                picture.loadFromData( data.mid( pos ) );

                m_frameValues << mimeType << pictureType << description << picture;
            }
            break;
        }
    }

    return m_frameValues;
}

void Id3Tag::readExtendedHeaders()
{
    qint64 pos = m_stream.device()->pos();

    quint32 size;

    m_stream >> size;

    if( m_tagHeader.majorVersion == 4 )
        size = convertSyncSafeInteger( size );

    m_stream.device()->seek( pos + size );
}

