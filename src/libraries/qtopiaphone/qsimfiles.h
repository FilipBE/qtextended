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

#ifndef QSIMFILES_H
#define QSIMFILES_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QSimFiles : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSimFiles( const QString& service = QString(), QObject *parent = 0,
                        QCommInterface::Mode mode = Client );
    ~QSimFiles();

public slots:
    virtual void requestFileInfo( const QString& reqid, const QString& fileid );
    virtual void readBinary( const QString& reqid, const QString& fileid,
                             int pos, int len );
    virtual void writeBinary( const QString& reqid, const QString& fileid,
                              int pos, const QByteArray& data );
    virtual void readRecord( const QString& reqid, const QString& fileid,
                             int recno, int recordSize = -1 );
    virtual void writeRecord( const QString& reqid, const QString& fileid,
                              int recno, const QByteArray& data );

signals:
    void error( const QString& reqid, QTelephony::SimFileError err );
    void fileInfo( const QString& reqid, int size, int recordSize,
                   QTelephony::SimFileType type );
    void readDone( const QString& reqid, const QByteArray& data, int pos );
    void writeDone( const QString& reqid, int pos );
};

#endif
