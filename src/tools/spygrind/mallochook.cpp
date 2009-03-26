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

// Record the mallocs and frees done by a process

#include "qsignalspycollector.h"

#include <QMutex>
#include <QThread>
#include <QCoreApplication>

#include <malloc.h>
#include <stdio.h>

typeof(__malloc_hook)  old_malloc_hook;
typeof(__realloc_hook) old_realloc_hook;
typeof(__free_hook)    old_free_hook;

void* malloc_hook(size_t,const void*);
void* realloc_hook(void*,size_t,const void*);
void  free_hook(void*,const void*);

#define STASH_HOOKS do {                    \
    Q_ASSERT(__malloc_hook == malloc_hook); \
    __malloc_hook  = old_malloc_hook;       \
    __realloc_hook = old_realloc_hook;      \
    __free_hook    = old_free_hook;         \
    Q_ASSERT(__malloc_hook != malloc_hook); \
} while(0)

#define UNSTASH_HOOKS do {                  \
    Q_ASSERT(__malloc_hook != malloc_hook); \
    old_malloc_hook  = __malloc_hook;       \
    __malloc_hook    = malloc_hook;         \
    old_realloc_hook = __realloc_hook;      \
    __realloc_hook   = realloc_hook;        \
    old_free_hook    = __free_hook;         \
    __free_hook      = free_hook;           \
    Q_ASSERT(__malloc_hook == malloc_hook); \
} while(0)

QMutex malloc_hook_mutex(QMutex::Recursive);

void* malloc_hook(size_t size, const void* caller)
{
    QMutexLocker lock(&malloc_hook_mutex);

    STASH_HOOKS;

    void* ret = malloc(size);
    QSignalSpyCollector::instance()->recordMalloc(caller, size, ret);

    UNSTASH_HOOKS;
    return ret;
}

void free_hook(void* ptr, const void* caller)
{
    QMutexLocker lock(&malloc_hook_mutex);
    STASH_HOOKS;

    free(ptr);
    QSignalSpyCollector::instance()->recordFree(caller, ptr);

    UNSTASH_HOOKS;
}

void* realloc_hook(void* ptr, size_t size, const void* caller)
{
    QMutexLocker lock(&malloc_hook_mutex);
    STASH_HOOKS;

    void* ret = realloc(ptr, size);
    QSignalSpyCollector::instance()->recordRealloc(caller, ptr, size, ret);

    UNSTASH_HOOKS;

    return ret;
}

void init_malloc_hooks()
{
    QMutexLocker lock(&malloc_hook_mutex);
    if (malloc_hook != __malloc_hook) {
        UNSTASH_HOOKS;
    }
}

void uninit_malloc_hooks()
{
    QMutexLocker lock(&malloc_hook_mutex);
    if (malloc_hook == __malloc_hook) {
        STASH_HOOKS;
    }
}

