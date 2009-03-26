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

#ifndef STAMP_P_H
#define STAMP_P_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <QtGlobal>

#define ERRNO_CHECK(func,...) do {              \
    if (func(__VA_ARGS__) == -1)                \
        qFatal("spygrind: " #func ": %s", strerror(errno));  \
} while(0)
#define qt_getrusage(...)     ERRNO_CHECK(getrusage,     __VA_ARGS__)
#define qt_clock_gettime(...) ERRNO_CHECK(clock_gettime, __VA_ARGS__)

// Represents a snapshot of resource usage at a particular point in time,
// such that a stamp at time t1 minus a stamp at time t0 will be equal to
// the resources used during the time from t0..t1.
struct Stamp
{
    inline Stamp()
    {
        ru.ru_utime.tv_sec  = 0;
        ru.ru_utime.tv_usec = 0;
        ru.ru_stime = ru.ru_utime;
        tp.tv_sec  = 0;
        tp.tv_nsec = 0;
        bytesAllocated = 0;
        bytesFreed = 0;
        bytesLeaked = 0;
    }

    static inline Stamp now()
    {
        Stamp ret;
        qt_getrusage(RUSAGE_SELF, &ret.ru);
        qt_clock_gettime(best_clock(), &ret.tp);
        return ret;
    }

    inline Stamp& operator-=(Stamp const& other)
    {
        ru.ru_utime.tv_sec  -= other.ru.ru_utime.tv_sec;
        ru.ru_utime.tv_usec -= other.ru.ru_utime.tv_usec;
        ru.ru_stime.tv_sec  -= other.ru.ru_stime.tv_sec;
        ru.ru_stime.tv_usec -= other.ru.ru_stime.tv_usec;
        tp.tv_sec  -= other.tp.tv_sec;
        tp.tv_nsec -= other.tp.tv_nsec;
        fixTimes(&ru.ru_utime);
        fixTimes(&ru.ru_stime);
        fixTimes(&tp);

        // This logic for leaks is unintuitive, but it is to workaround a kcachegrind
        // limitation, where it is assumed values can never be negative.  This means we
        // lose information about functions which free up more memory than they allocate.
        qint64 leak = (bytesAllocated - other.bytesAllocated) - (bytesFreed - other.bytesFreed);
        if (leak > 0)
            bytesLeaked += leak;
        bytesAllocated -= other.bytesAllocated;
        bytesFreed     -= other.bytesFreed;
        return *this;
    }

    inline Stamp& operator+=(Stamp const& other)
    {
        ru.ru_utime.tv_sec  += other.ru.ru_utime.tv_sec;
        ru.ru_utime.tv_usec += other.ru.ru_utime.tv_usec;
        ru.ru_stime.tv_sec  += other.ru.ru_stime.tv_sec;
        ru.ru_stime.tv_usec += other.ru.ru_stime.tv_usec;
        tp.tv_sec  += other.tp.tv_sec;
        tp.tv_nsec += other.tp.tv_nsec;
        fixTimes(&ru.ru_utime);
        fixTimes(&ru.ru_stime);
        fixTimes(&tp);
        bytesAllocated += other.bytesAllocated;
        bytesFreed     += other.bytesFreed;
        bytesLeaked    += other.bytesLeaked;
        return *this;
    }

    struct rusage   ru;
    struct timespec tp;

    quint64 bytesAllocated;
    quint64 bytesFreed;
    quint64 bytesLeaked;

private:
    static inline clockid_t best_clock()
    {
        // In theory, this can be determined by looking at _POSIX_MONOTONIC_CLOCK.
        // In practice, sometimes when compiling, that symbol is defined but the monotonic
        // clock doesn't work.  So we must discover it at runtime.
        static clockid_t ret = -1;
        if (-1 == ret) {
            ret = CLOCK_MONOTONIC;
            struct timespec dummy;
            // If monotonic fails, use realtime.
            // _everything_ is supposed to have realtime.
            if (-1 == clock_gettime(ret, &dummy))
                ret = CLOCK_REALTIME;
        }
        return ret;
    }

    inline void fixTimes(struct timeval* time)
    {
        if (time->tv_usec < 0) {
            time->tv_usec += 1000000;
            time->tv_sec  -= 1;
        }
        if (time->tv_usec >= 1000000) {
            time->tv_usec -= 1000000;
            time->tv_sec  += 1;
        }
    }

    inline void fixTimes(struct timespec* time)
    {
        if (time->tv_nsec < 0) {
            time->tv_nsec += 1000000000;
            time->tv_sec  -= 1;
        }
        if (time->tv_nsec >= 1000000000) {
            time->tv_nsec -= 1000000000;
            time->tv_sec  += 1;
        }
    }

};

inline Stamp operator-(Stamp const& l, Stamp const& r)
{
    Stamp ret = l;
    return (ret -= r);
}

inline Stamp operator+(Stamp const& l, Stamp const& r)
{
    Stamp ret = l;
    return (ret += r);
}

#endif

