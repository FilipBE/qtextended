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

#include "id3contentplugin.h"
#include "id3tag.h"
#include <qmimetype.h>
#include <QtDebug>

/*!
    \class Id3ContentHandler
    \internal
 */

static const char *genreLookup[] =
{
    "Blues",
    "Classic Rock",
    "Country",
    "Dance",
    "Disco",
    "Funk",
    "Grunge",
    "Hip-Hop",
    "Jazz",
    "Metal",
    "New Age",
    "Oldies",
    "Other",
    "Pop",
    "R&B",
    "Rap",
    "Reggae",
    "Rock",
    "Techno",
    "Industrial",
    "Alternative",
    "Ska",
    "Death Metal",
    "Pranks",
    "Soundtrack",
    "Euro-Techno",
    "Ambient",
    "Trip-Hop",
    "Vocal",
    "Jazz+Funk",
    "Fusion",
    "Trance",
    "Classical",
    "Instrumental",
    "Acid",
    "House",
    "Game",
    "Sound Clip",
    "Gospel",
    "Noise",
    "Alternative Rock",
    "Bass",
    "Soul",
    "Punk",
    "Space",
    "Meditative",
    "Instrumental Pop",
    "Instrumental Rock",
    "Ethnic",
    "Gothic",
    "Darkwave",
    "Techno-Industrial",
    "Electronic",
    "Pop-Folk",
    "Eurodance",
    "Dream",
    "Southern Rock",
    "Comedy",
    "Cult",
    "Gangsta",
    "Top 40",
    "Christian Rap",
    "Pop/Funk",
    "Jungle",
    "Native America",
    "Cabaret",
    "New Wave",
    "Psychdelic",
    "Rave",
    "Showtunes",
    "Trailer",
    "Lo-Fi",
    "Tribal",
    "Acid Punk",
    "Acid Jazz",
    "Polka",
    "Retro",
    "Musical",
    "Rock & Roll",
    "Hard Rock"
};

/*!
    \class Id3ContentPlugin
    \internal

    Plugin for reading content information from ID3 tags in media files.
*/

Id3ContentPlugin::Id3ContentPlugin()
{
}

Id3ContentPlugin::~Id3ContentPlugin()
{
}

QStringList Id3ContentPlugin::keys() const
{
    return QMimeType::fromId( QLatin1String( "audio/mpeg" ) ).extensions();
}

bool Id3ContentPlugin::installContent( const QString &filePath, QContent *content )
{
    QFile file( filePath );

    bool success = false;

    if( file.open( QIODevice::ReadOnly ) )
    {
        if( file.peek( 3 ) == "ID3" )
        {
            Id3Tag tag( &file );

            content->setFile( filePath );
            content->setType( QMimeType::fromFileName( filePath ).id() );
            content->setRole( QContent::Document );

            while( tag.nextFrame() )
            {
                switch( tag.frameId() )
                {
                case Id3Tag::TPE1:
                case Id3Tag::TP1:
                    {
                        QVariantList values = tag.frameValues();

                        if( !values.isEmpty() )
                            content->setProperty( QContent::Artist, qvariant_cast< QString >( values.first() ) );
                    }
                    break;
                case Id3Tag::TALB:
                case Id3Tag::TAL:
                    {
                        QVariantList values = tag.frameValues();

                        if( !values.isEmpty() )
                            content->setProperty( QContent::Album, qvariant_cast< QString >( values.first() ) );
                    }
                    break;
                case Id3Tag::TCON:
                case Id3Tag::TCO:
                    {
                        QVariantList values = tag.frameValues();

                        if( !values.isEmpty() )
                        {
                            QString genreId = qvariant_cast< QString >( values.first() );

                            if( genreId.startsWith( '(' ) && genreId.endsWith( ')' ) )
                                genreId = genreId.mid( 1, genreId.size() - 2 );

                            bool ok;

                            uint index = genreId.toInt( &ok );

                            if( ok && index < sizeof( genreLookup ) / sizeof( char * ) )
                                content->setProperty( QContent::Genre, QLatin1String( genreLookup[ index ] ) );
                            else
                                content->setProperty( QContent::Genre, genreId );
                        }
                    }
                    break;
                case Id3Tag::TRCK:
                case Id3Tag::TRK:
                    {
                        QVariantList values = tag.frameValues();

                        if( !values.isEmpty() )
                        {
                            QString track = qvariant_cast< QString >( values.first() );

                            int index = track.indexOf( '/' );
                            if( index != -1 )
                                track = track.left( index );

                            // Prepend 0 to single digit tracks
                            if( track.count() == 1 )
                                track.prepend( QLatin1String( "0" ) );

                            content->setProperty( QContent::Track, track );
                        }
                    }
                    break;
                case Id3Tag::TIT2:
                case Id3Tag::TT2:
                    {
                        QVariantList values = tag.frameValues();

                        if( !values.isEmpty() )
                            content->setProperty( QContent::Title, qvariant_cast< QString >( values.first() ) );
                    }
                    break;
                }
            }

            success = true;
        }
        file.close();
    }

    return success;
}

bool Id3ContentPlugin::updateContent( QContent *content )
{
    return installContent( content->fileName(), content );
}

QStringList Id3ContentPlugin::mimeTypes() const
{
    return QStringList()
            << QLatin1String("audio/mpeg")
            << QLatin1String("audio/mpeg3")
            << QLatin1String("audio/mp3")
            << QLatin1String("audio/x-mp3");
}

QImage Id3ContentPlugin::thumbnail(const QContent &content, const QSize &size, Qt::AspectRatioMode mode)
{
    static const int typePreference[] =
    {
        0x03,   // Cover front.
        0x02,   // Other file icon.
        0x01,   // 32x32 pixels 'file icon'.
        0x13,   // Band/Artist logotype.
        0x08,   // Artist Performer.
        0x07,   // Lead artist/lead performer/soloist.
        0x09,   // Conductor.
        0x0A,   // Band/Orchestra.
        0x0B,   // Composer.
        0x0C,   // Lyricist/text writer.
        0x12,   // Illustration
        0x00,   // Other.
        0x14    // Publisher/Studio logotype
    };

    QIODevice *device = content.open();

    QImage thumbnail;
    int thumbnailType = -1;

    if (device && device->peek( 3 ) == "ID3") {
        Id3Tag tag(device);

        while (tag.nextFrame()) {
            if (tag.frameId() == Id3Tag::APIC || tag.frameId() == Id3Tag::PIC) {
                QVariantList values = tag.frameValues();

                if (values.count() == 4) {
                    const int type = values.at(1).toInt();
                    const int typeCount = sizeof(typePreference) / sizeof(int);

                    for (int i = 0; i < typeCount && typePreference[i] != thumbnailType; ++i ) {
                        if (typePreference[i] == type ) {
                            QImage picture = qvariant_cast<QImage>(values.at(3)) ;

                            if (!picture.isNull()) {
                                thumbnail = picture;
                                thumbnailType = type;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    if(device)
        delete device;


    return !thumbnail.isNull() && size.isValid()
            ? thumbnail.scaled(size, mode)
            : thumbnail;
}

QContentPropertiesEngine *Id3ContentPlugin::createPropertiesEngine( const QContent &content )
{
    Q_UNUSED( content );

    return 0;
}

QTOPIA_EXPORT_PLUGIN(Id3ContentPlugin);

