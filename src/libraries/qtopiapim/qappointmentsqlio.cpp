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
#include <qtopiasql.h>
#include "qappointmentsqlio_p.h"
#include <QString>

#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QTimer>

#include <qtopianamespace.h>
#include <qtopiaipcenvelope.h>
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

static const char *contextsource = "default";
/*
data fields

description location start end allday starttimezone alarm alarmdelay repeatrule repeatfrequency repeatenddate repeatweekflags

:d, :l, :s, :e, :a, :stz, :n, :at, :ad, :rr, :rf, :re, :rw

*/

QAppointmentSqlIO::QAppointmentSqlIO(QObject *parent)
    : QPimSqlIO( parent, contextId(),
            "appointments", "appointmentcategories", "appointmentcustom",
            "description = :d, location = :l, start = :s, end = :e, "
            "allday = :a, starttimezone = :stz, "
            "alarm = :at, alarmdelay = :ad, "
            "repeatrule = :rr, repeatfrequency = :rf, "
            "repeatenddate = :re, "
            "repeatweekflags = :rw",
            "(recid, context, description, location, start, end, allday, "
            "starttimezone, alarm, alarmdelay, repeatrule, "
            "repeatfrequency, repeatenddate, "
            "repeatweekflags) VALUES (:i, :context, :d, :l, :s, :e, :a, :stz, "
            ":at, :ad, :rr, :rf, :re, :rw)",
            "PIM/Appointments"),
            lastAppointmentStatus(Empty),
            lastRow(-1),
            rType(QAppointmentModel::AnyDuration),
            mainTable("appointments"),
            catTable("appointmentcategories"),
            customTable("appointmentcustom"),
            exceptionTable("appointmentexceptions"),
            appointmentQuery("SELECT recid, description, location, start, end, allday, "
            "starttimezone, alarm, alarmdelay, repeatrule, "
            "repeatfrequency, repeatenddate, repeatweekflags "
            "FROM appointments WHERE recid = :i"),
            exceptionsQuery("SELECT edate, alternateid FROM appointmentexceptions WHERE recid=:id"),
            parentQuery("SELECT recid, edate FROM appointmentexceptions WHERE alternateid=:id")
{
     QStringList sort;
     sort << "start";
     sort << "end";
     sort << "repeatenddate";
     QPimSqlIO::setOrderBy(sort);
}


QAppointmentSqlIO::~QAppointmentSqlIO()
{
}

bool QAppointmentSqlIO::removeAppointment(const QUniqueId &id)
{
    if (id.isNull())
        return false;

    QAppointment original = appointment(id, true);
    if (original.isValid() && original.hasAlarm())
        removeAlarm(original);

    if (original.isException()) {
        // Removing an exception triggers an update of the exception parent (for the changelog)
        updateAppointment(this->appointment(original.exceptionParent()));
    } else {
        // Remove any exceptions first (or they'll hang around)
        QList<QAppointment::Exception> exceptions = original.exceptions();
        foreach(const QAppointment::Exception &e, exceptions) {
            if (!e.alternative.isNull()) {
                bool ok = removeAppointment(e.alternative);
                if (!ok) {
                    qWarning() << "Coult not remove exception" << e.alternative.toString() << "for appointment" << id.toString();
                }
            }
        }
    }

    if (QPimSqlIO::removeRecord(id)) {
        return true;
    }
    return false;
}

bool QAppointmentSqlIO::removeAppointments(const QList<QUniqueId> ids)
{
    foreach(QUniqueId id, ids) {
        QAppointment orig = appointment(id, true);
        if (orig.isValid() && orig.hasAlarm())
            removeAlarm(orig);
    }

    if (QPimSqlIO::removeRecords(ids)) {
        return true;
    }
    return false;
}

bool QAppointmentSqlIO::removeAppointment(int row)
{
    return removeAppointment(id(row));
}

bool QAppointmentSqlIO::updateAppointment(const QAppointment& appointment)
{
    QAppointment original = this->appointment(appointment.uid());
    if (original.isValid() && original.hasAlarm())
        removeAlarm(original);

    if (QPimSqlIO::updateRecord(appointment)) {
        addAlarm(appointment);
        return true;
    }
    return false;
}

bool QAppointmentSqlIO::setAppointmentField(int row, QAppointmentModel::Field k,  const QVariant &v)
{
    QAppointment t = appointment(row);
    if (QAppointmentModel::setAppointmentField(t, k, v))
        return updateAppointment(t);
    return false;
}

void QAppointmentSqlIO::refresh()
{
    invalidateCache();
}

void QAppointmentSqlIO::checkAdded(const QUniqueId &) { refresh(); }
void QAppointmentSqlIO::checkRemoved(const QUniqueId &) { refresh(); }
void QAppointmentSqlIO::checkRemoved(const QList<QUniqueId> &) { refresh(); }
void QAppointmentSqlIO::checkUpdated(const QUniqueId &) { refresh(); }

QUniqueId QAppointmentSqlIO::addAppointment(const QAppointment& appointment, const QPimSource &source, bool createuid)
{
    QPimSource s;
    s.identity = contextsource;
    s.context = contextId();
    QUniqueId i = addRecord(appointment, source.isNull() ? s : source, createuid);
    if (!i.isNull() && appointment.hasAlarm()) {
        QAppointment added = appointment;
        added.setUid(i);
        if (added.isValid())
            addAlarm(added);

    }
    return i;
}

bool QAppointmentSqlIO::updateExtraTables(uint, const QPimRecord &)
{
    return true; // not actually adding exceptions directly at the moment.
}

bool QAppointmentSqlIO::insertExtraTables(uint, const QPimRecord &)
{
    return true; // not actually adding exceptions directly at the moment.
}

bool QAppointmentSqlIO::removeExtraTables(uint uid)
{
    QPreparedSqlQuery q(database());
    q.prepare("DELETE FROM " + exceptionTable + " WHERE recid=:ida OR alternateid=:idb");

    q.bindValue(":ida", uid);
    q.bindValue(":idb", uid);
    if (!q.exec())
        return false;
    return true;
}

bool QAppointmentSqlIO::removeOccurrence(const QUniqueId &original,
        const QDate &date)
{
    QPreparedSqlQuery q(database());
    q.prepare("INSERT INTO " + exceptionTable + " (recid, edate) VALUES (:i, :ed)");

    q.bindValue(":i", original.toUInt());
    q.bindValue(":ed", date);
    if( !q.exec() )
        return false;

    // Removing an occurrence triggers an update of the parent appointment (for the changelog)
    updateAppointment(this->appointment(original));

    propagateChanges();
    return true;
}

bool QAppointmentSqlIO::restoreOccurrence(const QUniqueId &identifier,
        const QDate &date)
{
    QPreparedSqlQuery q(database());
    q.prepare("SELECT alternateid FROM appointmentexceptions WHERE recid = :i AND edate = :d");
    q.bindValue(":i", identifier.toUInt());
    q.bindValue(":d", date);

    if (!q.exec() || !q.next())
        return false; // no occurrence to restore.

    if (startTransaction())
    {

        QUniqueId replacementId(QUniqueId::fromUInt(q.value(0).toUInt()));

        if (replacementId.isNull() || removeAppointment(replacementId))
        {
            q.prepare("DELETE FROM appointmentexceptions WHERE recid = :i AND edate = :d");
            q.bindValue(":i", identifier.toUInt());
            q.bindValue(":d", date);
            if (q.exec())
                return commitTransaction();
        }
        abortTransaction();
    }

    return false;
}

QUniqueId QAppointmentSqlIO::replaceOccurrence(const QUniqueId &original,
        const QOccurrence &occurrence, const QDate& occurrenceDate)
{
    // TODO should do some checking to make sure request is reasonable.
    QDate date = occurrenceDate;
    if (date.isNull())
        date = occurrence.date();
    QAppointment a = occurrence.appointment();
    a.setRepeatRule(QAppointment::NoRepeat);
    int c = context(original);

    if (startTransaction())
    {
        QUniqueId u = addRecord(a, c, true);
        if (!u.isNull()) {
            QAppointment added = appointment(u);
            if (added.isValid() && added.hasAlarm())
                addAlarm(added);

            QPreparedSqlQuery q(database());
            q.prepare("INSERT INTO " + exceptionTable + " (recid, edate, alternateid)"
                    " VALUES (:i, :ed, :aid)");
            q.bindValue(":i", original.toUInt());
            q.bindValue(":ed", date);
            q.bindValue(":aid", u.toUInt());

            if(q.exec() && commitTransaction())
                return u;
        }
        abortTransaction();
    }
    return QUniqueId();
}

QUniqueId QAppointmentSqlIO::replaceRemaining(const QUniqueId &original,
        const QAppointment &remaining, const QDate& endDate)
{
    // TODO make sure that exceptions transferred to the replacement are sane for possible change in repeat days

    QAppointment ao = appointment(original);
    QDate date = endDate;
    if (date.isNull())
        date = remaining.start().date();
    ao.setRepeatUntil(date.addDays(-1));

    // will fail if 'remaining' isn't valid.  However in this case,
    // really should not be using this function anyway.
    // Its an assertion that the appointment would be valid to add.
    if (startTransaction())
    {
        int c = context(original);
        QUniqueId u = addRecord(remaining, c, true);
        if (updateAppointment(ao) && !u.isNull())
        {
            QAppointment added = appointment(u);
            if (added.isValid() && added.hasAlarm())
                addAlarm(added);

            //  Transfer any exceptions after the switchover date to the replacement appointment
            QPreparedSqlQuery q(database());
            q.prepare("UPDATE " + exceptionTable + " SET recid=:newid WHERE recid=:oldid AND edate >= :d");

            q.bindValue(":newid", u.toUInt());
            q.bindValue(":oldid", original.toUInt());
            q.bindValue(":d", remaining.start().date());

            if (q.exec() && commitTransaction())
                return u;
        }
        abortTransaction();
    }
    return QUniqueId();
}

QUuid QAppointmentSqlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("5ecdd517-9aed-4e4b-b248-1970c56eb49a");
    return u;
}

void QAppointmentSqlIO::setRangeFilter(const QDateTime &earliest, const QDateTime &latest)
{
    if (rStart == earliest && rEnd == latest)
        return;
    rStart = earliest;
    rEnd = latest;
    setFilters(currentFilters());
}

QStringList QAppointmentSqlIO::currentFilters() const
{
    QStringList f;
    if (rStart.isValid())
        f.append("((repeatenddate IS NULL OR repeatenddate >= '" + rStart.date().toString(Qt::ISODate)
            + "') OR end >= '" + rStart.toString(Qt::ISODate) + "')");

    if (rEnd.isValid())
        f.append("start < '" + rEnd.toString(Qt::ISODate) + "'");

    switch(rType){
        case QAppointmentModel::AnyDuration:
            break;
        case QAppointmentModel::AllDayDuration:
            f.append("allday = 'true'");
            break;
        case QAppointmentModel::TimedDuration:
            f.append("allday = 'false'");
            break;
    }
    return f;
}

    // assume, not using filter via setRangeFilter
/*
   Optimized for sqlite, to properly take advantage of end index
   and avoid wanted row->id and count calls.

   start must be valid, access must have no existing range restriction
   count will limit the results to only contain count non-repeating events.
   count for repeating events isn't useful from this function and
   all repeating events that may fall in the range will be returned.
   
*/
QList<QAppointment> QAppointmentSqlIO::fastRange(const QDateTime &start, const QDateTime &end, int count) const
{
    QList<QAppointment> result;
    if (!start.isValid() || (!end.isValid() && count <= 0))
        return result;

    QStringList nonRepeatFilter, repeatFilter;

    nonRepeatFilter.append("repeatrule = 0");
    repeatFilter.append("repeatrule > 0");

    nonRepeatFilter.append("end >= '" + start.toString(Qt::ISODate) + "'");
#if 0
    /*if we had a duration column, doing this rather than the line above
      would improve preformance in the middle of the data set,
      not just the end
      */
    nonRepeatFilter.append("end >= '" + start.toString(Qt::ISODate) + "'");
    nonRepeatFilter.append("end < '" + end.addDays(1).toString(Qt::ISODate) + "'");
#endif

    repeatFilter.append("(repeatenddate IS NULL OR repeatenddate >= '" + start.date().toString(Qt::ISODate) + "')");

    if (end.isValid()) {
        nonRepeatFilter.append("start < '" + end.toString(Qt::ISODate) + "'");
        repeatFilter.append("start < '" + end.toString(Qt::ISODate) + "'");
    }

    QPreparedSqlQuery q(database());
    static const QLatin1String queryFields("t1.recid, t1.description, t1.location, t1.'start', t1.'end', t1.allday, t1.starttimezone, t1.alarm, t1.alarmdelay, t1.repeatrule, t1.repeatfrequency, t1.repeatenddate, t1.repeatweekflags");

    if (count > 0) {
        q.prepare(selectText(queryFields, nonRepeatFilter) + " ORDER BY \"start\" LIMIT " + QString::number(count));
    } else {
        q.prepare(selectText(queryFields, nonRepeatFilter));
    }
    q.exec();
    while(q.next()) {
        QAppointment a;
        uint uid = q.value(0).toUInt();
        a.setUid(QUniqueId::fromUInt(uid));
        a.setDescription(q.value(1).toString());
        a.setLocation(q.value(2).toString());
        a.setStart(q.value(3).toDateTime());
        a.setEnd(q.value(4).toDateTime());
        a.setAllDay(q.value(5).toBool());
        a.setTimeZone(QTimeZone(q.value(6).toString().toLocal8Bit().constData()));

        QAppointment::AlarmFlags af = (QAppointment::AlarmFlags)q.value(7).toInt();
        if (af != QAppointment::NoAlarm)
            a.setAlarm(q.value(8).toInt(), af);

        a.setRepeatRule((QAppointment::RepeatRule)q.value(9).toInt());
        a.setFrequency(q.value(10).toInt());
        a.setRepeatUntil(q.value(11).toDate());
        a.setWeekFlags((QAppointment::WeekFlags)q.value(12).toInt());

        exceptionsQuery.prepare();
        exceptionsQuery.bindValue(":id", uid);
        exceptionsQuery.exec();

        QList<QAppointment::Exception> elist;
        while(exceptionsQuery.next()) {
            QAppointment::Exception ae;
            ae.date = exceptionsQuery.value(0).toDate();
            if (!exceptionsQuery.value(1).isNull())
                ae.alternative = QUniqueId::fromUInt(exceptionsQuery.value(1).toUInt());
            elist.append(ae);
        }
        a.setExceptions(elist);

        exceptionsQuery.reset();

        result.append(a);

        if (count > 0 && result.count() >= count)
            break;
    }

    q.prepare(selectText(queryFields, repeatFilter));
    q.exec();
    while(q.next()) {
        QAppointment a;
        uint uid = q.value(0).toUInt();
        a.setUid(QUniqueId::fromUInt(uid));
        a.setDescription(q.value(1).toString());
        a.setLocation(q.value(2).toString());
        a.setStart(q.value(3).toDateTime());
        a.setEnd(q.value(4).toDateTime());
        a.setAllDay(q.value(5).toBool());
        a.setTimeZone(QTimeZone(q.value(6).toString().toLocal8Bit().constData()));

        QAppointment::AlarmFlags af = (QAppointment::AlarmFlags)q.value(7).toInt();
        if (af != QAppointment::NoAlarm)
            a.setAlarm(q.value(8).toInt(), af);

        a.setRepeatRule((QAppointment::RepeatRule)q.value(9).toInt());
        a.setFrequency(q.value(10).toInt());
        a.setRepeatUntil(q.value(11).toDate());
        a.setWeekFlags((QAppointment::WeekFlags)q.value(12).toInt());

        exceptionsQuery.prepare();
        exceptionsQuery.bindValue(":id", uid);
        exceptionsQuery.exec();

        QList<QAppointment::Exception> elist;
        while(exceptionsQuery.next()) {
            QAppointment::Exception ae;
            ae.date = exceptionsQuery.value(0).toDate();
            if (!exceptionsQuery.value(1).isNull())
                ae.alternative = QUniqueId::fromUInt(exceptionsQuery.value(1).toUInt());
            elist.append(ae);
        }
        a.setExceptions(elist);

        exceptionsQuery.reset();

        result.append(a);
        if (count > 0 && result.count() >= count)
            break;
    }

    return result;
}


void QAppointmentSqlIO::setDurationType(QAppointmentModel::DurationType type)
{
    if (type == rType)
        return;
    rType = type;
    setFilters(currentFilters());
}

QAppointment QAppointmentSqlIO::appointment(const QUniqueId &u) const
{
    return appointment(u, false);
}

QAppointment QAppointmentSqlIO::appointment(const QUniqueId &u, bool minimal) const
{
    if (u == lastAppointment.uid()
            && ((minimal && lastAppointmentStatus != Empty)
                ||
                (lastAppointmentStatus == Full))
               )
    {
        return lastAppointment;
    }

    uint uid = u.toUInt();

    QAppointment t;

    if (!minimal)
        retrieveRecord(uid, t);

    appointmentQuery.prepare();

    appointmentQuery.bindValue(":i", uid);

    if (!appointmentQuery.exec()) {
        lastAppointmentStatus = Empty;
        return t;
    }

    if ( appointmentQuery.next() ) {

        t.setUid(QUniqueId::fromUInt(appointmentQuery.value(0).toUInt()));
        t.setDescription(appointmentQuery.value(1).toString());
        t.setLocation(appointmentQuery.value(2).toString());
        t.setStart(appointmentQuery.value(3).toDateTime());
        t.setEnd(appointmentQuery.value(4).toDateTime());
        t.setAllDay(appointmentQuery.value(5).toBool());
        t.setTimeZone(QTimeZone(appointmentQuery.value(6).toString().toLocal8Bit().constData()));

        QAppointment::AlarmFlags af = (QAppointment::AlarmFlags)appointmentQuery.value(7).toInt();
        if (af != QAppointment::NoAlarm)
            t.setAlarm(appointmentQuery.value(8).toInt(), af);

        t.setRepeatRule((QAppointment::RepeatRule)appointmentQuery.value(9).toInt());
        t.setFrequency(appointmentQuery.value(10).toInt());
        t.setRepeatUntil(appointmentQuery.value(11).toDate());
        t.setWeekFlags((QAppointment::WeekFlags)appointmentQuery.value(12).toInt());

        appointmentQuery.reset();

        exceptionsQuery.prepare();
        exceptionsQuery.bindValue(":id", uid);
        exceptionsQuery.exec();
        QList<QAppointment::Exception> elist;
        while(exceptionsQuery.next()) {
            QAppointment::Exception ae;
            ae.date = exceptionsQuery.value(0).toDate();
            if (!exceptionsQuery.value(1).isNull())
                ae.alternative = QUniqueId::fromUInt(exceptionsQuery.value(1).toUInt());
            elist.append(ae);
        }
        t.setExceptions(elist);

        exceptionsQuery.reset();

        parentQuery.prepare();
        parentQuery.bindValue(":id", uid);
        parentQuery.exec();
        if(parentQuery.next())
            t.setAsException(QUniqueId::fromUInt(parentQuery.value(0).toUInt()), parentQuery.value(1).toDate());

        parentQuery.reset();

        lastAppointment = t;
        if (minimal)
            lastAppointmentStatus = Minimal;
        else
            lastAppointmentStatus = Full;
        lastRow = -1; // will be corrected in appointmentField if need be.
    } else {
        appointmentQuery.reset();
        lastAppointmentStatus = Empty;
    }
    return t;
}

QAppointment QAppointmentSqlIO::appointment(int row) const
{
    return appointment(id(row));
}

QAppointment QAppointmentSqlIO::appointment(int row, bool minimal) const
{
    return appointment(id(row), minimal);
}

QVariant QAppointmentSqlIO::appointmentField(int row, QAppointmentModel::Field k) const
{
    switch (k) {
        case QAppointmentModel::Invalid:
            break;
        case QAppointmentModel::Description:
        case QAppointmentModel::Location:
        case QAppointmentModel::Start:
        case QAppointmentModel::End:
        case QAppointmentModel::AllDay:
        case QAppointmentModel::TimeZone:
        case QAppointmentModel::Alarm:
        case QAppointmentModel::RepeatRule:
        case QAppointmentModel::RepeatFrequency:
        case QAppointmentModel::RepeatEndDate:
        case QAppointmentModel::RepeatWeekFlags:
        case QAppointmentModel::Identifier:
            {
                if (lastAppointmentStatus != Empty && lastRow == row)
                    return QAppointmentModel::appointmentField(lastAppointment, k);

                QVariant v =  QAppointmentModel::appointmentField(appointment(id(row), true), k);
                if (v.isValid())
                    lastRow = row;
                return v;
            }
        case QAppointmentModel::Categories:
        case QAppointmentModel::Notes:
            {
                if (lastAppointmentStatus == Full && lastRow == row)
                    return QAppointmentModel::appointmentField(lastAppointment, k);

                QVariant v = QAppointmentModel::appointmentField(appointment(id(row), false), k);
                if (v.isValid())
                    lastRow = row;
                return v;
            }
    }
    return QAppointmentModel::appointmentField(appointment(row), k);
}

bool QAppointmentSqlIO::nextAlarm(QDateTime &when, QUniqueId &) const
{
    mAlarmStart = when;
    return false;
}

void QAppointmentSqlIO::bindFields(const QPimRecord &r, QPreparedSqlQuery &query) const
{
    const QAppointment &a = (const QAppointment &)r;
    query.bindValue(":d", a.description());
    query.bindValue(":l", a.location());
    query.bindValue(":s", a.start());
    query.bindValue(":e", a.end());
    query.bindValue(":a", a.isAllDay());
    query.bindValue(":stz", a.timeZone().id());
    query.bindValue(":at", (int)a.alarm());
    query.bindValue(":ad", a.alarmDelay());
    query.bindValue(":rr", (int)a.repeatRule());
    query.bindValue(":rf", a.frequency());
    query.bindValue(":re", a.repeatUntil());
    query.bindValue(":rw", (int)a.weekFlags());
}

QStringList QAppointmentSqlIO::sortColumns() const
{
    return QStringList() << "start";
}

void QAppointmentSqlIO::invalidateCache()
{
    QPimSqlIO::invalidateCache();
    lastAppointmentStatus = Empty;
}

QDateTime QAppointmentSqlIO::nextAlarm( const QAppointment &appointment)
{
    QDateTime now = QDateTime::currentDateTime();
    // -1 days to make room for timezone shift.
    QOccurrence o = appointment.nextOccurrence(now.date().addDays(-1));
    while (o.isValid()) {
        if (now <= o.alarmInCurrentTZ())
            return o.alarmInCurrentTZ();
        o = o.nextOccurrence();
    }
    return QDateTime();
}

// alarm functions
void QAppointmentSqlIO::removeAlarm(const QAppointment &appointment)
{
#ifdef Q_WS_QWS
    // TODO Needs to be able to set up the return to be a service.
    QDateTime when = nextAlarm(appointment);
    if (!when.isNull())
        Qtopia::deleteAlarm(when, "Calendar", "alarm(QDateTime,int)", appointment.alarmDelay());
#else
    Q_UNUSED( appointment );
#endif
}

void QAppointmentSqlIO::addAlarm(const QAppointment &appointment)
{
#ifdef Q_WS_QWS
    // TODO Needs to be able to set up the return to be a service.
    QDateTime when = nextAlarm(appointment);
    if (!when.isNull())
        Qtopia::addAlarm(when, "Calendar", "alarm(QDateTime,int)", appointment.alarmDelay());
#else
    Q_UNUSED( appointment );
#endif
}

/***************
 * CONTEXT
 **************/
QAppointmentDefaultContext::QAppointmentDefaultContext(QObject *parent, QObject *access)
    : QAppointmentContext(parent)
{
    mAccess = qobject_cast<QAppointmentSqlIO *>(access);
    Q_ASSERT(mAccess);
}

QIcon QAppointmentDefaultContext::icon() const
{
    static QIcon apptIcon(":icon/day");
    return apptIcon;
}

QString QAppointmentDefaultContext::description() const
{
    return tr("Default appointment storage");
}

QString QAppointmentDefaultContext::title() const
{
    return tr("Appointments");
}

bool QAppointmentDefaultContext::editable() const
{
    return true;
}

QSet<QPimSource> QAppointmentDefaultContext::sources() const
{
    QSet<QPimSource> list;
    list.insert(defaultSource());
    return list;
}

QUuid QAppointmentDefaultContext::id() const
{
    return mAccess->contextId();
}

void QAppointmentDefaultContext::setVisibleSources(const QSet<QPimSource> &set)
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();

    if (set.contains(defaultSource()))
        filter.remove(context);
    else
        filter.insert(context);

    mAccess->setContextFilter(filter);
}

QSet<QPimSource> QAppointmentDefaultContext::visibleSources() const
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();
    if (!filter.contains(context))
        return sources();
    return QSet<QPimSource>();
}

bool QAppointmentDefaultContext::exists(const QUniqueId &id) const
{
    return exists(id, defaultSource());
}

bool QAppointmentDefaultContext::exists(const QUniqueId &id, const QPimSource &source) const
{
    int context = QPimSqlIO::sourceContext(source);
    return mAccess->exists(id) && mAccess->context(id) == context;
}

QPimSource QAppointmentDefaultContext::defaultSource() const
{
    QPimSource s;
    s.context = mAccess->contextId();
    s.identity = contextsource;
    return s;
}

QPimSource QAppointmentDefaultContext::source(const QUniqueId &id) const
{
    if (exists(id))
        return defaultSource();
    return QPimSource();
}

bool QAppointmentDefaultContext::updateAppointment(const QAppointment &appointment)
{
    return mAccess->updateAppointment(appointment);
}

bool QAppointmentDefaultContext::removeAppointment(const QUniqueId &id)
{
    return mAccess->removeAppointment(id);
}

QUniqueId QAppointmentDefaultContext::addAppointment(const QAppointment &appointment, const QPimSource &source)
{
    if (source.isNull() || source == defaultSource())
        return mAccess->addAppointment(appointment, defaultSource());
    return QUniqueId();
}

bool QAppointmentDefaultContext::removeOccurrence(const QUniqueId &original, const QDate &date)
{
    return mAccess->removeOccurrence(original, date);
}

bool QAppointmentDefaultContext::restoreOccurrence(const QUniqueId &original, const QDate &date)
{
    return mAccess->restoreOccurrence(original, date);
}

QUniqueId QAppointmentDefaultContext::replaceOccurrence(const QUniqueId &original, const QOccurrence &occurrence, const QDate& date)
{
    return mAccess->replaceOccurrence(original, occurrence, date);
}

QUniqueId QAppointmentDefaultContext::replaceRemaining(const QUniqueId &original, const QAppointment &r, const QDate& date)
{
    return mAccess->replaceRemaining(original, r, date);
}
