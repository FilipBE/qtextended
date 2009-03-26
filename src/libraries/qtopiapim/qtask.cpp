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

#include "vobject_p.h"

#include <qtimestring.h>
#include <QRegExp>
#include <QString>
#include <QTextCodec>
#include <QApplication>
#include <QTextDocument>
#include <QDataStream>
#include <QBuffer>

#include <stdio.h>

class QTaskData : public QSharedData
{
public:
    QDate mDueDate;
    int mPriority;
    QString mDesc;

    int mStatus;
    QDate mStartedDate, mCompletedDate;
    QString mNotes;
    uint mPercentCompleted;

    QUniqueId mUid;
    QList<QString> mCategories;
    QMap<QString, QString> customMap;
};


/*!
  \class QTask
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QTask class holds the data of a task (to-do list) entry.

  A QTask stores several pieces of information about a task and its progress,
  including:
  \list
  \o a description
  \o a priority (see \l Priority)
  \o a due date for the task (optional)
  \o the progress of a task
  \o notes associated with the task (optional)
  \endlist

  The progress of a task is tracked in a number of ways:
  \list
  \o the status of a task (\l Status)
  \o the date a task was started (can be null)
  \o the date a task was finished (can be null)
  \o a percentage completed
  \endlist

  These progress fields are interrelated.  Setting one field
  may cause a change in other fields, since QTask will manage the
  dependencies between them automatically.  These dependencies include:
  \list
  \o startedDate - only set if status is not \c NotStarted
  \o completedDate - only set if status is \c Completed, and vice versa
  \o percentCompleted - 0 if status is \c NotStarted, 100 if status is \c Completed, otherwise between 0 and 99 inclusive
  \endlist

  These dependencies work in both directions.  For example:
  \list
  \o setting the \c percentCompleted to 0 will force the \c status to become \c NotStarted, and the \c startedDate and \c completedDate will be set to null
  \o setting the \c status to \c NotStarted will force the \c startedDate and \c completedDate to become null, and the \c percentCompleted to be 0
  \o setting the \c percentCompleted to 99 will force the \c status to become \c InProgress (unless it is currently \c Deferred or \c Waiting), sets the \c startedDate to be the current date (if it was previously null), and clears the \c completedDate
  \endlist

  \sa Status, Priority, {Pim Library}
*/



/*!
  Constructs a QTask as a copy of \a other.
*/
QTask::QTask(const QTask &other) : QPimRecord(other)
{
    d = other.d;
}

/*!
  Sets the task to be a copy of \a other.
*/
QTask &QTask::operator=(const QTask &other)
{
    d = other.d;
    return *this;
}

/*!
  Sets the priority of the task to \a priority.

  \sa priority()
*/
void QTask::setPriority( Priority priority ) { d->mPriority = priority; }

/*!
  Sets the priority of the task to \a priority.
  This is a convenience wrapper function.

  \sa priority()
*/
void QTask::setPriority( int priority ) {
    if (priority >= VeryHigh && priority <= VeryLow)
        setPriority((Priority)priority);
}

/*!
  Returns the priority of the task.

  \sa setPriority()
*/
QTask::Priority QTask::priority() const { return (Priority)d->mPriority; }

/*!
  Returns the notes for the task.

  \sa setNotes()
*/
QString QTask::notes() const { return d->mNotes; }

/*!
  Sets the notes of the task to \a notes.

  \sa notes()
*/
void QTask::setNotes(const QString &notes) { d->mNotes = notes; }

/*!
  Sets the description of the task to \a description.

  \sa description()
 */
void QTask::setDescription( const QString& description )
{ d->mDesc = description; }

/*!
  Returns the description of the task.

  \sa setDescription()
 */
QString QTask::description() const { return d->mDesc; }

/*!
  Clears the due date of the task.

  \sa setDueDate()
*/
void QTask::clearDueDate() { d->mDueDate = QDate(); }

/*!
  If \a b is true, marks the task as completed, and otherwise marks the task as incomplete.

  Several fields are dependent on the new completion state:
  \list
  \o If the task is not marked as complete, then the completed date is cleared, and
  the percentage complete is limited to a maximum of 99%.
  \o if the task is marked as complete, the percentage complete will be forced to 100%,
  and the completed date will be set to the current date if it was not previously set.
  \endlist

  \sa isCompleted(), completedDate()
*/
void QTask::setCompleted( bool b )
{
    if (b == isCompleted())
        return;

    d->mCompletedDate = b ? QDate::currentDate() : QDate();
    if ( !b && d->mPercentCompleted == 100 )
        d->mPercentCompleted = 99;
    // percentComplete returns 100% if completed.
}

/*!
  Returns true if the task is completed.  Otherwise returns false.

  \sa setCompleted()
*/
bool QTask::isCompleted() const { return status() == Completed; }

/*!
  Returns the due date of the task.

  \sa setDueDate(), clearDueDate()
 */
QDate QTask::dueDate() const { return d->mDueDate; }

/*!
  Returns true if there is a due date set for the task.  Otherwise returns
  false.

  \sa dueDate(), setDueDate(), clearDueDate()
*/
bool QTask::hasDueDate() const { return !d->mDueDate.isNull(); }

/*!
  Returns the date the task was started.  If the task has not yet been started, the returned
  date is null.

  \sa hasStartedDate(), setStartedDate()
*/
QDate QTask::startedDate() const { return d->mStartedDate; }

/*!
  Sets the tasks to have started on \a date.  If \a date is null, then
  the task will be marked as having status \c NotStarted, any
  completed date will be cleared, and the percentage completed will
  be 0.

  \sa hasStartedDate(), startedDate()
*/
void QTask::setStartedDate(const QDate &date)
{
    d->mStartedDate = date;

    if (date.isNull()) {
        d->mCompletedDate = QDate();
        d->mPercentCompleted = 0;
    }
}

/*!
  Returns the date the task was completed.  If the task is not completed, the returned
  date is null.

  \sa isCompleted(), setCompletedDate()
*/
QDate QTask::completedDate() const { return d->mCompletedDate; }

/*!
  Sets the tasks completed date to \a date.  If \a date is null, and the
  task was previously completed, then the task will be marked as having
  status \c InProgress, and the percentage completed will be forced to 99%.
  In addition, if the task's started date was not previously set, it will
  be set to the supplied date.

  \sa isCompleted(), completedDate()
*/
void QTask::setCompletedDate(const QDate &date)
{
    d->mCompletedDate = date;
    if (date.isNull()) {
        if (d->mPercentCompleted == 100)
            d->mPercentCompleted = 99;
    } else {
        d->mPercentCompleted = 100;
        if (d->mStartedDate.isNull())
            d->mStartedDate = date;
    }
}

/*!
  \reimp
*/
QUniqueId &QTask::uidRef() { return d->mUid; }
/*!
  \reimp
*/
const QUniqueId &QTask::uidRef() const { return d->mUid; }

/*!
  \reimp
*/
QList<QString> &QTask::categoriesRef() { return d->mCategories; }
/*!
  \reimp
*/
const QList<QString> &QTask::categoriesRef() const { return d->mCategories; }

/*!
  \reimp
*/
QMap<QString, QString> &QTask::customFieldsRef() { return d->customMap; }
/*!
  \reimp
*/
const QMap<QString, QString> &QTask::customFieldsRef() const { return d->customMap; }

/*!
  \enum QTask::Status

  These values describe the current \l status() of the Task.

  The values are:

  \value NotStarted the task has not been started yet (0% complete, no started or completed dates)
  \value InProgress the task has a startedDate (but no completedDate), and percentCompleted is between 0 and 99 inclusive
  \value Completed the task has a startedDate and a completedDate, and percentCompleted is 100
  \value Waiting similar to InProgress
  \value Deferred similar to InProgress
*/

/*!
  \enum QTask::Priority

  These values describe the current \l priority() of the Task.

  The values are:

  \value VeryHigh
  \value High
  \value Normal
  \value Low
  \value VeryLow
*/

/*!
  Creates a new, empty task.
*/
QTask::QTask() : QPimRecord()
{
    d = new QTaskData();
    d->mDueDate = QDate::currentDate();
    d->mPriority = Normal;
    d->mStatus = NotStarted;
    d->mPercentCompleted = 0;
}

/*!
  Returns true if \a other is identical to this task. Otherwise return false.
*/
bool QTask::operator==(const QTask &other) const
{
    if (!(QPimRecord::operator==(other))) return false;
    if (d->mDueDate != other.d->mDueDate) return false;
    if (d->mPriority != other.d->mPriority) return false;
    if (d->mDesc != other.d->mDesc) return false;
    if (status() != other.status()) return false;
    if (d->mStartedDate != other.d->mStartedDate) return false;
    if (d->mCompletedDate != other.d->mCompletedDate) return false;
    if (d->mNotes != other.d->mNotes) return false;
    if (percentCompleted() != other.percentCompleted()) return false;
    return true;
}

/*!
  Returns false if \a other is identical to this task. Otherwise return true.
*/
bool QTask::operator!=(const QTask &other) const
{
    return !(*this == other);
}

/*!
  Destroys the task
*/
QTask::~QTask()
{
}

/*!
  Returns the \l Status of the task.

  \sa setStatus()
*/
QTask::Status QTask::status() const
{
    if ( !d->mCompletedDate.isNull())
        return Completed;
    if ( d->mStartedDate.isNull())
        return NotStarted;
    // and the revers needs to hold true as well, only send completed not started for the above state.
    if (d->mStatus == Completed || d->mStatus == NotStarted)
        return InProgress;
    return (Status)d->mStatus;
}

/*!
  Sets the \l Status of the task to \a status.

  If the new status is \c NotStarted, the
  task's started date will be cleared.  Otherwise,
  if the task was not previously started, the
  current date will be used as the task's starting date.

  Similarly, if the new status \c Completed and the
  task was not previously completed, then the current
  date will be used as the task's completed date.
  Otherwise, if the new status is not \c Completed,
  the completed date will be cleared.

  If a task's status is \c Completed, the percent
  completed will be set to 100%.  Otherwise, the
  percent completed is limited to a maximum of 99%.

  \sa status()
*/
void QTask::setStatus(Status status)
{
    d->mStatus = status;
    // check date fields.
    if (d->mStartedDate.isNull()) {
        if (status != NotStarted)
            d->mStartedDate = QDate::currentDate();
    } else {
        if (status == NotStarted)
            d->mStartedDate = QDate();
    }
    if (d->mCompletedDate.isNull()) {
        if (status == Completed)
            d->mCompletedDate = QDate::currentDate();
    } else {
        if (status != Completed)
            d->mCompletedDate = QDate();
    }

    // adjust % complete
    if (status == Completed)
        d->mPercentCompleted = 100;
    else if(status == NotStarted)
        d->mPercentCompleted = 0;
    else if (d->mPercentCompleted == 100)
        d->mPercentCompleted = 99;
}

/*!
  Sets the \l Status of the task to \a status.
  This is a convenience wrapper function.

  \sa status()
*/
void QTask::setStatus(int status)
{
    if (status >= NotStarted && status <= Deferred)
        setStatus((Status)status);
}

/*!
  Returns the translated text for the the task status \a status.
*/
QString QTask::statusToText(Status status)
{
    if (status < NotStarted || status > Deferred)
        return QString();

    static const char *const status_strings[] = {
        QT_TRANSLATE_NOOP( "QTask", "Not Started" ),
        QT_TRANSLATE_NOOP( "QTask", "In Progress" ),
        QT_TRANSLATE_NOOP( "QTask", "Completed" ),
        QT_TRANSLATE_NOOP( "QTask", "Waiting" ),
        QT_TRANSLATE_NOOP( "QTask", "Deferred" ),
    };

    if (qApp)
        return qApp->translate("QTask", status_strings[status]);
    else
        return status_strings[status];
}

/*!
  Sets the due date of the task to \a date.

  \sa clearDueDate(), dueDate()
*/
void QTask::setDueDate( const QDate &date )
{
    d->mDueDate = date;
}

/*!
  Returns progress of the task as a percent completed.
  For tasks that have not been started, this will return 0, and
  for completed tasks, this function will always return 100.

  \sa setPercentCompleted(), status()
*/
uint QTask::percentCompleted() const
{
    QTask::Status s = status();
    if (s == NotStarted)
        return 0;
    if (s == Completed)
        return 100;
    return d->mPercentCompleted;
}

/*!
  Sets the tasks percent completed field to \a percent.

  The task's status field depends on the new percentage
  complete value in the following ways:

  \list
  \o If \a percent is greater than 99, then a value of
  100 is used, and this function will also set the
  task's status to \c Completed.
  \o If \a percent is 0, this function will set the
  task's status to \c InProgress (if the task's startedDate is not null) or \c NotStarted
  \o Otherwise, the task's status will be set to \c InProgress
  if it was not \c Deferred or \c Waiting.
  \endlist

  \sa percentCompleted(), status()
*/
void QTask::setPercentCompleted( uint percent )
{
    if (percent > 99) {
        setStatus(Completed);
        d->mPercentCompleted = 100;
    } else if (percent == 0) {
        if (d->mStartedDate.isNull())
            setStatus(NotStarted);
        else
            setStatus(InProgress);
        d->mPercentCompleted = 0;
    } else {
        QTask::Status s = status();
        if (s == NotStarted || s == Completed)
            setStatus(InProgress);
        d->mPercentCompleted = percent;
    }
}

/*!
  Returns true if the task has a started date.

  \sa startedDate(), setStartedDate()
*/

bool QTask::hasStartedDate() const
{
    return !d->mStartedDate.isNull();
}

static QString statusToTrString(QTask::Status s)
{
    switch( s ) {
        case QTask::NotStarted: return qApp->translate("QtopiaPim", "Not yet started"); break;
        case QTask::InProgress: return qApp->translate("QtopiaPim", "In progress"); break;
        case QTask::Waiting: return qApp->translate("QtopiaPim", "Waiting"); break;
        case QTask::Deferred: return qApp->translate("QtopiaPim", "Deferred"); break;
        default: return qApp->translate("QtopiaPim", "Completed"); break;
    }
}

/*!
  Returns true if this task matches regular expression \a r. Otherwise returns false.

  This is intended to allow the user to enter text to find matching tasks.

  The expression will be matched against the following fields:
  \list
  \o the task's priority (as a number)
  \o the task's due date, if valid
  \o the task's description
  \o the task's start date, if valid
  \o the task's completed date, if valid
  \o the task's percentage complete, if the task has been started but not finished.
  \o the task's notes.
  \o the task's status, as a translated string.
  \endlist
*/
bool QTask::match ( const QRegExp &r ) const
{
    // match on priority, description on due date...
    bool match = false;
    if ( QString::number( d->mPriority ).contains( r ) )
        match = true;
    else if ( d->mDueDate.isValid() && d->mDueDate.toString().contains( r ) )
        match = true;
    else if ( d->mDesc.contains( r ) )
        match = true;
    else if ( d->mStartedDate.isValid() && d->mStartedDate.toString().contains( r ) )
        match = true;
    else if ( d->mCompletedDate.isValid() && d->mCompletedDate.toString().contains( r ) )
        match = true;
    else if ( status() != NotStarted && status() != Completed &&
            QString::number(percentCompleted()).contains( r ) )
        match = true;
    else if ( d->mNotes.contains( r ) )
        match = true;
    else if ( statusToTrString( status() ).contains( r ) )
        match = true;

    return match;
}

// In pimrecord.cpp
void qpe_startVObjectInput();
bool qpe_vobjectCompatibility(const char* misfeature);
void qpe_endVObjectInput();
void qpe_startVObjectOutput();
void qpe_endVObjectOutput(VObject *,const char* type,const QPimRecord*);
void qpe_setVObjectProperty(const QString&, const QString&, const char* type, QPimRecord*);
VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value );
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{ return qpe_safeAddPropValue(o,prop,value); }
VObject *qpe_safeAddProp( VObject *o, const char *prop);
static inline VObject *safeAddProp( VObject *o, const char *prop)
{ return qpe_safeAddProp(o,prop); }

static VObject *createVObject( const QTask &t )
{
    qpe_startVObjectOutput();

    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *task = safeAddProp( vcal, VCTodoProp );

    if ( t.hasDueDate() )
        safeAddPropValue( task, VCDueProp,
                t.dueDate().toString(Qt::ISODate) );
    if ( t.isCompleted() ) {
        // if we say its completed, then we have a completed date.
        safeAddPropValue( task, VCStatusProp, "COMPLETED" );
        safeAddPropValue( task, VCCompletedProp,
                t.completedDate().toString(Qt::ISODate) );
    }
    safeAddPropValue( task, VCPriorityProp, QString::number( t.priority() ) );

    // status *2 (enum && %)
    // ^^ We don't match VCStatusProp and vCal doesn't support ::percent.

    safeAddPropValue( task, "X-Qtopia-STATUS", QString::number( t.status() ));
    safeAddPropValue( task, "X-Qtopia-PERCOMP",
            QString::number( t.percentCompleted() ) );

    if (t.hasStartedDate())
    {
        // ok, need to set this one too.
        safeAddPropValue( task, "X-Qtopia-STARTED",
                t.startedDate().toString(Qt::ISODate));
    }

    // vCal spec: VCSummaryProp is required
    // Palm m100:     No (violates spec)
    // Ericsson T39m: Yes
    if ( qpe_vobjectCompatibility("Palm-Task-DN") ) {
        safeAddPropValue( task, VCSummaryProp, t.description() );
        safeAddPropValue( task, VCDescriptionProp, t.description() );
        safeAddPropValue( task, VCAttachProp, t.notes() );
    } else {
        safeAddPropValue( task, VCSummaryProp, t.description() );
        safeAddPropValue( task, VCDescriptionProp, t.notes() );
    }

    qpe_endVObjectOutput(task,"Todo List",&t); // No tr

    return vcal;
}


static QTask parseVObject( VObject *obj )
{
    QTask t;

    VObjectIterator it;
    initPropIterator( &it, obj );
    QString summary, description, attach; // vCal properties, not Qtopias
    while( moreIteration( &it ) ) {
        VObject *o = nextVObject( &it );
        QString name = vObjectName( o );

        // check this key/value for a CHARSET field.
        VObjectIterator tnit;
        initPropIterator( &tnit, o );
        QTextCodec *tc = 0;
        while( moreIteration( &tnit ) ) {
            VObject *otc = nextVObject( &tnit );
            if ( qstrcmp(vObjectName(otc), VCCharSetProp ) == 0) {
                tc = QTextCodec::codecForName(vObjectStringZValue(otc));
                break;
            }
        }
        QString value;
        if (tc)
            value = tc->toUnicode( vObjectStringZValue( o ) );
        else
            value = vObjectStringZValue( o );

        if ( name == VCDueProp ) {
            t.setDueDate( QDate::fromString( value, Qt::ISODate ) );
        }
        else if ( name == VCSummaryProp ) {
            summary = value;
        }
        else if ( name == VCDescriptionProp ) {
            description = value;
        }
        else if (name == VCAttachProp ) {
            attach = value;
        }
        else if ( name == VCStatusProp ) {
            if ( value == "COMPLETED" )
                t.setCompleted( true );
        }
        else if ( name == VCCompletedProp ) {
            t.setCompletedDate( QDate::fromString( value ) );
        }
        else if ( name == VCPriorityProp ) {
            t.setPriority( (QTask::Priority) value.toInt() );
        }
        else if (name == "X-Qtopia-STATUS" ) {
            t.setStatus( (QTask::Status) value.toInt() );
        }
        else if (name == "X-Qtopia-PERCOMP" ) {
            t.setPercentCompleted( value.toInt() );
        }
        else if (name == "X-Qtopia-STARTED" ) {
            t.setStartedDate( QDate::fromString( value, Qt::ISODate ) );
        } else {
            qpe_setVObjectProperty(name,value,"Todo List",&t); // No tr
        }
    }

    // Find best mapping from (Summary,Description,Attach) to our (Description,Notes)
    // Similar code in event.cpp
    if ( !summary.isEmpty() && !description.isEmpty() && summary != description ) {
        t.setDescription( summary );
        t.setNotes( description );
        // all 3 - drop attach
    } else if ( !summary.isEmpty() ) {
        t.setDescription( summary );
        t.setNotes( attach );
    } else {
        t.setDescription( description );
        t.setNotes( attach );
    }

    return t;
}

/*!
  Writes the given list of \a tasks to the given \a device as vCalendars.

  Returns true on success.
  \sa readVCalendar()
*/
bool QTask::writeVCalendar( QIODevice *device, const QList<QTask> &tasks )
{
    foreach (QTask t, tasks)
        if (!writeVCalendar(device, t))
            return false;
    return true;
}

/*!
  Writes the given \a task to the given \a device as vCalendars.

  Returns true on success.
  \sa readVCalendar()
*/
bool QTask::writeVCalendar( QIODevice *device, const QTask &task )
{
    VObject *obj = createVObject(task);
    writeVObject( device, obj );
    cleanVObject( obj );
    cleanStrTbl();
    return true;
}

/*!
  Reads a list of vCalendars from the given \a device and returns the
  equivalent set of tasks.

  \sa writeVCalendar()
*/
QList<QTask> QTask::readVCalendar( QIODevice *device )
{
    QList<QTask> tasks;

    QBuffer *buffer = qobject_cast<QBuffer *>(device);
    QFile *file = qobject_cast<QFile *>(device);
    if (file) {
        int handle = file->handle();
        FILE *input = fdopen(handle, "r");
        if (input) {
            tasks = readVCalendarData( Parse_MIME_FromFile( input ) );
        }
    } else if (buffer) {
        tasks = readVCalendarData( Parse_MIME( (const char*)buffer->data(), buffer->data().count() ) );
    } else {
        const QByteArray bytes = device->readAll();
        tasks = readVCalendarData( Parse_MIME( (const char*)bytes, bytes.count() ) );
    }

    return tasks;
}

/*!
  \deprecated

   Write the list of \a tasks as vCalendar objects to the file
   specified by \a filename.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( const QString &filename, const QList<QTask> &tasks)
{
    FILE *f = fopen(filename.toLocal8Bit(),"w");
    if ( !f ) {
        qWarning("Unable to open vcalendar write");
        return;
    }

    QList<QTask>::ConstIterator it;
    for( it = tasks.begin(); it != tasks.end(); ++it ) {
        VObject *obj = createVObject( *it );
        writeVObject(f, obj );
        cleanVObject( obj );
    }

    cleanStrTbl();
    fclose(f);
}

/*!
  \deprecated

   Write the \a task as a vCalendar to the file specified by \a filename.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( const QString &filename, const QTask &task)
{
    FILE *f = fopen(filename.toLocal8Bit(),"w");
    if ( !f ) {
        qWarning("Unable to open vcalendar write");
        return;
    }

    VObject *obj = createVObject( task );
    writeVObject(f, obj );
    cleanVObject( obj );

    cleanStrTbl();
    fclose(f);
}

/*!
  \deprecated

   Writes this task as a vCalendar object to the file specified
   by \a filename.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( const QString &filename ) const
{
    writeVCalendar(filename, *this);
}

/*!
  \deprecated

   Writes this task as a vCalendar object to the given \a file,
   which must be already open for writing.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( QFile &file ) const
{
    QDataStream stream( &file );
    writeVCalendar( &stream );

}

/*!
  \deprecated

   Writes this task as a vCalendar object to the given \a stream,
   which must be writable.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( QDataStream *stream ) const
{
    VObject *obj = createVObject(*this);
    writeVObject( stream->device(), obj );
    cleanVObject( obj );
    cleanStrTbl();
}

/*!
  \deprecated

  Reads the file specified by \a filename as a list of vCalendar objects
  and returns a list of QTasks that correspond to the data.  Note that
  some vCalendar properties may not be supported by QTask.

  \sa writeVCalendar()
*/
QList<QTask> QTask::readVCalendar( const QString &filename )
{
    VObject *obj = Parse_MIME_FromFileName( (const char *)filename.toUtf8() );
    return readVCalendarData(obj);
}

/*!
  \internal

  Reads a list of tasks from an opened VObject
*/
QList<QTask> QTask::readVCalendarData( VObject *obj )
{
    QList<QTask> tasks;

    qpe_startVObjectInput();
    while ( obj ) {
        QString name = vObjectName( obj );
        if ( name == VCCalProp ) {
            VObjectIterator nit;
            initPropIterator( &nit, obj );
            while( moreIteration( &nit ) ) {
                VObject *o = nextVObject( &nit );
                QString name = vObjectName( o );
                if ( name == VCTodoProp )
                    tasks.append( parseVObject( o ) );
            }
        } else if ( name == VCTodoProp ) {
            // shouldn't happen, but just to be sure
            tasks.append( parseVObject( obj ) );
        }
        VObject *t = obj;
        obj = nextVObjectInList(obj);
        cleanVObject( t );
    }
    qpe_endVObjectInput();

    return tasks;
}

/*!
  \deprecated

  Reads the \a data of \a len bytes as a list of vCalendar objects
  and returns the list of corresponding QTasks.

  \sa writeVCalendar(), readVCalendar()
*/
QList<QTask> QTask::readVCalendarData( const char *data, unsigned long len )
{
    VObject *obj = Parse_MIME( data, len );
    return readVCalendarData(obj);
}

/*!
  \deprecated

  Reads the given vCalendar data in \a vcal and returns the list of
  corresponding QTasks.

  \sa writeVCalendar()
*/
QList<QTask> QTask::readVCalendar( const QByteArray &vcal )
{
    return readVCalendarData( Parse_MIME( (const char*)vcal, vcal.count() ) );
}

/*!
  Returns a rich text formatted QString of the QTask.
*/
QString QTask::toRichText() const
{
    QString text;

    text = "<center><b>" + Qt::escape(d->mDesc) + "</b></center><br>"; // No tr
    if ( !d->mDueDate.isNull() )
        text += "<b>" + qApp->translate("QtopiaPim", "Due:") + "</b> " +
            QTimeString::localYMD( d->mDueDate, QTimeString::Long ) + "<br>";
    if ( !d->mStartedDate.isNull() && status() != NotStarted)
        text += "<b>" + qApp->translate("QtopiaPim", "Started:") + "</b> " +
            QTimeString::localYMD( d->mStartedDate, QTimeString::Long ) +  "<br>";
    if ( !d->mCompletedDate.isNull() && isCompleted() )
        text += "<b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " +
            QTimeString::localYMD( d->mCompletedDate, QTimeString::Long ) + "<br>";

    //if ( !isCompleted() ) { //We remember old status and treat completed separately
    // such remembering is already done by, status();

    QString statusString = statusToTrString( status() );
    text += "<b>" + qApp->translate("QtopiaPim", "Status:") + "</b> " + statusString + "<br>";
    text +="<b>" + qApp->translate("QtopiaPim", "Priority:") + "</b> " + QString::number( d->mPriority ) + "<br>";

    if ( (status() != NotStarted && !isCompleted() ) )
        text += "<b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " + QString::number(percentCompleted()) + " " +
            qApp->translate("QtopiaPim", "percent", "Word or symbol after numbers for percentage") + "<br>";

    if ( !d->mNotes.isEmpty() )
        text += "<br> <b> " + qApp->translate("QtopiaPim", "Notes:") + "</b> "
            + d->mNotes;

    return text;
}


/*!
    \internal
    \fn void QTask::deserialize(Stream &s)
*/
template <typename Stream> void QTask::deserialize(Stream &s)
{
    s >> d->mUid;
    s >> d->mCategories;
    s >> d->customMap;
    quint8 val;
    s >> val; // old mDue field, for compat
    s >> d->mDueDate;
    if (!val)
        d->mDueDate = QDate();
    s >> val; // stream left to maintain compat, value ignored.
    //c.d->mCompleted = val == 0 ? false : true;
    qint32 p;
    s >> p;
    d->mPriority = (QTask::Priority) p;
    s >> d->mDesc;
    s >> d->mStartedDate;
    s >> d->mCompletedDate;

    qint32 i;
    s >> i;
    d->mStatus = (QTask::Status)i;

    s >> d->mNotes;
}

/*!
    \internal
    \fn void QTask::serialize(Stream &s) const
*/
template <typename Stream> void QTask::serialize(Stream &s) const
{
    s << d->mUid;
    s << d->mCategories;
    s << d->customMap;
    s << (uchar) (d->mDueDate.isNull() ? 0 : 1);
    s << d->mDueDate;
    s << (isCompleted() ? (uchar)1 : (uchar)0);
    s << d->mPriority;
    s << d->mDesc;
    s << d->mStartedDate;
    s << d->mCompletedDate;
    int i = (int) d->mStatus;
    s << i;
    s << d->mNotes;
}

Q_IMPLEMENT_USER_METATYPE(QTask)
