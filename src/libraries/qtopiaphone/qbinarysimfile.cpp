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

#include <qbinarysimfile.h>
#include <qsimfiles.h>
#include <quuid.h>
#include <qtimer.h>

/*!
    \class QBinarySimFile
    \inpublicgroup QtTelephonyModule

    \brief The QBinarySimFile class provides an interface to access binary files on a SIM.

    This class is typically used to access auxiliary information on a SIM
    such as icons, extended address book attributes, MMS settings, etc.

    This class can only be used on binary SIM files.  For record-based SIM
    files, use QRecordBasedSimFile instead.

    Binary SIM files are usually limited in size to 64 kbytes.  Attempting
    to read beyond this boundary will result in an error.  Read requests
    are also typically limited to no more than 256 bytes at a time.

    Unlike regular files, it isn't possible to supply a large buffer size
    and ask the SIM to read as many bytes as possible.  It is necessary
    to explicitly state the exact number of bytes required, even if short.
    Most binary files contain fixed-sized header information that can be
    used to determine the size and position of future reads.

    Consult 3GPP TS 11.11 and 3GPP TS 51.011 for more information on the
    format of individual SIM files.

    \sa QRecordBasedSimFile, QSimFiles

    \ingroup telephony
*/

class QBinarySimFilePrivate
{
public:
    QSimFiles *files;
    QString fileid;
    QString reqid;
};

/*!
    Create a binary SIM file access object for \a fileid and attach
    it to \a parent.  The \a service parameter indicates the telephony
    service to use to access the file, or an empty string to use the
    default service.

    The \a fileid should be the complete hexadecimal path to the file on
    the SIM.  For example, the \i EFimg file is numbered \c 4F20 underneath
    the \i DFgraphics directory (\c 5F50), which is in turn underneath the
    \i DFtelecom directory (\c 7F10).  Therefore, \a fileid should be
    \c 7F105F504F20.
*/
QBinarySimFile::QBinarySimFile
        ( const QString& fileid, const QString& service, QObject *parent )
    : QObject( parent )
{
    d = new QBinarySimFilePrivate();
    d->files = new QSimFiles( service, this );
    d->fileid = fileid;
    d->reqid = QUuid::createUuid().toString();
    connect( d->files, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SLOT(serverError(QString,QTelephony::SimFileError)) );
    connect( d->files,
             SIGNAL(fileInfo(QString,int,int,QTelephony::SimFileType)),
             this, SLOT(serverFileInfo(QString,int)) );
    connect( d->files, SIGNAL(readDone(QString,QByteArray,int)),
             this, SLOT(serverReadDone(QString,QByteArray,int)) );
    connect( d->files, SIGNAL(writeDone(QString,int)),
             this, SLOT(serverWriteDone(QString,int)) );
}

/*!
    Destroy this binary SIM file access object.
*/
QBinarySimFile::~QBinarySimFile()
{
    delete d->files;
    delete d;
}

/*!
    Request the current size of this binary SIM file.  The value
    will be returned in a fileSize() signal.

    \sa fileSize()
*/
void QBinarySimFile::requestFileSize()
{
    if ( d->files->available() )
        d->files->requestFileInfo( d->reqid, d->fileid );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    Request that \a len bytes be read from this binary file at
    position \a pos.  The data will be returned in a readDone()
    signal.

    \sa readDone(), write()
*/
void QBinarySimFile::read( int pos, int len )
{
    if ( d->files->available() )
        d->files->readBinary( d->reqid, d->fileid, pos, len );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    Write \a len bytes of \a data to position \a pos in this binary file.
    The writeDone() signal will be emitted once the write completes.

    \sa writeDone(), read()
*/
void QBinarySimFile::write( int pos, const char *data, int len )
{
    QByteArray ba( data, len );
    if ( d->files->available() )
        d->files->writeBinary( d->reqid, d->fileid, pos, ba );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    Write \a data to position \a pos in this binary file.  The writeDone()
    signal will be emitted once the write completes.

    \sa writeDone(), read()
*/
void QBinarySimFile::write( int pos, const QByteArray& data )
{
    if ( d->files->available() )
        d->files->writeBinary( d->reqid, d->fileid, pos, data );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    \fn void QBinarySimFile::error( QTelephony::SimFileError err )

    Signal that is emitted if an error \a err occurred while reading
    or writing this binary file.

    \sa read(), write()
*/

/*!
    \fn void QBinarySimFile::fileSize( int size )

    Signal that is emitted when the \a size of this binary file
    is returned in response to a requestFileSize() operation.

    \sa requestFileSize()
*/

/*!
    \fn void QBinarySimFile::readDone( const QByteArray& data, int pos )

    Signal that is emitted when a read from position \a pos of this
    binary file completes.  The \a data parameter contains the data
    that was read from the file.

    \sa read()
*/

/*!
    \fn void QBinarySimFile::writeDone( int pos )

    Signal that is emitted when a write to position \a pos of
    this binary file completes.

    \sa write()
*/

void QBinarySimFile::serviceUnavailable()
{
    emit error( QTelephony::SimFileServiceUnavailable );
}

void QBinarySimFile::serverError
    ( const QString& reqid, QTelephony::SimFileError err )
{
    if ( reqid == d->reqid )
        emit error( err );
}

void QBinarySimFile::serverFileInfo( const QString& reqid, int size )
{
    if ( reqid == d->reqid )
        emit fileSize( size );
}

void QBinarySimFile::serverReadDone
    ( const QString& reqid, const QByteArray& data, int pos )
{
    if ( reqid == d->reqid )
        emit readDone( data, pos );
}

void QBinarySimFile::serverWriteDone( const QString& reqid, int pos )
{
    if ( reqid == d->reqid )
        emit writeDone( pos );
}
