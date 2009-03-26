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

#ifndef QABSTRACTTEST_H
#define QABSTRACTTEST_H

#if ! defined Q_QDOC

#include <QObject>
#include <QTest>
#include <qtuitestglobal.h>
#include <qdebug.h>

#undef QTEST_APPLESS_MAIN
#define QTEST_APPLESS_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    TestObject tc; \
    QAbstractTest* at = qobject_cast<QAbstractTest*>(&tc);\
    if (at)\
        return at->exec(argc, argv, __FILE__);\
    else\
        return QTest::qExec(&tc, argc, argv); \
}

#define QTEST_APP_MAIN(TestObject,TestApplication) \
int main(int argc, char *argv[]) \
{ \
    TestApplication app(argc, argv); \
    TestObject tc; \
    QAbstractTest* at = qobject_cast<QAbstractTest*>(&tc);\
    if (at)\
        return at->exec(argc, argv, __FILE__);\
    else\
        return QTest::qExec(&tc, argc, argv); \
}

#undef QTEST_MAIN

#ifdef QT_GUI_LIB
# include <QtTest/qtest_gui.h>
# define QTEST_MAIN(TestObject) QTEST_APP_MAIN(TestObject,QApplication)
#else
# define QTEST_MAIN(TestObject) QTEST_APP_MAIN(TestObject,QCoreApplication)
#endif // QT_GUI_LIB

#ifndef qLog
# define qLog(dbgcat) if(!dbgcat##_QLog::enabled()); else dbgcat##_QLog::log(#dbgcat)
#endif

class QTUITEST_EXPORT Autotest_QLog {
public:
    static inline bool enabled() { return m_enabled; }
    static QDebug log(const char* category);
private:
    static bool m_enabled;
    friend class QAbstractTest;
};

class QAbstractTestPrivate;
class FatalTimeoutThread;
class Timer;

class QTUITEST_EXPORT QTimedTest : public QObject
{
#ifndef Q_MOC_RUN
    Q_OBJECT
#endif
public:
    QTimedTest(QObject *parent=0);
    virtual ~QTimedTest();

    int real_qt_metacall(QMetaObject::Call, int, void **);

protected:
    bool pass_through;
    bool print_times;
    bool have_init;
    bool have_cleanup;
    FatalTimeoutThread *timeout_thread;
    Timer *realTimer;
    Timer *cpuTimer;
};

#ifdef Q_QDOC
namespace QAbstractTest
#else
class QTUITEST_EXPORT QAbstractTest : public QTimedTest
#endif
{
    Q_OBJECT

public:
    enum LearnMode
    {
        LearnNone,
        LearnNew,
        LearnAll
    };

#ifndef Q_QDOC
    QAbstractTest(QString const &srcdir =
#ifdef QTUITEST_SRCDIR
            QTUITEST_SRCDIR
#else
            QString()
#endif
            , QObject *parent = 0);
    virtual ~QAbstractTest();

    int exec( int argc, char* argv[], char* filename = 0 );
#endif

public slots:
    LearnMode learnMode() const;
    void setLearnMode(LearnMode mode);

    QString currentDataPath() const;
    QString baseDataPath() const;
    void setBaseDataPath(QString const &path);
#ifdef TESTS_SHARED_DIR
    QString sharedDataPath() const;
#endif
    QString currentDataTag() const;
    virtual QString testCaseName() const;
    QString currentTestFunction( bool fullName = false ) const;

#ifndef Q_QDOC
protected:
    virtual int runTest(int argc, char *argv[]);
    virtual void printUsage( int argc, char *argv[] ) const;
    virtual void processCommandLine( int &argc, char *argv[] );
    virtual void setupTestDataPath(const char *filename);
#endif

private:
    QAbstractTestPrivate *d;
};

#endif

#endif
