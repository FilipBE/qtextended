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

#include <QTest>
#include <QObject>
#include <qalternatestack_p.h>
#include <shared/qtopiaunittest.h>

#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

//TESTED_COMPONENT=QA: Testing Framework (18707)

class tst_QAlternateStack : public QObject
{
    Q_OBJECT

private slots:
    void start();
    void switching();

    void init();

    void sigaltstack();

    void multipleInstances();

private:
    static void stack_entry(QAlternateStack*, QVariant const&);

    static void stack_entry1(QAlternateStack*, QVariant const&);
    static void stack_entry2(QAlternateStack*, QVariant const&);
    static void stack_entry3(QAlternateStack*, QVariant const&);
    static void stack_entry4(QAlternateStack*, QVariant const&);
    static void stack_entry_handler(QAlternateStack*,QVariant const&,int);

    int              m_shouldSwitchFrom;

    QAlternateStack* m_stack;
    QVariant         m_data;
    bool             m_isActive;
    bool             m_isCurrentStack;

    QList<QPair<int,QAlternateStack*> > m_stackEntries;
};

class tst_QAlternateStackWithStackBuffer : public tst_QAlternateStack
{
    Q_OBJECT
};

extern char*& qalternatestack_stackbuf();
extern int&   qalternatestack_stackbuf_len();

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    int ret = 0;

    // First, test the regular heap-allocated alternate stack.
    {
        tst_QAlternateStack test;
        ret += QTest::qExec(&test, argc, argv);
    }

    // Now try using memory allocated from the main stack only.
    {
        char buf[65536*4];
        qalternatestack_stackbuf() = buf;
        qalternatestack_stackbuf_len() = 256*1024;
        tst_QAlternateStackWithStackBuffer test;
        ret += QTest::qExec(&test, argc, argv);
    }

    return ret;
}

void tst_QAlternateStack::stack_entry(QAlternateStack* stack, QVariant const& data)
{
    QVariantList list = data.toList();
    if (!list.count()) return;
    if (!stack)        return;

    tst_QAlternateStack* test = qobject_cast<tst_QAlternateStack*>(list.at(0).value<QObject*>());
    if (!test) return;

    test->m_stack          = stack;
    test->m_data           = data;
    test->m_isActive       = stack->isActive();
    test->m_isCurrentStack = stack->isCurrentStack();

    for (; test->m_shouldSwitchFrom > 0; --test->m_shouldSwitchFrom) {
        stack->switchFrom();
        test->m_isActive       = stack->isActive();
        test->m_isCurrentStack = stack->isCurrentStack();
    }
}

void tst_QAlternateStack::stack_entry1(QAlternateStack* stack, QVariant const& data)
{ stack_entry_handler(stack, data, 1); }
void tst_QAlternateStack::stack_entry2(QAlternateStack* stack, QVariant const& data)
{ stack_entry_handler(stack, data, 2); }
void tst_QAlternateStack::stack_entry3(QAlternateStack* stack, QVariant const& data)
{ stack_entry_handler(stack, data, 3); }
void tst_QAlternateStack::stack_entry4(QAlternateStack* stack, QVariant const& data)
{ stack_entry_handler(stack, data, 4); }

void tst_QAlternateStack::stack_entry_handler(QAlternateStack* stack, QVariant const& data, int number)
{
    QVariantList list = data.toList();
    if (!list.count()) return;

    tst_QAlternateStack* test = qobject_cast<tst_QAlternateStack*>(list.at(0).value<QObject*>());
    if (!test) return;

    test->m_stackEntries << qMakePair(number, stack);
    test->m_stack          = stack;
    test->m_isActive       = stack->isActive();
    test->m_isCurrentStack = stack->isCurrentStack();

    // It is possible we've been called on a stack which overlaps some other memory.
    // When this happens, try to increase the chances of causing a crash by corrupting memory.
    static const int LEN = 32768;
    char buf[LEN];
    for (int i = 0; i < LEN; i += 4) {
        buf[i]   = 0xDE;
        buf[i+1] = 0xAD;
        buf[i+2] = 0xBE;
        buf[i+3] = 0xEF;
    }
    // Make sure compiler doesn't optimize it out
    qChecksum(buf, LEN);
}

/*
    \req QTOPIA-78

    \groups
*/
void tst_QAlternateStack::start()
{
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }

    QAlternateStack stack;

    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);

    QVariantList list;
    list.append( qVariantFromValue((QObject*)this) );

    // Switch to the stack...
    stack.start(tst_QAlternateStack::stack_entry, list);
    // ... it should have returned almost immediately.

    // Check that our private members were set to the expected values.
    QCOMPARE(m_stack,          &stack);
    QCOMPARE(m_data,           qVariantFromValue(list));
    QCOMPARE(m_isActive,       true);
    QCOMPARE(m_isCurrentStack, true);

    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);
}

/*
    \req QTOPIA-78

    \groups
*/
void tst_QAlternateStack::switching()
{
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }

    QAlternateStack stack;

    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);

    QVariantList list;
    list.append( qVariantFromValue((QObject*)this) );

    // Set it up so the alternate stack calls switchFrom() several times.
    const int switchCount = 5;
    m_shouldSwitchFrom = switchCount;

    stack.start(tst_QAlternateStack::stack_entry, list);

    for (int i = switchCount; i > 0; --i) {
        // Check that our private members were set to the expected values.
        QCOMPARE(m_stack,          &stack);
        QCOMPARE(m_data,           qVariantFromValue(list));
        QCOMPARE(m_isActive,       true);
        QCOMPARE(m_isCurrentStack, true);

        QCOMPARE(m_shouldSwitchFrom, i);

        // Still active, since we switched using switchFrom().
        QCOMPARE(stack.isActive(),       true);
        QCOMPARE(stack.isCurrentStack(), false);

        // Switch to the stack...
        stack.switchTo();
    }

    // No longer active.
    QCOMPARE(m_shouldSwitchFrom,     0);
    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);
}

char* addressof_dummy;
int   got_signal;

void test_sighandler(int signum)
{
    got_signal = signum;

    int dummy = 1;
    addressof_dummy = (char*)&dummy;
}

/*
    \req QTOPIA-78

    \groups

    Tests that sigaltstack() actually works on the target platform.

    On some platforms, like the Greenphone, sigaltstack seems to be broken
    and always causes a segfault.  When porting QtUitest to a new platform,
    run this testfunction to make sure QAlternateStack will work properly.
*/
void tst_QAlternateStack::sigaltstack()
{
    // isAvailable() returns false on platforms where sigaltstack is known
    // to be broken.
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }
    QByteArray buffer;
    buffer.resize(SIGSTKSZ);

    stack_t stack;
    stack.ss_sp    = (qalternatestack_stackbuf() ? qalternatestack_stackbuf() : buffer.data());
    stack.ss_size  = SIGSTKSZ;
    stack.ss_flags = 0;

    QVERIFY( 0 == ::sigaltstack(&stack, 0) );

    stack_t newstack;
    QVERIFY( 0 == ::sigaltstack(0, &newstack) );
    QVERIFY(stack.ss_sp    == newstack.ss_sp);
    QVERIFY(stack.ss_size  == newstack.ss_size);
    QVERIFY(stack.ss_flags == newstack.ss_flags);

    struct sigaction action;
    action.sa_handler = test_sighandler;
    action.sa_flags   = SA_ONSTACK;
    sigemptyset(&action.sa_mask);

    QVERIFY2( 0 == sigaction(SIGUSR2, &action, 0), strerror(errno) );

    struct sigaction newaction;
    QVERIFY2( 0 == sigaction(SIGUSR2,0,&newaction), strerror(errno) );
    QVERIFY(newaction.sa_handler == action.sa_handler);
    QVERIFY(newaction.sa_flags & SA_ONSTACK);

    addressof_dummy = 0;
    got_signal      = 0;
    qLog(Autotest) << "About to raise";

    // Greenphone crashes here.
    raise(SIGUSR2);

    QCOMPARE(got_signal, SIGUSR2);
    qLog(Autotest) << "dummy:" << (void*)addressof_dummy << "stack:" << stack.ss_sp;
    QVERIFY( (addressof_dummy > (char*)stack.ss_sp) && (addressof_dummy < (char*)stack.ss_sp + stack.ss_size) );
}

void tst_QAlternateStack::multipleInstances()
{
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }

    QVariantList list;
    list.append( qVariantFromValue((QObject*)this) );

    QAlternateStack stack1;
    QAlternateStack stack2;
    QAlternateStack stack3;
    QAlternateStack stack4;

    QList<QAlternateStack*> stacks;
    stacks << &stack1 << &stack2 << &stack3 << &stack4;

    // Verify the usual stack functions work as expected.
    foreach (QAlternateStack* stack, stacks) {
        m_stack            = 0;
        m_isActive         = false;
        m_isCurrentStack   = false;
        QVERIFY(!stack->isActive());
        QVERIFY(!stack->isCurrentStack());

        stack->start(stack_entry1, list);

        QCOMPARE(m_stack, stack);
        QVERIFY(m_isActive);
        QVERIFY(m_isCurrentStack);
        QVERIFY(!stack->isActive());
        QVERIFY(!stack->isCurrentStack());
    }

    {
        QList<QPair<int,QAlternateStack*> > expected;
        expected
            << qMakePair(1, &stack1)
            << qMakePair(1, &stack2)
            << qMakePair(1, &stack3)
            << qMakePair(1, &stack4)
        ;
        QCOMPARE(m_stackEntries, expected);
        m_stackEntries.clear();
    }

    // Verify the correct functions all get called as expected.
    stack1.start(stack_entry1, list);
    stack4.start(stack_entry4, list);
    stack2.start(stack_entry2, list);
    stack3.start(stack_entry3, list);
    stack1.start(stack_entry1, list);
    stack4.start(stack_entry4, list);
    stack3.start(stack_entry3, list);

    {
        QList<QPair<int,QAlternateStack*> > expected;
        expected
            << qMakePair(1, &stack1)
            << qMakePair(4, &stack4)
            << qMakePair(2, &stack2)
            << qMakePair(3, &stack3)
            << qMakePair(1, &stack1)
            << qMakePair(4, &stack4)
            << qMakePair(3, &stack3)
        ;

        QCOMPARE(m_stackEntries, expected);
    }
}

void tst_QAlternateStack::init()
{
    m_shouldSwitchFrom = 0;
    m_stack            = 0;
    m_data             = QVariant();
    m_isActive         = false;
    m_isCurrentStack   = false;
}

#include "tst_qalternatestack.moc"
