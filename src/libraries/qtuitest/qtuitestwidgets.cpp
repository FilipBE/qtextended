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


#include "demowidgets_p.h"
#include "qinputgenerator_p.h"
#include "qtuitestrecorder.h"
#include "qtuitestwidgetinterface.h"
#include "qtuitestwidgets_p.h"

#ifdef QTOPIA_TARGET
# include <QtopiaIpcAdaptor>
# include <QtopiaIpcEnvelope>
# include <qtopialog.h>
# include <Qtopia>
#else
# include <QDebug>
# define qLog(A)            if(1); else qDebug() << #A
#endif

/*
    If the QTUITEST_INPUT_DELAY environment variable is set, all calls to keyClick etc will
    immediately return but the resulting event won't be generated until QTUITEST_INPUT_DELAY
    milliseconds have passed.
    This can be used to simulate a slow system to catch race conditions.
    For example, setting QTUITEST_INPUT_DELAY to 500 roughly simulates running with a remote X
    server over a link with a round trip time of 250ms.

    Disable this define to disable the race condition testing code.
*/
#define QTUITEST_RACE_TEST

#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QPointer>
#include <QStringList>
#include <QTimer>

/*!
    \internal
    \class QtUiTestWidgets
    \brief The QtUiTestWidgets class provides an interface for creating and managing QtUiTest widgets.

    QtUiTestWidgets manages the lifetime of all test widgets and allows
    test widgets to simulate user interaction with Qtopia.
*/

class QtUiTestWidgetsService;

#ifdef QTUITEST_RACE_TEST
enum QtUiTestInputType { QtUiTestPress, QtUiTestRelease, QtUiTestClick };
struct QtUiTestKeyEvent
{
    QtUiTestInputType     type;
    Qt::Key               key;
    Qt::KeyboardModifiers mod;
    QtUiTest::InputOption opt;
};
struct QtUiTestMouseEvent
{
    QtUiTestInputType     type;
    QPoint                pos;
    Qt::MouseButtons      state;
    QtUiTest::InputOption opt;
};
static int qtUiTestGetInputDelay()
{
    bool ok;
    QByteArray value(qgetenv("QTUITEST_INPUT_DELAY"));
    int ret = value.toInt(&ok);
    if (!ok || ret < 0) ret = -1;
    return ret;
}
static int qtUiTestInputDelay()
{ static int ret = qtUiTestGetInputDelay(); return ret; }
#endif


class QtUiTestWidgetsPrivate
{
public:
    QtUiTestWidgetsPrivate(QtUiTestWidgets* parent);

    static QList<QByteArray> allImplementedInterfaces(QObject *o);

    void _q_objectDestroyed();

    QWidget *inputMethodsWidget() const;
    QString currentInputMethod() const;

    QtUiTestWidgets* q;

    QHash< QByteArray, QSet<QtUiTest::WidgetFactory*> >      factories;
    QHash< QObject*,   QHash<QByteArray, QPointer<QObject> > > testWidgets;
    QSet< QtUiTest::WidgetFactory* > factorySet;

    int inputOptions;

    QString errorString;

    QInputGenerator input;

#ifdef QTUITEST_RACE_TEST
    QList<QtUiTestKeyEvent>   pendingKeyEvents;
    QList<QtUiTestMouseEvent> pendingMouseEvents;
    void _q_postNextKeyEvent();
    void _q_postNextMouseEvent();
#else
    inline void _q_postNextKeyEvent()   {}
    inline void _q_postNextMouseEvent() {}
#endif
};

class QTWOptStack
{
public:
    QTWOptStack(QtUiTestWidgetsPrivate* obj,
            QtUiTest::InputOption opt)
        : d(obj), option(opt)
    {
        if (opt && (!(d->inputOptions & opt))) {
            d->inputOptions |= opt;
        } else {
            d = 0;
        }
    }

    ~QTWOptStack()
    {
        if (d) {
            d->inputOptions &= ~option;
        }
    }

    QtUiTestWidgetsPrivate *d;
    QtUiTest::InputOption option;
};

QtUiTestWidgetsPrivate::QtUiTestWidgetsPrivate(QtUiTestWidgets* parent)
    :   q(parent),
        inputOptions(QtUiTest::NoOptions)
{
}

QtUiTestWidgets::QtUiTestWidgets()
    : QObject(),
      d(new QtUiTestWidgetsPrivate(this))
{
    refreshPlugins();
}

QtUiTestWidgets::~QtUiTestWidgets()
{
    delete d;
    d = 0;
}

/*!
    Returns a static instance of QtUiTestWidgets.
*/
QtUiTestWidgets* QtUiTestWidgets::instance()
{
    static QtUiTestWidgets instance;
    return &instance;
}

/*!
    When an object is destroyed, deletes all test widgets pointing
    to that object.
*/
void QtUiTestWidgetsPrivate::_q_objectDestroyed()
{
    QHash< QByteArray, QPointer<QObject> > toDestroy
        = testWidgets.take(q->sender());

    foreach (QPointer<QObject> tw, toDestroy.values()) {
        if (tw) delete tw;
    }
}

/*!
    \internal
    Destroy all test widgets and unregister all factories.
    After calling this, refreshPlugins() must be called to be able to
    construct testwidgets from factories.

    For testing purposes only.
*/
void QtUiTestWidgets::clear()
{
    d->factories.clear();
    d->factorySet.clear();

    foreach (QObject *o, d->testWidgets.keys()) {
        foreach (QPointer<QObject> tw, d->testWidgets[o].values()) {
            if (tw && tw != o) delete tw;
        }
    }
    d->testWidgets.clear();
}

/*!
    Registers \a factory as a factory class for constructing Qt Extended test
    widgets.

    It is not necessary to explicitly call this from QtUiTest widget plugins.
    This function should only be called if a QtUiTest::WidgetFactory has been
    created without using the standard plugin interface.
*/
void
QtUiTestWidgets::registerFactory(QtUiTest::WidgetFactory* factory)
{
    if (!factory) return;

    d->factorySet << factory;
    foreach(QString k, factory->keys()) {
        d->factories[k.toLatin1()].insert(factory);
    }
}

/*!
    Returns a human-readable error string describing the last error which
    occurred while accessing a testwidget.

    The error string is used to report directly to a tester any unexpected
    errors.  The string will typically be used as a test failure message.

    \sa setErrorString()
*/
QString QtUiTestWidgets::errorString() const
{ return d->errorString; }

/*!
    Sets the human-readable \a error string describing the last error which
    occurred while accessing a testwidget.

    \sa errorString()
*/
void QtUiTestWidgets::setErrorString(QString const& error)
{
    if (error == d->errorString) return;
    d->errorString = error;

#ifdef QTOPIA_TARGET
    // Broadcast the error string change to all processes
    QtopiaIpcEnvelope req("QPE/QtUiTest", "setErrorString(QString)");
    req << error;
#endif
}

/*!
    Returns a list of all QtUiTest widget interfaces implemented by \a o .
*/
QList<QByteArray> QtUiTestWidgetsPrivate::allImplementedInterfaces(QObject *o)
{
    // FIXME this function should not have to be explicitly implemented.
    // Find some way to automatically handle all interfaces.
    QList<QByteArray> ret;
    if (qobject_cast<QtUiTest::Widget*>(o))         ret << "Widget";
    if (qobject_cast<QtUiTest::ActivateWidget*>(o)) ret << "ActivateWidget";
    if (qobject_cast<QtUiTest::LabelWidget*>(o))    ret << "LabelWidget";
    if (qobject_cast<QtUiTest::CheckWidget*>(o))    ret << "CheckWidget";
    if (qobject_cast<QtUiTest::TextWidget*>(o))     ret << "TextWidget";
    if (qobject_cast<QtUiTest::ListWidget*>(o))     ret << "ListWidget";
    if (qobject_cast<QtUiTest::InputWidget*>(o))    ret << "InputWidget";
    if (qobject_cast<QtUiTest::SelectWidget*>(o))   ret << "SelectWidget";
    return ret;
}

/*!
    Returns a test widget wrapper for \a object implementing the given
    \a interface.  If a test widget implementing \a interface is already
    wrapping \a object, that test widget will be returned.  Otherwise,
    a new test widget may be constructed using registered factories.

    Returns 0 if the given \a interface is not implemented on \a object
    or on any test widget which wraps \a object.

    The returned object should not be deleted by the caller.  QtUiTestWidgets
    retains ownership of the returned test widget and deletes it when \a object
    is destroyed.

    \sa registerFactory()
*/
QObject* QtUiTestWidgets::testWidget(QObject* object, QByteArray const &interface)
{
    if (!object) return 0;

    QMetaObject const *mo = object->metaObject();
    QObject *ret = d->testWidgets.value( object ).value( interface );

    bool watchingDestroyed = false;

    if (!ret) {
        QSet<QtUiTest::WidgetFactory*> usedFactories;
        while (mo) {
            foreach (QtUiTest::WidgetFactory *factory,
                    d->factories.value(mo->className()) - usedFactories) {

                QObject *testWidget = factory->create(object);
                usedFactories.insert(factory);
                if (testWidget) {
                    bool isValuable = false;
                    foreach (QByteArray const& thisIface,
                             d->allImplementedInterfaces(testWidget)) {
                        QHash<QByteArray, QPointer<QObject> > &subhash
                            = d->testWidgets[object];
                        if (!subhash[thisIface]) {
                            isValuable = true;
                            subhash.insert( thisIface, testWidget );
                            watchingDestroyed = watchingDestroyed
                                || connect(object, SIGNAL(destroyed()),
                                        this, SLOT(_q_objectDestroyed()));
                        }
                    }
                    if (!isValuable) {
                        delete testWidget;
                    } // if (!isValuable)
                    else {
                        QtUiTestRecorder::connectToAll(testWidget);
                    } // if (isValuable)
                } // if (testWidget)
            } // foreach factory
            mo = mo->superClass();
        }
        ret = d->testWidgets.value( object ).value( interface );
    }
    return ret;
}

/*!
    Returns a widget of \a type using factories.
*/
QObject* QtUiTestWidgets::findWidget(QtUiTest::WidgetType type)
{
    foreach (QtUiTest::WidgetFactory *factory, d->factorySet) {
        if (QObject *ret = factory->find(type))
            return ret;
    }

    return 0;
}

/*!
    \internal
    Load factories from all existing qtuitest_widgets plugins.
*/
void QtUiTestWidgets::refreshPlugins()
{
    /* First, handle static plugins. */
    foreach (QObject *o, QPluginLoader::staticInstances()) {
        registerFactory(qobject_cast<QtUiTest::WidgetFactory*>(o));
    }

#ifndef QTOPIA_TEST     // Don't load plugins from unit test
    QSet<QString> pluginsToLoad;

    QList<QDir> pluginDirs;
    foreach (QByteArray const& split, qgetenv("QTUITEST_WIDGETS_PATH").split(':')) {
        if (split.isEmpty()) continue;
        QDir dir(split);
        if (dir.exists()) pluginDirs << dir;
    }

    QString const pluginType("qtuitest_widgets");
    QSet<QString> libPaths = QCoreApplication::libraryPaths().toSet();
#ifdef QTOPIA_TARGET
    foreach (QString const& instPath, Qtopia::installPaths())
        libPaths << instPath + "/plugins";
#endif
    foreach (QString const& libPath, libPaths) {
        QDir dir(libPath + "/" + pluginType);
        if (!dir.exists()) {
            continue;
        }
        pluginDirs << dir;
    }

    foreach (QDir const& dir, pluginDirs) {
        foreach (QString const& file, dir.entryList(QDir::Files|QDir::NoDotAndDotDot)) {
            QString filename = dir.canonicalPath() + "/" + file;
            if (!QLibrary::isLibrary(filename)) continue;
            pluginsToLoad << filename;
        }
    }

    QLibrary libLoader;
    QSet<QString> lastPluginsToLoad;
    QStringList errors;

    // dumb dependency handling: keep trying to load plugins until we
    // definitely can't progress.
    while (lastPluginsToLoad != pluginsToLoad) {
        lastPluginsToLoad = pluginsToLoad;
        errors.clear();
        foreach (QString const& plugin, pluginsToLoad) {
            libLoader.setFileName(plugin);
            // enable RTLD_GLOBAL, so plugins can access each other's symbols.
            // This is why QPluginLoader can't be used here.
            // xxx workaround for Qt bug 190831: need to explicitly call this after
            // xxx each call to setFileName
            libLoader.setLoadHints(QLibrary::ExportExternalSymbolsHint);
            libLoader.load();

            typedef QObject* (*PluginFunction)();
            PluginFunction instance = (PluginFunction)libLoader.resolve("qt_plugin_instance");
            QString error;

            if (!instance)
                error = "cannot resolve 'qt_plugin_instance': " + libLoader.errorString();

            QObject *o = 0;
            if (instance)
                o = instance();
            QtUiTest::WidgetFactory* fact = qobject_cast<QtUiTest::WidgetFactory*>(o);
            if (!fact) {
                if (error.isEmpty()) error = libLoader.errorString();
                QString formattedError;
                QDebug(&formattedError)
                    << "QtUitest: failed to load qtuitest widgets plugin"
                    << "\n   plugin"   << plugin
                    << "\n   instance" << o
                    << "\n   error"    << error;
                errors << formattedError;
            } else {
                pluginsToLoad -= plugin;
                registerFactory(fact);
            }
        }
    }

    foreach (QString const& error, errors)
        qWarning() << qPrintable(error);
#endif
}

/*!
    Simulate a mouse press event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mousePress(QPoint const &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
#ifdef QTUITEST_RACE_TEST
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestPress, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }
#endif
    QTWOptStack st(d, opt);
    d->input.mousePress(pos, state);
}

/*!
    Simulate a mouse release event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mouseRelease(QPoint const &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
#ifdef QTUITEST_RACE_TEST
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestRelease, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }
#endif
    QTWOptStack st(d, opt);
    d->input.mouseRelease(pos, state);
}

/*!
    Simulate a mouse click event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mouseClick(QPoint const &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
#ifdef QTUITEST_RACE_TEST
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestClick, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }
#endif
    QTWOptStack st(d, opt);
    d->input.mouseClick(pos, state);
}

/*!
    Simulate a key press event, using the given \a key and \a mod.
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::keyPress(Qt::Key key, Qt::KeyboardModifiers mod,
        QtUiTest::InputOption opt)
{
#ifdef QTUITEST_RACE_TEST
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestKeyEvent event = {QtUiTestPress, key, mod, opt};
        d->pendingKeyEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextKeyEvent()));
        return;
    }
#endif
    QTWOptStack st(d, opt);
    d->input.keyPress(key, mod, opt & QtUiTest::KeyRepeat);
}

/*!
    Simulate a key release event, using the given \a key and \a mod.
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::keyRelease(Qt::Key key, Qt::KeyboardModifiers mod,
        QtUiTest::InputOption opt)
{
#ifdef QTUITEST_RACE_TEST
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestKeyEvent event = {QtUiTestRelease, key, mod, opt};
        d->pendingKeyEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextKeyEvent()));
        return;
    }
#endif
    QTWOptStack st(d, opt);
    d->input.keyRelease(key, mod);
}

/*!
    Simulate a key click event, using the given \a key and \a mod.
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::keyClick(Qt::Key key, Qt::KeyboardModifiers mod,
        QtUiTest::InputOption opt)
{
#ifdef QTUITEST_RACE_TEST
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestKeyEvent event = {QtUiTestClick, key, mod, opt};
        d->pendingKeyEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextKeyEvent()));
        return;
    }
#endif
    QTWOptStack st(d, opt);
    d->input.keyClick(key, mod);
}

#ifdef QTUITEST_RACE_TEST
void QtUiTestWidgetsPrivate::_q_postNextMouseEvent()
{
    QtUiTestMouseEvent const event = pendingMouseEvents.takeAt(0);
    QTWOptStack st(this, event.opt);
    if (event.type == QtUiTestPress)
        input.mousePress(event.pos, event.state);
    else if (event.type == QtUiTestRelease)
        input.mouseRelease(event.pos, event.state);
    else if (event.type == QtUiTestClick)
        input.mouseClick(event.pos, event.state);
}

void QtUiTestWidgetsPrivate::_q_postNextKeyEvent()
{
    QtUiTestKeyEvent const event = pendingKeyEvents.takeAt(0);
    QTWOptStack st(this, event.opt);
    if (event.type == QtUiTestPress)
        input.keyPress(event.key, event.mod);
    else if (event.type == QtUiTestRelease)
        input.keyRelease(event.key, event.mod);
    else if (event.type == QtUiTestClick)
        input.keyClick(event.key, event.mod);
}
#endif

/*!
    Set or clear the specified \a option for subsequent simulated input
    events.  The option is set if \a on is true, otherwise it is cleared.
*/
void QtUiTestWidgets::setInputOption(QtUiTest::InputOption option, bool on)
{
    if (on)
        d->inputOptions |= option;
    else
        d->inputOptions &= (~option);
    qLog(QtUitest) << "set input options to" << d->inputOptions;
}

/*!
    Returns true if \a option is currently set.
*/
bool QtUiTestWidgets::testInputOption(QtUiTest::InputOption option) const
{
    return (option == d->inputOptions)
        || (option & d->inputOptions);
}

#include "moc_qtuitestwidgets_p.cpp"

