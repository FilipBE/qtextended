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

#ifndef QMODEMSIMFILES_P_H
#define QMODEMSIMFILES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qmodemsimfiles.h>
#include <qatresult.h>

class QModemSimFileRequest : public QObject
{
    Q_OBJECT
public:
    QModemSimFileRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, bool useCSIM, QObject *parent );
    ~QModemSimFileRequest();

    void chat( int cmd, int p1, int p2, int p3,
               const QString& extra = QString() );

private slots:
    void selectResult( bool ok, const QAtResult& result );
    void chatResult( bool ok, const QAtResult& result );

signals:
    void error( const QString& reqid, QTelephony::SimFileError err );

protected:
    QModemService *service;
    QString reqid;
    QString fileid;
    bool isWriting;
    bool retryFromRoot;
    bool retryRequested;
    bool selectFailed;
    bool errorReported;
    bool useCSIM;
    QString command;

    virtual void done( const QByteArray& data ) = 0;

private:
    void sendSelects( bool fromRoot );
    QString formatRequest( int cmd, const QString& fileid,
                           int p1, int p2, int p3,
                           const QString& extra = QString() );
};

class QModemSimFileInfoRequest : public QModemSimFileRequest
{
    Q_OBJECT
public:
    QModemSimFileInfoRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, bool useCSIM, QObject *parent );
    ~QModemSimFileInfoRequest();

signals:
    void fileInfo( const QString& reqid, int size, int recordSize,
                   QTelephony::SimFileType type );

protected:
    void done( const QByteArray& data );
};

class QModemSimFileReadRequest : public QModemSimFileRequest
{
    Q_OBJECT
public:
    QModemSimFileReadRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, int pos, bool useCSIM, QObject *parent );
    ~QModemSimFileReadRequest();

signals:
    void readDone( const QString& reqid, const QByteArray& data, int pos );

public slots:
    void fileInfo( const QString& reqid, int size, int recordSize );
    void infoError( const QString& reqid, QTelephony::SimFileError err );

protected:
    void done( const QByteArray& data );

private:
    int pos;
};

class QModemSimFileWriteRequest : public QModemSimFileRequest
{
    Q_OBJECT
public:
    QModemSimFileWriteRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, int pos, bool useCSIM, QObject *parent );
    ~QModemSimFileWriteRequest();

signals:
    void writeDone( const QString& reqid, int pos );

protected:
    void done( const QByteArray& data );

private:
    int pos;
};

#endif
