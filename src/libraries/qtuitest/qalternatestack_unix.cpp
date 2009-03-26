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

#include "qalternatestack_p.h"

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <QDebug>

struct QAlternateStackPrivate
{
    QAlternateStack* q;

    // The memory containing the alternate stack.
    QByteArray stackBuffer;
    char*      stackBegin;
    int        stackSize;

    void allocate(int);

    // Alternate and original stack objects.
    stack_t alternateStack;
    stack_t originalStack;

    // Sigaction used to jump to new stack, and original sigaction.
    struct sigaction alternateAction;
    struct sigaction originalAction;

    // jmp_buf for jumping between stacks.
    jmp_buf alternateJmp;
    jmp_buf originalJmp;

    // Starting point of the new stack.
    QAlternateStackEntryPoint entryFunction;

    // Argument to be passed to entryFunction.
    QVariant entryData;

    // All instances of QAlternateStack.
    static QList<QAlternateStack*> instances;
};
QList<QAlternateStack*> QAlternateStackPrivate::instances;

#ifdef QTOPIA_TARGET
// When built as a part of Qt Extended, these symbols live in libqtopia.
// Otherwise they live in libqtuitest.
// This allows libqtopia to avoid linking against libqtuitest.
extern char*& qalternatestack_stackbuf();
extern int& qalternatestack_stackbuf_len();
#else
QTUITEST_EXPORT char*& qalternatestack_stackbuf()
{ static char* ret = 0; return ret; }
QTUITEST_EXPORT int& qalternatestack_stackbuf_len()
{ static int ret = 0; return ret; }
#endif

#ifndef QT_NO_DEBUG
QString qobject_to_string(QObject* object)
{
    QString ret;
    {
        QDebug dbg(&ret);
        dbg << object;
    }
    return ret;
}
#endif

void qt_sigaltstack(stack_t const* ss, stack_t* oss)
{
    if (0 != sigaltstack(ss, oss)) {
        qFatal("QAlternateStack: error in sigaltstack: %s", strerror(errno));
    }
}

void qt_sigaction(int signum, struct sigaction const* act, struct sigaction* oldact)
{
    if (0 != sigaction(signum,act,oldact)) {
        qFatal("QAlternateStack: error in sigaction: %s", strerror(errno));
    }
}

void qt_raise(int signum)
{
    if (-1 == raise(signum)) {
        qFatal("QAlternateStack: error in raise: %s", strerror(errno));
    }
}

QAlternateStackPrivate* qt_next_stack;

static const int JMP_SWITCHFROM         = 2;
static const int JMP_SWITCHTO           = 3;
static const int JMP_FINISH             = 4;
static const int QALTERNATESTACK_SIGNAL = SIGUSR2;

/*
    This wrapper function is called when we first enter an alternate stack,
    via the raising of QALTERNATESTACK_SIGNAL.
*/
void qt_entry_function_wrapper(int)
{
    // Establish this context into the alternate jmp_buf.
    // We cannot have the whole stack run during a signal handler,
    // so we store our context when we are first called, then return and allow
    // the original stack to jump back to us.

    int value = setjmp(qt_next_stack->alternateJmp);

    // We might be here for these reasons:
    //
    // 1. We are handling the signal and just called setjmp.
    //    value == 0.
    if (0 == value) {
        // Return from the signal handler; the original stack will jump back
        // to us via longjmp.
        return;
    }
    // 2. We are switched to from the original stack.
    //    value == JMP_SWITCHTO.
    else if (JMP_SWITCHTO == value) {
    }
    // Whoops, programmer error.
    else Q_ASSERT(0);

    QAlternateStackPrivate* stack = qt_next_stack;
    stack->entryFunction(stack->q, stack->entryData);

    // OK, jump back to the main stack and let it know that we're finished.
    // We can't just return here; if we did, we'd jump back to the signal
    // handler frame, but the signal handler is already finished.
    longjmp(stack->originalJmp, JMP_FINISH);
}

/*!
    \internal
    \class QAlternateStack
    \inpublicgroup QtUiTestModule
    \brief The QAlternateStack class provides a call stack.

    QAlternateStack can be used to switch between two call stacks without
    using threads.

    There is one anticipated use for this class, and that is to wait for a
    specific amount of time, while processing events, without hanging if a
    nested event loop occurs.

    Example:
    \code
    void runsInAlternateStack(QAlternateStack* stack, QVariant const& data)
    {
        int foo = data.toInt();
        // ...
        // Need to wait for about 1500 milliseconds here, while processing events.
        // Process events in the main stack to avoid hanging on nested event loops.
        QTimer::singleShot(1500, stack, SLOT(switchTo()));
        stack->switchFrom();

        // Execution resumes here in ~1500ms, as long as the main stack is
        // processing events, even if there was a nested event loop.
        // ...

        // Don't need the stack any more
        stack->deleteLater();
    }

    void runsInMainStack()
    {
        QAlternateStack* stack = new QAlternateStack;
        stack->start(runsInAlternateStack, 123);
    }
    \endcode

    Using this class makes code more difficult to understand.  Use an
    alternative where possible.
*/

/*!
    \relates QAlternateStack
    \typedef QAlternateStackEntryPoint

    Typedef for a pointer to a function with the signature
    \c{void my_function(QAlternateStack*,QVariant const&)}.

    Used as an entry point to a new stack.
*/

/*!
    Creates an alternate stack of \a size bytes with the given \a parent.

    Behavior is undefined if the alternate stack exceeds \a size.
*/
QAlternateStack::QAlternateStack(quint32 size, QObject* parent)
    : QObject(parent),
      d(new QAlternateStackPrivate)
{
    Q_ASSERT_X(QAlternateStack::isAvailable(), "QAlternateStack",
            "QAlternateStack is not available on this platform!");

    d->q = this;
    d->entryFunction = 0;

    d->allocate(size);

    QAlternateStackPrivate::instances << this;
}

void QAlternateStackPrivate::allocate(int size)
{
    if (!qalternatestack_stackbuf()) {
        // Allocate space for the new stack.
        stackBuffer.resize(size);
        stackBegin = stackBuffer.data();
        stackSize  = size;
        return;
    }

    /*
        On some systems, it has been found that due to limitations in the kernel and/or threading
        implementation, setting the stack pointer to point to memory allocated anywhere other than
        the main stack causes a crash (bug 209341).

        This code works around this bug by using a pool of memory which was earlier allocated on
        the stack.  qalternatestack_stackbuf() and qalternatestack_stackbuf_len() must be
        explicitly initialized to point to a block of memory and to tell us its length.
    */

    // Find the first chunk of unused memory.
    char* begin = qalternatestack_stackbuf();
    char* end   = qalternatestack_stackbuf() + qalternatestack_stackbuf_len();
    char* found = 0;
    int total_used = 0;

    for (char* buf = begin; buf <= end - size && !found; buf += size) {
        char* buf_end = buf + size;
        // Is this block of memory definitely unused?
        bool used = false;
        foreach (QAlternateStack* st, QAlternateStackPrivate::instances) {
            char* st_buf = st->d->stackBegin;
            char* st_buf_end = st->d->stackBegin + st->d->stackSize;
            // If we start after this stack ends, or we end before it finishes, we're OK.
            if (buf >= st_buf_end || buf_end <= st_buf) {
            } else {
                used = true;
                total_used += (st_buf_end - st_buf);
                break;
            }
        }
        if (!used) found = buf;
    }

    if (!found) {
        qFatal( "QtUiTest: QAlternateStack ran out of memory! There seem to be %d stacks using a "
                "total of %d bytes (out of %d), and a stack of size %d was requested. "
                "Please increase the size of qalternatestack_stackbuf!",
                QAlternateStackPrivate::instances.count(), total_used,
                    qalternatestack_stackbuf_len(), size);
    }
    stackBegin = found;
    stackSize  = size;
}

/*!
    Destroys the alternate stack.

    Behavior is undefined if the stack is destroyed while currently active.
*/
QAlternateStack::~QAlternateStack()
{
    delete d;
    d = 0;
    QAlternateStackPrivate::instances.removeAll(this);
}

/*!
    Switch from the original stack to the alternate stack, and start
    executing \a entry in the alternate stack.
    This stack and \a data will be passed as an argument to \a entry.

    This function should be called when first switching to an alternate
    stack.  When resuming a stack that is already active, use switchTo().

    \sa switchTo(), switchFrom()
*/
void QAlternateStack::start(QAlternateStackEntryPoint entry, const QVariant& data)
{
    Q_ASSERT_X(!isActive(), "QAlternateStack::start",
            qPrintable(QString("`start' called while already active. sender(): %1")
                .arg(qobject_to_string(sender()))));

    // Set up the alternate stack to be jumped to.
    d->alternateStack.ss_sp    = d->stackBegin;
    d->alternateStack.ss_flags = 0;
    d->alternateStack.ss_size  = d->stackSize;
    qt_sigaltstack(&d->alternateStack, &d->originalStack);

    // Set up signal handler to jump to entry function wrapper in alternate stack.
    d->alternateAction.sa_handler = qt_entry_function_wrapper;
    sigemptyset(&d->alternateAction.sa_mask);
    d->alternateAction.sa_flags   = SA_ONSTACK;
    qt_sigaction(QALTERNATESTACK_SIGNAL, &d->alternateAction, &d->originalAction);

    // Raise the signal, jumping to the alternate stack.
    qt_next_stack = d;
    qt_raise(QALTERNATESTACK_SIGNAL);

    // signal handler returns immediately after storing its context into
    // alternateJmp.  Restore the old signal handler.
    qt_sigaltstack(&d->originalStack, 0);
    qt_sigaction(QALTERNATESTACK_SIGNAL, &d->originalAction, 0);

    // OK, now store the entry function and data and jump to the new stack.
    d->entryFunction = entry;
    d->entryData     = data;

    switchTo();
}

/*!
    Switch from the original stack to the alternate stack.

    This function can be called to resume execution in an alternate stack.

    If execution has been suspended in the alternate stack by a call to
    switchFrom(), switchTo() will resume executing at that point.
    If the alternate stack has completed execution or hasn't started, this
    function does nothing and returns immediately.

    It is an error to call this function from the alternate stack.
*/
void QAlternateStack::switchTo()
{
    // We must not be the currently active stack.
    Q_ASSERT_X(!isCurrentStack(), "QAlternateStack::switchTo",
            qPrintable(QString("`switchTo' called in currently active stack. sender(): %1")
                .arg(qobject_to_string(sender()))));

    // Store where we currently are.
    int value = setjmp(d->originalJmp);

    // Now it really gets tricky.
    // At this particular point, we could be here for these reasons:
    //
    //  1. switchTo() was actually called from the original stack
    //     and we just returned from setjmp.
    //     value == 0.
    //
    //  2. The alternate stack switched to the original stack
    //     by calling switchFrom().
    //     value == JMP_SWITCHFROM.
    //
    //  3. The alternate stack finished execution and jumped back to us.
    //     value == JMP_FINISH.

    // Case 1: just returned from setjmp.
    //         Jump to the other stack.
    if (0 == value) {
        // If there is no entry function, that means `start' has not been called or we have
        // finished already.  In other words, there's no more to process on this stack, so
        // just return immediately.
        if (!d->entryFunction) return;

        // This function never returns; from here we will jump back to
        // the above call to setjmp() via case 2 or 3.
        longjmp(d->alternateJmp, JMP_SWITCHTO);
    }

    // Case 2: switchFrom() called in alternate stack.
    if (value == JMP_SWITCHFROM) {
        // Don't need to do anything, just return.
        return;
    }

    // Case 3: The alternate stack finished execution.
    // Just need to do a little cleanup.
    if (value == JMP_FINISH) {
        d->entryFunction = 0;
        d->entryData     = QVariant();
        return;
    }

    // Whoops, bad programmer.
    Q_ASSERT(0);
}

/*!
    Switch from the alternate stack to the original stack.

    Once execution takes place in the alternate stack, there are two ways
    to return to the original stack.

    The first is simply to return from the entry function passed to the
    constructor, at which point the switchTo() function in the original
    stack will return.

    The second is to call switchFrom().  This will cause the switchTo()
    function to return in the original stack.  switchFrom() will not
    return until switchTo() is called again in the original stack, which
    is not guaranteed to happen.

    It is an error to call this function from the original stack.
*/
void QAlternateStack::switchFrom()
{
    // We must be the currently active stack.
    Q_ASSERT_X(isCurrentStack(), "QAlternateStack::switchFrom",
        qPrintable(QString("`switchFrom' called from wrong stack. sender(): %1")
            .arg(qobject_to_string(sender()))));

    int value = setjmp(d->alternateJmp);

    // Two possibilities:
    //
    // 1. We just called setjmp and will now jump to main stack.
    //  value == 0
    if (0 == value) {
        longjmp(d->originalJmp, JMP_SWITCHFROM);
    }
    // 2. We just jumped to here from main stack.
    //    value == JMP_SWITCHTO
    else if (JMP_SWITCHTO == value) {
    }
    // Whoops, bad programmer.
    else Q_ASSERT(0);
}

/*!
    Returns true if the stack has started (entry function has been called)
    and not yet finished (entry function has not returned).

    \sa isCurrentStack()
*/
bool QAlternateStack::isActive() const
{ return d->entryFunction; }

/*!
    Returns true if the currently used stack is this QAlternateStack.

    \sa isActive()
*/
bool QAlternateStack::isCurrentStack() const
{
    // Test if a stack-allocated variable resides within the alternate
    // stack buffer.
    char dummy = 1;
    quint32 diff = &dummy - d->stackBegin;
    return (diff < (quint32)d->stackSize);
}

/*!
    Returns all created QAlternateStack objects.
*/
QList<QAlternateStack*> QAlternateStack::instances()
{ return QAlternateStackPrivate::instances; }

/*!
    Returns true if QAlternateStack is usable on this platform.

    QAlternateStack depends on System-V signal stacks, which is not reliable on
    some platforms.  On these platforms, this function will return false.

    Usage of QAlternateStack when this function returns false will typically
    result in a fatal error at runtime.
*/
bool QAlternateStack::isAvailable()
{
#ifdef QT_QWS_GREENPHONE
    // On the Greenphone, it works if and only if the stackbuf has been set up.
    return qalternatestack_stackbuf();
#else
    return true;
#endif
}

