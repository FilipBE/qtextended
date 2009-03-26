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

#include "qtopiamessagehandler_p.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QSettings>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct QtopiaMessageHandlerPrivate
{
    static inline clockid_t bestClock();
    static inline qint64    baselineTimestamp();
    static inline qint64    timestamp();

    static void preinstall();

    static inline bool& installed()
    { static bool ret = false; return ret; }

    static inline bool& skewOwner()
    { static bool ret = false; return ret; }

    static inline QtMsgHandler& oldHandler()
    { static QtMsgHandler ret(0); return ret; }

    static inline QByteArray& logFormat()
    { static QByteArray ret; return ret; }

    static inline QByteArray& applicationName()
    { static QByteArray ret; return ret; }

    static inline pid_t pid()
    { static pid_t ret = ::getpid(); return ret; }

    static inline int*& clockSkew()
    { static int* ret = 0; return ret; }

    static qint64 currentUptime();
    static qint64 parseUptime(QByteArray const&, char const*);

    static const key_t TimestampShmKey = 304878;
};

/*
    Get the message handler installed as early as possible so that even a qLog
    from the beginning of main() will be caught.
*/
static struct QtopiaMessageHandlerAutoInstaller
{
    inline QtopiaMessageHandlerAutoInstaller()
    { QtopiaMessageHandler::install(); }
} qtopiaMessageHandlerInstaller;

clockid_t QtopiaMessageHandlerPrivate::bestClock()
{
    struct timespec dontcare;

#ifdef CLOCK_MONOTONIC
    // CLOCK_MONOTONIC is preferable but not always available.
    if (0 == ::clock_gettime(CLOCK_MONOTONIC, &dontcare))
        return CLOCK_MONOTONIC;
#endif
    // CLOCK_REALTIME is available "everywhere", but if we use it then time changes
    // might mess with timestamps.
    return CLOCK_REALTIME;
}

/*
    Returns time in milliseconds since some fixed point in the past.
*/
qint64 QtopiaMessageHandlerPrivate::timestamp()
{
    struct timespec now;

    static clockid_t best_clock = bestClock();

    if (-1 == ::clock_gettime(best_clock, &now)) {
        ::perror("QtopiaMessageHandler: gettime:");
        return -1;
    }

    // If the clock is not monotonic, we have to record time changes and adjust
    // timestamps accordingly.
    static bool use_skew =
#ifdef CLOCK_MONOTONIC
        (best_clock != CLOCK_MONOTONIC)
#else
        true
#endif
    ;

    qint64 ret = now.tv_sec*1000 + now.tv_nsec/1000000;
    if (use_skew) ret += (*clockSkew())*1000;
    return ret;
}

void QtopiaMessageHandler::timeChanged(uint oldutc, uint newutc)
{
    // Only one process updates the skew.
    if (QtopiaMessageHandlerPrivate::skewOwner()) {
        *QtopiaMessageHandlerPrivate::clockSkew() += (oldutc - newutc);
    }
}

/*
    Returns realtime since boot in milliseconds according to /proc/uptime.
*/
qint64 QtopiaMessageHandlerPrivate::currentUptime()
{
    qint64 ret = 0;
    int fd = ::open("/proc/uptime", 0);
    char buf[512];
    ssize_t bytes, total = 0;
    if (fd == -1) {
        ::perror("QtopiaMessageHandler: open(\"/proc/uptime\")");
        goto out;
    }

    do {
        bytes = ::read(fd, &buf[total], 512 - total);
        if (bytes == -1 && errno != EINTR) {
            ::perror("QtopiaMessageHandler: read /proc/uptime");
            ::close(fd);
            goto out;
        }
        else if (bytes == -1) {}
        else if (bytes == 0) { break; }
        else { total += bytes; }
    } while(true);
    buf[total-1] = 0;

    ret = parseUptime(QByteArray::fromRawData(buf, total), "/proc/uptime");

    ::close(fd);

out:
    return ret;
}

qint64 QtopiaMessageHandlerPrivate::parseUptime(QByteArray const& ba, const char* input)
{
    qint64 ret = 0;

    // Example of /proc/uptime:
    //   5633725.47 4887800.54
    // First field is real time (what we want).
    int dot = ba.indexOf('.');
    if (dot != -1 && dot < ba.size()-3) {
        bool ok;
        // FIXME use QByteArray::fromRawData when Qt bug 231400 is fixed.
        ret += QByteArray(ba.constData(), dot).toLongLong(&ok,10)*1000;
        if (ok) {
            ret += QByteArray(&ba.constData()[dot+1], 2).toLongLong(&ok,10)*10;
        }
        if (!ok) {
            ::fprintf(stderr, "QtopiaMessageHandler: error parsing %s (%s)\n", input, ba.constData());
        }
    }

    return ret;
}

/*
    Returns the value of QtopiaMessageHandler::timestamp() when the first
    message handler was installed during the current Qtopia instance.
    Uses shared memory to return the same value across all processes.
*/
qint64 QtopiaMessageHandlerPrivate::baselineTimestamp()
{
    static qint64* ret = 0;
    if (ret) return *ret;

    int shmid = ::shmget(TimestampShmKey, sizeof(qint64)+sizeof(int), IPC_CREAT|00777);
    if (-1 == shmid) {
        ::perror("QtopiaMessageHandler: shmget");
        return -1;
    }
    ret = (qint64*)::shmat(shmid, 0, 0);
    if ((qint64*)-1 == ret) {
        ret = 0;
        ::perror("QtopiaMessageHandler: shmat");
        return -1;
    }
    clockSkew() = (int*)(&ret[1]);

    // Check if we are the first process to attach.
    struct shmid_ds shmstat;
    if ((0 == ::shmctl(shmid, IPC_STAT, &shmstat)) && (1 == shmstat.shm_nattch)) {
        *ret = 0;
    }

    // If we are the first process to attach (i.e. qpe), or the timestamp was
    // explicitly cleared, initialize the timestamp to now.
    if (0 == *ret) {
        *ret = timestamp();
        (*clockSkew()) = 0;

        // Let this application be the one to maintain clock skew.
        skewOwner() = true;

        // If we recorded a monotonic timestamp before launching Qtopia, use that
        // to get a fairly accurate approximation of time since fork().
        static const char launchvar[] = "QTOPIA_UPTIME_AT_LAUNCH";
        QByteArray uptime = qgetenv(launchvar);
        if (!uptime.isEmpty()) {
            qint64 current = currentUptime();
            qint64 parsed = parseUptime(uptime, launchvar);
            if (current && parsed)
                (*ret) += parsed - current;
        }
    }

    return *ret;
}

/*
    The Qt message handler.
    Called for every qLog, qDebug, qWarning, qCritical and qFatal.
*/
void QtopiaMessageHandler::handleMessage(QtMsgType type, const char* message)
{
    static const int QT_BUFFER_LENGTH = 8192;
    char buf[QT_BUFFER_LENGTH];
    QtopiaMessageHandler::snprintf(buf, QT_BUFFER_LENGTH, QtopiaMessageHandlerPrivate::logFormat(), message);

    switch (type) {
    case QtDebugMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        ::fprintf(stderr, "%s\n", buf);
        ::fflush(stderr);
        break;
    case QtFatalMsg:
        qInstallMsgHandler(0);
        qFatal("%s", buf);
        break;
    }
}

/*
    Like strncpy, but doesn't pad the remainder of the string with null bytes
    and returns the new pointer to the end of the string.  Considerably faster
    than strncpy when n > strlen(src) (the common case for us).
*/
inline char* qfaststrncpy(char* dest, char const* src, int n)
{
    if (!n) return 0;

    int len = strlen(src);
    if (len >= n) len = n-1;
    memcpy(dest, src, len+1);
    return dest+len;
}

/*
    An snprintf which parses the QtopiaMessageHandler log format string.
*/
void QtopiaMessageHandler::snprintf(char* buf, int n, const char* fmt, const char* msg)
{
    if (!n) return;

    qint64 toc = 0;
    char* buf_end = buf + n;
    buf[0] = 0;
    buf[n-1] = 0;

    bool in_percent = false;

    while (*fmt && (buf < buf_end-1)) {
        switch (*fmt) {

        case '%':
            // If we already were processing a %, then this is a real (escaped) %
            if (in_percent) goto plain_ol_character;
            in_percent = true;
            break;

        // %s: actual log message
        case 's':
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            buf = qfaststrncpy(buf, msg, buf_end - buf - 1);
            break;

        // %n: application name
        case 'n':
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            buf = qfaststrncpy(buf, QtopiaMessageHandlerPrivate::applicationName().constData(),
                    buf_end - buf - 1);
            break;

        // %t: monotonic timestamp in milliseconds
        case 't':
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            static qint64 tic = QtopiaMessageHandlerPrivate::baselineTimestamp();
            toc = QtopiaMessageHandlerPrivate::timestamp();
            buf += qsnprintf(buf, buf_end - buf - 1, "%lld", toc - tic);
            break;

        // %p: application pid
        case 'p':
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            buf += qsnprintf(buf, buf_end - buf - 1, "%d", QtopiaMessageHandlerPrivate::pid());
            break;

        default:
        plain_ol_character:
            in_percent = false;
            *buf = *fmt;
            ++buf;
            break;
        }
        ++fmt;
    }
}

/*
    Perform private initialization prior to installing message handler.
    It is important to call this before installing the message handler because
    some initialization may result in a qDebug or qWarning occurring, which would
    result in infinite recursion.
*/
void QtopiaMessageHandlerPrivate::preinstall()
{
    QtopiaMessageHandler::reload();
    (void)bestClock();
    (void)baselineTimestamp();
}

/*
    Start using QtopiaMessageHandler for log messages.
    This function must be called to apply log formatting to messages from Qtopia.
*/
void QtopiaMessageHandler::install()
{
    if (!QtopiaMessageHandlerPrivate::installed()) {
        QtopiaMessageHandlerPrivate::installed() = true;
        QtopiaMessageHandlerPrivate::preinstall();
        QtopiaMessageHandlerPrivate::oldHandler() = qInstallMsgHandler(QtopiaMessageHandler::handleMessage);
    }
}

/*
    Stop using QtopiaMessageHandler for log messages.
*/
void QtopiaMessageHandler::uninstall()
{
    qInstallMsgHandler(QtopiaMessageHandlerPrivate::oldHandler());
    QtopiaMessageHandlerPrivate::installed() = false;
}

/*
    Reload everything which has an impact on QtopiaMessageHandler.
*/
void QtopiaMessageHandler::reload()
{
    reloadConfiguration();
    reloadApplicationName();
}

/*
    Reload log settings.
*/
void QtopiaMessageHandler::reloadConfiguration()
{
    QtopiaMessageHandlerPrivate::logFormat() = QSettings("Trolltech", "Log2")
        .value("MessageHandler/Format", QByteArray("%s")).toByteArray();
}

/*
    Reload application name.
    Call this when the application name changes (for example, because QtopiaApplication
    has just been constructed, or because quicklauncher has changed the application name).
*/
void QtopiaMessageHandler::reloadApplicationName()
{
    QByteArray& appname = QtopiaMessageHandlerPrivate::applicationName();
    appname = QByteArray();
    if (QCoreApplication::instance()) {
        appname = QCoreApplication::instance()->applicationName().toLocal8Bit();
    }
#ifdef Q_OS_LINUX
    if (appname.isEmpty()) {
        char filebuf[PATH_MAX];
        if (filebuf == ::realpath("/proc/self/exe", filebuf)) {
            appname = QByteArray::fromRawData(filebuf, strlen(filebuf));
            int lastslash = appname.lastIndexOf("/");
            if ((-1 != lastslash) && (lastslash != appname.length()-1)) {
                appname = appname.mid(lastslash+1);
            } else {
                (void)appname.data(); // detach
            }
        }
    }
#endif
}

