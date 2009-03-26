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

#include "applicationlauncher.h"
#include <qtopianamespace.h>
#include <QEvent>
#include <QFile>
#include <QProcess>
#include <QTimer>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#else
#include <qcopchannel_x11.h>
#endif
#include <QValueSpaceObject>
#include "qcopfile.h"
#include <QtopiaApplication>
#include <unistd.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaabstractservice.h>
#include <qtopialog.h>
#ifdef Q_WS_QWS
#include <QWSServer>
#endif
#include <QContent>


/*!
  \class ApplicationIpcRouter::RouteDestination
    \inpublicgroup QtBaseModule
  \brief The RouteDestination class represents an IPC route destination.
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces

  A route destination allows an ApplicationTypeLauncher to add a manual
  transport route to the ApplicationIpcRouter instance.  Manual transport routes
  are usefull to adapt the system's primary IPC transport to other transport
  models.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  An overview of the application launcher mechanism and the role the
  RouteDestination plays in it is given in the documentation of the
  ApplicationLauncher class.
 */

/*!
  \fn ApplicationIpcRouter::RouteDestination::~RouteDestination()

  \internal
*/

/*!
  \fn void ApplicationIpcRouter::RouteDestination::routeMessage(const QString &appName, const QString &message, const QByteArray &data)

  Invoked by the system's IpcRouter to deliver an IPC message.  \a appName is
  set to the name of the application the message should be delivered to (the
  "destination").  \a message and \a data represent the message itself.
 */

/*!
  \class ApplicationIpcRouter
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces
  \brief The ApplicationIpcRouter class provides an interface through which
         ApplicationTypeLauncher instances to control IPC message routing.

  The ApplicationIpcRouter provides a Qt Extended Server Task interface.  Qt Extended Server Tasks are documented in full in the QtopiaServerApplication class
  documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  Generally only one task in the system implements the ApplicationIpcRouter task
  interface.  This task is known as the system's IPC Router and is broadly
  responsible for coupling the configured IPC system into Qtopia.  Currently
  Qt Extended only supports the both QCop IPC system, but could conceivably
  support other transport systems in the future.

  An overview of the application launcher mechanism and the role the
  ApplicationIpcRouter plays in it is given in the documentation of the
  ApplicationLauncher class.
 */

/*!
  \fn void ApplicationIpcRouter::addRoute(const QString &app, RouteDestination *dest)

  Add a manual transport route for \a app to \a dest to the router.  An IPC
  Router \bold {must} be able to deliver any pending messages to an installed
  route \bold {immediately}.
 */

/*!
  \fn void ApplicationIpcRouter::remRoute(const QString &app, RouteDestination *dest)
  Remove a manual transport route for \a app to \a dest from the router.
 */

/*!
  \class ApplicationTypeLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces
  \brief The ApplicationTypeLauncher class provides an interface to control
         a particular application type in the system.

  The ApplicationTypeLauncher provides a Qt Extended Server Task interface.  Qt Extended Server Tasks are documented in full in the QtopiaServerApplication class
  documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  ApplicationTypeLauncher implementers are used by the ApplicationLauncher to
  control a specific type of application.  An overview of the application
  launcher mechanism is given in the documentation of the ApplicationLauncher
  class.
 */

/*!
  \fn ApplicationTypeLauncher::~ApplicationTypeLauncher()

  Destruct the ApplicationTypeLauncher instance.
 */

/*!
  \fn ApplicationTypeLauncher::ApplicationState ApplicationTypeLauncher::applicationState(const QString &application)

  Return the current state of the \a application.
 */

/*!
  \fn bool ApplicationTypeLauncher::canLaunch(const QString &application)

  Return true if this ApplicationTypeLauncher can launch the \a application.
 */

/*!
  \fn void ApplicationTypeLauncher::launch(const QString &application)

  Attempt to launch the \a application.
 */

/*!
  \fn void ApplicationTypeLauncher::kill(const QString &application)

  Kill the \a application.
 */

/*!
  \fn QString ApplicationTypeLauncher::name()

  Return a descriptive name for the ApplicationTypeLauncher type.
 */

/*!
  \enum ApplicationTypeLauncher::TerminationReason

  Represents the reason an application terminated.

  \value Normal The application terminated normally.
  \value Killed The application was killed by the system.
  \value FailedToStart The application failed to start.
  \value Crashed The application crashed.
  \value Unknown The application terminated for an unknown reason.
*/

/*!
  \enum ApplicationTypeLauncher::ApplicationState

  Represents the state of an application.

  \value NotRunning The application is not running.
  \value Starting The application is starting.
  \value Running The application is running.
 */


/*!
  \fn void ApplicationTypeLauncher::applicationStateChanged(const QString &application, ApplicationTypeLauncher::ApplicationState state)

  Emitted whenever the \a application state changes.  \a state will be the
  application's new state.
*/

/*!
  \fn void ApplicationTypeLauncher::terminated(const QString &application, ApplicationTypeLauncher::TerminationReason reason)

  Emitted whenever the \a application terminates.  \a reason represents the
  cause of the termination.
*/

/*! 
  \fn void ApplicationTypeLauncher::pidStateChanged(const QString& application, Q_PID pid)

  This signal is used by the memory monitor. If \a pid is empty the Qtopia \a application
  exited; otherwise pid is greater than zero. Most server classes should use the
  terminated() or applicationStateChanged() signal.
*/

/*!
  \class ApplicationTerminationHandler
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces
  \brief The ApplicationTerminationHandler class allows tasks to be notified,
         and possibly filter, when an application terminates.

  The ApplicationTerminationHandler provides a Qt Extended Server Task interface.
  Qt Extended Server Tasks are documented in full in the QtopiaServerApplication
  class documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  When an application terminates, the ApplicationLauncher notifies all tasks, in
  order, that implement the ApplicationTerminationHandler interface.  A task
  that implements the ApplicationTeminationHandler interface can filter the
  termination notification by returning true from its terminated() notification
  method.  Filtering a termination will prevent subsequently ordered tasks from
  being notified.

  Regardless of whether or not a task is filtered, the ApplicationLauncher
  class will still emit the global ApplicationLauncher::applicationTerminated()
  signal, albeit with the \bold filtered parameter set to true.

  Termination handlers can be used to hide the termination of "system"
  applications from the user, or to implement custom termination notices.  The
  QTerminationHandler class, for example, uses a server side termination handler
  to display a more descriptive crash message when an application abnormally
  exits.
 */

/*!
  \fn ApplicationTerminationHandler::ApplicationTerminationHandler(QObject *parent = 0)

  Construct a new ApplicationTerminationHandler interface with the given
  \a parent.
 */

/*!
  \fn bool ApplicationTerminationHandler::terminated(const QString &appName,
                            ApplicationTypeLauncher::TerminationReason reason)

  Invoked when an application terminates.  \a appName is the name of the
  application that terminated and \a reason is the way in which it terminated.

  Returning false from this method allows the termination notification to
  continue to propagate to other ApplicationTerminationHandler instances.
  Returning true stops further propagation.  Implementors should only return
  true if they intend to notify the user of the termination, or if they have
  determined that notification is not required.
 */

class LegacyLauncherService : public QtopiaAbstractService
{
    Q_OBJECT

  public:
    LegacyLauncherService( QObject *parent )
        : QtopiaAbstractService( "Launcher", parent )
        { publishAll(); }

  public:
    ~LegacyLauncherService();

  public slots:
    void execute( const QString& app );
    void execute( const QString& app, const QString& document );
    void kill ( const QString &app );
};

/*!
  \class QtopiaServerApplicationLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The QtopiaServerApplicationLauncher class acts as a proxy for the Qt Extended Server within the application launcher framework.

  The QtopiaServerApplicationLauncher provides a Qt Extended Server Task.  Qt Extended Server Tasks are documented in full in the QtopiaServerApplication class
  documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \table
  \row \o Task Name \o QtopiaServerApplicationLauncher
  \row \o Interfaces \o ApplicationTypeLauncher
  \row \o Services \o None
  \endtable

  To enable the Qt Extended server itself to receive IPC messages, it must be known
  to the application launcher framework.  The QtopiaServerApplicationLauncher
  task provides this.

  It is \bold essential that the QtopiaServerApplicationLauncher is the first
  ordered ApplicationTypeLauncher provider.
  */
QTOPIA_TASK(QtopiaServerApplicationLauncher, QtopiaServerApplicationLauncher);
QTOPIA_TASK_PROVIDES(QtopiaServerApplicationLauncher, ApplicationTypeLauncher);

// define QtopiaServerApplicationLauncher
/*! \internal */
QtopiaServerApplicationLauncher::QtopiaServerApplicationLauncher()
{
}

/*! \internal */
bool QtopiaServerApplicationLauncher::canLaunch(const QString &app)
{
    return QtopiaApplication::applicationName() == app;
}

/*! \internal */
void QtopiaServerApplicationLauncher::launch(const QString &app)
{
    Q_ASSERT(canLaunch(app));
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if (r)
        r->addRoute(app,this);
    emit applicationStateChanged(app, Starting);
    emit applicationStateChanged(app, Running);
}

/*!
  \internal
  The kill function is a noop in this subclass because the
  server application must never be killed.
 */
void QtopiaServerApplicationLauncher::kill(const QString &)
{
    // noop.
}

/*! \internal */
QtopiaServerApplicationLauncher::ApplicationState
QtopiaServerApplicationLauncher::applicationState(const QString &app)
{
    Q_ASSERT(canLaunch(app));
    Q_UNUSED(app);
    return Running;
}

/*! \internal */
void QtopiaServerApplicationLauncher::routeMessage(const QString &app,
                                                   const QString &message,
                                                   const QByteArray &data)
{
    QCopFile::writeQCopMessage(app, message, data);
    QtopiaChannel::send("QPE/pid/" + QString::number(::getpid()),
                        "QPEProcessQCop()");
}

// declare ExeApplicationLauncherPrivate
struct ExeApplicationLauncherPrivate {
    struct RunningProcess {
        RunningProcess() : proc(0) { }
        RunningProcess(const QString& _a, QProcess* _p,
                       ApplicationTypeLauncher::ApplicationState _s)
            : app(_a),
              proc(_p),
              state(_s),
              pidChannelOpen(false),
              m_killed(false),
              m_pid(proc->pid()) { }
        ~RunningProcess() {
            if (proc) {
                proc->disconnect();
                proc->deleteLater();
            }
        }

        void kill() {
            if (proc) {
                proc->kill();
                m_killed = true;
            }
        }

        bool killed() const { return m_killed; }

        QString app;
        QProcess* proc;
        ApplicationTypeLauncher::ApplicationState state;
        bool pidChannelOpen;
        bool m_killed;
        int m_pid;
    };

    RunningProcess *runningProcess(const QString &);
    RunningProcess *runningProcess(int);
    RunningProcess *runningProcess(QProcess *);

    QMap<QString, RunningProcess *> m_runningProcesses;
};

/*!
  \class ExeApplicationLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The ExeApplicationLauncher class simplifies implementing ApplicationTypeLauncher for process based applications.

  The ExeApplicationLauncher is helpful for writing ApplicationTypeLauncher
  implementations that are slight variations on the simple executable process
  model.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  Many application types are just different ways of starting a QtopiaApplication
  application.  For example, the SimpleExeApplicationLauncher and the
  QuickExeApplicationLauncher types both start external processes that
  instantiate a QtopiaApplication instance.  The ExeApplicationLauncher
  encapsulates the commonality between these types of application launchers.

  It is the responsibility of derived classes to respond to the canLaunch() and
  launch() methods of the ApplicationTypeLauncher interface.  Once the derived
  class has brung a process to the state where it is launching, it should call
  addStartingApplication() to pass ownership of the process to the
  ExeApplicationLauncher.  The ExeApplicationLauncher class operates under
  the following assumptions:
  \list 1
  \i The application will send an \c {available(QString,int)} message to the
     \c {QPE/QtopiaApplication} channel when it has completed startup.
  \i The process, and thus the QProcess instance, will terminate normally when
     it is done.
  \endlist
 */

// define ExeApplicationLauncher
/*!
  Construct a new ExeApplicationLauncher instance.
 */
ExeApplicationLauncher::ExeApplicationLauncher()
    : d(new ExeApplicationLauncherPrivate)
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/QtopiaApplication", this);
    connect(channel,
            SIGNAL(received(QString,QByteArray)),
            this,
            SLOT(qtopiaApplicationChannel(QString,QByteArray)));

#ifdef Q_WS_X11
    QCopServer *qwsServer = QCopServer::instance();
#endif
    connect(qwsServer,
            SIGNAL(newChannel(QString)),
            this,
            SLOT(newChannel(QString)));
}

/*!
  Destroy the ExeApplicationLauncher instance.
 */
ExeApplicationLauncher::~ExeApplicationLauncher()
{
    delete d;
}

/*!
  Add a new process, \a proc, for the applications \a app to
  the ExeApplicationLauncher. The ExeApplicationLauncher will
  take ownership of the process.
  */
void ExeApplicationLauncher::addStartingApplication(const QString &app,
                                                    QProcess *proc)
{
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(appExited(int,QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(appError(QProcess::ProcessError)));

    d->m_runningProcesses.insert(app,
         new ExeApplicationLauncherPrivate::RunningProcess(app,proc,Starting));
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if (r)
        r->addRoute(app,this);
    emit applicationStateChanged(app,Starting);
}

/*!
  Returns true if \a app is already being managed by the
  ExeApplicationLauncher (that is, addStartingApplication()
  has been called), otherwise returns false.
  */
bool ExeApplicationLauncher::isRunning(const QString& app)
{
    return d->m_runningProcesses.contains(app);
}

/*! \internal */
void ExeApplicationLauncher::kill(const QString& app)
{
    ExeApplicationLauncherPrivate::RunningProcess* rp = d->runningProcess(app);
    if (!rp)
        return;
    qLog(OOM) << "kill():" << app;
    /*
      When a process is killed, the out-of-memory manager
      must remove it from its data structures.
     */
    emit pidStateChanged(rp->app, 0 );
    rp->kill();
}

/*! \internal */
ExeApplicationLauncher::ApplicationState
ExeApplicationLauncher::applicationState(const QString& app)
{
    ExeApplicationLauncherPrivate::RunningProcess* rp = d->runningProcess(app);
    if (!rp)
        return NotRunning;
    else
        return rp->state;
}

/*! \internal */
void ExeApplicationLauncher::routeMessage(const QString& app,
                                          const QString& message,
                                          const QByteArray& data)
{
    ExeApplicationLauncherPrivate::RunningProcess* rp = d->runningProcess(app);
    Q_ASSERT(rp);

    QCopFile::writeQCopMessage(app, message, data);
    QtopiaChannel::send("QPE/pid/" + QString::number(rp->proc->pid()),
                        "QPEProcessQCop()");
}

/*!
  \internal
 */
void ExeApplicationLauncher::appExited(int , QProcess::ExitStatus)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    Q_ASSERT(proc);

    if ( proc->exitCode() != 0  && proc->error() != QProcess::FailedToStart )
    {
        appError(QProcess::Crashed);
        return;
    }

    ExeApplicationLauncherPrivate::RunningProcess* rp =
        d->runningProcess(proc);
    Q_ASSERT(rp);
    Q_ASSERT(NotRunning != rp->state);

    ApplicationIpcRouter* r = qtopiaTask<ApplicationIpcRouter>();
    if (r)
        r->remRoute(rp->app,this);

    if (Starting == rp->state && proc->error() == QProcess::FailedToStart ) {
        rp->state = NotRunning;
        emit terminated(rp->app, FailedToStart);
        emit applicationStateChanged(rp->app, NotRunning);
    }
    else {
        rp->state = NotRunning;
        if (rp->killed())
            emit terminated(rp->app,Killed);
        else
            emit terminated(rp->app,Normal);
        emit applicationStateChanged(rp->app, NotRunning);
    }

    qLog(OOM) << "appExited():" << rp->app;
    /*
      When a process exits, the out-of-memory manager
      must remove it from its data structures.
     */
    emit pidStateChanged(rp->app, 0);
    d->m_runningProcesses.remove(rp->app);
    delete rp;
}

/*! \internal */
void ExeApplicationLauncher::appError(QProcess::ProcessError error)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    Q_ASSERT(proc);

    ExeApplicationLauncherPrivate::RunningProcess* rp =
        d->runningProcess(proc);
    Q_ASSERT(rp);
    Q_ASSERT(NotRunning != rp->state);

    TerminationReason reason = Unknown;
    switch(error) {
        case QProcess::FailedToStart:
            reason = FailedToStart;
            break;
        case QProcess::Crashed:
            reason = Crashed;
            break;
        default:
            break;
    };

    rp->state = NotRunning;
    /*
      When a process dies because of an error, the
      out-of-memory manager must remove it from its
      data structures.
     */
    emit pidStateChanged(rp->app, 0);
    d->m_runningProcesses.remove(rp->app);

    {
        QtopiaIpcEnvelope e(QLatin1String("QPE/QtopiaApplication"),
                            QLatin1String("notBusy(QString)") );
        e << rp->app;
    }

    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if (r)
        r->remRoute(rp->app,this);

#ifndef QT_NO_SXE
    QValueSpaceItem sxeVsi( "/Sxe/killedPids", this );
    QList<QVariant> killedPids = sxeVsi.value().toList();

    if (rp->killed() || (killedPids.contains(QVariant(rp->m_pid))))
    {
#else
    if(rp->killed())
    {
#endif
        reason = Killed;
    }

    emit terminated(rp->app,reason);
    emit applicationStateChanged(rp->app,NotRunning);
    delete rp;
}

/*! \internal */
void ExeApplicationLauncher::qtopiaApplicationChannel(const QString &message,
                                                      const QByteArray &data)
{
    if (message == "available(QString,int)") {
        QDataStream ds(data);
        QString name;
        int pid;
        ds >> name >> pid;
        ExeApplicationLauncherPrivate::RunningProcess *rp =
            d->runningProcess(name);
        if (rp && rp->proc->pid() == pid) {
            rp->state = Running;
            /*
              Tell the out-of-memory manager about this new
              process that is now running.
             */
            emit pidStateChanged(rp->app, pid);
            emit applicationStateChanged(rp->app,Running);
            return;
        }
    }
}

/*! \internal */
void ExeApplicationLauncher::newChannel( const QString& ch )
{
    QString pidChannel = "QPE/pid/"; // No tr
    if (ch.startsWith(pidChannel)) {
        int pid = ch.mid(pidChannel.count()).toInt();
        ExeApplicationLauncherPrivate::RunningProcess *rp =
            d->runningProcess(pid);
        if (rp != 0) {
            rp->pidChannelOpen = true;
        }
    }
}

ExeApplicationLauncherPrivate::RunningProcess*
ExeApplicationLauncherPrivate::runningProcess(const QString &app)
{
    QMap<QString,RunningProcess*>::Iterator i =
        m_runningProcesses.find(app);
    if (i == m_runningProcesses.end())
        return 0;
    else
        return *i;
}

ExeApplicationLauncherPrivate::RunningProcess*
ExeApplicationLauncherPrivate::runningProcess(int pid)
{
    for (QMap<QString,RunningProcess*>::Iterator i=m_runningProcesses.begin();
         i != m_runningProcesses.end();
         ++i)
        {
            if ((*i)->proc->pid() == pid)
                return *i;
        }
    return 0;
}

ExeApplicationLauncherPrivate::RunningProcess*
ExeApplicationLauncherPrivate::runningProcess(QProcess *proc)
{
    for (QMap<QString,RunningProcess*>::Iterator i=m_runningProcesses.begin();
         i != m_runningProcesses.end();
         ++i)
        {
            if ((*i)->proc == proc)
                return *i;
        }
    return 0;
}

// define SimpleExeApplicationLauncher
/*!
  \class SimpleExeApplicationLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The SimpleExeApplicationLauncher class supports launching regular
         QtopiaApplication executables.

  The SystemSuspend provides a Qt Extended Server Task.  Qt Extended Server Tasks are
  documented in full in the QtopiaServerApplication class documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \table
  \row \o Task Name \o SimpleExeApplicationLauncher
  \row \o Interfaces \o ApplicationTypeLauncher
  \row \o Services \o None
  \endtable

  The SimpleExeApplicationLauncher class provides the ApplicationTypeLauncher
  implementation for simple, executable based applications.  It also
  (implicitly) doubles as the fallback for the QuickExeApplicationLauncher.

  If the application requested is an absolute path, it is run as is.  Otherwise,
  the paths returned by the Qtopia::installPaths() method are searched for
  \c {bin/<application name>}.
*/
QTOPIA_TASK(SimpleExeApplicationLauncher, SimpleExeApplicationLauncher);
QTOPIA_TASK_PROVIDES(SimpleExeApplicationLauncher, ApplicationTypeLauncher);

/*!
  Constructs a new SimpleExeApplicationLauncher instance.
 */
SimpleExeApplicationLauncher::SimpleExeApplicationLauncher()
: d(0)
{
}

/*!
  Destroys the SimpleExeApplicationLauncher instance.
 */
SimpleExeApplicationLauncher::~SimpleExeApplicationLauncher()
{
}

/*! \internal */
bool SimpleExeApplicationLauncher::canLaunch(const QString &app)
{

    // Check whether the executable exists
    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii)
        if(QFile::exists(exes.at(ii)))
            return true;

    return false;
}

/*! \internal */
void SimpleExeApplicationLauncher::launch(const QString &app)
{
    if(isRunning(app))
        return; // We're already launching/have launched this guy

    Q_ASSERT(canLaunch(app));

    // We need to launch it
    QProcess *proc = new QProcess(this);
    proc->setReadChannelMode(QProcess::ForwardedChannels);
    proc->closeWriteChannel();

    QStringList args;
    args.append("-noshow");

    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if(QFile::exists(exes.at(ii))) {
            if (exes.at(ii).startsWith(Qtopia::packagePath()))
                setupPackageLaunch(exes.at(ii), proc);

            proc->start(exes.at(ii), args);
            addStartingApplication(app, proc);
            return; // Found and done
        }
    }

    delete proc;
}

/*! \internal */
QStringList
SimpleExeApplicationLauncher::applicationExecutable(const QString &app)
{
    if ( app.startsWith( "/" )) // app is a path, not just in standard location
        return QStringList() << app;

    QStringList rv;
    QStringList paths = Qtopia::installPaths();
    for(int ii = 0; ii < paths.count(); ++ii)
        rv.append(paths.at(ii) + "bin/" + app);

    return rv;
}

/*! \internal */
void SimpleExeApplicationLauncher::setupPackageLaunch(const QString &exec, QProcess *proc)
{
    QString packageDir=QFile::symLinkTarget(exec).left( Qtopia::packagePath().length() + 32 );

    //XDG_CONFIG_HOME sets the location to search for user scope QSettings files
    //(see qsettings source file)
    QStringList env = QProcess::systemEnvironment();
    env.append( QString("XDG_CONFIG_HOME=") + packageDir + "/Settings" );

    QRegExp rx("^LD_LIBRARY_PATH.*");
    int idx = env.indexOf( rx );
    if ( idx <= -1 )
        env.append( QString("LD_LIBRARY_PATH") + "=" + packageDir + "/lib" );
    else if ( idx > -1 )
        env.replace( idx, env.at(idx) + ":" + packageDir + "/lib" );

    proc->setEnvironment( env );

    proc->setWorkingDirectory( packageDir );
}

// declare BuiltinApplicationLauncherPrivate
struct BuiltinApplicationLauncherPrivate
{
    QMap<QString, QWidget *> runningApplications;
    QMap<QByteArray, BuiltinApplicationLauncher::BuiltinFunc> builtins;
};
Q_GLOBAL_STATIC(BuiltinApplicationLauncherPrivate, bat);

/*!
  \class BuiltinApplicationLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The BuiltinApplicationLauncher class supports launching simple
         applications that run inside the Qt Extended Server process.

  The BuiltinApplicationLauncher provides a Qt Extended Server Task.  Qt Extended Server
  Tasks are documented in full in the QtopiaServerApplication class
  documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \table
  \row \o Task Name \o BuiltinApplicationLauncher
  \row \o Interfaces \o ApplicationTypeLauncher
  \row \o Services \o None
  \endtable

  The BuiltinApplicationLauncher class provides the ApplicationTypeLauncher
  implementation for simple applications compiled into the Qt Extended Server -
  known as builtin applications.

  A builtin application must consist of a single, toplevel widget.  Generally
  builtins do not implement services, but they may.  If multiple builtins
  implement the same service, or the Qt Extended Server and a builtin implements the
  same service, the behaviour is undefined.

  Adding a builtin application is a simple case of linking it into the Qt Extended Server, and adding a QTOPIA_SIMPLE_BUILTIN() macro to your code.  The
  QTOPIA_SIMPLE_BUILTIN() macro registers the builtin application with the
  BuiltinApplicationLauncher class.  The macro takes the application name,
  and a simple static function that "launches" the application and returns a
  QWidget pointer as paramters.

  \i {Note:} Builtins may not currently implement services.  This functionality is
  planned for future versions of Qtopia.
 */

/*!
  \typedef BuiltinApplicationLauncher::BuiltinFunc

  The BuiltinFunc provides a type definition that the static function used to
  create a builtin application must conform to.  The exact specification is:
  \code
  typdef QWidget *(*BuiltinFunc)()
  \endcode
 */

/*!
  \macro QTOPIA_SIMPLE_BUILTIN(ApplicationName, createFunc)
  \relates BuiltinApplicationLauncher

  Add a new builtin application to the server.  \a ApplicationName must be the
  name of the application and \a createFunc a static function, of type
  BuiltinApplicationLauncher::BuiltinFunc, that creates the applications.  For
  example, the following code adds a simple builtin named "TestBuiltin" to the
  server.
  \code
    class TestBuiltinWindow : public QWidget
    {
     // ...
    };
    static QWidget *testBuiltinFunc()
    {
        static QPointer<TestBuiltinWindow> w = 0
        if (!w)
            w = new TestBuiltinWindow(0);

        return w;
    }
    QTOPIA_SIMPLE_BUILTIN(TestBuiltin, testBultinFunc);
  \endcode

    If the builtin doesn't return a widget the system assumes that the builtin application
    provides a background task. Therefore any UI builtin must return the widget.

    Also if the function keeps a QWidget pointer it must ensure that the pointer is valid.
    This is best ensured by wrapping the widget with a QPointer (see above) since the
    BuiltinApplicationLauncher may delete the widget when it is told to kill the builtin
    application.

*/
/*!
  Constructs a new BuiltinApplicationLauncher instance.
 */
BuiltinApplicationLauncher::BuiltinApplicationLauncher()
: d(0)
{
    vso = new QValueSpaceObject("/System/Applications", this);
}

/*!
  Destroys the BuiltinExeApplicationLauncher instance.
 */
BuiltinApplicationLauncher::~BuiltinApplicationLauncher()
{
}

/*! \internal */
void BuiltinApplicationLauncher::routeMessage(const QString &app,
                                          const QString &msg,
                                          const QByteArray &data)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;

    QMap<QString, QWidget *>::Iterator iter = p->runningApplications.find(app);
    if(iter == p->runningApplications.end()) return; // No app

    QWidget *w = *iter;
    if(!w) return; // No messages we can interpret

    if("raise()" == msg) {
        w->showMaximized();
        w->raise();
        w->activateWindow();
        vso->setAttribute(app+"/Tasks/UI", true);
        QtopiaIpcEnvelope env(QLatin1String("QPE/QtopiaApplication"), QLatin1String("appRaised(QString)"));
        env << app;
    } else if("close()" == msg) {
        w->close();
    } else if("setDocument(QString)" == msg && w->metaObject()->indexOfMethod("setDocument(QString)") != -1) {
        QString document;
        QDataStream stream( data );
        stream >> document;
        QMetaObject::invokeMethod(w, "setDocument", Q_ARG(QString, document));
        w->showMaximized();
        w->raise();
    }
}

/*!
    \reimp
*/
bool BuiltinApplicationLauncher::eventFilter(QObject* obj, QEvent *e)
{
    if (e->type() == QEvent::Show || e->type() == QEvent::Hide)
        if (obj->isWidgetType()) {
            BuiltinApplicationLauncherPrivate *p = bat();
            QWidget *wid = static_cast<QWidget *>(obj);
            for(QMap<QString, QWidget *>::Iterator iter = p->runningApplications.begin();
                    iter != p->runningApplications.end();
                    ++iter) {
                if(*iter == wid) {
                    vso->setAttribute(iter.key()+"/Tasks/UI", (e->type() == QEvent::Show)?true:false);
                    return ApplicationTypeLauncher::eventFilter(obj, e);
                }
            }
        }

    return ApplicationTypeLauncher::eventFilter(obj, e);
}

/*! \internal */
void BuiltinApplicationLauncher::appDestroyed(QObject *obj)
{
    Q_ASSERT(obj->isWidgetType());
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;
    QWidget *wid = static_cast<QWidget *>(obj);

    for(QMap<QString, QWidget *>::Iterator iter = p->runningApplications.begin();
            iter != p->runningApplications.end();
            ++iter) {
        if(*iter == wid) {
            // Found!
            emit terminated(iter.key(), Normal);
            emit applicationStateChanged(iter.key(), NotRunning);
            ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
            if(r) r->remRoute(iter.key(), this);
            vso->removeAttribute(iter.key()+"/Tasks/UI");
            p->runningApplications.erase(iter);
            return;
        }
    }
    Q_ASSERT(!"Couldn't find destroyed application.");
}

/*! \internal */
BuiltinApplicationLauncher::ApplicationState
BuiltinApplicationLauncher::applicationState(const QString &app)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return NotRunning;

    if(p->runningApplications.contains(app))
        return Running;
    else
        return NotRunning;
}

/*! \internal */
bool BuiltinApplicationLauncher::canLaunch(const QString &app)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if (!p)
        return false;

    return p->builtins.contains(app.toAscii());
}

/*! \internal */
void BuiltinApplicationLauncher::launch(const QString &app)
{
    Q_ASSERT(canLaunch(app));

    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;

    if(p->runningApplications.contains(app)) return; // Already running

    QMap<QByteArray, BuiltinApplicationLauncher::BuiltinFunc>::Iterator iter =
        p->builtins.find(app.toAscii());
    Q_ASSERT(iter != p->builtins.end());

    QWidget* wid = (*iter)();
    if (!wid) {
        // Non UI builtin
        // Starts then stops
        p->runningApplications.insert(app,0);
        emit applicationStateChanged(app,Starting);
        emit applicationStateChanged(app,Running);
        emit terminated(app, Normal);
        emit applicationStateChanged(app,NotRunning);
        p->runningApplications.remove(app);
    }
    else {
        // UI builtin
        p->runningApplications.insert(app,wid);
        QObject::connect(wid, SIGNAL(destroyed(QObject*)),
                         this, SLOT(appDestroyed(QObject*)));
        emit applicationStateChanged(app,Starting);
        emit applicationStateChanged(app,Running);
        ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
        if (r)
            r->addRoute(app, this);
        wid->installEventFilter(this);
        if (wid->isHidden())
            wid->deleteLater(); // Shutdown
    }
}

/*! \internal */
void BuiltinApplicationLauncher::kill(const QString &app)
{
    BuiltinApplicationLauncherPrivate* p = bat();
    if (!p)
        return;

    QMap<QString, QWidget *>::Iterator iter = p->runningApplications.find(app);
    if (iter != p->runningApplications.end())
        (*iter)->deleteLater();
        //(*iter)->close();
}

/*! \internal */
void BuiltinApplicationLauncher::install(const char *_name, BuiltinFunc func)
{
    QByteArray name(_name);
    if (name.isEmpty())
        return;

    BuiltinApplicationLauncherPrivate* p = bat();
    if(!p)
        return;

    p->builtins.insert(name,func);
}

QTOPIA_TASK(BuiltinApplicationLauncher, BuiltinApplicationLauncher);
QTOPIA_TASK_PROVIDES(BuiltinApplicationLauncher, ApplicationTypeLauncher);

// declare ConsoleApplicationLauncherPrivate
struct ConsoleApplicationLauncherPrivate
{
    ConsoleApplicationLauncherPrivate() {
        }

    ~ConsoleApplicationLauncherPrivate()
    {
    }


    struct App {
        App() : process(0) { process = new QProcess(0); }
        ~App()  { process->disconnect(); process->deleteLater(); }
        QString app;
        QProcess *process;
        ConsoleApplicationLauncher::ApplicationState state;
    };

    QMap<QString, App *> apps;

    App *processToApp(QProcess *proc)
    {
        for(QMap<QString, App *>::ConstIterator iter = apps.begin();
            iter != apps.end();
            ++iter) {
            if((*iter)->process == proc)
                return *iter;
        }
        return 0;
    }
};

// define ConsoleApplicationLauncher
/*!
  \class ConsoleApplicationLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The ConsoleApplicationLauncher class supports launching console
         applications.

  The ConsoleApplicationLauncher provides a Qt Extended Server Task.  Qt Extended Server
  Tasks are documented in full in the QtopiaServerApplication class
  documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \table
  \row \o Task Name \o ConsoleApplicationLauncher
  \row \o Interfaces \o ApplicationTypeLauncher
  \row \o Services \o None
  \endtable

  The ConsoleApplicationLauncher class provides the ApplicationTypeLauncher
  implementation for non-graphical, console applications.

  Any Linux executable may be a console application.  Console applications are
  distinguished from regular, GUI applications by the "ConsoleApplication"
  property in their content description being set to "1".

  Console applications are started whenever any application is sent to their
  application channel.  They do not respond to any messages specifically.
  Qt Extended does not try to manage (or understand) the life cycle of console
  applications.  An application is considered "Running" as soon as the
  executable is started (as opposed to regular Qt Extended applications that must
  create the QtopiaApplication object first).
*/

/*!
  Constructs a new ConsoleApplicationLauncher instance.
 */
ConsoleApplicationLauncher::ConsoleApplicationLauncher()
: d(new ConsoleApplicationLauncherPrivate)
{
}

/*!
  Destroys the ConsoleApplicationLauncher instance.
 */
ConsoleApplicationLauncher::~ConsoleApplicationLauncher()
{
    delete d;
    d = 0;
}

/*! \internal */
ConsoleApplicationLauncher::ApplicationState
ConsoleApplicationLauncher::applicationState(const QString &app)
{
    QMap<QString, ConsoleApplicationLauncherPrivate::App *>::Iterator iter =
        d->apps.find(app);
    if(iter == d->apps.end())
        return NotRunning;
    else
        return (*iter)->state;
}

/*! \internal */
bool ConsoleApplicationLauncher::canLaunch(const QString &app)
{
    QContent capp(app,false);
    if (capp.isNull())
        return false;
    return capp.property("ConsoleApplication") == QLatin1String("1");
}

/*! \internal */
void ConsoleApplicationLauncher::launch(const QString &app)
{
    if(d->apps.find(app) != d->apps.end())
        return;

    Q_ASSERT(canLaunch(app));

    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if(QFile::exists(exes.at(ii))) {

            ConsoleApplicationLauncherPrivate::App *capp = new
                ConsoleApplicationLauncherPrivate::App();
            capp->app = app;
            capp->state = Starting;
            capp->process->setReadChannelMode(QProcess::ForwardedChannels);
            capp->process->closeWriteChannel();
            qLog(QtopiaServer) << "Starting" << exes.at(ii);
            capp->process->start(exes.at(ii));

            QObject::connect(capp->process, SIGNAL(started()),
                             this, SLOT(appStarted()));
            QObject::connect(capp->process, SIGNAL(finished(int)),
                             this, SLOT(appExited(int)));
            QObject::connect(capp->process,SIGNAL(error(QProcess::ProcessError)),
                             this,SLOT(appError(QProcess::ProcessError)));

            d->apps.insert(app, capp);

            ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
            if(r) r->addRoute(app, this);

            emit applicationStateChanged(app, Starting);
            return;
        }
    }
}

void ConsoleApplicationLauncher::appStarted()
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ConsoleApplicationLauncherPrivate::App *app = d->processToApp(proc);
    Q_ASSERT(proc);

    app->state = Running;
    emit pidStateChanged(app->app, proc->pid());
    emit applicationStateChanged(app->app, Running);
}

void ConsoleApplicationLauncher::appExited(int)
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ConsoleApplicationLauncherPrivate::App *app = d->processToApp(proc);
    Q_ASSERT(proc);

    Q_ASSERT(Running == app->state);

    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->remRoute(app->app, this);

    app->state = NotRunning;
    emit pidStateChanged(app->app, 0);
    emit terminated(app->app, Normal);
    emit applicationStateChanged(app->app, NotRunning);

    d->apps.remove(app->app);
    delete app;
}

void ConsoleApplicationLauncher::appError(QProcess::ProcessError error)
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ConsoleApplicationLauncherPrivate::App *app = d->processToApp(proc);
    Q_ASSERT(proc);
    Q_ASSERT(NotRunning != app->state);

    TerminationReason reason = Unknown;
    switch(error) {
        case QProcess::FailedToStart:
            reason = FailedToStart;
            break;
        case QProcess::Crashed:
            reason = Crashed;
            break;
        default:
            break;
    };

    app->state = NotRunning;

    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if (r)
        r->remRoute(app->app, this);

    emit pidStateChanged(app->app, 0);
    emit terminated(app->app, reason);
    emit applicationStateChanged(app->app, NotRunning);

    d->apps.remove(app->app);
    delete app;
}

/*! \internal */
QStringList
ConsoleApplicationLauncher::applicationExecutable(const QString &app)
{
    if (app.startsWith( "/" )) // app is a path, not just in standard location
        return QStringList() << app;

    QStringList rv;
    QStringList paths = Qtopia::installPaths();
    for(int ii = 0; ii < paths.count(); ++ii)
        rv.append(paths.at(ii) + "bin/" + app);

    return rv;
}

/*! \internal */
void ConsoleApplicationLauncher::kill(const QString &app)
{
    QMap<QString, ConsoleApplicationLauncherPrivate::App *>::Iterator iter =
        d->apps.find(app);
    if(iter != d->apps.end()) {
        (*iter)->process->kill();
    }
}

/*! \internal */
void ConsoleApplicationLauncher::routeMessage(const QString& ,
                                              const QString& ,
                                              const QByteArray& )
{
    // Do nothing
}

QTOPIA_TASK(ConsoleApplicationLauncher, ConsoleApplicationLauncher);
QTOPIA_TASK_PROVIDES(ConsoleApplicationLauncher, ApplicationTypeLauncher);

/*!
  \class ApplicationLauncher
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The ApplicationLauncher class is responsible for fundamental application management and IPC routing within Qtopia.

  The ApplicationLauncher provides a Qt Extended Server Task.  Qt Extended Server Tasks
  are documented in full in the QtopiaServerApplication class documentation.

  \bold{Note:} This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \table
  \row \o Task Name \o ApplicationLauncher
  \row \o Interfaces \o ApplicationLauncher
  \row \o Services \o Suspend
  \endtable

  IPC and application control are tightly linked in Qtopia.  At any level higher
  than the ApplicationLauncher itself, Qt Extended does not intrinsically understand
  the notion of "starting" an application.  Qt Extended treats an application as
  a named IPC endpoint that exposes one or more IPC services for use by other
  applications or the system itself.  The named IPC endpoint is known as the
  application's "application channel".

  The primary role of the ApplicationLauncher is to manage a component's ability
  to receive and respond to service requests.  The ApplicationLauncher considers
  an application "running" when it is able to receive service messages even
  though the application may not be thought as such by an end user.  Management
  of end user features of an application (such as when the UI is raised or
  hidden) is done by other system components.

  The application launcher framework consists of three primary parts - the
  application launcher, a series of application type launchers and the
  application IPC router.

  Internally the ApplicationLauncher class has a very basic model of an
  application - it is something that can be started, stopped and can receive IPC
  messages.  The specifics of process control is handled by pluggable
  implementers of the ApplicationTypeLauncher interface.  By separating the
  specifics in this way, Qt Extended can easily be adapted to handle foreign
  applications, such as Java applications, in a seamless and highly integrated
  fashion.

  The ApplicationLauncher also doesn't intrinsically know the specifics of the
  IPC bus.  Instead an implementation of the ApplicationIpcRouter class
  abstracts the details from it.

  When the system attempts to start an application by sending it a message on
  its application channel, the ApplicationIpcRouter detects the message and
  asks the ApplicationLauncher to launch the application, by calling the
  ApplicationLauncher::launch() method.  This instructs the ApplicationLauncher
  to bring the application to a state where it can receive IPC messages.

  If the application is not running, the ApplicationLauncher iterates through
  the ordered list of tasks implementing the ApplicationTypeLauncher interface.
  Each task is asked if it can launch the application, and if it can, is asked
  to do so.  The ApplicationLauncher monitors the progress of the
  ApplicationTypeLauncher through Qt signals, which it consolidates and emits
  to the rest of the Qt Extended server.

  Once the application is ready to receive messages, the ApplicationTypeLauncher
  informs the router by invoking the ApplicationIpcRouter::addRoute() method,
  passing an implementation of the ApplicationIpcRouter::RouteDestination
  interface.  This call instructs the IPC Router to forward messages to the
  application through the ApplicationIpcRouter::RouteDestination.  All queued
  messages will be delivered at this time.

  How the ApplicationIpcRouter::RouteDestination implementation (usually the
  ApplicationTypeLauncher) handles delivery of the message is up to it.  This
  allows more advanced ApplicationTypeLauncher implementations - such as a
  Java application type launcher - to adapt Qt Extended service messages into a form
  suitable for the application type they manage.  For example, while Qt Extended uses a "raise()" message sent to an application's application channel to
  cause it to show its main UI, a Java application that has no notion of Qt Extended messages will need to have this message transformed appropriately.

  To summarize,

  \table
  \header \o Component \o Class \o Functionality
  \row \o Application Launcher \o ApplicationLauncher \o Manages ApplicationTypeLauncher instances and maintains a consolidated view of applications.  Other components within the server may use the ApplicationLauncher to monitor application state, but must remember that this state only represents an applications ability to receive IPC messages, and not necessarily user visible information such as whether the application is visible.
  \row \o Application Type Launchers \o ApplicationTypeLauncher \o Handles the specifics of different types of applications and launching mechanisms on behalf of the ApplicationLauncher.  Other components within the server should never need to access these instances directly.
  \row \o Application Ipc Router \o ApplicationIpcRouter \o Abstracts the IPC transport used for application messages from the ApplicationLauncher.  On reception of an application message, instructs the ApplicationLauncher to start the application and subsequently delivers messages to ApplicationTypeLauncher instances for possible transformation before delivery to the application.
  \endtable
 */

// define ApplicationLauncher
/*!
  \internal
 */
ApplicationLauncher::ApplicationLauncher()
    : m_vso(0)
{
    /*
      Get the list of application launcher objects.
     */
    QSet<ApplicationTypeLauncher*> connected;
    m_launchers = qtopiaTasks<ApplicationTypeLauncher>();
    for (int ii = 0; ii < m_launchers.count(); ++ii) {
        ApplicationTypeLauncher* atl = m_launchers.at(ii);
        if (connected.contains(atl))
            continue;
        connected.insert(atl);
        QObject::connect(atl,SIGNAL(applicationStateChanged(QString,ApplicationTypeLauncher::ApplicationState)),this,SLOT(handleStateChange(QString,ApplicationTypeLauncher::ApplicationState)));

        QObject::connect(atl,SIGNAL(terminated(QString,ApplicationTypeLauncher::TerminationReason)),this,SLOT(terminated(QString,ApplicationTypeLauncher::TerminationReason)));
    }

    m_vso = new QValueSpaceObject("/System/Applications", this);

    new LegacyLauncherService(this);

}

void
ApplicationLauncher::handleStateChange(const QString &app,
                              ApplicationTypeLauncher::ApplicationState state)
{
    if (!m_runningApps.contains(app))
        return; // I don't know about this app

    int oldBusyCount = busyApps.count();

    if (ApplicationTypeLauncher::NotRunning == state) {
        m_vso->removeAttribute(app + "/Info");
        m_runningApps.remove(app);
        m_orderedApps.removeAll(app);
        QtopiaIpcEnvelope e(QLatin1String("QPE/QtopiaApplication"),
                            QLatin1String("notBusy(QString)") );
        e << app;
        busyApps.removeAll(app);
    }
    else {
        if(ApplicationTypeLauncher::Starting == state)
            m_vso->setAttribute(app + "/Info/LaunchTime",
                                QDateTime::currentDateTime());

        QByteArray stateText("Unknown");
        switch(state) {
            case ApplicationTypeLauncher::NotRunning:
                stateText = "NotRunning";
                break;
            case ApplicationTypeLauncher::Starting:
                stateText = "Starting";
                break;
            case ApplicationTypeLauncher::Running:
                stateText = "Running";
                break;
        }

        if (state != ApplicationTypeLauncher::Starting) {
            QtopiaIpcEnvelope e(QLatin1String("QPE/QtopiaApplication"),
                                QLatin1String("notBusy(QString)") );
            e << app;
            busyApps.removeAll(app);
        } else {
            busyApps.append(app);
        }

        m_vso->setAttribute(app + "/Info/State", stateText);
        qLog(QtopiaServer) << "ApplicationLauncher::handleStateChanged(" << app << ", " << stateText << ")";
    }

    if (busyApps.count() != oldBusyCount)
        m_vso->setAttribute("Info/BusyCount", busyApps.count());

    emit applicationStateChanged(app,state);
}

/*!
  Return the list of starting and running applications.  The list order
  represents the order in which the applications were launched.
  */
QStringList ApplicationLauncher::applications() const
{
    return m_orderedApps;
}

/*!
  Returns the current state of the application, \a app.
 */
ApplicationTypeLauncher::ApplicationState
ApplicationLauncher::state(const QString &app) const
{
    QMap<QString, ApplicationTypeLauncher *>::ConstIterator iter =
        m_runningApps.find(app);
    if (iter != m_runningApps.end())
        return (*iter)->applicationState(app);
    else
        return ApplicationTypeLauncher::NotRunning;
}

/*!
  \fn void ApplicationLauncher::applicationTerminated(const QString &application, ApplicationTypeLauncher::TerminationReason reason, bool filtered)

  Emitted whenever the \a application terminates. \a reason
  will be set to the termination reason. If \a filtered is
  true, an ApplicationTerminationHandler instance has filtered the
  termination.
 */

/*!
  \fn void ApplicationLauncher::applicationStateChanged(const QString &application, ApplicationTypeLauncher::ApplicationState state)

  Emitted whenever the \a application state changes to \a state.
 */

/*!
  \fn void ApplicationLauncher::applicationNotFound(const QString &application)

  Emitted whenever a non-existant \a application is asked to
  launch. The corresponding call to launch() will also return
  false.
 */

void
ApplicationLauncher::terminated(const QString &app,
                           ApplicationTypeLauncher::TerminationReason reason)
{
    m_vso->removeAttribute(app + "/Info");
    m_runningApps.remove(app);
    m_orderedApps.removeAll(app);

    bool filtered = false;
    QList<ApplicationTerminationHandler *> termHandlers =
        qtopiaTasks<ApplicationTerminationHandler>();
    for (int ii=0; !filtered && ii<termHandlers.count(); ++ii)
        filtered = termHandlers.at(ii)->terminated(app, reason);

    if (qLogEnabled(QtopiaServer)) {
        QString reasonText("Unknown");
        switch(reason) {
            case ApplicationTypeLauncher::FailedToStart:
                reasonText = "FailedToStart";
                break;
            case ApplicationTypeLauncher::Crashed:
                reasonText = "Crashed";
                break;
            case ApplicationTypeLauncher::Unknown:
                reasonText = "Unknown";
                break;
            case ApplicationTypeLauncher::Normal:
                reasonText = "Normal";
                break;
            case ApplicationTypeLauncher::Killed:
                reasonText = "Killed";
                break;
        }

        qLog(QtopiaServer) << "ApplicationLauncher::applicationTerminated("
                           << app << ", " << reasonText << ", "
                           << (filtered?"true":"false") << ")";

    }

    m_vso->removeAttribute(app + "/Info");
    QtopiaIpcEnvelope e(QLatin1String("QPE/QtopiaApplication"),
                        QLatin1String("notBusy(QString)") );
    e << app;

    if (busyApps.removeAll(app))
        m_vso->setAttribute("Info/BusyCount", busyApps.count());

    emit applicationTerminated(app, reason, filtered);

    qLog(QtopiaServer) << "ApplicationLauncher::terminated("
                       << app << ", NotRunning)";
    emit applicationStateChanged(app, ApplicationTypeLauncher::NotRunning);
}

/*!
  Returns true if application \a app can be launched. Returns
  false otherwise. When true is returned, it means an instance
  of a subclass of ApplicationTypeLauncher instance exists that
  knows how to launch the application. The subsequent attempt
  to launch the application may still fail.

  Note that true is also returned if \a app is already running.
 */
bool ApplicationLauncher::canLaunch(const QString &app)
{
    if (m_runningApps.contains(app))
        return true;
    for (int ii = 0; ii < m_launchers.count(); ++ii) {
        if (m_launchers[ii]->canLaunch(app))
            return true;
    }
    return false;
}

/*!
  Attempts to launch application \a app. Returns true if
  \a app is launched successfully. Also returns true if
  \a app is already running. Returns false if no launcher
  can be found that knows how to launch \a app.
  */
bool ApplicationLauncher::launch(const QString &app)
{
    if (m_runningApps.contains(app))
        return true;

    for (int ii=0; ii < m_launchers.count(); ++ii) {
        if (m_launchers[ii]->canLaunch(app)) {
            m_runningApps.insert(app,m_launchers[ii]);
            m_orderedApps.append(app);
            m_launchers[ii]->launch(app);
            return true;
        }
    }

    emit applicationNotFound(app);
    return false;
}

/*!
  \internal

  Kill the process for application \a app. This method is
  used by the OOM handling system to prevent an actual OOM
  state from occurring.
  */
bool ApplicationLauncher::kill(const QString& app)
{
    for (int ii=0; ii < m_launchers.count(); ++ii) {
        if (m_launchers[ii]->canLaunch(app)) {
            m_runningApps.insert(app,m_launchers[ii]);
            m_launchers[ii]->kill(app);
            return true;
        }
    }
    return false;
}

QTOPIA_TASK(ApplicationLauncher, ApplicationLauncher);
QTOPIA_TASK_PROVIDES(ApplicationLauncher, ApplicationLauncher);

/*
  The LegacyLauncherService exists to provide the "Launcher" service used by
  the Qtopia::execute() method.  In Qtopia 4.1 it was an "external" service,
  for use by any application.
 */
LegacyLauncherService::~LegacyLauncherService()
{
    // nothing.
}

void LegacyLauncherService::execute( const QString& app )
{
    qLog(ApplicationLauncher) << "LegacyLauncherService: Request for execute("
                              << app << ")";
    QtopiaIpcEnvelope env("QPE/Application/" + app, "raise()");
}

void LegacyLauncherService::execute(const QString& app,
                                    const QString& document )
{
    qLog(ApplicationLauncher) << "LegacyLauncherService: Request for execute("
                              << app << ", " << document << ")";
    QtopiaIpcEnvelope env("QPE/Application/" + app, "setDocument(QString)");
    env << document;
}

void LegacyLauncherService::kill( const QString& app )
{
    qLog(ApplicationLauncher) << "LegacyLauncherService: Request for kill("
                              << app << ")";
    ApplicationLauncher* al = qtopiaTask<ApplicationLauncher>();
    if (al) {
        al->kill(app);
    }
}

/*!
  \fn QString ConsoleApplicationLauncher::name()
  \internal
 */
/*!
  \fn QString ExeApplicationLauncher::name()
  \internal
 */
/*!
  \fn QString SimpleExeApplicationLauncher::name()
  \internal
 */
/*!
  \fn QString QtopiaServerApplicationLauncher::name()
  \internal
 */
/*!
  \fn QString BuiltinApplicationLauncher::name()
  \internal
 */
#include "applicationlauncher.moc"
