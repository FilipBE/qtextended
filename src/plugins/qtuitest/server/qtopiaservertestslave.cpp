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

#include <qtopiaservertestslave.h>
#include <private/testslaveinterface_p.h>

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QMenu>
#include <QDesktopWidget>
#include <qtopiabase/qtopianamespace.h>
#include <qtopiabase/qtopiaipcenvelope.h>
#include <qtopia/inputmethodinterface.h>
#include <qtestwidgets.h>
#include "qtestslaveglobal.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QWSWindow>
#include <QWSMouseHandler>
#include <QWSClient>
#include <qwsevent_qws.h>
//#include <qwindowsystem_qws.h>
#include <QtDebug>
#include <QValueSpaceItem>
#include <QtopiaApplication>
#include <QScreen>
#include <qtopia/qsoftmenubar.h>

#include <QContentSet>
#include <QContent>
#include <QPainter>
#include <QtUiTestRecorder>
#include <QLocale>

#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif

#include <custom.h>

#include <sys/time.h>
#include <time.h>

#include <private/demowidgets_p.h>
#include "qmemorysampler.h"
#include "qtuitestlogreader_p.h"

using namespace QtUiTest;

// Don't allow key events on a mouse-preferred device, and vice versa
#define STRICT_SIMULATED_EVENTS        // Warn
//#define ULTRA_STRICT_SIMULATED_EVENTS  // Assert

// timeout for an application to launch and connect to the test server socket.
static const unsigned int APP_LAUNCH_TIMEOUT = 60000;

// timeout for communication with a running application
static const int APP_COMMS_TIMEOUT = -1;

class QtUiTestServerSocket : public QTcpServer
{
public:
    QtUiTestServerSocket(QtopiaServerTestSlave *parent);

private:
    virtual void incomingConnection( int socket );
    QtopiaServerTestSlave *slave;
};

class QtUiTestProtocolServer : public QTestServerSocket
{
    Q_OBJECT
public:
    QtUiTestProtocolServer(QtopiaServerTestSlavePrivate *parent);
    virtual ~QtUiTestProtocolServer();

    virtual void onNewConnection( int socket );

    QtopiaServerQueryMaster * findApp( QString appName, uint timeout = 0 );
    void disconnectAllApplications();

    inline QList< QPointer<QtopiaServerQueryMaster> > connections()
    { return connectionList; }

public slots:
    void onConnectionClosed( QTestProtocol* );
    void onNewMessage( const QTestMessage &msg );

signals:
    void newMessage( const QTestMessage & );

private:
    QList< QPointer<QtopiaServerQueryMaster> > connectionList;
    QPointer<QtopiaServerTestSlavePrivate> m_parent;
};


class QtopiaServerQueryMaster : public QTestProtocol
{
    Q_OBJECT
public:
    QtopiaServerQueryMaster( int socket, QtopiaServerTestSlavePrivate *tp );
    virtual ~QtopiaServerQueryMaster();

    virtual void processMessage( QTestMessage *msg );
    void queryName();

    QString appName;

signals:
    void newMessage( const QTestMessage &msg );
private:
    QPointer<QtopiaServerTestSlavePrivate> m_testslave_p;
};

/* Handler for test messages */
class QtopiaServerTestSlavePrivate : public QObject
{
Q_OBJECT
public:
    QtopiaServerTestSlavePrivate(QtopiaServerTestSlave *parent)
        : p(parent),
          titleVs(0),
          lastTitle(),
          memorySampler(),
          memorySamplerTimer(),
          current_application(qApp->applicationName()),
          applications(),
          appServer(new QtUiTestProtocolServer(this)),
          logReader(this)
    { QTimer::singleShot(0, this, SLOT(init())); }

private slots:
    void init() {
        titleVs = new QValueSpaceItem("/UI/ActiveWindow/Caption", this);
        connect(titleVs, SIGNAL(contentsChanged()), this, SLOT(currentTitleChanged()));
        connect(&memorySamplerTimer, SIGNAL(timeout()), &memorySampler, SLOT(sample()));
        connect(&memorySampler, SIGNAL(sampled(int)), this, SLOT(sampledMemory(int)));
    }
public:
    bool querySlave(QTestMessage const &message, QTestMessage *reply);

    QPoint getCenter(const QString &app, const QString &signature, QTestMessage *reply, bool &ok);

    QPoint mousePointForMessage(QTestMessage const&,QTestMessage&,bool&);

public slots:
    QTestMessage keyPress           (QTestMessage const&);
    QTestMessage keyRelease         (QTestMessage const&);
    QTestMessage keyClick           (QTestMessage const&);

    QTestMessage mousePress         (QTestMessage const&);
    QTestMessage mouseRelease       (QTestMessage const&);
    QTestMessage mouseClick         (QTestMessage const&);
    QTestMessage mousePreferred     (QTestMessage const&);

    QTestMessage scrollByMouse      (QTestMessage const&);

    QTestMessage gotoHomeScreen     (QTestMessage const&);
    QTestMessage startApp           (QTestMessage const&);

    QTestMessage waitForTitle       (QTestMessage const&);
    QTestMessage waitValueSpace     (QTestMessage const&);
    QTestMessage getValueSpace      (QTestMessage const&);

    QTestMessage ipcSend            (QTestMessage const&);

    QTestMessage startEventRecording(QTestMessage const&);
    QTestMessage stopEventRecording (QTestMessage const&);

    QTestMessage currentApplication (QTestMessage const&);

    QTestMessage setInputMethodHint (QTestMessage const&);

    QTestMessage getAppNames        (QTestMessage const&);

    QTestMessage documentsPath      (QTestMessage const&);
    QTestMessage restartQtopia      (QTestMessage const&);

    QTestMessage enableDemoMode     (QTestMessage const&);

    QTestMessage setTimeSynchronization(QTestMessage const&);
    QTestMessage setTimeFormat      (QTestMessage const&);
    QTestMessage setDateFormat      (QTestMessage const&);
    QTestMessage setSystemTime      (QTestMessage const&);
    QTestMessage systemTime         (QTestMessage const&);
    QTestMessage timeFormat         (QTestMessage const&);
    QTestMessage dateFormat         (QTestMessage const&);
    QTestMessage setTimeZone        (QTestMessage const&);
    QTestMessage timeZone           (QTestMessage const&);

    QTestMessage sampleMemory       (QTestMessage const&);
    QTestMessage getClipboardText   (QTestMessage const&);
    QTestMessage setClipboardText   (QTestMessage const&);

    QTestMessage buildOption        (QTestMessage const&);
    QTestMessage getVersion         (QTestMessage const&);

    QTestMessage startLogRead       (QTestMessage const&);

private slots:
    void currentTitleChanged();
    void sampledMemory(int);
    void logError(QString const&);
    void log(QStringList const&);

public:
    void broadcast(QTestMessage const&);

    QtopiaServerTestSlave *p;

    QValueSpaceItem *titleVs;
    QString lastTitle;

    QMemorySampler memorySampler;
    QTimer         memorySamplerTimer;

    QString current_application;
    QVariantMap applications;
    QtUiTestProtocolServer *appServer;
    QtUiTestLogReader logReader;

    friend class QtopiaServerQueryMaster;
    friend class QtopiaServerTestSlave;
};

#define RET(message, str) (\
    message["status"] = str,\
    message["location"] = QString("%1:%2%3").arg(__FILE__).arg(__LINE__).arg(!message["location"].toString().isEmpty() ? "\n" + message["location"].toString() : ""),\
    message)

class KeyFilter : public QWSServer::KeyboardFilter
{
    public:
        KeyFilter(QtopiaServerTestSlave *parent) : m_parent(parent) {
            QWSServer::addKeyboardFilter(this);
        }

        bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat) {
            Q_UNUSED(unicode);
            m_parent->keyboardFilter(keycode, modifiers, isPress, autoRepeat);
            return false;
        }

    private:
        QtopiaServerTestSlave *m_parent;
};

QWidget* inputMethodsWidget()
{
    static QWidget* inputMethods = 0;
    if (!inputMethods) {
        QWidgetList wl(QApplication::topLevelWidgets());
        foreach (QWidget *w, wl) {
            inputMethods = w->findChild<QWidget*>("InputMethods");
            if (inputMethods) break;
        }
    }
    return inputMethods;
}

QObject* inputMethodService()
{
    static QObject* ims = 0;
    if (ims) return ims;

    QObject *im = inputMethodsWidget();
    if (!im) return ims;
    foreach (QObject *child, im->children()) {
        if (child->inherits("InputMethodService")) {
            ims = child;
            break;
        }
    }
    return ims;
}

QString currentInputMethod()
{
    QString ret;
    if (inputMethodsWidget()) {
        QMetaObject::invokeMethod(inputMethodsWidget(), "currentShown",
                Qt::DirectConnection, Q_RETURN_ARG(QString, ret));
    }
    return ret;
}

// ***********************************************************************
// ***********************************************************************

/*
    This simple class sets up a TCP server that will listen for incoming connections from the Qt Extended Test Framework.
    An incoming connection will be hooked up to the testserver slave instance.

    A default port value is used, but an alternative value may be specified with the -remote <port> command line option.
*/
QtUiTestServerSocket::QtUiTestServerSocket(QtopiaServerTestSlave *parent) : QTcpServer(parent), slave(parent)
{
    setMaxPendingConnections( 1 );

    quint16 my_port = 5656;

    bool found = false;
    for (int i = 1; i<qApp->arguments().count(); i++) {
        if (qApp->arguments()[i] == "-remote") {
            if (found)
                qWarning("Multiple -remote arguments found, only the first will be used");
            else {
                found = true;
                QString ip_port = qApp->argv()[i+1];
                int pos = ip_port.indexOf( ":" );
                if (pos >= 0)
                    my_port = ip_port.mid(pos+1).toInt();
                else
                    my_port = ip_port.toInt();
            }
        }
    }

    listen( QHostAddress::Any, my_port );

    if (this->serverPort() == 0) {
        qWarning( QString("ERROR: port '%1' is already in use. System testing is not possible.").arg(my_port).toAscii() );
//        QApplication::exit(777);
    } else {
        qLog(QtUitest) << qPrintable(QString("QtUiTest is using port '%1' for System testing.").arg(my_port) );
    }
}

void QtUiTestServerSocket::incomingConnection( int socket )
{
    // hook up our internal system test slave to an incoming test connection
    slave->close();
    slave->setSocket( socket );
}

// ***********************************************************************
// ***********************************************************************

QtopiaServerTestSlave::QtopiaServerTestSlave()
    : QTestSlave()
    , waitVS(0)
    , m_keyfilter(0)
    , d(new QtopiaServerTestSlavePrivate(this))
{
    QObject::connect(QWSServer::instance(), SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)), this, SLOT(onWindowEvent(QWSWindow*,QWSServer::WindowEvent)));
    QObject::connect( d->appServer, SIGNAL(newMessage(QTestMessage)), this, SLOT(onNewMessage(QTestMessage)) );

    new QtUiTestServerSocket(this);
}

QtopiaServerTestSlave::~QtopiaServerTestSlave()
{
    delete d;
}

void QtopiaServerTestSlave::recordEvent(RecordEvent::Type type, QString const& widget, QString const& focusWidget, QVariant const& data)
{
    RecordEvent event;
    event.type = type;
    event.widget = widget;
    event.focusWidget = focusWidget;
    event.data = data;

    QList<RecordEvent> unpostedEvents;
    unpostedEvents << event;

    QTestMessage msg("recordedEvents");
    msg["events"] = QVariant::fromValue(unpostedEvents);
    postMessage(msg);
}

extern int qws_display_id;

void QtopiaServerTestSlave::onConnected()
{
    QTestSlave::onConnected();

    QTestMessage message("startupInfo");
    message["display_id"] = qws_display_id;
#ifdef QTOPIA_COMPATIBLE_DEVICES
    message["device"] = QString::fromLatin1(QTOPIA_COMPATIBLE_DEVICES).toLower();
#endif
    message["mousePreferred"] = Qtopia::mousePreferred();

    QDesktopWidget *desktop = QApplication::desktop();
    message["screenGeometry"] = desktop->screenGeometry(desktop->primaryScreen());

    // The theme includes the ".conf" part which we don't need
    QSettings cfg( "Trolltech", "qpe" );
    QString theme = cfg.value("Appearance/Theme").toString();
    message["theme"] = theme.left(theme.size() - 5);

    postMessage(message);
}

void QtopiaServerTestSlave::keyboardFilter(int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    QtUiTestRecorder::emitKeyEvent(keycode, modifiers, isPress, autoRepeat);
}

QTestMessage QtopiaServerTestSlavePrivate::keyPress(QTestMessage const &message)
{
    QTestMessage reply;
    if (!message["key"].isValid()) {
        return RET(reply, "ERROR_MISSING_KEY");
    }

    Qt::Key key = (Qt::Key)(message["key"].toInt());
    if (!key) return RET(reply, "ERROR_ZERO_KEY");

    QtUiTest::keyPress(key);

    if (message["duration"].isValid()) {
        int duration = message["duration"].toInt();
        if (duration >= 500) {
            QtUiTest::wait(500);
            duration -= 500;
            QtUiTest::keyPress(key, 0, QtUiTest::KeyRepeat);
            while (duration > 0) {
                QtUiTest::wait(150);
                duration -= 150;
                QtUiTest::keyPress(key, 0, QtUiTest::KeyRepeat);
            }
        } else {
            QtUiTest::wait(duration);
        }
    }

    return RET(reply, "OK" );
}

QPoint QtopiaServerTestSlavePrivate::mousePointForMessage(QTestMessage const &message, QTestMessage &reply, bool &ok)
{
    ok = false;
    if (message["pos"].isValid() && message["pos"].canConvert<QPoint>()) {
        ok = true;
        return message["pos"].value<QPoint>();
    }

    return getCenter( message["queryApp"].toString(), message["queryPath"].toString(), &reply, ok );
}

QTestMessage QtopiaServerTestSlavePrivate::mousePress(QTestMessage const &message)
{
    QTestMessage reply;
    bool ok;
    QPoint pos(mousePointForMessage(message, reply, ok));
    if (ok) QtUiTest::mousePress(pos);
    else return reply;
    return RET(reply, "OK" );
}

QTestMessage QtopiaServerTestSlavePrivate::mouseRelease(QTestMessage const &message)
{
    QTestMessage reply;
    bool ok;
    QPoint pos(mousePointForMessage(message, reply, ok));
    if (ok) QtUiTest::mouseRelease(pos);
    else return reply;
    return RET(reply, "OK" );
}

QTestMessage QtopiaServerTestSlavePrivate::mouseClick(QTestMessage const &message)
{
    QTestMessage reply;
    bool ok;
    while (1) {
        QPoint pos(mousePointForMessage(message, reply, ok));
        if (!ok) return reply;

        if (reply["isVisible"].isValid() && !reply["isVisible"].toBool()) {
            reply = scrollByMouse(message);
            if (!reply.statusOK()) return reply;
            continue;
        }
        QtUiTest::mouseClick(pos);
        break;
    }
    return RET(reply, "OK" );
}

QTestMessage QtopiaServerTestSlavePrivate::mousePreferred(QTestMessage const &/*message*/)
{
    QTestMessage reply;
    reply["mousePreferred"] = Qtopia::mousePreferred();
    return RET(reply, "OK" );
}

QTestMessage QtopiaServerTestSlavePrivate::scrollByMouse(QTestMessage const &msg)
{
    QTestMessage reply;
    QTestMessage message("getCenter", msg);
    QString qp = message["queryPath"].toString();
    qLog(QtUitest) << "Try scrolling to" << (qp + (!qp.isEmpty() ? "/" : "") + message["item"].toString());

    if (!querySlave(message, &reply)) return reply;
    if (!reply.statusOK()) return reply;
    QPoint target = reply["getCenter"].value<QPoint>();

    if (qLogEnabled(QtUitest) && reply["isVisible"].toBool())
        qLog(QtUitest) << "scrollByMouse: it's already visible";

    for (int i = 0; i < 100 && !reply["isVisible"].toBool(); ++i) {
        qLog(QtUitest) << "Scroll attempt" << i;

        QList<QPoint> points;
        foreach (QVariant v, reply["navigateByMouse"].toList()) {
            points << v.value<QPoint>();
        }

        if (points.isEmpty()) {
            return RET(reply, "ERROR: widget or item " + qp
                    + " is not visible and I can't figure out how to make it visible.");
        }
        qLog(QtUitest) << "Desired widget is currently at" << target;

        foreach (QPoint p, points) {
            QtUiTest::mouseClick(p);
        }

        if (reply["navigateByMouseWait"].toInt()) {
            QtUiTest::wait(reply["navigateByMouseWait"].toInt());
        }

        if (!querySlave(message, &reply)) return reply;
        if (!reply.statusOK()) return reply;
        target = reply["getCenter"].value<QPoint>();
    }

    if (reply["isVisible"].toBool())
        return RET(reply, "OK");
    else
        return RET(reply, "ERROR: couldn't make " + qp + " visible by scrolling");
}

QTestMessage QtopiaServerTestSlavePrivate::documentsPath(QTestMessage const &)
{
    QTestMessage reply;
    reply["documentsPath"] = Qtopia::documentDir();
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::restartQtopia(QTestMessage const &message)
{
    QTestMessage reply;
    int mode = message["mode"].toInt();
    if (mode != 1 && mode != 2 && mode != 3)
        return RET(reply, "ERROR: Invalid shutdown mode" );

    // kill all connections to Qtopia applications
    appServer->disconnectAllApplications();

    qRegisterMetaType<int>("QtopiaServerApplication::ShutdownType");

    bool ok =
        QMetaObject::invokeMethod(qApp, "shutdown", Qt::QueuedConnection,
            QArgument<int>("QtopiaServerApplication::ShutdownType", mode));

    return RET(reply,ok
            ? "OK"
            : "Error invoking QtopiaServerApplication::shutdown");
}

QTestMessage QtopiaServerTestSlavePrivate::enableDemoMode(QTestMessage const &message)
{
    QTestMessage reply;

    if (!message["enable"].isValid())
        return RET(reply, "ERROR: missing 'enable' parameter" );

    QtUiTest::setInputOption(QtUiTest::DemoMode, message["enable"].toBool());
    return RET(reply,"OK");
}

QTestMessage QtopiaServerTestSlavePrivate::keyRelease(QTestMessage const &message)
{
    QTestMessage reply;
    if (!message["key"].isValid()) {
        return RET(reply, "ERROR_MISSING_KEY");
    }
    Qt::Key key = (Qt::Key)(message["key"].toInt());
    if (!key) return RET(reply, "ERROR_ZERO_KEY");
    QtUiTest::keyRelease(key);
    return RET(reply, "OK" );
}

QTestMessage QtopiaServerTestSlavePrivate::keyClick(QTestMessage const &message)
{
    QTestMessage reply;
    if (!message["key"].isValid()) {
        return RET(reply, "ERROR_MISSING_KEY");
    }

    Qt::Key key = (Qt::Key)(message["key"].toInt());
    if (!key) return RET(reply, "ERROR_ZERO_KEY");
    QtUiTest::keyClick(key);
    return RET(reply, "OK" );
}

/*
    Returns a list of applications which do not have any registered tasks
    aside from their UI, and therefore should be closable.
*/
QStringList closableApplications()
{
    QStringList ret;
    foreach (QString app, QValueSpaceItem("/System/Applications").subPaths() ) {
        if ((QValueSpaceItem("/System/Applications/" + app + "/Tasks").subPaths()
                == QStringList("UI")) &&
            (QValueSpaceItem("/System/Applications/" + app + "/Tasks/UI").value() == true) )
            ret << app;
    }
    return ret;
}

QTestMessage QtopiaServerTestSlavePrivate::gotoHomeScreen(QTestMessage const &message)
{
    QTestMessage reply;
    bool backgroundApp = (message["bg"].isValid()) ? message["bg"].toBool() : false;

    { QtopiaIpcEnvelope e("QPE/System", "showHomeScreen()"); }

    if (!backgroundApp) {
        // Try to close all apps before we return.
        { QtopiaIpcEnvelope e("QPE/System", "close()"); }
        for (int i = 0; i < 10000 && !closableApplications().isEmpty(); i += 1000, QtUiTest::wait(1000)) {}
        QStringList apps = closableApplications();
        if (!apps.isEmpty()) {
            return RET(reply, "Error: one or more applications failed to close: " + apps.join(","));
        }
    }

    /* FIXME add a real check that we are at the home screen here.
     * Right now we are only checking that qpe has focus.
     */
    int i;
    for (i = 0;
         i < 5000 && current_application != qApp->applicationName();
         i += 100, QtUiTest::wait(100))
    {}
    if (i >= 5000) return RET(reply, "qpe did not gain focus within 5 seconds");

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::waitForTitle( QTestMessage const &message )
{
    QTestMessage reply;
    QElapsedTimer t;
    // The current title depends on the current application and when we start this query we may very well be looking at
    // the 'old' current application. So repeat the query each time with what we 'think' is currently the active application.
    QString title = message["title"].toString();
    if (title.isEmpty())
        return RET(reply, QString("ERROR: No title specified"));

    int timeout = message["timeout"].toInt();

    // Use QTestSlave::currentTitle()
    while (t.elapsed() < timeout) {
        querySlave(QTestMessage("currentTitle",current_application),&reply);
        if (reply.statusOK()) {
            if (reply["currentTitle"].toString() == title)
                return RET(reply,"OK");
        }
        // wait a little and try again
        QtUiTest::wait(10);
    }
/*
    QTestMessage msg(message);
    msg["delay"] = 100; // don't spend to much time in one query ... we may be querying the wrong application
    // Use QTestSlave::waitForTitle()
    while (t.elapsed() < timeout) {
        msg["queryApp"] = current_application;
        querySlave(msg,&reply);
        if (reply.statusOK()) {
            return reply;
        }
        // wait a little if we're qpe because we're not calling a processEvents in querySlave for this case
        if (current_application == "qpe")
            QtUiTest::wait(10);
    }
*/

    return RET(reply, QString("ERROR: Title did not change to '%1' within %2 ms. Current title is '%3'.").arg(title).arg(t.elapsed()).arg(reply["currentTitle"].toString()));
}

QTestMessage QtopiaServerTestSlavePrivate::waitValueSpace(QTestMessage const &message)
{
    QTestMessage reply;
    if (p->waitVS != 0) {
        qLog(QtUitest) << "The last waitValueSpace msg was not cleaned up properly";
        delete p->waitVS;
        p->waitVS = 0;
    }

    QString vsPath = message["path"].toString();
    p->waitValue = message["value"];

    p->waitVS = new QValueSpaceItem(vsPath);

    // If we are waiting for a certain value, and that value is already set, then no need to wait
    if ( p->waitVS->value() == p->waitValue )
        p->waitTrigger();
    else
        QObject::connect( p->waitVS, SIGNAL(contentsChanged()), p, SLOT(waitTrigger()) );
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::startApp(QTestMessage const &message)
{
    QTestMessage reply;

    qLog(QtUitest) <<  "startApp" ;
    qLog(QtUitest) <<  message.toString().toLatin1() ;
    QString subMenu = message["sub"].toString();
    QString appName = message["app"].toString();
    if (subMenu.isEmpty()) {
        int pos = appName.indexOf("/");
        while (subMenu.isEmpty() && pos > 0) {
            if (appName[pos-1] != '\\') {
                subMenu = appName.left(pos);
                appName = appName.mid(pos+1);
            } else {
                pos = appName.indexOf("/",pos+1);
            }
        }
    }
    appName.replace(QString("\\/"),QString("/"));
    subMenu.replace(QString("\\/"),QString("/"));
    qLog(QtUitest) <<  QString("startApp: %1 submenu: %2").arg(appName).arg(subMenu).toLatin1() ;

    if (appName.isEmpty()) return RET(reply, "ERROR: No application name specified");

    QString realName = p->realAppName(appName);

    QValueSpaceItem vsAppState( QString("/System/Applications/%1/Tasks/UI").arg(realName) );

    if (realName != "simapp") {
        /* If this app is already running, try hard to close it */
        QElapsedTimer t;
        while (t.elapsed() < 5000 && vsAppState.value().toString() == "true" ) {
            QtUiTest::wait(500);
            QtopiaIpcEnvelope closeE("QPE/Application/" + realName, "close()");
        }

        if (vsAppState.value().toString() == "true") {
            return RET(reply, "ERROR: Application '" + realName + "' is already running and cannot be closed" );
        }
    }

    { QtopiaIpcEnvelope e2("QPE/Application/" + realName, "raise()"); }

    // If the app isn't running yet, hook up a signal spy to the application state, and wait until it's running
//    QValueSpaceItem vsAppState( QString("/System/Applications/%1/Info/State").arg(realName) );
    if ( vsAppState.value().toString() != "true" ) {
        QElapsedTimer timeout;
        timeout.start();
        while (QtUiTest::waitForSignal( &vsAppState, SIGNAL(contentsChanged()), APP_LAUNCH_TIMEOUT - timeout.elapsed())) {
            qLog(QtUitest) <<  QString("startApp state changed to %1").arg(vsAppState.value().toString()).toLatin1() ;
            if (vsAppState.value().toString() == "true") {
                if (realName != "simapp") {
                    if (!p->findSlaveForApp( realName ))
                        return RET(reply, "ERROR: Application '" + realName + "' did not connect to the test framework");
                }
                return RET(reply, "OK");
            }
        }
        qLog(QtUitest) <<  QString("startApp timed out (%1 ms elapsed)").arg(timeout.elapsed()).toLatin1() ;
        return RET(reply, "ERROR: Timeout while starting application '" + realName + "'");
    }

    if (realName != "simapp") {
        if (!p->findSlaveForApp( realName ))
            return RET(reply, "ERROR: Application '" + realName + "' did not connect to the test framework");
    }

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::getValueSpace(QTestMessage const &message)
{
    QTestMessage reply;
    QString path = message["path"].toString();
    if (path.isEmpty()) return RET(reply, "ERROR_MISSING_PARAMETERS");

    QValueSpaceItem item( path );
    reply["getValueSpace"] = item.value();
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::startEventRecording(QTestMessage const &message)
{
    QTestMessage reply;
    p->setRecordingEvents(true);
    broadcast(message);
    if (!p->m_keyfilter) p->m_keyfilter = new KeyFilter(p);
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::stopEventRecording(QTestMessage const &message)
{
    QTestMessage reply;
    p->setRecordingEvents(false);
    broadcast(message);
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::ipcSend(QTestMessage const &message)
{
    QTestMessage reply;
    QString channel = message["channel"].toString();
    QString msg = message["message"].toString();
    msg.remove(' ');

    if (channel.isEmpty() || msg.isEmpty())
        return RET(reply, "ERROR_MISSING_PARAMETERS");

    QVariantList argList = message["args"].toList();

    QStringList paramTypes;
    QString params = msg.mid(msg.indexOf('(')+1);
    params.chop(1);
    if (params.contains(','))
        paramTypes = params.split(',');

    if (paramTypes.count() != argList.count()) {
        return RET(reply, "ERROR_ARGUMENT_MISMATCH");
    }

    {
        QtopiaIpcEnvelope e(channel, msg);
        for (int i = 0; i < argList.count(); ++i) {
            int type = QMetaType::type(qPrintable(paramTypes[i]));
            if (!QMetaType::save(e, type, argList[i].constData())) {
                reply["warning"] = QString("failed to save argument %1 (of type %2)").arg(i).arg(paramTypes[i]);
                return RET(reply, "ERROR_SAVING_ARGUMENT");
            }
        }
    }

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::currentApplication(QTestMessage const &message)
{
    QTestMessage reply;
    Q_UNUSED(message);

    reply["currentApplication"] = current_application;
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::setInputMethodHint(QTestMessage const &message)
{
    QTestMessage reply;

    QObject *ims = inputMethodService();

    if (!ims) return RET(reply, "ERROR_GETTING_INPUT_METHOD_SERVICE");

    if (message["hint"].isNull()) return RET(reply, "ERROR_MISSING_PARAMETERS");

    foreach(QWSWindow *w, QWSServer::instance()->clientWindows()) {
        if (!w) continue;
        QMetaObject::invokeMethod(ims, "inputMethodHint", Qt::DirectConnection,
                Q_ARG(QString, message["hint"].toString()),
                Q_ARG(int, w->winId()));
    }
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::getAppNames(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;

    if (applications.isEmpty()) {
        p->realAppName("foo");
    }

    reply["getAppNames"] = applications;
    return RET(reply, "OK");
}

bool QtopiaServerTestSlavePrivate::querySlave(QTestMessage const &message, QTestMessage *reply)
{
    QString app = message["queryApp"].toString();

    if (app == "qpe") {
        *reply = p->QTestSlave::constructReplyToMessage(message);
        return reply->statusOK();
    }
    QPointer<QtopiaServerQueryMaster> slave = p->findSlaveForApp( app );
    if (slave) {
        if (!slave->sendMessage( message, *reply, APP_COMMS_TIMEOUT )) {
            return false;
        }
    } else {
        (*reply)["status"] = "ERROR: Couldn't find application '"+app+"' for queryPath '"+message["queryPath"].toString()+"'";
    }
    return reply->statusOK();
}

QPoint QtopiaServerTestSlavePrivate::getCenter( const QString &app, const QString &signature,QTestMessage *reply, bool &ok  )
{
    if (!querySlave(QTestMessage("getCenter",app,signature), reply)) return QPoint();
    ok = true;
    return (*reply)["getCenter"].value<QPoint>();
}

class BroadcastSpy : public QObject
{
    Q_OBJECT

public slots:
    void onReplyReceived(int id)
    {
        replies << id;
        if (replies == expectedReplies)
            emit allRepliesReceived();
    }

public:
    QSet<int> replies;
    QSet<int> expectedReplies;

signals:
    void allRepliesReceived();
};

void QtopiaServerTestSlavePrivate::broadcast(QTestMessage const &message)
{
    BroadcastSpy* spy = new BroadcastSpy;

    foreach (QPointer<QtopiaServerQueryMaster> slave, appServer->connections()) {
        if (slave && !slave->appName.isEmpty() && !slave->appName.endsWith("_callback_slave")) {
            connect(slave, SIGNAL(replyReceived(int,const QTestMessage*)), spy, SLOT(onReplyReceived(int)));
            spy->expectedReplies << slave->postMessage(message);
        }
    }

    const int timeout = 2000;

    if (!QtUiTest::waitForSignal(spy, SIGNAL(allRepliesReceived()), timeout)) {
        qWarning() << "QtUitest:" << spy->expectedReplies.count() << " app(s) "
                      "failed to respond to broadcast of" << message.event()
                   << "within" << timeout << "ms";
    }
    spy->deleteLater();
}

QTestMessage QtopiaServerTestSlavePrivate::setTimeSynchronization(QTestMessage const &message)
{
    QTestMessage reply;
    QString mode = message["mode"].toString();
    if (mode != "auto" && mode != "manual") return RET(reply, "ERROR: invalid time synchronization mode '" + mode + "'");

    // FIXME: This code is copied from the systemtime application and therefore very fragile!!
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);

    // Need to process the QCOP event generated
    QtUiTest::wait(0);

    bool auto_timezone = (mode == "auto");
    bool auto_time = (mode == "auto");
    {
        QSettings lconfig("Trolltech","locale");
        lconfig.beginGroup( "Location" );
//        lconfig.setValue( "Timezone", tz->currentZone() );
        lconfig.setValue( "TimezoneAuto", auto_timezone );
        lconfig.setValue( "TimeAuto", auto_time );
        lconfig.setValue( "TimezoneAutoPrompt", auto_timezone );
        lconfig.setValue( "TimeAutoPrompt", auto_time );
    }

    // Restore screensaver
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::setTimeFormat(QTestMessage const &message)
{
    QTestMessage reply;
    QString mode = message["mode"].toString();
    if (mode != "24" && mode != "12") return RET(reply, "ERROR: invalid time format '" + mode + "'");
    bool ampm = (mode == "12");

    // FIXME: This code is copied from the systemtime application and therefore very fragile!!
    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );
    int show12hr = config.value("AMPM").toBool() ? 1 : 0;
    if ( show12hr != ampm) {
        config.setValue( "AMPM", !!ampm );
        QtopiaIpcEnvelope setClock( "QPE/System", "clockChange(bool)" );
        setClock << (int)ampm;
    }

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::timeFormat(QTestMessage const &/*message*/)
{
    QTestMessage reply;

    // FIXME: This code is copied from the systemtime application and therefore very fragile!!
    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );
    if (config.value("AMPM").toBool())
        reply["timeFormat"] = "12-hour";
    else
        reply["timeFormat"] = "24-hour";

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::setDateFormat(QTestMessage const &message)
{
    QTestMessage reply;
    QString new_format = message["mode"].toString();
    if (new_format == "English (U.S.)")
        new_format = "M/D/Y";
    new_format.replace("D", "%D"); //convert to QTimeString format
    new_format.replace("M", "%M");
    new_format.replace("Y", "%Y");

    QString cur_format = "";

    // FIXME: This code is copied from the systemtime application and therefore very fragile!!
    {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "Date" );
        cur_format = config.value("DateFormat").toString();
        if (cur_format == new_format)
            return RET(reply, "OK");

        config.setValue("DateFormat", new_format);
    }

    // Notify everyone what date format to use
    QtopiaIpcEnvelope setDateFormat( "QPE/System", "setDateFormat()" );

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::dateFormat(QTestMessage const &/*message*/)
{
    QTestMessage reply;
    QString cur_format = "";
    // FIXME: This code is copied from the systemtime application and therefore very fragile!!
    {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "Date" );
        cur_format = config.value("DateFormat").toString();
    }
    cur_format.replace("%D", "D"); //convert to QTimeString format
    cur_format.replace("%M", "M");
    cur_format.replace("%Y", "Y");
    reply["dateFormat"] = cur_format;
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::setTimeZone(QTestMessage const &message)
{
    QTestMessage reply;
    QString new_zone = message["timeZone"].toString();
    QString cur_zone = ::getenv("TZ");
    if (new_zone == cur_zone)
        return RET(reply,"OK");

    // FIXME: This code is copied from the systemtime application and therefore very fragile!!
    {
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);

        // Need to process the QCOP event generated
        QtUiTest::wait(0);

        {
            QSettings lconfig("Trolltech","locale");
            lconfig.beginGroup( "Location" );
            lconfig.setValue( "Timezone", new_zone );
        }
        setenv( "TZ", new_zone.toLocal8Bit().constData(), 1 );

        // Restore screensaver
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
    }

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::timeZone(QTestMessage const &/*message*/)
{
    QTestMessage reply;
    reply["timeZone"] = ::getenv("TZ");
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::systemTime(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    reply["systemTime"] = QDateTime::currentDateTime();
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::setSystemTime(QTestMessage const &message)
{
    QTestMessage reply;

    {
        QSettings lconfig("Trolltech","locale");
        lconfig.beginGroup( "Location" );
        bool auto_time = lconfig.value( "TimeAuto" ).toBool();
        if (auto_time) return RET(reply, "ERROR: System time is adjusted automatically by the device and cannot be altered manually.");
    }

    if (!message["timeZone"].toString().isEmpty()) {
        QTestMessage ret = setTimeZone(message);
        if (!ret.statusOK()) return ret;
    }

    QDateTime dt = message["dateTime"].value<QDateTime>();
    if (!dt.isValid()) return RET(reply, "ERROR: Invalid date/time specified");

    struct timeval myTv;
    myTv.tv_sec = dt.toTime_t();
    myTv.tv_usec = 0;

    if ( myTv.tv_sec != -1 )
        ::settimeofday( &myTv, 0 );

    // FIXME: this code is copied from Qtopia sources and therefore fragile
    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );
    int ampm = config.value("AMPM").toBool() ? 1 : 0;
    QtopiaIpcEnvelope setClock( "QPE/System", "clockChange(bool)" );
    setClock << (int)ampm;

    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::getClipboardText(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;

#ifndef QT_NO_CLIPBOARD
    QClipboard *cb = QApplication::clipboard();
    if (cb) {
        reply["getClipboardText"] = cb->text();
        return RET(reply, "OK");
    }
#endif

    return RET(reply, "ERROR: couldn't get clipboard");
}

QTestMessage QtopiaServerTestSlavePrivate::setClipboardText(QTestMessage const &message)
{
    QTestMessage reply;
    QString value = message["text"].toString();

#ifndef QT_NO_CLIPBOARD
    QClipboard *cb = QApplication::clipboard();
    if (cb) {
	cb->setText(value);
        return RET(reply, "OK");
    }
#endif

    return RET(reply, "ERROR: couldn't get clipboard");
}

QTestMessage QtopiaServerTestSlavePrivate::buildOption(QTestMessage const &message)
{
    QTestMessage reply;
    QString option = message["option"].toString();
    bool on = false;

#ifdef QTOPIA_HOMEUI
    if (option == "QTOPIA_HOMEUI") on = true;
#endif

#ifdef QTOPIA_VOIP
    if (option == "QTOPIA_VOIP")   on = true;
#endif

    reply["on"] = on;
    return RET(reply, "OK");
}

QTestMessage QtopiaServerTestSlavePrivate::getVersion(QTestMessage const &message)
{
    QTestMessage reply;
    QString versionType = message["type"].toString();
    QString versionData;

    if (versionType == "QtVersion") {
        versionData = QT_VERSION_STR;
    } else if (versionType == "QtExtendedVersion") {
        versionData = Qtopia::version();
    } else if (versionType == "KernelVersion") {
        QFile file("/proc/version");
        if(file.open(QFile::ReadOnly))
        {
            QTextStream t( &file );
            QString v;
            t >> v; t >> v; t >> v;
            versionData = v.left( 20 );
            file.close();
        }
    }

    reply["getVersion"] = versionData;
    return RET(reply, "OK");
}

/*
    Start reading log messages and posting them to the system test.
*/
QTestMessage QtopiaServerTestSlavePrivate::startLogRead(QTestMessage const &message)
{
    QTestMessage reply;
    QStringList commands = message["commands"].toStringList();

    if (!logReader.isActive()) {
        if (!QObject::connect(&logReader, SIGNAL(error(QString)),   this,   SLOT(logError(QString))))
            Q_ASSERT(0);
        if (!QObject::connect(&logReader, SIGNAL(log(QStringList)), this,   SLOT(log(QStringList))))
            Q_ASSERT(0);
        if (!QObject::connect(&logReader, SIGNAL(finished()),       &logReader, SLOT(deleteLater())))
            Q_ASSERT(0);
        logReader.start(commands);
    }

    return RET(reply, "OK");
}

uint QtopiaServerTestSlave::postMessage( QTestMessage const &message )
{
    return QTestSlave::postMessage(message);
}

/*!
    Process messages received from a remote test script
*/
QTestMessage QtopiaServerTestSlave::constructReplyToMessage( QTestMessage const &_msg )
{
    /* Wait for any applications currently starting. */
    {
        static QValueSpaceItem busyCount("/System/Applications/Info/BusyCount");

        int i = 0;
        for (i = 0;
                i < 5000 && busyCount.value().toBool();
                i += 100, QtUiTest::wait(100))
        {}

        /* Wait for wait indicator to fade out. */
        if (i)
            QtUiTest::wait(550);
    }

    QTestMessage reply;
    QTestMessage msg(_msg);

    QString qp = msg["queryPath"].toString();
    if (qp.startsWith(">@")) {
    qLog(QtUitest) <<  QString("queryPath = %1").arg(qp).toLatin1() ;
        QTestMessage reply;
        if (qp == SOFT_MENU_ALIAS) {
            msg["queryApp"] = "qpe";
        }
    }


    bool autoApp = false;
    QString originalApp;
    if (msg["queryApp"].toString().isEmpty()) {
        originalApp = d->current_application;
        msg["queryApp"] = d->current_application;
        autoApp = true;
    }

    bool shouldRetry = false;
    const int maxtries = 2;
    int tries = 0;
    do {
        shouldRetry = false;
        reply["status"] = "OK";
        qLog(QtUitest) <<  QString("QtopiaServerTestSlave::constructReplyToMessage(%1) try %2 for app %3").arg(msg.event()).arg(tries).arg(msg["queryApp"].toString()).toLatin1();

        /* Try to handle event ourselves... */
        if (reply.statusOK() && !QMetaObject::invokeMethod(d,
                    msg.event().toLatin1().constData(),
                    Qt::DirectConnection,
                    Q_RETURN_ARG(QTestMessage, reply),
                    Q_ARG(QTestMessage, msg))) {
            /* We couldn't handle it.  Either send it to an app, or fall through to superclass */
            QString app = msg["queryApp"].toString();
            if ( !app.isEmpty() && app != qApp->applicationName() ) {
                d->querySlave(msg,&reply);
                reply["from"] = qApp->applicationName() + "/" + app;
            } else {
                reply = QTestSlave::constructReplyToMessage(msg);
            }
        } else {
            reply["from"] = qApp->applicationName();
        }

        if (!reply.statusOK()) {
            qLog(QtUitest) <<  QString("QtopiaServerTestSlave::constructReplyToMessage(%1) error %2 current_app %3 autoApp %4 originalApp %5").arg(msg.event()).arg(reply["status"].toString()).arg(d->current_application).arg(autoApp).arg(originalApp).toLatin1();
        }

        /* If we failed to service the query, and we autodetected the application,
         * and the current application changed while processing the message, retry
         * with the new application. */
        if ( !reply.statusOK()
            && autoApp
            && d->current_application != originalApp
            ) {
            originalApp = d->current_application;
            msg["queryApp"] = originalApp;
            shouldRetry = true;
        }

    } while (shouldRetry && ++tries < maxtries);

    return reply;
}

void QtopiaServerTestSlave::waitTrigger()
{
    if (waitVS) {
        QVariant currValue(waitVS->value());
        if ( waitValue.isNull() || currValue == waitValue ) {
            delete waitVS;
            waitVS = 0;
            QTestMessage msg("valueSpaceResponse");
            msg["value"] = currValue;
            postMessage( msg );
        }
    }
}

QString QtopiaServerTestSlave::realAppName( const QString &appName )
{
    if (d->applications.isEmpty()) {
        QContentSet set(QContent::Application);
        QContentList list(set.items());
        foreach(QContent c, list) {
            d->applications[Qtopia::dehyphenate(c.name())] = c.executableName();
        }
    }

    // Applications not in the map have the same binary name, but in lower-case.
    return d->applications.contains(appName) ? d->applications[appName].toString() : appName.toLower();
}

QtopiaServerQueryMaster* QtopiaServerTestSlave::findSlaveForApp(QString const &capp) {
    QString app(capp);
    if (app.startsWith("qpe/"))
        app = app.mid(4);
    else if (app.startsWith("/"))
        app = app.mid(1);
    if (app.endsWith( ":" ))
        app = app.left( app.length() - 1 );

    QtopiaServerQueryMaster *ret(d->appServer->findApp( app, APP_LAUNCH_TIMEOUT ));

    return ret;
}

void QtopiaServerTestSlave::onWindowEvent( QWSWindow *window, QWSServer::WindowEvent event )
{
    if (event != QWSServer::Active) return;
    if (!window || !window->client()) return;

    QTestMessage message("appGainedFocus");
    QString old_app = d->current_application;
    if (window->client()->identity().isEmpty()) {
        d->current_application = qApp->applicationName();
    } else {
        d->current_application = window->client()->identity();
    }

    message["appName"] = d->current_application;

    postMessage(message);
}

void QtopiaServerTestSlavePrivate::currentTitleChanged()
{
    QString newTitle = Qtopia::dehyphenate( titleVs->value().toString() );
    if (newTitle != lastTitle) {
        lastTitle = newTitle;
        p->recordEvent( RecordEvent::TitleChanged, QString(), QString(), Qtopia::dehyphenate(newTitle) );
    }
}

void QtopiaServerTestSlave::onNewMessage( const QTestMessage &msg )
{
    postMessage( msg );
}

void QtopiaServerTestSlavePrivate::sampledMemory(int sample)
{
    QTestMessage message("sampledMemory");

    QVariantMap values;
    foreach (QString key, memorySampler.keys()) {
        values[key] = memorySampler.value(key, sample);
    }
    message["values"] = values;

    p->postMessage(message);
}

void QtopiaServerTestSlavePrivate::logError(QString const& error)
{
    QTestMessage message("logError");
    message["error"] = error;
    p->postMessage(message);
}

void QtopiaServerTestSlavePrivate::log(QStringList const& lines)
{
    QTestMessage message("log");
    message["lines"] = lines;
    p->postMessage(message);
}

QTestMessage QtopiaServerTestSlavePrivate::sampleMemory(QTestMessage const& message)
{
    QTestMessage reply;

    bool on = message["on"].toBool();
    int interval = -1;
    if (message["interval"].isValid()) {
        interval = message["interval"].toInt();
    }

    if (!on && memorySamplerTimer.isActive()) {
        memorySamplerTimer.stop();
        memorySampler.clear();
    }

    /* -1 interval: just sample memory this once */
    if (interval == -1) {
        bool were_blocked = memorySampler.blockSignals(true);
        int id = memorySampler.sample();
        memorySampler.blockSignals(were_blocked);
        QVariantMap values;
        foreach (QString key, memorySampler.keys()) {
            values[key] = memorySampler.value(key, id);
        }
        reply["values"] = values;
    }

    /* non-negative interval and 'on': sample at set intervals */
    if (on && interval != -1) {
        memorySamplerTimer.start(interval);
    }

    return RET(reply, "OK");
}

// ***********************************************************************
// ***********************************************************************

QtUiTestProtocolServer::QtUiTestProtocolServer(QtopiaServerTestSlavePrivate *parent)
    : QTestServerSocket(0)
    , connectionList()
{
    QFile f( Qtopia::homePath() + "/.qtestport" );
    if ( f.open( QIODevice::WriteOnly ) ) {
        QTextStream s( &f );
        s << port();
    }
    setParent(parent);
    m_parent = parent;
}

QtUiTestProtocolServer::~QtUiTestProtocolServer()
{
    QFile::remove( Qtopia::homePath() + "/.qtestport" );
    disconnectAllApplications();
}

void QtUiTestProtocolServer::disconnectAllApplications()
{
    while (!connectionList.isEmpty())
        delete connectionList.takeAt(0);
}

void QtUiTestProtocolServer::onNewConnection( int socket )
{
    QtopiaServerQueryMaster *queryMaster = new QtopiaServerQueryMaster(socket,m_parent);
    connectionList.append( queryMaster );

    if (m_parent->p->recordingEvents()) {
        QTestMessage msg("startEventRecording");
        queryMaster->postMessage(msg);
    }

    connect( queryMaster, SIGNAL(connectionClosed(QTestProtocol*)), this, SLOT(onConnectionClosed(QTestProtocol*)) );
    connect( queryMaster, SIGNAL(newMessage(QTestMessage)), this, SLOT(onNewMessage(QTestMessage)) );
}

void QtUiTestProtocolServer::onNewMessage( const QTestMessage &msg )
{
    emit newMessage( msg );
}

void QtUiTestProtocolServer::onConnectionClosed( QTestProtocol *connection )
{

    delete connectionList.takeAt( connectionList.indexOf( (QtopiaServerQueryMaster*)connection ) );
}

QtopiaServerQueryMaster * QtUiTestProtocolServer::findApp( QString appName, uint timeout )
{
    QElapsedTimer t;
    do {
        // It is necessary to take a copy of connectionList because it might
        // be changed during queryName.
        QList< QPointer<QtopiaServerQueryMaster> > connections(connectionList);
        foreach (QPointer<QtopiaServerQueryMaster> app, connections) {
            if (app && appName == app->appName)
                return app;
        }
        // If not found, query all the quicklauncher instances for their name
        foreach (QPointer<QtopiaServerQueryMaster> app, connections) {
            if (app && app->appName.startsWith("quicklauncher")) {
                // ask again for the name in case it has changed
                app->queryName();
            }
            if (app && appName == app->appName)
                return app;
        }
        if (t.elapsed() < int(timeout)) {
            QtUiTest::wait(10);
        }
    } while (t.elapsed() < int(timeout));

    return 0;
}

// ***********************************************************************
// ***********************************************************************

QtopiaServerQueryMaster::QtopiaServerQueryMaster( int socket, QtopiaServerTestSlavePrivate *tp )
    : QTestProtocol()
    , appName()
{
    setSocket( socket );
    m_testslave_p = tp;
}

QtopiaServerQueryMaster::~QtopiaServerQueryMaster()
{
}

void QtopiaServerQueryMaster::processMessage( QTestMessage *msg )
{
    if (!msg) return;

    if (msg->event() == "APP_NAME") {
        appName = (*msg)["appName"].toString();
    } else if (msg->event() == "recordEvent") {
        m_testslave_p->p->recordEvent(static_cast<RecordEvent::Type>((*msg)["type"].toInt()), (*msg)["widget"].toString(), (*msg)["focusWidget"].toString(), (*msg)["data"]);
    } else {
        if (appName.endsWith("_callback_slave")) {
            if (msg->event().toLatin1() == "keyClick") {
                if (m_testslave_p) m_testslave_p->keyClick(QTestMessage(*msg));
                if ((*msg)["reply"].toBool()) {
                    QTestMessage reply;
                    replyMessage(msg,RET(reply,"OK"));
                }
            } else if (msg->event().toLatin1() == "setInputMethodHint") {
                QTestMessage reply = m_testslave_p->setInputMethodHint(QTestMessage(*msg));
                if ((*msg)["reply"].toBool()) {
                    replyMessage(msg,reply);
                }
            } else {
                QTestMessage reply;
                replyMessage(msg,RET(reply,"ERROR: Unhandled callback_slave event"));
            }
        } else {
            emit newMessage( QTestMessage(*msg) );
        }
    }
}

void QtopiaServerQueryMaster::queryName()
{
    QTestMessage reply;
    if (sendMessage( QTestMessage("appName"), reply, APP_COMMS_TIMEOUT ) && reply["status"] == "OK") {
        appName = reply["appName"].toString();
    } else {
        qWarning(QString("Error in appName query: '%1'").arg(reply["status"].toString()).toLatin1());
    }
}

// ***********************************************************************

class QTUITEST_EXPORT ServerTestSlavePlugin : public QObject, public TestSlaveInterface
{
    Q_OBJECT
    Q_INTERFACES(TestSlaveInterface)

    public:
        ServerTestSlavePlugin(QObject *parent = 0)
            : QObject(parent) {}

        virtual void postMessage(QString const &name, QVariantMap const &data)
        { realSlave.postMessage( QTestMessage( name, data ) ); }

        virtual bool isConnected() const
        { return realSlave.isConnected(); }

        virtual void showMessageBox(QWidget* widget, QString const& title, QString const& text)
        { realSlave.showMessageBox(widget,title,text); }

        virtual void showDialog(QWidget* widget, QString const& title)
        { realSlave.showDialog(widget,title); }

    private:
        mutable QtopiaServerTestSlave realSlave;
};
Q_EXPORT_PLUGIN2(qpeslave, ServerTestSlavePlugin)

#include "qtopiaservertestslave.moc"

