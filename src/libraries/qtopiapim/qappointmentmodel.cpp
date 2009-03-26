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
#include <qappointmentmodel.h>
#include "qappointmentsqlio_p.h"
#if defined(GOOGLE_CALENDAR_CONTEXT) && !defined(QT_NO_OPENSSL)
#include "qgooglecontext_p.h"
#endif
#include "qdependentcontexts_p.h"
#include <QSettings>
#include <QMap>
#include <QList>
#include <QSet>
#include <QFile>
#include <QTimer>
#include <QApplication>

#include <QDebug>

class QAppointmentModelData
{
public:
    QAppointmentModelData() : durationType(QAppointmentModel::AnyDuration) , defaultio(0) {}

    QDateTime rStart;
    QDateTime rEnd;
    QAppointmentModel::DurationType durationType;
    QAppointmentSqlIO *defaultio;

    static QIcon getCachedIcon(const QString& str);
    static QHash<QString, QIcon> cachedIcons;
};

QHash<QString, QIcon> QAppointmentModelData::cachedIcons;

QIcon QAppointmentModelData::getCachedIcon(const QString &str)
{
    if (cachedIcons.contains(str))
        return cachedIcons.value(str);
    cachedIcons.insert(str, QIcon(str));
    return cachedIcons.value(str);
}

QMap<QAppointmentModel::Field, QString> QAppointmentModel::k2i;
QMap<QString, QAppointmentModel::Field> QAppointmentModel::i2k;
QMap<QAppointmentModel::Field, QString>  QAppointmentModel::k2t;

/*!
  \internal

  Initializes mappings from column enums to translated and non-translated strings.
*/
void QAppointmentModel::initMaps()
{
    if (k2t.count() > 0)
        return;
    struct KeyLookup {
        const char * ident;
        const char * trans;
        Field key;
    };
    static const KeyLookup l[] = {
        { "description", QT_TR_NOOP("Description"), Description },
        { "location", QT_TR_NOOP( "Location" ), Location },
        { "start", QT_TR_NOOP( "Start" ), Start },
        { "end", QT_TR_NOOP( "End" ), End },
        { "allday", QT_TR_NOOP( "All Day" ), AllDay },
        { "timezone", QT_TR_NOOP( "Time Zone" ), TimeZone },

        { "notes", QT_TR_NOOP( "Notes" ), Notes },
        { "alarms", QT_TR_NOOP( "Alarms" ), Alarm },
        { "repeatrule", QT_TR_NOOP( "Repeat Rule" ), RepeatRule },
        { "repeatfrequency", QT_TR_NOOP( "Repeat Frequency" ), RepeatFrequency },
        { "repeatenddate", QT_TR_NOOP( "Repeat End Date" ), RepeatEndDate },
        { "repeatweekflags", QT_TR_NOOP( "Repeat Week Flags" ), RepeatWeekFlags },

        { "identifier", QT_TR_NOOP( "Identifier" ), Identifier},
        { "categories", QT_TR_NOOP( "Categories" ), Categories},
        { 0, 0, Invalid }
    };

    const KeyLookup *k = l;
    while (k->key != 0) {
        k2t.insert(k->key, k->trans);
        k2i.insert(k->key, k->ident);
        i2k.insert(k->ident, k->key);
        ++k;
    }
}

/*!
  Returns a translated string describing the appointment model \a field.

  \sa fieldIcon(), fieldIdentifier(), identifierField()
*/
QString QAppointmentModel::fieldLabel(Field field)
{
    if (k2t.count() == 0)
        initMaps();
    if (!k2t.contains(field))
        return QString();
    return k2t[field];
}

/*!
  Returns a icon representing the appointment model \a field.

  Returns a null icon if no icon is available.

  \sa fieldLabel(), fieldIdentifier(), identifierField()
*/
QIcon QAppointmentModel::fieldIcon(Field field)
{
    QString ident = fieldIdentifier(field);

    if (ident.isEmpty())
        return QIcon();

    return QAppointmentModelData::getCachedIcon(":image/datebook/" + ident);
}

/*!
  Returns a non-translated string describing the appointment model \a field.

  \sa fieldLabel(), fieldIcon(), identifierField()
*/
QString QAppointmentModel::fieldIdentifier(Field field)
{
    if (k2i.count() == 0)
        initMaps();
    if (!k2i.contains(field))
        return QString();
    return k2i[field];
}

/*!
  Returns the appointment model field for the non-translated field \a identifier

  \sa fieldLabel(), fieldIcon(), fieldIdentifier()
*/
QAppointmentModel::Field QAppointmentModel::identifierField(const QString &identifier)
{
    if (i2k.count() == 0)
        initMaps();
    if (!i2k.contains(identifier))
        return Invalid;
    return i2k[identifier];
}

/*!
  \class QAppointmentModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QAppointmentModel class provides access to the Calendar data.

  User appointments are represented in the appointment model as a table, with each row corresponding to a
  particular appointment and each column as on of the fields of the appointment.  Complete QAppointment objects can
  be retrieved using the appointment() function which takes either a row, index, or unique identifier.

  The appointment model is a descendant of QAbstractItemModel, so it is suitable for use with
  the Qt View classes such as QListView and QTableView, as well as any custom views.

  The appointment model provides functions for sorting and some filtering of items.
  For filters or sorting that is not provided by the appointment model it is recommended that
  QSortFilterProxyModel is used to wrap the appointment model.

  A QAppointmentModel instance will also reflect changes made in other instances of QAppointmentModel,
  both within this application and from other applications.  This will result in
  the modelReset() signal being emitted.

  \sa QAppointment, QSortFilterProxyModel, {Pim Library}
*/

/*!
  Constructs an appointment model with the given \a parent.
*/
QAppointmentModel::QAppointmentModel(QObject *parent)
    : QPimModel(parent)
{

    QtopiaSql::instance()->openDatabase();
    d = new QAppointmentModelData();

    QAppointmentSqlIO *access = new QAppointmentSqlIO(this);
    QAppointmentContext *context = new QAppointmentDefaultContext(this, access);

    setAccess(access);
    d->defaultio = access;
    addContext(context);

#if defined(GOOGLE_CALENDAR_CONTEXT) && !defined(QT_NO_OPENSSL)
    addContext(new QGoogleCalendarContext(this, access));
#endif
    addContext(new QContactBirthdayContext(this, access));
    addContext(new QContactAnniversaryContext(this, access));
    addContext(new QTaskEventContext(this, access));
}

/*!
  Destroys the appointment model.
*/
QAppointmentModel::~QAppointmentModel()
{
    delete d;
}

/*!
  \enum QAppointmentModel::Field

  Enumerates the columns when in table mode and columns used for sorting.
  This is a subset of data retrievable from a QAppointment.

  \value Invalid
    An invalid field
  \value Description
    The description of the appointment
  \value Location
    The location of the appointment
  \value Start
    The start time of the appointment
  \value End
    The end time of the appointment
  \value AllDay
    Whether the appointment is an all day event
  \value TimeZone
    The time zone of the appointment
  \value Notes
    The notes of the appointment
  \value Alarm
    The type of alarm of the appointment
  \value RepeatRule
    The repeat rule of the appointment
  \value RepeatFrequency
    The repeat frequency of the appointment
  \value RepeatEndDate
    The date a repeating appointment repeats until.  If null the appointment
    repeats forever
  \value RepeatWeekFlags
    The flags specifying what days of the week the appointment repeats on
  \value Identifier
    The identifier of the appointment
  \value Categories
    The list of categories the appointment belongs to
*/

/*!
  \enum QAppointmentModel::QAppointmentModelRole

  Extends Qt::ItemDataRole

  \value LabelRole
    A short formatted text label of the appointments name.
*/

/*!
  \reimp
*/
QMimeData * QAppointmentModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    return 0;
}

/*!
  \reimp
*/
QStringList QAppointmentModel::mimeTypes() const
{
    return QStringList();
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.
*/
QVariant QAppointmentModel::data(const QModelIndex &index, int role) const
{
    int r = index.row();

    switch(index.column()) {
        case Description:
            if (index.row() < rowCount()){
                // later, take better advantage of roles.
                switch(role) {
                    default:
                        break;
                    case Qt::DisplayRole:
                        return d->defaultio->appointmentField(r, Description);
                    case Qt::EditRole:
                        return d->defaultio->appointmentField(r, Identifier).toByteArray();
                    case LabelRole:
                        {
                            QString l = d->defaultio->appointmentField(r, Description).toString();
                            return "<b>" + l + "</b>";
                        }
                        break;
                }
            }
            break;
        default:
            if (index.column() > 0 && index.column() < columnCount())
                return d->defaultio->appointmentField(r, (QAppointmentModel::Field)index.column());
            break;
    }
    return QVariant();
}

/*!
  \reimp
*/
int QAppointmentModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return Categories+1;// last column + 1
}

/*!
  \overload
  Sets the \a role data for the item at \a index to \a value. Returns true if successful.

  The appointment model only accepts data for the \c EditRole.  The column of the specified
  index specifies the \c QAppointmentModel::Field to set and the row of the index
  specifies which appointment to modify.

  \sa setAppointmentField()
*/
bool QAppointmentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    QAppointment a = appointment(index);
    if (!setAppointmentField(a, (Field)index.column(), value))
        return false;
    return updateAppointment(a);

#if 0
    /*
        disabled due to 'notifyUpdated' require whole record.
        While writing whole record is less efficient than partial - at
        this stage it was the easiest way of fixing the bug where setData
        did not result in cross-model data change from being propagated properly
   */

    int i = index.row();
    const QAppointmentIO *model = qobject_cast<const QAppointmentSqlIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((QAppointmentSqlIO *)model)->setAppointmentField(r, (Field)index.column(), value);
    return false;
#endif
}

/*!
  \reimp
*/
bool QAppointmentModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles)
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

/*!
  \reimp
*/
QMap<int,QVariant> QAppointmentModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> result;
    switch (index.column()) {
        case Description:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            result.insert(Qt::EditRole, data(index, Qt::EditRole));
            break;
        default:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            break;
    }
    return result;
}

/*!
  \reimp
*/
QVariant QAppointmentModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (section >= 0 && section < columnCount()) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
    }
    return QVariant();
}

/*!
  Returns the appointment for the row specified by \a index.
  The column of \a index is ignored.
*/
QAppointment QAppointmentModel::appointment(const QModelIndex &index) const
{
    return appointment(index.row());
}

/*!
  Returns the appointment for given \a row.
*/
QAppointment QAppointmentModel::appointment(int row) const
{
    return d->defaultio->appointment(row);
}

/*!
  Returns the appointment in the model with the given \a identifier.  The appointment does
  not have to be in the current filter mode for it to be returned.
*/
QAppointment QAppointmentModel::appointment(const QUniqueId & identifier) const
{
    return d->defaultio->appointment(identifier);
}

/*!
  Returns the value for the specified \a field of the given \a appointment.

  \sa data()
*/
QVariant QAppointmentModel::appointmentField(const QAppointment &appointment, QAppointmentModel::Field field)
{
    switch(field) {
        default:
        case QAppointmentModel::Invalid:
            break;
        case QAppointmentModel::Identifier:
            return QVariant(appointment.uid().toByteArray());
        case QAppointmentModel::Categories:
            return QVariant(appointment.categories());
        case QAppointmentModel::Notes:
            return appointment.notes();
        case QAppointmentModel::Description:
            return appointment.description();
        case QAppointmentModel::Location:
            return appointment.location();
        case QAppointmentModel::Start:
            return appointment.start();
        case QAppointmentModel::End:
            return appointment.end();
        case QAppointmentModel::AllDay:
            return appointment.isAllDay();
        case QAppointmentModel::TimeZone:
            return appointment.timeZone().id();
        case QAppointmentModel::Alarm:
            return (int)appointment.alarm();
        case QAppointmentModel::RepeatRule:
            return (int)appointment.repeatRule();
        case QAppointmentModel::RepeatFrequency:
            return appointment.frequency();
        case QAppointmentModel::RepeatEndDate:
            return appointment.repeatUntil();
        case QAppointmentModel::RepeatWeekFlags:
            return (int)appointment.weekFlags();
    }
    return QVariant();
}

/*!
  Sets the value for the specified \a field of the given \a appointment to \a value.

  Returns true if the appointment was modified.

  \sa setData()
*/
bool QAppointmentModel::setAppointmentField(QAppointment &appointment, QAppointmentModel::Field field,  const QVariant &value)
{
    switch(field) {
        default:
        case QAppointmentModel::Invalid:
        case QAppointmentModel::Identifier: // not a settable field
            return false;
        case QAppointmentModel::Categories:
            if (value.canConvert(QVariant::StringList)) {
                appointment.setCategories(value.toStringList());
                return true;
            }
            return false;
        case QAppointmentModel::Notes:
            if (value.canConvert(QVariant::String)) {
                appointment.setNotes(value.toString());
                return true;
            }
            return false;
        case QAppointmentModel::Description:
            if (value.canConvert(QVariant::String)) {
                appointment.setDescription(value.toString());
                return true;
            }
            return false;
        case QAppointmentModel::Location:
            if (value.canConvert(QVariant::String)) {
                appointment.setLocation(value.toString());
                return true;
            }
            return false;
        case QAppointmentModel::Start:
            if (value.canConvert(QVariant::DateTime)) {
                appointment.setStart(value.toDateTime());
                return true;
            }
            return false;
        case QAppointmentModel::End:
            if (value.canConvert(QVariant::DateTime)) {
                appointment.setEnd(value.toDateTime());
                return true;
            }
            return false;
        case QAppointmentModel::AllDay:
            if (value.canConvert(QVariant::Bool)) {
                appointment.setAllDay(value.toBool());
                return true;
            }
            return false;
        case QAppointmentModel::TimeZone:
            if (value.canConvert(QVariant::String)) {
                appointment.setTimeZone(QTimeZone(value.toString().toAscii()));
                return true;
            }
            return false;
        case QAppointmentModel::Alarm:
            if (value.canConvert(QVariant::Int)) {
                appointment.setAlarm(appointment.alarmDelay(), (QAppointment::AlarmFlags)value.toInt());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatRule:
            if (value.canConvert(QVariant::Int)) {
                appointment.setRepeatRule((QAppointment::RepeatRule)value.toInt());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatFrequency:
            if (value.canConvert(QVariant::Int)) {
                appointment.setFrequency(value.toInt());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatEndDate:
            if (value.canConvert(QVariant::Date)) {
                appointment.setRepeatUntil(value.toDate());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatWeekFlags:
            if (value.canConvert(QVariant::Int)) {
                appointment.setWeekFlags((QAppointment::WeekFlags)value.toInt());
                return true;
            }
            return false;
    }
    return false;
}

/*!
  Updates the appointment in the model with the same identifier as the specified \a appointment to
  equal the specified appointment.

  Returns true if a appointment was successfully updated.
*/
bool QAppointmentModel::updateAppointment(const QAppointment& appointment)
{
    QAppointmentContext *c= qobject_cast<QAppointmentContext *>(context(appointment.uid()));
    if (c && c->updateAppointment(appointment)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Removes the appointment from the model with the same identifier as the specified \a appointment.

  Returns true if a appointment was successfully removed.
*/
bool QAppointmentModel::removeAppointment(const QAppointment& appointment)
{
    return removeAppointment(appointment.uid());
}

/*!
  Removes the appointment from the model with the specified \a identifier.

  Returns true if a appointment was successfully removed.
*/
bool QAppointmentModel::removeAppointment(const QUniqueId& identifier)
{
    QAppointmentContext *c= qobject_cast<QAppointmentContext *>(context(identifier));
    if (c && c->removeAppointment(identifier)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Adds the \a appointment to the model under the specified storage \a source.
  If source is null the function will add the appointment to the default storage source.

  Returns a valid identifier for the appointment if the appointment was
  successfully added.  Otherwise returns a null identifier.

  Note the current identifier of the specified appointment is ignored.
*/
QUniqueId QAppointmentModel::addAppointment(const QAppointment& appointment, const QPimSource &source)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(source));

    QUniqueId id;
    if (c && !(id = c->addAppointment(appointment, source)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Updates the appointment in the model with the same identifier as the specified \a occurrence
  to have an exception on the date of the occurrence.

  Returns true if the exception was successfully added.
*/

bool QAppointmentModel::removeOccurrence(const QOccurrence& occurrence)
{
    return removeOccurrence(occurrence.uid(), occurrence.start().date());
}

/*!
  \overload

  Updates the appointment in the model with the same identifier as the specified \a appointment
  to have an exception on the specified \a date.

  Returns true if the exception was successfully added.
*/

bool QAppointmentModel::removeOccurrence(const QAppointment& appointment, const QDate &date)
{
    return removeOccurrence(appointment.uid(), date);
}

/*!
  \overload
  Updates the appointment in the model with the specified \a identifier to have an
  exception on the specified \a date.

  Returns true if the appointment was successfully updated.
*/

bool QAppointmentModel::removeOccurrence(const QUniqueId &identifier, const QDate &date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(identifier));
    if (c && c->removeOccurrence(identifier, date)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  If an recurring appointment in the model with the specified \a identifier
  has an exception listed for the given \a date, restores the original
  occurrence for that date.  If the exception included a replacement
  appointment will also remove the replacement.

  Returns true if the appointment was successfully updated.
*/
bool QAppointmentModel::restoreOccurrence(const QUniqueId &identifier, const QDate &date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(identifier));
    if (c && c->restoreOccurrence(identifier, date)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Updates the appointment in the model with the same identifier as the specified \a appointment to
  have an exception on the specified \a date.  The \a occurrence will be added as a child occurrence
  of the modified appointment.  If the specified date is null, the start date of the occurrence will be used
  to create the exception.

  Returns a valid identifier for the occurrence if the exception was successfully added. Otherwise
  returns a null identifier.
*/

QUniqueId QAppointmentModel::replaceOccurrence(const QAppointment& appointment, const QOccurrence& occurrence, const QDate& date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(appointment.uid()));
    QUniqueId id;
    if (c && !(id = c->replaceOccurrence(appointment.uid(), occurrence, date)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Updates the appointment in the model with the same identifier as the specified \a appointment to no longer repeat on or
  after the specified \a date.  If the specified date is null, the start date of the \a replacement
  will be used.  The \a replacement appointment will be added to the model.

  Returns a valid identifier for the replacement appointment if the model was successfully modified. Otherwise
  returns a null identifier.
*/

QUniqueId QAppointmentModel::replaceRemaining(const QAppointment& appointment, const QAppointment& replacement, const QDate& date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(appointment.uid()));
    QUniqueId id;
    if (c && !(id = c->replaceRemaining(appointment.uid(), replacement, date)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Removes the records in the model specified by the list of \a identifiers.

  Returns true if appointments were successfully removed.  Otherwise returns false.
*/
bool QAppointmentModel::removeList(const QList<QUniqueId> &identifiers)
{
    // TODO needs better mixing code.
    // e.g. build list for each model loaded,
    // then remove list at a time, instead of appointment at a time.
    QUniqueId id;
    foreach(id, identifiers) {
        if (!exists(id))
            return false;
    }
    foreach(id, identifiers) {
        removeAppointment(id);
    }
    return true;
}

/*!
  \overload

  Adds the PIM record encoded in \a bytes to the model under the specified storage \a source.
  The format of the record in \a bytes is given by the \a format string.  An empty format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  If the specified source is null the function will add the record to the default storage source.

  Returns a valid identifier for the record if the record was
  successfully added.  Otherwise returns a null identifier.

  Can only add PIM data that is represented by the model.  This means that only appointment data
  can be added using a appointment model.  Valid formats are "vCalendar" or an empty string.

  \sa addAppointment()
*/
QUniqueId QAppointmentModel::addRecord(const QByteArray &bytes, const QPimSource &source, const QString &format)
{
    if (format == "vCalendar") {
        QList<QAppointment> list = QAppointment::readVCalendar(bytes);
        if (list.count() == 1)
            return addAppointment(list[0], source);
    } else {
        QAppointment a;
        QDataStream ds(bytes);
        ds >> a;
        return addAppointment(a, source);
    }
    return QUniqueId();
}

/*!
  \overload
  Updates the corresponding record in the model to equal the record encoded in \a bytes.
  The format of the record in \a bytes is given by the \a format string.
  An empty \a format string will cause the record to be read using the data stream operators
  for the PIM data type of the model. If \a id is not null will set the record identifier to \a id
  before attempting to update the record.

  Returns true if the record was successfully updated.

  \sa updateAppointment()
*/
bool QAppointmentModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format)
{
    QAppointment a;
    if (format == "vCalendar") {
        QList<QAppointment> list = QAppointment::readVCalendar(bytes);
        if (list.count() == 1) {
            a = list[0];
        }
    } else {
        QDataStream ds(bytes);
        ds >> a;
    }
    if (!id.isNull())
        a.setUid(id);
    return updateAppointment(a);
}

/*!
  \fn bool QAppointmentModel::removeRecord(const QUniqueId &identifier)
  \overload

  Removes the record from the model with the specified \a identifier.

  Returns true if the record was successfully removed.

  \sa removeAppointment()
*/

/*!
  \overload

  Returns the record in the model with the specified \a identifier encoded in the format specified by the \a format string.
  An empty format string will cause the record to be written using the data stream
  operators for the PIM data type of the model.

  Valid formats are "vCalendar" or an empty string.

  \sa appointment()
*/
QByteArray QAppointmentModel::record(const QUniqueId &identifier, const QString &format) const
{
    QAppointment a = appointment(identifier);
    if (a.uid().isNull())
        return QByteArray();

    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    if (format == "vCalendar") {
        a.writeVCalendar(&ds);
        return bytes;
    } else {
        ds << a;
        return bytes;
    }
    return QByteArray();
}

/*!
   Sets the model to only contain appointments of the specified duration \a type
*/
void QAppointmentModel::setDurationType(DurationType type)
{
    if (d->durationType == type)
        return;
    d->durationType = type;

    d->defaultio->setDurationType(type);
}

/*!
  Returns the duration type of appointments currently included in the model.
*/
QAppointmentModel::DurationType QAppointmentModel::durationType() const
{
    return d->durationType;
}

/*!
  Sets the occurrence model to contain only occurrences
  that end on or after the specified range \a start time and start before
  the specified range \a end time.
  Both start and end time specified should be in local time.
*/
void QAppointmentModel::setRange(const QDateTime &start, const QDateTime &end)
{
    if (d->rStart == start && d->rEnd == end)
        return;
    d->rStart = start;
    d->rEnd = end;

    d->defaultio->setRangeFilter(start, end);
}

/*!
  Returns the start of the range filter for the model.  A null date time will be returned if there is no restriction on the range start.

  \sa setRange()
*/
QDateTime QAppointmentModel::rangeStart() const
{
    return d->rStart;
}

/*!
  Returns the end of the range filter for the model  A null date time will be returned if there is no restriction on the range end.

  \sa setRange()
 */
QDateTime QAppointmentModel::rangeEnd() const
{
    return d->rEnd;
}

// QOccurrenceModel start

class QOccurrenceModelData
{
public:
    QOccurrenceModelData() : requestedCount(-1), rangeChangeTimer(0) {}

    QAppointmentModel *appointmentModel;

    // cache of ranges
    QDateTime start;
    QDateTime end;
    int requestedCount;

    struct OccurrenceItem
    {
        QDateTime startInTZ;
        QDateTime endInTZ;
        QDate date;
        QUniqueId id;
        QAppointment appointment;
        bool minimal;
    };

    mutable QVector< OccurrenceItem > cache;
    mutable QTimer *rangeChangeTimer;
};

/*!
  \class QOccurrenceModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QOccurrenceModel class provides access to the Calendar data.

  It is a descendant of QAbstractItemModel so is suitable for use with the Qt View classes
  such as QListView and QTableView, as well as any custom developer Views.

  It differs from the QAppointmentModel in that it will show each occurrence of appointments as a
  separate item in the model.  Since some appointments have infinite occurrences a date
  range must be specified to limit the total set of occurrences included in the model.  Another important difference
  is that the values for the QAppointmentModel::Start and QAppointmentModel::End fields are converted
  to the current time zone of the device.

  \sa {Pim Library}
*/

/*!
  \enum QAppointmentModel::DurationType

  Describes the set of occurrences that should be shown by the model

  \value TimedDuration Occurrences that have times and dates specified for when they start and end.
  \value AllDayDuration Occurrences that only have a date specified for when they start and end.
  \value AnyDuration Occurrences with either date only or date and time specified for when they start and end.
*/

/*!
  Returns the appointment for the row specified by \a index.
  The column of \a index is ignored.
*/
QAppointment QOccurrenceModel::appointment(const QModelIndex &index) const
{
    return appointment(index.row());
}

/*!
  Returns the appointment for the given \a row.
*/
QAppointment QOccurrenceModel::appointment(int row) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    if (row >= 0 && row < count()) {
        if (od->cache[row].appointment.uid().isNull() || od->cache[row].minimal) {
            od->cache[row].appointment = od->appointmentModel->appointment(od->cache[row].id);
            od->cache[row].minimal = false;
        }
        return od->cache[row].appointment;
    }
    return QAppointment();
}


/*!
  Constructs an occurrence model with the given \a parent that 
  contains only occurrences that end on or after the specified
  range \a start time and start before the specified range \a end time.
  Both start and end time specified should be in local time.

*/
QOccurrenceModel::QOccurrenceModel(const QDateTime &start, const QDateTime &end, QObject *parent)
    : QAbstractItemModel(parent)
{
    init(new QAppointmentModel(this));
    setRange(start, end);
}

/*!
  Constructs an occurrence model with the given \a parent that contains
  only the first \a count occurrences that end
  on or after the specified range \a start time.
  The start time specified should be in local time.
*/
QOccurrenceModel::QOccurrenceModel(const QDateTime &start, int count, QObject *parent)
    : QAbstractItemModel(parent)
{
    init(new QAppointmentModel(this));
    setRange(start, count);
}

void QOccurrenceModel::init(QAppointmentModel *appointmentModel)
{
    od = new QOccurrenceModelData();

    od->appointmentModel = appointmentModel;
    connect(od->appointmentModel, SIGNAL(modelReset()), this, SLOT(forwardAppointmentReset()));

    od->rangeChangeTimer = new QTimer(this);
    od->rangeChangeTimer->setSingleShot(true);
    od->rangeChangeTimer->setInterval(0);

    connect(od->rangeChangeTimer, SIGNAL(timeout()), this, SLOT(rebuildCache()));
}

/*!
  Destroys the occurrence model.
*/
QOccurrenceModel::~QOccurrenceModel()
{
    delete od;
}

/*!
  \fn int QOccurrenceModel::count() const

  Returns the number of occurrences visible in the current filter mode.
*/

/*!
  Return the number of rows under the given \a parent.
*/
int QOccurrenceModel::rowCount(const QModelIndex &parent) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    Q_UNUSED(parent);
    return od->cache.size();
}

/*!
  \reimp
*/
int QOccurrenceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return QAppointmentModel::Categories + 1;   // last column + 1
}

/*!
  \reimp
*/
QMimeData *QOccurrenceModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)
    return 0;
}

/*!
  \reimp
*/
QStringList QOccurrenceModel::mimeTypes() const
{
    return QStringList();
}

QVariant QOccurrenceModel::appointmentData(int row, int column) const
{
    if (row < 0 || row >= count())
        return QVariant();

    QAppointment a;
    switch(column) {
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
            a = od->cache[row].appointment;
            break;

        case QAppointmentModel::Notes:
        case QAppointmentModel::Categories:
            a = appointment(row);
            break;
    }

    // Until we have a data(id) function...
    return QAppointmentModel::appointmentField(a, QAppointmentModel::Field(column));
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.

  The row of the index specifies which appointment to access and the column of the index is treated as
  a \c QAppointmentModel::Field.

  \sa QAppointmentModel::appointmentField()
*/
QVariant QOccurrenceModel::data(const QModelIndex &index, int role) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    QOccurrenceModelData::OccurrenceItem item = od->cache.at(index.row());

    switch(index.column()) {
        case QAppointmentModel::Description:
            switch(role) {
                default:
                    break;
                case Qt::DisplayRole:
                    return appointmentData(index.row(), QAppointmentModel::Description);
                case Qt::EditRole:
                    return item.id.toByteArray();
                case QAppointmentModel::LabelRole:
                    {
                        QString l = appointmentData(index.row(), QAppointmentModel::Description).toString();
                        return "<b>" + l + "</b>";
                    }
                case Qt::BackgroundColorRole:
                    if( appointmentData(index.row(), QAppointmentModel::RepeatRule)
                            != QAppointment::NoRepeat )
                        return QVariant( QColor( 0, 50, 255 ) );
                    else
                        return QVariant( QColor( 255, 50, 0 ) );
                case Qt::DecorationRole:
                    {
                        QList<QVariant> icons;
                        QPimContext *pc = od->appointmentModel->context(item.id);
                        if (pc) {
                            if (!pc->editable(item.id))
                                icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/readonly" ) ) );
                            QIcon contextIcon = pc->icon();
                            if (!contextIcon.isNull())
                                icons.append(contextIcon);
                        }
                        if( item.appointment.hasRepeat() )
                            icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/repeat" ) ) );
                        if( item.appointment.isException() )
                            icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/repeatException" ) ) );
                        if( item.appointment.timeZone() != QTimeZone() && item.appointment.timeZone() != QTimeZone::current() )
                            icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/globe" ) ) );
                        switch( item.appointment.alarm() ) {
                            case QAppointment::Audible:
                                icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/audible" ) ) );
                                break;
                            case QAppointment::Visible:
                                icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/silent" ) ) );
                                break;
                            default:
                                break;
                        }
                        return icons;
                    }
            }
            break;
        case QAppointmentModel::Start:
            return od->cache.at(index.row()).startInTZ;
        case QAppointmentModel::End:
            return od->cache.at(index.row()).endInTZ;
        default:
            break;
    }
    // appointment model ignores role so take advantage of the occurrence model cache.
    return appointmentData(index.row(), index.column());
}

/*!
  \overload
  Returns false since occurrence model does not allow editing.  \a index, \a value and \a role
  are ignored.
*/
bool QOccurrenceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false; // not settable in occurrence model.
}

/*!
  \overload
  Returns false since occurrence model does not allow editing.  \a index and \a values
  are ignored
*/
bool QOccurrenceModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &values)
{
    Q_UNUSED(index)
    Q_UNUSED(values)
    return false;
}

/*!
  \overload

  Returns a map with values for all predefined roles in the model for the item at the
  given \a index.
*/
QMap<int,QVariant> QOccurrenceModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> result;
    switch (index.column()) {
        case QAppointmentModel::Description:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            result.insert(Qt::EditRole, data(index, Qt::EditRole));
            break;
        default:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            break;
    }
    return result;
}

/*!
  Returns true if the current filter mode of the model contains \a index.
*/
bool QOccurrenceModel::contains(const QModelIndex &index) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    return (index.row() >= 0 && index.row() < rowCount());
}

/*!
  Returns true if the current filter mode of the model contains an occurrence
  for the appointment with the specified \a identifier.
*/
bool QOccurrenceModel::contains(const QUniqueId &identifier) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    for (int i = 0; i < od->cache.size(); ++i) {
        if (od->cache.at(i).id == identifier)
            return true;
    }
    return false;
}

/*!
  Returns the index of the first occurrence in the model that represents an appointment with the specified \a identifier.
  If no occurrences are found returns a null index.

  \sa contains()
*/
QModelIndex QOccurrenceModel::index(const QUniqueId &identifier) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    for (int i = 0; i < od->cache.size(); ++i) {
        if (od->cache.at(i).id == identifier)
            return createIndex(i, 0);
    }
    return QModelIndex();
}

/*!
  Return the identifier for the appointment represented by the occurrence
  at the row specified by \a index.  If index is null or out of the range of
  the model will return a null identifier.
*/
QUniqueId QOccurrenceModel::id(const QModelIndex &index) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    return od->cache.at(index.row()).id;
}

/*!
  If the model contains the \a occurrence returns the index of that occurrence.
  Otherwise returns a null index.
*/
QModelIndex QOccurrenceModel::index(const QOccurrence &occurrence) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    if (od->cache.count() < 1)
        return QModelIndex();
    for (int i = 0; i < od->cache.size(); ++i) {
        if (od->cache.at(i).id == occurrence.uid() && od->cache.at(i).date == occurrence.date())
            return createIndex(i, 0);
    }
    return QModelIndex();
}

/*!
  Returns the occurrence for the row specified by \a index.  The column of \a index
 is ignored.
*/
QOccurrence QOccurrenceModel::occurrence(const QModelIndex &index) const
{
    return occurrence(index.row());
}

/*!
  Returns the occurrence at the \a row specified.
*/
QOccurrence QOccurrenceModel::occurrence(int row) const
{
    if (od->rangeChangeTimer->isActive())
        rebuildCache();

    QOccurrenceModelData::OccurrenceItem value = od->cache[row];
    return QOccurrence(value.date, appointment(row));
}

/*!
   Returns the occurrence of the appointment with the specified \a identifier which occurs on
   the specified \a date.  If the appointment does not exists or does not occur on the date,
   returns a null occurrence.
*/
QOccurrence QOccurrenceModel::occurrence(const QUniqueId &identifier, const QDate &date) const
{
    QOccurrence o(date, appointment(identifier));
    if (index(o).isValid())
        return o;
    return QOccurrence();
}

/*!
  Returns the appointment with the specified \a identifier.  The appointment does
  not have to be in the current filter mode for it to be returned.
*/
QAppointment QOccurrenceModel::appointment(const QUniqueId &identifier) const
{
    return od->appointmentModel->appointment(identifier);
}

/*!
  \reimp
*/
QModelIndex QOccurrenceModel::index(int row,int column,const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || row >= rowCount() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

/*!
  \reimp
*/
QModelIndex QOccurrenceModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

/*!
  Sets the model to only contain occurrences of the specified duration \a type.
*/
void QOccurrenceModel::setDurationType(QAppointmentModel::DurationType type)
{
    od->appointmentModel->setDurationType(type);
}

/*!
  Returns the duration type of occurrences currently included in the model.
*/
QAppointmentModel::DurationType QOccurrenceModel::durationType() const
{
    return od->appointmentModel->durationType();
}

// no update or edit functions.
// get the appointment and edit that.

/*!
  Sets the model to contain only the first \a count occurrences that end
  on or after the specified range \a start time.
  The start time specified should be in local time.
*/
void QOccurrenceModel::setRange(const QDateTime &start, int count)
{
    if (!start.isValid() || count < 1)
        return;
    //od->appointmentModel->setRange(start, QDateTime());
    od->start = start;
    od->end = QDateTime();
    od->requestedCount = count;

    od->rangeChangeTimer->start();
    reset();
}

/*!
  Sets the model to only contain occurrences that start before the \a end of the range,
  and end at or after the \a start of the range.  Both \a start and \a end should be
  in the local time zone.
*/
void QOccurrenceModel::setRange(const QDateTime &start, const QDateTime &end)
{
    if (!start.isValid() || !end.isValid() || start >= end)
        return;
    //od->appointmentModel->setRange(start.addDays(-1), end.addDays(1)); // account for TZ
    od->start = start;
    od->end = end;
    od->requestedCount = -1;

    od->rangeChangeTimer->start();
    reset();
}

/*!
  Returns the start of the range specified that occurrences must end at or after to be
  included in the model, in the local time zone.
*/
QDateTime QOccurrenceModel::rangeStart() const
{
    return od->start;
}

/*!
  Returns the end of the range specified that occurrences must start before to be included
  in the model, in the local time zone.

  If the range was specified by a start and a count, this will return a null date time.
*/
QDateTime QOccurrenceModel::rangeEnd() const
{
    return od->end;
}

/*!
  Set the model to only contain occurrences accepted by the specified \a filter.
*/
void QOccurrenceModel::setCategoryFilter(const QCategoryFilter &filter)
{
    od->appointmentModel->setCategoryFilter(filter);
}

/*!
  Returns the category filter that occurrences are tested against for the current filter mode.
*/
QCategoryFilter QOccurrenceModel::categoryFilter() const
{
    return od->appointmentModel->categoryFilter();
}

/*!
  Returns the list of sources of occurrence data that are currently shown by the
  occurrence model.
*/
QSet<QPimSource> QOccurrenceModel::visibleSources() const
{
    return od->appointmentModel->visibleSources();
}

/*!
  Sets the model to show only appointments contained in the storage sources specified
  by \a list.

  Also refreshes the model.
*/
void QOccurrenceModel::setVisibleSources(const QSet<QPimSource> &list)
{
    od->appointmentModel->setVisibleSources(list);
}

/*!
  Returns the set of identifiers for storage sources that can be shown.
*/
QSet<QPimSource> QOccurrenceModel::availableSources() const
{
    return od->appointmentModel->availableSources();
}

/*!
  Returns true if the appointment with the specified \a identifier is stored in the specified storage \a source.
*/
bool QOccurrenceModel::sourceExists(const QPimSource &source, const QUniqueId &identifier) const
{
    return od->appointmentModel->sourceExists(source, identifier);
}

/*!
  Returns true if the occurrence for \a index can be updated or removed.
*/
bool QOccurrenceModel::editable(const QModelIndex &index) const
{
    return editable(id(index));
}

/*!
  Returns true if the appointment with the specified \a identifier can be updated or removed.
*/
bool QOccurrenceModel::editable(const QUniqueId &identifier) const
{
    return od->appointmentModel->editable(identifier);
}

/*!
  Rebuilds the cache of the occurrence model and calls QAbstractItemModel::reset()

  */
void QOccurrenceModel::forwardAppointmentReset()
{
    od->rangeChangeTimer->start();
    reset();
}

/*!
  Rebuilds the cache of repeating occurrences that fit within the range.

  Ensures any recently updates to the model will be reflected in queries against the model.

  This can be an expensive operation.  As QOccurrenceModel automatically rebuilds the cache when
  required it is recommended not to call this function explicitly as this may lead to excessive
  processing and a subsequent loss in application performance.
*/
void QOccurrenceModel::rebuildCache() const
{
    od->rangeChangeTimer->stop();
    /*
       for each appointment, build a list of occurrences.
   */
    if( (!od->end.isNull() || od->requestedCount > 0) && !od->start.isNull()) {
        QMultiMap< QDateTime, QOccurrenceModelData::OccurrenceItem > result;


        // ASSUMPTION, one model and its an SqlIO
        QAppointmentSqlIO *access = od->appointmentModel->d->defaultio;
        if (!access) {
            qWarning("Invalid access class retrieved by Occurrence Model");
            return;
        }

        QList<QAppointment> candidates = access->fastRange(od->start.addDays(-1), od->end.isNull() ? od->end : od->end.addDays(1), od->requestedCount);

        foreach (QAppointment a, candidates) {

            // not first occurrence, that may be far too early.
            QOccurrence o = a.nextOccurrence(od->start.date().addDays(-1));
            int individualCount = 0;
            while(o.isValid()) {
                individualCount++;
                // work out if in range.
                QDateTime start = o.startInCurrentTZ();
                QDateTime end = o.endInCurrentTZ().addSecs(-1);

                if ((od->end.isNull() || start < od->end) && end >= od->start) {
                    QOccurrenceModelData::OccurrenceItem item;
                    item.startInTZ = start;
                    item.endInTZ = end.addSecs(1);
                    item.date = o.date();
                    item.id = o.uid();
                    item.appointment = a;
                    item.minimal = true;
                    result.insert(start, item);
                    if (od->requestedCount > 0 && individualCount >= od->requestedCount)
                        break;
                }
                // escape for forever events going past end range.
                else if (!od->end.isNull() && start > od->end)
                    break;
                // escape for forever events going past max count.
                else if (od->requestedCount > 0 && individualCount >= od->requestedCount)
                    break;
                o = o.nextOccurrence();
            }
        }
        // sort
        od->cache = result.values().toVector();
        // truncate
        if (od->requestedCount > 0 && od->cache.size() > od->requestedCount)
            od->cache.resize(od->requestedCount);
    }
}

/*!
  Forces a refresh of the Occurrence data.
*/
void QOccurrenceModel::refresh()
{
    reset();
}

/*!
  \fn void QOccurrenceModel::fetchCompleted()
  \obsolete

  This signal is emitted when the occurrence model finished caching changes
  resulting from a change to the stored appointments.

  \sa completeFetch(), fetching()
*/

/*!
  \obsolete
  Forces the occurrence model to finish updating its internal cache now.
  This can be expensive and should only be called if no further modifications
  to appointments is expected.  The occurrence model will update its cache in
  the next Qt event loop if this function is not called.

  \sa fetchCompleted(), fetching()
*/
void QOccurrenceModel::completeFetch()
{
}

/*!
  \obsolete
  Returns true if the occurrence model has not yet updated the list of occurrence
  due to a change in stored appointments.

  \sa fetchCompleted(), completeFetch()
*/
bool QOccurrenceModel::fetching() const
{
    return false;
}
