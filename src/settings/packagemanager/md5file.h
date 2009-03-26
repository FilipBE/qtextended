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
#ifndef MD5FILE_H
#define MD5FILE_H

#include <QFile>
#include <QCryptographicHash>

class Md5File : public QFile
{
public:
    Md5File( QObject *parent = 0 );
    Md5File( const QString &fileName, QObject *parent = 0 );

    QString md5Sum() const;

    virtual bool open( OpenMode mode );
    virtual void close();

protected:
    virtual qint64 writeData( const char *data, qint64 maxSize );

private:
    QCryptographicHash m_hash;
    QString m_md5Sum;
};

#endif
