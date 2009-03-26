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

#ifndef QTOPIAUNITTEST_H
#define QTOPIAUNITTEST_H

#include <QTest>
#include <QDebug>
#include <QDir>

#define QTEST_APP_MAIN(TestObject,TestApplication) \
int main(int argc, char *argv[])                   \
{                                                  \
        TestApplication app(argc, argv);           \
        TestObject tc;                             \
        return QTest::qExec(&tc, argc, argv);      \
}

namespace QtopiaUnitTest
{
inline
QString baseDataPath()
{
    static QString ret = (!qgetenv("QTEST_DATA").isEmpty()
            ? qgetenv("QTEST_DATA")
            : QString("%1testdata").arg(
#ifdef TEST_SRCDIR
            TEST_SRCDIR "/"
#else
            ""
#endif
            ));
    static bool created = QDir("/").mkpath(ret);
    if (!created) {
        qFatal("baseDataPath(): Couldn't create %s!\n"
               "Ensure filesystem is writable, or create directory before "
               "running test.", qPrintable(ret));
    }
    return ret;
}
}

#ifndef qLog
# define qLog(dbgcat) if(!dbgcat##_QLog::enabled()); else dbgcat##_QLog::log(#dbgcat)
#endif

class Autotest_QLog {
public:
    static inline bool enabled()
    { return !qgetenv("QTEST_VERBOSE").isEmpty(); }

    static inline QDebug log(const char* category)
    { return QDebug(QtDebugMsg) << category << ":"; }
};

#endif

