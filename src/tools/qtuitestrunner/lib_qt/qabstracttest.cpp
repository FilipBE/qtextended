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

#include <qabstracttest.h>
#include <QDir>
#include <QFileInfo>
#include <QMetaMethod>
#include <QThread>
#include <QTimer>

#ifdef Q_OS_UNIX
# include <unistd.h>
# include <time.h>
# include <signal.h>
# include <setjmp.h>
#endif

/*!
    \enum QAbstractTest::LearnMode

    This enum specifies the learn mode setting.

    The learn mode affects the behavior of certain functions in QSystemTest, such as verifyImage().
    Additionally, a test may choose to use the current learn mode to determine how to handle
    a test failure.

    \value LearnNone
        Learn mode is off. Any mismatches encountered will cause a test failure.
    \value LearnNew
        The test should attempt to learn data which does not match existing test data.
    \value LearnAll
        The test should attempt to learn all data, even if it matches existing test data.

    \sa learnMode(), setLearnMode(), verifyImage()
*/

/*  \macro QTEST_MAIN(TestClass)
    \relates QAbstractTest

    Implements a main() function that instantiates a QApplication object and
    the \a TestClass, and executes all tests in the order they were defined.
    Use this macro to build stand-alone executables.

    If \a TestClass is a subclass of QAbstractTest, the runTest() function
    will be called.  Otherwise the object will be passed to \l{QTest::}{qExec()}.

    Example:
    \code
    class TestQString: public QAbstractTest { ... };
    QTEST_MAIN(TestQString)
    \endcode

    \sa {QAbstractTest}{QTEST_APPLESS_MAIN()}, QTEST_APP_MAIN(), QTest::qExec()
*/

/*  \macro QTEST_APPLESS_MAIN(TestClass)
    \relates QAbstractTest

    Implements a main() function that executes all tests in \a TestClass.

    Behaves like QTEST_MAIN(), but doesn't instantiate a QApplication
    object. Use this macro for really simple stand-alone non-GUI tests.

    \sa {QAbstractTest}{QTEST_MAIN()}, QTEST_APP_MAIN(), QTest::qExec()
*/

/*  \macro QTEST_APP_MAIN(TestClass,TestApplication)
    \relates QAbstractTest

    Implements a main() function that executes all tests in \a TestClass,
    using \a TestApplication as the application class.

    Behaves like QTEST_MAIN(), but allows you to specify which application
    class should be used.  Use this macro when a test depends on the behaviour
    of a specific application type.

    Example:
    \code
    class TestQDogWalker: public QObject { ... };
    QTEST_APP_MAIN(TestQDogWalker,QtopiaApplication)
    \endcode

    \sa {QAbstractTest}{QTEST_MAIN()}, {QAbstractTest}{QTEST_APPLESS_MAIN()}, QTest::qExec()
*/

bool Autotest_QLog::m_enabled = false;

QDebug Autotest_QLog::log(const char* category)
{
    QDebug r = QDebug(QtDebugMsg);
    if ( category )
        r << category << ": ";
    return r;
}

class FatalTimeoutThread : public QThread
{
Q_OBJECT
public:
    FatalTimeoutThread();
    bool inEventLoop() const     { return m_inEventLoop; }
    void setTimeout(int timeout) { m_timeout = timeout; }

    void startTimer();
    void stopTimer();
    void stopThread();

protected:
    void run();

private slots:
    void enteredEventLoop();
    void _startTimer();
    void _stopTimer();
    void _stopThread();

private:
    enum ExitCodes { TimedOut = 0, DidNotTimeOut, StartTimer, Exit };
    int m_timeout;
    bool m_inEventLoop;
};

class QAbstractTestPrivate
{
public:
    QAbstractTest::LearnMode learnMode;
    QString                  baseDataPath;
};

static QString noslash(QString const &in)
{
    QString out(in);
    while (out.endsWith("/")) out.chop(1);
    return out;
}

/*
    \internal
    \class QAbstractTest
    \inpublicgroup QtUiTestExtension
    \mainclass
    \brief The QAbstractTest class is the base class for all QtUiTest System Tests.

    \ingroup qtuitest_unittest
    \ingroup qtuitest_systemtest
    \internal

    QAbstractTest provides some functionality helpful for all kinds of tests.

    In practice, a test class will almost always subclass one of QtopiaUnitTest or QSystemTest.

    \sa QSystemTest, QtUiTest, QTestLib
*/

/*
    \internal
    Construct the test with the specified \a parent.
*/
QAbstractTest::QAbstractTest(QString const &srcdir, QObject *parent)
 : QTimedTest(parent)
{
    d = new QAbstractTestPrivate;
    d->baseDataPath = noslash(QDir::homePath()) + "/.qtest";
    d->learnMode = LearnNone;
    if (!srcdir.isEmpty())
        setupTestDataPath(qPrintable(srcdir + "/tst_phonytest.cpp"));
}

/*
    \internal
*/
QAbstractTest::~QAbstractTest()
{
    delete d;
}

/*!
    Returns the current learn mode.
*/
QAbstractTest::LearnMode QAbstractTest::learnMode() const
{
    return d->learnMode;
}

/*!
    Sets the current learn \a mode.
*/
void QAbstractTest::setLearnMode(QAbstractTest::LearnMode mode)
{
    d->learnMode = mode;
}

/*!
    Returns the path in which data for the current test function will be saved to/retrieved from.

    The path follows the format \c baseDataPath/testCaseName/testFunctionName/dataTag,
    where \c baseDataPath is the path returned by \l baseDataPath().

    Internally, the current data path does not affect the behaviour of QAbstractTest.
    However, subclasses may use the value returned by this function to decide where to
    store learned data.

    \sa baseDataPath(), learnMode()
*/
QString QAbstractTest::currentDataPath() const
{
    return d->baseDataPath + "/" + currentTestFunction() + "/" + currentDataTag();
}


/*!
    Returns the path which functions as the root directory to all the test data.

    This path defaults to the "testdata" subdirectory of the directory in which the source file of
    the current test is located; if the source file no longer exists, the path defaults to
    \c $HOME/.qtest/ .  In either case, it can be overridden by the user with the \c -data command line switch.

    The base data path contains the test data for functions such as verifyImage().
    It can also be used within a test to use testdata stored in files.

    \sa currentDataPath(), learnMode()
*/
QString QAbstractTest::baseDataPath() const
{
    return d->baseDataPath + "/";
}

/*!
    Set the \a path which functions as the root directory to all of the test data.

    The base data path contains the test data for functions such as verifyImage().
    It can also be used within a test to use testdata stored in files.

    \sa baseDataPath()
*/
void QAbstractTest::setBaseDataPath(QString const &path)
{
    d->baseDataPath = path;
}

#ifdef TESTS_SHARED_DIR
/*!
    Returns the path which functions as the shared directory available to tests.

    The shared data path contains scripts and other functions available to tests.  This is the default search
    location for the \c include feature.

*/
QString QAbstractTest::sharedDataPath() const
{
    return noslash(TESTS_SHARED_DIR);
}
#endif
/*!
    Returns the current data tag, or an empty string if the test function has no test data associated
    with it. The data tag is the string used to identify each row in the test data table.

    Example:
    \code
        walkTheDog: function(street) {
            // At the moment, Qt Extended crashes if we walk on concrete; this is
            // a known failure
            if (currentDataTag().startsWith("concrete sidewalk"))
                expectFail("concrete is known to break");

            enter( street, "Destination" );
            select( "Walk the Dog" );
            compare( getText("Status"), "Walked" );
        }
        walkTheDog_data: {
            "concrete sidewalk 1": [ "East St." ],
            "concrete sidewalk 2": [ "West St." ],
            grassy:                [ "North St." ],
            muddy:                 [ "South St." ]
        }
    \endcode
*/
QString QAbstractTest::currentDataTag() const
{
    return QTest::currentDataTag();
}

/*!
    Returns the name of the currently executing test case.

    \sa currentTestFunction()
*/
QString QAbstractTest::testCaseName() const
{
    return metaObject()->className();
}

/*!
    Returns the name of the testfunction that is currently being executed.
    If \a fullName is true, the fully qualified name (including the test case name) will be returned.

    Example, in a file named sys_mytest:
    \code
    testYoyo: function() {
        currentTestFunction(); // returns "testYoyo"
        currentTestFunction(true);  // returns "sys_mytest::testYoyo"
    }
    \endcode

    \sa testCaseName()
*/
QString QAbstractTest::currentTestFunction( bool fullName ) const
{
    return fullName ? (testCaseName() + "::" + QTest::currentTestFunction()) : (QTest::currentTestFunction());
}

/*
    \internal
    Executes all test functions as specified on the command line, while running the
    application event loop.

    The \a argc and \a argv parameters should be passed in from the main()
    function of the application and are used to parse command-line arguments
    specific to QAbstractTest.  Also, subclasses may override the runTest(),

    The \a filename parameter contains the full path to the source file containing the
    test in question.  It is used to determine where test data should be located.

    Generally the QTEST_MAIN() macro would be used rather than this function.

    \sa {QAbstractTest}{QTEST_MAIN()}
*/
int QAbstractTest::exec( int argc, char* argv[], char* filename )
{
    setupTestDataPath(filename);
    int _argc(argc);
    char **_argv = new char*[argc+1];
    for (int i = 0; i < argc; ++i) _argv[i] = argv[i];
    processCommandLine(_argc, _argv);
    int ret = runTest(_argc, _argv);
    delete[] _argv;
    return ret;
}

/*
    \internal
    Print a usage message.
    \a argc and \a argv are command-line arguments.
    Any subclass which overrides processCommandLine() should also override this
    function and provide documentation for their arguments.
*/
void QAbstractTest::printUsage( int argc, char* argv[] ) const
{
    qWarning( " Usage:\n"
        "    %s [options] [testfunction[:datatag]]...\n"
        "\n"
        "    'testfunctions' is a list of functions to execute (separated by spaces).\n"
        "    If no testfunctions are specified, ALL functions will be executed.\n"
        "\n"
        "  basic test options:\n"
        "    -functions        : Returns a list of current testfunctions\n"
        "    -xml              : Outputs results as XML document\n"
        "    -lightxml         : Outputs results as stream of XML tags\n"
        "    -o filename       : Writes all output into a file\n"
        "    -v                : Print 'Autotest' log messages\n"
        "    -v1               : Also print enter messages for each testfunction\n"
        "    -v2               : Also print out each QVERIFY/QCOMPARE/QTEST\n"
        "    -vs               : Print every signal emitted\n"
        "    -timed            : Print time elapsed (in milliseconds) for each testfunction\n"
        "    -maxtime ms       : Maximum time allowed per test, in milliseconds (implies -timed).\n"
        "                        If this time is reached, a fatal error occurs and subsequent tests will not run.\n"
        "    -eventdelay ms    : Set default delay for mouse and keyboard simulation to ms milliseconds\n"
        "    -keydelay ms      : Set default delay for keyboard simulation to ms milliseconds\n"
        "    -mousedelay ms    : Set default delay for mouse simulation to ms milliseconds\n"
        "    -keyevent-verbose : Turn on verbose messages for keyboard simulation\n"
        "    -maxwarnings n    : Sets the maximum amount of messages to output.\n"
        "                        0 means unlimited, default: 2000\n"
        "    -help             : This help\n"
        "\n"
        "  learn mode options:\n"
        "    -learn            : All 'learnable' new and changed data may be added/updated.\n"
        "    -learn-all        : All 'learnable' data may be added/updated, even if it has not changed.\n"
        "    -data d           : The location of the testdata; this value is used for dynamic data loading (i.e. dynamic\n"
        "                        data that can't be hardcoded in the testcase itself).\n"
        "                        If -data is not specified the testcase will first look in the testdata subdirectory\n"
        "                        of the directory containing the test source file, then in $HOME/.qtest\n"
        , (argc) ? argv[0] : "test");
}

/*
    \internal
    Processes the command line parameters specified by \a argc, \a argv.
    Subclasses may reimplement this function to handle specific arguments.
*/
void QAbstractTest::processCommandLine( int &argc, char* argv[] )
{
    int offset = 0;
    for (int i = 1; i < argc; ++i) {
        if ( !strcasecmp(argv[i], "-data") ) {
            argv[i] = 0;
            offset += 2;
            ++i;
            if ( i < argc ) {
                setBaseDataPath(QString(argv[i]));
                argv[i] = 0;
            }

        } else if ( !strcasecmp(argv[i], "-v") ||
                    !strcasecmp(argv[i], "-v1") ||
                    !strcasecmp(argv[i], "-v2") ) {
            // Don't consume -v1 or -v2
            if (!strcasecmp(argv[i], "-v")) {
                argv[i] = 0;
                offset++;
            }
            Autotest_QLog::m_enabled = true;

        } else if ( !strcasecmp(argv[i], "-learn") ) {
            argv[i] = 0;
            offset++;
            setLearnMode(LearnNew);

        } else if ( !strcasecmp(argv[i], "-learn-all") ) {
            argv[i] = 0;
            offset++;
            setLearnMode(LearnAll);

        } else if ( !strcasecmp(argv[i], "-timed") ) {
            argv[i] = 0;
            offset++;
            QTimedTest::pass_through = false;
            QTimedTest::print_times = true;

        } else if ( !strcasecmp(argv[i], "-maxtime") ) {
            argv[i] = 0;
            offset += 2;
            ++i;
            if ( i < argc ) {
                QTimedTest::pass_through = false;
                bool ok;
                timeout_thread = new FatalTimeoutThread;
                timeout_thread->setTimeout(QString::fromAscii(argv[i]).toInt(&ok));
                if (!ok) qFatal("Invalid parameter to -maxtime");
                argv[i] = 0;
            }

        } else if ( !strcasecmp(argv[i], "-help") ||
                    !strcasecmp(argv[i], "--help") ||
                    !strcasecmp(argv[i], "-h") ) {
            argv[i] = 0;
            offset++;
            printUsage(argc-offset, argv);
            exit(0);

        // Silently ignore a few system test specific arguments.
        // For compatibility, we'll silently ignore these so that
        // the same arguments can be specified for unit and system tests.
        } else if ( !strcasecmp(argv[i], "-autip")
                 || !strcasecmp(argv[i], "-authost")
                 || !strcasecmp(argv[i], "-autport")
                 || !strcasecmp(argv[i], "-remote") ) {
            argv[i] = 0;
            offset += 2;
            ++i;
            argv[i] = 0;

        } else if ( !strcasecmp(argv[i], "-keepaut")
                 || !strcasecmp(argv[i], "-silentaut")
                 || !strcasecmp(argv[i], "-auto")
                 || !strcasecmp(argv[i], "-demomode") ) {
            argv[i] = 0;
            offset++;

        } else {
            if (offset > 0) {
                argv[i-offset] = argv[i];
                argv[i] = 0;
            }
        }
    }
    argc-=offset;
}

#ifdef Q_OS_UNIX
jmp_buf segfault_jmp;

void handle_segfault(int signum)
{
    signal(signum, SIG_DFL);
    QTest::qFail("A segmentation fault occurred.", "Unknown file", 0);
    longjmp(segfault_jmp, 1);
    _exit(0);
}
#endif

/*
    \internal
    Run test with arguments \a argc, \a argv, and return an exit code.
    The base implementation executes private slots as testfunctions
    using QTest::qExec().  Subclasses may reimplement this function to provide
    other behaviour.
*/
int QAbstractTest::runTest(int argc, char *argv[])
{
#ifdef Q_OS_UNIX
    signal(SIGSEGV, handle_segfault);
    if (!setjmp(segfault_jmp))
#endif
        return QTest::qExec( this, argc, argv );
    return -1;
}

/*!
    \internal
    Set up the test data path, where \a filename is the name of the test source file.
*/
void QAbstractTest::setupTestDataPath(const char *filename)
{
    d->baseDataPath = noslash(QDir::homePath()) + "/.qtest/" + testCaseName();

    if (filename) {
        // If we are given a filename, try to use it as test path
        QFileInfo sourceFile( filename );
        QString sfPath = sourceFile.canonicalPath();
        QString dirPath = sourceFile.dir().canonicalPath();
        QString path = sfPath.isEmpty() ? dirPath : sfPath;
        if (!path.isEmpty()) {
            d->baseDataPath = noslash(path) + "/testdata";
        }
    }

    // Try to ensure the data directory exists
    QDir dir;
    dir.mkpath(d->baseDataPath);
}

class Timer
{
public:
    virtual ~Timer() {}
    virtual void start() = 0;
    virtual int elapsed() const = 0;
};

class RealTimeTimer : public Timer
{
public:
    void start()        { t.start(); }
    int elapsed() const { return t.elapsed(); }
private:
    QTime t;
};

#ifdef Q_OS_UNIX
class ProcessorTimeTimer : public Timer
{
public:
    void start()        { s = clock(); }
    int elapsed() const { return (int)(((double)(clock() - s))/((double)CLOCKS_PER_SEC)*1000.0); }
private:
    clock_t s;
};
#endif


QTimedTest::QTimedTest(QObject *parent)
 : QObject(parent), pass_through(true), print_times(false),
   timeout_thread(0), realTimer(0), cpuTimer(0)
{}

QTimedTest::~QTimedTest()
{
    if (timeout_thread) {
        timeout_thread->stopThread();
        timeout_thread->wait();
        delete timeout_thread;
    }
    delete realTimer;
    delete cpuTimer;
}

static bool isTimeableSlot(const QMetaMethod &sl)
{
    if (sl.access() != QMetaMethod::Private || !sl.parameterTypes().isEmpty()
        || qstrlen(sl.typeName()) || sl.methodType() != QMetaMethod::Slot)
        return false;
    const char *sig = sl.signature();
    int len = qstrlen(sig);
    if (len < 2)
        return false;
    if (sig[len - 2] != '(' || sig[len - 1] != ')')
        return false;
    if (len > 7 && strcmp(sig + (len - 7), "_data()") == 0)
        return false;
/*
    It makes sense to time init() etc also.
    if (strcmp(sig, "initTestCase()") == 0 || strcmp(sig, "cleanupTestCase()") == 0
        || strcmp(sig, "cleanup()") == 0 || strcmp(sig, "init()") == 0)
        return false;
*/

    return true;
}

FatalTimeoutThread::FatalTimeoutThread() : QThread()
{
    m_timeout = 0;
    m_inEventLoop = false;
}

void FatalTimeoutThread::enteredEventLoop()
{
    m_inEventLoop = true;
}

void FatalTimeoutThread::startTimer()
{
    QMetaObject::invokeMethod(this, "_startTimer");
}

void FatalTimeoutThread::stopTimer()
{
    QMetaObject::invokeMethod(this, "_stopTimer");
}

void FatalTimeoutThread::stopThread()
{
    QMetaObject::invokeMethod(this, "_stopThread");
}

void FatalTimeoutThread::_startTimer()
{
    exit(StartTimer);
}

void FatalTimeoutThread::_stopTimer()
{
    exit(DidNotTimeOut);
}

void FatalTimeoutThread::_stopThread()
{
    exit(Exit);
}

void FatalTimeoutThread::run()
{
    QTimer t;
    connect(&t, SIGNAL(timeout()), this, SLOT(quit()));
    QTimer::singleShot(0, this, SLOT(enteredEventLoop()));

    int code;
    do {
        code = exec();
        switch (code) {
            case TimedOut:
                QTest::qFail(qPrintable(QString("Test did not finish within the allowed %1 ms").arg(m_timeout)), "Unknown file", 0);
#ifdef Q_OS_UNIX
                longjmp(segfault_jmp, 1);
#endif
                _exit(0);
            case DidNotTimeOut:
                t.stop();
                break;
            case StartTimer:
                t.setInterval(m_timeout);
                t.start();
                break;
            default: break;
        }
    } while(code != Exit);
}

int QTimedTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    if (!realTimer) {
        have_init    = isTimeableSlot(metaObject()->method(metaObject()->indexOfMethod("init()")));
        have_cleanup = isTimeableSlot(metaObject()->method(metaObject()->indexOfMethod("cleanup()")));
        realTimer = new RealTimeTimer;
#ifdef Q_OS_UNIX
        cpuTimer = new ProcessorTimeTimer;
#endif
    }

    if (pass_through)
        return real_qt_metacall(_c, _id, _a);

    bool timed      = isTimeableSlot(metaObject()->method(_id));
    bool is_init    = (_id == metaObject()->indexOfMethod("init()"));
    bool is_cleanup = (_id == metaObject()->indexOfMethod("cleanup()"));
    bool is_single  = (_id == metaObject()->indexOfMethod("initTestCase()") ||
                       _id == metaObject()->indexOfMethod("cleanupTestCase()"));

    bool should_start_timer = timed && (is_single || is_init || (!have_init && !is_cleanup));
    bool should_stop_timer  = timed && (is_single || (!is_init && (!have_cleanup || is_cleanup)));

    if (should_start_timer) {
        if (timeout_thread) {
            if (timeout_thread != timeout_thread->thread()) {
                timeout_thread->moveToThread(timeout_thread);
                timeout_thread->start();
                while (!timeout_thread->inEventLoop()) {
                    timeout_thread->wait(50);
                }
            }
            timeout_thread->startTimer();
        }
        realTimer->start();
        if (cpuTimer) cpuTimer->start();
    }

    pass_through = true;
    int ret = this->qt_metacall(_c, _id, _a);
    pass_through = false;

    if (should_stop_timer) {
        if (print_times) QDebug(QtDebugMsg) <<
          qPrintable(
            QString("Test time: %1 ms CPU, %2 ms real")
            .arg(cpuTimer ? cpuTimer->elapsed() : -1)
            .arg(realTimer->elapsed())
          );
        if (timeout_thread) timeout_thread->stopTimer();
    }

    return ret;
}

static const uint qt_meta_data_QTimedTest[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_QTimedTest[] = {
    "QTimedTest\0"
};

const QMetaObject QTimedTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QTimedTest,
      qt_meta_data_QTimedTest, 0 }
};

const QMetaObject *QTimedTest::metaObject() const
{
    return &staticMetaObject;
}

void *QTimedTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QTimedTest))
        return static_cast<void*>(const_cast< QTimedTest*>(this));
    return QObject::qt_metacast(_clname);
}

int QTimedTest::real_qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

#include "qabstracttest.moc"

