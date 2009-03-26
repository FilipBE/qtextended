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
#include "qtsingleapplication.h"
#include <qwidget.h>

class QtSingletonPrivate
{
public:
    QString id;
};

/*!
    \class QtSingleApplication qtsingleapplication.h
    \brief The QtSingleApplication class provides an API to detect and
    communicate with running instances of an application.

    This class allows you to create applications that cannot have
    multiple instances running on the same machine for the same user.

    To use the QtSingleApplication class you must provide an ID string
    that it unique on the system you run the application on. Typical
    IDs are the name of the application and the application vendor, or
    a string representation of a \link QUuid UUID\endlink.

    The application should create the QtSingleApplication object very
    early in the startup phase, and try to send a message or call
    isRunning() to find out if an instance of this application is
    already running.

    If an instance is already running, this application instance
    should terminate. Otherwise the application should call
    initialize() immediately, and continue with the initialization of
    the application user interface before entering the event loop with
    exec(). The messageReceived() signal will be emitted when the
    application receives messages from another instance of the same
    application. If a message is received it might be helpful to the
    user to raise the application so that it becomes visible.

    Here's an example that shows how to convert an existing
    application to us QtSingleApplication. It is very simple and does
    not make use of all QtSingleApplication's functionality(see the
    examples for that).

    \code
    // Original
    int main(int argc, char **argv)
    {
	QApplication app(argc, argv);

	MyMainWidget mmw;
	app.setMainWidget(&mmw);

	mmw.show();
	return app.exec();
    }

    // Single instance
    int main(int argc, char **argv)
    {
	QtSingleApplication app("MySingleInstance", argc, argv);

	if (app.sendMessage("Do I exist?"))
	    return 0;

        app.initialize();

	MyMainWidget mmw;
	app.setMainWidget(&mmw);

	QObject::connect(&app, SIGNAL(messageReceived(QString)),
		&app, SLOT(activateMainWidget()));

	mmw.show();
	return app.exec();
    }
    \endcode

    Once this QtSingleApplication instance is destroyed(for example,
    when the user quits), when the user next attempts to run the
    application this instance will not, of course, be encountered.
*/

/*!
    Creates a QtSingleApplication object with the identifier \a id. \a
    argc, \a argv and \a type are passed on to the QAppliation
    constructor.

    There can only be one QtSingleApplication object(and since there
    can only be one QApplication object you do not need to create
    another QApplication object yourself).

    \warning On X11 type can not be QApplication::Tty.

*/
QtSingleApplication::QtSingleApplication(const QString &id, int &argc, char **argv, Type type)
    : QApplication(argc, argv, type)
{
#ifdef Q_WS_X11
    Q_ASSERT_X(type != Tty, "QtSingleApplication::QtSingleApplication",
               "QApplication::Tty cannot be used with QtSingleApplication on X11");
#endif
    d = new QtSingletonPrivate;
    d->id = id;

    sysInit();
}

#ifdef Q_WS_X11

/*!
    Creates a QtSingleApplication object, given an already open display
    \a dpy. Uses the identifier \a id. \a argc and \a argv are
    passed on to the QAppliation constructor. If \a visual and \a colormap
    are non-zero, the application will use those as the default Visual and
    Colormap contexts.

    There can only be one QtSingleApplication object(and since there
    can only be one QApplication object you do not need to create
    another QApplication object yourself).

    \warning Qt only supports TrueColor visuals at depths higher than 8
    bits-per-pixel.

    This is available only on X11.
*/
QtSingleApplication::QtSingleApplication(Display* dpy, const QString &id, int argc, char **argv,
                                         Qt::HANDLE visual, Qt::HANDLE colormap)
    : QApplication(dpy, argc, argv, visual, colormap)
{
    d = new QtSingletonPrivate;
    d->id = id;

    sysInit();
}

#endif // Q_WS_X11


/*!
    Destroys the object, freeing all allocated resources.

    If the same application is started again it will not find this
    instance.
*/
QtSingleApplication::~QtSingleApplication()
{
    sysCleanup();

    delete d;
}

/*!
    Returns the identifier of this singleton object.
*/
QString QtSingleApplication::id() const
{
    return d->id;
}

/*!
    Calls QWidget::setActiveWindow() on this application's
    mainWidget(). This function does nothing if no main widget has
    been set.

    This slot is typically connected to the messageReceived() signal.

    \sa messageReceived()
*/
void QtSingleApplication::activateMainWidget()
{
    QWidgetList widgets = QApplication::allWidgets();
    if (!widgets.isEmpty()) {
        QWidget *w = widgets.at(0);
        w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
        w->raise();
        w->activateWindow();
    }
}

/*! \fn bool QtSingleApplication::isRunning() const

    Returns true if another instance of this application has called
    initialize(); otherwise returns false.

    This function does not find instances of this application that are
    being run by a different user.

    \sa initialize()
*/

/*!
    \fn void QtSingleApplication::initialize(bool activate)

    Once this function has been called, this application instance
    becomes "visible" to other instances. This means that if another
    instance is started(by the same user), and calls isRunning(), the
    isRunning() function will return true to that other instance,
    which should then quit, leaving this instance to continue.

    If \a activate is true(the default) the messageReceived() signal
    will be connected to the activateMainWidget() slot.
*/

/*!
    \fn bool QtSingleApplication::sendMessage(const QString& message, int timeout)

    Tries to send the text \a message to the currently running
    instance. The QtSingleApplication object in the running instance
    will emit the messageReceived() signal when it receives the
    message.

    This function returns true if the message has been sent to, and
    processed by, the current instance. If there is no instance
    currently running, or if the running instance fails to process the
    message within \a timeout milliseconds this function return false.

    Note that on X11 systems the \a timeout parameter is ignored.

    \sa messageReceived()
*/

/*!
    \fn void QtSingleApplication::messageReceived(const QString& message)

    This signal is emitted when the current instance receives a \a
    message from another instance of this application.

    This signal is typically connected to the activateMainWidget()
    slot.

    \sa activateMainWidget(), initialize()
*/

