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

#include <QSettings>
#include <QtopiaIpcEnvelope>
#include <QContent>
#include <QContentSet>
#include <QStringList>
#include <QtopiaService>
#include <qtopialog.h>
#include "qtopiaserverapplication.h"
#include "qabstractmessagebox.h"
#include "startupapps.h"
#include "applicationlauncher.h"

class DaemonLauncher : public SystemShutdownHandler
{
Q_OBJECT
public:
    DaemonLauncher();
    ~DaemonLauncher();

    struct Application {
        QString group;
        bool running;
        bool silent;
        QString name;
        QString startupMessage;
        QString restartMessage;
        QString shutdownMessage;
    };

    void startup();
    void launch(const QString &);

protected:
    virtual bool systemRestart();
    virtual bool systemShutdown();
    virtual void timerEvent(QTimerEvent *);

private slots:
    void stateChanged(const QString &,
                      ApplicationTypeLauncher::ApplicationState);

private:
    void readConfig();
    void installTasks();
    void doShutdown();

    ApplicationLauncher *m_launcher;
    Application *readApplication(QSettings &, const QString &);
    QMap<QString, Application *> m_applications;
    QMap<QString, QList<Application *> > m_groups;
    QList<char *> m_taskNames;
    int m_runningWithShutdown;
    int m_shutdownTimer;
};
Q_GLOBAL_STATIC(DaemonLauncher, daemonLauncher);

class InstallDaemons {
public:
    InstallDaemons() { daemonLauncher()->startup(); };
};
static InstallDaemons id;

static QObject * daemonLauncherTaskFunc(void *arg)
{
    const char *task = (const char *)arg;
    Q_ASSERT(task);
    DaemonLauncher *dl = daemonLauncher();
    if(dl)
        dl->launch(task);
    return 0;
}

DaemonLauncher::DaemonLauncher()
    : m_launcher(0)
    , m_runningWithShutdown(0)
    , m_shutdownTimer(0)
{
}

DaemonLauncher::~DaemonLauncher()
{
    while (!m_taskNames.isEmpty()) {
        char *name = m_taskNames.takeLast();
        delete [] name;
    }

    QList<Application *> apps = m_applications.values();
    m_applications.clear();
    m_groups.clear();
    while (!apps.isEmpty())
        delete apps.takeLast();
}

void DaemonLauncher::doShutdown()
{
    Q_ASSERT(!m_shutdownTimer);
    Q_ASSERT(m_runningWithShutdown);

    for(QMap<QString, Application *>::Iterator iter = m_applications.begin();
            iter != m_applications.end(); ++iter) {

        Application *app = *iter;
        if(app->running && !app->shutdownMessage.isEmpty()) {
            qLog(QtopiaServer) << "DaemonLauncher: Shutting down application"
                               << app->name;
            QtopiaIpcEnvelope env("QPE/Application/" + app->name, app->shutdownMessage);
        }
    }

    m_shutdownTimer = startTimer(SystemShutdownHandler::timeout());
}

bool DaemonLauncher::systemRestart()
{
    if(!m_runningWithShutdown)
        return true;

    doShutdown();
    return false;
}

bool DaemonLauncher::systemShutdown()
{
    if(!m_runningWithShutdown)
        return true;

    doShutdown();
    return false;
}

void DaemonLauncher::timerEvent(QTimerEvent *)
{
    Q_ASSERT(m_shutdownTimer);
    killTimer(m_shutdownTimer);
    m_shutdownTimer = 0;

    struct Remaining {
        static inline QString apps(QList<Application*> const& applications) {
            QStringList ret;
            foreach (Application* app, applications) {
                if (app->running) {
                    ret << app->name;
                }
            }
            return ret.join(",");
        }
    };
    qLog(QtopiaServer) << "DaemonLauncher:" << m_runningWithShutdown << "applications did not shutdown:" << qPrintable(Remaining::apps(m_applications.values()));
    emit proceed();
}

void DaemonLauncher::stateChanged(const QString &app,
                                  ApplicationTypeLauncher::ApplicationState s)
{
    if(s != ApplicationTypeLauncher::NotRunning)
        return;

    QMap<QString, Application *>::Iterator iter = m_applications.find(app);
    if(iter == m_applications.end() || !(*iter)->running)
        return;

    Application *appl = *iter;

    appl->running = false;
    if(!appl->restartMessage.isEmpty()) {
        Q_ASSERT(m_runningWithShutdown);
        --m_runningWithShutdown;

        if(m_shutdownTimer && !m_runningWithShutdown) {
            // OK, we shut down everything we were supposed to.
            killTimer(m_shutdownTimer);
            m_shutdownTimer = 0;
            emit proceed();
            return;
        }
    }

    if(!appl->restartMessage.isEmpty()) {
        Q_ASSERT(m_launcher->canLaunch(app));
        {
            {
                qLog(QtopiaServer) << "DaemonLauncher: Restarting app" << app;
                QtopiaIpcEnvelope env("QPE/Application/" + app,
                                      appl->restartMessage);
            }
            appl->running = true;
            if(!appl->restartMessage.isEmpty())
                ++m_runningWithShutdown;
        }
    } else {
        qLog(QtopiaServer) << "DaemonLauncher: Non-restartable application"
                           << app << "terminated";
    }
}

void DaemonLauncher::launch(const QString &group)
{
    if(!m_launcher) {
        m_launcher = qtopiaTask<ApplicationLauncher>();
        Q_ASSERT(m_launcher &&
                 "ApplicationLauncher required for DaemonLauncher");

        QObject::connect(m_launcher, SIGNAL(applicationStateChanged(QString,ApplicationTypeLauncher::ApplicationState)), this, SLOT(stateChanged(QString,ApplicationTypeLauncher::ApplicationState)));
    }

    QMap<QString, QList<Application *> >::Iterator iter = m_groups.find(group);
    Q_ASSERT(iter != m_groups.end());
    const QList<Application *> &apps = *iter;

    qLog(QtopiaServer) << "DaemonLauncher: Launching" << group;
    for(int ii = 0; ii < apps.count(); ++ii) {
        Application *appl(apps.at(ii));
        if(appl->running) {
            qLog(QtopiaServer) << "    Skipping running application"
                               << appl->name;
        } else if(!m_launcher->canLaunch(appl->name)) {
            if(!appl->silent) {
                qWarning() << "DaemonLauncher: Unable to launch missing "
                              "application" << appl->name;
            }
        } else if(!appl->startupMessage.isEmpty()) {
            {
                qLog(QtopiaServer) << "DaemonLauncher: Launching app" << appl->name;
                QtopiaIpcEnvelope env("QPE/Application/" + appl->name, appl->startupMessage);
            }
            appl->running = true;
            if(!appl->restartMessage.isEmpty())
                ++m_runningWithShutdown;
        } else if(!appl->silent) {
            qLog(QtopiaServer) << "DaemonLauncher: No startup message for "
                                  "application" << appl;
        }
    }
}

void DaemonLauncher::startup()
{
    readConfig();
    installTasks();
}

void DaemonLauncher::installTasks()
{
    for(QMap<QString, QList<Application *> >::ConstIterator iter =
            m_groups.begin(); iter != m_groups.end(); ++iter) {
        Q_ASSERT(iter->count());

        char *taskName = new char[iter.key().length() + 1];
        ::strcpy(taskName, iter.key().toLatin1().constData());
        Q_ASSERT(QString(taskName) == iter.key());
        QtopiaServerApplication::addTask(taskName, false,
                                         daemonLauncherTaskFunc,
                                         (void *)taskName);
        m_taskNames.append(taskName);
    }
}

void DaemonLauncher::readConfig()
{
    //BackgroundApplications.conf is deprecated
    //TODO Remove in Qtopia 4.5 or later
    QSettings cfg("Trolltech", "BackgroundApplications");
    int count = cfg.value("Count", 0).toInt();

    for(int ii = 0; ii < count; ++ii) {
        QString appstr = "Application" + QString::number(ii);
        cfg.beginGroup(appstr);
        Application *app = readApplication(cfg, appstr);
        cfg.endGroup();
        if(m_applications.contains(app->name)) {
            qWarning() << "DaemonLauncher: Duplicate application" << app->name
                       << "discarded.";
            delete app;
        } else {
            m_applications.insert(app->name, app);
            m_groups[app->group].append(app);
        }
    }

    /* Check:
        1. HOME/Aplications/daemons
        2. INSTALLPATHS/etc/daemons
    */
    QStringList additionalPaths;
    QString homePath = Qtopia::homePath()+"/Applications/daemons";
    if (QFile::exists(homePath))
        additionalPaths+=homePath;

    QStringList ips = Qtopia::installPaths();
    foreach(QString ip, ips) {
        QString temp = ip + "etc/daemons";
        if ( QFile::exists(temp) )
            additionalPaths+=temp;
    }

    for (QStringList::ConstIterator iter = additionalPaths.begin();
            iter != additionalPaths.end(); iter++)
    {
        QDir dir(*iter);
        QFileInfoList flist = dir.entryInfoList( QStringList(), QDir::Files );
        for( int i = 0; i < flist.count(); i++ ) {
            QSettings config( flist[i].absoluteFilePath(), QSettings::IniFormat );
            if ( config.status() == QSettings::NoError &&
                    config.childGroups().contains("DaemonParameter") ) {
                config.beginGroup("DaemonParameter");
                Application *app = readApplication(config, flist[i].absoluteFilePath());
                config.endGroup();
                if(m_applications.contains(app->name)) {
                    qWarning() << "DaemonLauncher: Duplicate application" << app->name
                        << "discarded.";
                    delete app;
                } else {
                    m_applications.insert(app->name, app);
                    m_groups[app->group].append(app);
                }
            }
        }
    }
}

DaemonLauncher::Application *
DaemonLauncher::readApplication(QSettings &cfg, const QString &app)
{
    bool silent = cfg.value("Silent", false).toBool();

    QString appName = cfg.value("ApplicationName").toString();
    QString serviceName = cfg.value("ServiceName").toString();
    if(!appName.isEmpty() && !serviceName.isEmpty()) {
        if(!silent)
            qWarning() << "DaemonLauncher: Application name" << appName
                       << "overrides service name" << serviceName << "for"
                       << app;
    } else if(appName.isEmpty() && !serviceName.isEmpty()) {
        appName = QtopiaService::binding(serviceName);
    }

    if(appName.isEmpty()) {
        if(!silent)
            qWarning() << "DaemonLauncher: Cannot resolve application name for"
                       << app;
        return 0;
    }

    QString group = cfg.value("TaskGroup").toString();
    if(group.isEmpty()) {
        if(!silent)
            qWarning() << "DaemonLauncher: Application" << app << "does not"
                          "include a group specification";
        return 0;
    }

    QString startupMessage = cfg.value("StartupMessage").toString();
    if(startupMessage.isEmpty() && !silent)
        qWarning() << "DaemonLauncher: No startup message specified for"
                   << app;
    QString restartMessage = cfg.value("RestartMessage").toString();
    QString shutdownMessage = cfg.value("ShutdownMessage").toString();
    if(!startupMessage.isEmpty() && !serviceName.isEmpty())
        startupMessage.prepend(serviceName + "::");
    if(!shutdownMessage.isEmpty() && !serviceName.isEmpty())
        shutdownMessage.prepend(serviceName + "::");

    Application *appl = new Application;
    appl->name = appName;
    appl->group = group;
    appl->running = false;
    appl->silent = silent;
    appl->startupMessage = startupMessage;
    appl->restartMessage = restartMessage;
    appl->shutdownMessage = shutdownMessage;
    return appl;
}

// declare StartupApplicationsPrivate
class StartupApplicationsPrivate : public ApplicationTerminationHandler
{
Q_OBJECT
public:
    StartupApplicationsPrivate(QObject *parent)
    : ApplicationTerminationHandler(parent) {}

    virtual bool terminated(const QString &,
                            ApplicationTypeLauncher::TerminationReason);

signals:
    void preloadCrashed(const QString &);
};

// define StartupApplicationsPrivate
bool StartupApplicationsPrivate::terminated(const QString &name,
        ApplicationTypeLauncher::TerminationReason reason)
{
    if(ApplicationTypeLauncher::Normal == reason) return false;

    QContent app(name, false);
    if( app.isNull() ) return false;

    // Disable crashing preloaded applications
    if(app.isPreloaded()) {
        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("AppLoading");
        QStringList apps = cfg.value("PreloadApps").toStringList();
        QString exe = app.executableName();
        apps.removeAll(exe);
        cfg.setValue("PreloadApps", apps);
        emit preloadCrashed( name );

        return true;
    } else {
        return false;
    }
}

/*!
    \class StartupApplications
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer::Task
    \ingroup QtopiaServer::AppLaunch
    \brief The StartupApplications class launches applications preemptively at startup.

    The StartupApplications provides a Qt Extended Server Task.  Qt Extended Server Tasks
    are documented in full in the QtopiaServerApplication class documentation.

    Qt Extended provides support for preloading GUI applications, and for launching
    background daemon-type applications during startup.  The StartupApplications
    class provides this functionality.

    \section1 Preloaded Applications

    Preloaded applications are started immediately after Qt Extended starts, but in
    a hidden state.  Preloading an application ensures that it is available for
    almost instantaneouse "startup" whenever the user accesses it.  However, as
    preloaded applications continue to run indefinately, they consume system
    resources such as RAM and scheduler time even when not actively in use.  Only
    applications used very frequently should be preloaded.

    An application can be preloaded by adding it to the
    \c {AppLoading\PreloadApps} item list in the \c {Trolltech/Launcher}
    configuration file.  The following example shows both the \c addressbook and
    \c qtmail applications set to preload.

    \code
    [AppLoading]
    PreloadApps=addressbook,qtmail
    \endcode

    \section1 Background Applications

    Background applications allow simple QCop messages to be sent to an
    application during startup, whenever it terminates or at system shutdown.
    Background applications allow Qt Extended to launch and control simple daemon-style
    servers.

    \section2 Daemon configuration

    The configuration files for background applications can be found in the following
    path positions:

    \list 1
        \o \c {$HOME/Applications/daemons}
        \o \c {$QPE_IMAGE/etc/daemons}
        \o \c {$QTOPIA_PATH/etc/daemons}
    \endlist

    If the same background application is configured by more than one configuration file
    the above directory order determines the priority. This allows the user to override
    the system behavior.

    The configuration file has the following form:

    \code

    [DaemonParameter]
    ApplicationName=<Application Name>
    ServiceName=<Service Name>
    TaskGroup=<Qt Extended Task Group>
    Silent=<true/false>
    StartupMessage=<Message>
    RestartMessage=<Message>
    ShutdownMessage=<Message>
    \endcode


    Each option maps to the following functionality:
    \table
    \header \o Option \o Description
    \row \o \c {ApplicationName} \o The name of the application to send the QCop messages to.
    \row \o \c {ServiceName} \o A name of a service to send QCop messages to.  The default provider of the service will be used.  If both \c {ApplicationName} and \c {ServiceName} are specified, the \c {ServiceName} option is ignored.
    \row \o \c {TaskGroup} \o The name of the group to which the background application belogs.  Group names allow a collection of applications to be started together at startup by inserting the group name into the \c {Tasks.cfg} Qt Extended startup file.  Task groups are not optional.
    \row \o \c {Silent} \o If true, warning messages about this application will be suppressed.  For example, if the application or service cannot be found a qWarning() will only appear if \c {Silent} is false or omitted.
    \row \o \c {StartupMessage} \o A message to send when the application is started.  A common value for \c StartupMessage is \c raise.
    \row \o \c {RestartMessage} \o A message to send to restart the application when it terminates.  This message is optional and will only be sent if the application had previously been started with \c {StartupMessage}.
    \row \o \c {ShutdownMessage} \o A message to send at system shutdown.  This message is optional and will only be sent if the application had previously been started with \c {StartupMessage}.  Following this message, the application is expected to exit within 5 seconds.
    \endtable

    Here is a concrete example, specifying that the MessageServer application should be started with the 'startup' application group, and should be restarted
    if it terminates:

    \code
    [DaemonParameter]
    ApplicationName=messageserver
    TaskGroup=startup
    StartupMessage=raise()
    RestartMessage=raise()
    \endcode

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*!
  Create the StartupApplications class with the specified \a parent.
 */
StartupApplications::StartupApplications(QObject *parent)
: QObject(parent)
{
    StartupApplicationsPrivate *d = new StartupApplicationsPrivate(this);
    QObject::connect(d, SIGNAL(preloadCrashed(QString)),
                     this, SIGNAL(preloadCrashed(QString)));
    QtopiaServerApplication::addAggregateObject(this, d);
    QtopiaServerApplication::addAggregateObject(this, daemonLauncher());

    QSettings cfg("Trolltech","Launcher");
    cfg.beginGroup("AppLoading");
    QStringList apps = cfg.value("PreloadApps").toStringList();
    foreach (QString app, apps) {
        QtopiaIpcEnvelope e("QPE/Application/"+app, "enablePreload()");
    }
}

/*!
  \fn void StartupApplications::preloadCrashed(const QString &application)

  Emitted whenever a preloaded \a application crashes.  A crashing preloaded
  application will be automatically removed from the preloaded application
  list.

  The PreloadApplication class filter crashes, using the
  ApplicationTerminationHandler interface.
 */

QTOPIA_TASK(StartupApplications, StartupApplications);
QTOPIA_TASK_PROVIDES(StartupApplications, StartupApplications);
QTOPIA_TASK_PROVIDES(StartupApplications, ApplicationTerminationHandler);
QTOPIA_TASK_PROVIDES(StartupApplications, SystemShutdownHandler);

#include "startupapps.moc"
