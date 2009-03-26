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

#include <qsimiconreader.h>
#include "qsimiconreader_p.h"
#include <qrecordbasedsimfile.h>
#include <qatutils.h>
#include <QMap>
#include <QList>
#include <QTimer>

/*!
    \class QSimIconReader
    \inpublicgroup QtTelephonyModule

    \brief The QSimIconReader class provides access to icons on a SIM.
    \ingroup telephony

    This class is a convenience class wrapped around QBinarySimFile, to simplify
    access to SIM icons.  The class caches previous icon images so that
    multiple requests for the same icon will not entail multiple commands
    to the SIM.

    This class supports the monochrome and color icon formats described
    in 3GPP TS 51.011.

    \sa QBinarySimFile, QSimToolkit
*/

class QSimIconReaderPrivate
{
public:
    QString service;
    QRecordBasedSimFile *indexFile;
    int numIcons;
    QMap<int, QImage> cachedIcons;
    QList<int> pendingAvailable;
    QList<int> pendingNotFound;
    QList<int> pendingOnIndex;
    QList<int> pendingOnData;
    bool invalidIndex;
    bool indexRead;
    QList<QByteArray> indexData;
};

/*!
    Construct a new SIM icon reader and attach it to \a parent.
    The icon files will be read from \a service.
*/
QSimIconReader::QSimIconReader( const QString& service, QObject *parent )
    : QObject( parent )
{
    d = new QSimIconReaderPrivate();
    d->service = service;
    d->numIcons = -1;
    d->invalidIndex = false;
    d->indexRead = false;

    // Request the number of icons from the SIM index file.
    d->indexFile = new QRecordBasedSimFile( "7F105F504F20", service, this );
    connect( d->indexFile, SIGNAL(error(QTelephony::SimFileError)),
             this, SLOT(indexError()) );
    connect( d->indexFile, SIGNAL(fileInfo(int,int)),
             this, SLOT(indexFileInfo(int,int)) );
    connect( d->indexFile, SIGNAL(readDone(QByteArray,int)),
             this, SLOT(indexRead(QByteArray,int)) );
    d->indexFile->requestFileInfo();
}

/*!
    Destroy this SIM icon reader.
*/
QSimIconReader::~QSimIconReader()
{
    delete d;
}

/*!
    Returns true if we have already required icon \a iconId; false otherwise.
    If this function returns true, then the return value from icon() for
    \a iconId will be a valid image.

    \sa icon(), requestIcon()
*/
bool QSimIconReader::haveIcon( int iconId ) const
{
    return d->cachedIcons.contains( iconId );
}

/*!
    Returns the cached icon image associated with \a iconId.  If the
    icon has not been requested yet, this will return a null QImage.

    \sa haveIcon(), requestIcon()
*/
QImage QSimIconReader::icon( int iconId ) const
{
    QMap<int, QImage>::ConstIterator it;
    it = d->cachedIcons.find( iconId );
    if ( it != d->cachedIcons.end() )
        return it.value();
    else
        return QImage();
}

/*!
    Requests that the icon associated with \a iconId be fetched
    from the SIM.  The service will respond with either the
    iconAvailable() or iconNotFound() signals.

    \sa haveIcon(), icon(), iconAvailable(), iconNotFound()
*/
void QSimIconReader::requestIcon( int iconId )
{
    // If we already have this icon, arrange to emit the iconAvailable()
    // signal the next time we enter the event loop.
    if ( d->cachedIcons.contains( iconId ) ) {
        d->pendingAvailable.append( iconId );
        if ( d->pendingAvailable.size() == 1 )
            QTimer::singleShot( 0, this, SLOT(emitPendingAvailable()) );
        return;
    }

    // Determine if the icon identifier is obviously invalid.
    // Arrange to emit iconNotFound() if it is.
    if ( d->invalidIndex || iconId < 1 || iconId > 255 ||
         ( d->numIcons >= 0 && iconId > d->numIcons ) ) {
        d->pendingNotFound.append( iconId );
        if ( d->pendingNotFound.size() == 1 )
            QTimer::singleShot( 0, this, SLOT(emitPendingNotFound()) );
        return;
    }

    // Add the icon identifier to the pending on index list and process.
    if ( !d->pendingOnIndex.contains( iconId ) )
        d->pendingOnIndex.append( iconId );
    if ( d->indexRead )
        QTimer::singleShot( 0, this, SLOT(processPendingOnIndex()) );
}

/*!
    \fn void QSimIconReader::iconAvailable( int iconId )

    Signal that is emitted when the icon associated with \a iconId is
    available via the icon() function.

    \sa requestIcon()
*/

/*!
    \fn void QSimIconReader::iconNotFound( int iconId )

    Signal that is emitted when the icon associated with \a iconId
    could not be found, usually because \a iconId is invalid, or
    because the SIM does not support reading icon files.

    \sa requestIcon()
*/

void QSimIconReader::indexError()
{
    d->invalidIndex = true;
    d->indexRead = true;
    processPendingOnIndex();
}

void QSimIconReader::indexFileInfo( int numRecords, int recordSize )
{
    // Record the number of icons on the SIM.
    d->numIcons = numRecords;
    if ( d->numIcons == 0 ) {
        d->indexRead = true;
        processPendingOnIndex();
        return;
    }

    // Send read requests for the records in the icon index.
    for ( int recno = 0; recno < numRecords; ++recno )
        d->indexFile->read( recno, recordSize );
}

void QSimIconReader::indexRead( const QByteArray& data, int recno )
{
    // Add the returned data to the icon index list.
    d->indexData += data;

    // If we have everything now, then process pending requests.
    // In the future, we should optimise this so that it only requests
    // index information for the icons that are actually needed.
    if ( recno >= ( d->numIcons - 1 ) ) {
        d->indexRead = true;
        processPendingOnIndex();
    }
}

void QSimIconReader::emitPendingAvailable()
{
    QList<int> pending = d->pendingAvailable;
    d->pendingAvailable.clear();
    foreach ( int iconId, pending ) {
        emit iconAvailable( iconId );
    }
}

void QSimIconReader::emitPendingNotFound()
{
    QList<int> pending = d->pendingNotFound;
    d->pendingNotFound.clear();
    foreach ( int iconId, pending ) {
        emit iconNotFound( iconId );
    }
}

void QSimIconReader::processPendingOnIndex()
{
    QList<int> pending = d->pendingOnIndex;
    d->pendingOnIndex.clear();
    foreach ( int iconId, pending ) {

        // Skip the icon identifier if it is already pending on data fetch.
        if ( d->pendingOnData.contains( iconId ) )
            continue;

        // Validate the icon identifier, because we may have received
        // the icon request before we knew how many icons there were.
        if ( iconId > d->numIcons || d->invalidIndex ) {
            emit iconNotFound( iconId );
            continue;
        }

        // If we already fetched the icon, report it as available.
        if ( d->cachedIcons.contains( iconId ) ) {
            emit iconAvailable( iconId );
            continue;
        }

        // Start fetching the data for the icon image.
        QByteArray indexData = d->indexData[iconId - 1];
        int offset = ((indexData[6] & 0xFF) << 8) +
                      (indexData[7] & 0xFF);
        int length = ((indexData[8] & 0xFF) << 8) +
                      (indexData[9] & 0xFF);
        QString fileid = "7F105F50";
        fileid += QAtUtils::toHex( indexData.mid( 4, 2 ) );
        d->pendingOnData.append( iconId );

        // Determine if we need a color or monochrome reader.
        // Note: I'm not sure whether the values in 3GPP TS 51.011
        // are hex or decimal, so I will be paranoid and check for both.
        if ( indexData[3] == (char)0x21 ||
             indexData[3] == (char)21 ) {
            // Color icon image.
            QSimColorIconReader *colorReader;
            colorReader = new QSimColorIconReader
                ( d->service, iconId, fileid, offset, length, this );
            connect( colorReader, SIGNAL(iconFetched(int,QImage)),
                     this, SLOT(iconFetched(int,QImage)) );
            connect( colorReader, SIGNAL(iconFetchFailed(int)),
                     this, SLOT(iconFetchFailed(int)) );
        } else {
            // Monochrome icon image.
            QSimMonoIconReader *monoReader;
            monoReader = new QSimMonoIconReader
                ( d->service, iconId, fileid, offset, length, this );
            connect( monoReader, SIGNAL(iconFetched(int,QImage)),
                     this, SLOT(iconFetched(int,QImage)) );
            connect( monoReader, SIGNAL(iconFetchFailed(int)),
                     this, SLOT(iconFetchFailed(int)) );
        }
    }
}

void QSimIconReader::iconFetched( int iconId, const QImage& image )
{
    d->pendingOnData.removeAll( iconId );
    d->cachedIcons[iconId] = image;
    emit iconAvailable( iconId );
}

void QSimIconReader::iconFetchFailed( int iconId )
{
    d->pendingOnData.removeAll( iconId );
    emit iconNotFound( iconId );
}

QSimMonoIconReader::QSimMonoIconReader
        ( const QString& service, int iconId,
          const QString& fileid, int offset,
          int length, QObject *parent )
    : QObject( parent )
{
    this->iconId = iconId;
    this->length = length;
    file = new QBinarySimFile( fileid, service, this );
    connect( file, SIGNAL(error(QTelephony::SimFileError)),
             this, SLOT(error()) );
    connect( file, SIGNAL(readDone(QByteArray,int)),
             this, SLOT(readDone(QByteArray)) );
    errorReported = false;
    for ( int pos = 0; pos < length; pos += 128 ) {
        int len = length - pos;
        if ( len > 128 )
            len = 128;
        file->read( offset + pos, len );
    }
}

QSimMonoIconReader::~QSimMonoIconReader()
{
}

void QSimMonoIconReader::error()
{
    if ( !errorReported ) {
        errorReported = true;
        emit iconFetchFailed( iconId );
        deleteLater();
    }
}

void QSimMonoIconReader::readDone( const QByteArray& data )
{
    contents += data;
    if ( contents.size() >= length ) {
        // The entire contents of the file have been fetched.  Decode it.
        const char *d = contents.constData();
        int size = contents.size();
        int width, height;
        if ( size >= 2 ) {
            width = (d[0] & 0xFF);
            height = (d[1] & 0xFF);
        } else {
            width = 0;
            height = 0;
        }
        if ( !width || !height ) {
            // Invalid width or height specified.
            error();
        } else if ( ( ( width * height + 7 ) / 8 ) > ( size - 2 ) ) {
            // Insufficient pixel data for the monochrome image.
            error();
        } else {
            // Convert the image and return it to the requestor.
            QImage image( width, height, QImage::Format_Mono );
            for ( int y = 0; y < height; ++y ) {
                for ( int x = 0; x < width; ++x ) {
                    int posn = y * width + x;
                    if ( ( d[2 + posn / 8] & (0x80 >> (posn % 8)) ) != 0 )
                        image.setPixel( x, y, 1 );
                    else
                        image.setPixel( x, y, 0 );
                }
            }
            emit iconFetched( iconId, image );
            deleteLater();
        }
    }
}

QSimColorIconReader::QSimColorIconReader
        ( const QString& service, int iconId,
          const QString& fileid, int offset,
          int length, QObject *parent )
    : QObject( parent )
{
    this->iconId = iconId;
    this->length = length;
    file = new QBinarySimFile( fileid, service, this );
    connect( file, SIGNAL(error(QTelephony::SimFileError)),
             this, SLOT(error()) );
    connect( file, SIGNAL(readDone(QByteArray,int)),
             this, SLOT(readDone(QByteArray)) );
    errorReported = false;
    clutLength = 0;
    read( offset, length );
}

QSimColorIconReader::~QSimColorIconReader()
{
}

void QSimColorIconReader::error()
{
    if ( !errorReported ) {
        errorReported = true;
        emit iconFetchFailed( iconId );
        deleteLater();
    }
}

void QSimColorIconReader::readDone( const QByteArray& data )
{
    const char *d;
    if ( contents.size() < length ) {
        contents += data;
        if ( contents.size() >= length ) {
            // Contents done; find and fetch the color look-up table (CLUT).
            if ( contents.size() < 6 ) {
                error();
            } else {
                d = contents.constData();
                int offset = ((d[4] & 0xFF) << 8) + (d[5] & 0xFF);
                clutLength = (d[3] & 0xFF) * 3;
                if ( !clutLength )
                    error();        // Must be at least 1.
                else
                    read( offset, clutLength );
            }
        }
    } else {
        clut += data;
        if ( clut.size() >= clutLength ) {
            // We have the entire image data now, so decode it.
            if ( contents.size() < 6 ) {
                error();
            } else {
                d = contents.constData();
                int width = (d[0] & 0xFF);
                int height = (d[1] & 0xFF);
                int bitsPerPixel = (d[2] & 0xFF);
                if ( !width || !height || bitsPerPixel < 1 ||
                     bitsPerPixel > 8 ) {
                    // Invalid width, height, or bits per pixel specified.
                    error();
                } else {
                    int size = ( width * height * bitsPerPixel + 7 ) / 8;
                    if ( ( contents.size() - 6 ) < size ) {
                        // Image data is not big enough to contain the image.
                        error();
                    } else {
                        const char *cl = clut.constData();
                        QImage image( width, height, QImage::Format_RGB32 );
                        for ( int y = 0; y < height; ++y ) {
                            for ( int x = 0; x < width; ++x ) {
                                int posn = (y * width + x) * bitsPerPixel;
                                int pixel = fetchPixel
                                    ( d + 6 + posn / 8, posn % 8,
                                      bitsPerPixel );
                                pixel *= 3;
                                int r, g, b;
                                if ( ( pixel + 2 ) < clutLength ) {
                                    r = (cl[pixel] & 0xFF);
                                    g = (cl[pixel + 1] & 0xFF);
                                    b = (cl[pixel + 2] & 0xFF);
                                } else {
                                    // Convert invalid pixel values into white.
                                    r = g = b = 0xFF;
                                }
                                image.setPixel( x, y, qRgb( r, g, b ) );
                            }
                        }
                        emit iconFetched( iconId, image );
                        deleteLater();
                    }
                }
            }
        }
    }
}

void QSimColorIconReader::read( int offset, int length )
{
    for ( int pos = 0; pos < length; pos += 128 ) {
        int len = length - pos;
        if ( len > 128 )
            len = 128;
        file->read( offset + pos, len );
    }
}

int QSimColorIconReader::fetchPixel( const char *data, int start, int size )
{
    int raw = ((data[0] & 0xFF) << 8) + (data[1] & 0xFF);
    raw <<= start;
    return ( raw >> ( 16 - size ) ) & ( ( 1 << size ) - 1 );
}
