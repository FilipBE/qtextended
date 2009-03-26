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
#include "qsystemsemaphore.h"

#include <qdebug.h>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

class QSystemSemaphorePrivate
{
public:
    sem_t *semdata;
#ifdef USE_EVIL_SIGNAL_HANDLERS
    static QList<sem_t *> semlist;
#endif
};

static QString mungePath(const QString &in)
{
    // if you're curious about the below munging, see https://www.redhat.com/archives/phil-list/2003-January/msg00113.html
    QString mungedpath;
    if(in[0] != '/')
        mungedpath=in;
    else
        mungedpath=in.right(in.size()-1);
    mungedpath.replace(QString('/'), QString('_'));
    mungedpath = '/'+mungedpath;
    return mungedpath;
}

#ifdef USE_EVIL_SIGNAL_HANDLERS
QList<sem_t *> QSystemSemaphorePrivate::semlist;

int signalList[14] = {
    SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGKILL,
    SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2, SIGBUS
};

typedef void (*sighandler_t)(int);
sighandler_t oldHandlers[sizeof(signalList)/sizeof(int)];

void cleanUpSemaphoreSignalHandler(int sig)
{
    foreach(sem_t *current, QSystemSemaphorePrivate::semlist)
        sem_post(current);
    for ( uint i = 0; i < sizeof(signalList)/sizeof(int); i++ )
        if ( signalList[i] == sig ) {
            if ( oldHandlers[i] )
                oldHandlers[i](sig);
            else
                abort();
        }
}
#endif // USE_EVIL_SIGNAL_HANDLERS

QSystemSemaphore::QSystemSemaphore(const QString &fpath, int n)
{
    d = new QSystemSemaphorePrivate;
#ifdef USE_EVIL_SIGNAL_HANDLERS
    if(d->semlist.count() == 0) {
        for ( uint i = 0; i < sizeof(signalList)/sizeof(int); i++ ) {
            struct sigaction act;
            struct sigaction oldact;
            act.sa_handler = cleanUpSemaphoreSignalHandler;
            act.sa_flags = SA_ONESHOT | SA_ONSTACK;
            if ( sigaction(i, &act, &oldact) == 0 )
                oldHandlers[i] = oldact.sa_handler;
            else
                oldHandlers[i] = 0;
        }
    }
#endif // USE_EVIL_SIGNAL_HANDLERS

    d->semdata = sem_open(mungePath(fpath).toLocal8Bit(), 0);
    if(d->semdata == SEM_FAILED && errno == ENOENT)
    {
        d->semdata = sem_open(mungePath(fpath).toLocal8Bit(), O_CREAT, S_IRWXU, n);
        if(d->semdata == SEM_FAILED)
            qWarning() << "sem_open returned error(" << errno << ":" << strerror(errno) << ")";
    }
}

QSystemSemaphore::~QSystemSemaphore()
{
    if(d->semdata != SEM_FAILED)
    {
        if(sem_close(d->semdata) == -1)
            qWarning() << "sem_close returned error(" << errno << ":" << strerror(errno) << ")";
        // may need to test for/provide semaphore unlinking.
    }
    delete d;
}

void QSystemSemaphore::acquire()
{
    if(d->semdata != SEM_FAILED)
    {
        if(sem_wait(d->semdata) == -1)
            qWarning() << "sem_wait returned error(" << errno << ":" << strerror(errno) << ")";
#ifdef USE_EVIL_SIGNAL_HANDLERS
        else
            d->semlist.append(d->semdata);
#endif
    }
}

void QSystemSemaphore::release()
{
    if(d->semdata != SEM_FAILED)
    {
        if(sem_post(d->semdata) == -1)
            qWarning() << "sem_post returned error(" << errno << ":" << strerror(errno) << ")";
#ifdef USE_EVIL_SIGNAL_HANDLERS
        else
            d->semlist.removeAll(d->semdata);
#endif
    }
}

int QSystemSemaphore::available()
{
    int result=-1;
    if(d->semdata != SEM_FAILED)
    {
        if(sem_getvalue(d->semdata, &result) != -1)
            result = true;
        else
            qWarning() << "sem_getvalue returned error(" << errno << ":" << strerror(errno) << ")";
    }
    return result;
}

bool QSystemSemaphore::tryAcquire()
{
    bool result=false;
    if(d->semdata != SEM_FAILED)
    {
        if(sem_trywait(d->semdata) != -1)
            result = true;
        else
            qWarning() << "sem_trywait returned error(" << errno << ":" << strerror(errno) << ")";
    }
    return result;
}

bool QSystemSemaphore::tryAcquire(int timeout)
{
    bool result=false;
    if(d->semdata != SEM_FAILED)
    {
        timeval tv;
        timespec tstimeout;

        if(gettimeofday(&tv, NULL) != 0)
        {
            qWarning() << "gettimeofday returned error(" << errno << ":" << strerror(errno) << ")";
        }

        tstimeout.tv_sec = tv.tv_sec + timeout/1000;
        tstimeout.tv_nsec = tv.tv_usec * 1000 + (timeout%1000)*1000000L;

        if(sem_timedwait(d->semdata, &tstimeout) != -1)
            result = true;
        else
            qWarning() << "sem_timedwait returned error(" << errno << ":" << strerror(errno) << ")";
    }
    return result;
}

bool QSystemSemaphore::clear(const QString &fpath)
{
    if ( sem_unlink(mungePath(fpath).toLocal8Bit()) != -1 )
        return true;
    if ( errno == ENOENT ) {
        return true;
    } else {
        qWarning() << "sem_unlink returned error(" << errno << ":" << strerror(errno) << ")";
        return false;
    }
}

bool QSystemSemaphore::isValid()
{
    return d->semdata != SEM_FAILED;
}

