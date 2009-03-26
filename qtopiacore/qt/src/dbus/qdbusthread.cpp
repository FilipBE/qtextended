/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDBus module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>

#include <stdlib.h>
#include <qdbus_symbols_p.h>

QT_USE_NAMESPACE

static DBusMutex* mutex_new()
{
    return reinterpret_cast<DBusMutex *>(new QMutex(QMutex::NonRecursive));
}

#if 0
static DBusMutex* recursive_mutex_new()
{
    return reinterpret_cast<DBusMutex *>(new QMutex(QMutex::Recursive));
}
#endif

static void mutex_free(DBusMutex *mutex)
{
    delete reinterpret_cast<QMutex *>(mutex);
}

static dbus_bool_t mutex_lock(DBusMutex *mutex)
{
    reinterpret_cast<QMutex *>(mutex)->lock();
    return true;
}

static dbus_bool_t mutex_unlock(DBusMutex *mutex)
{
    reinterpret_cast<QMutex *>(mutex)->unlock();
    return true;
}

static DBusCondVar* condvar_new()
{
    return reinterpret_cast<DBusCondVar *>(new QWaitCondition);
}

static void condvar_free(DBusCondVar *cond)
{
    delete reinterpret_cast<QWaitCondition *>(cond);
}

static void condvar_wait(DBusCondVar *cond, DBusMutex *mutex)
{
    reinterpret_cast<QWaitCondition *>(cond)->wait(reinterpret_cast<QMutex *>(mutex));
}

static dbus_bool_t condvar_wait_timeout(DBusCondVar *cond, DBusMutex *mutex, int msec)
{
    return reinterpret_cast<QWaitCondition *>(cond)->wait(reinterpret_cast<QMutex *>(mutex), msec);
}

static void condvar_wake_one(DBusCondVar *cond)
{
    reinterpret_cast<QWaitCondition *>(cond)->wakeOne();
}

static void condvar_wake_all(DBusCondVar *cond)
{
    reinterpret_cast<QWaitCondition *>(cond)->wakeAll();
}

QT_BEGIN_NAMESPACE

bool qDBusInitThreads()
{
    // ### Disable the recursive mutex functions.
    static DBusThreadFunctions fcn = {
        DBUS_THREAD_FUNCTIONS_MUTEX_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_LOCK_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_UNLOCK_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAIT_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAIT_TIMEOUT_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAKE_ONE_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAKE_ALL_MASK,
#if 0
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_LOCK_MASK |
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_UNLOCK_MASK,
#endif
        mutex_new,
        mutex_free,
        mutex_lock,
        mutex_unlock,
        condvar_new,
        condvar_free,
        condvar_wait,
        condvar_wait_timeout,
        condvar_wake_one,
        condvar_wake_all,
#if 0
        recursive_mutex_new,
        mutex_free,
        mutex_lock,
        mutex_unlock,
#else
        0, 0, 0, 0,
#endif
        0, 0, 0, 0
    };

#if !defined QT_LINKED_LIBDBUS
    void (*threads_init_default)() = (void (*)())qdbus_resolve_conditionally("dbus_threads_init_default");
    void (*threads_init)(DBusThreadFunctions *) = (void (*)(DBusThreadFunctions*))qdbus_resolve_conditionally("dbus_threads_init");

    if (threads_init_default)
        threads_init_default();
    else if (threads_init)
        threads_init(&fcn);
    else
        return false;

    return true;
#else
    dbus_threads_init(&fcn);

    return true;
#endif
}

QT_END_NAMESPACE
