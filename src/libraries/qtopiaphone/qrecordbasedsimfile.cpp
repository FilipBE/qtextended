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

#include <qrecordbasedsimfile.h>
#include <qsimfiles.h>
#include <quuid.h>
#include <qtimer.h>

/*!
    \class QRecordBasedSimFile
    \inpublicgroup QtTelephonyModule

    \brief The QRecordBasedSimFile class provides an interface to access record-based files on a SIM.
    \ingroup telephony

    This interface is typically used to access auxiliary information on a SIM such
    as icons, extended address book attributes, MMS settings, etc.

    This class can only be used on record-based SIM files.  For binary SIM
    files, use QBinarySimFile instead.

    Record-based SIM files are usually limited in size to 255 records.
    Attempting to read beyond this boundary will result in an error.
    Records are also typically limited to no more than 256 bytes.

    Consult 3GPP TS 11.11 and 3GPP TS 51.011 for more information on the
    format of individual SIM files.

    \sa QBinarySimFile, QSimFiles
*/

class QRecordBasedSimFilePrivate
{
public:
    QSimFiles *files;
    QString fileid;
    QString reqid;
};

/*!
    Create a record-based SIM file access object for \a fileid and attach
    it to \a parent.  The \a service parameter indicates the telephony
    service to use to access the file, or an empty string to use the
    default service.

    For example, \c 6F3C indicates the \i EFsms file.

    The \a fileid should be the complete hexadecimal path to the file on
    the SIM.  For example, the \i EFsms file is numbered \c 6F3C underneath
    the \i DFtelecom directory (\c 7F10).  Therefore, \a fileid should be
    \c 7F106F3C.
*/
QRecordBasedSimFile::QRecordBasedSimFile
        ( const QString& fileid, const QString& service, QObject *parent )
    : QObject( parent )
{
    d = new QRecordBasedSimFilePrivate();
    d->files = new QSimFiles( service, this );
    d->fileid = fileid;
    d->reqid = QUuid::createUuid().toString();
    connect( d->files, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SLOT(serverError(QString,QTelephony::SimFileError)) );
    connect( d->files,
             SIGNAL(fileInfo(QString,int,int,QTelephony::SimFileType)),
             this, SLOT(serverFileInfo(QString,int,int)) );
    connect( d->files, SIGNAL(readDone(QString,QByteArray,int)),
             this, SLOT(serverReadDone(QString,QByteArray,int)) );
    connect( d->files, SIGNAL(writeDone(QString,int)),
             this, SLOT(serverWriteDone(QString,int)) );
}

/*!
    Destroy this record-based SIM file access object.
*/
QRecordBasedSimFile::~QRecordBasedSimFile()
{
    delete d->files;
    delete d;
}

/*!
    Request the number of records and record size for this record-based file.
    The value will be returned in a fileInfo() signal.

    \sa fileInfo()
*/
void QRecordBasedSimFile::requestFileInfo()
{
    if ( d->files->available() )
        d->files->requestFileInfo( d->reqid, d->fileid );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    Request that a single record be read from this record-based file at
    position \a recno.  The data will be returned in a readDone()
    signal.  Record numbers begin at zero.

    If \a recordSize is not -1, it indicates that the record size is
    already known.  Otherwise the service will determine the record size
    for itself.

    \sa readDone(), write()
*/
void QRecordBasedSimFile::read( int recno, int recordSize )
{
    if ( d->files->available() )
        d->files->readRecord( d->reqid, d->fileid, recno, recordSize );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    Write \a len bytes of \a data to position record \a recno
    in this record-based file.  The writeDone() signal will be emitted
    once the write completes.  Record numbers begin at zero.

    \sa writeDone(), read()
*/
void QRecordBasedSimFile::write( int recno, const char *data, int len )
{
    QByteArray ba( data, len );
    if ( d->files->available() )
        d->files->writeRecord( d->reqid, d->fileid, recno, ba );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    Write \a data to record \a recno in this record-based file.
    The writeDone() signal will be emitted once the write completes.

    \sa writeDone(), read()
*/
void QRecordBasedSimFile::write( int recno, const QByteArray& data )
{
    if ( d->files->available() )
        d->files->writeRecord( d->reqid, d->fileid, recno, data );
    else
        QTimer::singleShot( 0, this, SLOT(serviceUnavailable()) );
}

/*!
    \fn void QRecordBasedSimFile::error( QTelephony::SimFileError err )

    Signal that is emitted if an error \a err occurred while reading
    or writing this record-based file.

    \sa read(), write()
*/

/*!
    \fn void QRecordBasedSimFile::fileInfo( int numRecords, int recordSize )

    Signal that is emitted when the file information for this record-based
    file is returned.  The number of records in the file is \a numRecords,
    and the size of individual records is \a recordSize bytes.

    \sa requestFileInfo()
*/

/*!
    \fn void QRecordBasedSimFile::readDone( const QByteArray& data, int recno )

    Signal that is emitted when a read from record \a recno of this
    record-based file completes.  The \a data parameter contains the data
    that was read from the file.

    \sa read()
*/

/*!
    \fn void QRecordBasedSimFile::writeDone( int recno )

    Signal that is emitted when a write to record \a recno of
    this record-based file completes.

    \sa write()
*/

void QRecordBasedSimFile::serviceUnavailable()
{
    emit error( QTelephony::SimFileServiceUnavailable );
}

void QRecordBasedSimFile::serverError
    ( const QString& reqid, QTelephony::SimFileError err )
{
    if ( reqid == d->reqid )
        emit error( err );
}

void QRecordBasedSimFile::serverFileInfo
    ( const QString& reqid, int size, int recordSize )
{
    if ( reqid == d->reqid )
        emit fileInfo( ( recordSize ? ( size / recordSize ) : 0 ), recordSize );
}

void QRecordBasedSimFile::serverReadDone
    ( const QString& reqid, const QByteArray& data, int pos )
{
    if ( reqid == d->reqid )
        emit readDone( data, pos );
}

void QRecordBasedSimFile::serverWriteDone( const QString& reqid, int pos )
{
    if ( reqid == d->reqid )
        emit writeDone( pos );
}
