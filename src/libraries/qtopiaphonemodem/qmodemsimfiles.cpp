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

#include <qmodemsimfiles.h>
#include "qmodemsimfiles_p.h"
#include <qmodemservice.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemSimFiles
    \inpublicgroup QtCellModule

    \brief The QModemSimFiles class implements SIM file access for AT-based modems.
    \ingroup telephony::modem

    This class implements the QSimFiles telephony interface and uses the \c{AT+CRSM}
    or \c{AT+CSIM} commands from 3GPP TS 27.007 to access the files on the SIM.

    Client applications should use QBinarySimFile, QRecordBasedSimFile, or
    QSimFiles to access the files on a SIM rather than QModemSimFiles.

    \sa QSimFiles, QBinarySimFile, QRecordBasedSimFile
*/

class QModemSimFilesPrivate
{
public:
    QModemService *service;
};

/*!
    Create a new modem SIM file access object for \a service.
*/
QModemSimFiles::QModemSimFiles( QModemService *service )
    : QSimFiles( service->service(), service, QCommInterface::Server )
{
    d = new QModemSimFilesPrivate();
    d->service = service;
}

/*!
    Destroy this modem SIM file access object.
*/
QModemSimFiles::~QModemSimFiles()
{
    delete d;
}

/*!
    \reimp
*/
void QModemSimFiles::requestFileInfo
        ( const QString& reqid, const QString& fileid )
{
    // Send the "GET RESPONSE" request.
    QModemSimFileRequest *request;
    request = new QModemSimFileInfoRequest
        ( d->service, reqid, fileid, useCSIM(), this );
    connect( request, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SIGNAL(error(QString,QTelephony::SimFileError)) );
    connect( request, SIGNAL(fileInfo(QString,int,int,QTelephony::SimFileType)),
             this, SIGNAL(fileInfo(QString,int,int,QTelephony::SimFileType)) );
    request->chat( 192, 0, 0, 0 );
}

/*!
    \reimp
*/
void QModemSimFiles::readBinary
        ( const QString& reqid, const QString& fileid, int pos, int len )
{
    // Sanity-check the position and length parameters.
    if ( pos < 0 || pos > 65535 || len < 1 || len > 256 ||
         ( pos + len ) > 65536 ) {
        emit error( reqid, QTelephony::SimFileInvalidRead );
        return;
    }

    // Send the "READ BINARY" request.
    QModemSimFileRequest *request;
    request = new QModemSimFileReadRequest
                ( d->service, reqid, fileid, pos, useCSIM(), this );
    connect( request, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SIGNAL(error(QString,QTelephony::SimFileError)) );
    connect( request, SIGNAL(readDone(QString,QByteArray,int)),
             this, SIGNAL(readDone(QString,QByteArray,int)) );
    request->chat( 176, (pos >> 8) & 0xFF, pos & 0xFF, len & 0xFF );
}

/*!
    \reimp
*/
void QModemSimFiles::writeBinary
        ( const QString& reqid, const QString& fileid,
          int pos, const QByteArray& data )
{
    // Sanity-check the position and length parameters.
    int len = data.size();
    if ( pos < 0 || pos > 65535 || len < 1 || len > 256 ||
         ( pos + len ) > 65536 ) {
        emit error( reqid, QTelephony::SimFileInvalidWrite );
        return;
    }

    // Send the "UPDATE BINARY" request.
    QModemSimFileRequest *request;
    request = new QModemSimFileWriteRequest
                    ( d->service, reqid, fileid, pos, useCSIM(), this );
    connect( request, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SIGNAL(error(QString,QTelephony::SimFileError)) );
    connect( request, SIGNAL(writeDone(QString,int)),
             this, SIGNAL(writeDone(QString,int)) );
    request->chat( 214, (pos >> 8) & 0xFF, pos & 0xFF, len & 0xFF,
                   QAtUtils::toHex( data ) );
}

/*!
    \reimp
*/
void QModemSimFiles::readRecord
        ( const QString& reqid, const QString& fileid, int recno,
          int recordSize )
{
    // Sanity-check the record number parameter.
    if ( recno < 0 || recno >= 255 ||
         ( recordSize != -1 && ( recordSize < 1 || recordSize > 255 ) ) ) {
        emit error( reqid, QTelephony::SimFileInvalidRead );
        return;
    }

    // If we already know the record size, then send the "READ RECORD" now.
    if ( recordSize != -1 ) {
        QModemSimFileRequest *request;
        request = new QModemSimFileReadRequest
                ( d->service, reqid, fileid, recno, useCSIM(), this );
        connect( request, SIGNAL(error(QString,QTelephony::SimFileError)),
                 this, SIGNAL(error(QString,QTelephony::SimFileError)) );
        connect( request, SIGNAL(readDone(QString,QByteArray,int)),
                 this, SIGNAL(readDone(QString,QByteArray,int)) );
        request->chat( 178, recno + 1, 4, recordSize & 0xFF );
        return;
    }

    // Send a "GET RESPONSE" command to determine the record limit
    // and record size for this read.  The done() method will then
    // cause the "READ RECORD" request once the parameters are validated.
    QModemSimFileRequest *request1;
    QModemSimFileRequest *request2;
    request1 = new QModemSimFileInfoRequest
            ( d->service, reqid, fileid, useCSIM(), this );
    request2 = new QModemSimFileReadRequest
            ( d->service, reqid, fileid, recno, useCSIM(), this );
    connect( request1,
             SIGNAL(fileInfo(QString,int,int,QTelephony::SimFileType)),
             request2, SLOT(fileInfo(QString,int,int)) );
    connect( request2, SIGNAL(readDone(QString,QByteArray,int)),
             this, SIGNAL(readDone(QString,QByteArray,int)) );
    connect( request1, SIGNAL(error(QString,QTelephony::SimFileError)),
             request2, SLOT(infoError(QString,QTelephony::SimFileError)) );
    connect( request2, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SIGNAL(error(QString,QTelephony::SimFileError)) );
    request1->chat( 192, 0, 0, 0 );
}

/*!
    \reimp
*/
void QModemSimFiles::writeRecord
        ( const QString& reqid, const QString& fileid,
          int recno, const QByteArray& data )
{
    // Sanity-check the record and length parameters.
    int len = data.size();
    if ( recno < 0 || recno >= 255 || len < 1 || len > 255 ) {
        emit error( reqid, QTelephony::SimFileInvalidWrite );
        return;
    }

    // Send the "UPDATE RECORD" request.
    QModemSimFileRequest *request;
    request = new QModemSimFileWriteRequest
                ( d->service, reqid, fileid, recno, useCSIM(), this );
    connect( request, SIGNAL(error(QString,QTelephony::SimFileError)),
             this, SIGNAL(error(QString,QTelephony::SimFileError)) );
    connect( request, SIGNAL(writeDone(QString,int)),
             this, SIGNAL(writeDone(QString,int)) );
    request->chat( 220, recno + 1, 0x04, len & 0xFF, QAtUtils::toHex( data ) );
}

/*!
    Returns true if the modem SIM file access object should use \c{AT+CSIM}
    to access SIM files instead of \c{AT+CRSM}; false otherwise.  The default implementation
    returns false.  The \c{AT+CRSM} command is more reliable, so modem
    vendor plug-ins should use it wherever possible.
*/
bool QModemSimFiles::useCSIM() const
{
    return false;
}

QModemSimFileRequest::QModemSimFileRequest
        ( QModemService *service, const QString& reqid,
          const QString& fileid, bool useCSIM, QObject *parent )
    : QObject( parent )
{
    this->service = service;
    this->reqid = reqid;
    this->fileid = fileid;
    this->isWriting = false;
    this->retryRequested = false;
    this->retryFromRoot = false;
    this->selectFailed = false;
    this->errorReported = false;
    this->useCSIM = useCSIM;
}

QModemSimFileRequest::~QModemSimFileRequest()
{
}

void QModemSimFileRequest::chat
        ( int cmd, int p1, int p2, int p3, const QString& extra )
{
    command = formatRequest( cmd, fileid, p1, p2, p3, extra );
    sendSelects( false );
    service->chat( command, this, SLOT(chatResult(bool,QAtResult)) );
}

QString QModemSimFileRequest::formatRequest
    ( int cmd, const QString& fileid, int p1, int p2, int p3,
      const QString& extra )
{
    QString command;
    if ( useCSIM ) {
        QByteArray data;
        data.append( (char)0xA0 );
        data.append( (char)cmd );
        data.append( (char)p1 );
        data.append( (char)p2 );
        if ( cmd == 164 )
            p3 = 2;             // Always send 2 bytes with "SELECT".
        else if ( cmd == 192 )
            p3 = 15;            // Always return 15 bytes with "GET RESPONSE".
        data.append( (char)p3 );
        if ( !extra.isEmpty() )
            data += QAtUtils::fromHex( extra );
        else if ( cmd == 164 )
            data += QAtUtils::fromHex( fileid.right(4) );
        command = "AT+CSIM=" + QString::number( data.size() * 2 ) + "," +
                  QAtUtils::toHex( data );
    } else {
        if ( cmd == 164 ) {
            // Request for simple "SELECT" command - turn into "GET RESPONSE"
            // because AT+CRSM does not have a separate "SELECT" command.
            cmd = 192;
        }
        command = "AT+CRSM=" + QString::number( cmd );
        command += "," + QString::number( fileid.right(4).toInt( 0, 16 ) );
        if ( cmd != 192 ) {
            command += "," + QString::number( p1 );
            command += "," + QString::number( p2 );
            command += "," + QString::number( p3 );
            if ( !extra.isEmpty() )
                command += "," + extra;
        }
    }
    return command;
}

// Select directories leading up to the file we want.
void QModemSimFileRequest::sendSelects( bool fromRoot )
{
    bool sentSelect = false;
    if ( fileid.length() >= 12 ) {
        // Always start from the root for 3rd-level files, because
        // we'll probably need to retry anyway.
        fromRoot = true;
    }
    if ( fromRoot && !fileid.startsWith( "3F" ) ) {
        service->chat( formatRequest( 164, "3F00", 0, 0, 0 ),
                       this, SLOT(selectResult(bool,QAtResult)) );
        sentSelect = true;
    }
    int posn = 0;
    while ( ( posn + 8 ) <= fileid.length() ) {
        QString dirid = fileid.mid( posn, 4 );
        service->chat( formatRequest( 164, dirid, 0, 0, 0 ),
                       this, SLOT(selectResult(bool,QAtResult)) );
        sentSelect = true;
        posn += 4;
    }
    errorReported = false;
    retryFromRoot = false;
    if ( !fromRoot ) {
        // Determine if it is worth the effort retrying from root next time.
        // We need a top-level DF or EF at the front of the fileid.
        retryRequested = ( fileid.startsWith( "7F" ) ||
                           fileid.startsWith( "2F" ) );
        if ( retryRequested && !sentSelect )
            retryFromRoot = true;   // No selects sent, so force retry.
    } else {
        // We are doing the retry, so don't request another one.
        retryRequested = false;
    }
    if ( useCSIM ) {
        // Now select the actual file that we are interested in.
        // The AT+CRSM command has an implicit in-built select,
        // so this only needs to be done for AT+CSIM.
        service->chat( formatRequest( 164, fileid.right(4), 0, 0, 0 ),
                       this, SLOT(selectResult(bool,QAtResult)) );
    }
}

void QModemSimFileRequest::selectResult( bool ok, const QAtResult& result )
{
    // Check for errors in the response.
    if ( ok ) {
        QAtResultParser parser( result );
        uint sw1 = 0;
        QString line;
        uint posn = 0;
        if ( parser.next( "+CRSM:" ) ) {
            line = parser.line();
            sw1 = QAtUtils::parseNumber( line, posn );
        } else if ( parser.next( "+CSIM:" ) ) {
            line = parser.line();
            QAtUtils::parseNumber( line, posn );    // Skip length.
            if ( ((int)posn) < line.length() && line[posn] == ',' )
                ++posn;
            QByteArray data = QAtUtils::fromHex( line.mid( (int)posn ) );
            sw1 = ( data.size() >= 2 ? (data[data.size() - 2] & 0xFF) : 0 );
        }
        if ( sw1 != 0x90 && sw1 != 0x91 && sw1 != 0x9E && sw1 != 0x9F ) {
            ok = false;
        }
    }

    // Determine if we should retry the request from the root
    // directory, or bail out once control reaches chatResult().
    if ( !ok && !errorReported ) {
        if ( retryRequested && !retryFromRoot )
            retryFromRoot = true;
        else
            selectFailed = true;
        errorReported = true;
    }
}

void QModemSimFileRequest::chatResult( bool ok, const QAtResult& result )
{
    if ( !ok ) {
        // The command failed outright, so AT+CRSM is probably not
        // supported by the modem or the SIM is not inserted.  Report
        // the SIM as unavailable.
        emit error( reqid, QTelephony::SimFileSimUnavailable );
        deleteLater();
        return;
    }

    QAtResultParser parser( result );
    if ( useCSIM ) {
        if ( !parser.next( "+CSIM:" ) ) {
            // Shouldn't happen, but do something useful.
            emit error( reqid, QTelephony::SimFileSimUnavailable );
            deleteLater();
            return;
        }
    } else {
        if ( !parser.next( "+CRSM:" ) ) {
            // Shouldn't happen, but do something useful.
            emit error( reqid, QTelephony::SimFileSimUnavailable );
            deleteLater();
            return;
        }
    }

    // Extract the fields from the line.
    QString line = parser.line();
    uint posn = 0;
    uint sw1, sw2;
    QString data;
    if ( useCSIM ) {
        QAtUtils::parseNumber( line, posn );    // Skip length.
        if ( ((int)posn) < line.length() && line[posn] == ',' )
            data = line.mid( posn + 1 );
        else
            data = line.mid( posn );
        if ( data.contains( QChar('"') ) )
            data = data.remove( QChar('"') );
        if ( data.length() <= 4 ) {
            // Need at least sw1 and sw2 on the end of the command.
            emit error( reqid, QTelephony::SimFileSimUnavailable );
            deleteLater();
            return;
        }
        sw1 = data.mid( data.length() - 4, 2 ).toUInt( 0, 16 );
        sw2 = data.mid( data.length() - 2, 2 ).toUInt( 0, 16 );
        data = data.left( data.length() - 4 );
    } else {
        sw1 = QAtUtils::parseNumber( line, posn );
        sw2 = QAtUtils::parseNumber( line, posn );
        if ( ((int)posn) < line.length() && line[posn] == ',' )
            data = line.mid( posn + 1 );
        else
            data = line.mid( posn );
    }

    // Determine if the command succeeded or failed.
    if ( sw1 == 0x90 || sw1 == 0x91 || sw1 == 0x9E || sw1 == 0x9F ) {
        done( QAtUtils::fromHex( data ) );
    } else if ( selectFailed ) {
        // One of the directory select commands failed, so file was not found.
        emit error( reqid, QTelephony::SimFileNotFound );
    } else if ( retryFromRoot ) {
        // We need to retry the command, starting at the root directory.
        sendSelects( true );
        service->chat( command, this, SLOT(chatResult(bool,QAtResult)) );
        return;
    } else if ( sw1 == 0x94 && ( sw2 == 0x02 || sw2 == 0x08 ) ) {
        if ( isWriting )
            emit error( reqid, QTelephony::SimFileInvalidWrite );
        else
            emit error( reqid, QTelephony::SimFileInvalidRead );
    } else {
        emit error( reqid, QTelephony::SimFileNotFound );
    }

    // This object is no longer required.
    deleteLater();
}

QModemSimFileInfoRequest::QModemSimFileInfoRequest
        ( QModemService *service, const QString& reqid,
          const QString& fileid, bool useCSIM, QObject *parent )
    : QModemSimFileRequest( service, reqid, fileid, useCSIM, parent )
{
}

QModemSimFileInfoRequest::~QModemSimFileInfoRequest()
{
}

void QModemSimFileInfoRequest::done( const QByteArray& data )
{
    // The file size is stored in the third and fourth bytes of the data.
    int size;
    if ( data.size() >= 4 ) {
        size = ((((int)(data[2])) & 0xFF) << 8) |
                (((int)(data[3])) & 0xFF);
    } else {
        size = 0;
    }

    // Get the file type and structure.
    int type, structure;
    if ( data.size() >= 7 ) {
        type = (((int)(data[6])) & 0xFF);
    } else {
        type = -1;
    }
    if ( type == 0x04 && data.size() >= 14 ) {
        structure = (((int)(data[13])) & 0xFF);
    } else {
        structure = -1;
    }

    // Convert the raw file type information into something more useful.
    QTelephony::SimFileType ftype;
    if ( type == 0x01 ) {
        ftype = QTelephony::SimFileRootDirectory;
    } else if ( type == 0x02 ) {
        ftype = QTelephony::SimFileDirectory;
    } else if ( type == 0x04 ) {
        if ( structure == 0x00 )
            ftype = QTelephony::SimFileTransparent;
        else if ( structure == 0x01 )
            ftype = QTelephony::SimFileLinearFixed;
        else if ( structure == 0x03 )
            ftype = QTelephony::SimFileCyclic;
        else
            ftype = QTelephony::SimFileUnknown;
    } else {
        ftype = QTelephony::SimFileUnknown;
    }

    // If this is a record-based file, then the record size is in
    // the fifteenth byte of the data (offset 14 counting from zero).
    int recordSize = 1;
    if ( ftype == QTelephony::SimFileLinearFixed ||
         ftype == QTelephony::SimFileCyclic ) {
        if ( data.size() >= 15 )
            recordSize = (((int)(data[14])) & 0xFF);
    }

    // Report the file information to the requestor.
    emit fileInfo( reqid, size, recordSize, ftype );
}

QModemSimFileReadRequest::QModemSimFileReadRequest
        ( QModemService *service, const QString& reqid,
          const QString& fileid, int pos, bool useCSIM, QObject *parent )
    : QModemSimFileRequest( service, reqid, fileid, useCSIM, parent )
{
    this->pos = pos;
}

QModemSimFileReadRequest::~QModemSimFileReadRequest()
{
}

void QModemSimFileReadRequest::fileInfo
        ( const QString&, int size, int recordSize )
{
    // This is called when we are doing a "READ RECORD" to notify
    // us of the file's record size.

    // Check the validity of the record number.
    if ( pos >= ( size / recordSize ) ) {
        emit error( reqid, QTelephony::SimFileInvalidRead );
        deleteLater();
        return;
    }

    // Now send the "READ RECORD" request.
    chat( 178, pos + 1, 0x04, recordSize & 0xFF );
}

void QModemSimFileReadRequest::infoError
        ( const QString& reqid, QTelephony::SimFileError err )
{
    // This is called to tell us that the "GET RESPONSE" command
    // failed.  Pass the error on and then delete ourselves.
    emit error( reqid, err );
    deleteLater();
}

void QModemSimFileReadRequest::done( const QByteArray& data )
{
    emit readDone( reqid, data, pos );
}

QModemSimFileWriteRequest::QModemSimFileWriteRequest
        ( QModemService *service, const QString& reqid, const QString& fileid,
          int pos, bool useCSIM, QObject *parent )
    : QModemSimFileRequest( service, reqid, fileid, useCSIM, parent )
{
    this->pos = pos;
    this->isWriting = true;
}

QModemSimFileWriteRequest::~QModemSimFileWriteRequest()
{
}

void QModemSimFileWriteRequest::done( const QByteArray& )
{
    emit writeDone( reqid, pos );
}
