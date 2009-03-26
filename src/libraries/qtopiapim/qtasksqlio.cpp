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

#include "qtask.h"
#include "qtaskmodel.h"
#include "qpimsource.h"
#include <qtopiaipcenvelope.h>
#include "qtasksqlio_p.h"
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

// both... until I can get all the required widgets sorted out.
#include <qcategorymanager.h>

#include <QString>
#include <QSqlError>
#include <QTimer>

#include <QDebug>

// For task due date event feature:
#include "qappointment.h"
#include "qappointmentsqlio_p.h"
#include "qdependentcontexts_p.h"
#include "qpimdependencylist_p.h"

static const char *contextsource = "default";
QStringList QTaskSqlIO::sortColumns() const
{
    switch(cSort) {
        default:
        case QTaskModel::Invalid:
        case QTaskModel::Description:
            return QStringList() << "description"; // no tr
        case QTaskModel::Priority:
            return QStringList() << "priority"; // no tr
        case QTaskModel::Completed:
        case QTaskModel::PercentCompleted:
            return QStringList() << "percentcompleted"; // no tr
        case QTaskModel::Status:
            return QStringList() << "status"; // no tr
        case QTaskModel::StartedDate:
            return QStringList() << "started"; // no tr
        case QTaskModel::DueDate:
            return QStringList() << "due"; // no tr
        case QTaskModel::CompletedDate:
            return QStringList() << "completed"; // no tr
    }
}

QStringList QTaskSqlIO::otherFilters() const
{
    QStringList l;
    if (cCompFilter)
        l << "percentcompleted != 100";
    return l;
}

QTaskSqlIO::QTaskSqlIO(QObject *parent, const QString &)
    : QPimSqlIO(parent, contextId(), "tasks", "taskcategories", "taskcustom",
            "description = :description, status = :status, priority = :priority, "
            "percentcompleted = :pc, due = :due, started = :started, completed = :completed",
            " (recid, context, description, status, priority, percentcompleted, due, started, completed"
            ") VALUES (:i, :context, :description, :status, :priority, :pc, :due, :started, "
            ":completed)",
            "PIM/Tasks"),
    cCompFilter(false), cSort(QTaskModel::Description), taskByRowValid(false),
    repeatFieldQuery("SELECT alarm, repeatrule, repeatenddate, repeatfrequency, repeatweekflags FROM appointments WHERE recid = (SELECT destrecid FROM pimdependencies WHERE srcrecid = :id AND deptype = 'duedate')")
{
    QStringList sortColumns;
    sortColumns << "completed";
    sortColumns << "priority";
    sortColumns << "description";
    QPimSqlIO::setOrderBy(sortColumns);
}

QTaskSqlIO::~QTaskSqlIO()
{
}

QUuid QTaskSqlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("10b16464-b37b-4e0b-9f15-7a5d895b70c6");
    return u;
}

void QTaskSqlIO::bindFields(const QPimRecord& r, QPreparedSqlQuery &q) const
{
    const QTask &t = (const QTask &)r;
    q.bindValue(":description", t.description());
    q.bindValue(":status", t.status());
    q.bindValue(":priority", t.priority());
    q.bindValue(":pc", t.percentCompleted());
    q.bindValue(":due", t.dueDate());
    q.bindValue(":started", t.startedDate());
    q.bindValue(":completed", t.completedDate());
}

// by uid doesn't neeed caching... always fast and unlikely to be in order?
QTask QTaskSqlIO::task( const QUniqueId & u ) const
{
    if (taskByRowValid && u == lastTask.uid())
        return lastTask;

    QPreparedSqlQuery q(database());
    q.prepare("SELECT recid, description, priority, status, percentcompleted, due, started, completed from tasks where recid = :i");
    q.bindValue(":i", u.toUInt());

    QTask t;
    retrieveRecord(u.toUInt(), t);
    if (!q.exec()) {
        qWarning("failed to select task: %s", (const char *)q.lastError().text().toLocal8Bit());
        taskByRowValid = false;
        return t;
    }

    if ( q.next() ) {
        t.setUid(QUniqueId::fromUInt(q.value(0).toUInt()));
        t.setDescription(q.value(1).toString());
        t.setPriority((QTask::Priority)q.value(2).toInt());
        t.setStatus((QTask::Status)q.value(3).toInt());
        t.setPercentCompleted(q.value(4).toInt());
        if (!q.isNull(5))
            t.setDueDate(q.value(5).toDate());
        else
            t.clearDueDate();
        if (!q.isNull(6))
            t.setStartedDate(q.value(6).toDate());
        if (!q.isNull(7))
            t.setCompletedDate(q.value(7).toDate());

        lastTask = t;
        taskByRowValid = true;
    } else {
        taskByRowValid = false;
    }
    return t;
}

void QTaskSqlIO::setCompletedFilter(bool b)
{
    if (cCompFilter != b) {
        cCompFilter = b;
        if (cCompFilter)
            QPimSqlIO::setFilter(QLatin1String("percentcompleted != 100"));
        else
            QPimSqlIO::clearFilter();
    }
}

void QTaskSqlIO::setSortKey(QTaskModel::Field s)
{
    if (cSort != s) {
        cSort = s;
        invalidateCache();
    }
}

bool QTaskSqlIO::completedFilter() const
{
    return cCompFilter;
}


QTaskModel::Field QTaskSqlIO::sortKey() const
{
    return cSort;
}

void QTaskSqlIO::invalidateCache()
{
    QPimSqlIO::invalidateCache();
    taskByRowValid = false;
}

QTask QTaskSqlIO::task(int row) const
{
    // caching belongs in task lookup.
    return task(id(row));
}

QVariant QTaskSqlIO::taskField(int row, QTaskModel::Field k) const
{
    // only handle the repeat field stuff, fall back for the other stuff
    QVariant ret;
    switch(k)
    {
        // assumes that the enums are in order, consecutive and
        // match the order of the query.
        case QTaskModel::Alarm:
        case QTaskModel::RepeatRule:
        case QTaskModel::RepeatEndDate:
        case QTaskModel::RepeatFrequency:
        case QTaskModel::RepeatWeekFlags:
            repeatFieldQuery.prepare();
            repeatFieldQuery.bindValue(":id", id(row).toUInt());
            repeatFieldQuery.exec();
            ret = repeatFieldQuery.value(k-QTaskModel::Alarm);
            repeatFieldQuery.reset();
            if (k == QTaskModel::RepeatEndDate)
                ret.convert(QVariant::Date);
            else
                ret.convert(QVariant::Int);
            return ret;
        default:
            break;
    }

    QTask t = task(row);
    switch(k) {
        default:
        case QTaskModel::Invalid:
            break;
        case QTaskModel::Identifier:
            return t.uid().toByteArray();
        case QTaskModel::Categories:
            return QVariant(t.categories());
        case QTaskModel::Description:
            return t.description();
        case QTaskModel::Priority:
            return t.priority();
        case QTaskModel::Completed:
            return t.isCompleted();
        case QTaskModel::PercentCompleted:
            return t.percentCompleted();
        case QTaskModel::Status:
            return t.status();
        case QTaskModel::DueDate:
            return t.dueDate();
        case QTaskModel::StartedDate:
            return t.startedDate();
        case QTaskModel::CompletedDate:
            return t.completedDate();
        case QTaskModel::Notes:
            return t.notes();
    }
    return QVariant();
}

bool QTaskSqlIO::setTaskField(int row, QTaskModel::Field k,  const QVariant &v)
{
    QTask t = task(row);
    switch(k) {
        default:
        case QTaskModel::Invalid:
            break;
        case QTaskModel::Categories:
            if (v.canConvert(QVariant::StringList)) {
                t.setCategories(v.toStringList());
                return updateTask(t);
            }
            return false;
        case QTaskModel::Description:
            if (v.canConvert(QVariant::String)) {
                t.setDescription(v.toString());
                return updateTask(t);
            }
            return false;
        case QTaskModel::Priority:
            if (v.canConvert(QVariant::Int)) {
                t.setPriority(v.toInt());
                return updateTask(t);
            }
            return false;
        case QTaskModel::Completed:
            if (v.canConvert(QVariant::Bool)) {
                t.setCompleted(v.toBool());
                return updateTask(t);
            }
            return false;
        case QTaskModel::PercentCompleted:
            if (v.canConvert(QVariant::Int)) {
                t.setPercentCompleted(v.toInt());
                return updateTask(t);
            }
            return false;
        case QTaskModel::Status:
            if (v.canConvert(QVariant::Int)) {
                t.setStatus(v.toInt());
                return updateTask(t);
            }
            return false;
        case QTaskModel::DueDate:
            if (v.canConvert(QVariant::Date)) {
                t.setDueDate(v.toDate());
                return updateTask(t);
            }
            return false;
        case QTaskModel::StartedDate:
            if (v.canConvert(QVariant::Date)) {
                t.setStartedDate(v.toDate());
                return updateTask(t);
            }
            return false;
        case QTaskModel::CompletedDate:
            if (v.canConvert(QVariant::Date)) {
                t.setCompletedDate(v.toDate());
                return updateTask(t);
            }
            return false;
        case QTaskModel::Notes:
            if (v.canConvert(QVariant::String)) {
                t.setNotes(v.toString());
                return updateTask(t);
            }
            return false;
    }
    return false;
}

bool QTaskSqlIO::removeTask(int row)
{
    QUniqueId u = id(row);
    return removeTask(u);
}

bool QTaskSqlIO::removeTask(const QUniqueId & id)
{
    if (id.isNull())
        return false;

    if (QPimSqlIO::removeRecord(id)) {
        return true;
    }
    return false;
}

bool QTaskSqlIO::removeTask(const QTask &t)
{
    return removeTask(t.uid());
}

bool QTaskSqlIO::removeTasks(const QList<int> &rows)
{
    return removeTasks(ids(rows));
}

bool QTaskSqlIO::removeTasks(const QList<QUniqueId> &ids)
{
    if (QPimSqlIO::removeRecords(ids)) {
        return true;
    }
    return false;
}

bool QTaskSqlIO::updateTask(const QTask &t)
{
    if (QPimSqlIO::updateRecord(t)) {
        return true;
    }
    return false;
}

QUniqueId QTaskSqlIO::addTask(const QTask &task, const QPimSource &source, bool createuid)
{
    QPimSource s;
    s.identity = contextsource;
    s.context = contextId();
    QUniqueId i = addRecord(task, source.isNull() ? s : source, createuid);
    if (!i.isNull()) {
        QTask added = task;
        added.setUid(i);
    }
    return i;
}

/***************
 * CONTEXT
 **************/
class QTaskDefaultContextData
{
public:
    QTaskDefaultContextData(QTaskDefaultContext *parent) :
        mAppointmentAccess(0),
        mTaskEventContext(0),
        mDependencyList(0),
        mFindRecurringTasks("SELECT tasks.recid,appointments.recid FROM tasks INNER JOIN pimdependencies ON pimdependencies.srcrecid = tasks.recid INNER JOIN appointments ON appointments.recid=pimdependencies.destrecid WHERE pimdependencies.deptype = 'duedate' AND tasks.completed IS NOT NULL AND appointments.repeatrule <> 0 AND tasks.due < :today AND (appointments.repeatenddate IS NULL or appointments.repeatenddate >= :today2)"),
        mFindNextRecurringDueDate("SELECT tasks.due FROM tasks INNER JOIN pimdependencies ON pimdependencies.srcrecid = tasks.recid INNER JOIN appointments ON appointments.recid=pimdependencies.destrecid WHERE pimdependencies.deptype = 'duedate' AND tasks.completed IS NOT NULL AND appointments.repeatrule <> 0 AND (appointments.repeatenddate IS NULL OR appointments.repeatenddate >= :today) AND tasks.due >= :today2 ORDER BY tasks.due LIMIT 1"),
        q(parent)
    {}

    // Lazy
    QDependentAppointmentSqlIO* appointmentAccess()
    {
        if (!mAppointmentAccess)
            mAppointmentAccess = new QDependentAppointmentSqlIO(q);
        return mAppointmentAccess;
    }

    QTaskEventContext* taskEventContext()
    {
        if (!mTaskEventContext)
            mTaskEventContext = new QTaskEventContext(q, appointmentAccess());
        return mTaskEventContext;
    }

    // Helper functions
    void avoidNestedTransactions()
    {
        appointmentAccess()->setCurrentSyncTransactionTime(mAccess->currentSyncTransactionTime());
    }

    bool addTaskEvent(const QUniqueId &taskId, const QTask &task, const QString& type)
    {
        bool ret = false;
        QAppointment a;
        QDate date = task.dueDate();
        QUniqueId newId;
        QDependentEventsContext *context = taskEventContext();
        uint newContextId = context->mappedContext();

        // XXX this should go into QUniqueId
        newId = QUniqueId::fromUInt(newContextId << 24 | taskId.toUInt() & 0x00ffffff);

        a.setStart(QDateTime(date, QTime(0,0)));
        a.setEnd(QDateTime(date, QTime(23,59)));
        a.setAllDay(true);
        a.setDescription(task.description());
        a.setUid(newId);

        QUniqueId added = context->addAppointment(a, context->defaultSource());
        if (!added.isNull()) {
            if (QPimDependencyList::addDependency(taskId, added, type))
                ret = true;
            else {
                // Sigh, remove the appointment (XXX transactions)
                context->removeAppointment(added);
            }
        }

        return ret;
    }

    void processRecurringTasks()
    {
        mFindRecurringTasks.prepare();
        QDate today = QDate::currentDate();
        mFindRecurringTasks.bindValue(":today", today);
        mFindRecurringTasks.bindValue(":today2", today);

        if (!mFindRecurringTasks.exec()) {
            qWarning("Failed to execute task field query: %s", mFindRecurringTasks.lastError().text().toLocal8Bit().constData());
        } else {
            QUniqueId taskId, apptId;
            QAppointment a;
            QTask t;
            QDate d;

            // Can't update while a select is in progress, so queue them
            QList<QTask> updatedTasks;

            while (mFindRecurringTasks.next()) {
                apptId = QUniqueId::fromUInt(mFindRecurringTasks.value(1).toUInt());
                QAppointment a = appointmentAccess()->appointment(apptId);
                QDate d = a.nextOccurrence(today).startInCurrentTZ().date();

                if (d.isValid()) {
                    taskId = QUniqueId::fromUInt(mFindRecurringTasks.value(0).toUInt());
                    t = mAccess->task(taskId);
                    if (t.dueDate() != d) {
                        t.setDueDate(d);
                        t.setStatus(QTask::NotStarted);
                        updatedTasks.append(t);
                    }
                }
            }

            mFindRecurringTasks.reset();

            // Now actually update the tasks
            foreach (QTask t, updatedTasks) {
                mAccess->updateTask(t);
            }
        }
        scheduleNextRecurringDueDate();
    }

    void scheduleNextRecurringDueDate()
    {
        // Find out if we have any existing alarms.
        QSettings c("Trolltech", "PIM");
        QDate oldAlarm = c.value("NextRecurringTaskAlarm").toDate();
        QDate alarm;

        mFindNextRecurringDueDate.prepare();
        QDate today = QDate::currentDate();
        mFindNextRecurringDueDate.bindValue(":today", today);
        mFindNextRecurringDueDate.bindValue(":today2", today);

        if (!mFindNextRecurringDueDate.exec()) {
            qWarning("Failed to execute task field query: %s", mFindNextRecurringDueDate.lastError().text().toLocal8Bit().constData());
        } else {
            if (mFindNextRecurringDueDate.next()) {
                alarm = mFindNextRecurringDueDate.value(0).toDate();

                // Schedule the alarm for the start of the following day
                if (alarm.isValid())
                    alarm = alarm.addDays(1);
            }
            mFindNextRecurringDueDate.reset();
            if (alarm != oldAlarm) {
                if (oldAlarm.isValid())
                    Qtopia::deleteAlarm(QDateTime(oldAlarm), "Tasks", "updateRecurringTasks(QDateTime,int)");

                if (!alarm.isValid()) {
                    c.remove("NextRecurringTaskAlarm");
                } else {
                    // New alarm, remove the old, set the new
                    Qtopia::addAlarm(QDateTime(alarm), "Tasks", "updateRecurringTasks(QDateTime,int)");
                    c.setValue("NextRecurringTaskAlarm", alarm);
                }
            }
        }
    }

    QTaskSqlIO *mAccess;

    QDependentAppointmentSqlIO *mAppointmentAccess;
    QTaskEventContext *mTaskEventContext;
    QPimDependencyList *mDependencyList;
    QPreparedSqlQuery mFindRecurringTasks;
    QPreparedSqlQuery mFindNextRecurringDueDate;

    QTaskDefaultContext* q;
};


QTaskDefaultContext::QTaskDefaultContext(QObject *parent, QObject *access)
    : QTaskContext(parent)
{
    d = new QTaskDefaultContextData(this);
    d->mAccess = qobject_cast<QTaskSqlIO *>(access);
    Q_ASSERT(d->mAccess);
}

QIcon QTaskDefaultContext::icon() const
{
    return QPimContext::icon(); // redundent, but will do for now.
}

QString QTaskDefaultContext::description() const
{
    return tr("Default task storage");
}

QString QTaskDefaultContext::title() const
{
    return tr("Tasks");
}

bool QTaskDefaultContext::editable() const
{
    return true;
}

QSet<QPimSource> QTaskDefaultContext::sources() const
{
    QSet<QPimSource> list;
    list.insert(defaultSource());
    return list;
}

QUuid QTaskDefaultContext::id() const
{
    return d->mAccess->contextId();
}

/* TODO set mapping to int */
void QTaskDefaultContext::setVisibleSources(const QSet<QPimSource> &set)
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = d->mAccess->contextFilter();

    if (set.contains(defaultSource()))
        filter.remove(context);
    else
        filter.insert(context);

    d->mAccess->setContextFilter(filter);
}

QSet<QPimSource> QTaskDefaultContext::visibleSources() const
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = d->mAccess->contextFilter();
    if (!filter.contains(context))
        return sources();
    return QSet<QPimSource>();
}

bool QTaskDefaultContext::exists(const QUniqueId &id) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    return d->mAccess->exists(id) && d->mAccess->context(id) == context;
}

QPimSource QTaskDefaultContext::defaultSource() const
{
    QPimSource s;
    s.context = d->mAccess->contextId();
    s.identity = contextsource;
    return s;
}

QPimSource QTaskDefaultContext::source(const QUniqueId &id) const
{
    if (exists(id))
        return defaultSource();
    return QPimSource();
}

bool QTaskDefaultContext::updateTask(const QTask &task)
{
    bool ret = d->mAccess->updateTask(task);

    //
    // We have to update some of the dependent information:
    //  - due date appointments
    //  - recurring task due dates
    //
    if (ret) {
        QUniqueId apptId = QPimDependencyList::typedChildrenRecords(task.uid(), QString("duedate")).value(0);
        if (task.hasDueDate()) {
            d->avoidNestedTransactions();
            if (apptId.isNull()) {
                d->addTaskEvent(task.uid(), task, "duedate");
            } else {
                QDate due = task.dueDate();
                QAppointment a = d->appointmentAccess()->appointment(apptId);

                a.setDescription(task.description());
                // In the presence of recurring events, we can't just modify this
                // If the new due date is an occurrence of the original appointment,
                // we don't adjust the appointment start dates etc.
                if (a.nextOccurrence(due).startInCurrentTZ().date() != due) {
                    a.setStart(QDateTime(due, QTime(0,0)));
                    a.setEnd(QDateTime(due, QTime(23,59)));
                    a.setAllDay(true);
                }
                d->taskEventContext()->updateAppointment(a);

                // Completed tasks with due dates might be recurring
                // This may result in two update qcops, if the
                // task was marked complete after the due date,
                // but that is conceptually correct anyway (completed,
                // then new due date)

                // This is similar logic to d->processRecurringTasks
                if (a.hasRepeat()) {
                    if (task.isCompleted()) {
                        QDate nextDue = a.nextOccurrence(QDate::currentDate()).startInCurrentTZ().date();
                        if (nextDue.isValid() && nextDue != due) {
                            QTask t2(task);
                            t2.setDueDate(nextDue);
                            t2.setStatus(QTask::NotStarted);
                            d->mAccess->updateTask(t2);
                        }
                    }

                    // In case this was the earliest recurring and we mark it incomplete,
                    // or to schedule the appropriate time if we mark it complete
                    d->scheduleNextRecurringDueDate();
                }
            }
        } else {
            if (!apptId.isNull()) {
                d->avoidNestedTransactions();
                if (d->taskEventContext()->removeAppointment(apptId))
                    QPimDependencyList::removeDependency(task.uid(), apptId, QString("duedate"));
            }
        }

    }

    return ret;
}

bool QTaskDefaultContext::removeTask(const QUniqueId &id)
{
    bool ret = d->mAccess->removeTask(id);
    if (ret) {
        QUniqueId appid = QPimDependencyList::typedChildrenRecords(id, QString("duedate")).value(0);
        if (!appid.isNull()) {
            if (d->taskEventContext()->removeAppointment(appid))
                QPimDependencyList::removeDependency(id, appid, QString("duedate"));

            // Just in case this was the earliest recurring date
            d->scheduleNextRecurringDueDate();
        }
    }

    return ret;
}

QUniqueId QTaskDefaultContext::addTask(const QTask &task, const QPimSource &source)
{
    if (source.isNull() || source == defaultSource()) {
        QUniqueId added = d->mAccess->addTask(task, defaultSource());
        if (!added.isNull() && task.hasDueDate()) {
            d->avoidNestedTransactions();
            d->addTaskEvent(added, task, "duedate");
        }

        return added;
    }
    return QUniqueId();
}

void QTaskDefaultContext::processRecurringTasks()
{
    d->processRecurringTasks();
}
