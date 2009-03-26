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
#include "longstream_p.h"
#include <QApplication>
#include <QIODevice>
#include <QTextStream>
#include <QTemporaryFile>
#include <sys/vfs.h>

/*  Helper class to reduce memory usage while downloading large mails */
LongStream::LongStream()
{
    lastLine = QString::null;
    QString tmpName( LongStream::tempDir() + QLatin1String( "/qtopiamail" ) );
    tmpFile = new QTemporaryFile( tmpName + QLatin1String( ".XXXXXX" ));
    tmpFile->open(); // todo error checking
    tmpFile->setPermissions(QFile::ReadOwner | QFile::WriteOwner);
    ts = new QTextStream( tmpFile );
    ts->setCodec( "UTF-8" ); // Mail should be 7bit ascii
    len = 0;
}

LongStream::~LongStream()
{
    tmpFile->close();
    delete ts;
    delete tmpFile;
}

void LongStream::reset()
{
    lastLine = QString::null;
    delete ts;
    tmpFile->resize( 0 );
    tmpFile->close();
    tmpFile->open();
    ts = new QTextStream( tmpFile );
    ts->setCodec( "UTF-8" ); // Mail should be 7bit ascii
    len = 0;
    c = QChar::Null;
    resetStatus();
}

QString LongStream::detach()
{
    QString detachedName = fileName();
    lastLine = QString::null;
    delete ts;
    tmpFile->setAutoRemove(false);
    tmpFile->close();
    delete tmpFile;
    QString tmpName( LongStream::tempDir() + QLatin1String( "/qtopiamail" ) );
    tmpFile = new QTemporaryFile( tmpName + QLatin1String( ".XXXXXX" ));
    tmpFile->open();
    tmpFile->setPermissions(QFile::ReadOwner | QFile::WriteOwner);
    ts = new QTextStream( tmpFile );
    ts->setCodec( "UTF-8" ); // Mail should be 7bit ascii
    len = 0;
    c = QChar::Null;
    resetStatus();
    return detachedName;
}

void LongStream::append(QString str)
{
    *ts << str << flush; //todo error checking - out of disk
    len += str.length();
    updateStatus();
}

int LongStream::length()
{
    return len;
}

QString LongStream::fileName()
{
    return tmpFile->fileName();
}

// QTextStream is currently not memory-efficient enough for our purposes
//#define USE_QTEXTSTREAM_READLINE

QString LongStream::readLine()
{
#ifdef USE_QTEXTSTREAM_READLINE
    return ts->readLine();
#else
    QString s;

    // Don't return any of CR, LF, CRLF
    if (!c.isNull() && (c != '\r') && (c != '\n'))
        s += c;
    while (!ts->atEnd() && (c != '\r') && (c != '\n')) {
        *ts >> c;
        if ((c == '\r') || (c == '\n'))
            break;
        s += c;
    }
    if ((!ts->atEnd()) && (c == '\r')) {
        *ts >> c;
        if (c == '\n')
            *ts >> c;
    } else if ((!ts->atEnd()) && (c == '\n')) {
        *ts >> c;
        /* LFCR is not a valid newline sequence...
        if (c == '\r')
            *ts >> c;
        */
    }
    if (s.isNull() && !ts->atEnd())
        return "";
    return s;
#endif
}


QString LongStream::first()
{
    ts->seek( 0 );

    lastLine = readLine();
    if (!lastLine.isEmpty())
        lastLine += "\015\012";

    return lastLine;
}

QString LongStream::next()
{
    lastLine = readLine();
    if (!lastLine.isNull())
        lastLine += "\015\012";

    return lastLine;
}

QString LongStream::current()
{
    return lastLine;
}

LongStream::Status LongStream::status()
{
    return mStatus;
}

void LongStream::resetStatus()
{
    mStatus = Ok;
}

void LongStream::updateStatus()
{
    if (!freeSpace())
        setStatus( LongStream::OutOfSpace );
}

void LongStream::setStatus( Status status )
{
    mStatus = status;
}

bool LongStream::freeSpace( const QString &path, int min)
{
    unsigned long long boundary = minFree;
    if (min >= 0)
        boundary = min;
    struct statfs stats;
    QString partitionPath = tempDir() + "/.";
    if (!path.isEmpty())
        partitionPath = path;

    statfs( QString( partitionPath ).toLocal8Bit(), &stats);
    unsigned long long bavail = ((unsigned long long)stats.f_bavail);
    unsigned long long bsize = ((unsigned long long)stats.f_bsize);
    return (bavail * bsize) > boundary;
}

QString LongStream::errorMessage( const QString &prefix )
{
    QString str = QApplication::tr( "Storage for messages is full. Some new "
                                    "messages could not be retrieved." );
    if (!prefix.isEmpty())
        return prefix + str;
    return str;
}

static QString tempDirPath()
{
    QString path = Qtopia::applicationFileName("qtopiamail", "temp");
    if (path.isEmpty())
        path = Qtopia::tempDir();
    QDir dir;
    if (!dir.exists( path ))
        dir.mkpath( path );
    return path;
}

QString LongStream::tempDir()
{
    static QString path(tempDirPath());
    return path;
}

void LongStream::cleanupTempFiles()
{
    QDir dir( LongStream::tempDir(), "qtopiamail.*" );
    QStringList list = dir.entryList();
    for (int i = 0; i < list.size(); ++i) {
        QFile file( LongStream::tempDir() + list.at(i) );
        if (file.exists())
            file.remove();
    }
}
