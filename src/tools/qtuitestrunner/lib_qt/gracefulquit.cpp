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

#include "gracefulquit.h"

#ifdef Q_OS_UNIX

#include <QTimer>
#include <QSocketNotifier>

#include <signal.h>
#include <errno.h>
#include <unistd.h>

int graceful_quit_fds[2];
sighandler_t graceful_quit_old_handlers[32];
int graceful_quit_signal;

void graceful_quit_alarm_handler(int)
{
    ::kill(::getpid(), graceful_quit_signal);
    qFatal("Terminated.");
}

void graceful_quit_signal_handler(int sig)
{
    graceful_quit_signal = sig;

    // Replace the old handlers.
    signal(SIGINT,  graceful_quit_old_handlers[SIGINT]);
    signal(SIGQUIT, graceful_quit_old_handlers[SIGQUIT]);
    signal(SIGTERM, graceful_quit_old_handlers[SIGTERM]);
    signal(SIGHUP,  graceful_quit_old_handlers[SIGHUP]);

    /*
       Write to the monitored pipe to wake up any watching socket notifiers.
    */
    char byte = 0x01;
    write(graceful_quit_fds[1], &byte, sizeof(byte));
}

/*
    This class ensures that the process really dies if it fails to gracefully
    quit (e.g., it's stuck in a nested event loop).
*/
class QKillTimer : public QObject
{
Q_OBJECT
public:
    QKillTimer(QSocketNotifier*);

private slots:
    void on_activated();

private:
    QSocketNotifier* m_notifier;
};

QKillTimer::QKillTimer(QSocketNotifier* parent)
    : QObject(parent),
      m_notifier(parent)
{
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(on_activated()));
}

void QKillTimer::on_activated()
{
    m_notifier->setEnabled(false);
    signal(SIGALRM, graceful_quit_alarm_handler);
    ::alarm(1);
}

/*!
    \class GracefulQuit
    \inpublicgroup QtUiTestExtension
    \brief The GracefulQuit class provides a method for quitting applications
           gracefully when a terminating unix signal is received.
*/

/*!
    Installs a graceful quit handler on \a object.

    \a object must have a quit() slot, such as QCoreApplication.
    When a Unix signal occurs which would normally cause a termination, such
    as SIGINT or SIGTERM, the quit() slot will be invoked on \a object.

    Example:
    \code
    int main(int argc, char** argv)
    {
        QApplication app(argc, argv);

        // Ensure we shut down sanely upon receiving SIGINT etc.
        GracefulQuit::install(&app);

        // Do regular stuff...
        MyWidget widget;
        widget.show();

        // If we get a signal, all objects will be destroyed as they should be.
        return app.exec();
    }
    \endcode
*/
void GracefulQuit::install(QObject* object)
{
    Q_ASSERT(object);

    /*
       Create a pipe for internal use.
       When the pipe is written into, that means we've received an exit
       signal.
    */
    static bool made_pipe = false;
    if (!made_pipe) {
        if (-1 == pipe(graceful_quit_fds)) {
            qWarning("GracefulQuit::install(): pipe() failed: %s", strerror(errno));
            return;
        } else {
            made_pipe = true;

            /*
                Install the signal handler.
            */
            graceful_quit_old_handlers[SIGINT]  = signal(SIGINT,  graceful_quit_signal_handler);
            graceful_quit_old_handlers[SIGQUIT] = signal(SIGQUIT, graceful_quit_signal_handler);
            graceful_quit_old_handlers[SIGTERM] = signal(SIGTERM, graceful_quit_signal_handler);
            graceful_quit_old_handlers[SIGHUP]  = signal(SIGHUP,  graceful_quit_signal_handler);
        }
    }

    /*
       Watch the readable fd.  When it can be read from, it's time for us
       to quit.
    */
    static QSocketNotifier* sn = new QSocketNotifier
        (graceful_quit_fds[0], QSocketNotifier::Read, object);

    // Give the application 1 second to gracefully quit.
    static QKillTimer* kt = new QKillTimer(sn);
    Q_UNUSED(kt);

    if (!object->connect(sn, SIGNAL(activated(int)), SLOT(quit())))
        Q_ASSERT(0);

}

#else
void GracefulQuit::install(QObject*)
{}
#endif

#include "gracefulquit.moc"

