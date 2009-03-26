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

#include "qbuild.h"
#include "solution.h"
#include "options.h"
#include <QTime>
#include "qoutput.h"
#ifndef NO_BACKTRACE
#include <stdio.h>
#include <execinfo.h>
#define TRACE_LIMIT 999
#endif

QStringList QBuild::m_options;
SolutionProject QBuild::m_root;
QMutex QBuild::m_perfLock;
QThreadStorage<ThreadPerfInfo *> QBuild::m_perfStack;
QHash<QString, int> QBuild::m_perfTiming;

static void dump_backtrace()
{
#ifndef NO_BACKTRACE
    // print a backtrace (currently glibc-specific)
    void *array[TRACE_LIMIT];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, TRACE_LIMIT);
    strings = backtrace_symbols(array, size);
    if ( strings != 0 ) {
        fprintf(stdout, "Obtained %zd stack frames:\n", size-1);

        for ( i = 1; i < size; i++ )
            fprintf(stdout, "%03d %s\n", size-i, strings[i]);

        free(strings);
    }
    fflush(stdout);
#endif
}

void QBuild::shutdown()
{
    if (::options.debug & Options::StackOnFatal)
        dump_backtrace();
    // FIXME do something a bit nicer than this!
    abort();
}

QStringList QBuild::options()
{
    return m_options;
}

void QBuild::setOptions(const QStringList &opt)
{
    m_options = opt;
}

SolutionProject QBuild::rootProject()
{
    return m_root;
}

void QBuild::setRootProject(const SolutionProject &proj)
{
    m_root = proj;
}

QList<QBuild::Reset *> QBuild::Reset::resetObjects;
QBuild::Reset::Reset()
{
    registerResetObject(this);
}

void QBuild::Reset::registerResetObject(Reset *obj)
{
    resetObjects << obj;
}

void QBuild::Reset::resetAll()
{
    for (int ii = 0; ii < resetObjects.count(); ++ii)
        resetObjects.at(ii)->reset();
}

struct ThreadPerfInfo
{
    QTime timer;
    QStringList perfStack;
};

void QBuild::beginPerfTiming(const QString &type)
{
    if (!(::options.debug & Options::PerfTiming))
        return;

    ThreadPerfInfo *info = m_perfStack.localData();
    if (!info) {
        info = new ThreadPerfInfo;
        m_perfStack.setLocalData(info);
    }

    if (!info->perfStack.isEmpty()) {
        int elapsed = info->timer.elapsed();
        LOCK(QBuild);
        m_perfLock.lock();
        m_perfTiming[info->perfStack.last()]+= elapsed;
        m_perfLock.unlock();
    }
    info->perfStack.append(type);
    info->timer.start();
}

void QBuild::beginPerfTiming(const char *type, const QString &arg)
{
    if (!(::options.debug & Options::PerfTiming))
        return;

    beginPerfTiming(QString(type) + ": " + arg);
}

void QBuild::endPerfTiming()
{
    if (!(::options.debug & Options::PerfTiming))
        return;
    ThreadPerfInfo *info = m_perfStack.localData();
    Q_ASSERT(info && !info->perfStack.isEmpty());
    int elapsed = info->timer.elapsed();
    LOCK(QBuild);
    m_perfLock.lock();
    m_perfTiming[info->perfStack.last()]+= elapsed;
    m_perfLock.unlock();
    info->perfStack.removeLast();
    info->timer.start();
}

static bool operator<(const QPair<QString, int> &lhs, const QPair<QString, int> &rhs)
{
    return lhs.second < rhs.second;
}

void QBuild::displayPerfTiming()
{
        LOCK(QBuild);
    m_perfLock.lock();
    qWarning() << "QBuild: Measured module performance:";
    QList<QPair<QString, int> > timings;

    for (QHash<QString, int>::ConstIterator iter = m_perfTiming.begin();
            iter != m_perfTiming.end();
            iter++) {
        if (!iter.key().isEmpty())
            timings.append(qMakePair(iter.key(), iter.value()));
    }

    qSort(timings.begin(), timings.end());
    for (int ii = 0; ii < timings.count(); ++ii) {
        qOutput() << "    " << timings.at(ii).first << timings.at(ii).second;
    }

    m_perfLock.unlock();

}

