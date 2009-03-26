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

#include "qfuturesignal.h"
#include <QSignalIntercepter>
#include <QEventLoop>
#include <QTimer>

/*!
    \class QFutureSignal
    \internal
    \mainclass
    \brief The QFutureSignal class intercepts signals so that they can be inspected in the future.
    \ingroup qtuitest
    \ingroup qtuitest_unittest

    Testing objects that emit signals in unit tests can be difficult.
    The QSignalSpy class helps, but it cannot catch signals that happen
    upon the next entry to the event loop.

    To catch signals upon the next entry to the event loop, a slot needs
    to be created that is connected to the signal.  And then the unit
    test needs to somehow wait, processing events, until either the
    signal happens or there is a timeout.  This can lead to unit tests
    containing many "signal occurred" flags and calls to QTest::qWait().

    The QFutureSignal class simplifies unit tests that involve signals
    by combining the behavior of QSignalSpy with a nested event loop.

    The following example demonstrates how to catch a \c{changed()} signal
    on \c{obj} with a 3 second timeout:

    \code
    QFutureSignal fs(obj, SIGNAL(changed());
    ...
    QVERIFY(fs.wait(3000));
    \endcode

    If the signal occurs before the call to wait(), it will return true
    immediately, and the test will succeed.  If the signal does not occur
    before the call to wait(), it will enter a nested event loop until
    either the signal occurs, or the timeout expires.  The wait()
    function will return false on timeout, failing the test case.

    If the signal has a value associated with it, it can be checked
    using results():

    \code
    QFutureSignal fs(obj, SIGNAL(valueChanged(int));
    ...
    QVERIFY(fs.wait(3000));
    QCOMPARE(fs.resultCount(), 1);
    QCOMPARE(fs.results()[0][0].toInt(), 26);
    \endcode

    The above can be simplified using the expect() convenience function:

    \code
    QFutureSignal fs(obj, SIGNAL(stateChanged(bool));
    ...
    QVERIFY(fs.expect(3000, 26));
    \endcode

    The expect() function checks that the signal is emitted exactly one
    time, whereas wait() will be satisfied with one or more emits.
    For a signal with no arguments, the following can be done:

    \code
    QFutureSignal fs(obj, SIGNAL(changed());
    ...
    QVERIFY(fs.expect(3000));
    \endcode

    If the signal will always happen upon the next entry to the event loop,
    then it is possible to create a QFutureSignal object and wait() or expect()
    on it using a single line of code:

    \code
    QVERIFY(QFutureSignal::wait(obj, SIGNAL(changed()), 3000));
    QVERIFY(QFutureSignal::expect(obj, SIGNAL(valueChanged(int)), 3000, 26));
    \endcode

    Doing this is very risky if the signal might occur before the
    wait() or expect() call.  This simplified style should only be used
    when the programmer knows for sure that the signal will happen
    upon the next entry to the event loop.

    \sa QFutureSignalList, QSignalSpy
*/

class QFutureSignalPrivate : public QSignalIntercepter
{
    // Do not put Q_OBJECT here, because of how QSignalIntercepter works.
public:
    QFutureSignalPrivate(QObject *sender, const QByteArray& signal)
        : QSignalIntercepter(sender, signal)
    {
        eventLoop = 0;
        expectedCount = 1;
        allList = 0;
    }

    QList< QList<QVariant> > results;
    QEventLoop *eventLoop;
    int expectedCount;
    QFutureSignalList *allList;

protected:
    void activated(const QList<QVariant>& args);
};

void QFutureSignalPrivate::activated(const QList<QVariant>& args)
{
    results += args;
    if (allList)
        allList->checkResults();
    else if (eventLoop && results.size() >= expectedCount)
        eventLoop->quit();
}

/*!
    Constructs a future signal intercepter for \a signal on \a sender.
    Whenever the signal occurs, the signal parameters are added to a
    results() list so that they can be inspected sometime in the future.

    \sa wait(), expect()
*/
QFutureSignal::QFutureSignal(QObject *sender, const QByteArray& signal)
{
    d = new QFutureSignalPrivate(sender, signal);
}

/*!
    Constructs a copy of \a other with a new results() list.
*/
QFutureSignal::QFutureSignal(const QFutureSignal& other)
{
    d = new QFutureSignalPrivate(other.d->sender(), other.d->signal());
}

/*!
    Destroys this future signal intercepter.
*/
QFutureSignal::~QFutureSignal()
{
    delete d;
}

/*!
    Copies the contents of \a other into this object.  The results()
    list will be cleared by this operation, even if \a other
    contained results.
*/
QFutureSignal& QFutureSignal::operator=(const QFutureSignal& other)
{
    if (this != &other) {
        delete d;
        d = new QFutureSignalPrivate(other.d->sender(), other.d->signal());
    }
    clear();
    return *this;
}

/*!
    Waits for the signal to occur, processing Qt events while waiting.
    Returns true if one or more signals occurred, or false on timeout.
    If the signal has already occurred, this function returns true
    immediately.

    If \a timeout is greater than or equal to zero, it indicates that
    this function should return false if the signal does not occur within
    the specified timeout.  If \a timeout is less than zero, then this
    function will wait forever for the signal.

    The \a expectedCount parameter indicates the number of signal emissions
    we expect before wait() returns.  Normally this is 1.

    The following example demonstrates how to catch a signal that
    either happens before the call to \c{fs.wait()}, or upon the next
    entry to the Qt event loop:

    \code
    QFutureSignal fs(obj, SIGNAL(changed());
    ...
    QVERIFY(fs.wait());
    \endcode

    \sa expect(), results(), clear()
*/
bool QFutureSignal::wait(int timeout, int expectedCount)
{
    // If we already have sufficient results, then return immediately.
    if (d->results.size() >= expectedCount)
        return true;

    // Bail out if we were recursively re-entered.
    if (d->eventLoop)
        return false;

    // Run a nested event loop until the signal is emitted, or timeout.
    d->eventLoop = new QEventLoop(d);
    d->expectedCount = expectedCount;
    if (timeout >= 0)
        QTimer::singleShot(timeout, d->eventLoop, SLOT(quit()));
    d->eventLoop->exec();
    delete d->eventLoop;
    d->eventLoop = 0;

    // Indicate to the caller whether we got something before the timeout.
    return !d->results.isEmpty();
}

/*!
    Creates a QFutureSignal object for \a sender and \a signal and
    calls wait() on it.  The wait() call will be passed \a timeout
    and \a expectedCount.

    This function is convenience for waiting for signals without
    creating an explicit QFutureSignal object first.  For example:

    \code
    QVERIFY(QFutureSignal::wait(obj, SIGNAL(changed()), 3000));
    \endcode

    Note that if the signal has already occurred, the above code
    will not catch it, and if a timeout is not supplied, the test
    case could hang forever.  This function should therefore only
    be used for signals that will happen upon the next entry
    to the Qt event loop.  If the signal may occur before the next
    entry to the Qt event loop, the correct sequence is as follows:

    \code
    QFutureSignal fs(obj, SIGNAL(changed());
    // Do something that may cause the signal.
    QVERIFY(fs.wait(3000));
    \endcode

    \sa expect(), results(), clear()
*/
bool QFutureSignal::wait
    (QObject *sender, const QByteArray& signal, int timeout, int expectedCount)
{
    QFutureSignal fs(sender, signal);
    return fs.wait(timeout, expectedCount);
}

/*!
    Waits for the signal to occur and then checks that there was
    exactly one signal emission.

    Returns false if the signal did not occur before the specified
    \a timeout, or if more than one signal occurred.

    If there are any arguments on the signal, they can be accessed
    via results() after expect() returns.  This differs from the
    other variations on expect() which check that the arguments
    are exactly as desired.

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect(int timeout)
{
    if (!wait(timeout, 1))
        return false;
    if (d->results.size() != 1)
        return false;
    return true;
}

/*!
    Waits for the signal to occur and then checks that there was
    exactly one signal emission, and that its argument was the
    same as \a value.

    Returns false if the signal did not occur before the specified
    \a timeout, if more than one signal occurred, or the signal
    did not have \a value as its argument.

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect(int timeout, const QVariant& value)
{
    if (!wait(timeout, 1))
        return false;
    if (d->results.size() != 1)
        return false;
    if (d->results[0].size() != 1)
        return false;
    return (value == d->results[0][0]);
}

/*!
    Waits for the signal to occur and then checks that there was
    exactly one signal emission, and that its arguments were
    \a value1 and \a value2.

    Returns false if the signal did not occur before the specified
    \a timeout, if more than one signal occurred, or the signal
    did not have \a value1 and \a value2 as its arguments.

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect(int timeout, const QVariant& value1, const QVariant& value2)
{
    if (!wait(timeout, 1))
        return false;
    if (d->results.size() != 1)
        return false;
    if (d->results[0].size() != 2)
        return false;
    if (value1 != d->results[0][0])
        return false;
    return (value2 == d->results[0][1]);
}

/*!
    Waits for the signal to occur and then checks that there was
    exactly one signal emission, and that its arguments were \a values.

    Returns false if the signal did not occur before the specified
    \a timeout, if more than one signal occurred, or the signal
    did not have \a values as its arguments.

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect(int timeout, const QList<QVariant>& values)
{
    if (!wait(timeout, 1))
        return false;
    if (d->results.size() != 1)
        return false;
    if (d->results[0].size() != values.size())
        return false;
    for (int index = 0; index < values.size(); ++index) {
        if (values[index] != d->results[0][index])
            return false;
    }
    return true;
}

/*!
    Creates a QFutureSignal object for \a sender and \a signal and
    calls expect() on it.  The expect() call will be passed \a timeout.

    This function is convenience for expecting signals without
    creating an explicit QFutureSignal object first.  For example:

    \code
    QVERIFY(QFutureSignal::expect(obj, SIGNAL(changed()), 3000));
    \endcode

    Note that if the signal has already occurred, the above code
    will not catch it, and if a timeout is not supplied, the test
    case could hang forever.  This function should therefore only
    be used for signals that will happen upon the next entry
    to the Qt event loop.  If the signal may occur before the next
    entry to the Qt event loop, the correct sequence is as follows:

    \code
    QFutureSignal fs(obj, SIGNAL(changed());
    // Do something that may cause the signal.
    QVERIFY(fs.expect(3000));
    \endcode

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect(QObject *sender, const QByteArray& signal, int timeout)
{
    QFutureSignal fs(sender, signal);
    return fs.expect(timeout);
}

/*!
    Creates a QFutureSignal object for \a sender and \a signal and
    calls expect() on it.  The expect() call will be passed \a timeout
    and \a value.

    This function is convenience for expecting signals without
    creating an explicit QFutureSignal object first.  For example:

    \code
    QVERIFY(QFutureSignal::expect(obj, SIGNAL(valueChanged(int)), 3000, 26));
    \endcode

    Note that if the signal has already occurred, the above code
    will not catch it, and if a timeout is not supplied, the test
    case could hang forever.  This function should therefore only
    be used for signals that will happen upon the next entry
    to the Qt event loop.  If the signal may occur before the next
    entry to the Qt event loop, the correct sequence is as follows:

    \code
    QFutureSignal fs(obj, SIGNAL(valueChanged(int));
    // Do something that may cause the signal.
    QVERIFY(fs.expect(3000, 26));
    \endcode

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect(QObject *sender, const QByteArray& signal,
                           int timeout, const QVariant& value)
{
    QFutureSignal fs(sender, signal);
    return fs.expect(timeout, value);
}

/*!
    Creates a QFutureSignal object for \a sender and \a signal and
    calls expect() on it.  The expect() call will be passed \a timeout,
    \a value1, and \a value2.

    This function is convenience for expecting signals without
    creating an explicit QFutureSignal object first.  For example:

    \code
    QVERIFY(QFutureSignal::expect(obj, SIGNAL(valueChanged(int,bool)), 3000, 26, true));
    \endcode

    Note that if the signal has already occurred, the above code
    will not catch it, and if a timeout is not supplied, the test
    case could hang forever.  This function should therefore only
    be used for signals that will happen upon the next entry
    to the Qt event loop.  If the signal may occur before the next
    entry to the Qt event loop, the correct sequence is as follows:

    \code
    QFutureSignal fs(obj, SIGNAL(valueChanged(int,bool));
    // Do something that may cause the signal.
    QVERIFY(fs.expect(3000, 26, true));
    \endcode

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect
        (QObject *sender, const QByteArray& signal,
         int timeout, const QVariant& value1,
         const QVariant& value2)
{
    QFutureSignal fs(sender, signal);
    return fs.expect(timeout, value1, value2);
}

/*!
    Creates a QFutureSignal object for \a sender and \a signal and
    calls expect() on it.  The expect() call will be passed \a timeout
    and \a values.

    This function is convenience for expecting signals without
    creating an explicit QFutureSignal object first.  For example:

    \code
    QList<QVariant> list;
    list << 26;
    list << true;
    list << QString("foo");
    QVERIFY(QFutureSignal::expect(obj, SIGNAL(valueChanged(int,bool,QString)), 3000, list));
    \endcode

    Note that if the signal has already occurred, the above code
    will not catch it, and if a timeout is not supplied, the test
    case could hang forever.  This function should therefore only
    be used for signals that will happen upon the next entry
    to the Qt event loop.  If the signal may occur before the next
    entry to the Qt event loop, the correct sequence is as follows:

    \code
    QFutureSignal fs(obj, SIGNAL(valueChanged(int,bool,QString));
    // Do something that may cause the signal.
    QList<QVariant> list;
    list << 26;
    list << true;
    list << QString("foo");
    QVERIFY(fs.expect(3000, list));
    \endcode

    \sa wait(), results(), clear()
*/
bool QFutureSignal::expect
        (QObject *sender, const QByteArray& signal,
         int timeout, const QList<QVariant>& values)
{
    QFutureSignal fs(sender, signal);
    return fs.expect(timeout, values);
}

/*!
    Returns the signal results that have been collected so far.

    \sa wait(), clear(), resultCount()
*/
QList< QList<QVariant> > QFutureSignal::results() const
{
    return d->results;
}

/*!
    Returns the number of signal results that have been collected so far.

    \sa results()
*/
int QFutureSignal::resultCount() const
{
    return d->results.size();
}

/*!
    Clears the signal results that have been collected so far.  This can be
    used when waiting for the same signal multiple times, checking the
    results between each call to wait().

    \sa results(), wait()
*/
void QFutureSignal::clear()
{
    d->results = QList< QList<QVariant> >();
}

/*!
    \class QFutureSignalList
    \internal
    \mainclass
    \brief The QFutureSignalList class allows for a unit test to wait for any or all of a group of signals to be emitted.
    \ingroup qtuitest
    \ingroup qtuitest_unittest

    The following example waits for any of the objects \c obj1, \c obj2, and
    \c obj3 to emit the \c{changed()} signal.  As soon as one of them does,
    waitAny() will return true.  If however none of them emits the signal
    within 1 second, waitAny() will return false and the test will fail.

    \code
    QFutureSignalList fsl;
    fsl.addFutureSignal(obj1, SIGNAL(changed()));
    fsl.addFutureSignal(obj2, SIGNAL(changed()));
    fsl.addFutureSignal(obj3, SIGNAL(changed()));
    QVERIFY(fsl.waitAny(1000));
    \endcode

    The unit test can determine which of the signals actually fired using
    the at() function:

    \code
    if (fsl.at(0)->resultCount() > 0)
        ... // obj1 fired
    if (fsl.at(1)->resultCount() > 0)
        ... // obj2 fired
    if (fsl.at(2)->resultCount() > 0)
        ... // obj3 fired
    \endcode

    \sa QFutureSignal
*/

class QFutureSignalListPrivate
{
public:
    QFutureSignalListPrivate()
    {
        eventLoop = 0;
    }

    QList<QFutureSignal *> list;
    QEventLoop *eventLoop;
};

/*!
    Constructs a list of QFutureSignal objects.
*/
QFutureSignalList::QFutureSignalList()
{
    d = new QFutureSignalListPrivate();
}

/*!
    Destroys this list of QFutureSignal objects.
*/
QFutureSignalList::~QFutureSignalList()
{
    foreach (QFutureSignal *fs, d->list)
        delete fs;
}

/*!
    Adds a copy of \a fs to this list of QFutureSignal objects.
*/
void QFutureSignalList::addFutureSignal(const QFutureSignal& fs)
{
    d->list.append(new QFutureSignal(fs));
}

/*!
    Adds a QFutureSignal object for \a sender and \a signal to this list.
*/
void QFutureSignalList::addFutureSignal(QObject *sender, const QByteArray& signal)
{
    d->list.append(new QFutureSignal(sender, signal));
}

/*!
    Returns the number of items in this list.
*/
int QFutureSignalList::count() const
{
    return d->list.size();
}

/*!
    Returns the QFutureSignal object at \a index in this list.
    If \a index is invalid, returns null.
*/
QFutureSignal *QFutureSignalList::at(int index) const
{
    if (index >= 0 && index < d->list.size())
        return d->list[index];
    else
        return 0;
}

/*!
    Waits until any of the QFutureSignal objects on this list
    reach \a expectedCount signal emissions.  Returns true when
    this happens, or false if the \a timeout is reached first.

    \sa waitAll(), QFutureSignal::wait()
*/
bool QFutureSignalList::waitAny(int timeout, int expectedCount)
{
    // If the list is empty, then bail out because the condition
    // can never be satisfied.
    if (d->list.isEmpty())
        return false;

    // Return if we already have sufficient results on one of the signals.
    foreach (QFutureSignal *fs, d->list) {
        if (fs->resultCount() >= expectedCount)
            return true;
    }

    // Bail out if we were recursively re-entered.
    if (d->eventLoop)
        return false;

    // Run a nested event loop until the signal is emitted, or timeout.
    d->eventLoop = new QEventLoop();
    foreach (QFutureSignal *fs, d->list) {
        fs->d->eventLoop = d->eventLoop;
        fs->d->expectedCount = expectedCount;
    }
    if (timeout >= 0)
        QTimer::singleShot(timeout, d->eventLoop, SLOT(quit()));
    d->eventLoop->exec();
    delete d->eventLoop;
    d->eventLoop = 0;
    foreach (QFutureSignal *fs, d->list) {
        fs->d->eventLoop = 0;
    }

    // Determine if we got the result we were looking for before the timeout.
    foreach (QFutureSignal *fs, d->list) {
        if (fs->resultCount() >= expectedCount)
            return true;
    }
    return false;
}

/*!
    Waits until all of the QFutureSignal objects on this list
    reach \a expectedCount signal emissions.  Returns true when
    this happens, or false if the \a timeout is reached first.

    \sa waitAny(), QFutureSignal::wait()
*/
bool QFutureSignalList::waitAll(int timeout, int expectedCount)
{
    // If the list is empty, then bail out because the condition
    // can never be satisfied.
    if (d->list.isEmpty())
        return false;

    // Return if we already have sufficient results on all of the signals.
    bool tooSmall = false;
    foreach (QFutureSignal *fs, d->list) {
        if (fs->resultCount() < expectedCount) {
            tooSmall = true;
            break;
        }
    }
    if (!tooSmall)
        return true;

    // Bail out if we were recursively re-entered.
    if (d->eventLoop)
        return false;

    // Run a nested event loop until the signal is emitted, or timeout.
    d->eventLoop = new QEventLoop();
    foreach (QFutureSignal *fs, d->list) {
        fs->d->eventLoop = d->eventLoop;
        fs->d->expectedCount = expectedCount;
        fs->d->allList = this;
    }
    if (timeout >= 0)
        QTimer::singleShot(timeout, d->eventLoop, SLOT(quit()));
    d->eventLoop->exec();
    delete d->eventLoop;
    d->eventLoop = 0;
    foreach (QFutureSignal *fs, d->list) {
        fs->d->eventLoop = 0;
        fs->d->allList = 0;
    }

    // Determine if we got the result we were looking for before the timeout.
    tooSmall = false;
    foreach (QFutureSignal *fs, d->list) {
        if (fs->resultCount() < expectedCount) {
            tooSmall = true;
            break;
        }
    }
    return !tooSmall;
}

/*!
    Clears the result lists of all QFutureSignal objects on this list.

    \sa QFutureSignal::clear()
*/
void QFutureSignalList::clear()
{
    foreach (QFutureSignal *fs, d->list) {
        fs->clear();
    }
}

// Check to see if the "all" condition has been satisfied.
void QFutureSignalList::checkResults()
{
    foreach (QFutureSignal *fs, d->list) {
        if (fs->resultCount() < fs->d->expectedCount)
            return;
    }
    if (d->eventLoop)
        d->eventLoop->quit();
}
