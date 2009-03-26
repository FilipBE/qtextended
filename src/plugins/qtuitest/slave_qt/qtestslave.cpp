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

#include <qtestslave.h>
#include <qtestslaveglobal.h>
#include <qtestwidgets.h>
#include <qtuitestnamespace.h>
#include <qtuitestrecorder.h>

#include <QWidget>
#include <QPixmap>
#include <QApplication>
#include <QTimer>
#include <QDir>
#include <QVariant>
#include <QMetaProperty>
#include <QMetaObject>
#include <QMetaType>
#include <QPoint>
#include <QProcess>
#include <QLabel>
#include <QGroupBox>
#include <QSettings>
#include <QDesktopWidget>

#include <QMenu>
#include <QVBoxLayout>
#include <QAbstractButton>

#ifdef QTOPIA_TARGET
# include <qtopiabase/qtopialog.h>
#else
# define qLog(A) if(1); else qDebug() << #A
#endif

#ifdef Q_OS_UNIX
# include <sys/time.h>
#endif

#include <qalternatestack_p.h>

using namespace QtUiTest;

/* Handler for test messages */
class QTestSlavePrivate : public QObject
{
Q_OBJECT
public:
    QTestSlavePrivate(QTestSlave *parent)
        :   eventRecordingEnabled(false),
            p(parent),
            recorder(this)
    {}

    bool event(QEvent *e);

    bool waitForIdle(int timeout = 10000);

    bool eventRecordingEnabled;
    QTestSlave *p;

    bool grabPixmap(QWidget*,QPixmap&,QString&);
    QString recursiveDelete(QString const&) const;
    void startWaitForIdle(int);

    static void processMessage(QAlternateStack*,QVariant const&);

    QPoint mousePointForMessage(QTestMessage const&,QTestMessage&,bool&);

    void ensureFocus();
    QTime lastFocusWarn;

    QtUiTestRecorder recorder;

public slots:
    void record_entered(QObject*, QVariant const&);
    void record_selected(QObject*, QString const&);
    void record_activated(QObject*);
    void record_stateChanged(QObject*, int);
    void record_gotFocus(QObject*);

    QTestMessage widget             (QTestMessage const&);

    QTestMessage currentTitle       (QTestMessage const&);
    QTestMessage appName            (QTestMessage const&);
    QTestMessage isVisible          (QTestMessage const&);

    QTestMessage startEventRecording(QTestMessage const&);
    QTestMessage stopEventRecording (QTestMessage const&);

    QTestMessage locateWidgetByText (QTestMessage const&);
    QTestMessage focusWidget        (QTestMessage const&);
    QTestMessage hasFocus           (QTestMessage const&);

    QTestMessage grabPixmap         (QTestMessage const&);
    QTestMessage getSelectedText    (QTestMessage const&);
    QTestMessage getText            (QTestMessage const&);
    QTestMessage getCenter          (QTestMessage const&);
    QTestMessage getList            (QTestMessage const&);
    QTestMessage getLabels          (QTestMessage const&);

    QTestMessage isChecked          (QTestMessage const&);
    QTestMessage setChecked         (QTestMessage const&);

    QTestMessage enterText          (QTestMessage const&);
    QTestMessage selectItem         (QTestMessage const&);
    QTestMessage activate           (QTestMessage const&);

    QTestMessage activeWidgetInfo   (QTestMessage const&);

    QTestMessage inherits           (QTestMessage const&);
    QTestMessage invokeMethod       (QTestMessage const&);
    QTestMessage setProperty        (QTestMessage const&);
    QTestMessage getProperty        (QTestMessage const&);

    QTestMessage setSetting         (QTestMessage const&);
    QTestMessage getSetting         (QTestMessage const&);

    QTestMessage putFile            (QTestMessage const&);
    QTestMessage getFile            (QTestMessage const&);
    QTestMessage deletePath         (QTestMessage const&);
    QTestMessage getDirectoryEntries(QTestMessage const&);

    QTestMessage getImageSize       (QTestMessage const&);
    QTestMessage getGeometry        (QTestMessage const&);
    QTestMessage setSystemTime      (QTestMessage const&);
    QTestMessage systemTime         (QTestMessage const&);

    QTestMessage waitForIdle        (QTestMessage const&);
    QTestMessage waitForText        (QTestMessage const&);

    QTestMessage keyPress           (QTestMessage const&);
    QTestMessage keyRelease         (QTestMessage const&);
    QTestMessage keyClick           (QTestMessage const&);

    QTestMessage mousePress         (QTestMessage const&);
    QTestMessage mouseRelease       (QTestMessage const&);
    QTestMessage mouseClick         (QTestMessage const&);

    QTestMessage scrollByMouse      (QTestMessage const&);

    QTestMessage getenv(QTestMessage const&);

    void sendBecameIdleMessage();

signals:
    void applicationBecameIdle();
};

class IdleEvent : public QEvent
{
public:
    enum EventType { Type = QEvent::User + 10 };
    IdleEvent(QTime itime, int itimeout)
        : QEvent( (QEvent::Type)Type ), time(itime), timeout(itimeout) {}

    QTime time;
    int timeout;
};

#include "qtestslave.moc"

#define RET(message, str) (\
    message["status"] = str,\
    message["location"] = QString("%1:%2%3").arg(__FILE__).arg(__LINE__).arg(!message["location"].toString().isEmpty() ? "\n" + message["location"].toString() : ""),\
    message)

QTestSlave::QTestSlave()
    : QTestProtocol()
    , d(new QTestSlavePrivate(this))
{
    // Stop text cursors from flashing so that we can take reproducible screen shots
    QApplication::setCursorFlashTime(0);
    QtUiTest::testInputOption(QtUiTest::NoOptions);
}

QTestSlave::~QTestSlave()
{
    delete d;
    disconnect();
}

void QTestSlave::onConnected()
{
    QTestProtocol::onConnected();

    QTestMessage msg("APP_NAME");
    msg["appName"] = qApp->applicationName();
    postMessage( msg );
}

void QTestSlave::setRecordingEvents(bool on)
{
    d->eventRecordingEnabled = on;
    if (on) {
        QObject::connect(&d->recorder, SIGNAL(entered(QObject*,QVariant)),
                d, SLOT(record_entered(QObject*,QVariant)));
        QObject::connect(&d->recorder, SIGNAL(selected(QObject*,QString)),
                d, SLOT(record_selected(QObject*,QString)));
        QObject::connect(&d->recorder, SIGNAL(gotFocus(QObject*)),
                d, SLOT(record_gotFocus(QObject*)));
        QObject::connect(&d->recorder, SIGNAL(activated(QObject*)),
                d, SLOT(record_activated(QObject*)));
        QObject::connect(&d->recorder, SIGNAL(stateChanged(QObject*,int)),
                d, SLOT(record_stateChanged(QObject*,int)));
    } else {
        QObject::disconnect(&d->recorder, 0, d, 0);
    }
}

bool QTestSlave::recordingEvents() const
{ return d->eventRecordingEnabled; }

void QTestSlavePrivate::ensureFocus()
{
    // Try to ensure something in this application has focus.
    for (int i = 0; i < 1000 && !QApplication::focusWidget(); i += 100, QtUiTest::wait(100)) {
        QWidget* top = 0;
        foreach (QWidget* w, qApp->topLevelWidgets()) {
            if (w->isVisible()) {
                top = w;
                break;
            }
        }
        if (!top) break;
        QCursor::setPos(top->mapToGlobal(top->geometry().center()));
    }

    if (!QApplication::focusWidget() && (lastFocusWarn.elapsed() > 30000 || lastFocusWarn.elapsed() < 0)) {
        lastFocusWarn.start();
        qWarning() <<   "QtUitest: could not give focus to tested application, "
                        "keyboard input might be lost!";
    }
}

QTestMessage QTestSlavePrivate::currentTitle(QTestMessage const&)
{
    QTestMessage reply;

    QObject* focus = QtUiTest::findWidget(QtUiTest::Focus);
    QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(focus);
    while (w && !(w->windowFlags() & Qt::Window) && w->parent()) {
        w = qtuitest_cast<QtUiTest::Widget*>(w->parent());
    }
    if (w) {
        reply["currentTitle"] = w->windowTitle();
        reply["status"]       = "OK";
    } else if (!focus) {
        reply["status"]       = "Could not get current window title: could not find currently focused widget!";
    } else {
        reply["status"]       = "Could not get current window title: could not find a top-level QtUiTest::Widget!";
    }
    return reply;
}

QTestMessage QTestSlavePrivate::inherits(QTestMessage const &message)
{
    QTestMessage reply;
    QString error;
    QTestWidget *tw;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error, 0)) {
        return RET(reply,error);
    }

    if (tw && tw->instance()) {
        reply["inherits"] = tw->instance()->inherits(message["className"].toString().toLatin1().constData());
        return RET(reply, "OK");
    }
    reply["inherits"] = false;
    return RET(reply, "ERROR: Could not find a Test Widget for '" + message["queryPath"].toString() + "'" );
}

QTestMessage QTestSlavePrivate::grabPixmap(QTestMessage const &message)
{
    ensureFocus();

    QTestMessage reply;
    QString error;

    // decompose the data
    QString tmp = message["MASK"].toString();
    QStringList maskWidgets = tmp.split( "," );

    // see if there's any widget (such as those showing the time) that we need to mask
    QList<QWidget*> masked_widgets;
    for (int i=0; i<maskWidgets.count(); i++) {
        if (!maskWidgets[i].isEmpty()) {
            /* FIXME */
            return RET(reply, "Masking is broken");
        }
    }

    // grab
    QTestWidget *tw = 0;
    if (!message["queryPath"].toString().isEmpty()) {
        if (!QActiveTestWidget::instance()->findWidget(message["queryPath"].toString(),tw,error))
            return RET(reply,error);
    }

    QPixmap pix;
    if (grabPixmap( tw ? qobject_cast<QWidget*>(tw->instance()) : 0, pix, error)) {
        reply["grabPixmap"] = pix.toImage();
        qLog(QtUitest) << "Took snapshot of" << (tw ? tw->signature() : "screen") << "isNull" << reply["grabPixmap"].value<QImage>().isNull();
        (void)RET(reply, "OK");
    } else {
        qLog(QtUitest) << "Failed to take snapshot of" << (tw ? tw->signature() : "screen");
        (void)RET(reply, error);
    }

    // unset the mask for masked widgets
    // TODO: it would be better if we first checked if widgets already HAD a mask and set that back.
    while (!masked_widgets.isEmpty())
        masked_widgets.takeFirst()->clearMask();

    return reply;
}

QTestMessage QTestSlavePrivate::activeWidgetInfo(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    reply["activeWidgetInfo"] = QActiveTestWidget::instance()->toString();
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::locateWidgetByText(QTestMessage const &message)
{
    QTestMessage reply;
    reply["locateWidgetByText"] = QTestWidgets::locateWidgetByText( message["text"].toString(), message["typelist"].toStringList() );
    if (!reply["locateWidgetByText"].toString().isEmpty())
        return RET(reply, "OK");
    return RET(reply, "ERROR: widget not found");
}

QTestMessage QTestSlavePrivate::getSelectedText(QTestMessage const &message)
{
    QTestMessage reply;
    QString error;
    QTestWidget *tw;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);

    bool ok;
    QtUiTest::setErrorString(QString());
    reply["getSelectedText"] = tw->getSelectedText(&ok);
    if (!QtUiTest::errorString().isEmpty())
        return RET(reply, QtUiTest::errorString());
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getText(QTestMessage const &message)
{
    QTestMessage reply;
    QString error;
    QTestWidget *tw;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);

    bool ok;
    QtUiTest::setErrorString(QString());
    reply["getText"] = tw->getText(&ok);
    if (!QtUiTest::errorString().isEmpty())
        return RET(reply, QtUiTest::errorString());
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getList(QTestMessage const &message)
{

    QTestMessage reply;
    QString error;
    QTestWidget *tw;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);

    bool ok;
    QtUiTest::setErrorString(QString());
    reply["getList"] = tw->getList(&ok);

    if (!QtUiTest::errorString().isEmpty())
        return RET(reply, QtUiTest::errorString());
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getLabels(QTestMessage const &/*message*/)
{
    QTestMessage reply;
    QString error;
    if (!QActiveTestWidget::instance()->rescan(error)) {
        return RET(reply, error);
    }
    reply["getLabels"] = QActiveTestWidget::instance()->allLabels();
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::isVisible(QTestMessage const &message)
{
    QTestWidget *tw;
    QTestMessage reply;
    QString error;
    reply["isVisible"] = QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error );
    if (reply["isVisible"].toBool() && tw) {
        QtUiTest::Widget *newW;
        if ((newW = qtuitest_cast<QtUiTest::Widget*>(tw->instance()))) {
            reply["isVisible"] = !(newW->visibleRegion() | newW->childrenVisibleRegion()).isEmpty();
        } else {
            reply["isVisible"] = !tw->totalVisibleRegion().isEmpty();
        }
    }
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::widget(QTestMessage const &message)
{
    QTestWidget *tw;
    QTestMessage reply;
    QString error;
    int offset = 0;
    if (!message["offset"].toString().isEmpty()) offset = message["offset"].toInt();
    if (QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error, offset )) {
        reply["widget"] = tw->signature();
        return RET(reply,"OK");
    }
    return RET(reply, error);
}

QTestMessage QTestSlavePrivate::isChecked(QTestMessage const &message)
{
    QTestWidget *tw;
    QTestMessage reply;
    QString error;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error ))
        return RET(reply,error);

    bool ok;
    QString text = tw->isChecked(&ok);
    if (ok) reply["isChecked"] = (text == "YES");
    return RET(reply, ok ? "OK" : text);
}

QTestMessage QTestSlavePrivate::setChecked(QTestMessage const &message)
{
    QTestWidget *tw;
    QTestMessage reply;
    QString error;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error ))
        return RET(reply,error);

    QtUiTest::CheckWidget* cw
        = qtuitest_cast<QtUiTest::CheckWidget*>(tw->instance());
    if (!cw) return RET(reply, "Not a check widget: " + tw->signature());

    Qt::CheckState state = Qt::Unchecked;
    if (message["doCheck"].toBool())
        state = Qt::Checked;
    if (cw->checkState() == state)
        return RET(reply, "OK");
    return RET(reply, cw->setCheckState(state) ? "OK" : "Failed to change check state of " + tw->signature());
}

QTestMessage QTestSlavePrivate::getCenter(QTestMessage const &message)
{
    QTestMessage reply;
    QPoint pos;
    QTestWidget *tw;
    QWidget *w;
    QString error;
    if (!QActiveTestWidget::instance()->findWidget(message["queryPath"].toString(), tw, error)) return RET(reply,error);
    w = qobject_cast<QWidget*>(tw->instance());
    QRegion visibleRegion = tw->totalVisibleRegion();
    /* If we have just gotten focus, the visible region might be empty.
        * Wait for up to 100ms for screen expose events to propagate correctly. */
    for (int i = 0;
            i < 100 && visibleRegion.isEmpty();
            QtUiTest::wait(20), i += 20, visibleRegion = tw->totalVisibleRegion()) {}

    if (message["item"].isValid()) {
        bool ok;
        pos = (tw->getCenter(message["item"].toString(), &ok));
        if (!ok) return RET(reply, "ERROR: couldn't get center of item " + message["item"].toString() + " in widget " + tw->signature());
    }

    if (pos.isNull()) {
        int w_offset = w->width()/2;
        int h_offset = w->height()/2;

        // xxx FIXME
        // xxx Make sure we return the checkable portion of checkboxes and
        // xxx groupboxes.
        // xxx Needs to be rewritten properly to take styles into account.
        QAbstractButton *b = qobject_cast<QAbstractButton*>(w);
        if (b && b->isCheckable()) {
            w_offset = 15;
        }
        QGroupBox *gb = qobject_cast<QGroupBox*>(w);
        if (gb && gb->isCheckable()) {
            w_offset = 15;
            h_offset = 15;
        }

        pos = w->mapToGlobal( QPoint(w_offset, h_offset) );
    }
    reply["getCenter"] = QVariant::fromValue(pos);

    if (w) {
        reply["isVisible"] = QVariant::fromValue(visibleRegion.contains(w->mapFromGlobal(pos)));
        qLog(QtUitest) << "got center" << pos << qPrintable((message["item"].isValid() ? ("of item '" + message["item"].toString()) + "'": "")) << ", is visible" << reply["isVisible"].toBool();
        QPoint local = w->mapFromGlobal(pos);
        qLog(QtUitest) << "widget" << w << "point (local)" << local << "visible region (local)" << visibleRegion;

        /* See if we can figure out where we need to click in order to scroll
            * towards it. */
        if (!reply["isVisible"].toBool()) {
            bool ok;
            int wait = 0;
            QList<QPoint> points = tw->navigateByMouse(message["item"].toString(), pos, &wait, &ok);
            if (ok) {
                QVariantList list;
                foreach(QPoint p, points) {
                    qLog(QtUitest) << "point" << p;
                    list << QVariant::fromValue(p);
                }
                reply["navigateByMouse"] = QVariant::fromValue(list);
                reply["navigateByMouseWait"] = wait;
            }
        }
    }
    return RET(reply, w ? "OK" : "ERROR_NO_SUCH_WIDGET");
}

QTestMessage QTestSlavePrivate::focusWidget(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    QElapsedTimer t;
    while (t.elapsed() < VISIBLE_RESPONSE_TIME) {
        QString error;
        QActiveTestWidget::instance()->rescan(error);
        QWidget *w = qApp->focusWidget();
        if (w) {
            reply["focusWidget"] = QTestWidgets::signature( w );
            return RET(reply, "OK");
        }
        QtUiTest::wait(3);
    }
    return RET(reply, QString("ERROR: No focus widget found in application '%1' in %2 ms.").arg(qApp->applicationName()).arg(t.elapsed()));
}

QTestMessage QTestSlavePrivate::hasFocus( QTestMessage const &message )
{
    QTestMessage reply;
    QTestWidget *tw;
    QString error;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);

    QWidget *w = qobject_cast<QWidget*>(tw->instance());
    if (w) {
        if (w->inherits("QMenu")) {
            reply["hasFocus"] = !w->isHidden();
        } else {
//            reply["hasFocus"] = w->getProperty("focus");
            reply["hasFocus"] = w->hasFocus();
        }
        return RET(reply,"OK");
    }
    return RET(reply,"ERROR: Invalid pointer to widget. This should not happen.");
}

QTestMessage QTestSlavePrivate::appName(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    reply["appName"] = qApp->applicationName();
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::startEventRecording(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    if (qApp != 0) {
        p->setRecordingEvents(true);
        qApp->installEventFilter( this );
        return RET(reply, "OK");
    }
    return RET(reply, "ERROR: No application available to record events. This is unusual.");
}

QTestMessage QTestSlavePrivate::stopEventRecording(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    if (qApp != 0) {
        p->setRecordingEvents(false);
        qApp->removeEventFilter( this );
        return RET(reply, "OK");
    }
    return RET(reply, "ERROR: No application available to stop event recording. This is unusual.");
}

void QTestSlave::recordEvent(RecordEvent::Type event, QString const& widget, QString const& focusWidget, QVariant const& data)
{
    if (qApp->type() == QApplication::GuiServer)
        return;

    QTestMessage msg("recordEvent");
    msg["type"] = QVariant::fromValue(static_cast<int>(event));
    msg["widget"] = widget;
    msg["focusWidget"] = focusWidget;
    msg["data"] = data;
    postMessage(msg);
}

QTestMessage QTestSlavePrivate::invokeMethod(QTestMessage const &message)
{
    QTestWidget *tw;
    QString error;
    QTestMessage reply;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);
    QObject *o = tw->instance();

    QString method = message["method"].toString();
    bool returns   = message["returns"].toBool();
    Qt::ConnectionType connType = (Qt::ConnectionType)message["conntype"].toInt();

    method = QMetaObject::normalizedSignature(qPrintable(method));

    QVariantList argList = message["args"].toList();

    QMetaObject const *mo = o->metaObject();

    int m = mo->indexOfMethod(QMetaObject::normalizedSignature(qPrintable(method)));
    if (-1 == m) {
        return RET(reply, "ERROR_NO_METHOD");
    }
    QMetaMethod mm = mo->method(m);
    if (mm.methodType() == QMetaMethod::Method) {
        return RET(reply, "ERROR_METHOD_NOT_INVOKABLE");
    }
    QList<QByteArray> paramTypes = mm.parameterTypes();
    if (paramTypes.count() != argList.count()) {
        (void)RET(reply, "ERROR_WRONG_ARG_COUNT");
        reply["warning"] = QString("actual args %1, expected args %2").arg(paramTypes.count()).arg(argList.count());
        return reply;
    }
    QString retType = mm.typeName();
    if (returns && retType.isEmpty()) {
        return RET(reply, "ERROR_NO_RETURN");
    }

    QGenericArgument arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9;
#define LARG(i) do {\
    if (argList.count() <= i) break;\
    arg##i = QGenericArgument(paramTypes[i], argList[i].constData());\
} while(0);
    LARG(0);LARG(1);LARG(2);LARG(3);LARG(4);LARG(5);LARG(6);
    LARG(7);LARG(8);LARG(9);
#undef LARG

    QString methodName = method.left(method.indexOf('('));

    if (!retType.isEmpty()) {
        /* FIXME!  What if the variable is larger than this? */
        char buf[16384];
        QGenericReturnArgument rarg(qPrintable(retType), static_cast<void *>(buf));
        if (!QMetaObject::invokeMethod(o, qPrintable(methodName), connType,
            rarg, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)) {
            return RET(reply, "ERROR_IN_INVOKE");
        }
        int typeId = QMetaType::type(qPrintable(retType));
        QVariant var(typeId, static_cast<const void *>(buf));
        reply["returns"] = var;
    } else {
        if (!QMetaObject::invokeMethod(o, qPrintable(methodName), connType,
            arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)) {
            return RET(reply, "ERROR_IN_INVOKE");
        }
    }

    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::setProperty(QTestMessage const &message)
{
    QTestWidget *tw;
    QString error;
    QTestMessage reply;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);
    QObject *o = tw->instance();

    QString property = message["property"].toString();

    if (property.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }

    QVariant var = message["value"];

    QMetaObject const *mo = o->metaObject();
    int p = mo->indexOfProperty(qPrintable(property));
    if (-1 == p) {
        return RET(reply, "ERROR_PROPERTY_NOT_FOUND");
    }

    QMetaProperty mp = mo->property(p);
    if (!mp.isWritable()) {
        return RET(reply, "ERROR_PROPERTY_NOT_WRITABLE");
    }
    if (!mp.write(o, var)) {
        return RET(reply, "ERROR_SETTING_PROPERTY");
    }

    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getProperty(QTestMessage const &message)
{
    QTestMessage reply;
    QString property = message["property"].toString();
    if (property.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }

    QTestWidget *tw;
    QString error;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);
    QObject *o = tw->instance();

    QMetaObject const *mo = o->metaObject();
    int pr = mo->indexOfProperty(qPrintable(property));
    if (-1 == pr) {
        return RET(reply, "ERROR_PROPERTY_NOT_FOUND");
    }

    QMetaProperty mp = mo->property(pr);
    if (!mp.isReadable()) {
        return RET(reply, "ERROR_PROPERTY_NOT_READABLE");
    }
    QVariant var = mp.read(o);
    if (!var.isValid()) {
        return RET(reply, "ERROR_READING_PROPERTY");
    }

    reply["getProperty"] = var;
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getSetting(QTestMessage const &message)
{
    QTestMessage reply;
    QString org   = message["org"].toString();
    QString app   = message["app"].toString();
    QString path  = message["path"].toString();
    QString group = message["group"].toString();
    QString key   = message["key"].toString();
    path = p->processEnvironment(path);
    if (group.isEmpty() || key.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }
    if (!path.isEmpty() && (!org.isEmpty() || !app.isEmpty())) {
        return RET(reply, "ERROR_BAD_PARAMETERS");
    } else if (path.isEmpty() && org.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }

    QSettings *settings = 0;
    if (!path.isEmpty()) settings = new QSettings(path, QSettings::NativeFormat);
    else                 settings = new QSettings(org, app);

    settings->beginGroup(group);
    reply["getSetting"] = settings->value(key);
    delete settings;

    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::setSetting(QTestMessage const &message)
{
    QTestMessage reply;
    QString org   = message["org"].toString();
    QString app   = message["app"].toString();
    QString path  = message["path"].toString();
    QString group = message["group"].toString();
    QString key   = message["key"].toString();
    path = p->processEnvironment(path);
    if (key.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }
    if (!path.isEmpty() && (!org.isEmpty() || !app.isEmpty())) {
        return RET(reply, "ERROR_BAD_PARAMETERS");
    } else if (path.isEmpty() && org.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }

    QSettings *settings = 0;
    if (!path.isEmpty()) settings = new QSettings(path, QSettings::NativeFormat);
    else                 settings = new QSettings(org, app);

    if (!group.isEmpty())
        settings->beginGroup(group);
    settings->setValue(key, message["value"]);
    delete settings;

    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::putFile(QTestMessage const &message)
{
    QTestMessage reply;

    QString path = message["path"].toString();
    path = p->processEnvironment(path);
    QByteArray data = message["data"].toByteArray();

    if (path.isEmpty())
        return RET(reply, "ERROR_MISSING_PARAMETERS");

    {
        QFileInfo info = QFileInfo(path);
        QDir dir = info.dir();
        if (info.exists()) {
            dir.remove(info.fileName());
        } else if (!dir.exists() && !QDir("/").mkpath(dir.absolutePath())) {
            return RET(reply, "Could not create path '" + dir.absolutePath() + "'");
        }
    }

    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        if (message["permissions"].isValid() && !f.setPermissions(static_cast<QFile::Permissions>(message["permissions"].toInt()))) {
            return RET(reply, "ERROR_FILE_PERMISSIONS");
        }
        QDataStream ds(&f);
        if (data.constData()) {
            int bytesWritten = ds.writeRawData(data.constData(), data.size());
            if (bytesWritten == data.size())
                return RET(reply, "OK");
            else {
                reply["warning"] = QString("Wrote %1 byte(s), expected %2").arg(bytesWritten).arg(data.size());
                return RET(reply, "ERROR_TRUNCATED");
            }
        }
        // It's OK for data to be empty, then we create an empty file.
        return RET(reply, "OK");
    }

    return RET(reply, "ERROR_FILE_OPEN");
}

QTestMessage QTestSlavePrivate::getFile(QTestMessage const &message)
{
    QTestMessage reply;
    QString path = message["path"].toString();
    path = p->processEnvironment(path);

    if (path.isEmpty())
        return RET(reply, "ERROR_MISSING_PARAMETERS");

    QFile f(path);
    if (!f.exists()) return RET(reply, "ERROR_FILE_NOT_EXIST");
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray data;
        QDataStream ds(&f);
        data.resize(f.size());
        int bytesRead = ds.readRawData(data.data(), f.size());
        reply["getFile"] = data;
        if (bytesRead == f.size())
            return RET(reply, "OK");
        else {
            reply["warning"] = QString("Read %1 byte(s), expected %2").arg(bytesRead).arg(data.size());
            return RET(reply, "ERROR_TRUNCATED");
        }
    }
    return RET(reply, "ERROR_FILE_OPEN");
}

QTestMessage QTestSlavePrivate::getImageSize(QTestMessage const &message)
{
    QTestMessage reply;
    QString path = message["path"].toString();
    path = p->processEnvironment(path);

    if (path.isEmpty())
        return RET(reply, "Error: Path is Empty.");

    QImage f(path);
    if (f.isNull())
        return RET(reply, "Error: No Image Loaded.");
    QSize imgSize = f.size();
    reply["getImageSize"] = imgSize;
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getGeometry(QTestMessage const &message)
{
    QTestMessage reply;
    QTestWidget *tw;
    QtUiTest::Widget *w;
    QString error;

    if (!QActiveTestWidget::instance()->findWidget(message["queryPath"].toString(), tw, error))
        return RET(reply, error);

    w = qtuitest_cast<QtUiTest::Widget*>(tw->instance());

    QRect ret = w->geometry();
    ret.moveTo( w->mapToGlobal( QPoint(0,0) ) );
    reply["getGeometry"] = ret;
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::setSystemTime(QTestMessage const &message)
{
    QTestMessage reply;
#ifdef Q_OS_UNIX
    QDateTime dt = message["dateTime"].value<QDateTime>();
    if (!dt.isValid()) return RET(reply, "ERROR: Invalid date/time specified");

    struct timeval myTv;
    myTv.tv_sec = dt.toTime_t();
    myTv.tv_usec = 0;

    if ( myTv.tv_sec != -1 )
        ::settimeofday( &myTv, 0 );

    return RET(reply, "OK");
#else
    return RET(reply, "ERROR: setSystemTime is not implemented on this platform!");
#endif
}

QTestMessage QTestSlavePrivate::systemTime(QTestMessage const &message)
{
    Q_UNUSED(message);
    QTestMessage reply;
    reply["systemTime"] = QDateTime::currentDateTime();
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::getDirectoryEntries(QTestMessage const &message)
{
    QTestMessage reply;
    QString path = message["path"].toString();
    path = p->processEnvironment(path);

    if (path.isEmpty()) {
        return RET(reply, "ERROR_MISSING_PARAMETERS");
    }

    QDir d(path);
    if (!d.exists()) {
        reply["getDirectoryEntries"] = QStringList();
        return RET(reply, "OK");
    }

    QDir::Filters filters;
    {
        int filters_int = message["filters"].toInt();
        filters = QFlag(filters_int);
    }

    QStringList list = d.entryList(filters);
    reply["getDirectoryEntries"] = list;
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::deletePath(QTestMessage const &message)
{
    QTestMessage reply;
    QString path = message["path"].toString();
    path = p->processEnvironment(path);
    return RET(reply, recursiveDelete(path));
}

// All messages get processed on their own stack.
// This function is the entry point into the alternate stacks.
void QTestSlavePrivate::processMessage(QAlternateStack* stack, QVariant const& data)
{
    QVariantList list = data.toList();
    QTestSlave* slave = qobject_cast<QTestSlave*>(list.at(0).value<QObject*>());
    QTestMessage msg  = list.at(1).value<QTestMessage>();

    QTestMessage reply = slave->constructReplyToMessage(msg);

    if (!reply.isNull()) {
        // Warn if we couldn't use QAlternateStack.
        static char mem = 0;
        while (!mem) {
            mem = 1;
            if (stack) break;
            if (QApplication::type() != QApplication::GuiServer) break;
            static const char warning[] =
                "QAlternateStack is not available on this platform.\n"
                "Modal dialogs, message boxes and nested event loops may cause a hang.";
            qWarning("QtUitest: %s", warning);
            reply["warning"] = QString::fromLatin1(warning);
        }
        slave->replyMessage(&msg, reply);
    }

    // The alternate stack is no longer required, so delete it after
    // we return.
    if (stack) stack->deleteLater();
}

Q_DECLARE_METATYPE(QAlternateStackEntryPoint);
Q_DECLARE_METATYPE(QVariant);

void QTestSlave::processMessage( QTestMessage *msg )
{
    // Make sure we don't process within an alternate stack.
    foreach (QAlternateStack* stack, QAlternateStack::instances()) {
        if (stack->isCurrentStack())
            qFatal( "%s received message %s while in an alternate stack! "
                    "This is a programmer error; it means events are being "
                    "processed in an alternate stack. This should never "
                    "happen.",
                    qPrintable(qApp->applicationName()),
                    qPrintable(msg->event()));
    }

    QVariantList list;
    list.append( QVariant::fromValue(static_cast<QObject*>(this)) );
    list.append( QVariant::fromValue(*msg) );

    // If possible, use an alternate stack to handle the messages.
    // This is to avoid bug 194361.
    // We only want to switch to that stack once we get back to the event loop.
    if (QAlternateStack::isAvailable()) {
        QAlternateStack* stack = new QAlternateStack(65536, this);
        static const int type1 = qRegisterMetaType<QAlternateStackEntryPoint>();
        static const int type2 = qRegisterMetaType<QVariant>();
        Q_UNUSED(type1); Q_UNUSED(type2);
        QMetaObject::invokeMethod(stack, "start", Qt::QueuedConnection,
                Q_ARG(QAlternateStackEntryPoint, QTestSlavePrivate::processMessage),
                Q_ARG(QVariant, qVariantFromValue(list)));
    }
    // Some platforms can't use QAlternateStack.
    // In that case, just process the message as usual.  This means bug 194361
    // can happen.
    else {
        QTestSlavePrivate::processMessage(0, list);
    }
}

QTestMessage QTestSlave::constructReplyToMessage( QTestMessage const &_msg )
{
    QTestMessage reply;
    QTestMessage msg(_msg);
    reply["from"] = qApp->applicationName();

/* TO HANDLE A NEW SYSTEM TEST EVENT:
    implement a function with signature:
    QTestMessage QTestSlavePrivate::eventName(QTestMessage const &message)
*/

    QString qp = msg["queryPath"].toString();

    if (!QMetaObject::invokeMethod(d, msg.event().toLatin1().constData(), Qt::DirectConnection,
                                    Q_RETURN_ARG(QTestMessage, reply),
                                    Q_ARG(QTestMessage, msg))) {
        (void)RET(reply, "ERROR: Unhandled event '" + msg.event() + "'");
    }

    return reply;
}

/*!
    Capture a pixmap image of \a widget.

    Note that this function relies on the text cursor not blinking.  We ensure that this
    is the case by calling QApplication::setCursorFlashTime(0) in the constructor,
    but this behaviour is undocumented.

*/
bool QTestSlavePrivate::grabPixmap( QWidget *widget, QPixmap &pixmap, QString &error )
{
    Q_ASSERT(qApp);
    if ( qApp->type() == QApplication::Tty ) {
        error = "ERROR_NOT_A_GUI";
        return false;
    }
    if (!widget) {
        if (qApp->type() == QApplication::GuiServer) {
            QWidget *w = QApplication::desktop();
            WId wid = w->winId();
            pixmap = QPixmap::grabWindow(wid);
            if (pixmap.isNull())
                qLog(QtUitest) << "grabWindow() gave a null pixmap for window" << (int)wid;
            error = "OK";
            return true;
        }
        widget = qApp->activeWindow();
        qLog(QtUitest) << "using currently active window for grabPixmap";

        if (widget) {
            QWidget *w = QApplication::desktop();
            WId wid = w->winId();
            QRect rect = widget->rect();
            rect.moveTopLeft(w->mapFromGlobal(widget->mapToGlobal(rect.topLeft())));
            pixmap = QPixmap::grabWindow(wid, rect.x(), rect.y(), rect.width(), rect.height());
            error = "OK";
            return true;
        }
    }

    if ( widget == 0 ) {
        error = "ERROR_NULL_WIDGET";
        return false;
    }

    // now it's time to get the widget snapshot
    pixmap = QPixmap::grabWidget( widget );

    error = "OK";
    return true;
}

QString QTestSlave::processEnvironment( QString const& in ) const
{
    struct SystemEnvironment {
        static QMap<QString,QString> get() {
            QMap<QString,QString> ret;
            QStringList env = QProcess::systemEnvironment();
            foreach (QString str, env) {
                if (str.contains('=')) {
                    ret[str.left(str.indexOf('=')).toUpper()] = str.mid(str.indexOf('=') + 1);
                }
            }
            return ret;
        }
    };
    static const QMap<QString,QString> environment( SystemEnvironment::get() );

    QString out;
    static QRegExp re("\\$[{(]?([A-Za-z0-9_]+)[})]?");
    int offset = 0;
    while (true) {
        int index = re.indexIn(in, offset);
        if (-1 == index) {
            out += in.mid(offset);
            break;
        }
        out += in.mid(offset, index - offset);
        out += environment.value(re.cap(1).toUpper());
        offset += re.matchedLength();
    }

    return out;
}

QString QTestSlavePrivate::recursiveDelete( const QString &path ) const
{
    if (path.isEmpty()) return "ERROR_CANT_DELETE_EMPTY_PATH";

    QFileInfo i(path);
    if (!i.exists()) return "OK";
    if (!i.isDir() || i.isSymLink()) {
        if (!i.dir().remove(i.fileName())) {
            return "ERROR_CANT_DELETE_FILE_" + path;
        } else {
            return "OK";
        }
    }

    QDir dir(path);

    QStringList children = dir.entryList( QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot );

    QString res;
    foreach (QString child, children) {
        res = recursiveDelete( dir.absolutePath() + "/" + child );
        if (res != "OK" ) return res;
    }

    QString dirName = dir.dirName();
    dir.cdUp();

    if (!dir.rmdir(dirName)) {
        return "ERROR_CANT_DELETE_DIRECTORY_" + path;
    }
    return "OK";
}

bool QTestSlavePrivate::waitForIdle(int timeout)
{
    connect(this, SIGNAL(applicationBecameIdle()), this, SLOT(sendBecameIdleMessage()));
    startWaitForIdle(timeout);

    QtUiTest::waitForSignal(this, SIGNAL(applicationBecameIdle()), timeout);

    return true;
}

QTestMessage QTestSlavePrivate::waitForIdle(QTestMessage const &message)
{
    QTestMessage reply;
    if (waitForIdle(message["timeout"].toInt()))
        return RET(reply, "OK");
    return RET(reply, "ERROR_IDLE_TIMEOUT");
}

QTestMessage QTestSlavePrivate::selectItem( QTestMessage const &message )
{
    ensureFocus();

    bool should_rescan = false;
    QTestMessage reply;
    int try_count = 5;
    QString text = message["text"].toString();
    QString error;

    qLog(QtUitest) << "selectItem" << text << message["queryPath"].toString();

    while (try_count-- > 0) {
        if (should_rescan) {
            // Give Qtopia x ms to process events and then try again
            QtUiTest::wait(200);
            if (!QActiveTestWidget::instance()->rescan(error)) {
                return RET(reply, error);
            }
            reply["warning"] = "Item selection failed ... will try again in 200 ms";
            should_rescan = false;
        }
        QTestWidget *tw;
        QString qp = message["queryPath"].toString();

        /* See if we should interpret select("Something") as activate("Something").
            * This is a bit kludgy, but probably OK.
            * From a user's point of view, activating a button from a set of buttons
            * and selecting an action from a list are pretty much the same thing.
            */
        if (qp.isEmpty() && !text.isEmpty()) {
            if (!QActiveTestWidget::instance()->rescan(error)) {
                return RET(reply, error);
            }
            QStringList labels = QActiveTestWidget::instance()->allLabels();

            /* Only interpret it as an activate if the query path is unambiguous.*/
            QRegExp re("(^ *|.+/ *)" + QRegExp::escape(text) + " *$");
            int ind1 = labels.indexOf(re);
            int ind2 = labels.lastIndexOf(re);
            if (ind1 != -1 && ind1 == ind2) {
                QTestMessage newMessage(message);
                newMessage["queryPath"] = labels.at(ind1);
                return activate(newMessage);
            }
        }
        QtUiTest::SelectWidget *sw = 0;
        QtUiTest::ListWidget   *lw = 0;

        /* Special cases. */
        if (text == CALL_ACCEPT_ALIAS || text == CALL_HANGUP_ALIAS) {
            text = (text == CALL_ACCEPT_ALIAS) ? "accept" : "hangup";
            using namespace QtUiTest;
            QObject* callManager = findWidget  ( CallManager );
            sw = qtuitest_cast<SelectWidget*>( callManager );
            lw = qtuitest_cast<ListWidget*>  ( callManager );
            qp = "CallManager";
        }

        /* Regular case: look up test widget by query path. */
        if (!sw) {
            if (!QActiveTestWidget::instance()->findWidget(qp, tw, error)) {
                if (try_count > 0) {
                    should_rescan = true;
                    continue;
                } else {
                    return RET(reply,error);
                }
            }
            if (qp.isEmpty()) qp = tw->signature();;
            sw = qtuitest_cast<QtUiTest::SelectWidget*>(tw->instance());
            lw = qtuitest_cast<QtUiTest::ListWidget*>  (tw->instance());
        }

        if (!sw) {
            QString resolvedQp;
            QDebug(&resolvedQp) << tw->instance();
            return RET(reply, "ERROR: " + qp + " (" + resolvedQp + ") is not a SelectWidget.");
        }
        if (!sw->select(text)) {
            if (!sw->canSelect(text)) {
                if (try_count > 0) {
                    should_rescan = true;
                    continue;
                } else {
                    QString error = "ERROR: item '" + text
                        + "' is not available for selection in " + qp;
                    if  (!QtUiTest::errorString().isEmpty()) {
                        error += "\n" + QtUiTest::errorString();
                    }
                    if (lw) {
                        error += "\nAvailable items: " + lw->list().join(",");
                    }
                    return RET(reply, error);
                }
            }
            error = "ERROR: unknown error selecting item '" + text
                    + "' from " + qp;
            if (!QtUiTest::errorString().isEmpty())
                error = QtUiTest::errorString();
            return RET(reply, error);
        }
        return RET(reply, "OK");
    }

    error = "ERROR: unknown error selecting item '" + text + "'";
    return RET(reply, error);
}

QTestMessage QTestSlavePrivate::activate( QTestMessage const &message )
{
    ensureFocus();

    QTestWidget *tw;
    QString error;
    QTestMessage reply;
    QString qp = message["queryPath"].toString();

    if (!QActiveTestWidget::instance()->findWidget(qp, tw, error))
        return RET(reply,error);
    if (qp.isEmpty()) qp = tw->signature();;

    QtUiTest::ActivateWidget *aw
        = qtuitest_cast<QtUiTest::ActivateWidget*>(tw->instance());

    if (!aw) {
        return RET(reply, "ERROR: " + qp + " is not an ActivateWidget.");
    }
    aw->activate();
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::enterText( QTestMessage const &message )
{
    ensureFocus();

    QTestWidget* tw;
    QString error;
    QString qp = message["queryPath"].toString();
    QVariant text = message["text"];
    bool mode = message["mode"] == "NoCommit";

    QTestMessage reply;
    if (!QActiveTestWidget::instance()->findWidget(qp, tw, error))
        return RET(reply,error);

    QPointer<QTestWidget> safeTw = tw;

    QtUiTest::InputWidget *iw
        = qtuitest_cast<QtUiTest::InputWidget*>(tw->instance());

    QString sig = QTestWidgets::signature(tw->instance());

    if (!iw) {
        return RET(reply, "ERROR: " + qp + " is not an InputWidget. Signature: " + sig);
    }

    if (!iw->enter(text, mode)) {
        QString error;
        if (!safeTw) {
            error = "ERROR: " + qp + " was destroyed while entering text into it.";
        } else if (!iw->canEnter(text) && QtUiTest::errorString().isEmpty()) {
            QString resolvedQp;
            QDebug(&resolvedQp) << tw->instance();
            return RET(reply, "ERROR: '" + text.toString()
                    + "' is not valid input for " + qp + " (" + resolvedQp + "; " + sig + ")");
        } else {
            error = "ERROR: unknown error entering '" + text.toString()
                + "' into " + qp + ". Signature: " + sig;
            if (!QtUiTest::errorString().isEmpty())
                error = QtUiTest::errorString();
        }
        return RET(reply, error);
    }
    return RET(reply, "OK");
}

QTestMessage QTestSlavePrivate::waitForText( QTestMessage const &message )
{
    QTestWidget *tw;
    QString error;
    QTestMessage reply;
    if (!QActiveTestWidget::instance()->findWidget( message["queryPath"].toString(), tw, error))
        return RET(reply,error);

    QStringList waitText = message["waitText"].toStringList();
    bool exact = message["exact"].toBool();
    int timeout = message["timeout"].toInt();
    if (timeout == -1) timeout = VISIBLE_RESPONSE_TIME;

    QElapsedTimer t;
    QString ret;
    bool ok;
    do {
        QtUiTest::wait(5);
        ret = tw->getSelectedText(&ok);
        if (ret.isEmpty())
            return RET(reply,"ERROR: Could not 'read' the progressBar");

        if (exact) {
            for (int i=0; i<waitText.count(); i++) {
                if (ret == waitText[i])
                    return RET(reply,"OK");
            }
        } else {
            for (int i=0; i<waitText.count(); i++) {
                if (ret.contains( waitText[i] ))
                    return RET(reply,"OK");
            }
        }
    } while (t.elapsed() < timeout);

    if (waitText.count() > 1)
        return RET(reply,QString("ERROR: the field did not change to one of [%1] within '%2' ms").arg(waitText.join(",")).arg(t.elapsed()));
    else
        return RET(reply,QString("ERROR: the field did not change to '%1' within '%2' ms").arg(waitText[0]).arg(t.elapsed()));
}




QTestMessage QTestSlavePrivate::keyPress(QTestMessage const &message)
{
    ensureFocus();
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
            bool keyRepeat=QtUiTest::testInputOption(QtUiTest::KeyRepeat);
            QtUiTest::setInputOption(QtUiTest::KeyRepeat, true);
            QtUiTest::keyPress(key);
            while (duration > 0) {
                QtUiTest::wait(150);
                duration -= 150;
                QtUiTest::keyPress(key);
            }
            QtUiTest::setInputOption(QtUiTest::KeyRepeat, keyRepeat);
        } else {
            QtUiTest::wait(duration);
        }
    }

    return RET(reply, "OK" );
}

QTestMessage QTestSlavePrivate::keyRelease(QTestMessage const &message)
{
    ensureFocus();
    QTestMessage reply;
    if (!message["key"].isValid()) {
        return RET(reply, "ERROR_MISSING_KEY");
    }
    Qt::Key key = (Qt::Key)(message["key"].toInt());
    if (!key) return RET(reply, "ERROR_ZERO_KEY");
    QtUiTest::keyRelease(key);
    return RET(reply, "OK" );
}

QTestMessage QTestSlavePrivate::keyClick(QTestMessage const &message)
{
    ensureFocus();
    QTestMessage reply;
    if (!message["key"].isValid()) {
        return RET(reply, "ERROR_MISSING_KEY");
    }

    Qt::Key key = (Qt::Key)(message["key"].toInt());
    if (!key) return RET(reply, "ERROR_ZERO_KEY");
    QtUiTest::keyClick(key);
    return RET(reply, "OK" );
}

QTestMessage QTestSlavePrivate::mousePress(QTestMessage const &message)
{
    ensureFocus();
    QTestMessage reply;
    bool ok;
    QPoint pos(mousePointForMessage(message, reply, ok));
    if (ok) QtUiTest::mousePress(pos);
    else return reply;
    return RET(reply, "OK" );
}

QTestMessage QTestSlavePrivate::mouseRelease(QTestMessage const &message)
{
    ensureFocus();
    QTestMessage reply;
    bool ok;
    QPoint pos(mousePointForMessage(message, reply, ok));
    if (ok) QtUiTest::mouseRelease(pos);
    else return reply;
    return RET(reply, "OK" );
}

QTestMessage QTestSlavePrivate::mouseClick(QTestMessage const &message)
{
    ensureFocus();
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

QTestMessage QTestSlavePrivate::scrollByMouse(QTestMessage const &msg)
{
    QTestMessage reply;
    QTestMessage message("getCenter", msg);
    QString qp = message["queryPath"].toString();
    qLog(QtUitest) << "Try scrolling to" << (qp + (!qp.isEmpty() ? "/" : "") + message["item"].toString());

    reply = getCenter(msg);
    if (!reply.statusOK()) return reply;
    QPoint target = reply["getCenter"].value<QPoint>();

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

        if (!reply.statusOK()) return reply;
        target = reply["getCenter"].value<QPoint>();
    }

    if (reply["isVisible"].toBool())
        return RET(reply, "OK");
    else
        return RET(reply, "ERROR: couldn't make " + qp + " visible by scrolling");
}

QPoint QTestSlavePrivate::mousePointForMessage(QTestMessage const &message, QTestMessage &reply, bool &ok)
{
    ok = true;
    if (message["pos"].isValid())
        return message["pos"].value<QPoint>();
    reply = getCenter( message );
    return reply["getCenter"].value<QPoint>();
}

QTestMessage QTestSlavePrivate::getenv( QTestMessage const &message )
{
    QTestMessage reply;
    reply["getenv"] = QVariant::fromValue(qgetenv(message["key"].toString().toLatin1()));
    return RET(reply, "OK");
}

bool QTestSlavePrivate::event(QEvent *e)
{
    if ((int)IdleEvent::Type != (int)e->type()) return false;
    e->accept();

    extern uint qGlobalPostedEventsCount();

    static uint lastCount = 0;
    static uint times     = 0;

    IdleEvent *i = static_cast<IdleEvent*>(e);
    uint count = qGlobalPostedEventsCount();
    if (count == lastCount) {
        ++times;
    } else {
        lastCount = count;
        times = 0;
    }
    if (times < 50 && count > 1) {
        if (i->timeout == -1 || i->time.elapsed() < i->timeout) {
            IdleEvent *new_i = new IdleEvent(*i);
            QCoreApplication::postEvent(this, new_i, -2000);
        }
    } else {
        emit applicationBecameIdle();
    }

    return true;
}

void QTestSlavePrivate::startWaitForIdle(int timeout)
{
    // FIXME: this still may go wrong with time changes. Should we derive QElapsedTimer from QTime and used that instead?
    QTime now;
    now.start();
    IdleEvent *i = new IdleEvent(now, timeout);
    QCoreApplication::postEvent(this, i, -2000);
}

void QTestSlavePrivate::sendBecameIdleMessage()
{
    QTestMessage msg("appBecameIdle");
    msg["appName"] = QCoreApplication::applicationName();
    p->postMessage(msg);
}

// ***********************************************************************
// ***********************************************************************

#define RECORD_IMPL(Type, ...) \
    p->recordEvent(RecordEvent::Type, QActiveTestWidget::instance()->friendlyName(object), QActiveTestWidget::instance()->friendlyName(QActiveTestWidget::instance()->focusWidget()), ##__VA_ARGS__)

void QTestSlavePrivate::record_entered(QObject* object, QVariant const& item)
{ RECORD_IMPL(Entered, item); }

void QTestSlavePrivate::record_gotFocus(QObject* object)
{ RECORD_IMPL(GotFocus); }

void QTestSlavePrivate::record_activated(QObject* object)
{ RECORD_IMPL(Activated); }

void QTestSlavePrivate::record_selected(QObject* object, QString const& item)
{ RECORD_IMPL(Selected, item); }

void QTestSlavePrivate::record_stateChanged(QObject* object, int state)
{ RECORD_IMPL(CheckStateChanged, state); }

// ***********************************************************************
// ***********************************************************************

void QTestSlave::showMessageBox( QWidget *window, QString const& title, QString const& text )
{
    qLog(QtUitest) << "Message box appeared:" << window << title << text;
    QTestMessage message("show_messagebox");
    message["title"]     = title;
    message["text"]      = text;
    message["signature"] = QTestWidgets::signature(window);
    postMessage(message);
}

void QTestSlave::showDialog( QWidget *window, QString const& title )
{
    QTestMessage message("show_dialog");
    message["title"]     = title;
    message["text"]      = "";
    message["signature"] = QTestWidgets::signature(window);
    postMessage(message);
}
