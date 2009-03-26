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

#include <qsimfiles.h>

/*!
    \class QSimFiles
    \inpublicgroup QtTelephonyModule

    \brief The QSimFiles class provides an interface for reading and writing the files on a SIM.
    \ingroup telephony

    Client applications should use QBinarySimFile and QRecordBasedSimFile
    to access SIM files as they provide a cleaner interface.  The
    QSimFiles class is mainly intended for use by service implementations,
    which inherit from QSimFiles and override the methods described below.
    QModemSimFiles is the standard implementation of the service for
    AT-based modems.

    \sa QBinarySimFile, QRecordBasedSimFile, QModemSimFiles
*/

/*!
    Construct a new SIM file access object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SIM file access.  If there is more
    than one service that supports SIM file access, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QSimFiles objects for each.

    Normally this constructor would not be used by client applications.
    Client applications should use QBinarySimFile and QRecordBasedSimFile.
    Server applications should inherit from the QSimFiles class and
    call this constructor with \a mode set to QCommInterface::Server.

    \sa QCommServiceManager::supports()
*/
QSimFiles::QSimFiles( const QString& service, QObject *parent,
                      QCommInterface::Mode mode )
    : QCommInterface( "QSimFiles", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this SIM file access object.
*/
QSimFiles::~QSimFiles()
{
}

/*!
    Request the current file information for \a fileid.  The \a reqid
    parameter indicates a caller-assigned request identifier.

    Once the request completes, the fileInfo() signal will be emitted
    when the same \a reqid.  If the request fails, the error() signal
    will be emitted.

    Client applications should use QBinarySimFile::requestFileSize()
    or QRecordBasedSimFile::requestFileInfo() instead.

    \sa fileInfo()
*/
void QSimFiles::requestFileInfo( const QString& reqid, const QString& fileid )
{
    invoke( SLOT(requestFileInfo(QString,QString)), reqid, fileid );
}

/*!
    Read \a len bytes from position \a pos of the binary file \a fileid.
    The \a reqid parameter indicates a caller-assigned request identifier.

    Once the read completes, the readDone() signal will be emitted
    with the same \a reqid.  If the request fails, the error() signal
    will be emitted.

    Client applications should use QBinarySimFile::read() instead.

    \sa readDone(), writeBinary()
*/
void QSimFiles::readBinary( const QString& reqid, const QString& fileid,
                            int pos, int len )
{
    invoke( SLOT(readBinary(QString,QString,int,int)) )
        << reqid << fileid << pos << len;
}

/*!
    Write \a data at position \a pos of the binary file \a fileid.  The
    \a reqid parameter indicates a caller-assigned request identifier.

    Once the write completes, the writeDone() signal will be emitted
    with the same \a reqid.  If the request fails, the error() signal
    will be emitted.

    Client applications should use QBinarySimFile::write() instead.

    \sa writeDone(), readBinary()
*/
void QSimFiles::writeBinary( const QString& reqid, const QString& fileid,
                             int pos, const QByteArray& data )
{
    invoke( SLOT(writeBinary(QString,QString,int,QByteArray)) )
        << reqid << fileid << pos << data;
}

/*!
    Read record \a recno the record-based file \a fileid.  The \a reqid
    parameter indicates a caller-assigned request identifier.

    Once the read completes, the readDone() signal will be emitted
    with the same \a reqid.  If the request fails, the error() signal
    will be emitted.

    If \a recordSize is not -1, it indicates that the record size is
    already known.  Otherwise the service will determine the record size
    for itself.

    Client applications should use QRecordBasedSimFile::read() instead.

    \sa readDone(), writeRecord()
*/
void QSimFiles::readRecord
    ( const QString& reqid, const QString& fileid, int recno,
      int recordSize )
{
    invoke( SLOT(readRecord(QString,QString,int,int)) )
        << reqid << fileid << recno << recordSize;
}

/*!
    Write \a data at position \a recno of the record-based file \a fileid.
    The \a reqid parameter indicates a caller-assigned request identifier.

    Once the write completes, the writeDone() signal will be emitted
    with the same \a reqid.  If the request fails, the error() signal
    will be emitted.

    Client applications should use QRecordBasedSimFile::write() instead.

    \sa writeDone(), readRecord()
*/
void QSimFiles::writeRecord( const QString& reqid, const QString& fileid,
                             int recno, const QByteArray& data )
{
    invoke( SLOT(writeRecord(QString,QString,int,QByteArray)) )
        << reqid << fileid << recno << data;
}

/*!
    \fn void QSimFiles::error( const QString& reqid, QTelephony::SimFileError err )

    Signal that is emitted when the request \a reqid fails with an error
    code \a err.

    \sa readBinary(), writeBinary(), readRecord(), writeRecord()
*/

/*!
    \fn void QSimFiles::fileInfo( const QString& reqid, int size, int recordSize, QTelephony::SimFileType type )

    Signal that is emitted when the file information request \a reqid was
    successful.  The size of the file in bytes is \a size.  If the
    file is record-based, then \a recordSize will be the size of
    the individual records.  If the file is binary-based, then
    \a recordSize will be 1.  The \a type parameter specifies the actual
    file type.

    \sa requestFileInfo()
*/

/*!
    \fn void QSimFiles::readDone( const QString& reqid, const QByteArray& data, int pos )

    Signal that is emitted when the read request \a reqid was successful
    in reading \a data from \a pos in the file.

    \sa readBinary(), readRecord()
*/

/*!
    \fn void QSimFiles::writeDone( const QString& reqid, int pos )

    Signal that is emitted when the write request \a reqid successfully
    wrote to position \a pos of the file.

    \sa writeBinary(), writeRecord()
*/
