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

#ifndef QTASK_H
#define QTASK_H

#include <qtopianamespace.h>
#include <qpimrecord.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

#include <QList>
#include <QDateTime>
#include <QSharedData>
#include <QSharedDataPointer>

class QTaskData;
class VObject;
class QTOPIAPIM_EXPORT QTask : public QPimRecord
{
public:
    enum Status {
        NotStarted = 0,
        InProgress,
        Completed,
        Waiting,
        Deferred
    };

    enum Priority {
        VeryHigh=1,
        High,
        Normal,
        Low,
        VeryLow
    };

    QTask();
    QTask(const QTask &);

    QTask &operator=(const QTask &other);

    bool operator==(const QTask &other) const;
    bool operator!=(const QTask &other) const;

    virtual ~QTask();

    static bool writeVCalendar( QIODevice *, const QList<QTask> & );
    static bool writeVCalendar( QIODevice *, const QTask & );
    static QList<QTask> readVCalendar( QIODevice * );

    /* deprecated - keep for source compatibility */
    static void writeVCalendar( const QString &filename, const QList<QTask> &tasks);
    static void writeVCalendar( const QString &filename, const QTask &task);
    void writeVCalendar( const QString &filename ) const;
    void writeVCalendar( QFile &file ) const;
    void writeVCalendar( QDataStream *stream ) const;
    static QList<QTask> readVCalendar( const QString &filename );
    static QList<QTask> readVCalendarData( const char *, unsigned long );
    static QList<QTask> readVCalendar( const QByteArray &vcard );

    void setPriority( Priority priority );
    void setPriority( int priority );
    Priority priority() const;

    void setDescription( const QString& description );
    QString description() const;

    void setDueDate( const QDate& date);
    void clearDueDate();

    QDate dueDate() const;
    bool hasDueDate() const;

    QDate startedDate() const;
    void setStartedDate(const QDate &date);
    bool hasStartedDate() const;

    QDate completedDate() const;
    void setCompletedDate(const QDate &date);

    Status status() const;
    void setStatus(Status s);
    void setStatus(int s);

    bool isCompleted() const;
    void setCompleted( bool b );

    uint percentCompleted() const;
    void setPercentCompleted( uint u );

    QString notes() const;
    void setNotes(const QString &s);

    bool match( const QRegExp &r ) const;

    QString toRichText() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

//protected:
    //virtual int endFieldMarker() const {return TaskFieldCount; }
    static QString statusToText(Status s);

protected:
    QUniqueId &uidRef();
    const QUniqueId &uidRef() const;

    QList<QString> &categoriesRef();
    const QList<QString> &categoriesRef() const;

    QMap<QString, QString> &customFieldsRef();
    const QMap<QString, QString> &customFieldsRef() const;

private:
    static QList<QTask> readVCalendarData( VObject *obj);

    QSharedDataPointer<QTaskData> d;
};

Q_DECLARE_USER_METATYPE(QTask)

#endif
