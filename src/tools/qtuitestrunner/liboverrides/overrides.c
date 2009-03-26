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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

/*
    This file contains replacement functions for some commonly used C functions, to
    allow certain things to be done through the test framework.  It is compiled into
    a library which is to be loaded using LD_PRELOAD when running Qtopia.
    runqtopia will do this if you run it with `-- -qtuitest-overrides'.

    Currently implemented:
     - Allow setting the time and timezone in Qtopia without running as root
*/

#define P(...) do {\
    fprintf(stderr, "%s:%d", __FILE__, __LINE__);\
    fprintf(stderr, __VA_ARGS__);\
} while(0)

#define SHMKEY 304877

#define TZ_FILENAME "/tmp/qtuitest_tz"

/* Declare a function pointer "real" to the function with the specified signature */
#define RESOLVE(real, ret, name, ...) \
    static ret (*real)(__VA_ARGS__) = 0;\
    if (!real) real = (ret(*)(__VA_ARGS__)) dlsym(RTLD_NEXT, #name); \
    if (!real) P("Couldn't resolve symbol '%s', will probably crash now\n", #name);

/* Offsets in shared memory of various variables. */
#define GIVEN_TV_OFFSET     (0)
#define GIVEN_TV_SET_OFFSET (GIVEN_TV_OFFSET + sizeof(struct timeval))
#define END_OFFSET          (GIVEN_TV_SET_OFFSET + sizeof(struct timeval))

struct timeval *given_tv;
struct timeval *given_tv_set;
int constructed = 0;

void ctor() __attribute__((constructor));

/* Run when shared library is loaded */
void ctor()
{
    if (constructed) return;
    int i_am_the_boss = 0;

    /* Create or open shared memory.  If we successfully create "exclusively", we created
       rather than opened, so we are responsible for initial setup across all processes. */
    int shmid = shmget(SHMKEY, END_OFFSET, IPC_CREAT | IPC_EXCL | 00777);
    if (-1 == shmid) {
        shmid = shmget(SHMKEY, END_OFFSET, IPC_CREAT | 00777);
        if (-1 == shmid) {
            P("shmget failed: %s\n", strerror(errno));
        }
    } else {
        i_am_the_boss = 1;
    }
    void *ptr = shmat(shmid, 0, 0);
    if (-1 == (int)ptr) P("shmat failed: %s\n", strerror(errno));

    given_tv =     (struct timeval *)((char *)ptr + GIVEN_TV_OFFSET);
    given_tv_set = (struct timeval *)((char *)ptr + GIVEN_TV_SET_OFFSET);

    /* The boss needs to initialize memory to 0 and copy the current timezone. */
    if (i_am_the_boss) {
        memset(ptr, 0, END_OFFSET);

        /* For some reason, using system() here does not work. */
        if (0 == fork()) {
            char *argv[] = { "sh", "-c", "cp -f /etc/localtime " TZ_FILENAME, 0 };
            char *envp[] = { 0 };
            execve("/bin/sh", argv, envp);
        }
    }

    constructed = 1;
}

time_t time(time_t *t)
{
    if (!constructed) ctor();
    RESOLVE(f, time_t, time, time_t *t);

    time_t ret = f(t) - given_tv_set->tv_sec + given_tv->tv_sec;
    if (t) *t = ret;
    return ret;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    if (!constructed) ctor();
    RESOLVE(f, int, clock_gettime, clockid_t clk_id, struct timespec *tp);

    if (!tp) return f(clk_id, tp);

    static struct timespec real_tp;
    if (0 != f(clk_id, &real_tp)) return -1;
    tp->tv_sec  = real_tp.tv_sec  - given_tv_set->tv_sec  + given_tv->tv_sec;
    long long nsec = (long long)real_tp.tv_nsec - 1000LL*(long long)given_tv_set->tv_usec + 1000LL*(long long)given_tv->tv_usec;
    if (nsec < 0) {
        --tp->tv_sec;
        tp->tv_nsec = 1000000000L + (long)nsec;
    } else if (nsec > 1000000000LL) {
        ++tp->tv_sec;
        tp->tv_nsec = (long)nsec - 1000000000L;
    } else {
        tp->tv_nsec = (long)nsec;
    }

    return 0;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (!constructed) ctor();
    RESOLVE(f, int, gettimeofday, struct timeval *tv, struct timezone *tz);

    if (!tv) return f(tv,tz);
    if (tz) P("Warning, unhandled tz parameter was passed to gettimeofday\n");


    static struct timeval real_tv;
    if (0 != f(&real_tv, 0)) return -1;
    tv->tv_sec  = real_tv.tv_sec  - given_tv_set->tv_sec  + given_tv->tv_sec;
    tv->tv_usec = real_tv.tv_usec - given_tv_set->tv_usec + given_tv->tv_usec;

    return 0;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
    if (!constructed) ctor();
    RESOLVE(f, int, settimeofday, const struct timeval *tv, const struct timezone *tz);
    RESOLVE(_gettimeofday, int, gettimeofday, const struct timeval *tv, const struct timezone *tz);

    if (!tv) return f(tv, tz);

    if (tz) P("Warning, unhandled tz parameter was passed to settimeofday\n");

    *given_tv = *tv;
    if (0 != _gettimeofday(given_tv_set, 0)) {
        P("Call to real gettimeofday failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/*
    Handles the following commands:
     "/sbin/hwclock -w" - pretends to succeed

    Everything else is passed to the real system().
*/
int system(const char *command)
{
    if (!constructed) ctor();
    RESOLVE(f, int, system, const char *command);

    if (!command) return f(command);

    // For "/sbin/hwclock -w", do nothing, successfully
    if (0 == strcmp(command, "/sbin/hwclock -w")) {
        return WEXITSTATUS(0);
    }

    return f(command);
}

