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

#ifndef QAPPOINTMENTSQLIO_P_H
#define QAPPOINTMENTSQLIO_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qlist.h>
#include <qdatetime.h>
#include <qappointment.h>
#include <qtopiasql.h>
#include <qsqlquery.h>
#include "qpimsqlio_p.h"
#include "qpimsource.h"

#include "qappointmentmodel.h"

class QAppointmentSqlIO;
class QAppointmentDefaultContext : public QAppointmentContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QAppointmentDefaultContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;
    using QAppointmentContext::title;
    QString title() const;

    // better to be flags ?
    using QAppointmentContext::editable;
    bool editable() const; // default true

    QPimSource defaultSource() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QAppointmentContext::exists;
    bool exists(const QUniqueId &) const;
    bool exists(const QUniqueId &id, const QPimSource &source) const;
    QPimSource source(const QUniqueId &) const;

    bool updateAppointment(const QAppointment &);
    bool removeAppointment(const QUniqueId &);
    QUniqueId addAppointment(const QAppointment &, const QPimSource &);

    bool removeOccurrence(const QUniqueId &original, const QDate &);
    bool restoreOccurrence(const QUniqueId &original, const QDate &);
    QUniqueId replaceOccurrence(const QUniqueId &original, const QOccurrence &, const QDate&);
    QUniqueId replaceRemaining(const QUniqueId &original, const QAppointment &, const QDate&);
private:
    QAppointmentSqlIO *mAccess;
};

class QAppointmentSqlIO : public QPimSqlIO {

    Q_OBJECT

public:
    explicit QAppointmentSqlIO(QObject *parent = 0);
    ~QAppointmentSqlIO();

    static QDateTime nextAlarm(const QAppointment &appointment);
    static void removeAlarm(const QAppointment &appointment);
    static void addAlarm(const QAppointment &appointment);

    bool editableByRow() const { return true; }
    bool editableByField() const { return true; }

    bool removeAppointment(const QUniqueId &id);
    bool removeAppointment(int);
    bool removeAppointments(const QList<QUniqueId> ids);
    bool updateAppointment(const QAppointment& appointment);
    QUniqueId addAppointment(const QAppointment& appointment, const QPimSource &s)
    { return addAppointment(appointment, s, true); }
    QUniqueId addAppointment(const QAppointment& appointment, const QPimSource &, bool);

    bool removeOccurrence(const QUniqueId &original,
            const QDate &);
    bool restoreOccurrence(const QUniqueId &original,
            const QDate &);
    QUniqueId replaceOccurrence(const QUniqueId &original,
            const QOccurrence &, const QDate &);
    QUniqueId replaceRemaining(const QUniqueId &original,
            const QAppointment &, const QDate &);

    QVariant appointmentField(int row, QAppointmentModel::Field k) const;
    bool setAppointmentField(int row, QAppointmentModel::Field k,  const QVariant &);

    QUuid contextId() const;

    void setRangeFilter(const QDateTime &earliest, const QDateTime &latest);
    QDateTime rangeStart() const;
    QDateTime rangeEnd() const;

    void setDurationType(QAppointmentModel::DurationType);

    QAppointment appointment(const QUniqueId &) const;
    QAppointment appointment(int row) const;

    bool nextAlarm(QDateTime &when, QUniqueId &) const;

    bool updateExtraTables(uint, const QPimRecord &);
    bool insertExtraTables(uint, const QPimRecord &);
    bool removeExtraTables(uint);

    /* subclased from sql io */
    void bindFields(const QPimRecord &r, QPreparedSqlQuery &) const;

    QStringList sortColumns() const;

    // forces re-read of data and view reset.
    void refresh();

    void checkAdded(const QUniqueId &);
    void checkRemoved(const QUniqueId &);
    void checkRemoved(const QList<QUniqueId> &);
    void checkUpdated(const QUniqueId &);

    QAppointment appointment(const QUniqueId &, bool minimal) const;
    QAppointment appointment(int, bool minimal) const;


    QList<QAppointment> fastRange(const QDateTime &start, const QDateTime &end, int count) const;

protected:
    void invalidateCache();

private:


    QStringList currentFilters() const;

    enum AppointmentCacheStatus {
        Empty = 0,
        Minimal = 1,
        Full = 2
    };

    mutable AppointmentCacheStatus lastAppointmentStatus;
    mutable QAppointment lastAppointment;
    mutable int lastRow;

    mutable QDateTime mAlarmStart;

    QDateTime rStart;
    QDateTime rEnd;
    QAppointmentModel::DurationType rType;

    const QString mainTable;
    const QString catTable;
    const QString customTable;
    const QString exceptionTable;

    // Saved queries
    mutable QPreparedSqlQuery appointmentQuery;
    mutable QPreparedSqlQuery exceptionsQuery;
    mutable QPreparedSqlQuery parentQuery;
};

#endif
