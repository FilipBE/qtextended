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

#include "imageio.h"

#include <cmath>

#include <qfile.h>
#include <qfileinfo.h>

#include <QImageWriter>

ImageIO::ImageIO( QObject* parent )
    : QObject( parent ), image_levels( 0 ), image_samples( 0 )
{ }

ImageIO::~ImageIO()
{
    // Delete loaded image samples
    delete[] image_samples;
}

ImageIO::Status ImageIO::load( const QContent& lnk, int levels )
{
    static const int maxFileSize = 2097152;

    _lnk = lnk;

    QImageReader reader( lnk.fileName() );

    _format = reader.format();

    int maxArea = maxSize().width() * maxSize().height();

    QImage image;

    if( reader.supportsOption( QImageIOHandler::Size ) )
    {
        image_size = reader.size();

        if( !image_size.isValid() || image_size.width() * image_size.height() > maxArea )
        {
            _status = SIZE_ERROR;

            return _status;
        }
    }
    else if( QFileInfo( lnk.file() ).size() > maxFileSize )
    {
        image_size = QSize();

        _status = SIZE_ERROR;

        return _status;
    }

    if( reader.read( &image ) )
        _status = load( image, levels );
    else
        _status = LOAD_ERROR;

    return _status;
}

ImageIO::Status ImageIO::load( const QImage& image, int levels )
{
#define SUPPORTED_DEPTH QImage::Format_ARGB32
    // Remove previously loaded image samples
    delete[] image_samples;

    image_size = image.size();

    // Create image levels
    image_samples = new QImage[ image_levels = levels ];

    if( image.isNull() ) {
        // Notify of change to image
        emit changed();

        _status = LOAD_ERROR;

        return LOAD_ERROR;
    }

    // Load the original image
    switch( image.format() )
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        image_samples[ 0 ] = image;
        break;
    default:
        {
            // Try to convert image to supported depth
            image_samples[ 0 ] = image.convertToFormat( QImage::Format_ARGB32 );
            if( image_samples[ 0 ].isNull() ) {
                // Notify of change to image
                emit changed();

                _status = DEPTH_ERROR;

                return DEPTH_ERROR;
            }
        }
    }

    // For each level after the first
    for( int i = 1; i < levels; ++i ) {
        // Load image half the size of the previous image
        QImage *prev_sample = image_samples + i - 1;
        int width = prev_sample->width() / 2;
        int height = prev_sample->height() / 2;
        image_samples[ i ] = prev_sample->scaled( width ? width : 1, height ? height : 1 );
    }

    // Notify of change to image
    emit changed();

    _status = NORMAL;

    return NORMAL;
}

bool ImageIO::isSaveSupported() const
{
    return QImageWriter::supportedImageFormats().contains( _format );
}

bool ImageIO::isReadOnly() const
{
    QFileInfo origFile( _lnk.fileName() );
    if ( !origFile.exists() || !origFile.isWritable() || _status != NORMAL )
        return true;

    return false;
}

QString ImageIO::saveType() const
{
#define DEFAULT_MIME_TYPE "image/png"

    return isSaveSupported()
            ? _lnk.type()
            : QLatin1String(DEFAULT_MIME_TYPE);
}

bool ImageIO::save(const QImage& image, QContent *content)
{
#define DEFAULT_FORMAT "PNG"


    // If saving supported, save using original format
    // Otherwise, save using default format
    bool saved = false;

    QByteArray format = isSaveSupported()
            ? _format
            : DEFAULT_FORMAT;

    if (QIODevice *device = content->open(QIODevice::WriteOnly)) {
        if ((saved = image.save(device, format.constData()))) {
            content->commit();

            _lnk = *content;
        }

        delete device;
    }

    return saved;
}

int ImageIO::level( double x ) const
{
    // Determine level using inverse reduction function log2 1/x
    int n = (int)( log( 1.0 / x ) / log( 2.0 ) );
    // Limit level to within range
    if( n < 0 ) n = 0;
    else if( n >= image_levels ) n = image_levels - 1;

    return n;
}

double ImageIO::factor( int n ) const
{
    // Determine factor using reduction function 1/2^n
    return 1.0 / pow( 2, n );
}

// Return image at level
QImage ImageIO::image(int level)
{
    return image_levels < 1 ? QImage() : image_samples[ level ];
}

QImage ImageIO::image( const QRect& rect, int level ) const
{
    if ( image_levels < 1 ) {
        return QImage();
    }

    // Reduce desired area using reduction function
    QRect area( rect );
    double reduction_factor = factor( level );
    area.setTop( (int)( area.top() * reduction_factor ) );
    area.setLeft( (int)( area.left() * reduction_factor ) );
    area.setBottom( (int)( area.bottom() * reduction_factor ) );
    area.setRight( (int)( area.right() * reduction_factor ) );

    QRect sample( image_samples[ level ].rect() );
    // If area is wider than image, reduce area to fit width
    if( area.left() < sample.left() ) area.setLeft( sample.left() );
    if( area.right() > sample.right() ) area.setRight( sample.right() );

    // If area is taller than image, reduce area to fit height
    if( area.top() < sample.top() ) area.setTop( sample.top() );
    if( area.bottom() > sample.bottom() ) area.setBottom( sample.bottom() );

    return image_samples[ level ].copy( area );
}

QSize ImageIO::maxSize()
{
    static const int maxWidth = 1600;
    static const int maxHeight = 1200;

    return QSize(maxWidth, maxHeight);
}

