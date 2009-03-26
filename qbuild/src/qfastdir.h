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

#ifndef QFASTDIR_H
#define QFASTDIR_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


class QFastDir
{
public:
    QFastDir(const QString &);

    QStringList dirs() const;
    QStringList files() const;

    static bool isRelativePath(const QString &);
    static bool exists(const QString &);
    static bool isDir(const QString &);
    static bool isFile(const QString &);
    static time_t lastModified(const QString &);
    static QString dir(const QString &);
    static QString fileName(const QString &);

private:
    friend class QFastDirCache;
    static struct stat filestat(const QString &);

    void init(const QString &);

    QStringList m_dirs;
    QStringList m_files;
};

class QFastDirCache
{
public:
    QFastDirCache();

    void refresh(const QString &);
    bool exists(const QString &);
    bool isDir(const QString &);
    bool isFile(const QString &);
    time_t lastModified(const QString &);

private:
    struct stat filestat(const QString &);
    typedef QHash<QString, struct stat> CacheMap;
    CacheMap cache;
};

#endif
