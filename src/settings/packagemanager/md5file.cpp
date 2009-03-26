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
#include "md5file.h"
#include <stdlib.h>
#include <QtDebug>

Md5File::Md5File( QObject *parent )
    : QFile( parent )
    , m_hash( QCryptographicHash::Md5 )
{
}

Md5File::Md5File( const QString &fileName, QObject *parent )
    : QFile( fileName, parent )
    , m_hash( QCryptographicHash::Md5 )
{
}

QString Md5File::md5Sum() const
{
    return m_md5Sum;
}

bool Md5File::open( OpenMode mode )
{
    if( QFile::open( mode ) )
    {
        m_hash.reset();

        return true;
    }
    else
        return false;
}

void Md5File::close()
{
    QFile::close();

    m_md5Sum = m_hash.result().toHex();
}

qint64 Md5File::writeData( const char *data, qint64 maxSize )
{
    m_hash.addData( data, maxSize );

    return QFile::writeData( data, maxSize );
}
