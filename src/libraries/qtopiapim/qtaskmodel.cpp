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
#include "qtaskmodel.h"
#include "qtasksqlio_p.h"
#include <QPainter>
#include <QIcon>
#include <QDebug>

QMap<QTaskModel::Field, QString> QTaskModel::k2i;
QMap<QString, QTaskModel::Field> QTaskModel::i2k;
QMap<QTaskModel::Field, QString>  QTaskModel::k2t;

class QTaskModelData
{
public:
    QTaskModelData() : defaultio(0) {}
    ~QTaskModelData() {}

    static QIcon getCachedIcon(const QString& path);
    static QHash<QString, QIcon> cachedIcons;

    QTaskSqlIO *defaultio;
};

QHash<QString, QIcon> QTaskModelData::cachedIcons;

QIcon QTaskModelData::getCachedIcon(const QString& path)
{
    if (cachedIcons.contains(path))
        return cachedIcons.value(path);

    cachedIcons.insert(path, QIcon(path));
    return cachedIcons.value(path);
}


/*!
  \internal

  Initializes mappings from column enums to translated and non-translated strings.
*/
void QTaskModel::initMaps()
{
    if (k2t.count() > 0)
        return;
    struct KeyLookup {
        const char* ident;
        const char* trans;
        Field key;
    };
    static const KeyLookup l[] = {
        { "completed", QT_TR_NOOP( "Completed" ), Completed },
        { "priority", QT_TR_NOOP( "Priority" ), Priority },
        { "description", QT_TR_NOOP("Description"), Description },
        { "percentcompleted", QT_TR_NOOP( "Percent Completed" ), PercentCompleted },
        { "status", QT_TR_NOOP( "Status" ), Status },
        { "duedate", QT_TR_NOOP( "Due Date" ), DueDate },
        { "starteddate", QT_TR_NOOP( "Started Date" ), StartedDate },
        { "completeddate", QT_TR_NOOP( "Completed Date" ), CompletedDate },
        { "notes", QT_TR_NOOP( "Notes" ), Notes },

        { "identifier", QT_TR_NOOP( "Identifier" ), Identifier},
        { "categories", QT_TR_NOOP( "Categories" ), Categories},

        { "alarms", QT_TR_NOOP( "Alarms" ), Alarm },
        { "repeatrule", QT_TR_NOOP( "Repeat Rule" ), RepeatRule },
        { "repeatfrequency", QT_TR_NOOP( "Repeat Frequency" ), RepeatFrequency },
        { "repeatenddate", QT_TR_NOOP( "Repeat End Date" ), RepeatEndDate },
        { "repeatweekflags", QT_TR_NOOP( "Repeat Week Flags" ), RepeatWeekFlags },
        { 0, 0, Invalid }
    };

    const KeyLookup *k = l;
    while (k->key != Invalid) {
        k2t.insert(k->key, tr(k->trans));
        k2i.insert(k->key, k->ident);
        i2k.insert(k->ident, k->key);
        ++k;
    }
}

/*!
  Returns a translated string describing the task model \a field.

  \sa fieldIcon(), fieldIdentifier(), identifierField()
*/
QString QTaskModel::fieldLabel(Field field)
{
    if (k2t.count() == 0)
        initMaps();
    if (!k2t.contains(field))
        return QString();
    return k2t[field];
}

/*!
  Returns an icon representing the task model \a field.

  Returns a null icon if no icon is available.

  \sa fieldLabel(), fieldIdentifier(), identifierField()
*/
QIcon QTaskModel::fieldIcon(Field field)
{
    QString ident = fieldIdentifier(field);

    if (ident.isEmpty() || !QFile::exists(":icon/todolist/" + ident))
        return QIcon();

    return QTaskModelData::getCachedIcon(":icon/todolist/" + ident);
}


/*!
  Returns a non-translated string describing the task model \a field.

  \sa fieldLabel(), fieldIcon(), identifierField()
*/
QString QTaskModel::fieldIdentifier(Field field)
{
    if (k2i.count() == 0)
        initMaps();
    if (!k2i.contains(field))
        return QString();
    return k2i[field];
}

/*!
  Returns the task model field for the non-translated field \a identifier.

  \sa fieldLabel(), fieldIcon(), fieldIdentifier()
*/
QTaskModel::Field QTaskModel::identifierField(const QString &identifier)
{
    if (i2k.count() == 0)
        initMaps();
    if (!i2k.contains(identifier))
        return Invalid;
    return i2k[identifier];
}


/*!
  \class QTaskModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QTaskModel class provides access to Tasks data.

  User tasks are represented in the task model as a table, with each row corresponding to a
  particular task and each column as on of the fields of the task.  Complete QTask objects can
  be retrieved using the task() function which takes either a row, index, or unique identifier.

  The task model is a descendant of QAbstractItemModel,  so it is suitable for use with the Qt View
  classes such as QListView and QTableView, as well as QTaskListView or any custom views.

  The task model provides functions for filtering of tasks by category and completion status,
  and by default will sort by completion status, priority and description.  Note that this
  sort order may change in future versions of Qtopia.

  The appointment model provides functions for sorting and some filtering of items.
  For filters or sorting that is not provided by the task model, it is recommended that
  QSortFilterProxyModel be used to wrap the task model.

  A QTaskModel instance will also reflect changes made in other instances of QTaskModel,
  both within this application and from other applications.  This will result in
  the modelReset() signal being emitted.

  \sa QTask, QTaskListView, QSortFilterProxyModel, {Pim Library}
*/

/*!
  \enum QTaskModel::Field

  Enumerates the columns when in table mode and columns used for sorting.
  This is a subset of the data retrievable from a QTask.

  \value Invalid
    An invalid field
  \value Description
    The description of the task
  \value Priority
    The priority of the task
  \value Completed
    Whether the task is completed
  \value PercentCompleted
    The percent completed of the task
  \value Status
    The status of the task
  \value DueDate
    The due date of the task
  \value StartedDate
    The started date of the task
  \value CompletedDate
    The completed date of the task
  \value Notes
    The notes of the task
  \value Identifier
    The identifier of the task
  \value Categories
    The list of categories the task belongs to
  \value Alarm
    The type of alarm of the task, if it has a due date
  \value RepeatRule
    The repeat rule of the task, if it has a due date
  \value RepeatFrequency
    The repeat frequency of the task, if it has a due date
  \value RepeatEndDate
    The date a repeating task repeats until, if it has a due
    date.  If null the task repeats forever
  \value RepeatWeekFlags
    The flags specifying what days of the week the task repeats on,
    if it has a due date.
*/

/*!
  Constructs a task model with the given \a parent.
*/
QTaskModel::QTaskModel(QObject *parent)
    : QPimModel(parent)
{
    d = new QTaskModelData;
    QtopiaSql::instance()->openDatabase();

    QTaskSqlIO *access = new QTaskSqlIO(this);
    QTaskDefaultContext *context = new QTaskDefaultContext(this, access);

    setAccess(access);
    d->defaultio = access;
    addContext(context);
}

/*!
  Destroys the task model.
*/
QTaskModel::~QTaskModel()
{
    delete d;
}

/*!
  \reimp
*/
int QTaskModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return RepeatWeekFlags+1;// last column + 1
}

/*!
  Returns the task for the row specified by \a index.
  The column of \a index is ignored.
*/
QTask QTaskModel::task(const QModelIndex &index) const
{
    return task(index.row());
}

/*!
  Returns the task in the model with the given \a identifier.  The task does
  not have to be in the current filter mode for it to be returned.
*/
QTask QTaskModel::task(const QUniqueId & identifier) const
{
    return d->defaultio->task(identifier);
}

/*!
  Return the task for the given \a row.
*/
QTask QTaskModel::task(int row) const
{
    return d->defaultio->task(row);
}

/*!
  Updates the task in the model with the same identifier as the specified \a task to
  equal the specified task.

  Returns true if the task was successfully updated.
*/
bool QTaskModel::updateTask(const QTask &task)
{
    QTaskContext *c= qobject_cast<QTaskContext *>(context(task.uid()));
    if (c && c->updateTask(task)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Removes the task from the model with the same identifier as the specified \a task.

  Returns true if the task was successfully removed.
*/
bool QTaskModel::removeTask(const QTask &task)
{
    return removeTask(task.uid());
}

/*!
  Removes the task from the model with the specified \a identifier.

  Returns true if the task was successfully removed.
*/
bool QTaskModel::removeTask(const QUniqueId& identifier)
{
    QTaskContext *c = qobject_cast<QTaskContext *>(context(identifier));
    if (c && c->removeTask(identifier)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Removes the records in the model specified by the list of \a identifiers.

  Returns true if tasks were successfully removed.
*/
bool QTaskModel::removeList(const QList<QUniqueId> &identifiers)
{
    QUniqueId id;
    foreach(id, identifiers) {
        if (!exists(id))
            return false;
    }
    foreach(id, identifiers) {
        removeTask(id);
    }
    return true;
}

/*!
  Adds the \a task to the model under the specified storage \a source.
  If source is null the function will add the task to the default storage source.

  Returns a valid identifier for the task if the task was
  successfully added.  Otherwise returns a null identifier.

  Note the current identifier of the specified appointment is ignored.
*/
QUniqueId QTaskModel::addTask(const QTask& task, const QPimSource &source)
{
    QTaskContext *c = qobject_cast<QTaskContext *>(context(source));

    QUniqueId id;
    if (c && !(id = c->addTask(task, source)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  \overload

  Adds the PIM record encoded in \a bytes to the model under the specified storage \a source.
  The format of the record in \a bytes is given by \a format.  An empty format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  If the specified source is null the function will add the record to the default storage source.

  Returns a valid identifier for the record if the record was
  successfully added.  Otherwise returns a null identifier.

  Can only add PIM data that is represented by the model.  This means that only task data
  can be added using a task model.  Valid formats are "vCalendar" or an empty string.

  \sa addTask()
*/
QUniqueId QTaskModel::addRecord(const QByteArray &bytes, const QPimSource &source, const QString &format)
{
    if (format == "vCalendar") {
        QList<QTask> list = QTask::readVCalendar(bytes);
        if (list.count() == 1)
            return addTask(list[0], source);
    } else {
        QTask t;
        QDataStream ds(bytes);
        ds >> t;
        return addTask(t, source);
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

  Valid formats are "vCalendar" or an empty string.

  \sa updateTask()
*/
bool QTaskModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format)
{
    QTask t;
    if (format == "vCalendar") {
        QList<QTask> list = QTask::readVCalendar(bytes);
        if (list.count() == 1) {
            t = list[0];
        }
    } else {
        QDataStream ds(bytes);
        ds >> t;
    }
    if (!id.isNull())
        t.setUid(id);
    return updateTask(t);
}

/*!
  \fn bool QTaskModel::removeRecord(const QUniqueId &identifier)
  \overload

  Removes the record from the model with the specified \a identifier.

  Returns true if the record was successfully removed.

  \sa removeTask()
*/

/*!
  \overload

  Returns the record in the model with the specified \a identifier encoded in the format specified by the \a format string.
  An empty format string will cause the record to be written using the data stream
  operators for the PIM data type of the model.

  Valid formats are "vCalendar" or an empty string.

  \sa task()
*/
QByteArray QTaskModel::record(const QUniqueId &identifier, const QString &format) const
{
    QTask t = task(identifier);
    if (t.uid().isNull())
        return QByteArray();

    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    if (format == "vCalendar") {
        t.writeVCalendar(&ds);
        return bytes;
    } else {
        ds << t;
        return bytes;
    }
    return QByteArray();
}

/*! \internal */
void QTaskModel::setSortField(Field field)
{
    if (field == sortField())
        return;

    d->defaultio->setSortKey(field);
}

/*! \internal */
QTaskModel::Field QTaskModel::sortField() const
{
    return d->defaultio->sortKey();
}

/*!
  Sets whether the model contains only completed Tasks to \a completedOnly.
*/
void QTaskModel::setFilterCompleted(bool completedOnly)
{
    if (completedOnly == filterCompleted())
        return;

    d->defaultio->setCompletedFilter(completedOnly);
}

/*!
  Returns true if the model contains only completed tasks.
*/
bool QTaskModel::filterCompleted() const
{
    return d->defaultio->completedFilter();
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.

  The row of the index specifies which task to access and the column of the index is treated as
  a \c QTaskModel::Field.

  \sa taskField()
*/
QVariant QTaskModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return d->defaultio->taskField(index.row(), (Field) index.column());
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        if (d->defaultio->taskField(index.row(), Completed).toBool())
            return d->getCachedIcon(":icon/ok");
        else if(d->defaultio->taskField(index.row(), Priority).toInt() <= 2)
            return d->getCachedIcon(":icon/priority");
    }
    return QVariant();
}

/*!
  \overload
  Sets the \a role data for the item at \a index to \a value. Returns true if successful.

  The task model only accepts data for the \c EditRole.  The column of the specified
  index specifies the \c QTaskModel::Field to set and the row of the index
  specifies which task to modify.

  \sa setTaskField()
*/
bool QTaskModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    QTask t = task(index);
    if (!setTaskField(t, (Field)index.column(), value))
        return false;
    return updateTask(t);

#if 0
    /* disabled due to 'notifyUpdated' require whole record.
       While writing whole record is less efficient than partial - at
       this stage it was the easiest way of fixing the bug where setData
       did not result in cross-model data change from being propagated properly
   */

    int i = index.row();
    const QTaskIO *model = qobject_cast<const QTaskSqlIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((QTaskSqlIO *)model)->setTaskField(r, (Field)index.column(), value);
    return false;
#endif
}

/*!
  \reimp
*/
QMap<int, QVariant> QTaskModel::itemData ( const QModelIndex &index ) const
{
    QMap<int, QVariant> m;
    m.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
    m.insert(Qt::DecorationRole, data(index, Qt::DecorationRole));
    return m;
}

/*!
  \reimp
*/
bool QTaskModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles)
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

/*!
  \reimp
*/
QVariant QTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::DecorationRole)
            return qvariant_cast<QIcon>(fieldIcon((Field)section));
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
    }
    return QVariant();
}

/*!
  \reimp
*/
Qt::ItemFlags QTaskModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

/*!
  Returns a list of indexes for the items where the data matches the specified \a value.
  The list that is returned may be empty.  The search starts from the \a start index.

  Currently unimplemented, this function ignores \a start, \a role, \a value, \a hits
  and \a flags.
*/
QModelIndexList QTaskModel::match(const QModelIndex &start, int role, const QVariant &value,
            int hits, Qt::MatchFlags flags) const
{
    Q_UNUSED(start)
    Q_UNUSED(value)
    Q_UNUSED(hits);
    Q_UNUSED(role);
    Q_UNUSED(flags);

    return QModelIndexList();
}

/*!
  \reimp
*/
QMimeData * QTaskModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    return 0;
}

/*!
  \reimp
*/
QStringList QTaskModel::mimeTypes() const
{
    return QStringList();
}

/*!
  This function will update any recurring tasks
  that require updating (for example, to set the new due
  date).  This should be called in response to
  the QCop message "Tasks::updateRecurringTasks()", or when
  the application wants to ensure that all recurring tasks
  are up to date.

  The library will generate the QCop message at the
  appropriate times.

  Returns true if updates were successful, false otherwise.
*/
bool QTaskModel::updateRecurringTasks()
{
    bool ret = false;
    QList<QPimContext *> ctexts = contexts();

    foreach(QPimContext* p, ctexts) {
        QTaskDefaultContext *td = qobject_cast<QTaskDefaultContext*>(p);
        if (td) {
            td->processRecurringTasks();
            ret = true;
        }
    }

    return ret;
}


/*!
  \overload

  Sorts the model by \a column in ascending order

  Currently \a order is ignored but may be implemented at a future date.
*/
void QTaskModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(order)

    if (column >= 0 && column < columnCount())
        setSortField((Field)column);
}

/*!
  Returns the value for the specified \a field of the given \a task.

  Note:  Certain fields (\c Alarm, \c RepeatRule, \c RepeatFrequency,
  \c RepeatEndDate and \c RepeatWeekFlags) are not accessible by this
  function.  Use \l data() instead, or fetch the dependent QAppointment
  directly.

  \sa data()
 */
QVariant QTaskModel::taskField(const QTask &task, QTaskModel::Field field)
{
    switch(field) {
        default:
        case Invalid:
            break;
        case Identifier:
            return QVariant(task.uid().toByteArray());
        case Categories:
            return QVariant(task.categories());
        case Description:
            return QVariant(task.description());
        case Priority:
            return QVariant(task.priority());
        case Completed:
            return QVariant(task.isCompleted());
        case PercentCompleted:
            return QVariant(task.percentCompleted());
        case Status:
            return QVariant(task.status());
        case DueDate:
            return QVariant(task.dueDate());
        case StartedDate:
            return QVariant(task.startedDate());
        case CompletedDate:
            return QVariant(task.completedDate());
        case Notes:
            return QVariant(task.notes());
    }
    return QVariant();
}

/*!
  Sets the value for the specified \a field of the given \a task to \a value.

  Returns true if the task was modified.

  \sa setData()
*/
bool QTaskModel::setTaskField(QTask &task, QTaskModel::Field field,  const QVariant &value)
{
    switch (field) {
        default:
        case Invalid:
            return false;
        case Description:
            if (value.canConvert(QVariant::String)) {
                task.setDescription(value.toString());
                return true;
            }
            return false;
        case Priority:
            if (value.canConvert(QVariant::Int)) {
                task.setPriority(value.toInt());
                return true;
            }
            return false;
        case Completed:
            if (value.canConvert(QVariant::Bool)) {
                task.setCompleted(value.toBool());
                return true;
            }
            return false;
        case PercentCompleted:
            if (value.canConvert(QVariant::Int)) {
                task.setPercentCompleted(value.toInt());
                return true;
            }
            return false;
        case Status:
            if (value.canConvert(QVariant::Int)) {
                task.setStatus(value.toInt());
                return true;
            }
            return false;
        case DueDate:
            if (value.canConvert(QVariant::Date)) {
                task.setDueDate(value.toDate());
                return true;
            }
            return false;
        case StartedDate:
            if (value.canConvert(QVariant::Date)) {
                task.setStartedDate(value.toDate());
                return true;
            }
            return false;
        case CompletedDate:
            if (value.canConvert(QVariant::Date)) {
                task.setCompletedDate(value.toDate());
                return true;
            }
            return false;
        case Notes:
            if (value.canConvert(QVariant::String)) {
                task.setNotes(value.toString());
                return true;
            }
            return false;
    }
    return false;
}
