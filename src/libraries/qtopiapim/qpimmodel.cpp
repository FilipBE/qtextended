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
#include "qpimmodel.h"
#include <QPainter>
#include <QIcon>
#include <QDebug>

#include "qpimsqlio_p.h"
#include "quniqueid.h"

class QPimModelData
{
public:
    QPimModelData() : defaultContext(0), defaultModel(0), searchModel(0), filterFlags(0) {}
    ~QPimModelData() {
        if (searchModel)
            delete searchModel;
    }

    QPimContext *defaultContext;
    QPimSqlIO *defaultModel;

    QPimSource defaultSource;
    QList<QPimContext *> contexts;
    QHash<uint, QPimContext* > mappedContexts;

    mutable QPimModel *searchModel;
    mutable QString filterText;
    mutable int filterFlags;
};

/*!
  \class QPimModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimModel class provides an abstract interface to the PIM model classes.

  The QPimModel class defines a standard interface that is implemented by all of the PIM
  model classes.  As it is an abstract interface, the QPimModel class should not be used directly.
  Instead use the classes that provide access for the specific PIM data types, QAppointmentModel, QContactModel
  or QTaskModel.
  

  \sa QAppointmentModel, QContactModel, QTaskModel, {Pim Library}
*/

/*!
  \fn QUniqueId QPimModel::addRecord(const QByteArray &bytes, const QPimSource &source = QPimSource(), const QString &format = QString())

  Adds the PIM record encoded in \a bytes to the model under the specified storage \a source.
  The format of the record in \a bytes is given by \a format.  An empty format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  Valid format strings are documented in QAppointmentModel, QContactModel and QTaskModel.
  If the specified source is null the function will add the record to the default storage source.

  Returns a valid identifier for the record if the record was
  successfully added.  Otherwise returns a null identifier.

  Can only add PIM data that is represented by the model.

  \sa updateRecord(), removeRecord()
*/

/*!
  \fn bool QPimModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format = QString())

  Updates the corresponding record in the model to equal the record encoded in \a bytes.
  The format of the record in \a bytes is given by the \a format string.
  An empty \a format string will cause the record to be read using the data stream operators
  for the PIM data type of the model. If \a id is not null will set the record identifier to \a id
  ready from the given \a bytes before attempting to update the record.

  Returns true if the record was successfully updated.

  \sa addRecord(), removeRecord()
*/

/*!
  \fn bool QPimModel::removeRecord(const QUniqueId &identifier)

  Removes the record from the model with the specified \a identifier.

  Returns true if the record was successfully removed.
  
  \sa addRecord(), updateRecord()
*/

/*!
  \fn QByteArray QPimModel::record(const QUniqueId &identifier, const QString &format = QString()) const

  Returns the record in the model with the specified \a identifier encoded in the format specified by the \a format string.
  An empty format string will cause the record to be written using the data stream
  operators for the PIM data type of the model.
*/

/*!
  Constructs a PIM model with the given \a parent.
*/
QPimModel::QPimModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new QPimModelData();
}

/*!
  Destroys the PIM model.
*/
QPimModel::~QPimModel()
{
    delete d;
}

/*!
  \internal

  Adds the record \a context to the QPimModel.  If no contexts have
  been added previously the \a context is set to be the default context.
  \sa context()
*/
void QPimModel::addContext(QPimContext *context)
{
    Q_ASSERT(context);
    if (!d->defaultContext) {
        d->defaultContext = context;
        d->defaultSource = context->defaultSource();
    }
    d->contexts.append(context);
    QSet<QPimSource> sources = context->sources();
    foreach(QPimSource s, sources)
        d->mappedContexts.insert(QPimSqlIO::sourceContext(s), context);
}

/*!
  \internal
  Sets the access to records for the model to the given \a accessModel
*/
void QPimModel::setAccess(QPimSqlIO *accessModel)
{
    d->defaultModel = accessModel;
    connect(d->defaultModel, SIGNAL(recordsUpdated()), this, SLOT(voidCache()));
}

/*!
    \internal
    Causes the model to reset notifying any attached views to update their representation
    of the model.
*/
void QPimModel::voidCache()
{
    reset();
}

/*!
   Returns the contexts of record data that is shown by the record model.
*/
const QList<QPimContext*> &QPimModel::contexts() const
{
    return d->contexts;
}

/*!
  Returns the list of sources of record data that is currently shown by the
  record model.

  \sa setVisibleSources(), availableSources()
*/
QSet<QPimSource> QPimModel::visibleSources() const
{
    QSet<QPimSource> set;
    foreach(QPimContext *c, d->contexts)
        set.unite(c->visibleSources());
    return set;
}

/*!
  Sets the model to show only records contained in the storage sources specified
  by \a list.

  Also refreshes the model.

  \sa visibleSources(), availableSources()
*/
void QPimModel::setVisibleSources(const QSet<QPimSource> &list)
{
    foreach (QPimContext *c, d->contexts) {
        QSet<QPimSource> cset = c->sources();
        cset.intersect(list);
        c->setVisibleSources(cset);
    }
    refresh();
}

/*!
  Returns the set of storage sources that can be shown.

  \sa setVisibleSources(), visibleSources()
*/
QSet<QPimSource> QPimModel::availableSources() const
{
    QSet<QPimSource> set;
    foreach(QPimContext *c, d->contexts)
        set.unite(c->sources());
    return set;
}

/*!
  Returns the default storage sources relating to the device memory.
  \sa source(), availableSources(), visibleSources()
 */
QPimSource QPimModel::defaultSource() const
{
    return d->defaultSource;
}

/*!
  Returns the storage source that contains the record with the specified \a identifier.
  Returns a null source if the record does not exist.

  \sa availableSources()
*/
QPimSource QPimModel::source(const QUniqueId &identifier) const
{
    return d->defaultModel->source(identifier);
}

/*!
  Returns the context that could contain the record with specified \a identifier.
  This function does not check if the record currently exists in the
  corresponding context, but instead returns the context that this identifier
  belongs to.  You can call \c QPimContext::exists() if you wish to be more
  certain, but be aware that other threads or processes may make the result
  out of date.

  Returns 0 if this identifier does not belong to any context.

  \sa contexts()
*/
QPimContext *QPimModel::context(const QUniqueId &identifier) const
{
    return d->mappedContexts.value(d->defaultModel->context(identifier));
}

/*!
  Prepares the currently visible sources for synchronization.
  All modifications between startSyncTransaction() and commitSyncTransaction()
  will be marked with the given \a timestamp for modification or creation as appropriate.

  Returns true if transaction successfully initiated.

  Does not abort the transaction if it is unsuccessful. The calling code must call
  abortSyncTransaction() to do that.

  Timestamps are stored in UTC.  Please use QTimeZone::toUtc() for times
  passed into this function.

  \sa commitSyncTransaction(), abortSyncTransaction()
*/
bool QPimModel::startSyncTransaction(const QDateTime &timestamp)
{
    return d->defaultModel->startSyncTransaction(visibleSources(), timestamp);
}

/*!
    Starts an SQL transaction on the database.  Allows for calling multiple
    functions that may modify the database as an atomic operation.

    Note that the database will be locked for the current model only.  Do
    not modify any other PIM model instance until either committing or
    aborting the transaction.

    Returns true if transaction successfully initiated.

    \sa commitTransaction(), abortTransaction()
*/
bool QPimModel::startTransaction()
{
    return d->defaultModel->startTransaction();
}

/*!
    Aborts the current synchronization transaction.

    Returns true if transaction successfully aborted.

    \sa startSyncTransaction(), commitSyncTransaction()
*/
bool QPimModel::abortSyncTransaction()
{
    return d->defaultModel->abortSyncTransaction();
}

/*!
    Commits the current synchronization transaction.

    Returns true if transaction successfully committed.

    Does not abort transaction if unsuccessfully committed.

    \sa startSyncTransaction(), abortSyncTransaction()
*/
bool QPimModel::commitSyncTransaction()
{
    return d->defaultModel->commitSyncTransaction();
}

/*!
    \fn bool QPimModel::abortTransaction();
    Aborts the current transaction.

    Returns true if transaction successfully aborted.

    \sa startTransaction(), commitTransaction()
*/

/*!
    \fn bool QPimModel::commitTransaction();
    Commits the current transaction.

    Returns true if transaction successfully committed.

    Does not abort transaction if unsuccessfully committed.

    \sa startTransaction(), abortTransaction()
*/

/*!
  Returns the list of identifiers for records removed from the current set of visible sources
  on or after the specified \a timestamp.

  Timestamps are stored in UTC.  Please use QTimeZone::toUtc() for times
  passed into this function.

  \sa added(), modified()
*/
QList<QUniqueId> QPimModel::removed(const QDateTime &timestamp) const
{
    return d->defaultModel->removed(visibleSources(), timestamp);
}

/*!
  Returns the list of identifiers for records added to the current set of visible sources
  on or after the specified \a timestamp.

  Timestamps are stored in UTC.  Please use QTimeZone::toUtc() for times
  passed into this function.

  \sa removed(), modified()
*/
QList<QUniqueId> QPimModel::added(const QDateTime &timestamp) const
{
    return d->defaultModel->added(visibleSources(), timestamp);
}

/*!
  Returns the list of identifiers for records modified in the current set of visible sources
  on or after the specified \a timestamp.

  Timestamps are stored in UTC.  Please use QTimeZone::toUtc() for times
  passed into this function.

  \sa added(), removed()
*/
QList<QUniqueId> QPimModel::modified(const QDateTime &timestamp) const
{
    return d->defaultModel->modified(visibleSources(), timestamp);
}

/*!
  Return the number of records visible in the in the current filter mode.
*/
int QPimModel::count() const
{
    return d->defaultModel->count();
}

/*!
  \reimp
*/
int QPimModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return count();
}

/*!
  Returns true if the current filter mode of the model contains the given \a index.

  \sa exists()
*/
bool QPimModel::contains(const QModelIndex &index) const
{
    return (index.row() >= 0 && index.row() < count());
}

/*!
  Returns true if the current filter mode of the model contains the record with given \a identifier.

  \sa exists()
*/
bool QPimModel::contains(const QUniqueId & identifier) const
{
    return index(identifier).isValid();
}

/*!
  Returns true if a record with the given \a identifier is stored in the model.

  The specified record does not need to be in the current filter mode.

  \sa contains()
*/
bool QPimModel::exists(const QUniqueId &identifier) const
{
    return d->defaultModel->exists(identifier);
}

/*!
  Returns true if the record for the given \a index can be updated or removed.
*/
bool QPimModel::editable(const QModelIndex &index) const
{
    return editable(id(index));
}

/*!
  Returns true if the record for the given \a identifier can be updated or removed.
*/
bool QPimModel::editable(const QUniqueId &identifier) const
{
    foreach (QPimContext *c, d->contexts) {
        if (c->exists(identifier))
            return c->editable(identifier);
    }
    return false;
}

/*!
  Returns true if the record for the given \a identifier is stored in
  the specified storage \a source.

  \sa exists(), availableSources()
*/
bool QPimModel::sourceExists(const QPimSource &source, const QUniqueId &identifier) const
{
    foreach (QPimContext *c, d->contexts) {
        if (c->sources().contains(source))
            return c->exists(identifier, source);
    }
    return false;
}

/*!
  Returns the index of the record for the given \a identifier.  If no
  record is found returns a null index.

  \sa contains(), exists()
*/
QModelIndex QPimModel::index(const QUniqueId & identifier) const
{
    int i = d->defaultModel->row(identifier);
    if (i == -1)
        return QModelIndex();
    return createIndex(i, 0);
}

/*!
  \reimp
*/
QModelIndex QPimModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}


/*!
  \reimp
*/
bool QPimModel::hasChildren(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return false;
}

/*!
  Ensures the data for the model is in a state suitable for syncing.

  Returns true upon success.

  \sa startSyncTransaction()
*/
bool QPimModel::flush() {
    return true;
}

/*!
  Forces a refresh of the data for the model.  The PIM data models update automatically and it should not be required that this function is called in normal use of the PIM data models.

  Returns true upon success.
*/
bool QPimModel::refresh() {
    return true;
}

/*!
  Returns the identifier for the record at the specified \a index.
  If index is null or out of the range of the model, will return a null identifier.
*/
QUniqueId QPimModel::id(const QModelIndex &index) const
{
    int i = index.row();
    return id(i);
}

/*!
  Returns the identifier for record at the specified \a row.
*/
QUniqueId QPimModel::id(int row) const
{
    return d->defaultModel->id(row);
}

/*!
  \reimp
*/
QModelIndex QPimModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row < 0 || row >= count() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

/*!
  If the given \a source exists in the model returns the context that provides that
  source.  Otherwise returns 0.

  \sa contexts(), availableSources()
*/
QPimContext *QPimModel::context(const QPimSource &source) const
{
    QPimContext *c = 0;
    if (source.isNull()) {
        c = d->defaultContext;
    } else {
        foreach(QPimContext *context, d->contexts) {
            if (context->sources().contains(source)) {
                c = context;
                break;
            }
        }
    }
    return c;
}

/*!
  Sets the model to only contain records accepted by the
  specified category \a filter.

  \sa categoryFilter()
*/
void QPimModel::setCategoryFilter(const QCategoryFilter &filter)
{
    if (filter == categoryFilter())
        return;

    d->defaultModel->setCategoryFilter(filter);
}

/*!
  Returns the category filter that records are tested against in the current filter mode.

  \sa setCategoryFilter()
*/
QCategoryFilter QPimModel::categoryFilter() const
{
    return d->defaultModel->categoryFilter();
}
