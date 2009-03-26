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

#if !defined Q_QDOC

#include <qtopiasystemtest.h>

#include <qtopiasystemtestmodem.h>
#include <qtopiasystemtest_p.h>

#undef qLog
#define qLog(A) if (!verbose()); else (QDebug(QtDebugMsg) << #A ":")

#define BT(message) (\
    message["location"] = QString("%1:%2%3").arg(__FILE__).arg(__LINE__).arg(!message["location"].toString().isEmpty() ? "\n" + message["location"].toString() : ""),\
    message)

#define QUERY_RET(ret,msg,queryPath) do{\
    typeof(msg) _msg = msg;\
    QTestMessage _reply;\
    QTestMessage _btmsg(msg);\
    if (!doQuery(BT(_btmsg), queryPath, &_reply, 5000, "OK", "")) return ret;\
    if (!_reply[_msg].isValid()) {\
        _reply["status"] = "ERROR_NO_DATA_" + _reply["status"].toString();\
        setQueryError(_reply);\
        return ret;\
    }\
    ret = _reply[_msg].value<typeof(ret)>();\
    return ret;\
} while(0)

#undef QFAIL
#define QFAIL(message) \
do {\
    setQueryError(message);\
    return;\
} while(0)

/*!
    \enum QtopiaSystemTest::VersionType

    This enum specifies which version is returned by \l{getVersion()}.

    \value QtVersion            The version of Qt used on the test system.

    \value QtExtendedVersion    The version of Qt Extended used on the test system.

    \value KernelVersion        The version of the Linux kernel used on the test system.
*/

/*!
    \enum QtopiaSystemTest::MemoryType

    This enum specifies which type of memory to measure when using the memory
    related performance functions.

    \value NoMemoryType    Refers to no memory at all.

    \value TotalMemory     The total physical memory available to the test
                            system.

    \value FreeMemory      The free memory available to the test system.
                            Note that most operating systems, including Linux,
                            will intentionally consume almost all free memory
                            in buffers and caches, to improve filesystem
                            performance.  Therefore, the value returned by
                            FreeMemory will usually be very low.
                            A better measure of the effectively free memory
                            can be obtained by using EffectiveFreeMemory.

    \value EffectiveFreeMemory The memory which is either free, or used by
                            the kernel in such a way that it can probably be
                            easily freed if an application requests more memory.

    \value BuffersMemory   Memory currently consumed in kernel buffers.
                            The kernel is usually able to easily free most of
                            this memory if an application requests more memory.

    \value CacheMemory     Memory currently consumed in filesystem caches.
                            The kernel is usually able to easily free most of
                            this memory if an application requests more memory.

    \value ActiveMemory    Memory currently considered active by the kernel.
                            "Active" memory typically refers to memory that
                            would take some effort for the kernel to free.

    \value InactiveMemory  Memory currently considered inactive by the kernel.
                            "Inactive" memory typically refers to memory that
                            is backed by disk or able to be discarded easily
                            by the kernel.
*/

/*!
    \enum QtopiaSystemTest::SampledMemoryInfoFlag

    This enum specifies which data to retrieve from the recorded memory usage
    information when using the getSampledMemoryInfo() function.

    \value NoSampledMemoryInfoFlags  No flags.

    \value SampleMinimum             Return the minimum sampled value.
    \value SampleMaximum             Return the maximum sampled value.
    \value SampleMean                Return the mean of all sampled values.
                                        Note that this flag cannot be used with
                                        SampleFile or SampleLine.

    \value SampleFile                Return the file containing the test script
                                        which was running at the time this sample
                                        was taken.
    \value SampleLine                Return the line number of the test script
                                        at the time this sample was taken.
*/

/*!
    \preliminary
    \namespace QtopiaSystemTest
    \brief The QtopiaSystemTest namespace provides script based system test functionality for Qt Extended.

    \ingroup qtuitest_systemtest
    \inpublicgroup QtUiTestModule

    This documentation describes the API reference for the QtUiTest scripting language. Please read the \l{QtUiTest Manual} for a full description of the system test tool.

    All system tests which make use of the QtopiaSystemTest namespace also have access to the QSystemTest namespace.
*/

/*!
    \internal
    Creates the test class.
    Generally you would use the \l {QTest}{QTEST_MAIN()} macro rather than create the class directly.
*/
QtopiaSystemTest::QtopiaSystemTest()
    : QSystemTest()
    , d(new QtopiaSystemTestPrivate(this))
{}

/*!
    \internal
    Destroys the test class.
*/
QtopiaSystemTest::~QtopiaSystemTest()
{
    delete d;
    d = 0;
}

/*!
    Returns true if the specified \a application gains focus within
    \a timeout milliseconds.
*/
void QtopiaSystemTest::waitForCurrentApplication(QString const &application, int timeout)
{
    if (!timeout) timeout = visibleResponseTime();

    QString real_app = d->realAppName(application);
    if (real_app != currentApplication()) {
        QEventLoop loop;
        if (!connect(d, SIGNAL(appGainedFocus(QString)), &loop, SLOT(quit())))
            Q_ASSERT(0);
        if (timeout >= 0) QTimer::singleShot(timeout, &loop, SLOT(quit()));
        loop.exec();
    }
    if (currentApplication() != real_app) {
        fail(QString("%1 did not gain focus within %2 ms").arg(application).arg(timeout));
    }
}

/*!
    Send an IPC \a message on a Qt Extended IPC \a channel.
    The optional arguments \a arg0, \a arg1, \a arg2, \a arg3, \a arg4, \a arg5, \a arg6,
    \a arg7, \a arg8 and \a arg9 are arguments to the IPC message.

    Example:
    \code
        // Put a new document on the system and force a document rescan
        putFile("testdata/mydoc.txt", "$HOME/Documents/mydoc.txt");
        ipcSend("QPE/DocAPI", "scanPath(QString,int)", "all", 1);
    \endcode
*/
void QtopiaSystemTest::ipcSend( const QString &channel, const QString &message,
                            const QVariant &arg0, const QVariant &arg1, const QVariant &arg2,
                            const QVariant &arg3, const QVariant &arg4, const QVariant &arg5,
                            const QVariant &arg6, const QVariant &arg7, const QVariant &arg8,
                            const QVariant &arg9  )
{
    QTestMessage msg("ipcSend");
    msg["channel"] = channel;
    msg["message"] = message;

    QVariantList argList;
    if (arg0.isValid()) argList << arg0;
    if (arg1.isValid()) argList << arg1;
    if (arg2.isValid()) argList << arg2;
    if (arg3.isValid()) argList << arg3;
    if (arg4.isValid()) argList << arg4;
    if (arg5.isValid()) argList << arg5;
    if (arg6.isValid()) argList << arg6;
    if (arg7.isValid()) argList << arg7;
    if (arg8.isValid()) argList << arg8;
    if (arg9.isValid()) argList << arg9;

    msg["args"] = argList;

    doQuery(BT(msg), "qpe:");
}

/*!
    \internal
    Get the valuespace item specified by \a vsPath.
    Use this as a last resort only, since a user cannot see the value space.
*/
QVariant QtopiaSystemTest::getValueSpace( const QString &vsPath )
{
    QTestMessage message("getValueSpace");
    message["path"] = vsPath;
    QTestMessage reply;
    if (!doQuery(BT(message), "qpe:", &reply)) return QVariant();

    if (!reply.contains("getValueSpace")) {
        reply["status"] = "reply was missing return value";
        setQueryError(reply);
        return QVariant();
    }
    return reply["getValueSpace"];
}

/*!
    Restart Qtopia.

    This method instructs QPE to restart in the same way as the "Restart" option in the shutdown dialog.
    This is actually implemented by simply closing QPE and relying on the script which started it to
    restart it.  Hence whether or not this method succeeds depends on how QPE was launched.

    Example:
    \code
        // Restore Greenphone to factory defaults for the next test
        setSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "RestoreDefaults", "State", 1);
        restartQtopia();
        // Put back the setting so we don't restore to factory defaults after next restart
        setSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "RestoreDefaults", "State", 0);
    \endcode

    \sa reboot(), {Application Management}
*/
void QtopiaSystemTest::restartQtopia()
{
    resetCurrentApplication();

    bool was_demo_mode = demoMode();

    /* First, tell QtopiaServerApplication to restart. */
    QTestMessage message("restartQtopia");
    message["mode"] = 3;
    message["queryApp"] = "qpe";
    doQuery(message);

    // Avoid failures due to dialogs when restarting.
    bool was_ignoring = ignoreUnexpectedDialogs();
    setIgnoreUnexpectedDialogs(true);

    QTime t;
    t.start();
    while (isConnected() && t.elapsed() < 60000) {
        QTest::qWait(10);
    }
    d->got_startup_info = false;

    clearUnexpectedDialog();

    /* Now try to reconnect for up to 2 minutes. */
    t.start();
    while (!isConnected() && t.elapsed() < 120000) {
        connectToAut(10000);
    }

    setIgnoreUnexpectedDialogs(was_ignoring);


    if (!isConnected())
        QFAIL( "Couldn't re-establish connection with Qt Extended after restart." );

    if (was_demo_mode) setDemoMode(true);

    // wait until Qt Extended 'thinks' it's ready
    waitForQtopiaStart();
    if (queryFailed()) return;

    if (d->sample_memory_interval > 0) {
        startSamplingMemory(d->sample_memory_interval);
    }
}

/*!
    Shut down Qtopia.
*/
void QtopiaSystemTest::shutdownQtopia()
{
    if (!isConnected()) return;

    resetCurrentApplication();

    /* First, tell QtopiaServerApplication to shutdown. */
    QTestMessage message("restartQtopia");
    message["mode"] = 1;
    message["queryApp"] = "qpe";
    doQuery(message);

    setIgnoreUnexpectedDialogs(true);

    QTime t;
    t.start();
    while (isConnected() && t.elapsed() < 60000) {
        QTest::qWait(10);
    }
}

/*!
    Reboot the device running Qt Extended.

    This method instructs QPE to reboot the device in the same way as the "Reboot" option in the shutdown dialog.
    Whether or not this method succeeds depends on how QPE was launched and the permissions of the user account under
    which it is running.

    Example:
    \code
        // Start performance testing, and reboot Qt Extended to get the cold startup time of Qt Extended setSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "PerfTest", "State", 1);
        reboot();
    \endcode

    \sa restartQtopia(), {Application Management}
*/
void QtopiaSystemTest::reboot()
{
    resetCurrentApplication();

    bool was_demo_mode = demoMode();

    // Avoid failures due to dialogs when restarting.
    bool was_ignoring = ignoreUnexpectedDialogs();
    setIgnoreUnexpectedDialogs(true);

    /* First, tell QtopiaServerApplication to restart. */
    QTestMessage message("restartQtopia");
    message["mode"] = 2;
    message["queryApp"] = "qpe";
    doQuery( message );
    // Disconnect and wait 10 seconds.
    QTime t;
    t.start();
    disconnectFromAut();
    while (t.elapsed() < 5000 && isConnected()) {
        QTest::qWait(50);
    }

    QTest::qWait(5000);

    /* Now try to reconnect for up to 6 minutes. */
    t.start();
    while (!isConnected() && t.elapsed() < 360000) {
        connectToAut(30000);
    }

    setIgnoreUnexpectedDialogs(was_ignoring);

    if (!isConnected())
        QFAIL( "Couldn't re-establish connection with Qt Extended after reboot." );

    if (was_demo_mode) setDemoMode(true);

    // wait until Qt Extended 'thinks' it's ready
    waitForQtopiaStart();
    if (queryFailed()) return;

    if (d->sample_memory_interval > 0) {
        startSamplingMemory(d->sample_memory_interval);
    }
}

/*!
    \internal
    Waits for the valuespace item specified by \a vsPath to change to \a waitValue. If
    \a waitValue is an invalid QVariant, then it will simply wait for \i any change.

    If the item does not change to the specified value within \a timeout milliseconds, the function returns false.
    Otherwise, returns true.

    Example:
    \code
        // Wait up to 60 seconds for 'new contact' screen to come up
        QVERIFY( waitForValueSpace( "/UI/ActiveWindow/Caption", "New Contact", 60000 ) );
    \endcode

    Only use this as a last resort, since users can't access the value space.

    \sa QValueSpaceItem
*/
bool QtopiaSystemTest::waitForValueSpace( const QString &vsPath, const QVariant &waitValue, int timeout )
{
    // Clear the previous (if any) reply
    d->reply_msg = QTestMessage();

    QTestMessage message("waitValueSpace");
    message["value"] = waitValue;
    message["path"] = vsPath;

    QTestMessage reply;
    if (!doQuery(BT(message), "qpe:", &reply)) return false;

    QTime t;
    t.start();
    while ( d->reply_msg.event() != "valueSpaceResponse" && (timeout < 0 || t.elapsed() < timeout) )
        QTest::qWait(10);

    if (getValueSpace(vsPath) == waitValue)
        return true;

    if (d->reply_msg.event() != "valueSpaceResponse")
        return false;

    return true;
}

/*!
    Waits up to \a timeout milliseconds for Qt Extended to become fully initialized.
    On the desktop, the first call to this function will start Qt Extended using the runqtopia script.

    Once initialized:
    \list
    \o the Date format will be set to 'D/M/Y'.
    \o the Time format will be set to 24 hour.
    \endlist
*/
void QtopiaSystemTest::waitForQtopiaStart(int timeout)
{
    if (!launchQtopia()) return;

    static char const vsPath[]  = "/System/Tasks/WindowManagement/State";
    static char const vsValue[] = "Active";

    QTime t;
    t.start();
    bool ok = waitForValueSpace(vsPath, QVariant(vsValue), timeout );
    if (ok) {
        set12HourTimeFormat( false );
        setDateFormat();
    } else {
        setQueryError(QString("Qt Extended started and connected to the test framework, but didn't set the value space item '%1' to '%2' within %3 milliseconds.").arg(vsPath).arg(vsValue).arg(timeout));
        return;
    }

    for (; t.elapsed() < timeout && !d->got_startup_info; wait(500))
    {}

    if (!d->got_startup_info) {
        setQueryError(QString("Qt Extended started and connected to the test framework, but failed to send system info within %1 milliseconds.").arg(timeout));
        return;
    }
    qLog(QtUitest) << "Connected device:" << d->device;
}

/*!
    Start the specified \a application.

    \a application can be the name of a binary to execute on the test system (e.g. 'addressbook')
    or the full name of an application as shown in the launcher grid (e.g. 'Contacts').
    The application must connect to the test framework within \a timeout ms or
    the test fails.

    \a flags specifies additional behaviour.

    The application will be launched by sending a Qt Extended IPC message.
    To simulate launching an application by key clicks, use select( <appname>, launcherMenu() ).

    Example:
    \code
        // Start several apps and keep them in the background
        startApplication("Notes", 1000, BackgroundCurrentApplication);
        startApplication("Contacts", 1000, BackgroundCurrentApplication);
        startApplication("todolist", 1000, BackgroundCurrentApplication);

        // Now start btsettings without Background, which means all current apps
        // will be exited.
        startApplication("btsettings");
    \endcode

    \sa {Application Management}
*/
void QtopiaSystemTest::startApplication( const QString &application, int timeout, QSystemTest::StartApplicationFlags flags )
{
    Q_UNUSED(timeout);
    d->startApp(application, QString(), flags);
}

/*!
    Returns the path of the documents directory on the test system.

    Example:
    \code
        // Put a test image into the image directory in documents on the test system.
        putFile( baseDataPath() + "tiger.jpg", documentsPath() + "image/tiger.jpg");
    \endcode

    \sa QSystemTest::getFile(), QSystemTest::putFile(), QSystemTest::getData(), QSystemTest::putData(), {File Management}
*/
QString QtopiaSystemTest::documentsPath()
{
    QString ret;
    QUERY_RET(ret, "documentsPath", "qpe:");
}

/*!
    Returns the version number specified by \a type.

    Example:
    \code
        print( "Qt Extended Version: " + getVersion(QtExtendedVersion) );
        print( "Qt Version:          " + getVersion(QtVersion) );
        print( "Linux Version:       " + getVersion(KernelVersion) );
    \endcode

*/
QString QtopiaSystemTest::getVersion(VersionType type)
{
    QString versionType;
    versionType =   (type == QtVersion)     ? "QtVersion" :
                    (type == KernelVersion) ? "KernelVersion" : "QtExtendedVersion";

    QTestMessage message("getVersion");
    message["type"] = versionType;

    QTestMessage reply;
    if (!doQuery(BT(message), "qpe:", &reply)) return QString();
    if (!reply["getVersion"].isValid()) {
        fail("No data in reply to getVersion");
        return QString();
    }
    return reply["getVersion"].toString();
}

/*!
    Returns true if the test is running on an actual device, and false if it is running in a virtual framebuffer (on the desktop machine).
*/
bool QtopiaSystemTest::runsOnDevice()
{
    return autHost() != QString::fromLatin1("127.0.0.1");
}

/*!
    Close all currently running applications and return to the Home screen.

    \sa {Application Management}
*/
void QtopiaSystemTest::gotoHome()
{
    // we don't care about a possibly unexpected dialog ... we just close it.
    clearUnexpectedDialog();

    doQuery(QTestMessage("gotoHomeScreen"), "qpe:");
    waitForCurrentApplication("qpe", 10000);
}

/*!
    Background all currently running applications and return to the Home screen.

    \sa {Application Management}
*/
void QtopiaSystemTest::backgroundAndGotoHome()
{
    QTestMessage message("gotoHomeScreen");
    message["bg"] = true;
    if (!doQuery(BT(message), "qpe:")) return;
    waitForCurrentApplication("qpe", 10000);
}

/*!
    Get the QtopiaSystemTestModem currently used as the modem device.

    If Qt Extended is not currently connected to a QtopiaSystemTestModem, Qt Extended will be restarted
    and forced to connect to a new QtopiaSystemTestModem, which will be returned.

    The returned QtopiaSystemTestModem may be explicitly deleted if desired.  Otherwise,
    it will be destroyed when the system test completes.  In either case,
    when the QtopiaSystemTestModem is destroyed, Qt Extended will be restarted and will no longer
    connect to the simulated modem.

    Example:
    \code
        // Change operator name from within a system test
        QVERIFY( testModem() );
        testModem()->setVariable("OP", "Qt Extended Comms");
        testModem()->send("+CREG: 5");
    \endcode

    \sa restartQtopia(), {Telephony Simulation}
*/
QtopiaSystemTestModem *QtopiaSystemTest::testModem()
{
#ifndef QTUITEST_USE_PHONESIM
    return 0;
#else
    if (!d->test_modem) {
        d->test_modem = new QtopiaSystemTestModem(d);
    }
    if (!d->test_modem->isValid()) {
        delete d->test_modem; d->test_modem = 0;
    }
    return d->test_modem;
#endif
}

/*!
    \internal
    Enables or disables performance testing, depending on the value of \a on.

    Turning performance testing on or off may result in Qt Extended being restarted.
    If \a forceRestart is true, Qt Extended will always be restart (or, if not
    running on the desktop, the test device will be rebooted).  This is useful
    for measuring cold startup times.
*/
void QtopiaSystemTest::setPerformanceTest(bool on, bool forceRestart)
{
    waitForQtopiaStart();
    if (queryFailed()) return;

    bool needs_restart = false;

    if (on) {
        // FIXME this code is yucky.
        // This covers the greenphone...
        if (runsOnDevice() && getSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "PerfTest", "State").toInt() != 1) {
            if (queryFailed()) return;
            needs_restart = true;
            setSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "PerfTest", "State", 1);
            if (queryFailed()) return;
        }

        // And this covers the desktop...
        if (!runsOnDevice()) {
            needs_restart = d->ensureEnvironment("QTOPIA_PERFTEST", "1");
            if (queryFailed()) return;
            needs_restart = d->ensureEnvironment("QWS_DISPLAY",
                    "$(echo $QWS_DISPLAY | sed -r "
                    "-e 's|QVFb:|perftestqvfb:|' "
                    "-e 's|LinuxFb:|perftestlinuxfb:|'"
                    ")") || needs_restart;
            if (queryFailed()) return;
        }
    } else {
        if (runsOnDevice() && getSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "PerfTest", "State").toInt() != 0) {
            if (queryFailed()) return;
            needs_restart = true;
            setSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "PerfTest", "State", 0);
            if (queryFailed()) return;
        }

        if (!runsOnDevice()) {
            needs_restart = d->ensureEnvironment("QTOPIA_PERFTEST", QString());
            if (queryFailed()) return;
            needs_restart = d->ensureEnvironment("QWS_DISPLAY", QString()) || needs_restart;
            if (queryFailed()) return;
        }
    }

    needs_restart = needs_restart || forceRestart;

    if (needs_restart) {
        qWarning("%s to turn %s performance testing",
                (!runsOnDevice()) ? "Restarting Qt Extended" : "Rebooting",
                on ? "on" : "off");
        if (!runsOnDevice())
            restartQtopia();
        else
            reboot();
    }
}

/*!
    Measure and return the current memory usage, in kilobytes,
    on the test system for memory of the given \a type.

    If \a type is a flag containing multiple types, the memory usage
    of the desired types will be added together.

    Example:
    \code
        // Get the amount of effectively free memory on the system.
        function freeMemory() {
            return getMemoryInfo(FreeMemory|BuffersMemory|CacheMemory);
        }
    \endcode

    It is not necessary to call startSamplingMemory() before using this
    function.

    \sa startSamplingMemory(), stopSamplingMemory(), getSampledMemoryInfo()
    */
int QtopiaSystemTest::getMemoryInfo(MemoryTypes type)
{
    QTestMessage message("sampleMemory");
    message["on"] = true;

    QTestMessage reply;
    if (!doQuery(message, "qpe:", &reply)) return -1;

    QVariantMap vm = reply["values"].value<QVariantMap>();
    int total = 0;
    foreach (QString key, vm.keys()) {
        if (type & d->stringToMemoryType(key))
            total += vm.value(key).toInt();
    }

    return total;
}

/*!
    Start sampling memory at fixed intervals.

    Every \a interval milliseconds, the current memory usage will be
    sampled and stored.  Once sampling has finished, the data can be
    retrieved using getSampledMemoryInfo().

    Each memory sample causes network traffic between the system test
    and Qt Extended.  Therefore, using an extremely small interval will
    have a negative impact on performance.

    \sa stopSamplingMemory(), getMemoryInfo(), getSampledMemoryInfo()
    */
void QtopiaSystemTest::startSamplingMemory(int interval)
{
    QTestMessage message("sampleMemory");
    message["on"] = true;
    message["interval"] = interval;

    doQuery(message, "qpe:");
    if (!queryFailed()) {
        if (d->sample_memory_interval <= 0) {
            d->memorySamples.clear();
            d->sample_memory_interval = interval;
        }
    }
}

/*!
    Stop sampling memory.

    \sa startSamplingMemory(), getMemoryInfo(), getSampledMemoryInfo()
*/
void QtopiaSystemTest::stopSamplingMemory()
{
    QTestMessage message("sampleMemory");
    message["on"] = false;

    doQuery(message, "qpe:");
    if (!queryFailed()) {
        d->sample_memory_interval = -1;
    }
}

/*!
    Get information about sampled memory for the specified memory \a types,
    using the given \a flags to decide what data to return.

    Use startSamplingMemory(), stopSamplingMemory() and this function to
    measure the effective memory usage of particular use cases of Qt Extended.

    For example:

    \code
    spin_the_yoyo: function() {
        var total_memory = getMemoryInfo(TotalMemory);

        // Ensure we start measuring from the home screen
        gotoHome();
        startSamplingMemory(500);

        // Spin the yoyo
        startApplication("yoyo-manager");
        select("Spin");
        waitForTitle("Spin complete");

        // Measure the memory used to spin the yoyo
        stopSamplingMemory();
        var min_free = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum);
        var min_free_line = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum|SampleLine);
        print("Max used memory: " + (total_memory - min_free) + " Kb, at line " + min_free_line);
    }
    \endcode
*/
QVariant QtopiaSystemTest::getSampledMemoryInfo(MemoryTypes types, SampledMemoryInfoFlags flags)
{
    QVariant ret;
    if ((flags & SampleMean) && ((flags & SampleFile) || (flags & SampleLine))) {
        setQueryError("Cannot get file or line for mean!");
        return ret;
    }

    int grand_total = 0;
    int min = -1;
    QString min_file;
    int min_line = 0xBEEFFACE;
    int max = -1;
    QString max_file;
    int max_line = 0xBEEFFACE;
    qreal mean = 0.0;


    for (int i = 0; i < d->memorySamples.count(); ++i) {
        QtopiaSystemTestPrivate::MemorySample const& s = d->memorySamples.at(i);

        int total = 0;
        foreach (QString key, s.values.keys()) {
            if (types & d->stringToMemoryType(key))
                total += s.values.value(key).toInt();
        }

        if (-1 == min || total < min) {
            min = total;
            min_file = s.file;
            min_line = s.line;
        }

        if (-1 == max || total > max) {
            max = total;
            max_file = s.file;
            max_line = s.line;
        }

        grand_total += total;
    }

    if (d->memorySamples.count())
        mean = qreal(grand_total) / qreal(d->memorySamples.count());

    if (flags & SampleMean) {
        ret = mean;
    } else if (flags & SampleMaximum) {
        if (flags & SampleFile) {
            ret = max_file;
        } else if (flags & SampleLine) {
            ret = max_line;
        } else {
            ret = max;
        }
    } else if (flags & SampleMinimum) {
        if (flags & SampleFile) {
            ret = min_file;
        } else if (flags & SampleLine) {
            ret = min_line;
        } else {
            ret = min;
        }
    }

    return ret;
}

/*!
    \internal
    Processes the command line parameters.
*/
void QtopiaSystemTest::processCommandLine( int &argc, char *argv[] )
{
    QString launchPath = argv[0];
    launchPath = launchPath.left(launchPath.lastIndexOf('/'));

    d->qtopia_script = launchPath + "/runqtopia -test -runmode systemtesthelper";

    QSystemTest::processCommandLine(argc, argv);
}

/*!
    \internal
    Print any special usage information which should be shown when test is launched
    with -help.
*/
void QtopiaSystemTest::printUsage(int argc, char *argv[]) const
{
    QSystemTest::printUsage(argc, argv);
}

/*!
    \internal
    Attempts to launch the aut script, and returns whether it was successful.

    The \c -remote command line argument is appended to the script with the IP set to the address of the
    local machine. This is to allow the test system to connect to the machine the test is running on.
    Any scripts used should support this.
*/
bool QtopiaSystemTest::launchQtopia()
{
    if (connectToAut(runsOnDevice() ? 60000 : 5000)) return true;

    qLog(QtUitest) << "Couldn't connect to Qt Extended.";

    if (!d->qtopia_script.isEmpty() && !runsOnDevice()) {
        // Attempt to kick off Qt Extended
        QString error;
        QString command = d->qtopia_script;

        qLog(QtUitest) << qPrintable(QString("Launching Qt Extended using: %1").arg(command)) ;
        QSystemTest::startApplication(command, 60000);
        if (queryFailed()) return false;
    } else {
        setQueryError("Could not connect to Qt Extended. " +
                (runsOnDevice()
                    ? QString("Please ensure device has QtUiTest installed and enabled.")
                    : QString("Please specify a script for launching Qt Extended."))
        );
        return false;
    }

    return true;
}

/*!
    Returns an alias that will be resolved by Qt Extended to find the Launcher Menu.
    Example:
    \code
        select( "Contacts", launcherMenu() );
    \endcode

    \sa softMenu(), callAccept(), callHangup()
*/
QString QtopiaSystemTest::launcherMenu()
{
    return LAUNCHER_MENU_ALIAS;
}

/*!
    Returns an alias that will be resolved by Qt Extended to find the Options Menu.
    Example:
    \code
        select( "New", optionsMenu() ); // select the item with text 'New' from the options menu
        print( signature(optionsMenu()) ); // to print the name of the resolved widget.
    \endcode
    Note that 'signature(optionsMenu())' incurs an extra roundtrip and should only be used in exceptional cases.

    The test will fail if no options menu can be activated, or if multiple option menus are found.

    \sa softMenu(), callAccept(), callHangup()
*/
QString QtopiaSystemTest::optionsMenu()
{
    return OPTIONS_MENU_ALIAS;
}

/*!
    Returns an alias that will be resolved by Qt Extended to find the Soft Menu.
    Example:
    \code
        select( "Help", softMenu() ); // select the item with text 'Help' from the soft menu
        print( signature(softMenu()) ); // to print the name of the resolved widget.
    \endcode
    Note that 'signature(softMenu())' incurs an extra roundtrip and should only be used in exceptional cases.

    The test will fail if no visible soft menu is found, or if multiple soft menus are found.

    \sa callAccept(), callHangup()
*/
QString QtopiaSystemTest::softMenu()
{
    return SOFT_MENU_ALIAS;
}

/*!
    Returns an alias that will be resolved by Qt Extended to activate the callAccept function, either by simulating a Call-Accept keyclick
    or by selecting a Call Accept somehow on a touchscreen.
    Example:
    \code
        select(callAccept()); // presses the CallAccept button
    \endcode
    The test will fail if no Call Accept can be simulated.

    \sa optionsMenu(), softMenu(), callHangup()
*/
QString QtopiaSystemTest::callAccept()
{
    return CALL_ACCEPT_ALIAS;
}

/*!
    Returns an alias that will be resolved by Qt Extended to activate the callHangup function, either by simulating a Call-Hangup keyclick
    or by selecting a Call Hangup somehow on a touchscreen.
    Example:
    \code
        select(callHangup()); // presses the CallHangup button
    \endcode
    The test will fail if no Call Hangup can be simulated.

    \sa optionsMenu(), softMenu(), callAccept()
*/
QString QtopiaSystemTest::callHangup()
{
    return CALL_HANGUP_ALIAS;
}

/*!
    \internal
    Does whatever is necessary to select the given messagebox \a option
    from the currently shown \a messageBox .
    For Qtopia, uses the soft menu.
*/
void QtopiaSystemTest::selectMessageBoxOption(QString const& option, QString const& messageBox)
{
    // If there is a soft menu, use that, otherwise use the focused widget.
    if (!isVisible(softMenu())) {
        if (queryFailed()) return;
        QSystemTest::selectMessageBoxOption(option, messageBox);
        return;
    }

    // wait for option to become available
    QStringList options = getList(softMenu());
    if (queryFailed()) return;

    QTime t;
    t.start();
    while (!options.contains(option)) {
        QTest::qWait(200);
        if (t.elapsed() > 10000) {
            fail( "Timeout while waiting for option '" + option + "' to appear in soft menu" );
            return;
        }
        options = getList(softMenu());
        if (queryFailed()) return;
    }
    select( option, softMenu() );
}

/*!
    \internal
    Call this to start forwarding log messages read from runqtopia to log.js.
    Only makes sense when running on the desktop.
*/
void QtopiaSystemTest::startDirectLogRead()
{ d->direct_log_read = true; }

/*! \internal */
void QtopiaSystemTest::applicationStandardOutput(QList<QByteArray> const& lines)
{
    if (!d->direct_log_read) {
        foreach (QByteArray const& line, lines) d->log_buffer.append(line);
        return;
    }

    // Avoid having to write QtScript conversions for QList<QByteArray>
    QStringList strlines;
    for (int i = 0, c = d->log_buffer.count(); i < c; ++i)
        strlines << QString::fromLatin1(d->log_buffer.at(i));
    d->log_buffer.clear();
    foreach (QByteArray const& line, lines)         strlines << QString::fromLatin1(line);

    QTestMessage message("log");
    message["lines"] = QVariant::fromValue(strlines);
    processMessage(message);
}

/*! \internal */
void QtopiaSystemTest::applicationStandardError(QList<QByteArray> const& lines)
{ applicationStandardOutput(lines); }

/*!
    \internal
    Called for each incoming message from Qt Extended.
*/
void QtopiaSystemTest::processMessage(const QTestMessage& message)
{
    QTestMessage copy(message);
    if (!d->processMessage(&copy))
        QSystemTest::processMessage(message);
}

#endif

#include "qtopiasystemtest.moc"

