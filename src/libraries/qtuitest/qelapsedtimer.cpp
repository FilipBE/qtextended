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

#include "qelapsedtimer_p.h"

#include <time.h>
#include <errno.h>
#include <string.h>
#include <QtGlobal>

/*!
    \internal
    \class QElapsedTimer
    \inpublicgroup QtUiTestModule

    QElapsedTimer provides a lightweight
    subset of the QTime API while keeping correct time even if the system time
    is changed.
*/

int qelapsedtimer_gettime() {
#ifdef Q_OS_WIN32
    // FIXME: on Windows, won't keep correct time if system time is changed.
    return time(0)*1000;
#else
    struct timespec tms;
    if (-1 == clock_gettime(
#ifdef _POSIX_MONOTONIC_CLOCK
                CLOCK_MONOTONIC
#else
                CLOCK_REALTIME
#endif
                , &tms)) {
        qFatal("QtUitest: clock_gettime failed: %s",
               strerror(errno));
    }
    return tms.tv_sec*1000 + tms.tv_sec/1000000;
#endif
}

/*!
    Creates a new timer.
    start() is automatically called on the new timer.
*/
QElapsedTimer::QElapsedTimer()
{ start(); }

/*!
    Starts or restarts the timer.
*/
void QElapsedTimer::start()
{ start_ms = qelapsedtimer_gettime(); }

/*!
    Returns the elapsed time (in milliseconds) since start() was called.

    This function returns the correct value even if the system time has
    been changed.  The value may overflow, however.
 */
int QElapsedTimer::elapsed()
{ return qelapsedtimer_gettime() - start_ms; }

