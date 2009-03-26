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

#include <qsystemtest.h>
#include "qsystemtestmaster_p.h"

#include "qtestverifydlg_p.h"
#include "qtestremote_p.h"
#include "recordevent_p.h"

#include <QProcess>
#include <QTextEdit>
#include <QMessageBox>

#include <QMetaEnum>

#include "qsystemtest_p.h"

#undef qLog
#define qLog(A) if (!verbose); else (QDebug(QtDebugMsg) << #A ":")

#define OBJECT_EXIST_TIMEOUT 1000

#define BT(message) (\
    message["location"] = QString("%1:%2%3").arg(__FILE__).arg(__LINE__).arg(!message["location"].toString().isEmpty() ? "\n" + message["location"].toString() : ""),\
    message)

#define DEFAULT_AUT_PORT 5656

QSystemTest *QSystemTestPrivate::singleton = 0;

QSystemTestPrivate::QSystemTestPrivate(QSystemTest *parent)
    : QObject(parent)
    , event_timer(0)
    , aut()
    , test_app(0)
    , recorded_events_edit(0)
    , recorded_events_as_code(true)
    , record_prompt(false)
    , query_warning_mode(true)
    , fatal_timeouts(1)
    , timeouts(0)
    , keyclickhold_key((Qt::Key)0)
    , current_application("")
    , loc_fname("")
    , loc_line(-1)
    , ignore_msg_boxes(false)
    , auto_mode(false)
    , aut_host("127.0.0.1")
    , aut_port(DEFAULT_AUT_PORT)
    , keep_aut(false)
    , silent_aut(false)
    , no_aut(false)
    , demo_mode(false)
    , verbose(false)
    , env()
    , wait_message_shown(false)
    , strict_mode(false)
    , visible_response_time(4000)
    , recorded_events()
    , recorded_code()
    , display_id(0)
    , device()
    , mousePreferred(false)
    , screenGeometry()
    , theme()
    , config_id()
    , got_startup_info(false)
    , recording_events(false)
    , unexpected_dialog_title()
    , ignore_unexpected_dialogs(false)
    , expect_app_close(false)
    , sample_memory_interval(-1)
    , p(parent)
{
    (void)qMetaTypeId<RecordEvent>();
    (void)qMetaTypeId< QList<RecordEvent> >();
}

QSystemTestPrivate::~QSystemTestPrivate()
{
    delete event_timer;
    delete test_app;
    while (expected_msg_boxes.count() > 0)
        delete expected_msg_boxes.takeFirst();
}

/*!
    \internal
    Determines whether to learn the new pixmap by displaying a prompt to the user.
    \a actual is the current snapshot, while \a expected is the stored pixmap (if any).
    If the user does not accept the new snapshot, a failure is generated for \a file at \a line, and
    \c false is returned. Otherwise the new snapshot replaces the previous stored one, and \c true is returned.
    \a comment is a comment intended to help the user decide whether or not to accept the pixmap.

    If this is a GUI application, the prompt is displayed directly. Otherwise it sends a message to
    any connected IDE's to display the message.
*/
bool QSystemTestPrivate::learnImage(const QImage &actual, const QImage &expected, const QString &comment)
{
    if (qtest_ide.isConnected()) {
        QTestMessage returnValue;
        QTestMessage msg("VERIFY_IMAGE");
        msg["actual"] = actual.isNull() ? QPixmap() : QPixmap::fromImage(actual);
        msg["expected"] = expected.isNull() ? QPixmap() : QPixmap::fromImage(expected);
        msg["comment"] = comment;

        if (qtest_ide.sendMessage( msg, returnValue, -1 ) && returnValue["status"] == "OK") {
            return returnValue["accepted"].toBool();
        } else {
            return p->fail( "Verification of pixmap not possible: remote test tool did not correctly respond." );
        }
    } else {
        QTestVerifyDlg dlg;
        dlg.setData(
                actual.isNull() ? QPixmap() : QPixmap::fromImage(actual),
                expected.isNull() ? QPixmap() : QPixmap::fromImage(expected),
                comment );
        if ( dlg.exec() == QDialog::Accepted )
            return true;
    }

    return false;
}

/*!
    \internal
    Compares the \a actual and \a expected images and returns true if the images are
    the same. If \a strict is false, small differences between the images are allowed.
*/
bool QSystemTestPrivate::imagesAreEqual( const QImage &actual, const QImage &expected, bool strict )
{
    // images are considered the same if both contain a null image
    if (actual.isNull() && expected.isNull())
        return true;

    // images are not the same if one images contains a null image
    if (actual.isNull() || expected.isNull())
        return false;

    // images do not have the same size
    if (actual.size() != expected.size())
        return false;

    QImage a = actual.convertToFormat(QImage::Format_ARGB32);
    QImage e = expected.convertToFormat(QImage::Format_ARGB32);

    bool equal = true;
    int threshold = 80;
    for ( int y=0; y<a.height(); y++ ) {
        for ( int x=0; x<a.width(); x++ ) {
            QRgb actPix = a.pixel( x, y );
            QRgb expPix = e.pixel( x, y );

            if ( qAlpha(actPix)==0 && qAlpha(expPix)==0 )
                continue;
            if ( actPix != expPix) {
                if (strict ||
                    (qAbs(qRed(actPix) - qRed(expPix)) +
                        qAbs(qGreen(actPix) - qGreen(expPix)) +
                        qAbs(qBlue(actPix) - qBlue(expPix)) > threshold)
                    ) {
                    equal = false;
                    goto done;
                }
            }
        }
    }

done:
    return equal;
}


/*!
    \internal
    Sends a query test message \a msg to the Application Under Test (AUT) identified by \a queryPath and returns the reply message. An error is logged in the testresult if the AUT doesn't respond within \a timeout ms.

    Note that it is seldom necessery to call query directly.
*/
QTestMessage QSystemTestPrivate::query( const QTestMessage &msg, const QString &queryPath, int timeout )
{
    QTestMessage message(msg);

    static QRegExp appSeperator("[^\\\\]:");
    // start with a sanity check
//    int pos1 = queryPath.indexOf( ":" );
    int pos1 = queryPath.indexOf( appSeperator );
    int pos2 = queryPath.indexOf( ">" );
    if (pos2 >= 0 && pos2 < pos1 || queryPath == "qpe/qpe:") {
        QTestMessage reply;
        reply["status"] = "MALFORMED_QUERY_PATH: '" + queryPath + "' for event: '" + msg.event() + "'";
        p->setQueryError(reply);
        return reply;
    }

    // split queryPath into application and widget description
    QString app;
    QString qp = queryPath;
    if (pos1 >= 0) {
        if (msg.event() == "getAppNames") {
            app = "qpe";
        } else {
            qp = queryPath.mid(pos1+2);
            app = queryPath.left(pos1+1);
            int pos = app.indexOf("/");
            if (pos >= 0) app = app.mid(pos+1);
            app = realAppName(app);
        }
    }
    qp.replace("\\:", ":");
    message["queryPath"] = qp;
    if (!app.isEmpty()) message["queryApp"] = app;

    query_failed = false;
    error_msg = QTestMessage();
    last_msg_sent = QTestMessage();

    if (-1 != fatal_timeouts && timeouts >= fatal_timeouts) {
        QTestMessage reply;
        reply["status"] = "ERROR_FATAL_TIMEOUT";
        p->setQueryError(reply);
        return reply;
    }

    QTestMessage reply;
    reply["status"] = "ERROR_UNSET_REPLY";

    last_msg_sent = message;

    if (test_app) {

        static int breakpoint_mode = -1;
        if (qtest_ide.isConnected()) {
            if (breakpoint_mode == -1) {
                QTestMessage reply;
                QTestMessage ide_msg("queryBreakpointMode");
                qtest_ide.sendMessage( ide_msg, reply, -1 );
                breakpoint_mode = reply["queryBreakpointMode"].toBool();
            }
            if (breakpoint_mode == 1) {
                QTestMessage ide_msg("queryBreakpoint");
                ide_msg["file"] = p->currentFile();
                ide_msg["line"] = QString("%1").arg(p->currentLine());
                QTestMessage reply;
                qtest_ide.sendMessage( ide_msg, reply, -1 );
            }
        }

        if (!test_app->sendMessage( message, reply, timeout )) {
            p->setQueryError(reply);
            return reply;
        }
    } else {
        p->setQueryError( "No Application Under Test (AUT) available" );
    }

    QString warning = reply["warning"].toString();
    if (!warning.isEmpty()) {
        qWarning("In response to '%s', received warning from test system: %s", qPrintable(message.event()), qPrintable(warning));
    }
    if (reply["status"] == "ERROR_REPLY_TIMEOUT" || reply["status"] == "ERROR_NO_CONNECTION") ++timeouts;

    return reply;
}

/*!
    \internal
    Sends a query specified with \a message to the Application Under Test (AUT). If the query was successfull and \a reply is not null the reply message is returned in \a reply.

    The function returns true if the query result string equals \a passResult and false if it equals
    \a failResult. The function also returns false in cases where a query failure has occurred.

    The \a queryPath can be used to specify which specific application/object the query is intended for.

    The \a timeout parameter can be used to limit the time the query will wait for a reply.

    If the function is NOT called from within a test macro such as QVERIFY, QCOMPARE, etc, the function
    will also write a warning message to the display describing the error.
*/
bool QSystemTestPrivate::queryPassed( const QStringList &passResult, const QStringList &failResult,
                                        const QTestMessage &message, const QString &queryPath, QTestMessage *reply, int timeout )
{
    QTime t;
    if (verbose) t.start();

    QTestMessage msg(message);

    if (expect_app_close) {
        msg["expectClose"] = "yes";
    }

    QTestMessage ret = query( BT(msg), queryPath, timeout );
    if (reply) *reply = ret;
    QString query_msg = message.event();
    if (query_msg == "getProperty")
        query_msg += "(" + message["property"].toString() + ")";

    if (!p->queryFailed()) {
        if (passResult.contains( ret["status"].toString() )) {
            if (verbose) qLog(QtUitest) <<  QString("query '%1' took %2 ms").arg(query_msg).arg(t.elapsed()).toLatin1() ;
            return true;
        }
        if (failResult.contains( ret["status"].toString() )) {
            if (verbose) qLog(QtUitest) <<  QString("query '%1' took %2 ms").arg(query_msg).arg(t.elapsed()).toLatin1() ;
            return false;
        }
    }

    if (verbose) qLog(QtUitest) <<  QString("query '%1' took %2 ms").arg(message.event()).arg(t.elapsed()).toLatin1() ;
    return p->setQueryError( ret );
}

/*!
    \internal
    \overload
    Equivalent to queryPassed() with passResult and failResult a single-element string list.
*/
bool QSystemTestPrivate::queryPassed( const QString &passResult, const QString &failResult,
                                        const QTestMessage &message, const QString &queryPath, QTestMessage *reply, int timeout )
{
    QStringList pass;
    QStringList fail;
    if (!passResult.isEmpty()) pass << passResult;
    if (!failResult.isEmpty()) fail << failResult;
    return queryPassed(pass, fail, message, queryPath, reply, timeout);
}


/*!
    \internal
    Implementation for the two public getSetting functions.
*/
QVariant QSystemTestPrivate::getSetting( const QString &org, const QString &app, const QString &file, const QString &group, const QString &key )
{
    QTestMessage message("getSetting");
    if (!org.isNull())  message["org"] = org;
    if (!app.isNull())  message["app"] = app;
    if (!file.isNull()) message["path"] = file;
    message["group"] = group;
    message["key"] = key;

    QVariant out;
    QTestMessage reply;
    if (!queryPassed("OK", "", BT(message), "qpe:", &reply)) return out;
    if (!reply.contains("getSetting")) {
        reply["status"] = "reply was missing return value";
        p->setQueryError(reply);
        return QVariant();
    }
    return reply["getSetting"];
}

/*!
    \internal
    Implementation for the two public setSetting functions.
*/
void QSystemTestPrivate::setSetting( const QString &org, const QString &app, const QString &file, const QString &group, const QString &key, const QVariant &value )
{
    QTestMessage message("setSetting");
    if (!org.isNull())  message["org"] = org;
    if (!app.isNull())  message["app"] = app;
    if (!file.isNull()) message["path"] = file;
    message["group"] = group;
    message["key"] = key;
    message["value"] = value;

    queryPassed("OK", "", BT(message), "qpe:");
}

/*!
    \internal
    Processes incoming messages from the system under test (such as Qtopia).
*/
bool QSystemTestPrivate::processMessage( QTestMessage *msg )
{
    // show a warning for an unexpected dialog (if we haven't cancelled the warning).
    if (msg->event() != "appGainedFocus"
        && msg->event() != "recordedEvents") showUnexpectedDialog();

    if (msg->event().startsWith("key")) {
        resetEventTimer();
        if (recorded_events_as_code)
            parseKeyEventToCode( msg );
        return true;
    } else if (msg->event().startsWith("mouse")) {
        resetEventTimer();
        return true;
    } else if (msg->event() == "information") {
        if (recorded_events_as_code)
            recordedEvent( msg );
        return true;
    } else if (msg->event() == "command") {
        if (recorded_events_as_code)
            recordedEvent( msg );
        return true;
    } else if (msg->event() == "recordedEvents") {
        recordedEvent( msg );
        return true;
    } else if (msg->event() == "appGainedFocus") {
        QString new_app = (*msg)["appName"].toString();
        if (new_app != current_application) {
            current_application = new_app;
            emit appGainedFocus(current_application);
        }
        return true;
    } else if (msg->event() == "appBecameIdle") {
        emit appBecameIdle((*msg)["appName"].toString());
    } else if (msg->event() == "show_messagebox") {
        onMessageBox(msg);
        return true;
    } else if (msg->event() == "show_dialog") {
        onDialog(msg);
        return true;
    }

    return false;
}

struct StaticQtMetaObject : public QObject
{
    static const QMetaObject *get()
    { return &static_cast<StaticQtMetaObject*> (0)->staticQtMetaObject; }
};

/*!
    \internal
*/
void QSystemTestPrivate::parseKeyEventToCode( QTestMessage *msg) {
    bool pressed = (msg->event() == "keyPress");

    Qt::Key key    = (Qt::Key)(*msg)["key"].toInt();
    Qt::KeyboardModifiers mod   = (Qt::KeyboardModifiers)(*msg)["mod"].toInt();
    bool autoRepeat = (*msg)["rep"].toBool();
    if (autoRepeat) return;

    QMetaObject const *qt = StaticQtMetaObject::get();
    QMetaEnum key_enum = qt->enumerator(qt->indexOfEnumerator("Key"));
    char const *key_str = key_enum.valueToKey(key);

    if (pressed) {
        key_hold_started = QTime::currentTime();
        return;
    }

    int timePressed = key_hold_started.msecsTo(QTime::currentTime());
    bool is_script = true; //p->inherits("QScriptSystemTest",QString());

    QString key_identifier = (is_script) ? "Qt." : "Qt::";
    key_identifier += QString::fromLatin1(key_str);

    QString code;
    if (!is_script) code += "QCHECKED( ";
    if (timePressed > 300) {
        code += QString("keyClickhold( %1, %2 )").arg( key_identifier ).arg( timePressed );
    } else {
        code += QString("keyClick( %1 )").arg( key_identifier );
    }
    if (!is_script) code += " )";
    code += ";";

    QTestMessage out("CODE");
    out["code"] = code;
    recordedEvent( &out );
}

/*!
    \internal
*/
bool QSystemTestPrivate::recentEventRecorded()
{
    if (event_timer == 0) {
        return false;
    } else {
        return event_timer->elapsed() < 4800;
    }
}

/*!
    \internal
*/
void QSystemTestPrivate::resetEventTimer()
{
    if (event_timer == 0) {
        event_timer = new QTime();
    }

    event_timer->start();
}

/*!
    \internal
    Signals the test that event recording needs to be switched on.
*/
void QSystemTestPrivate::recordPrompt()
{
    record_prompt = true;
}

void compressCode(QString &code)
{
    QString old_c;
    QString c = code;
    while (old_c != c) {
        old_c = c;

        // When using keypad, pressing select key will do
        // select( "Select|Accept|OK", softMenu() ), but this is almost always
        // implied by selecting something else.
        static QRegExp select(
                "select\\( \"(Select|Accept|OK)\", softMenu\\(\\) \\);\n"
                "(select\\( [^\n]+ \\);\n)",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != select.indexIn(c))
            c.replace(select, "\\2");

        // select( "Options", softMenu() ) is implied by selecting from
        // optionsMenu()
        static QRegExp selectOptionsMenu(
                "select\\( \"Options\", softMenu\\(\\) \\);\n"
                "(select\\( \"Select\", softMenu\\(\\) \\);\n)?"
                "(select\\( \"[^\n]+\", optionsMenu\\(\\) \\);\n)",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != selectOptionsMenu.indexIn(c))
            c.replace(selectOptionsMenu, "\\2");
        static QRegExp optionsMenuHideBefore(
                "select\\( \"Hide\", softMenu\\(\\) \\);\n"
                "(select\\( \"[^\n]+\", optionsMenu\\(\\) \\);\n)",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != optionsMenuHideBefore.indexIn(c))
            c.replace(optionsMenuHideBefore, "\\1");
        static QRegExp optionsMenuHideAfter(
                "(select\\( \"[^\n]+\", optionsMenu\\(\\) \\);\n)"
                "(//[^\n]+\n|waitForTitle\\( \"[^\n]+\" \\);\n)"
                "select\\( \"Hide\", softMenu\\(\\) \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != optionsMenuHideAfter.indexIn(c))
            c.replace(optionsMenuHideAfter, "\\1\\2");

        // toggling a checkbox is done by choosing "Select"
        // from soft menu.
        static QRegExp setCheckedSoft(
                "select\\( \"Select\", softMenu\\(\\) \\);\n"
                "(setChecked\\( [^\n]+ \\);\n)",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != setCheckedSoft.indexIn(c))
            c.replace(setCheckedSoft, "\\1");

        // toggling a checkbox also causes it to be selected.
        static QRegExp setCheckedSelect(
                "setChecked\\( (true|false), ([^\n]+) \\);\n"
                "select\\( \\2 \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != setCheckedSelect.indexIn(c))
            c.replace(setCheckedSelect, "setChecked( \\1, \\2 );\n");
        static QRegExp selectSetChecked(
                "select\\( ([^\n]+) \\);\n"
                "setChecked\\( (true|false), \\1 \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != selectSetChecked.indexIn(c))
            c.replace(selectSetChecked, "setChecked( \\2, \\1 );\n");

        // We get an enter event for each key press typically, so
        // compress enter("f"), enter("fo"), enter("foo") into enter("foo")
        static QRegExp enter(
                "enter\\( [^\n]+(, [^\n]+) \\);\n"
                "enter\\( ([^\n]+)\\1 \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != enter.indexIn(c))
            c.replace(enter, "enter( \\2\\1 );\n");

        // Do the above also when there is no querypath
        static QRegExp enter2(
                "enter\\( \"(?:[^\"\\\\]*(?:\\\\.)*)*\" \\);\n"
                "enter\\( \"((?:[^\"\\\\]*(?:\\\\.)*)*)\" \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != enter2.indexIn(c))
            c.replace(enter2, "enter( \"\\1\" );\n");

        // Fold "Accept" and "Edit" into enter().
        static QRegExp enterAccept(
                "(enter\\( [^\n]+ \\);\n)"
                "select\\( \"Accept\", softMenu\\(\\) \\);\n"
                // if title changed, then 'Accept' had a side effect.
                "(?!waitForTitle)(.)",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != enterAccept.indexIn(c))
            c.replace(enterAccept, "\\1\\2");
        static QRegExp enterEdit(
                "select\\( \"Edit\", softMenu\\(\\) \\);\n"
                "(enter\\( [^\n]+ \\);\n)",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != enterEdit.indexIn(c))
            c.replace(enterEdit, "\\1");

        // We get a select tab bar event each time we switch tabs, so
        // compress them.
        static QRegExp selectTabBar(
                "select\\( [^\n]+, tabBar\\(\\) \\);\n"
                "select\\( ([^\n]+), tabBar\\(\\) \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != selectTabBar.indexIn(c))
            c.replace(selectTabBar, "select( \\1, tabBar() );\n");

        // selecting from launcher menu implies raising the launcher menu,
        // so we can get rid of those steps.
        static QRegExp selectLauncherMenu(
                "select\\( (?:\"Menu\", softMenu\\(\\)|\"mainmenu\") \\);\n"
                "(?:waitForTitle\\( [^\n]+ \\);\n|select\\( \"Select\", softMenu\\(\\) \\);\n)*"
                "select\\( ([^\n]+), launcherMenu\\(\\) \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != selectLauncherMenu.indexIn(c))
            c.replace(selectLauncherMenu,
                    "select( \\1, launcherMenu() );\n"
                    "waitForTitle( \\1 );\n");

        // Figure out when we have selected something from a submenu of the launcher.
        static QRegExp selectLauncherMenuSub(
                "select\\( \"(Applications|Settings|Games)\", launcherMenu\\(\\) \\);\n"
                "waitForTitle\\( \"\\1\" \\);\n"
                "select\\( \"([^\n]+)\" \\);\n"
                "waitForTitle\\( \"\\2\" \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != selectLauncherMenuSub.indexIn(c))
            c.replace(selectLauncherMenuSub,
                    "select( \"\\1/\\2\", launcherMenu() );\n"
                    "waitForTitle( \"\\2\" );\n");

        // No point in having duplicate waitForTitle().
        static QRegExp waitForTitle(
                "(waitForTitle\\( [^\n]+ \\);\n){2}",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != waitForTitle.indexIn(c))
            c.replace(waitForTitle, "\\1");

        // Wrap code in an expectMessageBox.
        static QRegExp expectMessageBox(
                "\n"
                "([^\n]+\n)"
                "// Message box shown, title: \"([^\n]+)\", text: \"([^\n]+)\"\n"
                "select\\( \"([^\n]+)\", softMenu\\(\\) \\);\n",
                Qt::CaseSensitive, QRegExp::RegExp2);
        while (-1 != expectMessageBox.indexIn(c))
            c.replace(expectMessageBox,
                    "\n"
                    "expectMessageBox( \"\\2\", \"\\3\", \"\\4\" ) {\n"
                    "    \\1"
                    "}\n");

        // Not sure how to handle empty selects.  Strip them out.
        static QRegExp selectNothing(
                "select\\( (\"\")? \\);\n");
        while (-1 != selectNothing.indexIn(c))
            c.replace(selectNothing, QString());

        // Strip out redundant selects (eg, when selecting from combobox)
        static QRegExp selectTestAbstractItemView(
                "select\\( \"[^\n]+\", \"TestAbstractItemView\\[\\w*\\]\" \\);\n");
        while (-1 != selectTestAbstractItemView.indexIn(c))
            c.replace(selectTestAbstractItemView, QString());

        // Strip out redundant selects (eg, when selecting from combobox)
        static QRegExp selectSmoothList(
                "select\\( (\"[^\n]+\"), \"TestSmoothList\\[\\w*\\]\" \\);\n");
        while (-1 != selectSmoothList.indexIn(c))
            c.replace(selectSmoothList, "select( \\1 );\n");

    }
    code = c;
}

QString escapeString(QString const& in)
{
    QString out;
    foreach (QChar c, in) {
        if (c == '"')
            out += "\\\"";
        else if (c == '\\')
            out += "\\\\";
        else if (c == '\n')
            out += "\\n";
        else
            out += c;
    }
    return out;
}

QString widgetParameter(QString const& widget)
{
    QString ret;
    if (widget == SOFT_MENU_ALIAS)
        ret = "softMenu()";
    else if (widget == TAB_BAR_ALIAS)
        ret = "tabBar()";
    else if (widget == LAUNCHER_MENU_ALIAS)
        ret = "launcherMenu()";
    else if (widget == OPTIONS_MENU_ALIAS)
        ret = "optionsMenu()";
    else {
        ret = '"' + escapeString(widget) + '"';
    }
    return ret;
}

QString recordedEventsToCode( QList<RecordEvent> const &events )
{
    QStringList statements;
    QString focusWidget;
    foreach (RecordEvent e, events) {
        QString cmd;
        switch (e.type) {
            case RecordEvent::GotFocus:
                // There is currently no API for simply navigating to a widget.
                // If one were added, we would do something like this:
                // statements << QString("setFocus( \"%1\" );").arg(e.widget);
                focusWidget = e.widget;
                break;

            case RecordEvent::Entered:
                cmd = QString("enter( \"%1\"").arg(escapeString(e.data.toString()));
                if (!e.widget.isEmpty() && e.widget != focusWidget)
                    cmd += QString(", %1").arg(widgetParameter(e.widget));
                cmd += " );";
                statements << cmd;
                focusWidget = e.widget;
                break;

            case RecordEvent::Activated:
                statements << QString("select( %1 );").arg(widgetParameter(e.widget));
                focusWidget = QString();
                break;

            case RecordEvent::Selected:
                cmd = QString("select( \"%1\"").arg(escapeString(e.data.toString()));
                if (!e.widget.isEmpty() && e.widget != focusWidget)
                    cmd += QString(", %1").arg(widgetParameter(e.widget));
                cmd += " );";
                statements << cmd;
                focusWidget = QString();
                break;

            case RecordEvent::CheckStateChanged:
                cmd = QString("setChecked( %1").arg(e.data.toBool() ? "true" : "false");
                if (!e.widget.isEmpty() && e.widget != focusWidget)
                    cmd += QString(", %1").arg(widgetParameter(e.widget));
                cmd += " );";
                statements << cmd;
                focusWidget = e.widget;
                break;

            case RecordEvent::TitleChanged:
                statements << QString("waitForTitle( \"%1\" );").arg(escapeString(e.data.toString()));
                focusWidget = QString();
                break;

            case RecordEvent::MessageBoxShown: {
                QVariantMap map = e.data.toMap();
                QString title = map["title"].toString();
                QString text  = map["text"].toString();
                statements << QString("// Message box shown, title: \"%1\", text: \"%2\"").arg(escapeString(title)).arg(escapeString(text));
                focusWidget = QString();
                break;
            }

            default:
                statements << QString("// unknown event: %1 %2 %3").arg(e.type).arg(widgetParameter(e.widget)).arg(escapeString(e.data.toString()));
                break;
        }
    }

    return statements.join("\n");
}

/*!
    \internal
*/
void QSystemTestPrivate::recordedEvent( QTestMessage *msg ) {
    if (msg->event() == "recordedEvents") {
        recordEvents( (*msg)["events"].value< QList<RecordEvent> >() );
    }
}

void QSystemTestPrivate::recordEvents(QList<RecordEvent> const& events)
{
    recorded_events << events;
    QString out = recordedEventsToCode( recorded_events );
    recorded_events.clear();
    if (!out.isEmpty()) {
        recorded_code += out + "\n";

        // For debugging purposes, allow code compression to be skipped.
        static bool skip_compression = !qgetenv("QTUITEST_DEBUG_RECORD").isEmpty();
        if (!skip_compression) compressCode(recorded_code);
        if (recorded_events_edit && recorded_events_edit->isVisible()) {
            recorded_events_edit->setPlainText(recorded_code);
            recorded_events_edit->moveCursor(QTextCursor::End);
            recorded_events_edit->ensureCursorVisible();
        }
        if (qtest_ide.isConnected()) {
            QTestMessage ide_msg("recordedCode");
            ide_msg["code"] = recorded_code;
            qtest_ide.postMessage( ide_msg );
        }
    }
}

/*!
    \internal
*/
QString QSystemTestPrivate::realAppName(QString const &appName)
{
    if (appName == "qpe")
        return "qpe";

    if (appNameToBinary.isEmpty()) {
        QTestMessage msg("getAppNames");
        QTestMessage reply;
        if (queryPassed("OK", "", msg, "qpe:", &reply)) {
            foreach(QString s, reply["getAppNames"].toMap().keys()) {
                appNameToBinary[s] = reply["getAppNames"].toMap()[s].toString();
            }
        }
    }

    return appNameToBinary.contains(appName) ? appNameToBinary[appName] : appName.toLower();
}

/*!
    \internal
    Handle message boxes being displayed by the system and fail the test if the message box was unexpected.
*/
void QSystemTestPrivate::onMessageBox( QTestMessage const *message )
{
    if (message == 0) return;

    QString msgbox_err_str = "";
    QString msgbox_option = QString();
    Qt::Key msgbox_keyaction = (Qt::Key)0;
    QString msgbox_title = (*message)["title"].toString();
    QString msgbox_text = (*message)["text"].toString();
    QString msgbox_focus_widget = (*message)["signature"].toString();

    // If recording events, record the message box popup so we can generate
    // code for it.
    if (recording_events) {
        QVariantMap map;
        map["title"] = msgbox_title;
        map["text"]  = msgbox_text;

        RecordEvent event;
        event.type = RecordEvent::MessageBoxShown;
        event.data = map;
        recordEvents( QList<RecordEvent>() << event );
        return;
    }

    if (ignore_msg_boxes) {
        msgbox_err_str = QString("A message box was shown with:\n   Title: '%1'\n   Text: '%2'").arg(msgbox_title).arg(msgbox_text);
        qWarning( msgbox_err_str.toLatin1() );
        return;
    }

    QTestMessage msg;
    if (keyclickhold_key) {
        Qt::Key hold_key = keyclickhold_key;
        QString hold_path = keyclickhold_path;
        keyclickhold_key = (Qt::Key)0;
        keyclickhold_path = "";
        p->keyRelease(hold_key, hold_path);
        if (p->queryFailed( &msg )) {
            qWarning( msg.toString().toLatin1() );
        }
    }

    bool is_expected = false;

    for (int i=0; i<expected_msg_boxes.count(); i++) {
        QSystemTestPrivate::ExpectedMessageBox *box = expected_msg_boxes.at(i);
        if (box &&
            box->test_function == QTest::currentTestFunction() &&
            box->data_tag == QTest::currentDataTag() &&
            msgbox_title.contains( box->title ) &&
            msgbox_text.contains( box->text ))
        {
            msgbox_option = box->option;
            is_expected = true;
            // cleanup the expected box as a signal that we've catched it.
            delete expected_msg_boxes.takeAt(i);
            break;
        }
    }

    if (!is_expected) {
        msgbox_err_str = QString("A message box was shown unexpectedly with:\n   Title: '%1'\n   Text: '%2'").arg(msgbox_title).arg(msgbox_text);
        msgbox_keyaction = Qt::Key_Back;
        p->fail( msgbox_err_str );
        msgbox_err_str = "";
    }


    if (msgbox_keyaction != (Qt::Key)0) {
        Qt::Key key = msgbox_keyaction;
        msgbox_keyaction = (Qt::Key)0;
        QTest::qSleep(1000); // show the dialog very briefly
        p->keyClick( key, "" );
        QTime t;
        t.start();
        // wait until the messagebox is gone
        while (test_app->isConnected() && msgbox_focus_widget == p->focusWidget()) {
            QTest::qWait(200);
            if (t.elapsed() > 10000) {
                p->fail( "Timeout while trying to close a messagebox" );
                return;
            }
        }
    } else if (!msgbox_option.isEmpty()) {
        QTest::qSleep(1000); // show the dialog very briefly
        QString msgbox_focus_widget = p->focusWidget();
        p->selectMessageBoxOption(msgbox_option, msgbox_focus_widget);

        QTime t;
        t.start();
        // wait until the messagebox is gone
        while (test_app->isConnected() && msgbox_focus_widget == p->focusWidget()) {
            QTest::qWait(200);
            if (t.elapsed() > 10000) {
                p->fail( "Timeout while trying to close a messagebox" );
                return;
            }
        }

    }
}

/*!
    \internal
    Fail the test when a QDialog is shown by the server.
*/
void QSystemTestPrivate::onDialog( QTestMessage const *message )
{
    if (ignore_unexpected_dialogs) return;
    if (message == 0) return;

    // fail with an unexpected dialog if we don't run a 'waitForTitle' as the next command
    unexpected_dialog_title = (*message)["title"].toString();
}

/*!
    \internal
    Reports an unexpected dialog to the test.
*/
void QSystemTestPrivate::showUnexpectedDialog()
{
    if (ignore_unexpected_dialogs) {
        p->clearUnexpectedDialog();
        return;
    }
    if (!unexpected_dialog_title.isEmpty()) {
        if (expected_msg_boxes.count() == 0) {
            QString warn_message = QString("A Dialog was shown unexpectedly with title '%1'.\nHint: You probably want to do a 'waitForTitle(\"%1\");' before the current line: %2(%3).").arg(unexpected_dialog_title).arg(p->currentFile()).arg(p->currentLine());

            if (strict_mode)
                p->fail( warn_message );
            else
                qWarning( warn_message.toLatin1() );
            p->clearUnexpectedDialog();
        } else {
            p->clearUnexpectedDialog();
        }
    }
}

/*!
    \internal
    Cancels a future warning message for an unexpected dialog with the specified \a title.
*/
void QSystemTestPrivate::resetUnexpectedDialog( const QString &title )
{
    if (!unexpected_dialog_title.isEmpty()) {
        if (unexpected_dialog_title == title) {
            p->clearUnexpectedDialog();
        }
    }
}

/*
    On the desktop ONLY, ensures that the environment variable \a key is set to \a value.
    Returns true if a change was made, indicating that Qt Extended must be restarted before
    the change will take effect.
*/
bool QSystemTestPrivate::ensureEnvironment(QString const& key, QString const& value)
{
    Q_ASSERT(!p->runsOnDevice());

    QFile f(QString("/tmp/qtopia-%1/qtuitest_cmds").arg(display_id));
    if (!f.open(QIODevice::ReadWrite)) {
        p->setQueryError("Couldn't open " + f.fileName());
        return false;
    }

    QStringList in_lines = QString(f.readAll()).split('\n');
    f.close();
    QByteArray out_data;

    bool changed = false;
    bool handled = false;

    foreach (QString line, in_lines) {
        if (line.startsWith("export " + key + "=")) {
            handled = true;
            if (!value.isNull()) {
                QString out = QString("export %1=\"%2\"").arg(key).arg(value);
                out_data.append(out + "\n");
                changed = (out != line);
            } else {
                changed = true;
            }
        } else if (!line.isEmpty()) {
            out_data.append(line + "\n");
        }
    }
    if (!handled && !value.isNull()) {
        QString out = QString("export %1=\"%2\"").arg(key).arg(value);
        out_data.append(out + "\n");
        changed = true;
    }

    if (!f.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        p->setQueryError("Couldn't open " + f.fileName());
        return false;
    }

    f.write(out_data);

    if (changed && (p->getenv("QTOPIA_SYSTEMTESTHELPER") != "1")) {
        p->setQueryError("When not running on a device, Qtopia needs to be run with '-runmode systemtesthelper' to use this feature!");
        return false;
    }
    return changed;
}

