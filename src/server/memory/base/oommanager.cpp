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

#include "oommanager.h"
#include "qtopiaserverapplication.h"
#include <QDir>
#include <QSet>
#include <QMap>
#include <QSettings>
#include <QStringList>
#include <qtopialog.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/**
  \class OomPrivate
    \inpublicgroup QtDevToolsModule
    \inpublicgroup QtBaseModule
  \brief The OomPrivate class contains all the Out Of Memory data.

  There is one instance of OomPrivate, which is a Q_GLOBAL_STATIC.
  Each instance of the \l {OomManager} {Out-Of-Memory (OOM)} manager
  contains a pointer to it.

  This class contains three sets of application names. The first
  set is called \c critical. It contains the names of all the
  Qt Extended applications that must not be killed, when Qt Extended runs
  out of memory, eg Qt Extended itself (qpe) is in the critical set.

  The second set is called \c expendable. It contains the names
  of the applications that the user wants killed first, when
  Qt Extended runs out of memory.

  The third set is called \c important. It contains the names of
  applications the user wants to avoid killing, if possible, when
  Qt Extended runs out of memory. Important processes will not be killed
  if there are expendable processes running, but expendable processes
  can be killed. Only critical processes are not killable.

  All this data is read from a configuration file called \c{oom.conf}.
  The user can create \c{oom.conf} with a text editor. Here is an
  example:

  \code

  [oom_adj]
  qpe=critical
  qasteroids=expendable
  fifteen=expendable
  minesweep=expendable
  snake=expendable
  calculator=important
  clock=important
  datebook=important

  [values]
  critical=250
  verylow=120
  low=60
  samples=5
  percent=20
  long=10000
  short=1000
  rlimit=32

  \endcode

  Whenever a new process achieves the running state, its name
  and pid are inserted into a map here. Additionally, the
  process's oom_adj value is set here according to whether the
  process is critical, expendable, or important. The oom_adj
  value is used by the linux kernel to help it determine which
  processes to kill when a hard out-of-memory condition occurs.
  The oom_adj value is stored in /proc/<pid>/oom_adj.

  Hopefully, if the OOM Manager and the MemoryMonitor do the
  right stuff, they will prevent any hard out-of-memory events.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \internal
 */

class OomPrivate
{
  public:
    OomPrivate();
    OomPrivate(const OomPrivate& other);
    ~OomPrivate();

    void insert(const QString& app, int pid);
    void remove(const QString& app);

    const QMap<QString,int>& expendableProcs() const {
        return expendable_procs_;
    }
    const QMap<QString,int>& importantProcs() const {
        return important_procs_;
    }
    bool hasExpendableProcs() const {
        return !expendable_procs_.isEmpty();
    }
    bool hasImportantProcs() const {
        return !important_procs_.isEmpty();
    }
    bool isExpendable(const QString& app) const {
        return expendable_apps_.contains(app);
    }
    bool isImportant(const QString& app) const {
        return important_apps_.contains(app);
    }
    bool isCritical(const QString& app) const {
        return critical_apps_.contains(app);
    }

    void printOomValues(bool score) const;
    void printOomScores(const QMap<QString,int>& map) const;
    QString procWithBiggestScore(const QMap<QString,int>& map) const;

    QSet<QString>       expendable_apps_;
    QSet<QString>       important_apps_;
    QSet<QString>       critical_apps_;

    QMap<QString,int>   expendable_procs_;
    QMap<QString,int>   important_procs_;
    QMap<QString,int>   critical_procs_;
};

/*!
  The default constructor reads the \c{oom.conf} file, and
  initializes its state from the data in the file.
  \internal
 */
OomPrivate::OomPrivate()
{
    QSettings cfg("Trolltech","oom");
    //qLog(OOM) << "Reading oom.conf";
    int i = 0;
    cfg.beginGroup("oom_adj");
    QStringList slist = cfg.childKeys();
    for (i=0; i<slist.size(); ++i) {
        QString name = slist.at(i);
        QString value = cfg.value(name).toString();
        if (value == QString("critical"))
            critical_apps_.insert(name);
        else if (value == QString("important"))
            important_apps_.insert(name);
        else if (value == QString("expendable"))
            expendable_apps_.insert(name);
        else
            qLog(OOM) << "oom.conf illegal value:" << value;
    }
    cfg.endGroup();

    if (!critical_apps_.contains(QString("qpe")))
        critical_apps_.insert(QString("qpe"));

    if (qtopiaLogEnabled("OOM")) {
        QSet<QString>::const_iterator s = expendable_apps_.constBegin();
        while (s != expendable_apps_.constEnd()) {
            qLog(OOM) << "Expendable: " << *s;
            ++s;
        }
        s = important_apps_.constBegin();
        while (s != important_apps_.constEnd()) {
            qLog(OOM) << "Important: " << *s;
            ++s;
        }
        s = critical_apps_.constBegin();
        while (s != critical_apps_.constEnd()) {
            qLog(OOM) << "Critical: " << *s;
            ++s;
        }
        printOomValues(true);
    }

    //we assume we run as part of the server. Add the server to the list of apps.
}

/*!
  The destructor has nothing to do.
  \internal
 */
OomPrivate::~OomPrivate()
{
    // nothing.
}

/*!
  Inserts the process id \a pid into the appropriate map of id's
  of running processes and map it to application name \a app.
  Also uses the linux \c system command to set the oom_adj
  value for the process apropos its importance, ie critical,
  expendable, or important process. This hopefully guarantees
  that if an OOM occurs that is not detected by Qtopia, then
  when Linux kills a process it will not kill any process
  that is critical to Qtopia. eg, it will not kill the Qt Extended server itself.
  \internal
 */
void OomPrivate::insert(const QString& app, int pid)
{
    if (app.isEmpty())
        return;
    int oomadj = 0;
    if (important_apps_.contains(app)) {
        important_procs_.insert(app,pid);
        qLog(OOM) << "Important proc added:" << app << pid;
    }
    else if (critical_apps_.contains(app)) {
        critical_procs_.insert(app,pid);
        oomadj = -16;
        qLog(OOM) << "Critical proc added:" << app << pid;
    }
    else {
        oomadj = 15;
        expendable_procs_.insert(app,pid);
        qLog(OOM) << "Expendable proc added:" << app << pid;
    }
    QString oom_file = QString("/proc/%1/oom_adj").arg(pid);
    QFile file(oom_file);
    if (!file.exists()) {
        qLog(OOM) << oom_file << "does not exist";
        return;
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qLog(OOM) << "Unable to open" << oom_file << "for writing";
        return;
    }
    qLog(OOM) << "Writing" << oomadj << "to" << oom_file;
    QTextStream out(&file);
    out << oomadj << endl;
    out.flush();
    file.flush();

    if (qtopiaLogEnabled("OOM")) {
        printOomScores(critical_procs_);
        printOomScores(important_procs_);
        printOomScores(expendable_procs_);
    }
}

/*!
  Removes the mapping from application \a app to its process id.
  \internal
 */
void OomPrivate::remove(const QString& app)
{
    if (app.isEmpty())
        return;
    int pid = 0;
    QMap<QString,int>::iterator i;
    i = important_procs_.find(app);
    if (i != important_procs_.end()) {
        pid = i.value();
        important_procs_.erase(i);
        qLog(OOM) << "Removed important proc:" << app << pid;
        return;
    }
    i = critical_procs_.find(app);
    if (i != critical_procs_.end()) {
        pid = i.value();
        critical_procs_.erase(i);
        qLog(OOM) << "Removed critical proc:" << app << pid;
        return;
    }
    i = expendable_procs_.find(app);
    if (i != expendable_procs_.end()) {
        pid = i.value();
        expendable_procs_.erase(i);
        qLog(OOM) << "Removed expendable proc:" << app << pid;
        return;
    }
}

/*!
  Returns the application name of the process that has the
  biggest OOM score found in \a map. It reads the value in
  the oom_score file in /proc for each app/pid pair in \a map.
  \internal
 */
QString OomPrivate::procWithBiggestScore(const QMap<QString,int>& map) const
{
    int biggest_score = -10000;
    QString oom_file;
    QString application("");
    if (map.isEmpty())
        return application;
    QMap<QString,int>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        oom_file = QString("/proc/%1/oom_score").arg(i.value());
        QFile file(oom_file);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qLog(OOM) << "Unable to open:" << oom_file;
            continue;
        }
        bool ok;
        QTextStream in(&file);
        QString s = in.readAll();
        long oom_score = s.toLong(&ok);
        if (ok) {
            qLog(OOM) << oom_file << "oom_score =" << oom_score;
            if (oom_score > biggest_score) {
                biggest_score = oom_score;
                application = i.key();
            }
        }
        ++i;
    }
    return application;
}

/*!
  This function prints the oom_score values for the processes
  in the \a map.
  \internal
 */
void OomPrivate::printOomScores(const QMap<QString,int>& map) const
{
    if (map.isEmpty())
        return;
    QString oom_file;
    QString oom_app;
    QMap<QString,int>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        oom_file = QString("/proc/%1/oom_score").arg(i.value());
        oom_app = QString("/proc/%1/oom_score").arg(i.key());
        ++i;
        QFile file(oom_file);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qLog(OOM) << "Unable to open:" << oom_file;
            continue;
        }
        bool ok;
        QTextStream in(&file);
        QString s = in.readAll();
        long oom_score = s.toLong(&ok);
        qLog(OOM) << oom_app << "= " << oom_score;
    }
}

/*!
  If \a score is true, print the oom_score values for all
  the processes. Otherwise, print the oom_adj values for
  all the processes.
  \internal
*/
void OomPrivate::printOomValues(bool score) const
{
    QDir proc("/proc");
    QStringList procDirs;
    procDirs = proc.entryList(QDir::AllDirs,QDir::Name|QDir::Reversed);

    QString dir;
    QString oom_file;
    bool ok = false;
    foreach (dir,procDirs) {
        dir.toLong(&ok); //only want processes
        if (ok) {
            oom_file = QString("/proc/") + dir;
            if (score)
                oom_file = oom_file + QString("/oom_score");
            else
                oom_file = oom_file + QString("/oom_adj");
            QFile file(oom_file);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qLog(OOM) << "Unable to open " << oom_file;
                continue;
            }
            QTextStream in(&file);
            QString s = in.readAll();
            long oom_value = s.toLong(&ok);
            if (score)
                qLog(OOM) << oom_file << "oom_score = " << oom_value;
            else
                qLog(OOM) << oom_file << "oom_adj = " << oom_value;
        }
    }
}

Q_GLOBAL_STATIC(OomPrivate, oomPrivate);

/*!
  \class OomManager
    \inpublicgroup QtDevToolsModule
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Memory
  \brief The OomManager class manages low and out of memory situations.

  This class is a wrapper for the \l{Handling Out Of Memory}{Out-of-memory manager}.
  It is used in sublasses of the
  \l {ApplicationTypeLauncher} class that launch applications
  as linux processes. It is also used in \l {ApplicationLauncher}
  and \l {LowMemoryTask}.

  This class contains three sets of application names. The first
  set is called \c critical. It contains the names of all the
  Qt Extended applications that must not be killed, when Qt Extended runs
  out of memory, eg Qt Extended itself (qpe) is in the critical set.

  The second set is called \c expendable. It contains the names
  of the applications that the user wants killed first, when
  Qt Extended runs out of memory.

  The third set is called \c important. It contains the names of
  applications the user wants to avoid killing, if possible, when
  Qt Extended runs out of memory. Important processes will not be killed
  if there are expendable processes running, but expendable processes
  can be killed. Only critical processes are not killable.

  All this data is read from a configuration file called \c{oom.conf}.
  The user can create \c{oom.conf} with a text editor. Here is an
  example:

  \code

  [oom_adj]
  qpe=critical
  qasteroids=expendable
  fifteen=expendable
  minesweep=expendable
  snake=expendable
  calculator=important
  clock=important
  datebook=important

  [values]
  critical=250
  verylow=120
  low=60
  samples=5
  percent=20
  long=10000
  short=1000
  rlimit=32

  \endcode

  Whenever a new process achieves the running state, its name
  and pid are inserted into a map here. Additionally, the
  process's oom_adj value is set here according to whether the
  process is critical, expendable, or important. The oom_adj
  value is used by the linux kernel to help it determine which
  processes to kill when a hard out-of-memory condition occurs.
  The oom_adj value is stored in \c{/proc/<pid>/oom_adj}.

  \bold{Note:} All OomManager instances share the same internal list
  of applications.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */


/*!
  This constrcutor creates a new OomManager instance.
 */
OomManager::OomManager() : d(oomPrivate())
{
    QSet<ApplicationTypeLauncher*> connected;
    QList<ApplicationTypeLauncher*> m_al = qtopiaTasks<ApplicationTypeLauncher>();
    for (int i=0; i<m_al.count(); i++) {
        ApplicationTypeLauncher* atl = m_al.at(i);
        if ( connected.contains(atl) )
            continue;
        
        connect( atl, SIGNAL(pidStateChanged(QString,Q_PID)),
                 this, SLOT(pidStateChanged(QString,Q_PID)) );
    }

    //we assume we are running as part of the server
    d->insert("qpe", ::getpid());
}

/*!
  Destroys the OomManager instance.
 */
OomManager::~OomManager()
{
    d->remove("qpe");
}

/*!
  Insert a mapping from \a app to \a pid. The process \a pid
  must be in the running state, and it must be an instance of
  \a app. This function must be called when process \a pid
  enters the running state.

  The value in \c{/proc/pid/oom_adj} is set to a value read from
  oom.conf according to whether the process is a critical,
  expendable, or important process.
 */
void
OomManager::insert(const QString& app, int pid)
{
    d->insert(app,pid);
}

/*!
  Remove the mapping from application \a app to its process
  id. This function must be called for any linux process
  started by the \l {ApplicationLauncher}, when the process
  is terminated for any reason.
 */
void
OomManager::remove(const QString& app)
{
    d->remove(app);
}

/*!
  Returns a reference to the map of expendable processes.
  that maps application name to process id. The map can be
  empty.
 */
const QMap<QString,int>&
OomManager::expendableProcs() const
{
    return d->expendableProcs();
}

/*!
  Returns a reference to the map of important processes.
  that maps application name to process id. The map can be
  empty.
 */
const QMap<QString,int>&
OomManager::importantProcs() const
{
    return d->importantProcs();
}

/*!
  Returns true if at least one of the running process is
  marked as being expendable.
 */
bool OomManager::hasExpendableProcs() const
{
    return d->hasExpendableProcs();
}

/*!
  Returns true if at least one of the running process is
  marked as being important.
 */
bool OomManager::hasImportantProcs() const
{
    return d->hasImportantProcs();
}

/*!
  \enum OomManager::Importance

  This enum is used to describe the various application priority
  as seen by the OOM Manager.

  \value Expendable Expendable applications have lowest priority and will be killed first.
  \value Important Important applications should only be killed if no expendable applications are left to be killed.
  \value Critical Critical applications cannot be killed.
  */

/*!
  Returns the application name of the process marked as \a t
  that has the biggest OOM score.
 */
QString OomManager::procWithBiggestScore(Importance t) const
{
    if (t == Expendable)
        return d->procWithBiggestScore(expendableProcs());
    if (t == Important)
        return d->procWithBiggestScore(importantProcs());
    return QString("");
}

/*!
  \internal
  If \a scroe is true, print the oom_score values for all
  the processes. Otherwise, print the oom_adj values for
  all the processes.
 */
void OomManager::printOomValues(bool score)
{
    d->printOomValues(score);
}

/*!
  Returns true if \a app is not marked important or critical.
 */
bool OomManager::isExpendable(const QString& app) const
{
    return (d->isImportant(app) || d->isCritical(app)) ? false : true;
}

/*!
  Returns true if \a app is marked important.
 */
bool OomManager::isImportant(const QString& app) const
{
    return d->isImportant(app);
}

/*!
  \internal
  Monitors what process come and go.
 */
void OomManager::pidStateChanged(const QString& app, Q_PID pid)
{
    if ( 0 == pid ) {
        d->remove(app);
    } else {
        d->insert( app, pid );
    }
}

QTOPIA_TASK( OomManager, OomManager );
QTOPIA_TASK_PROVIDES( OomManager, OomManager );
