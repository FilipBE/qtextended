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

#ifndef SEMAPHORE_P_H
#define SEMAPHORE_P_H

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

struct sembuf;

// We need slightly different semantics to those of QSystemMutex - all users of
// the qtopiamail library are peers, so no single caller is the owner.  We will
// allow the first library user to create the semaphore, and any subsequent users
// will attach to the same semaphore set.  No-one will close the semaphore set,
// we will rely on process undo to maintain sensible semaphore values as
// clients come and go...

class Semaphore
{
    int m_id;
    bool m_remove;
    int m_semId;
    int m_initialValue;

    bool operation(struct sembuf *op, int milliSec);

public:
    Semaphore(int id, bool remove, int initial);
    ~Semaphore();

    bool decrement(int milliSec = -1);
    bool increment(int milliSec = -1);

    bool waitForZero(int milliSec = -1);
};

class ProcessMutex : public Semaphore
{
public:
    ProcessMutex(int id) : Semaphore(id, false, 1) {}

    bool lock(int milliSec) { return decrement(milliSec); }
    void unlock() { increment(); }
};

class ProcessReadLock : public Semaphore
{
public:
    ProcessReadLock(int id) : Semaphore(id, false, 0) {}

    void lock() { increment(); }
    void unlock() { decrement(); }

    bool wait(int milliSec) { return waitForZero(milliSec); }
};

#endif
