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

#include "qtuitestrecorder.h"
#include "qtuitestnamespace.h"

#include <QSet>
#include <QApplication>
#include <QWidget>
#include <QMetaType>
#include <QPointer>

Q_DECLARE_METATYPE(QPointer<QObject>)

/*!
    \preliminary
    \class QtUiTestRecorder
    \inpublicgroup QtUiTestModule
    \brief The QtUiTestRecorder class provides an interface for observing
           application-wide high level user actions.

    QtUiTestRecorder aggregates the signals declared in the
    \l{QtUiTest}{QtUiTest widget interfaces} to provide a single object
    capable of watching for user input.

    Note that using this class is expensive because it involves watching
    for signals on a large number of objects.  When you create a
    QtUiTestRecorder and connect to its signals, this will result in every
    existing widget being wrapped in a \l{QtUiTest}{test widget}.
    To avoid this, do not connect to the signals of a QtUiTestRecorder
    unless you will definitely use the result.
*/

class WidgetWatcher;

class QtUiTestRecorderPrivate : public QObject
{
    Q_OBJECT
public:
    static WidgetWatcher* watcher;
    static void createOrDestroyWatcher();

public slots:
    void on_gotFocus();
    void on_activated();
    void on_stateChanged(int);
    void on_entered(QVariant const&);
    void on_selected(QString const&);
};

WidgetWatcher* QtUiTestRecorderPrivate::watcher = 0;

Q_GLOBAL_STATIC(QtUiTestRecorderPrivate, qtopiaTestRecorderPrivate);
Q_GLOBAL_STATIC(QSet<QtUiTestRecorder*>, qtopiaTestRecorderInstances);

/*! \internal */
void QtUiTestRecorder::emitKeyEvent(int key, int mod, bool press, bool repeat)
{
    foreach(QtUiTestRecorder* r, *qtopiaTestRecorderInstances()) {
        emit r->keyEvent(key, mod, press, repeat);
    }
}

/*! \internal */
void QtUiTestRecorder::emitMouseEvent(int x, int y, int state)
{
    foreach(QtUiTestRecorder* r, *qtopiaTestRecorderInstances()) {
        emit r->mouseEvent(x, y, state);
    }
}

#define ON_IMPL(func, ...) \
    foreach(QtUiTestRecorder* r, *qtopiaTestRecorderInstances()) { \
        emit r->func(sender(), ##__VA_ARGS__); \
    }

void QtUiTestRecorderPrivate::on_gotFocus()
{ ON_IMPL(gotFocus); }

void QtUiTestRecorderPrivate::on_activated()
{ ON_IMPL(activated); }

void QtUiTestRecorderPrivate::on_stateChanged(int state)
{ ON_IMPL(stateChanged, state); }

void QtUiTestRecorderPrivate::on_entered(QVariant const& item)
{ ON_IMPL(entered, item); }

void QtUiTestRecorderPrivate::on_selected(QString const& item)
{ ON_IMPL(selected, item); }

/*
    This helper class attempts to ensure that all created objects immediately
    have wrappers constructed around them.  This is necessary for event
    recording to work properly, as it relies on test widgets emitting signals.

    Since it is so expensive, an instance of this class should not be left
    around whenever we don't have any QtUiTestRecorder instances.
*/
class WidgetWatcher : public QObject
{
    Q_OBJECT
public:
    WidgetWatcher();
    void castAllChildren(QObject*);

public slots:
    bool eventFilter(QObject*,QEvent*);
    void castAllChildren(QPointer<QObject>);
};

WidgetWatcher::WidgetWatcher()
{
    castAllChildren(qApp);
    foreach(QWidget* w, qApp->topLevelWidgets()) {
        castAllChildren(w);
    }
}

bool WidgetWatcher::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::ChildAdded) {
        static struct Registrar {
            Registrar() {
                qRegisterMetaType< QPointer<QObject> >();
            }
        } r;
        QPointer<QObject> ptr(o);
        QMetaObject::invokeMethod(this, "castAllChildren",
                                  Qt::QueuedConnection,
                                  Q_ARG(QPointer<QObject>, ptr));
    }
    return false;
}

void WidgetWatcher::castAllChildren(QPointer<QObject> o)
{ castAllChildren(static_cast<QObject*>(o)); }

void WidgetWatcher::castAllChildren(QObject* o)
{
    if (!o) return;
    qtuitest_cast<QtUiTest::Widget*>(o);
    o->installEventFilter(this);
    foreach (QObject* child, o->children())
        castAllChildren(child);
}

void QtUiTestRecorderPrivate::createOrDestroyWatcher()
{
    QSet<QtUiTestRecorder*> const *recorders = qtopiaTestRecorderInstances();

    bool need_watcher = false;
    foreach (QtUiTestRecorder* rec, *recorders) {
        if (rec->receivers(SIGNAL(gotFocus(QObject*)))
         || rec->receivers(SIGNAL(activated(QObject*)))
         || rec->receivers(SIGNAL(stateChanged(QObject*,int)))
         || rec->receivers(SIGNAL(entered(QObject*,QVariant)))
         || rec->receivers(SIGNAL(selected(QObject*,QString))))
            need_watcher = true;

        if (need_watcher) break;
    }

    if (watcher && !need_watcher) {
        delete watcher;
        watcher = 0;
    } else if (!watcher && need_watcher) {
        watcher = new WidgetWatcher;
    }
}


/*!
    Constructs a QtUiTestRecorder with the given \a parent.
*/
QtUiTestRecorder::QtUiTestRecorder(QObject* parent)
    : QObject(parent)
{
    *qtopiaTestRecorderInstances() << this;
}

/*!
    Destroys the QtUiTestRecorder.
*/
QtUiTestRecorder::~QtUiTestRecorder()
{
    qtopiaTestRecorderInstances()->remove(this);
    QtUiTestRecorderPrivate::createOrDestroyWatcher();
}

/*!
    \internal
    May cause a WidgetWatcher to be created.
*/
void QtUiTestRecorder::connectNotify(char const*)
{
    QtUiTestRecorderPrivate::createOrDestroyWatcher();
}

/*!
    \internal
    May cause a WidgetWatcher to be destroyed.
*/
void QtUiTestRecorder::disconnectNotify(char const*)
{
    QtUiTestRecorderPrivate::createOrDestroyWatcher();
}

/*!
    \fn void QtUiTestRecorder::gotFocus(QObject* widget)

    Emitted when \a widget obtains focus by any means.

    \sa QtUiTest::Widget::gotFocus()
*/

/*!
    \fn void QtUiTestRecorder::activated(QObject* widget)

    Emitted when \a widget is activated by any means.

    In this context, "activated" means, for example, clicking a button.

    \sa QtUiTest::ActivateWidget::activated()
*/

/*!
    \fn void QtUiTestRecorder::stateChanged(QObject* widget, int state)

    Emitted when the check state changes to \a state in \a widget.

    \sa QtUiTest::CheckWidget::stateChanged()
*/

/*!
    \fn void QtUiTestRecorder::entered(QObject* widget, QVariant const& item)

    Emitted when \a item is entered into \a widget.

    \sa QtUiTest::InputWidget::entered()
*/

/*!
    \fn void QtUiTestRecorder::selected(QObject* widget, QString const& item)

    Emitted when \a item is selected from \a widget.

    \sa QtUiTest::SelectWidget::selected()
*/

/*!
    \fn void QtUiTestRecorder::keyEvent(int key, int modifiers, bool isPress, bool isAutoRepeat)

    Emitted when \a key (with \a modifiers) is pressed or released.

    \a key is compatible with Qt::Key and \a modifiers is compatible with
    Qt::KeyboardModifiers.

    If the event is a press \a isPress is true, otherwise it is a release.
    \a isAutoRepeat is true if the event is generated due to autorepeat.

    This signal is only emitted within the server process, because individual
    applications cannot monitor system-wide key events.
*/

/*!
    \fn void QtUiTestRecorder::mouseEvent(int x, int y, int state)

    Emitted when the mouse changes state.

    \a x and \a y are the global co-ordinates of the event.
    \a state is the current state of the mouse buttons, compatible with Qt::MouseButtons.

    This signal is only emitted within the server process, because individual
    applications cannot monitor system-wide mouse events.
*/

/*!
    \internal
    Connects all testwidget signals on \a object to all test recorder
    instances.
*/
void QtUiTestRecorder::connectToAll(QObject* o)
{
    if (!o) return;

    QMetaObject const* mo = o->metaObject();
    QSet<QByteArray> _signals;

#define _DO(Iface, Signal) \
    if (qobject_cast<QtUiTest::Iface*>(o) && (-1 != mo->indexOfSignal(Signal))) { \
        _signals << Signal; \
    }

    _DO(Widget,         "gotFocus()");
    _DO(ActivateWidget, "activated()");
    _DO(CheckWidget,    "stateChanged(int)");
    _DO(InputWidget,    "entered(QVariant)");
    _DO(SelectWidget,   "selected(QString)");

#undef _DO

    foreach (QByteArray sig, _signals) {
        QByteArray recorder_slot = SLOT(on_) + sig;
        sig.prepend(SIGNAL());

        /* Avoid multiple connections */
        disconnect(o, sig, qtopiaTestRecorderPrivate(), recorder_slot);
        connect(   o, sig, qtopiaTestRecorderPrivate(), recorder_slot);
    }
}

#include "qtuitestrecorder.moc"

