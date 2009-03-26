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

#ifndef QAPPOINTMENT_H
#define QAPPOINTMENT_H

#include <QDateTime>
#include <QList>
#include <QColor>
#include <QSharedData>

#include <qpimrecord.h>

#include <qtimezone.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

#include <sys/types.h>

class QDataStream;
class QAppointmentData;
class QOccurrence;
class QFile;
class VObject;
class QTOPIAPIM_EXPORT QAppointment : public QPimRecord
{
public:
    enum RepeatRule
    {
        NoRepeat,
        Daily,
        Weekly,
        MonthlyDate, // 26th of each month
        MonthlyDay, // third thursday of each month
        Yearly, // 26 of Feb.
        MonthlyEndDay, // third last thursday of each month
    };

    //enum Type { Normal, AllDay };
    //
    enum AlarmFlag
    {
        NoAlarm = 0x00,
        Visible = 0x01,
        Audible = 0x02 | Visible,
    };

    enum WeekFlag {
        OccurMonday = 0x01,
        OccurTuesday = 0x02,
        OccurWednesday = 0x04,
        OccurThursday = 0x08,
        OccurFriday = 0x10,
        OccurSaturday = 0x20,
        OccurSunday = 0x40
    };

    struct Exception {
        QDate date;
        QUniqueId alternative;
    };

    Q_DECLARE_FLAGS(AlarmFlags, AlarmFlag)
    Q_DECLARE_FLAGS(WeekFlags, WeekFlag)

    static WeekFlag fromDateDay(int);
    static int toDateDay(WeekFlag);

    QAppointment();
    QAppointment(const QAppointment &);
    QAppointment(const QDateTime &start, const QDateTime &end);
    virtual ~QAppointment();

    QAppointment &operator=( const QAppointment &evt );

    bool operator==( const QAppointment &evt ) const;
    bool operator!=( const QAppointment &evt ) const;

    QString description() const;
    QString location() const;
    QDateTime start() const;
    QDateTime end() const;
    QString notes() const;
    QTimeZone timeZone() const;

    int alarmDelay() const;
    AlarmFlags alarm() const;

    RepeatRule repeatRule() const;
    int frequency() const;
    QDate repeatUntil() const;
    QDate repeatUntilInCurrentTZ( ) const;
    bool repeatForever() const;
    bool showOnNearest() const;
    WeekFlags weekFlags() const;
    bool isAllDay() const;

    void setDescription( const QString &s );
    void setLocation( const QString &s );
    void setStart( const QDateTime &d );
    void setEnd( const QDateTime &e );
    void setNotes( const QString &n );
    void setTimeZone( const QTimeZone & );
    void setWeekFlags(WeekFlags);
    void setAllDay(bool enable = true);

    void setAlarm( int minutes, AlarmFlags );
    void clearAlarm();

    void setRepeatRule( RepeatRule t );
    void setFrequency( int );
    void setRepeatUntil( const QDate & );
    void setShowOnNearest( bool );

    bool isValid() const;

    void setRepeatForever();
    void setRepeatOnWeekDay(int day, bool enable);
    bool repeatOnWeekDay(int day) const;
    int weekOffset() const;
    bool hasRepeat() const { return repeatRule() != NoRepeat; }
    bool hasAlarm() const { return alarm() != NoAlarm; }
    QDateTime startInCurrentTZ( ) const;
    QDateTime endInCurrentTZ( ) const;

    void setExceptionParent( const QUniqueId &id );
    void setAsException(const QUniqueId &parent, const QDate &date);
    QUniqueId exceptionParent() const;
    QDate exceptionDate() const;

    bool hasExceptions() const { return exceptions().count() != 0; }
    QList<Exception> exceptions() const;
    void setExceptions(const QList<Exception> &);
    void clearExceptions();
    bool isException() const;

    QOccurrence nextOccurrence(const QDate &from) const;
    QOccurrence firstOccurrence() const;

    static bool writeVCalendar( QIODevice *, const QList<QAppointment> & );
    static bool writeVCalendar( QIODevice *, const QAppointment & );
    static QList<QAppointment> readVCalendar( QIODevice * );

    /* deprecated - keep for source compatibility */
    static void writeVCalendar( const QString &, const QList<QAppointment> & );
    static void writeVCalendar( const QString &, const QAppointment & );
    void writeVCalendar( const QString &filename ) const;
    void writeVCalendar( QFile &file ) const;
    void writeVCalendar( QDataStream *stream ) const;
    static QList<QAppointment> readVCalendar( const QString & );
    static QList<QAppointment> readVCalendarData( const char *, unsigned long );
    static QList<QAppointment> readVCalendar( const QByteArray &vcard );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

protected:
    QUniqueId &uidRef();
    const QUniqueId &uidRef() const;

    QList<QString> &categoriesRef();
    const QList<QString> &categoriesRef() const;

    QMap<QString, QString> &customFieldsRef();
    const QMap<QString, QString> &customFieldsRef() const;

private:
    static QList<QAppointment> readVCalendarData( VObject * );
    QDate p_nextOccurrence(const QDate &from) const;
    bool p_weekFlagsActive() const;

    void init(const QDateTime &, const QDateTime &);

    QSharedDataPointer<QAppointmentData> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAppointment::WeekFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QAppointment::AlarmFlags)

class QTOPIAPIM_EXPORT QOccurrence
{
public:
    QOccurrence();
    QOccurrence(const QOccurrence &other);
    QOccurrence(const QDate &start, const QAppointment &);
    ~QOccurrence();

    QOccurrence& operator=(const QOccurrence &other);

    bool operator==(const QOccurrence &other) const;

    bool isValid() const
    { return !mStart.isNull(); }

    QOccurrence nextOccurrence() const;

    bool conflicts(const QOccurrence &other) const
    { return conflicts(other.startInCurrentTZ(), other.endInCurrentTZ()); }

    bool conflicts(const QDateTime &from, const QDateTime &to) const
    { return from < endInCurrentTZ() && to > startInCurrentTZ(); }

    QString description() const { return appointmentCache.description(); }
    QString location() const { return appointmentCache.location(); }
    QString notes() const { return appointmentCache.notes(); }
    QTimeZone timeZone() const { return appointmentCache.timeZone(); }

    int alarmDelay() const { return appointmentCache.alarmDelay(); }
    QAppointment::AlarmFlags alarm() const { return appointmentCache.alarm(); }

    QDateTime alarmInCurrentTZ() const;

    QDate endDate() const;
    QDateTime start() const;
    QDateTime end() const;

    QDateTime startInCurrentTZ( ) const;
    QDateTime endInCurrentTZ( ) const;

    QAppointment appointment() const { return appointmentCache; }
    QUniqueId uid() const { return appointmentCache.uid(); }
    QList<QString> categories() const { return appointmentCache.categories(); }

    QDate date() const { return mStart; }

private:
    QAppointment appointmentCache;
    QDate mStart;
};

Q_DECLARE_USER_METATYPE(QAppointment)

#endif
