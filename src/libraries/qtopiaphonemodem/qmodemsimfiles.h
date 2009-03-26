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

#ifndef QMODEMSIMFILES_H
#define QMODEMSIMFILES_H

#include <qsimfiles.h>

class QModemService;
class QModemSimFilesPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemSimFiles : public QSimFiles
{
    Q_OBJECT
public:
    explicit QModemSimFiles( QModemService *service );
    ~QModemSimFiles();

public slots:
    void requestFileInfo( const QString& reqid, const QString& fileid );
    void readBinary( const QString& reqid, const QString& fileid,
                     int pos, int len );
    void writeBinary( const QString& reqid, const QString& fileid,
                      int pos, const QByteArray& data );
    void readRecord( const QString& reqid, const QString& fileid,
                     int recno, int recordSize );
    void writeRecord( const QString& reqid, const QString& fileid,
                      int recno, const QByteArray& data );

protected:
    virtual bool useCSIM() const;

private:
    QModemSimFilesPrivate *d;
};

#endif
