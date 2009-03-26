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

#include "exifcontentplugin.h"
#include "exifcontentproperties.h"
#include <QExifImageHeader>
#include <qmimetype.h>

/*!
    \class ExifContentPlugin
    \internal

    Plugin for reading meta data from images with exif tags.
*/

ExifContentPlugin::ExifContentPlugin()
{
}

ExifContentPlugin::~ExifContentPlugin()
{
}

QStringList ExifContentPlugin::keys() const
{
    return QMimeType( QLatin1String( "image/jpeg" ) ).extensions();
}

bool ExifContentPlugin::installContent( const QString &filePath, QContent *content )
{
    bool success = false;

    QExifImageHeader exif;

    if( exif.loadFromJpeg( filePath ) )
    {
        QDateTime date = exif.value(QExifImageHeader::DateTime).toDateTime();
        if (date.isValid()) {
            content->setProperty(QLatin1String("CreationDate"), date.toString(Qt::ISODate));
        }
        QString string = exif.value( QExifImageHeader::ImageDescription ).toString();
        if( !string.isEmpty() )
            content->setProperty( QContent::Description, string );

        string = exif.value( QExifImageHeader::Artist ).toString();
        if( !string.isEmpty() )
            content->setProperty( QContent::Artist, string );

        content->setName( QFileInfo( filePath ).baseName() );
        content->setType( QMimeType( filePath ).id() );
        content->setFile( filePath );

        success = true;
    }

    return success;
}

bool ExifContentPlugin::updateContent( QContent *content )
{
    return installContent( content->fileName(), content );
}

QStringList ExifContentPlugin::mimeTypes() const
{
    return QStringList()
            << QLatin1String("image/jpeg");
}

QImage ExifContentPlugin::thumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode )
{
    QImage image;

    QIODevice *device = content.open();

    if (device) {
        QExifImageHeader exif;

        if (exif.loadFromJpeg(device)) {
            image = exif.thumbnail();

            if (!image.isNull() && size.isValid())
                image = image.scaled(size, mode, Qt::SmoothTransformation);
        }

        device->close();

        delete device;
    }

    return image;
}

QContentPropertiesEngine *ExifContentPlugin::createPropertiesEngine( const QContent &content )
{
    return new ExifContentProperties( content );
}

QTOPIA_EXPORT_PLUGIN(ExifContentPlugin);
