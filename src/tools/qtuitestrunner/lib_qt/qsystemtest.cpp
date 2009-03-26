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

#include <qsystemtest.h>

#include "qsystemtestmaster_p.h"

#include "qsystemtest_p.h"
#include "gracefulquit.h"
#include "ui_recorddlg.h"
#include "ui_manualverificationdlg.h"
#include "ui_failuredlg.h"

#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QSignalSpy>
#include <QSettings>

#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#undef qLog
#define qLog(A) if (!d->verbose); else (QDebug(QtDebugMsg) << #A ":")

#define OBJECT_EXIST_TIMEOUT 1000

#define BT(message) (\
    message["location"] = QString("%1:%2%3").arg(__FILE__).arg(__LINE__).arg(!message["location"].toString().isEmpty() ? "\n" + message["location"].toString() : ""),\
    message)

#undef QCHECKED
#define QCHECKED(code) \
do {\
    {\
        code;\
    }\
    if (queryFailed()) return;\
} while(0)

#undef QFAIL
#define QFAIL(message) \
do {\
    setQueryError(message);\
    return;\
} while(0)

// DON'T use these in QSYstemTest.cpp
#undef QVERIFY
#undef QVERIFY2

#define DEFAULT_AUT_PORT 5656

/*!
    \enum QSystemTest::EnterMode

    This enum specifies whether enter() should commit the entered value (eg, by simulating a Select
    keypress) or not.

    \value Commit          Commit the value (default).
    \value NoCommit        Do not commit the value.
*/

/*!
    \enum QSystemTest::StartApplicationFlag

    This enum describes additional behaviour to use when starting applications
    by startApplication().

    \value NoFlag          Don't apply any of the below flags.
    \value WaitForFocus    Wait for the application to gain keyboard focus before returning.
    \value BackgroundCurrentApplication Use multitasking to background the current application.
                            The default behaviour is to exit the current application.
*/

/*!
    \enum QSystemTest::TimeSynchronizationMode

    This enum defines the mode that is used to synchronize the device system time.

    \value HostTimeSynchronization Switches time synchronization to manual and syncs the device to the host machine time.
    \value ManualTimeSynchronization Switches time synchronization to manual.
    \value AutoTimeSynchronization Switches time synchronization to automatic. The device will use the network time as provided by a Phone Network to set the device system time.
*/

/*!
    \enum QSystemTest::SkipMode

    This enum describes the modes for skipping tests during execution of the test data.
    \value SkipSingle Skip the rest of this test function for the current test data entry, but continue execution of the remaining test data.
    \value SkipAll Skip the rest of this test function, and do not process any further test data for this test function.

    \sa skip(), QTest::SkipMode
*/

/*!
    \internal
    \macro DUMP_OBJECTS(queryPath)
    \relates QSystemTest

    Outputs a string of all the objects that are children of the object specified by \a queryPath.
    If only the application part of \a queryPath is defined, then all objects for that application
    are listed. The string is organized in a tree type hierarchy showing the relationship between
    objects.

    This macro is intended to help with the development of testcases.
*/

/*!
    \internal
    \macro DUMP_WIDGETS(queryPath)
    \relates QSystemTest

    Outputs a string of all the widgets that are children of the object specified by \a queryPath.
    If only the application part of \a queryPath is defined, then all widgets for that application
    are listed. The string is organized in a tree type hierarchy showing the relationship between
    widgets.

    This macro is intended to help with the development of testcases.
*/

/*!
    \internal
    \macro QEXPECT_FAIL_UNTIL_QTOPIAVERSION( version, dataIndex, comment, mode )
    \relates QSystemTest

    If the current Qt Extended version is less than \a version, do nothing.  Otherwise,
    mark the next QCOMPARE() or QVERIFY() as an expected failure. Instead of
    adding a failure to the test log, an expected failure will be reported.

    If a QVERIFY() or QCOMPARE() is marked as an expected failure, but passes
    instead, an unexpected pass (XPASS) is written to the test log.

    The parameter \a dataIndex describes for which entry in the test data the
    failure is expected. Pass an empty string ("") if the failure is expected
    for all entries or if no test data exists.

    \a comment will be appended to the test log for the expected failure.

    \a mode is a QTest::TestFailMode and sets whether the test should continue to execute or not.

    Example:
    \code
        // DogWalker known to be broken, will be fixed in Qtopia 4.3.0
        QEXPECT_FAIL_UNTIL_QTOPIAVERSION("4.3.0", "", "Bug 123", Continue);
        DogWalker walker;
        QVERIFY(walker.walkies());
    \endcode
*/

/*!
    \preliminary
    \namespace QSystemTest
    \brief The QSystemTest namespace provides script based system test functionality for Qt.

    \ingroup qtuitest_systemtest
    \inpublicgroup QtUiTestModule

    This documentation describes the API reference for the QtUiTest scripting language. Please read the \l{QtUiTest Manual} for a full description of the system test tool.

*/

/*! \internal */
QMap<QString, int> QSystemTest::filteredMessages() const
{
    return d->filteredMessages;
}

/*! \internal */
void QSystemTest::clearFilteredMessages()
{
    d->filteredMessages.clear();
}

/*! \internal */
bool QSystemTest::shouldFilterMessage(char const *msg)
{
    static QList<QRegExp> filters;
    if (filters.isEmpty()) {
        QStringList defaultFilters;
        defaultFilters
            << "^Connected to VFB server"
            << "^QTimeLine::start: already running$" // Bug
        ;
        QStringList stringFilters = QSettings("Trolltech", "QtUitest")
                                .value("Logging/Filters", defaultFilters)
                                .toStringList();
        foreach (QString s, stringFilters) {
            filters << QRegExp(s);
        }
    }

    QString message = QString::fromLocal8Bit(msg);
    foreach (QRegExp r, filters) {
        if (-1 != r.indexIn(message)) {
            ++d->filteredMessages[message];
            return true;
        }
    }
    return false;
}

/*!
    \internal
    Creates the test class.
    Generally you would use the \l {QTest}{QTEST_MAIN()} macro rather than create the class directly.
*/
QSystemTest::QSystemTest()
    : QAbstractTest()
    , d(new QSystemTestPrivate(this))
{
    QSystemTestPrivate::singleton = this;
    qRegisterMetaType<QTestMessage>("QTestMessage");
    qRegisterMetaType<QTestMessage*>("QTestMessage*");
}

/*!
    \internal
    Destroys the test class.
*/
QSystemTest::~QSystemTest()
{
    delete d;
    QSystemTestPrivate::singleton = 0;
}

/*!
    Returns the name of the specific widget with input focus specified by \a {queryPath}. If \a queryPath refers to a widget, then the name of the child widget with focus will be returned, otherwise all widgets in the specified application will be queried.

    Example:
    \code
        // Get the currently focused widget in the current app
        print( focusWidget() );

        // Get the currently active widget in textedit
        var foo = focusWidget("qpe/textedit:");

        // Get the currently active widget on the home screen
        print( focusWidget("qpe:PhoneLauncher[abcde]/HomeScreen[12bd7f8]") );
    \endcode

    \sa {Query Paths}
*/
QString QSystemTest::focusWidget( const QString &queryPath )
{
    QString ret;
    return queryWithReturn(ret, "focusWidget", queryPath);
}


/*!
    Returns the currently selected text from the widget specified by \a {queryPath}.
    If no text is selected, returns all text.
    For list-type widgets, returns the text for the currently selected item.

    Example:
    \code
        // Enter text in two fields, then go back to the first
        // and make sure it still has the right text
        enter("Australia", "Home");
        enter("dog walker", "Occupation");
        compare( getSelectedText("Home"), "Australia" );
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
QString QSystemTest::getSelectedText( const QString &queryPath )
{
    QString ret = "";
    return queryWithReturn(ret, "getSelectedText", queryPath);
}

/* FIXME write an example for below */
/*!
    Returns all text from the widget specified by \a {queryPath}.
    For list-type widgets, returns the text for all items separated by newlines.

    Example:
    \code
        // Get current content of "name" field
        select("Name");
        print( getText() );

        // Get current content of "address" field, without navigating to it
        print( getText("Address") );
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
QString QSystemTest::getText( const QString &queryPath )
{
    QString ret;
    return queryWithReturn(ret, "getText", queryPath);
}

/*!
    Returns true if the primary input method is mouse/touchscreen.
    \sa Qtopia::mousePreferred()
*/
bool QSystemTest::mousePreferred()
{
    return d->mousePreferred;
}

/*!
    Returns a list of all items from the list-type widget specified by \a {queryPath}.

    The items will be returned in the order they are stored in the widget (for example,
    for a simple list view the items will be returned from top to bottom).

    Example:
    \code
        // Verify that "gender" combobox contains male and female only
        var g = getList("Gender");
        compare( g.length == 2 );
        verify( g.contains("Male") );
        verify( g.contains("Female") );
    \endcode

    getList() can also be used to query for lists from widgets such as the options Menu.
    Example:
    \code
        var L = getList(optionsMenu());
    \endcode
    This function will temporarily open the options menu, take a snapshot of the list contents, close the menu again and finally
    return the list. If the options menu was open already it will be left open.

    \sa {Query Paths}, {Querying Objects}
*/
QStringList QSystemTest::getList( const QString &queryPath )
{
    QStringList ret;
    return queryWithReturn(ret, "getList", queryPath);
}

/*!
    Returns a list of all the labels that are visible in the current active window or the widget specified by \a {queryPath}.
    A label is usually a non-editable widget (such as a QLabel) that is associated with an editable field. The label is used to
    give the user a visual clue of the meaning of the editable field. Labels are used by the user, and by QtUitest, to
    navigate to fields.

    The items will be returned in the order they are stored in the widget, i.e. from top to bottom.

    Example:
    \code
        // Verify that the current dialogs contains Labels named 'Name' and 'Email'
        var g = getLabels();
        QVERIFY( g.length == 2 );
        QVERIFY( g.contains("Name") );
        QVERIFY( g.contains("Email") );
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
QStringList QSystemTest::getLabels( const QString &queryPath )
{
    QStringList ret;
    return queryWithReturn(ret, "getLabels", queryPath);
}

/*!
    Returns text stored in the clipboard, or an empty string if the clipboard does not contain any text.

    \sa setClipboardText()
*/
QString QSystemTest::getClipboardText( )
{
    QString ret;
    return queryWithReturn(ret, "getClipboardText", "");
}

/*!
    Copies \a {text} into the clipboard.

    \sa getClipboardText()
*/
void QSystemTest::setClipboardText( const QString& text )
{
    QTestMessage message("setClipboardText");
    message["text"] = text;
    d->queryPassed( "OK", "", BT(message), "qpe:");
}

/*!
    Returns the current window title for the application specified by \a {queryPath}.
    If \a queryPath contains a widget component, it will be ignored.

    Example:
    \code
        startApplication("Contacts") );

        ...

        // Make sure we are still in contacts
        compare( currentTitle(), "Contacts" );
    \endcode

    \sa {Query Paths}
*/
QString QSystemTest::currentTitle( const QString &queryPath )
{
    QString ret;
    return queryWithReturn(ret, "currentTitle", queryPath);
}

/*!
    Returns the name of the application which currently has keyboard focus.
    The name will be the name returned by QCoreApplication::applicationName(),
    which is typically the name of the executable file.

    Example:
    \code
        startApplication("Contacts");

        ...

        // Make sure we are still in contacts
        compare( currentApplication(), "addressbook" );
    \endcode
*/
QString QSystemTest::currentApplication()
{
    // If we know what the current app is, just return it.
    // Otherwise ask Qtopia to tell us the current app.
    if (d->current_application.isEmpty()) {
        QTestMessage message("currentApplication");
        QTestMessage reply = d->query(BT(message), "qpe:");
        QString ret;
        if (!queryFailed()) {
            ret = reply["currentApplication"].toString();
            if (ret.isEmpty()) {
                setQueryError("cannot get name of current application");
            } else {
                d->current_application = ret;
            }
        }
    }
    return d->current_application;
}

/*!
    \internal
*/
void QSystemTest::resetCurrentApplication()
{ d->current_application = QString(); }


/*!
    \internal
    Returns the value of the environment variable for \a key set on the test
    system.
*/
QString QSystemTest::getenv(QString const& key)
{
    QTestMessage message("getenv");
    message["key"] = key;

    QTestMessage reply;
    if (!d->queryPassed( "OK", "", BT(message), "qpe:", &reply )) return QString();
    if (!reply["getenv"].isValid()) {
        fail("No data in reply to getenv");
        return QString();
    }
    return reply["getenv"].toString();
}

/*!
    \internal
    Grabs a snapshot of the widget specified by \a {queryPath}, optionally excluding \a maskedWidgets from the snapshot.

    To get a snapshot of the entire screen, use the query path "qpe:".

    Example:
    \code
        // Get current screenshot and save to disk
        QImage img = grabImage("qpe:");
        QVERIFY( img.save(currentDataPath() + "/snapshot.png", "PNG") );
    \endcode

    \sa {Query Paths}, verifyImage(), saveScreen()
*/
QImage QSystemTest::grabImage(const QString &queryPath, const QStringList &maskedWidgets ) {
    QImage im;

    // Now query Qtopia for the current snapshot
    QTestMessage message("grabPixmap");
    message["mask"] = maskedWidgets;

    QTestMessage reply = d->query( BT(message), queryPath );
    if (reply["status"] != "OK" || queryFailed()) {
        fail( "Couldn't grab image: " + reply.toString());
    } else {
        if (!reply["grabPixmap"].isValid()) {
            fail( "Test slave returned no image" );
        } else {
            im = reply["grabPixmap"].value<QImage>();
        }
    }

    return im;
}

/*!
    Grabs a snapshot of the widget specified by \a {queryPath}, and compares it against the reference snapshot \a expectedName.

    New snapshots are gathered by running the test in \l {Learn Mode}{learn mode}, and are stored in the \c testdata subdirectory of the directory containing the test script.  When learn mode is used, a failed image comparison will result in a tester being presented with a manual verification dialog.

    If there is a mismatch between the images, the current test fails.

    When in learn mode, if \a comment is provided it will be shown to the user to help in determining whether or not the pixmap should be accepted.

    \a maskedWidgets is a list of query paths specifying widgets to be excluded from the snapshot.  This allows constantly changing widgets to be hidden from view while the snapshot is taken.

    Example:
    \code
        select("Mark task complete", optionsMenu());
        verifyImage( "task_completed", "", "Verify that the current task is shown with a green tick indicating completion" );
    \endcode

    \sa saveScreen()
*/
void QSystemTest::verifyImage( const QString &expectedName, const QString &queryPath, const QString &comment, const QStringList &maskedWidgets )
{
    // Determine the filename
    // If the function was passed an explicit filename (indicated by an extension) it will be used directly
    // otherwise system specific values are prepended to the name.
    QString expectedFilename = currentDataPath();
    if ( expectedName.endsWith( ".png" ) ) {
        expectedFilename += "/" + expectedName;
    } else {
        expectedFilename += QString("/%1_%2.png")
            .arg( d->config_id )
            .arg( expectedName );
    }

    // The reference snapshot should exist in the data directory for this testcase.
    // If it's not there, and we're not in learn mode, we will fail.
    if ( (learnMode() == LearnNone || d->auto_mode) && !QFile::exists(expectedFilename) ) {
        fail(QString("Reference snapshot '%1' doesn't exist yet. Please manually run test in learn mode.").arg(expectedFilename));
        return;
    }

    // Now query Qtopia for the current snapshot
    QImage actualIm( grabImage(queryPath, maskedWidgets) );
    if (queryFailed()) return;

    bool snapshotOk = false;
    QDir("/").mkpath(currentDataPath());
    QImage expectedIm(expectedFilename);

    // Now do the actual comparisons.
    // If we are not in learn mode, a difference in snapshots will cause a failure.
    // If we are in learn mode, a difference will cause a user prompt to accept the new snapshot.
    // If we are in learn-all mode, the prompt will be displayed every time.

    if ( (learnMode() != LearnAll || d->auto_mode) && QFile::exists(expectedFilename) ) {
        snapshotOk = d->imagesAreEqual(actualIm, expectedIm);
    }

    if ( !d->auto_mode && !snapshotOk && learnMode() != LearnNone ) {
        if ( d->learnImage(actualIm, expectedIm, comment) ) {
            QFile::remove(expectedFilename);
            if ( !actualIm.save(expectedFilename, "PNG", 0) ) {
                QWARN(QString("Failed to save image to %1!").arg(expectedFilename).toLatin1());
            } else {
                if (d->qtest_ide.isConnected()) {
                    QTestMessage msg("NEW_TESTDATA");
                    msg["filename"] = expectedFilename;
                    d->qtest_ide.postMessage(msg);
                }
            }
            snapshotOk = true;
        } else {
            fail( "New image not accepted by tester" );
            return;
        }
    }

    if (!snapshotOk) {
        // Failure, so rename the snapshot to identify it for future reference
        expectedFilename.replace(".png", "_failure.png");
        if ( !actualIm.save(expectedFilename, "PNG", 0) ) {
            QWARN(QString("Failed to save failure pixmap to %1!").arg(expectedFilename).toLatin1());
        }
        fail( "Snapshots are not the same" );
        return;
    }

    /* By using compare, we go through the "expected failure" processing. */
    /* Without this, we won't get XPASS */
    QTest::compare_helper( true, "Snapshots are the same", qPrintable(currentFile()), currentLine() );
}

/*!
    Compares the widget specified by \a {queryPath} against the reference snapshot \a expectedName.

    \a maskedWidgets is a list of query paths specifying widgets to be excluded from the snapshot.  This allows constantly changing widgets to be hidden from view while the snapshot is taken.

    Returns true if the images match, or false otherwise.

    The reference snapshot can be one previously learned using verifyImage(), or an image saved using saveScreen(), in which case
    the .png filename extension must be specified.

    \sa verifyImage(), saveScreen()
*/
bool QSystemTest::compareImage( const QString &expectedName, const QString &queryPath, const QStringList &maskedWidgets )
{
    // Determine the filename
    // If the function was passed an explicit filename (indicated by an extension) it will be used directly
    // otherwise system specific values are prepended to the name.
    QString expectedFilename = currentDataPath();
    if ( expectedName.endsWith( ".png" ) ) {
        expectedFilename += "/" + expectedName;
    } else {
        expectedFilename += QString("/%1_%2.png")
            .arg( d->config_id )
            .arg( expectedName );
    }

    // The reference snapshot should exist in the data directory for this testcase.
    if ( !QFile::exists(expectedFilename) ) {
        fail(QString("Reference snapshot '%1' doesn't exist.").arg(expectedFilename));
        return false;
    }

    // Now query Qtopia for the current snapshot
    QImage actualIm( grabImage(queryPath, maskedWidgets) );
    if (queryFailed()) return false;

    QImage expectedIm(expectedFilename);

    // Now do the actual comparisons.
    return d->imagesAreEqual(actualIm, expectedIm);
}

/*!
    Reads \a srcFile from the test system and returns its contents.
    if \a srcFile contains environment variables, they will be expanded on the test system.

    \sa {File Management}, getFile(), putData(), putFile()
*/
QString QSystemTest::getData( const QString &srcFile )
{
    QTestMessage message("getFile");
    message["path"] = srcFile;

    QTestMessage reply;
    if (!d->queryPassed( "OK", "", BT(message), "qpe:", &reply )) return QString();
    if (!reply["getFile"].isValid()) {
        fail("No data in reply to getData");
        return QString();
    }
    return reply["getFile"].toString();
}

/*!
    \internal
    Returns image size of image specified in \a srcFile.
    if \a srcFile contains environment variables, they will be expanded on the test system.

    Note: QSize does not have toString(), see QSize docs for methods returning common types.

    Example:
    \code
        // Find the image size of specified image
        var imgSize = getImageSize( documentsPath() + "image/foo.jpg" );
        prompt( "Image size is: " + imgSize.width() + " by " + imgSize.height() + ".\n" );
    \endcode
*/
QSize QSystemTest::getImageSize( const QString &srcFile)
{
    QTestMessage message("getImageSize");
    message["path"] = srcFile;

    QTestMessage reply;
    if (!d->queryPassed( "OK", "", BT(message), "qpe:", &reply )) return QSize();
    if (!reply["getImageSize"].isValid()) {
        fail("No data in reply to getImageSize");
        return QSize();
    }
    return reply["getImageSize"].value<QSize>();
}
/*!
    Returns geometry of the widget specified in \a queryPath, with position (x,y) co-ordinates being global.
    Note: QRect does not have toString() method, refer to QRect docs for methods returning common types.

    Example:
    \code
        // pass the test if widgets do not overlap
        var first_widget = getGeometry("Button1");
        var second_widget = getGeometry("Button2");
        // intersects returns true on overlap, false when not; verify causes test to fail on false
        verify( !first_widget.intersects(second_widget), "Specified widgets overlap.");
    \endcode
    Example two - a non-mainstream situation:

        select() may work in an undefined manner with custom widgets/items, implementing custom select() methods isn't ideal - each would require writing and testing.
        On a device with a primary input method of mouse/touchscreen there may not be key code mapping for keys which don't exist - therefore mouse events should be used. However devices may have different geometry, and widget geometry can change between invocations. The example below uses mouseClick() without prior geometry knowledge, though a way is needed to determine where to click, the example shows mouseClick() in the middle of an area defined by the 4th col and 4th row in a uniform grid of the area of the active widget.
    \code
        // mouseClick() a widget or item with a fixed position inside its parent widget
        var geo = getGeometry();
        var select_x = geo.x() + (( geo.width() / 8) * 7);
        var select_y = geo.y() + (( geo.height() / 8) * 7);
        mouseClick(select_x, select_y);
    \endcode

    \sa select(), mouseClick(), QRect
*/
QRect QSystemTest::getGeometry( const QString &queryPath )
{
    QRect ret;
    return queryWithReturn(ret, "getGeometry", queryPath);
}

/*!
    Retrieves \a srcFile from the test system and copies it to \a destFile on the local machine.
    if \a srcFile contains environment variables, they will be expanded on the test system.

    Example:
    \code
        // Copy a settings file to the local machine
        getFile("$HOME/Settings/foo.conf", "/tmp/foo.conf" );
    \endcode

    \sa {File Management}, getData(), putData(), putFile()
*/
void QSystemTest::getFile( const QString &srcFile, const QString &destFile )
{
    QTestMessage message("getFile");
    message["path"] = srcFile;

    QTestMessage reply;
    if (!d->queryPassed( "OK", "", BT(message), "qpe:", &reply )) return;
    if (!reply["getFile"].isValid()) {
        reply["status"] = "ERROR_MISSING_DATA";
        QFAIL(reply);
    }
    QByteArray data = reply["getFile"].toByteArray();
    QFile out(destFile);
    if (!out.open(QIODevice::WriteOnly|QIODevice::Truncate))
        QFAIL( "Couldn't open local file '" + destFile + "'");
    qint64 b;
    for (b = out.write(data); b < data.size() && -1 != b; ++b) {
        qint64 this_b = out.write(data.mid(b));
        if (-1 == this_b) b  = -1;
        else              b += this_b;
    }
    if (-1 == b)
        QFAIL( "Couldn't write to local file '" + destFile + "'");
}

/*!
    Transfers \a srcFile from the local machine and copies it to \a destFile on the test system.
    if \a destFile contains environment variables, they will be expanded on the test system.

    By default, the file permissions of the destination file will be set to those of the source
    file. This can be overridden by specifying \a permissions.

    Example:
    \code
        // Force test system to use certain settings
        putFile("testdata/my_settings.conf", "$HOME/Settings/foo.conf");

        // Specify file permissions
        putFile("testdata/my_file", "$HOME/my_file", QFile.WriteOwner | QFile.ReadOwner | QFile.ReadOther);
    \endcode

    \sa {File Management}, putData()
*/
void QSystemTest::putFile( const QString &srcFile, const QString &destFile, QFile::Permissions permissions )
{
    QFile f(srcFile);
    if (!f.open(QIODevice::ReadOnly))
        QFAIL( "Couldn't open '" + srcFile + "'" );

    putData(f.readAll(), destFile, permissions ? permissions : f.permissions());
}

/*!
    Transfers \a data from the local machine and copies it to \a destFile on the test system.
    if \a destFile contains environment variables, they will be expanded on the test system.
    The file permissions of the destination file can be specified using \a permissions.

    \sa {File Management}, putFile()
*/
void QSystemTest::putData( const QByteArray &data, const QString &destFile, QFile::Permissions permissions )
{
    QTestMessage message("putFile");
    message["path"] = destFile;
    message["data"] = data;
    if (permissions) {
        message["permissions"] = static_cast<int>(permissions);
    }
    d->queryPassed("OK", "", BT(message), "qpe:");
}

/*!
    Delete \a path from the test system.  Can be a file, or can be a directory
    tree, in which case the entire tree is recursively deleted.
    If \a path contains environment variables, they will be expanded on the
    test system.

    Example:
    \code
        // Force test system to start with clean settings
        deletePath("$HOME/Settings");
        restartQtopia();
    \endcode

    \sa {File Management}
*/
void QSystemTest::deletePath( const QString &path )
{
    QTestMessage message("deletePath");
    message["path"] = path;
    d->queryPassed( "OK", "", BT(message), "qpe:" );
}

/*!
    Invoke method \a method on object \a queryPath on the test system.
    Invokable methods include only Qt signals and slots.

    The method will be invoked using the Qt connection type \a type.  This can
    almost always be Qt::AutoConnection, but in a few cases Qt.QueuedConnection may
    be necessary (for example, if executing a method that will cause Qt Extended to shutdown,
    Qt.QueuedConnection should be used to ensure Qt Extended sends a response to the system
    test before shutting down).

    The optional arguments \a arg0, \a arg1, \a arg2, \a arg3, \a arg4, \a arg5, \a arg6,
    \a arg7, \a arg8 and \a arg9 will be passed to the method if given.

    Returns true if the method could be invoked, false otherwise.

    Example:
    \code
        // Hide this field because it keeps changing and we want a snapshot
        QVERIFY( invokeMethod("Time", "setVisible(bool)", Qt.AutoConnection, false) );
        verifyImage("good_snapshot");
        // Put the field back
        QVERIFY( invokeMethod("Time", "setVisible(bool)", Qt.AutoConnection, true) );
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
bool QSystemTest::invokeMethod( const QString &queryPath, const QString &method, Qt::ConnectionType type,
                                const QVariant &arg0, const QVariant &arg1, const QVariant &arg2,
                                const QVariant &arg3, const QVariant &arg4, const QVariant &arg5,
                                const QVariant &arg6, const QVariant &arg7, const QVariant &arg8,
                                const QVariant &arg9 )
{
    QTestMessage message("invokeMethod");
    message["method"]  = method;
    message["returns"] = false;
    message["conntype"] = (int)type;

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

    message["args"] = argList;

    QTestMessage reply;
    if (!d->queryPassed( QStringList("OK"), QStringList()
            << "ERROR_NO_METHOD"
            << "ERROR_METHOD_NOT_INVOKABLE"
            << "ERROR_WRONG_ARG_COUNT"
            << "ERROR_NO_RETURN"
            << "ERROR_IN_INVOKE", BT(message), queryPath, &reply)) return false;

    return true;
}

/*!
    \overload
    Invokes the given method using connection type Qt.AutoConnection.
*/
bool QSystemTest::invokeMethod( const QString &queryPath, const QString &method,
                                const QVariant &arg0, const QVariant &arg1, const QVariant &arg2,
                                const QVariant &arg3, const QVariant &arg4, const QVariant &arg5,
                                const QVariant &arg6, const QVariant &arg7, const QVariant &arg8,
                                const QVariant &arg9 )
{
    return invokeMethod(queryPath, method, Qt::AutoConnection, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 );
}

/*!
    Set the Qt property named \a name on object \a queryPath to value \a value
    on the test system.

    Errors can occur in this function.

    Example:
    \code
        // Set the text of this field without simulating key presses
        setProperty("Name", "text", "Billy Jones");
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
void QSystemTest::setProperty( const QString &queryPath, const QString &name, const QVariant &value )
{
    QTestMessage message("setProperty");
    message["property"] = name;
    message["value"] = value;

    d->queryPassed( "OK", "", BT(message), queryPath );
}

/*!
    Get the value of the Qt property named \a name on object \a queryPath on the test system.

    Example:
    \code
        // Get the text of this field without using getText()
        var t = getProperty("Name", "text").toString();
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
QVariant QSystemTest::getProperty( const QString &queryPath, const QString &name )
{
    QTestMessage message("getProperty");
    message["property"] = name;

    QVariant out;

    QTestMessage reply;
    if (!d->queryPassed( "OK", "", BT(message), queryPath, &reply)) return out;

    return reply["getProperty"];
}

/*!
    Retrieves a QSettings settings value from the test system located in \a file, settings group \a group, key \a key.
    If \a file contains environment variables, they will be expanded on the test system.

    Example:
    \code
        // What's our primary input mode?
        var primaryInput = getSetting("$QPEDIR/etc/defaultbuttons.conf", "Device", "PrimaryInput");
    \endcode

    \sa setSetting(), QSettings
*/
QVariant QSystemTest::getSetting( const QString &file, const QString &group, const QString &key )
{
    return d->getSetting(QString(), QString(), file, group, key);
}

/*!
    Retrieves a QSettings settings value from the test system located in the settings file for
    organization \a organization and application \a application, as passed to the QSettings
    constructor.  The settings value retrieved will be group \a group, key \a key.

    \sa setSetting(), QSettings
*/
QVariant QSystemTest::getSetting( const QString &organization, const QString &application, const QString &group, const QString &key )
{
    return d->getSetting(organization, application, QString(), group, key);
}

/*!
    Set a QSettings settings \a value on the test system located in \a file, settings group \a group, key \a key.

    Example:
    \code
        // Turn on english and deutsch input languages
        setSetting("$HOME/Settings/Trolltech/locale.conf", "Language", "InputLanguages", "en_US de" );
    \endcode

    \sa getSetting(), QSettings
*/
void QSystemTest::setSetting( const QString &file, const QString &group, const QString &key, const QVariant &value )
{
    d->setSetting(QString(), QString(), file, group, key, value);
}

/*!
    Set a QSettings settings \a value on the test system located in the settings file
    for the given \a organization and \a application, as passed to a QSettings constructor.
    The value set will be in settings group \a group, key \a key.

    \sa getSetting(), QSettings
*/
void QSystemTest::setSetting( const QString &organization, const QString &application, const QString &group, const QString &key, const QVariant &value )
{
    d->setSetting(organization, application, QString(), group, key, value);
}

static QStringList filter = QStringList()
    << "Connected to VFB server"
    << "QtUitest: timed out waiting for key release"
    << "unhandled event watchForKeyRelease"
    << "QIODevice::putChar"
    << "QFile::seek: IODevice is not open"
    << "QSqlQuery::exec"
    << "QSqlQuery::prepare"
    << "QLayout: Attempting"
    << "QTimeLine::start: already running"
    << "QAbstractSocket::waitForBytesWritten) is not allowed in UnconnectedState"
    << "QmemoryFile: No size not specified"
    << "*******************"
    << "Unable to register with BlueZ"
    << "Unable to unregister with BlueZ"
    << "QProcess: Destroyed while process is still running"
    << "omega_detect"
    << "QAlternateStack"
    << "QVFb display 0 is already running"
    << "Using QWS_DISPLAY QVFb"
    << "Unable to send SIGKILL"
    << "Unable to send SIGTERM";

class TestProcess : public QProcess
{
    Q_OBJECT
    public:
        TestProcess(QObject *parent = 0)
            : QProcess(parent)
            , env()
            , test(0)
        {
            connect(this,SIGNAL(readyReadStandardError()),this,SLOT(error()));
            connect(this,SIGNAL(readyReadStandardOutput()),this,SLOT(output()));
            connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),
                    this,SLOT(deleteLater()));
        }

        QStringList env;
        QSystemTest* test;

    private slots:
        void output()
        {
            QByteArray text = readAllStandardOutput();
            if (!test) return;

            while (text.endsWith("\n")) text.chop(1);
            QList<QByteArray> lines = text.split('\n');
            test->applicationStandardOutput(lines);
        }

        void error()
        {
            QByteArray text = readAllStandardError();
            if (!test) return;

            while (text.endsWith("\n")) text.chop(1);
            QList<QByteArray> lines = text.split('\n');
            test->applicationStandardError(lines);
        }

    protected:
        virtual void setupChildProcess()
        {
#if defined Q_OS_UNIX && defined QTUITEST_IMAGE_PATH
#define _STR(X) #X
#define STR(X) _STR(X)
            // Set up the qtuitest hook to be loaded into this process.
            static QString basePath = QSettings("Trolltech", "QtUitest").value("paths/image", STR(QTUITEST_IMAGE_PATH)).toString();
#undef _STR
#undef STR
            static QString overrides = QString("%1/liboverrides/libqtuitestoverrides.so").arg(basePath);
            static QString preload = QString("%1/lib/libqtuitest.so:%1/qtslave/libqtslave.so:%1/loader/libqtuitestloader.so%2").arg(basePath).arg(QFile::exists(overrides) ? QString(":%1").arg(overrides) : QString());
            static QString widgets = QString("%1/qtwidgets").arg(basePath);

            prependToEnv("LD_PRELOAD", preload.toLocal8Bit());
            prependToEnv("QTUITEST_WIDGETS_PATH", widgets.toLocal8Bit());
#endif
            foreach (QString const& e, env) {
                int equals = e.indexOf('=');
                if (equals == -1) continue;
                QString key = e.left(equals);
                QString value = e.mid(equals+1);
                prependToEnv(key.toLocal8Bit(), value.toLocal8Bit());
            }
        }

    private:
        void prependToEnv(QByteArray const& key, QByteArray const& value)
        {
            // Environment variables which are a colon-separated list (like PATH)
            static const QList<QByteArray> pathlike = QList<QByteArray>()
                << "LD_LIBRARY_PATH"
                << "LD_PRELOAD"
                << "PATH"
                << "QTUITEST_WIDGETS_PATH"
            ;

            // Environment variables which can be silently clobbered
            static const QList<QByteArray> clobber = QList<QByteArray>()
                << "DISPLAY"
            ;

            QByteArray current = qgetenv(key.constData());
            QByteArray set = value;
            if (!current.isEmpty()) {
                if (pathlike.contains(key)) {
                    set = set + ":" + current;
                } else if (clobber.contains(key)) {
                } else {
                    // Cannot use qWarning because we are in a child process and we will pass
                    // through message handlers twice
                    fprintf(stderr, "Environment variable %s is already set (to \"%s\"); "
                            "qtuitestrunner will clobber it (with \"%s\") when starting test "
                            "process!"
                        ,key.constData()
                        ,current.constData()
                        ,set.constData()
                    );
                }
            }

            setenv(key.constData(), set.constData(), 1);
        }
};

/*! \internal */
void QSystemTest::applicationStandardOutput(QList<QByteArray> const& lines)
{
    foreach (QByteArray const& line, lines)
        QDebug(QtDebugMsg) << line;
}

/*! \internal */
void QSystemTest::applicationStandardError(QList<QByteArray> const& lines)
{
    foreach (QByteArray const& line, lines)
        QDebug(QtDebugMsg) << line;
}

/*!
    Switches the 'strict syntax' checking mode for the System test to \a on.

    In strict mode the following commands are no longer allowed and will cause an immediate failure:
    \list
    \i keyClick()
    \i keyPress()
    \i keyRelease()
    \i keyClickHold()
    \endlist

    Strict mode also verifies that every 'Title' change is covered by a call to waitForTitle(): any
    action that results in a Dialog to be shown (with a different title) will cause a test failure
    unless a waitForTitle() is called on the next line of the test script.
*/
void QSystemTest::strict( bool on )
{
    d->strict_mode = on;
}

/*!
    Simulates a \a key press for the application specified by \a queryPath.
    \a key is a Qt::Key describing the key to be pressed.

    Example:
    \code
        // Press (do not release) F23 key in current app
        keyPress( Qt.Key_F23 );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::keyPress( Qt::Key key, const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: keyPress is not allowed in strict mode" );

    QTestMessage message("keyPress");
    message["key"] = (int)key;
    QString qp = queryPath;
    if (qp.isEmpty()) qp = "qpe:";
    d->queryPassed( "OK", "", BT(message), qp );
}

/*!
    Simulates a \a key release for the application specified by \a queryPath.
    \a key is a Qt::Key describing the key to be released.

    Example:
    \code
        // Release Up key in current app
        keyRelease( Qt.Key_Up );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::keyRelease( Qt::Key key, const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: keyRelease is not allowed in strict mode" );

    QTestMessage message("keyRelease");
    message["key"] = (int)key;
    QString qp = queryPath;
    if (qp.isEmpty()) qp = "qpe:";
    d->queryPassed( "OK", "", BT(message), qp );
}

/*!
    Simulates a \a key click (press and release) for the application specified by \a queryPath.
    \a key is a string describing the key to be released.

    Example:
    \code
        // Go right 5 times, then select
        for (int i = 0; i < 5; ++i) keyClick( Qt.Key_Right );
        keyClick( Qt.Key_Select );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::keyClick( Qt::Key key, const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: keyClick is not allowed in strict mode" );

    QTestMessage message("keyClick");
    message["key"] = (int)key;
    QString qp = queryPath;
    if (qp.isEmpty()) qp = "qpe:";
    d->queryPassed( "OK", "", BT(message), qp );
}

/*!
    Simulates a \a key click and hold (press + wait + release) for \a queryPath.
    The interval between press and release is set in milliseconds by \a duration.

    Example:
    \code
        // Hold hangup key to bring up shutdown app
        keyClickHold(Qt.Key_Hangup, 3000);
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::keyClickHold( Qt::Key key, int duration, const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: keyClickHold is not allowed in strict mode" );

    d->keyclickhold_key = key;
    d->keyclickhold_path = queryPath;

    QTestMessage message("keyPress");
    message["key"] = (int)key;
    message["duration"] = duration;
    QString qp = queryPath;
    if (qp.isEmpty()) qp = "qpe:";
    d->queryPassed( "OK", "", BT(message), qp );

    if (queryFailed()) return;

    QTest::qWait(duration);

    if (d->keyclickhold_key) {
        d->keyclickhold_key = (Qt::Key)0;
        d->keyclickhold_path = "";

        keyRelease(key, queryPath);
    }
}

/*!
    Simulates a mouse click / touchscreen tap at co-ordinates ( \a x, \a y ).

    Example:
    \code
        mouseClick(200, 300);
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mouseClick( int x, int y )
{
    if (d->strict_mode) QFAIL( "ERROR: mouseClick is not allowed in strict mode" );

    QTestMessage message("mouseClick");
    message["pos"] = QPoint(x,y);
    d->queryPassed( "OK", "", BT(message), "qpe:" );
}

/*!
    Simulates a mouse click / touchscreen tap at the center of the widget
    specified by \a queryPath.

    Example:
    \code
        // Click on Accept button
        mouseClick("Accept");
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mouseClick( const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: mouseClick is not allowed in strict mode" );

    QTestMessage message("mouseClick");
    d->queryPassed( "OK", "", BT(message), queryPath );
    wait(200);
}

/*!
    Simulates a mouse click / touchscreen tap at co-ordinates ( \a x, \a y ),
    with a custom \a duration in milliseconds between press and release.

    Example:
    \code
        // Hold at (200, 300) for three seconds
        mouseClickHold(200, 300, 3000);
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mouseClickHold( int x, int y, int duration )
{
    if (d->strict_mode) QFAIL( "ERROR: mouseClickHold is not allowed in strict mode" );

    mousePress(x, y);
    QTest::qWait(duration);
    mouseRelease(x, y);
}

/*!
    Simulates a mouse click / touchscreen tap at the center of the widget
    specified by \a queryPath, with a custom \a duration in milliseconds
    between press and release.

    Example:
    \code
        // Hold on the "Shutdown" button for three seconds
        mouseClickHold("Shutdown", 3000);
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mouseClickHold( const QString &queryPath, int duration )
{
    if (d->strict_mode) QFAIL( "ERROR: mouseClickHold is not allowed in strict mode" );

    mousePress(queryPath);
    QTest::qWait(duration);
    mouseRelease(queryPath);
}

/*!
    Simulates a mouse / touchscreen press at co-ordinates ( \a x, \a y ).

    Example:
    \code
        mousePress(200, 300);
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mousePress( int x, int y )
{
    if (d->strict_mode) QFAIL( "ERROR: mousePress is not allowed in strict mode" );

    QTestMessage message("mousePress");
    message["pos"] = QPoint(x,y);
    d->queryPassed( "OK", "", BT(message), "qpe:" );
}

/*!
    Simulates a mouse / touchscreen press at the center of the widget
    specified by \a queryPath.

    Example:
    \code
        // Press "Edit" button
        mousePress("Edit");
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mousePress( const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: mousePress is not allowed in strict mode" );

    QTestMessage message("mousePress");
    d->queryPassed( "OK", "", BT(message), queryPath );
}

/*!
    Simulates a mouse / touchscreen release at co-ordinates ( \a x, \a y ).

    Example:
    \code
        mouseRelease(200, 300);
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mouseRelease( int x, int y )
{
    if (d->strict_mode) QFAIL( "ERROR: mouseRelease is not allowed in strict mode" );

    QTestMessage message("mouseRelease");
    message["pos"] = QPoint(x,y);
    d->queryPassed( "OK", "", BT(message), "qpe:" );
}

/*!
    Simulates a mouse / touchscreen release at the center of the widget
    specified by \a queryPath.

    Example:
    \code
        // Release mouse over "Edit" button
        mouseRelease("Edit");
    \endcode

    \sa {Query Paths}, {Mouse / Touchscreen Simulation}
*/
void QSystemTest::mouseRelease( const QString &queryPath )
{
    if (d->strict_mode) QFAIL( "ERROR: mouseRelease is not allowed in strict mode" );

    QTestMessage message("mouseRelease");
    d->queryPassed( "OK", "", BT(message), queryPath );
}

/*!
    Simulates \a text being entered from a key input device into the widget specified by \a queryPath.
    First navigates to the specified widget, then sets the input method hint to null and
    generates a series of key or mouse clicks.

    Most widgets go into an editing mode when entering text and need to be taken
    out of the editing mode by e.g. a Qt::Key_Select or by navigating to
    another field in the dialog. By default, enter() will automatically take whatever
    action is necessary to commit the text and leave edit mode. Set \a mode to NoCommit to
    override this.

    Example:
    \code
        // Enter my job in "Occupation" field.
        enter( "Dog Walker", "Occupation" );
        // Enter my phone number in "Home phone" field.
        enter( "+61 12345", "Home phone" );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::enter( const QString &text, const QString &queryPath, EnterMode mode )
{
    if (text.isNull()) return;
    QTestMessage message("enterText"); // FIXME: QTestSlave function should be renamed to enter() as well
    message["text"] = text;

    switch (mode)
    {
        case NoCommit: message["mode"] = "NoCommit"; break;
        default: message["mode"] = "Commit"; break;
    }

    d->queryPassed( "OK", "", BT(message), queryPath );
}

/*!
    \overload
    Simulates \a date being entered into the widget specified by \a queryPath.

    \a queryPath must refer to a widget that accepts dates as input
    (for example, QDateEdit or QCalendarWidget).

    Example:
    \code
        // Enter my birth date in "Birthday" field.
        var birthday = new QDate(1985, 11, 10);
        enter( birthday, "Birthday" );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::enter( const QDate &date, const QString &queryPath, EnterMode mode )
{
    if (date.isNull() || !date.isValid()) return;
    QTestMessage message("enterText");
    message["date"] = QVariant::fromValue(date);
    message["text"] = message["date"];
    switch (mode)
    {
        case NoCommit: message["mode"] = "NoCommit"; break;
        default: message["mode"] = "Commit"; break;
    }
    d->queryPassed( "OK", "", BT(message), queryPath );
}

/*!
    \overload
    Simulates \a time being entered into the widget specified by \a queryPath.

    \a queryPath must refer to a widget that accepts times as input
    (for example, QTimeEdit).

    Example:
    \code
        enter( new QTime(11, 30), "Start" );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::enter( const QTime &time, const QString &queryPath, EnterMode mode )
{
    if (time.isNull() || !time.isValid()) return;
    QTestMessage message("enterText");
    message["time"] = QVariant::fromValue(time);
    message["text"] = message["time"];
    switch (mode)
    {
        case NoCommit: message["mode"] = "NoCommit"; break;
        default: message["mode"] = "Commit"; break;
    }
    d->queryPassed( "OK", "", BT(message), queryPath );
}

/*!
    Selects the \a item from the application/widget specified by \a queryPath.
    This can be used to, e.g., select a certain item from a list widget or combo box.
    select() works with widgets which are conceptually a list, including
    list views, combo boxes and menus.

    When used with a list widget, the specified item is navigated to and no further
    action is taken.

    When used with a combo box, the drop-down list is opened,
    the item is selected, and the drop-down list is closed again.

    Items in submenus are denoted using '/' delimiters (e.g., "View/All" means
    navigate to the "View" submenu, then the "All" item).  Menu items which
    have a '/' in their name can be escaped using '\' (e.g. "Add\\/Remove Words").

    Example:
    \code
        // Select "Female" from "Gender" field
        select("Female", "Gender");
    \endcode

    If the queryPath is set to optionsMenu(), select will attempt to select the \a item
    from the Options Menu of the current application.

    Items in submenus can be specified by separating items with a '/'.  If an item contains
    a '/', escape it with a backslash.

    Example:
    \code
        // Select the "All Added" option from the "Show Only" submenu in the options menu.
        select( "Show only.../All Added", optionsMenu() );

        // Select the 'Word Lookup/Add' option from the options menu.
        select( "Word Lookup\\/Add", optionsMenu() );
    \endcode

    \sa {Query Paths}, {Keypad Simulation}
*/
void QSystemTest::select( const QString &item, const QString &queryPath )
{
    if (item.isNull()) return;
    QTestMessage message("selectItem"); // FIXME: Ideally QTestSlave has a 'select()' function instead of 'selectItem()'
    message["text"] = item;
    d->queryPassed("OK", "", BT(message), queryPath);

    /* FIXME: this wait should be able to be put in the test widgets.
        * Currently it can't because of the new bop problem (bug 194361).
        */
    wait((d->demo_mode) ? 1500 : 150);
}

/*!
    Start the specified \a application.

    \a application is the combined path and arguments of a program to launch.
    The application must connect to the test framework within \a timeout ms or
    the test fails.

    \a flags specifies additional behaviour.

    \sa {Application Management}
*/
void QSystemTest::startApplication( const QString &application, int timeout, StartApplicationFlags flags )
{
    if (!(flags & BackgroundCurrentApplication)) {
        // If we aren't backgrounding the current app, then we want to kill it.
        while (d->aut.count()) {
            QPointer<QProcess> aut = d->aut.takeFirst();
            if (aut) {
                aut->terminate();
                if (!aut->waitForFinished(5000)) {
                    aut->kill();
                    aut->waitForFinished(5000);
                }
                delete aut;
            }
        }
    }

    TestProcess* proc = new TestProcess(this);
    proc->test = this;
    proc->env = d->env;
    proc->env << QString("QTUITEST_PORT=%1").arg(d->aut_port);
    proc->start(application);
    if (!proc->waitForStarted()) {
        fail(QString("Failed to start process '%1': %2").arg(application).arg(proc->errorString()));
        delete proc;
        return;
    }

    // Give it a little time for the slave to come up.
    wait(1000);

    if (!connectToAut(timeout)) {
        fail(QString("Could not connect to process '%1'.").arg(application));
        proc->terminate();
        proc->waitForFinished(5000);
        proc->kill();
        delete proc;
        return;
    }

    d->aut << QPointer<QProcess>(proc);
}

/*!
    Returns true if the widget specified by \a queryPath exists and is currently visible
    to the user.

    The widget is considered to be visible to the user if QWidget::visibleRegion() returns
    a non-empty region.  Thus, isVisible() will return true if even a single pixel of the
    widget is unobscured.

    \sa {Query Paths}, {Querying Objects}
*/
bool QSystemTest::isVisible( const QString &queryPath )
{
    bool ret = false;
    return queryWithReturn(ret, "isVisible", queryPath);
}

/*!
    The function returns whether the widget specified by \a queryPath is enabled.

    Example:
    \code
        // Verify the AM/PM field is disabled when using 24 hour format
        select("24 hour", "Time format");
        QVERIFY2( !isEnabled("AM-PM"), "AM-PM field still enabled." );
    \endcode

    \sa {Query Paths}, {Querying Objects}
*/
bool QSystemTest::isEnabled( const QString &queryPath )
{
    return getProperty( queryPath, "enabled" ).toBool();
}

/*!
    \internal
    Returns a string of all the visible widgets that are children of the object specified by \a queryPath.
    If only the application part of \a queryPath is defined, then all visible widgets for that application
    are listed. The string is organized in a tree type hierarchy showing the relationship between
    widgets.

    This function is intended to help with the development of testcases.

    \sa {Query Paths}
*/
QString QSystemTest::activeWidgetInfo()
{
    QString ret;
    return queryWithReturn(ret, "activeWidgetInfo", "");
}

/*!
    Returns the checked state of the checkbox-like widget specified by \a queryPath.
    Checkbox-like widgets include QCheckBox and anything inheriting QAbstractButton.

    \sa {Query Paths}, {Querying Objects}, setChecked()
*/
bool QSystemTest::isChecked( const QString &queryPath )
{
    bool ret = false;
    return queryWithReturn(ret, "isChecked", queryPath);
}

/*!
    Based on the value of \a doCheck checks or un-checks a checkbox-like widget specified by \a queryPath.
    Checkbox-like widgets include QCheckBox and anything inheriting QAbstractButton.

    \sa {Query Paths}, {Querying Objects}, isChecked()
*/
void QSystemTest::setChecked( bool doCheck, const QString &queryPath)
{
    QTestMessage message("setChecked");
    message["doCheck"] = doCheck;
    d->queryPassed("OK", "", BT(message), queryPath);
}

/*!
    Returns the signature of the buddy widget that is associated with \a labelText.
    A buddy relationship usually exists between a QLabel (or QIconSelector) class and an editable field
    such as a QLineEdit. The label tells the user what data should be entered into the editable field.

    The buddy relationship in QtUitest is determined by:
    \list
    \i the 'buddy' value is set in the Label, or
    \i the Label/IconSelector and the editable field are displayed next to each other, or
    \i the 'buddy' is a child of a GroupBox
    \endlist

    If \a offset is a number != 0 the query will return the n'th next or previous buddy field. This can be used for
    exceptional situations where a field doesn't have a label associated with it.

    The signature() function only needs to be used in exceptional cases: the preferred mechanism is to identify
    fields by their Label text.

    \sa {Query Paths}, {Querying Objects}, {QObject::inherits()}
*/
QString QSystemTest::signature( const QString &labelText, int offset )
{
    QTestMessage message("widget");
    message["offset"] = offset;
    QTestMessage reply;
    if (!d->queryPassed("OK", "", BT(message), labelText, &reply)) return "";
    return reply["widget"].toString();
}

/*!
    List the contents of directory \a dir on the test system, applying \a filters to the listing.
    If \a dir contains environment variables, they will be expanded on the test system.

    The returned listing will be relative to \a dir.

    Example:
    \code
        // Delete entire contents of Documents directory on the test system
        var list = getDirectoryEntries( documentsPath(), QDir.AllEntries);
        for (var i = 0; i < list.length; ++i) {
            deletePath( documentsPath() + list[i]);
        }
    \endcode

    \sa QDir::entryList(), {File Management}
*/
QStringList QSystemTest::getDirectoryEntries( const QString &dir, QDir::Filters filters )
{
    QTestMessage message("getDirectoryEntries");
    message["path"] = dir;
    message["filters"] = (int)filters;

    QStringList out;
    QTestMessage reply;
    if (!d->queryPassed( "OK", "", BT(message), "qpe:", &reply)) return out;
    if (reply["getDirectoryEntries"].isNull()) {
        setQueryError("Got no data in response to getDirectoryEntries");
        return out;
    }
    out = reply["getDirectoryEntries"].toStringList();
    return out;
}

/*!
    Returns the current date and time of the test system.

    \sa setDateTime(), setTimeSynchronization()
*/
QDateTime QSystemTest::getDateTime()
{
    QDateTime ret;
    return queryWithReturn(ret, "systemTime", "qpe:");
}

/*!
    Sets date and time to \a dateTime on the test system.

    If specified the \a timeZone will also be set.

    Setting the time generally requires root access on the test system.  However, this will
    also work for a test build of Qt Extended on the desktop, if Qt Extended is launched
    by the system test using the \c runqtopia script.

    Example:
    \code
        setDateTime( QDateTime( 2007, 5, 27, 23, 59, 59 ), "Australia/Brisbane" );
    \endcode
    The entered date is entered in the sequence: year, month, day, hour, minute, second

    If the time synchronization is set to AutoTimeSynchronizationMode any attempt to set the date
    and time will result in a fail.

    \sa getDateTime(), setTimeSynchronization()
*/
void QSystemTest::setDateTime( const QDateTime &dateTime, const QString &timeZone )
{
    if (!dateTime.isValid()) {
        setQueryError( "ERROR: Invalid date/time specified '" + dateTime.toString() + "'" );
        return;
    }
    QTestMessage message("setSystemTime");
    message["datetime"] = dateTime;
    message["timeZone"] = timeZone;
    d->queryPassed( "OK", "", BT(message), "qpe:");
}

/*!
    Synchronizes the date and time on the device to the current date/time of the host computer, i.e. the machine on which the system test is running.

    This generally requires root access on the test system.  However, this will
    also work for a test build of Qt Extended on the desktop, if Qt Extended is launched
    by the system test using the \c runqtopia script.

    Example:
    \code
        synchronizeDateTime();
    \endcode

    If the time synchronization is set to AutoTimeSynchronizationMode any attempt to set the date
    and time will result in a fail.

    \sa getDateTime(), setTimeSynchronization()
*/
void QSystemTest::synchronizeDateTime()
{
    setDateTime( QDateTime::currentDateTime() );
}

/*!
    Sets the time synchronization mode on the device to \a mode.
    This action should be equivalent to 'opening the Date/Time application and then setting the time mode to 'Auto vs Manual'.

    When \a mode is HostTimeSynchronization the mode is set to 'Manual' on the device and then the time is synchronized with
    the time on the host machine, i.e. the machine on which the system test is running.
    The returned value is the approximate amount of time the device was adjusted (in seconds).

    Note: Host time synching has an accuracy of +- 1 second.

    \sa getDateTime(), setDateTime()
*/
int QSystemTest::setTimeSynchronization( TimeSynchronizationMode mode )
{
    int offset = 0;

    QTestMessage message("setTimeSynchronization");
    switch (mode)
    {
        case AutoTimeSynchronization: message["mode"] = "auto"; break;
        default: message["mode"] = "manual"; break; // both 'HostSyncMode' and 'ManualSyncMode' means the device must be in manual time sync mode
    }
    d->queryPassed( "OK", "", BT(message), "qpe:");

    if (mode == HostTimeSynchronization) {
        if (runsOnDevice()) {
            QDateTime dt = getDateTime();
            QDateTime cur = QDateTime::currentDateTime();
            offset = dt.secsTo( cur );
            if (offset <= 1 && offset >= -1) offset = 0;

            if (offset) {
                setDateTime( QDateTime::currentDateTime() );
                wait(1000);
            }
        }
    } else if (mode == AutoTimeSynchronization) {
    } else if (mode == ManualTimeSynchronization) {
    }

    return offset;
}

/*!
    Sets the date format on the device to \a dateFormat.
    Accepted values for \a dateFormat:
    \list
    \o "English (U.S.)" : sets date format to "M/D/Y".
    \o "D/M/Y" : sets date format to "D/M/Y"
    \o "D.M.Y" : sets date format to "D.M.Y"
    \o "M/D/Y" : sets date format to "M/D/Y"
    \o "Y-M-D" : sets date format to "Y-M-D"
    \endlist

    \sa set12HourTimeFormat()
*/
void QSystemTest::setDateFormat( const QString &dateFormat )
{
    if (dateFormat != "English (U.S.)" &&
        dateFormat != "D/M/Y" &&
        dateFormat != "D.M.Y" &&
        dateFormat != "M/D/Y" &&
        dateFormat != "Y-M-D") {
        setQueryError( "ERROR: Invalid dateFormat '" + dateFormat + "'" );
        return;
    }

    QTestMessage message("setDateFormat");
    message["mode"] = dateFormat;
    d->queryPassed( "OK", "", BT(message), "qpe:");
}

/*!
    Returns the date format that is currently used on the device.
*/
QString QSystemTest::dateFormat()
{
    QString ret;
    return queryWithReturn(ret, "dateFormat", "");
}

/*!
    Sets the time format on the device to \a ampm.
    Accepted values for \a ampm:
    \list
    \o true: Time format is set to 12 hour mode,
    \o false: Time format is set to 24 hour mode.
    \endlist

    \sa setDateFormat()
*/
void QSystemTest::set12HourTimeFormat( bool ampm )
{
    QTestMessage message("setTimeFormat");
    if (ampm) message["mode"] = "12";
    else message["mode"] = "24";

    d->queryPassed( "OK", "", BT(message), "qpe:");
}

/*!
    Returns the time format that is currently used on the device.
*/
QString QSystemTest::timeFormat()
{
    QString ret;
    return queryWithReturn(ret, "timeFormat", "");
}

/*!
    Sets the \a timeZone on the device.

    \sa timeZone()
*/
void QSystemTest::setTimeZone( const QString &timeZone )
{
    QTestMessage message("setTimeZone");
    message["timeZone"] = timeZone;
    d->queryPassed( "OK", "", BT(message), "qpe:");
}

/*!
    Returns the time zone that is currently used on the device.

    \sa setTimeZone()
*/
QString QSystemTest::timeZone()
{
    QString ret;
    return queryWithReturn(ret, "timeZone", "");
}

/*!
    Returns true if the test is running on an actual device, and false if it is running in a virtual framebuffer (on the desktop machine).
*/
bool QSystemTest::runsOnDevice()
{
    return d->aut_host != "127.0.0.1";
}

/*!
    Wait for \a msecs milliseconds, while still processing events from the event loop.
*/
void QSystemTest::wait(int msecs)
{
    QTest::qWait(msecs);
}

/*!
    Returns the currently set Visual Response Time. This time is used in QtUiTest to decide whether the User
    Interface is responsive to user events. For instance, after selecting "New Event" from the options menu a user
    expects a dialog in which a new event can be entered. If Qt Extended does not respond in some visible way within
    the visual response time, the test will fail.

    By default the visual response time is set to 4 seconds, i.e. any UI that doesn't respond to events within this time
    is considered at fault.

    The visibleResponseTime is also used as the default value for some queries such as waitForTitle().

    \sa setVisibleResponseTime(), waitForTitle()
*/
int QSystemTest::visibleResponseTime()
{
    return d->visible_response_time;
}

/*!
    Sets the Visual Response Time to \a time.

    \sa visibleResponseTime(), waitForTitle()
*/
void QSystemTest::setVisibleResponseTime( int time )
{
    if (d->visible_response_time != time) {
        qLog(QtUitest) <<  "Need to set a new visual response time" ;
    }
    d->visible_response_time = time;
}

/*!
    Take a full screenshot of Qt Extended and save it as \a name.
    The screenshot will be placed in the test data directory in PNG format,
    and will automatically have .png appended to the name.

    This function is intended to be used as a simple way to automate the
    gathering of screenshots, i.e. to be used in documentation and such.

    If a \a queryPath is specified the snapshot will be limited to the Widget
    that is identified by the queryPath.

    Example:
    \code
        // Take snapshots of home screen and launcher grid
        gotoHome();
        saveScreen("homescreen");
        keyClick( Qt::Key_Select );
        saveScreen("launcher");
    \endcode

    \sa verifyImage()
*/
void QSystemTest::saveScreen(const QString &name, const QString &queryPath)
{
    QString cdp = currentDataPath();
    if (!QDir("/").exists(cdp) && !QDir("/").mkpath(cdp)) QFAIL(QString("Path '' didn't exist and I couldn't create it").arg(cdp));
    QImage img;
    QCHECKED( img = grabImage(queryPath) );
    if (img.isNull()) QFAIL( "Qtopia returned a null screenshot." );
    if (!img.save(cdp + "/" + name + ".png", "PNG")) QFAIL(QString("Couldn't save image '%1' to '%2'").arg(name).arg(cdp) );
}


/*!
    \fn QSystemTest::expectMessageBox(String title, String text, String option, Number timeout)
    Denotes the start of a block of code which, immediately after or during execution, should
    cause a message box to pop up with the given \a title and \a text.  When the message box
    appears, the given menu \a option will be chosen from the message box softmenu bar (if one exists).

    If the message box hasn't appeared by the end of the block of code, the test will
    wait until the \a timeout expires.  If it still doesn't appear, the current test fails.

    If a message box appears which hasn't been marked as expected, the current test fails.

    Example:
    \code
    // Delete a contact - select "Yes" on the popped-up message box
    // If the message box doesn't pop up, the test fails.
    expectMessageBox("Contacts", "Are you sure you want to delete: " + contact_name + "?", "Yes") {
        select("Delete contact", optionsMenu());
    }
    \endcode
*/

/*!
    Informs the test that a message box is expected to pop up (soon) with the given \a title and \a text.
    When \a option is specified QtUiTest will try to activate a menu item from the softmenu that has the text specified by \a option.
    If a message box appears while executing a test which hasn't been marked as expected, the current test fails.

    When the message box appears, the user-visible \a option will be selected.

    Use this function when expectMessageBox() is unsuitable, e.g. when several different types of message boxes
    might occur.

    Example:
    \code
    addExpectedMessageBox("Calendar", "Are you sure you want to delete:");
    addExpectedMessageBox("Delete Event", "This appointment is part of");
    select( "Delete event", optionsMenu() );
    waitExpectedMessageBox(4000,false);
    if (currentTitle() == "Delete Event") select( "Back", softMenu() )
    else if (currentTitle() == "Calendar") select( "Yes", softMenu() );
    clearExpectedMessageBoxes();
    \endcode
    \sa expectMessageBox(), clearExpectedMessageBox(), clearExpectedMessageBoxes(), waitExpectedMessageBox()
*/
void QSystemTest::addExpectedMessageBox( const QString &title, const QString &text, const QString &option )
{
    QSystemTestPrivate::ExpectedMessageBox *box = new QSystemTestPrivate::ExpectedMessageBox();
    box->test_function = QTest::currentTestFunction();
    box->data_tag = QTest::currentDataTag();
    box->title = title;
    box->text = text;
    box->option = option;
    d->expected_msg_boxes.append( box );
}

/*!
    Removes the message box with the given \a title and \a text from the list of expected message boxes.
    \sa addExpectedMessageBox(), waitExpectedMessageBox()
*/
void QSystemTest::clearExpectedMessageBox( const QString &title, const QString &text )
{
    for (int i = 0; i < d->expected_msg_boxes.count(); ++i) {
        QSystemTestPrivate::ExpectedMessageBox *box = d->expected_msg_boxes.at(i);
        if (box && box->title == title && box->text == text
                && box->test_function == QTest::currentTestFunction()
                && box->data_tag == QTest::currentDataTag()) {
            delete d->expected_msg_boxes.takeAt(i);
            break;
        }
    }
}
/*!
    Clears the list of expected message boxes.
    \sa addExpectedMessageBox(), waitExpectedMessageBox(), clearExpectedMessageBox()
*/
void QSystemTest::clearExpectedMessageBoxes()
{
    while (d->expected_msg_boxes.count() > 0) {
        delete d->expected_msg_boxes.takeFirst();
    }
}

/*!
    Waits for expected message boxes to occur up until a maximum of \a timeout ms.
    Returns true if the expected message box occurs during or before this function.

    It is an error to call this function for a message box which has not been
    passed to expectMessageBox() or addExpectedMessageBox().

    If \a waitForAll is true (the default) ALL messageBoxes will need to be shown before the wait terminates,
    or else it is sufficient if any of the expected message boxes has been shown.

    When a \a title and \a text are specified the function will wait until that specific message box has been shown.
    Note that specifying a \a title and \a text only makes sense when \a waitForAll is set to false.

    \sa addExpectedMessageBox(), clearExpectedMessageBox(), clearExpectedMessageBoxes(), expectMessageBox()
*/
bool QSystemTest::waitExpectedMessageBox( uint timeout, bool waitForAll, const QString& title, const QString& text )
{
    // expected_msg_boxes contains all message boxes which we are expecting, but
    // have not yet arrived.  When it arrives while we are in wait(), it will be
    // removed.  So, loop until expected_msg_boxes does not contain the message
    // box(es) we are waiting for.

    bool ret = false;
    QTime timer;
    timer.start();
    for (timer.start(); !ret && uint(timer.elapsed()) < timeout; wait(200)) {
        bool found = false;
        for (int i = 0; i < d->expected_msg_boxes.count() && !found; ++i) {
            QSystemTestPrivate::ExpectedMessageBox *box = d->expected_msg_boxes.at(i);
            if (box->title.contains(title)
                    && box->text.contains(text)) {
                found = true;
            }
        }
        if (waitForAll) {
            // If we're waiting for all, we can only return once the count is 0.
            ret = (d->expected_msg_boxes.count() == 0);
        } else {
            // If we didn't find the message box in the list of expected message
            // boxes, then it must have popped up.
            ret |= !found;
        }
    }
    return ret;
}

/*!
    If \a ignore is true, message boxes will be ignored by the test framework. Under normal circumstances during automated
    testing, message boxes need to be processed. However, short-lived, self dismissing message boxes may disappear before
    the test framework can handle them. Set \a ignore to false to resume normal processing of message boxes.

    \sa addExpectedMessageBox(), expectMessageBox(), clearExpectedMessageBox(), clearExpectedMessageBoxes(), waitExpectedMessageBox()
*/
void QSystemTest::ignoreMessageBoxes( bool ignore )
{
    d->ignore_msg_boxes = ignore;
}

/*!
    Indicate to the test framework if the application under test is expected to close.

    If \a value is true, the test framework will not report a failure when it loses its connection to the application.
    If \a value is false, unexpected application terminations will result in a test failure.

    Example:
    \code
    expectApplicationClose( true );
    select( "Close" );                // Selecting this causes the current application to close
    expectApplicationClose( false );  // Resume normal checking
    \endcode
*/
void QSystemTest::expectApplicationClose( bool value )
{
    d->expect_app_close = value;
}

/*!
    \internal
    Processes the command line parameters.
*/
void QSystemTest::processCommandLine( int &argc, char *argv[] )
{
    int offset = 0;

    // Whenever we discover an option that 'we' understand, eat away the option (and its parameters) so that QTest doesn't get
    // confused by them.
    for (int i=1; i<argc; ++i) {

        if ( !strcasecmp(argv[i], "-remote") ) {
            argv[i] = 0;
            ++offset;

            if ( i+1 >= argc || !strlen(argv[i+1]) )
                qFatal("Expected a host:port argument after -remote");

            QString host_port( argv[i+1] );
            QStringList host_port_parts = host_port.split(":");

            bool ok = (host_port_parts.count() == 2);
            int port = -1;
            QString host;
            if (ok) port = host_port_parts[1].toInt(&ok);
            if (ok) host = host_port_parts[0];
            ok = ok && port > 0 && port < 65536 && !host.isEmpty();

            if (!ok)
                qFatal("'%s' is not a valid host:port argument", qPrintable(host_port));

            argv[i+1] = 0;
            ++offset;
            ++i;

            d->qtest_ide.openRemote( host, port );
            connect(&d->qtest_ide, SIGNAL(abort()), this, SLOT(abortTest()));

        } else if ( !strcasecmp(argv[i], "-autip") || !strcasecmp(argv[i], "-authost") ) {
            char *given_arg = argv[i]; // so we can output 'autip' or 'authost' in error messages

            argv[i] = 0;
            ++offset;
            if ( i+1 >= argc || !strlen(argv[i+1]) )
                qFatal("Expected a host specifier after %s", given_arg);

            d->aut_host = argv[i+1];
            argv[i+1] = 0;
            ++offset;
            ++i;

        } else if ( !strcasecmp(argv[i], "-autport") ) {
            argv[i] = 0;
            ++offset;
            if ( i+1 >= argc || !strlen(argv[i+1]) )
                qFatal("Expected a port specifier after -autport");

            bool ok;
            d->aut_port = QString(argv[i+1]).toUShort( &ok );
            if (!ok)
                qFatal("%s is not a valid port specifier", argv[i+1]);

            argv[i+1] = 0;
            ++offset;
            ++i;

        } else if ( !strcasecmp(argv[i], "-keepaut") ) {
            argv[i] = 0;
            ++offset;
            d->keep_aut = true;

        } else if ( !strcasecmp(argv[i], "-silentaut") ) {
            argv[i] = 0;
            offset++;
            d->silent_aut = true;

        } else if ( !strcasecmp(argv[i], "-noaut") ) {
            argv[i] = 0;
            offset++;
            d->no_aut = true;

        } else if ( !strcasecmp(argv[i], "-auto") ) {
            argv[i] = 0;
            offset++;
            d->auto_mode = true;

        } else if ( !strcasecmp(argv[i], "-env") ) {
            argv[i] = 0;
            offset++;

            if ( i+1 >= argc || !strlen(argv[i+1]) )
                qFatal("Expected a string after -env");

            d->env << QString::fromLocal8Bit(argv[i+1]);
            argv[i+1] = 0;
            ++offset;
            ++i;

        } else if ( !strcasecmp(argv[i], "-demomode") ) {
            argv[i] = 0;
            offset++;
            d->demo_mode = true;

        } else if ( !strcasecmp(argv[i], "-verbose-perf") ) {
            argv[i] = 0;
            offset++;
            d->verbose_perf = true;

        } else if ( !strcasecmp(argv[i], "-v") ) {
            argv[i] = 0;
            offset++;
            d->verbose = true;

        } else {
            if (offset > 0) {
                argv[i-offset] = argv[i];
                argv[i] = 0;
            }
        }
    }
    argc-=offset;

    QAbstractTest::processCommandLine(argc, argv);

    if (d->auto_mode && learnMode() != LearnNone) {
        qWarning("Can't learn in auto mode; learn options ignored.");
    }
    if (!d->auto_mode && learnMode() == LearnNone) {
        setLearnMode(LearnNew);
    }
}

/*!
    \internal
    Starts event recording. While event recording is busy a dialog will be visible that shows all recorded events and also
    enables the user to stop the recording session. \a file and \a line specifies the location in a source file where event
    recording commenced.

    If \a manualSteps are specified the dialog will have an additional field showing all the manual steps.

    Don't use this, use prompt() instead.
*/
bool QSystemTest::recordEvents( const QString &manualSteps, bool gui )
{
    if (d->auto_mode && gui) {
        skip("Can't record events in auto mode.", SkipSingle );
        return false;
    }

    d->recorded_events_edit = 0;
    d->recorded_code = QString();
    if (!d->queryPassed( "OK", "", QTestMessage("startEventRecording"))) return false;
    if (gui) {
        if (d->qtest_ide.isConnected()) {
            QTestMessage message("EVENT_RECORDING_STARTED");
            message["file"] = currentFile();
            message["line"] = currentLine();
            message["steps"] = manualSteps;
            d->qtest_ide.postMessage( message );
            d->qtest_ide.must_stop_event_recording = false;
            d->qtest_ide.event_recording_aborted = false;
            d->recording_events = true;
            while (!d->qtest_ide.must_stop_event_recording) {
                QTest::qWait( 50 );
            }
            d->recording_events = false;
            if (d->qtest_ide.event_recording_aborted) {
                skip("Event recording aborted.", SkipSingle );
            }
        } else {
            QDialog recordWindow;
            Ui::RecordDialog ui;
            ui.setupUi(&recordWindow);

            if (!manualSteps.isEmpty()) {
                ui.steps_view->setPlainText( manualSteps );
            } else {
                ui.steps_view->hide();
                ui.steps_label->hide();
            }

            d->recorded_events_edit = ui.codeEdit;

            connect( ui.abort_button, SIGNAL(clicked()), &recordWindow, SLOT(close()) );
            connect( ui.abort_button, SIGNAL(clicked()), this, SLOT(abortPrompt()) );
            abort_prompt = false;

            d->recording_events = true;
            recordWindow.exec();
            d->recording_events = false;
            if (abort_prompt) {
                return false;
            }

        }
        return d->queryPassed( "OK", "", QTestMessage("stopEventRecording"));
    }
    return true;
}

/*!
    \internal
    Stops recording events, returning the generated code from event recording.
    Used internally for testing event recording.
*/
QString QSystemTest::stopRecordingEvents()
{
    d->queryPassed( "OK", "", QTestMessage("stopEventRecording"));
    return d->recorded_code;
}

/*!
    Displays a dialog with the \a promptText and Pass/Fail/Record buttons, then waits for a response from the user.
    If Pass is clicked, the test will continue.
    If Fail is clicked, a failure will be generated and the current testcase will abort.
    If Record is clicked, event recording will start and recorded events can be appended to the testcase once recording is
    finished.

    This function is intended to be used for verification of testcases that cannot be fully automated. Testcases containing
    this function will be skipped if the test is run with the \c -auto option.

    Example:
    \code
        prompt( "Did something totally amazing just happen?" );
    \endcode

    By using a prompt() the process of automating testcases can be an iterative process. In the first iteration the whole
    testcase can be a manual test and the systemtest merely is used as a complex mechanism to show a dialog. For example:
    \code
        void sys_mytest::test1()
        {
            prompt(
                "step 1: Start application foo\n"
                "step 2: Select 'Properties' from Options Menu\n"
                "step 3: Enter several fields in the properties dialog\n"
                "step 4: Close dialog\n"
                "step 5: Do something to verify that the properties are set correctly"
            );
        }
    \endcode
    In the above example the user/tester has to perform all the steps manually and then click on Pass or Fail to
    indicate the success of the testcase.

    A reason for writing a fully manual test might be that it is extremely difficult to write an automated test, or it's
    simply not clear yet how to write an automated test. As time progresses however usually an understanding is build up
    of how to automate the test, or parts of the test. Even if part of the test is automated there is already an
    advantage. In the previous example for instance it would already be advantageous if the application was started by
    the test framework and then the rest was a few manual steps.

    For example:
    \code
        void sys_mytest::test1()
        {
            select( "foo", launcherMenu() ); // use test framework to start application using the launcher.
            select( "Properties", optionsMenu() ); // and select an option from the options menu

            prompt(
                "step 3: Enter several fields in the properties dialog\n"
                "step 4: Close dialog\n"
                "step 5: Do something to verify that the properties are set correctly"
            );
        }
    \endcode
    The first two steps are now automated, the rest is still manual.

    Next we realize that entering the data in the properties can also be automated:
    \code
        void sys_mytest::test1()
        {
            startApp( "foo" ); // use test framework to start application
            select( "Properties", optionsMenu() ); // and select an option from the options menu

            // enter the properties
            enter( "dungeon&dragons", "Name" );
            enter( "example.org", "Server" );

            // close dialog
            select( "Back", softMenu() );

            prompt( "step 5: Do something to verify that the properties are set correctly" );
        }
    \endcode
    The testcase now has only one manual (verification) step left. But the biggest advantage is that the 'entering data'
    part of the test is now automated. If the testcase was extended with a few sets of testdata the same routine could
    be used many times, and for instance could be testing multiple languages without any effort from the tester.
*/
void QSystemTest::prompt( const QString &promptText )
{
    if (d->auto_mode) {
        skip("Manual test skipped", SkipSingle);
        return;
    }

    if (promptText.isEmpty()) {
        recordEvents( "" );
        return;
    }

    bool prev_ignore = d->ignore_msg_boxes;
    ignoreMessageBoxes(true);
    int level1_count = 0;
    int level2_count = 0;
    int level3_count = 0;
    QStringList prompt_text = promptText.split("\n");
    for (int i=0; i<prompt_text.count(); i++) {
        if (prompt_text[i].startsWith("* ")) {
            prompt_text[i] = QString("%1. %2").arg(++level1_count).arg(prompt_text[i].mid(1));
            level2_count=0;
        } else if (prompt_text[i].startsWith("** ")) {
            prompt_text[i] = QString("    %1. %2").arg(++level2_count).arg(prompt_text[i].mid(2));
            level3_count=0;
        } else if (prompt_text[i].startsWith("*** ")) {
            prompt_text[i] = QString("        %1. %2").arg(++level2_count).arg(prompt_text[i].mid(3));
        }
    }

    QDialog promptWindow;
    Ui::ManualVerificationDlg promptUi;
    promptUi.setupUi(&promptWindow);
    promptUi.test_steps->setPlainText( prompt_text.join("\n") );
    if (currentTestFunction() != NULL) {
        promptWindow.setWindowTitle( "Manual Verification Step: "+ currentTestFunction() );
    }

    QDialog failureWindow;
    Ui::FailureDlg failureUi;
    failureUi.setupUi(&failureWindow);
    connect( promptUi.cancelButton, SIGNAL(clicked()), &failureWindow, SLOT(exec()) );
    connect( promptUi.learnButton, SIGNAL(clicked()), d, SLOT(recordPrompt()) );
    connect( promptUi.learnButton, SIGNAL(clicked()), &promptWindow, SLOT(close()) );
    connect( promptUi.abort_button, SIGNAL(clicked()), &promptWindow, SLOT(close()) );
    connect( promptUi.abort_button, SIGNAL(clicked()), this, SLOT(abortPrompt()) );

    connect( failureUi.buttonBox, SIGNAL(accepted()), &promptWindow, SLOT(reject()) );

    d->record_prompt = false;
    abort_prompt = false;
    promptWindow.exec();
    ignoreMessageBoxes(prev_ignore);

    if (abort_prompt) {
        skip( "Manual verification step has been aborted", SkipSingle );
        return;
    }

    if (d->record_prompt) {
        recordEvents( prompt_text.join("\n") );
        return;
    }
    bool ret = (promptWindow.result() == QDialog::Accepted);

    /* If we fail with no text, encourage user to enter text */
    if (learnMode() == LearnNone) {
        while (!ret && failureUi.failureEdit->toPlainText().isEmpty() ) {
            int clicked = QMessageBox::warning( 0, "No message entered",
                "You have not entered a failure message.  This may make it difficult to determine "
                "the reason for failure from the test logs.  Continue without entering a message?",
                QMessageBox::Ok, QMessageBox::Cancel );
            if (clicked == QMessageBox::Ok) break;
            QMetaObject::invokeMethod(promptUi.cancelButton, "click", Qt::QueuedConnection);
            promptWindow.exec();
            ret = (promptWindow.result() == QDialog::Accepted);
        }
    }

    QString failureMessage = failureUi.failureEdit->toPlainText();

    if (ret) return;

    QString message = "Manual verification steps failed";
    if (!failureMessage.isEmpty()) {
        message += "; " + failureMessage;
    }
    fail( message );
}

/*! \internal */
void QSystemTest::abortPrompt()
{
    abort_prompt = true;
}

/*!
    \internal
    This function is called when the test IDE wants to abort the current test.
*/
void QSystemTest::abortTest()
{
    // Kill ourselves.  GracefulQuit will ensure that Qtopia is also killed if
    // it was started by us.
    ::raise(SIGINT);
}

/*!
    \internal
    Print any special usage information which should be shown when test is launched
    with -help.
*/
void QSystemTest::printUsage(int argc, char *argv[]) const
{
    QAbstractTest::printUsage(argc, argv);
    qWarning(
        "  System test options:\n"
        "    -authost <host>   : Specify the IP address or host name of the machine on which the AUT is running.\n"
        "                        By default, the system test will connect to 127.0.0.1.\n"
        "    -autport <port>   : Specify the port on which the AUT is listening for a system test connection.\n"
        "                        Defaults to %d.\n"
        "    -keepaut          : Leave the AUT running after the system test finishes.\n"
        "                        By default, if the system test launches the AUT, it will kill the AUT when testing\n"
        "                        completes.\n"
        "    -silentaut        : Hide the output of the AUT.  By default, if the system test launches the AUT, the AUT's\n"
        "                        standard output and standard error will be mixed with that of the test itself.\n"
        "\n"
        "    -auto             : Run in fully-automated mode.  Any tests which require the presence of a manual\n"
        "                        tester will be skipped.\n"
        "\n"
        "    -verbose-perf     : Output all performance messages from the AUT to the console.  This may be used to run\n"
        "                        a post-processing script on performance logs.\n"
        "\n"
        "    -remote <host:port> : Specify the host and port of a QtUiTest-compatible test IDE, used for manual\n"
        "                          verification steps.  The default is to not use an IDE.\n"
        "    -env VAR=VALUE    : Specify additional environment variables to be applied to tested \n"
        "                        applications.  For example, pass -env DISPLAY=:123 to run tested \n"
        "                        applications on a different X server.\n"
        , DEFAULT_AUT_PORT
    );
}

#ifndef Q_QDOC
/*
    Fail returning a bool is for internal use only.
    Hide from documentation.
*/
/*!
    \internal
    Cause a test failure with the given \a message.
    Returns true if the test should continue (e.g., because the failure was expected), false otherwise.
*/
bool QSystemTest::fail(QString const &message)
{
    if (d->expect_app_close &&
        message.startsWith("ERROR: Connection lost") ||
        message.startsWith("ERROR: no data in reply to") ||
        message.startsWith("reply was missing return value") ||
        message.startsWith("ERROR_NO_CONNECTION") ) {
        return true;
    }

    static bool saving_screen = false;
    static bool saving_info   = false;
    static QString saving_screen_failed;
    static QString saving_info_failed;
    /* Prevent recursive failure on saving screen/widget info */
    if (saving_screen) {
        saving_screen_failed = "Failed to save failure screenshot: " + message;
        return true;
    }
    if (saving_info) {
        saving_info_failed = "Failed to save widget info on failure: " + message;
        return true;
    }

    bool ret = QTest::compare_helper( false, qPrintable(message), qPrintable(currentFile()), currentLine() );
    if (!ret) {
        saving_screen = true;
        saving_screen_failed = QString();

        QString config = configurationIdentifier();
        saveScreen("failure_" + config);

        saving_screen = false;

        if (saving_screen_failed.isEmpty()) {
            // If we saved it, let the IDE know.
            QFileInfo info(currentDataPath() + "/failure_" + config + ".png");
            if (info.exists()) {
                QTestMessage msg("failure_screenshot");
                msg["screenshot"]   = info.canonicalFilePath();
                msg["filename"]     = currentFile();
                msg["line"]         = currentLine();
                msg["testfunction"] = QTest::currentTestFunction();
                d->qtest_ide.postMessage(msg);
            }
        }

        saving_info = true;
        saving_info_failed = QString();

        QString info = activeWidgetInfo();

        saving_info = false;
        if (!info.isEmpty()) {
            QFile f(currentDataPath() + "/failure_" + config + ".txt");
            if (!f.exists() && f.open(QIODevice::WriteOnly)) {
                QTextStream(&f)
                    <<  message << "\n"
                    << "Location: " << currentFile() << ":" << currentLine() << "\n"
                    << (saving_screen_failed.isEmpty()
                            ? "Also see failure.png for a screenshot."
                            : saving_screen_failed)
                    << "\n"
                    << (saving_info_failed.isEmpty()
                            ? info
                            : saving_info_failed)
                    << "\n";
            }
        }
    }
    return ret;
}
#endif

/*!
    Cause a test to skip with the given \a message and \a mode.
*/
void QSystemTest::skip(QString const &message, SkipMode mode)
{
    QTest::qSkip(qPrintable(message), QTest::SkipMode(mode), qPrintable(currentFile()), currentLine());
}

/*!
    \internal
    Returns true if the last executed query has failed.
    If \a message is not null, a failure message is returned in \a message.
    If \a sent is not null, the message sent from the system test which
    caused the failure is returned in \a sent.
*/
bool QSystemTest::queryFailed( QTestMessage *message, QTestMessage *sent )
{
    if (message != 0)
        *message = d->error_msg;
    if (sent != 0)
        *sent = d->error_msg_sent;
    return d->query_failed;
}

/*!
    \internal
    Informs the testframework that error situations need to be reported as warnings.
*/
void QSystemTest::enableQueryWarnings( bool enable, const QString &file, int line )
{
    setLocation( file, line );

    if (!enable) {
        d->error_msg = QTestMessage();
        d->query_failed = false;
    }
    d->query_warning_mode = enable;
}

/*!
    \internal
    Saves the \a file and \a line information for internal usage. Whenever an error situation occurs this data is appended to the error message to improve traceability of the error.

    It is usually not necessery to call this function directly.
*/
void QSystemTest::setLocation( const QString &file, int line )
{
    d->loc_fname = file;
    d->loc_line = line;
}

/*!
    \internal
    Returns the current filename, i.e. the file that was being executed when the error situation occurred.
*/
QString QSystemTest::currentFile()
{
    return d->loc_fname;
}

/*!
    \internal
    Returns the current line number, i.e. the line number that presumably caused the error situation.
*/
int QSystemTest::currentLine()
{
    return d->loc_line;
}

/*!
    \internal
    Saves the error string \a errString for future processing. If warning mode is enabled the error is written directly to std out as a warning.
    Base implementation returns false; subclasses may return true to indicate that the error is not considered "fatal".
*/
bool QSystemTest::setQueryError( const QString &errString )
{
    d->query_failed = true;
    d->error_msg["status"] = errString;
    d->error_msg_sent = d->last_msg_sent;
    if (d->query_warning_mode)
        qWarning( errString.toLatin1() );
    return false; // query is NOT successfull
}

/*!
    \internal
    Saves the error specified in the test \a message for future processing. If warning mode is enabled the error is written directly to std out as a warning.
    Base implementation returns false; subclasses may return true to indicate that the error is not considered "fatal".
*/
bool QSystemTest::setQueryError( const QTestMessage &message )
{
    d->query_failed = true;
    d->error_msg = message;
    d->error_msg_sent = d->last_msg_sent;
    if (d->query_warning_mode)
        qWarning( QString("%1 %2").arg(message.event()).arg(message.toString()).toLatin1().constData() );
    return false; // query is NOT successfull
}

/*!
    \internal
    Launch AUT and run the test.
*/
int QSystemTest::runTest(int argc, char *argv[]) {
    // Try to launch the aut script, and if successful, start executing the test.
    // QTest::qExec will also start the application event loop, and will not return until the tests have finished.

    return QAbstractTest::runTest(argc, argv);
}

/*!
    \internal
    Attempts to launch the aut script, and returns whether it was successful.
    The \c -remote command line argument is appended to the script with the IP set to the address of the
    local machine. This is to allow the test system to connect to the machine the test is running on.

    Any scripts used should support this.
*/
bool QSystemTest::connectToAut(int timeout)
{
    if (!d->test_app)
        d->test_app = new QSystemTestMaster( d );

    QTime t;
    t.start();

    qLog(QtUitest) << "Trying to connect to test app on" << qPrintable(d->aut_host) << d->aut_port;
    while (t.elapsed() < timeout && !isConnected()) {
        d->test_app->connect( d->aut_host, d->aut_port );
        d->test_app->waitForConnected(1000);
        if (!d->test_app->isConnected()) {
            if (d->test_app->error() == QAbstractSocket::ConnectionRefusedError) {
                qLog(QtUitest) << qPrintable(QString("Connection refused while trying to connect to test app on %1:%2").arg(d->aut_host).arg(d->aut_port)) ;
            }
            QTest::qWait(100);
        }
    }

    if (!d->test_app->isConnected()) {
        return false;
    }
    d->test_app->enableReconnect(true, 10000);
    if (d->demo_mode) setDemoMode(true);

    return true;
}

/*!
    \internal
*/
void QSystemTest::disconnectFromAut()
{ d->test_app->disconnect(); }

/*!
    \internal
*/
bool QSystemTest::isConnected() const
{ return (d && d->test_app && d->test_app->isConnected()); }

/*!
    \internal
*/
QSystemTest* QSystemTest::lastInstance()
{
    return QSystemTestPrivate::singleton;
}

/*!
    Returns the username that is running the test on the desktop machine.
*/
QString QSystemTest::userName()
{
    QString user_name;

#if defined Q_OS_TEMP
    user_name = "WinCE";
#elif defined Q_OS_WIN32
    user_name = ::getenv("USERNAME");
#elif defined Q_OS_UNIX
    user_name = ::getenv("USER");
    if (user_name == "")
        user_name = ::getenv( "LOGNAME" );
#elif defined Q_OS_MAC
    user_name = ::getenv();
#endif

    return user_name.toLower();
}

/*!
    Runs \a application with arguments \a args on the local machine,
    with \a input given as standard input. The combined stderr and stdout text will be returned.

    If the process fails to run or returns a non-zero exit code, the current test fails.
*/
QString QSystemTest::runProcess( const QString &application, const QStringList &args, const QString &input )
{
    QString output;
    QProcess p;
    p.setReadChannelMode(QProcess::MergedChannels);
    p.setReadChannel(QProcess::StandardOutput);
    p.start(application, args);
    if (p.waitForStarted()) {
        if (!input.isEmpty()) {
            p.write( input.toLatin1() );
        }
        p.closeWriteChannel();
        while (p.state() == QProcess::Running) {
            while (p.canReadLine()) {
                output += p.readLine();
            }
            wait(10); //important
        }
        while (p.canReadLine()) {
            output += p.readLine();
        }
        if (p.exitStatus() != QProcess::NormalExit) {
            setQueryError("Process didn't exit normally\n" + output);
            return QString();
        }
        int ec = p.exitCode();
        if (0 != ec) {
            QString err_str = QString("Process exited with exit code: %1\n%2").arg(ec).arg(output);
            setQueryError(err_str);
            return QString();
        }
        return output;
    }
    setQueryError("Process didn't start");
    return output;
}

/******************************************************************************
* DOCUMENTATION FOR FUNCTIONS IN builtins.js
******************************************************************************/

/*!
    Compares the \a actual string with the \a expected string and reports a fail
    with a nicely formatted error message in case the strings are not equal.

    Example:
    \code
        var my_variable1 = "Test";
        var my_variable2 = "Test2";
        compare( my_variable1, "Test" ); // passes
        compare( my_variable2, "Test" ); // will fail the test, and test execution will stop at this line
    \endcode

    \sa verify()
*/
#ifdef Q_QDOC
void QSystemTest::compare( const QString &actual, const QString &expected )
{
    // This code is implemented in the QtUiTestrunner bindings
}
#endif

/*!
    Aborts the test with a failure message if \a statement is false.
    The failure messages returned by compare() are generally speaking better readable (more informative) and is preferred when working with strings.

    Example:
    \code
        var my_variable1 = "Test";
        var my_variable2 = "Test2";
        verify( my_variable1 == "Test" ); // passes
        verify( my_variable2 == "Test" ); // will fail the test, and test execution will stop at this line
    \endcode

    \sa compare()
*/
#ifdef Q_QDOC
void QSystemTest::verify( bool statement )
{
    // This code is implemented in the QtUiTestrunner bindings
}
#endif

/*!
    \fn QSystemTest::waitFor(Number timeout, Number intervals, String message)

    Denotes the start of a block of code which should be repeatedly executed
    until it returns true.  If the block of code doesn't return true within
    \a timeout milliseconds, the current test fails with the given \a message.

    \a intervals is the maximum amount of times the block of code should be
    executed; i.e., the code will be executed every \a timeout / \a intervals
    milliseconds.

    Example:
    \code
    // Start scanning for bluetooth devices, and wait a while until the list of
    // bluetooth devices is not empty.
    select("Scan...",optionsMenu());
    waitFor() {
        return getList().length > 0;
    }
    \endcode
    */


/*
    \fn QSystemTest::defineTestData(String className, StringArray fields)

    Define a new class (named \a className) of test data which may contain the given \a fields.

    A test data class can be used to safely pass bundles of test data between functions.
    Unlike the use of regular objects with properties, attempting to read or
    write to nonexistent fields in a test data class will cause errors.

    Each field in \a fields will be accessible through appropriately named
    getter and setter methods.

    Example:
    \code
    defineTestData("PersonData", [ "firstName", "lastName", "age" ]);

    var pd = new PersonData({firstName: "Frank", lastName: "Jones"});

    pd.firstName();  // returns "Frank"
    pd.lastName();   // returns "Jones"
    pd.age();        // returns undefined

    pd.setAge(21);
    pd.age();        // returns 21

    pd.age;          // returns undefined - MUST use getter

    pd.middleName();    // Error: middleName is not a permitted field
    pd.setMiddleName(); // Error: middleName is not a permitted field

    pd = new PersonData({first_name: "Billy", age: 32}); // Error: first_name is not a permitted field
    \endcode
*/

/*!
    \fn QSystemTest::verify(Boolean condition, String message)

    Verifies that \a condition is true.  If \a condition is not true, the
    current test fails.  If \a message is given, it will be appended to
    the failure message.

    Example:
    \code
    select("Frank", "Contacts");
    waitForTitle("Details: Frank");
    var details = getText();
    // Verify that Frank's phone number is shown somewhere
    verify( details.contains("12345") );
    // Same, but with more details in error message
    verify( details.contains("12345"), "Frank's phone number is missing!" );
    \endcode
*/

/*!
    \fn QSystemTest::compare(Variant actual, Variant expected)

    Verifies that \a actual is equal to \a expected.  If this is not the case,
    the current test fails.

    Note that the order of \a actual and \a expected is significant, as it
    affects the test failure message.
*/

/*!
    \fn QSystemTest::fail(String message)

    Immediately fail the current test with the specified \a message.
*/

/*!
    Returns an alias that will be resolved by Qt Extended to find the Options Menu.
    Example:
    \code
        select( "New", optionsMenu() ); // select the item with text 'New' from the options menu
        print( signature(optionsMenu()) ); // to print the name of the resolved widget.
    \endcode
    Note that 'signature(optionsMenu())' incurs an extra roundtrip and should only be used in exceptional cases.

    The test will fail if no options menu can be activated, or if multiple option menus are found.

    \sa tabBar(), progressBar()
*/
QString QSystemTest::optionsMenu()
{
    return OPTIONS_MENU_ALIAS;
}

/*!
    Returns an alias that will be resolved by Qt Extended to find the currently active TabBar.
    Example:
    \code
        select( "Personal", tabBar() ); // select the tab with text 'Personal' from the tab bar.
        print( signature(tabBar()) ); // to print the name of the resolved widget.
    \endcode
    Note that 'signature(tabBar())' incurs an extra roundtrip and should only be used in exceptional cases.

    The test will fail if no visible tabbar is found, or if multiple tabbars are found.

    \sa optionsMenu(), progressBar()
*/
QString QSystemTest::tabBar()
{
    return TAB_BAR_ALIAS;
}

/*!
    Returns an alias that will be resolved by Qt Extended to find the currently visible progress bar.
    Example:
    \code
        print( getText(progressBar()) ); // prints the text currently shown by the progress bar.
        print( signature(progressBar()) ); // to print the name of the current progress bar.
    \endcode
    Note that 'signature(progressBar())' incurs an extra roundtrip and should only be used in exceptional cases.

    The test will fail if no visible progress bar is found, or if multiple progress bars are found.

    \sa optionsMenu(), tabBar()
*/
QString QSystemTest::progressBar()
{
    return PROGRESS_BAR_ALIAS;
}

/*!
    \internal
    Returns an identifier for the current runtime configuration of Qtopia.
    Includes mousePreferred(), screen size and theme.
*/
QString QSystemTest::configurationIdentifier() const
{ return d->config_id; }

/*!
    \internal
*/
void QSystemTest::setConfigurationIdentifier(QString const& config)
{ d->config_id = config; }

/*!
    \internal
*/
bool QSystemTest::verbose() const
{ return d->verbose; }

/*!
    \internal
*/
bool QSystemTest::doQuery(const QTestMessage& message, const QString& queryPath, QTestMessage* reply, int timeout, const QStringList& pass, const QStringList& fail)
{ return d->queryPassed(pass, fail, message, queryPath, reply, timeout); }

/*!
    \internal
    Called for each incoming message from the AUT.
*/
void QSystemTest::processMessage(const QTestMessage& message)
{
    QTestMessage copy(message);
    d->processMessage(&copy);
}

/*!
    Waits for the current title to change to \a title
    The test fails if the current title hasn't changed to \a title within \a timeout ms.
*/
void QSystemTest::waitForTitle( const QString &title, int timeout )
{
    d->resetUnexpectedDialog(title);

    if (!timeout) timeout = visibleResponseTime();

    QTestMessage replyMsg;
    QTestMessage queryMsg("waitForTitle");
    queryMsg["title"] = title;
    queryMsg["timeout"] = timeout;
    d->queryPassed( "OK", "", queryMsg, "", &replyMsg);
}

/*!
    \internal
    Prints \a value.
*/
void QSystemTest::print( QVariant const& value )
{
    QDebug(QtDebugMsg) << qPrintable(value.toString());
}

/*! \internal
    Enables/disables waits and animations for demo purposes.
*/
void QSystemTest::setDemoMode( bool enabled )
{
    QTestMessage queryMsg("enableDemoMode");
    queryMsg["enable"] = enabled;
    d->queryPassed( "OK", "", queryMsg, "qpe:" );
}

/*!
    \internal
*/
bool QSystemTest::demoMode() const
{ return d->demo_mode; }

/*!
    \internal
*/
QString QSystemTest::autHost() const
{ return d->aut_host; }

/*!
    \internal
*/
int QSystemTest::autPort() const
{ return d->aut_port; }

/*!
    \internal
*/
void QSystemTest::setIgnoreUnexpectedDialogs(bool ignore)
{ d->ignore_unexpected_dialogs = ignore; }

/*!
    \internal
*/
bool QSystemTest::ignoreUnexpectedDialogs() const
{ return d->ignore_unexpected_dialogs; }

/*!
    \internal
*/
void QSystemTest::clearUnexpectedDialog()
{ d->unexpected_dialog_title = QString(); }

/*!
    \internal
    Does whatever is necessary to select the given messagebox \a option
    from the currently shown \a messageBox .
*/
void QSystemTest::selectMessageBoxOption(QString const& option, QString const& messageBox)
{ select( option, messageBox ); }

/*!
    Uses the \a reason to mark the current testfunction as expected to fail.
*/
void QSystemTest::expectFail( const QString &reason )
{
    QEXPECT_FAIL(currentDataTag().toLatin1().constData(), qstrdup(reason.toLatin1().constData()), Abort);
}

/*!
    Returns whether the compile time \a option is defined.
*/
bool QSystemTest::isBuildOption( const QString &option )
{

    QTestMessage message("buildOption");
    message["option"] = option;
    QTestMessage reply;

    if (!d->queryPassed( "OK", "", BT(message), "", &reply )) return false;
    if (!reply["on"].isValid()) {
        fail("No data in reply to build options");
        return false;
    }
    return reply["on"].toBool();
}

/*! \typedef StringArray
    \relates QSystemTest

    An \l Array object in which every element is a \l String.
*/

/*! \typedef QVariantArray
    \relates QSystemTest

    An \l Array object in which every element is a \l QVariant.
*/

/*! \typedef Function
    \relates QSystemTest

    The Function type as documented in ECMA-262, section 15.3.
*/

/*! \typedef Array
    \relates QSystemTest

    The Array type as documented in ECMA-262, section 15.4.

    The following extensions are provided in QtUiTest.

    \table
    \row \o \tt{\l Boolean Array.prototype.contains(value)}
            \o Returns true if the array contains the given value.
    \endtable
*/

/*! \typedef String
    \relates QSystemTest

    The String type as documented in ECMA-262, section 15.5.

    The following extensions are provided in QtUiTest.

    \table
    \row \o \tt{ \l Boolean String.prototype.contains(String text)}
            \o Returns true if the string contains the given text.
    \row \o \tt{ \l Boolean String.prototype.contains(\l QRegExp regex)}
            \o Returns true if the string is matched by the given regular expression.
    \row \o \tt{ \l Boolean String.prototype.startsWith(String text)}
            \o Returns true if the string starts with the given text.
    \row \o \tt{ String String.prototype.left(\l Number n)}
            \o Returns the first n characters of the string.
    \row \o \tt{ String String.prototype.right(\l Number n)}
            \o Returns the last n characters of the string.
    \endtable
*/


/*! \typedef Boolean
    \relates QSystemTest

    The Boolean type as documented in ECMA-262, section 15.6.
*/

/*! \typedef Number
    \relates QSystemTest

    The Number type as documented in ECMA-262, section 15.7.
*/

#endif

#include "qsystemtest.moc"

