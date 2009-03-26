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

#include <QAction>
#include <QApplication>
#include <QObject>
#include <QTest>
#include <shared/util.h>

#include <qalternatestack_p.h>
#include <qtuitestconnectionmanager_p.h>
#include <qtuitestnamespace.h>

#include <QTime>
#include <QTimer>

class tst_QtUiTestNamespace : public QObject
{
    Q_OBJECT

private slots:
    void wait();
    void connectFirst();
    void connectFirstIsFirst();
    void connectFirstDestruction();
    void connectFirstInvalid();
    void connectFirstDefaultParam();
    void disconnectFirstInvalid();
    void disconnectFirstWildcards();
};

int      g_exit_code;
int      g_argc;
char**   g_argv;
bool     g_test_done;
QObject* g_test;

void run_test(QAlternateStack*,QVariant const&)
{
    g_exit_code = QTest::qExec(g_test, g_argc, g_argv);
    g_test_done = true;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    tst_QtUiTestNamespace test;
    g_test = &test;
    g_argc = argc;
    g_argv = argv;

    // Most QtUiTest code runs in an alternate stack, while the original
    // stack runs the event loop.
    // Try to replicate that behavior for this test.
    QAlternateStack stack;

    g_test_done = false;
    stack.start(run_test);
    while (!g_test_done) {
        app.processEvents(QEventLoop::WaitForMoreEvents);
    }
    return g_exit_code;
}

/*
    Event loop where 'exec' is a slot, for convenience.
*/
class TestEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    TestEventLoop(QObject* parent =0)
        :   QEventLoop(parent),
            m_execCount(0),
            m_exitCount(0)
    {}

    int m_execCount;
    int m_exitCount;

public slots:
    int exec(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
    {
        ++m_execCount;
        int ret = QEventLoop::exec(flags);
        ++m_exitCount;
        return ret;
    }
};

/*
    Class for emitting a particular integer value.
*/
class Emitter : public QObject
{
    Q_OBJECT

public:
    Emitter(QObject* parent =0)
        : QObject(parent)
    {}

    void emitValue(int value)
    {
        emit valueChanged(value);
        emit valueChanged2(value);
        emit valueChangedWithDefault(value);
    }

    enum Values { DefaultValue = 1338 };

signals:
    void valueChanged(int);
    void valueChanged2(int);
    void valueChangedWithDefault(int = DefaultValue);
};

/*
    Class for receiving an emitted value.
*/
class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver(QObject* parent =0)
        : QObject(parent)
    {}

    QList<int> values;
    QList<int> values2;

    enum Values { DefaultValue = 1337 };

public slots:
    void receive(int value)
    { values << value; }

    void receive2(int value)
    { values2 << value; }

    void receiveMultipliedByTwo(int value)
    { values << 2*value; }

    void receiveDefault(int value = DefaultValue)
    { values << value; }

    void clear()
    { values.clear(); }

    void clear2()
    { values2.clear(); }
};

// Verify that \a actual is bounded by \a min and \a max.
#define QCOMPARE_BOUNDED(actual, min, max)                           \
    do {                                                             \
        QVERIFY2(actual >= min && actual <= max, qPrintable(QString( \
            "actual %1, expected to be in range %2 .. %3"            \
        ).arg(actual).arg(min).arg(max)));                           \
    } while(0)

/*
    The most important test ever written.

    Verify that we can wait() for an arbitrary amount of time, while processing
    events, without hanging if a nested event loop occurs.

    See bug 194361.
*/
void tst_QtUiTestNamespace::wait()
{
    {
        QTime t;
        t.start();

        QtUiTest::wait(1000);

        QCOMPARE_BOUNDED(t.elapsed(), 1000, 30000);
    }

    // OK, so we can wait when there are no nested event loops.
    // Big deal.  The real test is:  if we have a nested event loop, can we
    // avoid hanging.
    {
        TestEventLoop loop;

        QTime t;
        t.start();

        // The nested event loop will run for 2.5 seconds.
        // But we only want to wait for 1 second.
        // What on earth will happen... ???
        QTimer::singleShot(0,    &loop, SLOT(exec()));
        QTimer::singleShot(2500, &loop, SLOT(quit()));
        QtUiTest::wait(1000);

        // Verify the loop really did exec.
        QCOMPARE(loop.m_execCount, 1);
        // Verify the loop really hasn't exited yet.
        QCOMPARE(loop.m_exitCount, 0);
        // Verify that we've waited for about as long as we wanted to wait.
        QCOMPARE_BOUNDED(t.elapsed(), 1000, 30000);

        // OK, now check inner loop really does exit, to ensure we haven't
        // screwed things up by switching stacks.
        for (int i = 0; i < 5000 && !loop.m_exitCount; i+=100, QtUiTest::wait(100))
        {}
        QCOMPARE(loop.m_exitCount, 1);
        QCOMPARE_BOUNDED(t.elapsed(), 2500, 30000);
    }
}

/*
    Tests QtUiTest::connectFirst(), which works like QObject::connect but guarantees
    that the connection comes before all existing connections.
*/
void tst_QtUiTestNamespace::connectFirst()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

    {
        // Basic connection
        Emitter  e;
        Receiver r;

        QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));
        QCOMPARE(cm->m_connections.count(), 1);

        e.emitValue(1024);
        e.emitValue(1025);
        QCOMPARE(r.values, QList<int>() << 1024 << 1025);
        r.values.clear();

        // Basic disconnection
        QVERIFY(QtUiTest::disconnectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));
        QCOMPARE(cm->m_connections.count(), 0);

        e.emitValue(1027);
        QCOMPARE(r.values, QList<int>());
    }

    {
        // Multiply-connected
        Emitter e1, e2;
        Receiver r1, r2;

        QVERIFY(QtUiTest::connectFirst(&e1, SIGNAL(valueChanged(int)), &r1, SLOT(receive(int))));
        QCOMPARE(cm->m_connections.count(), 1);

        QVERIFY(QtUiTest::connectFirst(&e1, SIGNAL(valueChanged(int)), &r1, SLOT(receive(int))));
        QCOMPARE(cm->m_connections.count(), 2);

        QVERIFY(QtUiTest::connectFirst(&e1, SIGNAL(valueChanged(int)), &r1, SLOT(receive(int))));
        QCOMPARE(cm->m_connections.count(), 3);

        // Connect to another object to make sure it doesn't just happen to work when all
        // connections are identical.
        QVERIFY(QtUiTest::connectFirst(&e2, SIGNAL(valueChanged2(int)), &r2, SLOT(receive2(int))));
        QCOMPARE(cm->m_connections.count(), 4);

        e1.emitValue(128);
        e1.emitValue(129);
        QCOMPARE(r1.values,  QList<int>() << 128 << 128 << 128 << 129 << 129 << 129);
        QCOMPARE(r1.values2, QList<int>());
        QCOMPARE(r2.values,  QList<int>());
        QCOMPARE(r2.values2, QList<int>());
        r1.values.clear();

        e2.emitValue(130);
        e2.emitValue(150);
        QCOMPARE(r1.values,  QList<int>());
        QCOMPARE(r1.values2, QList<int>());
        QCOMPARE(r2.values,  QList<int>());
        QCOMPARE(r2.values2, QList<int>() << 130 << 150);
        r2.values2.clear();

        // Disconnecting something which is multiply-connected
        QVERIFY(QtUiTest::disconnectFirst(&e1, SIGNAL(valueChanged(int)), &r1, SLOT(receive(int))));
        QCOMPARE(cm->m_connections.count(), 1);

        e1.emitValue(1280);
        e1.emitValue(1290);
        QCOMPARE(r1.values,  QList<int>());
        QCOMPARE(r1.values2, QList<int>());
        QCOMPARE(r2.values,  QList<int>());
        QCOMPARE(r2.values2, QList<int>());
        r1.values.clear();

        e2.emitValue(1300);
        e2.emitValue(1500);
        QCOMPARE(r1.values,  QList<int>());
        QCOMPARE(r1.values2, QList<int>());
        QCOMPARE(r2.values,  QList<int>());
        QCOMPARE(r2.values2, QList<int>() << 1300 << 1500);
        r2.values2.clear();

        QVERIFY(QtUiTest::disconnectFirst(&e2, SIGNAL(valueChanged2(int)), &r2, SLOT(receive2(int))));
        QCOMPARE(cm->m_connections.count(), 0);

        e2.emitValue(280);
        e2.emitValue(290);
        QCOMPARE(r1.values,  QList<int>());
        QCOMPARE(r1.values2, QList<int>());
        QCOMPARE(r2.values,  QList<int>());
        QCOMPARE(r2.values2, QList<int>());

    }

    {
        // Large number of connections
        QList<Emitter*>  emitters;
        QList<Receiver*> receivers;

        static const int LARGE = 1000;
        while (emitters.count() < LARGE) {
            emitters << new Emitter;
            receivers << new Receiver;
        }

        for (int i = 0; i < LARGE; ++i) {
            QVERIFY(QtUiTest::connectFirst(emitters.at(i), SIGNAL(valueChanged(int)), receivers.at(i), SLOT(receive(int))));
            QVERIFY(QtUiTest::connectFirst(emitters.at(i), SIGNAL(valueChanged(int)), receivers.at(i), SLOT(receive(int))));
            QVERIFY(QtUiTest::connectFirst(emitters.at(i), SIGNAL(valueChanged2(int)), receivers.at(LARGE-i-1), SLOT(receive2(int))));
            QVERIFY(QtUiTest::connectFirst(emitters.at(i), SIGNAL(valueChanged2(int)), receivers.at(LARGE-i-1), SLOT(receive2(int))));
            QCOMPARE(cm->m_connections.count(), (i+1)*4);
        }

        for (int i = 0; i < LARGE; ++i) {
            emitters.at(i)->emitValue(i);
        }

        for (int i = 0; i < LARGE; ++i) {
            // receiver at i should have received (twice):
            //  value: i
            //  value2: LARGE-i-1
            QCOMPARE( receivers.at(i)->values,  QList<int>() << i << i );
            QCOMPARE( receivers.at(i)->values2, QList<int>() << (LARGE-i-1) << (LARGE-i-1) );
            receivers.at(i)->values.clear();
            receivers.at(i)->values2.clear();
        }

        // Disconnect half of the objects.
        for (int i = 0; i < LARGE; i += 2) {
            QVERIFY(QtUiTest::disconnectFirst(emitters.at(i), SIGNAL(valueChanged(int)), 0, 0));
            QVERIFY(QtUiTest::disconnectFirst(emitters.at(i), SIGNAL(valueChanged2(int)), 0, 0));
        }
        QCOMPARE(cm->m_connections.count(), LARGE*2);

        for (int i = 0; i < LARGE; ++i) {
            emitters.at(i)->emitValue(i);
        }

        for (int i = 0; i < LARGE; ++i) {
            if (i % 2) {
                QCOMPARE( receivers.at(i)->values,  QList<int>() << i << i);
                receivers.at(i)->values.clear();
            } else {
                QCOMPARE( receivers.at(i)->values,  QList<int>() );
            }

            if (!(i % 2)) {
                QCOMPARE( receivers.at(i)->values2, QList<int>() << (LARGE-i-1) << (LARGE-i-1));
                receivers.at(i)->values2.clear();
            } else {
                QCOMPARE( receivers.at(i)->values2, QList<int>() );
            }
        }

        // Delete half of the objects.
        for (int i = 0; i < LARGE/2; ++i) {
            delete receivers.takeLast();
            delete emitters.takeLast();
        }
        // All the `2' connections have been severed
        QCOMPARE(cm->m_connections.count(), LARGE/2);

        for (int i = 0; i < LARGE/2; ++i) {
            emitters.at(i)->emitValue(i);
        }

        for (int i = 0; i < LARGE/2; ++i) {
            if (i % 2) {
                QCOMPARE( receivers.at(i)->values,  QList<int>() << i << i);
                receivers.at(i)->values.clear();
            } else {
                QCOMPARE( receivers.at(i)->values,  QList<int>() );
            }
            QCOMPARE( receivers.at(i)->values2, QList<int>() );
        }

        // Delete the rest.
        while (receivers.count()) {
            delete receivers.takeFirst();
            delete emitters.takeFirst();
        }
        QCOMPARE(cm->m_connections.count(), 0);
    }
}


void tst_QtUiTestNamespace::connectFirstIsFirst()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

    // Tests that connectFirst really does what its name says: establishes a connection which is
    // activated _before_ all QObject::connect connections.
    Emitter  e;
    Receiver r;

    static const int LARGE = 200;

    QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));

    for (int i = 0; i < LARGE; ++i)
        QVERIFY(QObject::connect  (&e, SIGNAL(valueChanged(int)), &r, SLOT(receiveMultipliedByTwo(int))));

    QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));

    for (int i = 0; i < LARGE; ++i)
        QVERIFY(QObject::connect (&e, SIGNAL(valueChanged(int)), &r, SLOT(receiveMultipliedByTwo(int))));

    QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));

    e.emitValue(1);
    QCOMPARE(r.values.count(), LARGE*2+3);

    // Verify that the QtUiTest-connected signals came first
    for (int i = 0; i < 3; ++i)
        QCOMPARE(r.values.at(i), 1);
    for (int i = 0; i < LARGE*2; ++i)
        QCOMPARE(r.values.at(i+3), 2);
}

void tst_QtUiTestNamespace::connectFirstDestruction()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

    {
        // Verify connections are destroyed when sender is destroyed
        Receiver r;
        {
            Emitter e;

            QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));
            QCOMPARE(cm->m_connections.count(), 1);
        }
        QCOMPARE(cm->m_connections.count(), 0);
    }

    {
        // Verify connections are destroyed when receiver is destroyed
        Emitter e;
        {
            Receiver r;

            QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));
            QCOMPARE(cm->m_connections.count(), 1);
        }
        QCOMPARE(cm->m_connections.count(), 0);
    }
}

void tst_QtUiTestNamespace::connectFirstInvalid()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

    Emitter e;
    Receiver r;

    QTest::ignoreMessage(QtWarningMsg, "QObject::connect: Cannot connect (null)::(null) to (null)::(null)");
    QVERIFY(!QtUiTest::connectFirst(0, 0, 0, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "QObject::connect: Cannot connect Emitter::(null) to (null)::(null)");
    QVERIFY(!QtUiTest::connectFirst(&e, 0, 0, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "QObject::connect: Cannot connect Emitter::(null) to Receiver::(null)");
    QVERIFY(!QtUiTest::connectFirst(&e, 0, &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "QObject::connect: Cannot connect (null)::(null) to Receiver::(null)");
    QVERIFY(!QtUiTest::connectFirst(0, 0, &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "QObject::connect: Cannot connect Emitter::notexist() to Receiver::(null)");
    QVERIFY(!QtUiTest::connectFirst(&e, SIGNAL(notexist()), &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "QObject::connect: Cannot connect Emitter::valueChanged(int) to Receiver::(null)");
    QVERIFY(!QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "Object::connect: No such slot Receiver::notexist()");
    QVERIFY(!QtUiTest::connectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(notexist())));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "Object::connect: No such signal Emitter::notexist()");
    QVERIFY(!QtUiTest::connectFirst(&e, SIGNAL(notexist()), &r, SLOT(receive(int))));
    QVERIFY(!cm->m_connections.count());
}

void tst_QtUiTestNamespace::disconnectFirstInvalid()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

    Emitter e;
    Receiver r;

    QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: Unexpected null parameter");
    QVERIFY(!QtUiTest::disconnectFirst(0, 0, 0, 0));
    QVERIFY(!cm->m_connections.count());

    // Not an error but there are no connections
    QVERIFY(!QtUiTest::disconnectFirst(&e, 0, 0, 0));
    QVERIFY(!cm->m_connections.count());

    // Not an error but there are no connections
    QVERIFY(!QtUiTest::disconnectFirst(&e, 0, &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: Unexpected null parameter");
    QVERIFY(!QtUiTest::disconnectFirst(0, 0, &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: No such signal Emitter::notexist()");
    QVERIFY(!QtUiTest::disconnectFirst(&e, SIGNAL(notexist()), &r, 0));
    QVERIFY(!cm->m_connections.count());

    // Not an error but there are no connections
    QVERIFY(!QtUiTest::disconnectFirst(&e, SIGNAL(valueChanged(int)), &r, 0));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: No such slot Receiver::notexist()");
    QVERIFY(!QtUiTest::disconnectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(notexist())));
    QVERIFY(!cm->m_connections.count());

    QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: No such signal Emitter::notexist2()");
    QVERIFY(!QtUiTest::disconnectFirst(&e, SIGNAL(notexist2()), &r, SLOT(receive(int))));
    QVERIFY(!cm->m_connections.count());

    // Not an error but there are no connections
    QVERIFY(!QtUiTest::disconnectFirst(&e, SIGNAL(valueChanged(int)), &r, SLOT(receive(int))));
    QVERIFY(!cm->m_connections.count());
}

/*
    Tests QtUiTest::connectFirst() for signals/slots which have default parameters.
*/
void tst_QtUiTestNamespace::connectFirstDefaultParam()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

    {   // x,x
        Emitter e;
        Receiver r;

        QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChangedWithDefault(int)), &r, SLOT(receiveDefault(int))));
        QCOMPARE(cm->m_connections.count(), 1);
        e.emitValue(-99);
        QCOMPARE(r.values, QList<int>() << -99);
    }

    {   // x,0
        Emitter e;
        Receiver r;

        QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChangedWithDefault(int)), &r, SLOT(receiveDefault())));
        QCOMPARE(cm->m_connections.count(), 1);
        e.emitValue(129);
        QCOMPARE(r.values, QList<int>() << r.DefaultValue);
    }

    {   // 0,x: can't work
        Emitter e;
        Receiver r;

        QTest::ignoreMessage(QtWarningMsg,
            "QObject::connect: Incompatible sender/receiver arguments"
            "\n\tEmitter::valueChangedWithDefault() --> Receiver::receiveDefault(int)");
        QVERIFY(!QtUiTest::connectFirst(&e, SIGNAL(valueChangedWithDefault()), &r, SLOT(receiveDefault(int))));
        QCOMPARE(cm->m_connections.count(), 0);
    }

    {   // 0,0
        Emitter e;
        Receiver r;

        QVERIFY(QtUiTest::connectFirst(&e, SIGNAL(valueChangedWithDefault()), &r, SLOT(receiveDefault())));
        QCOMPARE(cm->m_connections.count(), 1);
        e.emitValue(130);

        QEXPECT_FAIL("", "Bug 227908: signals with default parameters are discarded by QtUiTest::connectFirst()", Abort);

        QCOMPARE(r.values.count(), 1);
        QCOMPARE(r.values, QList<int>() << r.DefaultValue);
    }
}

void tst_QtUiTestNamespace::disconnectFirstWildcards()
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();
    if (cm->m_connections.count())
        QSKIP("Test cannot proceed; previous test left connection manager in incorrect state", SkipAll);

#define INIT() \
    Receiver r1;  Emitter e1;   \
    Receiver r2;  Emitter e2a;  Emitter e2b;    \
    Receiver r3a; Receiver r3b; Emitter e3;     \
    Receiver r4a; Receiver r4b; Emitter e4;     \
\
    QVERIFY(QtUiTest::connectFirst(&e1,  SIGNAL(valueChanged(int)),  &r1,  SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e1,  SIGNAL(valueChanged(int)),  &r1,  SLOT(receive2(int))));   \
\
    QVERIFY(QtUiTest::connectFirst(&e2a, SIGNAL(valueChanged(int)),  &r2,  SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e2a, SIGNAL(valueChanged(int)),  &r2,  SLOT(receive2(int))));   \
    QVERIFY(QtUiTest::connectFirst(&e2b, SIGNAL(valueChanged(int)),  &r2,  SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e2b, SIGNAL(valueChanged(int)),  &r2,  SLOT(receive2(int))));   \
\
    QVERIFY(QtUiTest::connectFirst(&e3,  SIGNAL(valueChanged(int)),  &r3a, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e3,  SIGNAL(valueChanged(int)),  &r3a, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e3,  SIGNAL(valueChanged(int)),  &r3b, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e3,  SIGNAL(valueChanged(int)),  &r3b, SLOT(receive2(int))));   \
\
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged(int)),  &r4a, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged(int)),  &r4a, SLOT(receive2(int))));   \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged2(int)), &r4a, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged2(int)), &r4a, SLOT(receive2(int))));   \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged(int)),  &r4b, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged(int)),  &r4b, SLOT(receive2(int))));   \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged2(int)), &r4b, SLOT(receive(int))));    \
    QVERIFY(QtUiTest::connectFirst(&e4,  SIGNAL(valueChanged2(int)), &r4b, SLOT(receive2(int))));

    {
        INIT();

        // Verify connections for correctness
        QCOMPARE(cm->m_connections.count(), 18);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>() << 10);
        QCOMPARE(r1.values2,  QList<int>() << 10);
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>() << 30 << 30);
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4a.values2, QList<int>() << 40 << 40);
        QCOMPARE(r4b.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4b.values2, QList<int>() << 40 << 40);
    }

    {
        INIT();

        // All of these disconnects should fail:
        //  0,0,0,0
        //  0,0,0,x
        //  0,0,x,0
        //  0,0,x,x
        //  0,x,0,0
        //  0,x,0,x
        //  0,x,x,0
        //  0,x,x,x
        for (int i = 0; i < 8; ++i)
            QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: Unexpected null parameter");
        QVERIFY(!QtUiTest::disconnectFirst(0, 0, 0, 0));
        QVERIFY(!QtUiTest::disconnectFirst(0, 0, 0, SLOT(receive(int))));
        QVERIFY(!QtUiTest::disconnectFirst(0, 0, &r1, 0));
        QVERIFY(!QtUiTest::disconnectFirst(0, 0, &r1, SLOT(receive(int))));
        QVERIFY(!QtUiTest::disconnectFirst(0, SIGNAL(valueChanged(int)), 0, 0));
        QVERIFY(!QtUiTest::disconnectFirst(0, SIGNAL(valueChanged2(int)), 0, SLOT(receive(int))));
        QVERIFY(!QtUiTest::disconnectFirst(0, SIGNAL(valueChanged(int)), &r1, 0));
        QVERIFY(!QtUiTest::disconnectFirst(0, SIGNAL(valueChanged2(int)), &r2, SLOT(receive2(int))));
        QCOMPARE(cm->m_connections.count(), 18);
    }

    {
        INIT();

        // Test x,0,0,0
        QVERIFY(QtUiTest::disconnectFirst(&e4,0,0,0));
        QCOMPARE(cm->m_connections.count(), 10);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>() << 10);
        QCOMPARE(r1.values2,  QList<int>() << 10);
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>() << 30 << 30);
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>());
        QCOMPARE(r4a.values2, QList<int>());
        QCOMPARE(r4b.values,  QList<int>());
        QCOMPARE(r4b.values2, QList<int>());
    }

    {
        INIT();

        // Test x,0,0,x: should fail
        QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: Unexpected null parameter");
        QVERIFY(!QtUiTest::disconnectFirst(&e4, 0, 0, SLOT(receive2(int))));
        QCOMPARE(cm->m_connections.count(), 18);
    }

    {
        INIT();

        // Test x,0,x,0
        QVERIFY(QtUiTest::disconnectFirst(&e4, 0, &r4a, 0));
        QCOMPARE(cm->m_connections.count(), 14);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>() << 10);
        QCOMPARE(r1.values2,  QList<int>() << 10);
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>() << 30 << 30);
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>());
        QCOMPARE(r4a.values2, QList<int>());
        QCOMPARE(r4b.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4b.values2, QList<int>() << 40 << 40);
    }

    {
        INIT();

        // Test x,0,x,x
        QVERIFY(QtUiTest::disconnectFirst(&e4, 0, &r4a, SLOT(receive2(int))));

        QCOMPARE(cm->m_connections.count(), 16);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>() << 10);
        QCOMPARE(r1.values2,  QList<int>() << 10);
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>() << 30 << 30);
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4a.values2, QList<int>());
        QCOMPARE(r4b.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4b.values2, QList<int>() << 40 << 40);
    }

    {
        INIT();

        // Test x,x,0,0
        QVERIFY(QtUiTest::disconnectFirst(&e4, SIGNAL(valueChanged(int)), 0, 0));
        QCOMPARE(cm->m_connections.count(), 14);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>() << 10);
        QCOMPARE(r1.values2,  QList<int>() << 10);
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>() << 30 << 30);
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>() << 40);
        QCOMPARE(r4a.values2, QList<int>() << 40);
        QCOMPARE(r4b.values,  QList<int>() << 40);
        QCOMPARE(r4b.values2, QList<int>() << 40);
    }

    {
        INIT();

        // Test x,x,0,x: should fail
        QTest::ignoreMessage(QtWarningMsg, "Object::disconnect: Unexpected null parameter");
        QVERIFY(!QtUiTest::disconnectFirst(&e2a, SIGNAL(valueChanged(int)), 0, SLOT(receive(int))));

        QCOMPARE(cm->m_connections.count(), 18);
    }

    {
        INIT();

        // Test x,x,x,0
        QVERIFY(QtUiTest::disconnectFirst(&e1, SIGNAL(valueChanged(int)), &r1, 0));

        QCOMPARE(cm->m_connections.count(), 16);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>());
        QCOMPARE(r1.values2,  QList<int>());
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>() << 30 << 30);
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4a.values2, QList<int>() << 40 << 40);
        QCOMPARE(r4b.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4b.values2, QList<int>() << 40 << 40);
    }

    {
        INIT();

        // Test x,x,x,x
        QVERIFY(QtUiTest::disconnectFirst(&e3, SIGNAL(valueChanged(int)), &r3a, SLOT(receive(int))));

        QCOMPARE(cm->m_connections.count(), 16);

        e1. emitValue(10);
        e2a.emitValue(20);
        e2b.emitValue(21);
        e3. emitValue(30);
        e4. emitValue(40);
        QCOMPARE(r1.values,   QList<int>() << 10);
        QCOMPARE(r1.values2,  QList<int>() << 10);
        QCOMPARE(r2.values,   QList<int>() << 20 << 21);
        QCOMPARE(r2.values2,  QList<int>() << 20 << 21);
        QCOMPARE(r3a.values,  QList<int>());
        QCOMPARE(r3a.values2, QList<int>());
        QCOMPARE(r3b.values,  QList<int>() << 30);
        QCOMPARE(r3b.values2, QList<int>() << 30);
        QCOMPARE(r4a.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4a.values2, QList<int>() << 40 << 40);
        QCOMPARE(r4b.values,  QList<int>() << 40 << 40);
        QCOMPARE(r4b.values2, QList<int>() << 40 << 40);
    }

}

#include "tst_qtuitestnamespace.moc"
