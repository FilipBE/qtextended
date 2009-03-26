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

#ifndef APPLICATIONLAUNCHER_H
#define APPLICATIONLAUNCHER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QProcess>
#include "qtopiaserverapplication.h"

class QValueSpaceObject;

// XXX - QString's used for application names should become QContent's

class ApplicationIpcRouter : public QObject
{
    Q_OBJECT
public:
    class RouteDestination {
    public:
        virtual void routeMessage(const QString &, const QString &, const QByteArray &) = 0;
        virtual ~RouteDestination() {;}
    };
    virtual void addRoute(const QString &app, RouteDestination *) = 0;
    virtual void remRoute(const QString &app, RouteDestination *) = 0;
};
QTOPIA_TASK_INTERFACE(ApplicationIpcRouter);

class ApplicationTypeLauncher : public QObject
{
  Q_OBJECT

  public:
    enum TerminationReason { FailedToStart, Crashed, Unknown, Normal, Killed };
    enum ApplicationState { NotRunning, Starting, Running };

    virtual ~ApplicationTypeLauncher() { /* nothing */ }
    virtual ApplicationState applicationState(const QString &) = 0;
    virtual bool canLaunch(const QString &) = 0;
    virtual void launch(const QString &) = 0;
    virtual void kill(const QString &) = 0;
    virtual QString name() = 0;

  signals:
    void applicationStateChanged(const QString&,
				ApplicationTypeLauncher::ApplicationState);
    void terminated(const QString&,
                    ApplicationTypeLauncher::TerminationReason);
    void pidStateChanged(const QString&,Q_PID);
};
QTOPIA_TASK_INTERFACE(ApplicationTypeLauncher);

class ApplicationTerminationHandler : public QObject
{
    Q_OBJECT

  public:
    ApplicationTerminationHandler(QObject *parent = 0)
        : QObject(parent) {}

    virtual bool terminated(const QString &,
                            ApplicationTypeLauncher::TerminationReason) = 0;
};
QTOPIA_TASK_INTERFACE(ApplicationTerminationHandler);

class ApplicationLauncher : public QObject
{
  Q_OBJECT
  public:
    ApplicationLauncher();

    bool canLaunch(const QString &);
    bool launch(const QString &); // Bring to a state where it can receive QCop
    bool kill(const QString &);   // For memory restoration

    // Order of starting
    QStringList applications() const;
    ApplicationTypeLauncher::ApplicationState state(const QString &) const;

  signals:
    void applicationTerminated(const QString &,
                               ApplicationTypeLauncher::TerminationReason,
                               bool filtered = false);
    void applicationStateChanged(const QString &,
                                 ApplicationTypeLauncher::ApplicationState);
    void applicationNotFound(const QString &);


  private slots:
    void handleStateChange(const QString &,
			   ApplicationTypeLauncher::ApplicationState);
    void terminated(const QString &,
                    ApplicationTypeLauncher::TerminationReason);

  private:
    QList<QString> m_orderedApps;
    QMap<QString, ApplicationTypeLauncher *> m_runningApps;
    QList<ApplicationTypeLauncher *> m_launchers;
    QValueSpaceObject *m_vso;
    QStringList busyApps;
};
QTOPIA_TASK_INTERFACE(ApplicationLauncher);

class QtopiaServerApplicationLauncher : public ApplicationTypeLauncher,
                                        public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    QtopiaServerApplicationLauncher();

    // ApplicationTypeLauncher
    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);
    virtual void kill(const QString &app);
    virtual ApplicationState applicationState(const QString &app);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);
    virtual QString name() {
        return QString("QtopiaServerApplicationLauncher");
    }
};

class ExeApplicationLauncherPrivate;
class ExeApplicationLauncher : public ApplicationTypeLauncher,
                               public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    ExeApplicationLauncher();
    virtual ~ExeApplicationLauncher();

    // ApplicationTypeLauncher
    virtual void kill(const QString &app);
    virtual ApplicationState applicationState(const QString &app);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);
    virtual QString name() {
	return QString("ExeApplicationLauncher");
    }

protected:
    void addStartingApplication(const QString &, QProcess *);
    bool isRunning(const QString &);

private slots:
    void appExited(int, QProcess::ExitStatus);
    void appError(QProcess::ProcessError);
    void qtopiaApplicationChannel(const QString &,const QByteArray &);
    void newChannel( const QString& ch );

private:
    ExeApplicationLauncherPrivate *d;
};

class SimpleExeApplicationLauncherPrivate;
class SimpleExeApplicationLauncher : public ExeApplicationLauncher
{
Q_OBJECT
public:
    SimpleExeApplicationLauncher();
    virtual ~SimpleExeApplicationLauncher();

    // ApplicationTypeLauncher
    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);
    virtual QString name() {
	return QString("SimpleExeApplicationLauncher");
    }

protected:
        void setupPackageLaunch(const QString &exec, QProcess *proc);

private:
    static QStringList applicationExecutable(const QString &app);

    SimpleExeApplicationLauncherPrivate *d;
};

class ConsoleApplicationLauncherPrivate;
class ConsoleApplicationLauncher : public ApplicationTypeLauncher,
                                   public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    ConsoleApplicationLauncher();
    virtual ~ConsoleApplicationLauncher();

    // ApplicationTypeLauncher
    virtual ApplicationState applicationState(const QString &);
    virtual bool canLaunch(const QString &);
    virtual void launch(const QString &);
    virtual void kill(const QString &);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);
    virtual QString name() {
	return QString("ConsoleApplicationLauncher");
    }

private slots:
    void appStarted();
    void appExited(int);
    void appError(QProcess::ProcessError error);

private: 
    QStringList applicationExecutable(const QString &app);
    ConsoleApplicationLauncherPrivate *d;
};

class BuiltinApplicationLauncherPrivate;
class BuiltinApplicationLauncher : public ApplicationTypeLauncher,
                                   public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    BuiltinApplicationLauncher();
    virtual ~BuiltinApplicationLauncher();

    // ApplicationTypeLauncher
    virtual ApplicationState applicationState(const QString &);
    virtual bool canLaunch(const QString &);
    virtual void launch(const QString &);
    virtual void kill(const QString &);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);
    virtual QString name() {
	return QString("BuiltinApplicationLauncher");
    }

    bool eventFilter(QObject* o, QEvent *e);
    typedef QWidget *(*BuiltinFunc)();
    static void install(const char *, BuiltinFunc);

private slots:
    void appDestroyed(QObject *);

private:
    friend class _BuiltinInstaller;
    BuiltinApplicationLauncherPrivate *d;
    QValueSpaceObject *vso;
};

class _BuiltinInstaller
{
public:
    _BuiltinInstaller(const char *n, BuiltinApplicationLauncher::BuiltinFunc f)
    { BuiltinApplicationLauncher::install(n, f); }
};

#define QTOPIA_SIMPLE_BUILTIN(ApplicationName, createFunc) _BuiltinInstaller _builtin_install_ ## ApplicationName(# ApplicationName, createFunc)

#endif
