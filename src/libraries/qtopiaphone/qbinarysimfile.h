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

#ifndef QBINARYSIMFILE_H
#define QBINARYSIMFILE_H

#include <qtelephonynamespace.h>
#include <qtopiaglobal.h>

class QBinarySimFilePrivate;

class QTOPIAPHONE_EXPORT QBinarySimFile : public QObject
{
    Q_OBJECT
public:
    explicit QBinarySimFile( const QString& fileid, const QString& service = QString(),
                             QObject *parent = 0 );
    ~QBinarySimFile();

    void requestFileSize();
    void read( int pos, int len );
    void write( int pos, const char *data, int len );
    void write( int pos, const QByteArray& data );

signals:
    void error( QTelephony::SimFileError err );
    void fileSize( int size );
    void readDone( const QByteArray& data, int pos );
    void writeDone( int pos );

private slots:
    void serviceUnavailable();
    void serverError( const QString& reqid, QTelephony::SimFileError err );
    void serverFileInfo( const QString& reqid, int size );
    void serverReadDone( const QString& reqid, const QByteArray& data, int pos );
    void serverWriteDone( const QString& reqid, int pos );

private:
    QBinarySimFilePrivate *d;
};

#endif
