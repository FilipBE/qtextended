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

#include "qsystemlock.h"
#include <QDebug>
#include <qtopialog.h>
#include <QMutex>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Must define this union according to the semctl manpage
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux specific) */
};

extern int semtimedop(int __semid, struct sembuf *__sops, size_t __nsops,
                      __const struct timespec *__timeout) __THROW;


class QSystemReadWriteLockPrivate
{
public:
    QSystemReadWriteLockPrivate(unsigned int _id, bool _own)
        : id(_id), semId(-1), own(_own), readLocks(0), writeLocks(0) {}
    unsigned int id;
    int semId;
    bool own;

    QMutex lock;
    unsigned int readLocks;
    unsigned int writeLocks;

    static const unsigned int ReadSem = 0;
    static const unsigned int WriteSem = 1;

};

/*!
   \class QSystemReadWriteLock
    \inpublicgroup QtBaseModule

   \brief The QSystemReadWriteLock class provides read-write locking between
   processes.

   A read-write lock is a synchronization tool for protecting resources that can
   be accessed for reading and writing. This type of lock is useful if you want
   to allow multiple threads to have simultaneous read-only access, but as soon
   as one thread wants to write to the resource, all other threads must be
   blocked until the writing is complete.

   QSystemReadWriteLock behaves much like the QReadWriteLock class, but it also
   works across multiple processes (although it also works perfectly well, 
   albeit slightly less efficiently, in a single process).  In order to clean up
   the system resources used to coordinate cross process synchronization, one 
   QReadWriteLock instance is designated the lock "owner".  This instance 
   creates the required system resources, and removes them when it is destroyed.
   The lock owner should always be instantiated before any others.

   System locks are identified by a 32-bit integer, which allows other processes
   to share the same lock.  While the selection of this identifier is left
   upto the caller, under Linux it is common to use the ftok(3) function call
   which uses the identity of a specified file to generate such an identifier.

   For example, to create the lock owner:

   \code
   int id = (int)::ftok("/tmp/my_lock_identifier", 0);

   QSystemReadWriteLock lock(id, true);
   \endcode

   The file passed to ftok(3) is only used to generate a unique identifier for
   the lock and is otherwise unrelated to the lock.  It is possible, although 
   bad practice due to potential unintended clashes with other applications that
   do the same, to simply make up a number for the lock id.  

   Other applications can then easily create a non-owner reference to the lock:
   
   \code
   int id = (int)::ftok("/tmp/my_lock_identifier", 0);

   QSystemReadWriteLock lock(id, false);
   \endcode
   
   An ftok(3) call on the same file was used to ensure the owner and the 
   non-owner use the same id and thus refer to the same system-global lock.

   \section1
   \section2 Algorithm

   The QSystemReadWriteLock class uses Linux kernel semaphores to synchronize
   access between readers and writers.  Two semaphores ReadCount and WriteCount
   are used to allow either multiple concurrent readers or a single exclusive
   writer.  When writers are waiting, any new readers must wait until all
   writers complete.  That is, writers can starve readers.

   The following semaphore conditions determine reader and writer operations.

   \table
   \header
       \o Operation
       \o Condition Steps
   \row
       \o Reader Progression
       \o WAIT(Increment ReadCount) AND WAIT(WriteCount == 0)
   \row
       \o Reader Complete
       \o WAIT(Decrement ReadCount)
   \row
       \o Writer Progression
       \o WAIT(Increment WriteCount)
   \row
       \o
       \o WAIT(ReadCount == 0)
   \row
       \o Writer Complete
       \o WAIT(Decrement WriteCount)
   \endtable

   \sa QSystemMutex

   \ingroup thread
   \ingroup environment
   \ingroup ipc
*/

/*!
  Construct a system read write lock from the provided \a id.  If \a owner is
  true, the instance will create the system resources required for the lock and
  will remove them when it is destructed.
 */
QSystemReadWriteLock::QSystemReadWriteLock(unsigned int id, bool owner)
: d(new QSystemReadWriteLockPrivate(id, owner))
{
    d->semId = ::semget(d->id, 2, owner?IPC_CREAT|00644:0);
    if(-1 == d->semId) {
        qLog(ILFramework) << "QSystemReadWriteLock: Unable to access semaphore"
                          << d->id << "(" << ::strerror(errno) << ")";
    } else if(owner) {
        union semun  {
            int val;
            struct semid_ds *buf;
            ushort *array;
        } arg;
        arg.val = 0;
        if(-1 == ::semctl(d->semId, QSystemReadWriteLockPrivate::ReadSem,
                          SETVAL, arg) ||
           -1 == ::semctl(d->semId, QSystemReadWriteLockPrivate::WriteSem,
                          SETVAL, arg)) {
            qLog(ILFramework) << "QSystemReadWriteLock: Unable to reset semaphore"
                              << d->id << "(" << ::strerror(errno) << ")";
            ::semctl(d->semId, 0, IPC_RMID);
            d->semId = -1;
        }
    }
}

/*!
  Destroy the lock instance.  If owner was specified in the QSystemReadWriteLock
  constructor, all the system resources used by this lock will also be removed
  and further use of the lock by other threads or processes will fail.
  */
QSystemReadWriteLock::~QSystemReadWriteLock()
{
    unlock();
    if(!isNull() && d->own) {
        // Destroy
        if(-1 == ::semctl(d->semId, 0, IPC_RMID))
            qWarning("QSystemReadWriteLock: Unable to remove semaphore "
                     "%x (%s).", d->id, ::strerror(errno));
    }
    Q_ASSERT(d);
    delete d;
    d = 0;
}

/*!
  Return true if the lock is invalid.
  */
bool QSystemReadWriteLock::isNull() const
{
    return (-1 == d->semId);
}

/*!
  Return the id of lock as passed to the constructor.
  */
unsigned int QSystemReadWriteLock::id() const
{
    return d->id;
}

/*!
  Attempt to acquire the read lock.  This method will return true if the lock
  was successfully acquired, false otherwise.

  Aquisition of the read lock may fail if:
  \list 1
  \i The timeout \a milliSec, in milliseconds, expired.

     If the caller wants to poll the lock in a non-blocking way, it should
     specify a timeout of 0.  If the caller would prefer to block until the lock
     is acquired it should specify a timeout of -1.

     Currently, only systems that support the semtimedop(2) system call can 
     perform non-blocking, or time blocked calls.  All other systems will block
     indefinately until the lock is acquired, regardless of the \a milliSec
     value.

  \i The QSystemReadWriteLock instance does not refer to a valid lock.

     Callers can check for an invalid lock using the isNull() method.

  \endlist
  */
bool QSystemReadWriteLock::lockForRead(int milliSec)
{
    if(isNull())
        return false;

    struct timespec ts;
    ts.tv_sec = milliSec / 1000;
    ts.tv_nsec = (milliSec % 1000) * 1000000;

    struct sembuf ops[2];

    ops[0].sem_num = QSystemReadWriteLockPrivate::ReadSem;
    ops[0].sem_op = 1;
    ops[0].sem_flg = SEM_UNDO;

    ops[1].sem_num = QSystemReadWriteLockPrivate::WriteSem;
    ops[1].sem_op = 0;
    ops[1].sem_flg = 0;

#ifdef QTOPIA_HAVE_SEMTIMEDOP
    int semoprv = ::semtimedop(d->semId, ops, 2, &ts);
#else
    int semoprv = ::semop(d->semId, ops, 2);
#endif
    if(-1 != semoprv) {
        d->lock.lock();
        Q_ASSERT(0 == d->writeLocks);
        ++d->readLocks;
        d->lock.unlock();
        return true;
    } else {
        return false;
    }
}

/*!
  Attempt to acquire the write lock.  This method will return true if the lock
  was successfully acquired, false otherwise.

  Aquisition of the write lock may fail if:
  \list 1
  \i The timeout \a milliSec, in milliseconds, expired.

     If the caller wants to poll the lock in a non-blocking way, it should
     specify a timeout of 0.  If the caller would prefer to block until the lock
     is acquired it should specify a timeout of -1.

     Currently, only systems that support the semtimedop(2) system call can 
     perform non-blocking, or time blocked calls.  All other systems will block
     indefinately until the lock is acquired, regardless of the \a milliSec
     value.

  \i The QSystemReadWriteLock instance does not refer to a valid lock.

     Callers can check for an invalid lock using the isNull() method.

  \endlist
   */
bool QSystemReadWriteLock::lockForWrite(int milliSec)
{
    if(isNull())
        return false;

    struct timespec ts;
    ts.tv_sec = milliSec / 1000;
    ts.tv_nsec = (milliSec % 1000) * 1000000;

    struct sembuf op;
    op.sem_num = QSystemReadWriteLockPrivate::WriteSem;
    op.sem_op = 1;
    op.sem_flg = SEM_UNDO;

    // Assumed to be "instant" (or there abouts)
    int semoprv = ::semop(d->semId, &op, 1);
    if(-1 == semoprv) return false;

    op.sem_num = QSystemReadWriteLockPrivate::ReadSem;
    op.sem_op = 0;
    op.sem_flg = 0;

#ifdef QTOPIA_HAVE_SEMTIMEDOP
    semoprv = ::semtimedop(d->semId, &op, 1, &ts);
#else
    semoprv = ::semop(d->semId, &op, 1);
#endif
    if(-1 == semoprv) {
        // Decrement our write lock
        op.sem_num = QSystemReadWriteLockPrivate::WriteSem;
        op.sem_op = -1;
        op.sem_flg = 0;
        ::semop(d->semId, &op, 1);
        return false;
    } else {
        // Sweet
        d->lock.lock();
        Q_ASSERT(0 == d->readLocks);
        ++d->writeLocks;
        d->lock.unlock();
        return true;
    }
}

/*!
  Release the lock.
 */
void QSystemReadWriteLock::unlock()
{
    d->lock.lock();
    struct sembuf op;
    if(d->readLocks) {
        --d->readLocks;
        op.sem_num = QSystemReadWriteLockPrivate::ReadSem;
    } else if(d->writeLocks) {
        --d->writeLocks;
        op.sem_num = QSystemReadWriteLockPrivate::WriteSem;
    } else {
        d->lock.unlock();
        return;
    }

    op.sem_op = -1;
    op.sem_flg = 0;
    ::semop(d->semId, &op, 1);
    d->lock.unlock();
}

class QSystemMutex_Private
{
public:
    QSystemMutex_Private(unsigned int _id, bool _own)
    : id(_id), semId(-1), own(_own) {}
    unsigned int id;
    int semId;
    bool own;
};

/*!
    \class QSystemMutex
    \inpublicgroup QtBaseModule

    \brief The QSystemMutex class provides mutual exclusion between processes.

    A mutex is a synchronization tool for protecting critical sections of the 
    code.  The purpose of a QSystemMutex is to protect an object, data structure
    or section of code so that only one thread, or process, can access it at a 
    time.

    QSystemMutex behaves much like the QMutex class, but it also works across 
    multiple processes (although it also works perfectly well, albeit slightly 
    less efficiently, in a single process).  In order to clean up the system 
    resources used to coordinate cross process synchronization, one 
    QSystemMutex instance is designated the lock "owner".  This instance creates
    the required resources, and removes them when it is destroyed.  The lock 
    owner should always be instantiated before any others.

    System locks are identified by a 32-bit integer, which allows other 
    processes to share the same lock.  While the selection of this identifier is
    left upto the caller, under Linux it is common to use the ftok(3) function 
    call which uses the identity of a specified file to generate such an 
    identifier.

    \code
    int id = (int)::ftok("/tmp/my_lock_identifier", 0);

    QSystemMutex lock(id, true);
    \endcode

    The file passed to ftok(3) is only used to generate a unique identifier for
    the mutex and is otherwise unrelated to the mutx.  It is possible, although 
    bad practice due to potential unintended clashes with other applications 
    that do the same, to simply make up a number for the mutex id.  

    Other applications can then easily create a non-owner reference to the 
    mutex:

    \code
    int id = (int)::ftok("/tmp/my_lock_identifier", 0);

    QSystemMutex lock(id, false);
    \endcode

    An ftok(3) call on the same file was used to ensure the owner and the 
    non-owner use the same id and thus refer to the same system-global mutex.

    \sa QSystemReadWriteLock

    \ingroup ipc
    \ingroup environment
 */

/*!
    Construct a system mutex from the provided \a id.  If \a owner is true, the
    instance will create the system resources required for the mutex and will 
    remove them when it is destructed.
 */
QSystemMutex::QSystemMutex(unsigned int id, bool owner)
    : m_data(new QSystemMutex_Private(id, owner))
{
    m_data->semId = ::semget(m_data->id, 1, owner?IPC_CREAT | IPC_EXCL | S_IRWXU:0);
    if(-1 == m_data->semId)
        qLog(ILFramework) << "QSystemMutex: Unable to access semaphore"
                          << m_data->id << "(" << ::strerror(errno) << ")";

    if (owner) {
        union semun arg;
        arg.val = 1;
        int status = ::semctl(m_data->semId, 0, SETVAL, arg);
        if (status == -1) {
            m_data->semId = -1;   // isNull() now return true
            qLog(ILFramework) << "QSystemMutex: Unable to initialize the semaphore"
                              << m_data->id << "(" << ::strerror(errno) << ")";
        }
    }
}

/*!
  Destroy the mutex instance.  If owner was specified in the QSystemMutex
  constructor, all the system resources used by this mutex will also be removed
  and further use of the lock by other threads or processes will fail.
 */
QSystemMutex::~QSystemMutex()
{
    unlock();
    if(!isNull() && m_data->own) {
        // Destroy
        if(-1 == ::semctl(m_data->semId, 0, IPC_RMID))
            qWarning("QSystemMutex: Unable to remove semaphore "
                    "%x (%s).", m_data->id, ::strerror(errno));
    }
    Q_ASSERT(m_data);
    delete m_data;
    m_data = 0;
}

/*!
  Return true if the mutex is invalid.
 */
bool QSystemMutex::isNull() const
{
    return (-1 == m_data->semId);
}

/*!
  Return the id of mutex as passed to the constructor.
 */
unsigned int QSystemMutex::id() const
{
    return m_data->id;
}

/*!
  Attempt to acquire the lock.  This method will return true if the lock
  was successfully acquired, false otherwise.

  Aquisition of the mutex may fail if:
  \list 1
  \i The timeout \a milliSec, in milliseconds, expired.

     If the caller wants to poll the mutex in a non-blocking way, it should
     specify a timeout of 0.  If the caller would prefer to block until the 
     mutex is acquired it should specify a timeout of -1.

     Currently, only systems that support the semtimedop(2) system call can 
     perform non-blocking, or time blocked calls.  All other systems will block
     indefinately until the mutex is acquired, regardless of the \a milliSec
     value.

  \i The QSystemMutex instance does not refer to a valid lock.

     Callers can check for an invalid lock using the isNull() method.

  \endlist
 */
bool QSystemMutex::lock(int milliSec)
{
    if(isNull())
        return false;


    struct sembuf ops[1];

    ops[0].sem_num = 0;
    ops[0].sem_op = -1;
    ops[0].sem_flg = SEM_UNDO;

#ifdef QTOPIA_HAVE_SEMTIMEDOP
    struct timespec ts;
    ts.tv_sec = milliSec / 1000;
    ts.tv_nsec = (milliSec % 1000) * 1000000;
    int semoprv = ::semtimedop(m_data->semId, ops, 1, &ts);
#else
    Q_UNUSED(milliSec);
    int semoprv = ::semop(m_data->semId, ops, 1);
#endif

    if(-1 != semoprv)
        return true;

    return false;
}

/*!
  Release the mutex.
 */
void QSystemMutex::unlock()
{
    if(isNull())
        return;

    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = SEM_UNDO;
    ::semop(m_data->semId, &op, 1);
}

