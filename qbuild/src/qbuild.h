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

#ifndef QBUILD_H
#define QBUILD_H

#include <QStringList>
#include <QMutex>
#include <QThreadStorage>
class SolutionProject;
struct ThreadPerfInfo;

//#define LOCK(name) qWarning() << "LOCK " # name;
#define LOCK(name)

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#endif

class QBuild
{
public:
    static void shutdown() NORETURN; // does not return

    static QStringList options();
    static void setOptions(const QStringList &);

    static SolutionProject rootProject();
    static void setRootProject(const SolutionProject &);

    struct Reset {
        Reset();
        virtual ~Reset() {}
        virtual void reset() = 0;
        static void resetAll();
    private:
        static void registerResetObject(Reset *);
        static QList<Reset *> resetObjects;
    };

    static void beginPerfTiming(const QString &);
    static void beginPerfTiming(const char *, const QString & = QString());
    static void endPerfTiming();
    static void displayPerfTiming();
private:
    static SolutionProject m_root;
    static QStringList m_options;
    static QMutex m_perfLock;
    static QThreadStorage<ThreadPerfInfo *> m_perfStack;
    static QHash<QString, int> m_perfTiming;
};

#endif
