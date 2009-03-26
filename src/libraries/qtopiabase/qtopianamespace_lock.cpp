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

#include <qtopianamespace.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mntent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*!
\enum Qtopia::Lockflags
 This enum controls what type of locking is performed on a file.

 Current defined values are:

 \value LockShare Allow lock to be shared. Reserved for future use
 \value LockWrite Create a write lock.
 \value LockBlock Block the process when lock is encountered. Under WIN32
                  this blocking is limited to ten(10) failed attempts to
                  access locked file. Reserved for future use.
*/

/*!
  \fn bool Qtopia::lockFile(QFile &f, int flags)
  Lock a file. Any locks created should be released before
  the program exits. Returns true if successful.

  \a f must be an open file.
  \a flags any combination of LockShare, LockWrite, LockBlock.
  \sa Qtopia::unlockFile(), Qtopia::isFileLocked()
*/

/*!
  \fn bool Qtopia::unlockFile(QFile &f)
  Unlock a file. Returns true if successful.

  \a f must be an open file previously locked.

  \sa Qtopia::lockFile(), Qtopia::isFileLocked()
 */

/*!
  \fn bool Qtopia::isFileLocked(QFile &f, int flags)
  Tests locking a file. Any locks created should be released before
  the program exits. Returns true if locking would be successful.

  \a f must be an open file.

  \a flags any combination of LockShare, LockWrite.
  \sa Qtopia::unlockFile(), Qtopia::lockFile()
 */

bool Qtopia::lockFile(QFile &f, int flags)
{
    struct flock fileLock;

    if (!f.isOpen())
        return false;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    int lockCommand;

    fileLock.l_len = f.size();

    // read or write lock is defaulted to being the appropriate lock for the
    // open mode of the file: read lock if opened for read, write if for write
    // these defaulting semantics are still available if LockBlock is specified
    if (flags == -1 || flags == LockBlock){
        fileLock.l_type =  F_RDLCK;
        if (f.openMode() == QIODevice::ReadOnly)
            fileLock.l_type = F_RDLCK;
        else
            fileLock.l_type = F_WRLCK;
        lockCommand = ( flags == LockBlock ) ? F_SETLKW : F_SETLK;
    }else{
        if (flags & LockWrite)
            fileLock.l_type = F_WRLCK;
        else
            fileLock.l_type = F_RDLCK;
        if (flags & LockBlock)
            lockCommand = F_SETLKW; // block process if possible
        else
            lockCommand = F_SETLK;
    }

    if (::fcntl(f.handle(), lockCommand, &fileLock) != -1)
        return true;
    else
        return false;
}

bool Qtopia::unlockFile(QFile &f)
{
    struct flock fileLock;

    if (!f.isOpen())
        return false;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;

    fileLock.l_len = f.size();

    fileLock.l_type = F_UNLCK;

    if (::fcntl(f.handle(), F_SETLK, &fileLock) != -1)
        return true;
    else
        return false;

}

bool Qtopia::isFileLocked(QFile &f, int flags)
{
    struct flock fileLock;

    if (!f.isOpen())
        return false;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;

    fileLock.l_len = f.size();

    if (flags == -1){
        fileLock.l_type =  F_RDLCK;
        if (f.openMode() == QIODevice::ReadOnly)
            fileLock.l_type = F_RDLCK;
        else
            fileLock.l_type = F_WRLCK;
    }else{
        if (flags & LockWrite)
            fileLock.l_type = F_WRLCK;
        else
            fileLock.l_type = F_RDLCK;
    }

    fileLock.l_pid = 0;

    if (::fcntl(f.handle(), F_GETLK, &fileLock) != -1)
        return false;

    return fileLock.l_type == F_UNLCK;
}

