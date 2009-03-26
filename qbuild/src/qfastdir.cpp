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

#include "qfastdir.h"
#include "qoutput.h"
#include <QMutex>
#include "qbuild.h"

struct Cache {
    typedef QHash<QString, struct stat> CacheMap;
    CacheMap cache;
    QMutex lock;
};
static QThreadStorage<Cache *> statCache;
static inline Cache *cache()
{
    Cache *rv = statCache.localData();
    if (!rv) {
        rv = new Cache;
        statCache.setLocalData(rv);
    }
    return rv;
}
//Q_GLOBAL_STATIC(Cache, cache);

//static Cache cache;
/*!
  \class QFastDir
  \brief A fast replacement for QFile/QDir
*/

/*!
  Construct a QFastDir for \a dir.
*/
QFastDir::QFastDir(const QString &dir)
{
    init(dir);
}

/*!
  Returns true if \a path is relative.
*/
bool QFastDir::isRelativePath(const QString &path)
{
    return path.isEmpty() || path.at(0) != QChar('/');
}

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

/*!
  Returns true if \a path exists.
*/
bool QFastDir::exists(const QString &path)
{
    struct stat buf = filestat(path);
    return S_ISREG(buf.st_mode) || S_ISDIR(buf.st_mode);
}

/*!
  Returns true if \a path is a file.
*/
bool QFastDir::isFile(const QString &path)
{
    struct stat buf = filestat(path);
    return S_ISREG(buf.st_mode);
}

/*!
  Returns true if \a path is a directory.
*/
bool QFastDir::isDir(const QString &path)
{
    struct stat buf = filestat(path);
    return S_ISDIR(buf.st_mode);
}

// qdoc cannot handle "struct stat QFastDir::filestat"
typedef struct stat stat_struct;
/*!
  \internal
*/
stat_struct QFastDir::filestat(const QString &path)
{
    struct stat buf;

    Cache *c = cache();
   // LOCK(QFastDir);
    //c->lock.lock();

    Cache::CacheMap::ConstIterator iter = c->cache.find(path);
    if (iter == c->cache.end()) {
        ::memset(&buf, 0, sizeof(buf));
        buf.st_mtime = -1;
        ::stat(path.toLocal8Bit().constData(), &buf);
        c->cache.insert(path, buf);
    } else {
        buf = *iter;
    }
//    c->lock.unlock();

    return buf;
}

/*!
  \internal
*/
void QFastDir::init(const QString &path)
{
    DIR *dir = ::opendir(path.toLocal8Bit().constData());
    if (!dir) return;

    while (struct dirent *dirent = ::readdir(dir)) {
        if (dirent->d_name[0] == '.' &&
           (dirent->d_name[1] == '\0' ||
            (dirent->d_name[1] == '.' && dirent->d_name[2] == '\0')))
            continue;

        QString dir = path+"/"+dirent->d_name;
        if (QFastDir::isDir(dir))
            m_dirs.append(dirent->d_name);
        else if (QFastDir::isFile(dir))
            m_files.append(dirent->d_name);
    }

    ::closedir(dir);
}

/*!
  Returns the files in the selected directory.
*/
QStringList QFastDir::files() const
{
    return m_files;
}

/*!
  Returns the directories in the selected directory.
*/
QStringList QFastDir::dirs() const
{
    return m_dirs;
}

/*!
  Returns the last modified time of \a path.
*/
time_t QFastDir::lastModified(const QString &path)
{
    struct stat buf = filestat(path);
    return buf.st_mtime;
}

/*!
  Returns the directory component of \a name.
  This is equivalent to the \c dirname utility.
*/
QString QFastDir::dir(const QString &name)
{
    int idx = name.lastIndexOf('/');
    QString dir;
    if (idx != -1)
        dir = name.left(idx);
    return dir;
}

/*!
  Returns the file component of \a name.
  This is equivalent to the \c basename utility (but it does not do anything with file extensions).
*/
QString QFastDir::fileName(const QString &name)
{
    int idx = name.lastIndexOf('/');
    QString filename;
    if (idx != -1)
        filename = name.mid(idx + 1);
    else
        filename = name;
    return filename;
}

/*!
  \internal
  \class QFastDirCache
  \brief A lock-free, single threaded version of QFastDir

  This is faster when only used from a single thread.
 */
QFastDirCache::QFastDirCache()
{
}

void QFastDirCache::refresh(const QString &file)
{
    cache.remove(file);
}

bool QFastDirCache::exists(const QString &path)
{
    struct stat buf = filestat(path);
    return S_ISREG(buf.st_mode) || S_ISDIR(buf.st_mode);
}

bool QFastDirCache::isDir(const QString &path)
{
    struct stat buf = filestat(path);
    return S_ISDIR(buf.st_mode);
}

bool QFastDirCache::isFile(const QString &path)
{
    struct stat buf = filestat(path);
    return S_ISREG(buf.st_mode);
}

time_t QFastDirCache::lastModified(const QString &path)
{
    struct stat buf = filestat(path);
    return buf.st_mtime;
}

struct stat QFastDirCache::filestat(const QString &path)
{
    CacheMap::ConstIterator iter = cache.find(path);
    struct stat buf;

    if (iter == cache.end()) {
        ::memset(&buf, 0, sizeof(buf));
        buf.st_mtime = -1;
        ::stat(path.toLocal8Bit().constData(), &buf);
        cache.insert(path, buf);
    } else {
        buf = *iter;
    }

    return buf;
}

