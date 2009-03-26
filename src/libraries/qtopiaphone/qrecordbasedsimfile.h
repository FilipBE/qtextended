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

#ifndef QRECORDBASEDSIMFILE_H
#define QRECORDBASEDSIMFILE_H

#include <qtelephonynamespace.h>
#include <qtopiaglobal.h>

class QRecordBasedSimFilePrivate;

class QTOPIAPHONE_EXPORT QRecordBasedSimFile : public QObject
{
    Q_OBJECT
public:
    explicit QRecordBasedSimFile( const QString& fileid,
                                  const QString& service = QString(),
                                  QObject *parent = 0 );
    ~QRecordBasedSimFile();

    void requestFileInfo();
    void read( int recno, int recordSize = -1 );
    void write( int recno, const char *data, int len );
    void write( int recno, const QByteArray& data );

signals:
    void error( QTelephony::SimFileError err );
    void fileInfo( int numRecords, int recordSize );
    void readDone( const QByteArray& data, int recno );
    void writeDone( int recno );

private slots:
    void serviceUnavailable();
    void serverError( const QString& reqid, QTelephony::SimFileError err );
    void serverFileInfo( const QString& reqid, int size, int recordSize );
    void serverReadDone( const QString& reqid, const QByteArray& data, int pos );
    void serverWriteDone( const QString& reqid, int pos );

private:
    QRecordBasedSimFilePrivate *d;
};

#endif
