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

#include <qpimsourcemodel.h>
#include <QIcon>
#include <QDebug>

class QPimSourceModelData
{
public:
    QList<QPimSource> mSourceList;
    QList<QPimContext*> mContextList;
    QSet<QPimSource> mCheckedSources;

    bool mCheckable;
};

/*!
  \class QPimSourceModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimSourceModel class provides a Model class for manipulating a set of QPimSource
  objects.

  This is intended to be used with a QAbstractItemView derived class like
  \l QListView, in order to provide the user with a way to select a source
  (or sources) for their PIM data.  It provides sorting of the sources,
  accessing translated strings for source titles, and the ability to check
  and uncheck sources for use with one of the PIM models (\l QContactModel,
  \l QTaskModel and \l QAppointmentModel).

  By default, this model will provide a non-checkable model, suitable for
  selecting a single source, but if \l setCheckedSources() is called this
  model will allow for multiple sources to be selected.

  \sa QPimModel, QPimContext, {Pim Library}
*/

/*!
  Constructs a QPimSourceModel with parent \a parent.
*/
QPimSourceModel::QPimSourceModel(QWidget *parent)
    : QAbstractListModel(parent)
{
    d = new QPimSourceModelData();
    d->mCheckable = false;
}

/*!
  Destroys the QPimSourceModel.
*/
QPimSourceModel::~QPimSourceModel()
{
    delete d;
}

/*!
  Sets the contexts defining the PIM data sources to display to \a contexts
  The available sources for each context are built into a list of PIM
  data sources.

  Typically, the list of contexts is provided by \l QPimModel::contexts().
*/
void QPimSourceModel::setContexts(const QList<QPimContext *> &contexts)
{
    /*
       build list of sources
       sort list of sources
       emit update, reset style fine in this case.
   */
    d->mContextList = contexts;
    d->mSourceList.clear();
    foreach(QPimContext *con, contexts)
        d->mSourceList += con->sources().toList();

    qSort(d->mSourceList);

    reset();
}

/*!
  \overload

  This is a convenience function for the contexts of a \l QContactModel.
*/

void QPimSourceModel::setContexts(const QList<QContactContext *> &contexts)
{
    d->mContextList.clear();
    d->mSourceList.clear();
    foreach(QContactContext *con, contexts) {
        d->mContextList.append(con);
        d->mSourceList += con->sources().toList();
    }

    qSort(d->mSourceList);

    reset();
}

/*!
  \overload

  This is a convenience function for the contexts of a \l QAppointmentModel.
*/
void QPimSourceModel::setContexts(const QList<QAppointmentContext *> &contexts)
{
    d->mContextList.clear();
    d->mSourceList.clear();
    foreach(QAppointmentContext *con, contexts) {
        d->mContextList.append(con);
        d->mSourceList += con->sources().toList();
    }

    qSort(d->mSourceList);

    reset();
}

/*!
  \overload

  This is a convenience function for the contexts of a \l QTaskModel.
*/
void QPimSourceModel::setContexts(const QList<QTaskContext *> &contexts)
{
    d->mContextList.clear();
    d->mSourceList.clear();
    foreach(QTaskContext *con, contexts) {
        d->mContextList.append(con);
        d->mSourceList += con->sources().toList();
    }

    qSort(d->mSourceList);

    reset();
}

/*!
  Sets the sources that should be marked as checked to those contained in
  \a set.  Calling this function will also indicate that this
  model should allow multiple selections.

  Typically, this function will be called with the results from
  \l QPimModel::visibleSources().

  \sa checkedSources()
*/
void QPimSourceModel::setCheckedSources(const QSet<QPimSource> &set)
{
    d->mCheckable = true;
    d->mCheckedSources = set;

    reset();
}

/*!
  Returns the set of checked sources.

  \sa setCheckedSources()
*/
QSet<QPimSource> QPimSourceModel::checkedSources() const
{
    return d->mCheckedSources;
}

/*!
  Returns the PIM data source at \a index.

  \sa context()
*/
QPimSource QPimSourceModel::source(const QModelIndex &index) const
{
    return d->mSourceList[index.row()];
}

/*!
  Returns the context that controls the PIM data source at \a index.
  Returns 0 if no context controlling the PIM data source is found,
  or if the index is invalid.

  \sa source()
*/
QPimContext *QPimSourceModel::context(const QModelIndex &index) const
{
    QPimSource s = source(index);
    foreach(QPimContext *c, d->mContextList) {
        if (c->sources().contains(s))
            return c;
    }
    return 0;
}

/*!
  Returns the index for this model that corresponds to the PIM data \a source.
  Returns a null index if the source is not listed in the model.
*/
QModelIndex QPimSourceModel::index(const QPimSource &source) const
{
    int i = d->mSourceList.indexOf(source);
    if (i < 0)
        return QModelIndex();
    return createIndex(i, 0);
}

/*!
  Returns the number of rows in the model.  The parameter \a parent is not used.
*/
int QPimSourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->mSourceList.count();
}

/*!
  Returns the item flags for the given \a index.

  If \l setCheckedSources() has been called, these flags will
  include \c Qt::ItemIsUserCheckable, as a hint to the view
  to provide multiple selection capabilities.
*/
Qt::ItemFlags QPimSourceModel::flags(const QModelIndex &index) const
{
    if (d->mCheckable)
        return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
    else
        return QAbstractListModel::flags(index);
}

/*!
  Returns the data stored under the given \a role for the item referred to by the
  \a index.

  This function will return the QPimSource's title for role \c Qt::DisplayRole,
  the icon for the corresponding QPimContext for role \c Qt::DecorationRole, and the
  appropriate check state for \c Qt::CheckStateRole.

  \sa setData()
*/
QVariant QPimSourceModel::data(const QModelIndex &index, int role) const
{
    if (!context(index))
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return context(index)->title(source(index));
        case Qt::DecorationRole:
            return context(index)->icon();
        case Qt::CheckStateRole:
            if (d->mCheckedSources.contains(source(index)))
                return Qt::Checked;
            else
                return Qt::Unchecked;

        default:
            return QVariant();
    }
}

/*!
  Sets the \a role data for the item at \a index to \a value.
  Returns true if successful, otherwise returns false.

  In this implementation only the data for the \c Qt::CheckStateRole
  can be modified this way.

  \sa data()
*/
bool QPimSourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::CheckStateRole)
        return false;
    if (value.toInt() == Qt::Checked)
        d->mCheckedSources.insert(source(index));
    else
        d->mCheckedSources.remove(source(index));
    dataChanged(index, index);
    return true;
}

/*!
  Adds the PIM data \a source to the model

  \sa removeSource(), updateSource()
*/
void QPimSourceModel::addSource(const QPimSource &source)
{
    // first iterate through list to find target row, then insert
    int pos = 0;
    foreach(QPimSource s, d->mSourceList) {
        if (source < s)
            break;
        pos++;
    }
    beginInsertRows(QModelIndex(), pos, pos);
    d->mSourceList.append(source);
    qSort(d->mSourceList);
    endInsertRows();
}

/*!
  Removes the PIM data \a source from the model.

  \sa addSource(), updateSource()
*/
void QPimSourceModel::removeSource(const QPimSource &source)
{
    int pos = d->mSourceList.indexOf(source);
    if (pos < 0)
        return;
    beginRemoveRows(QModelIndex(), pos, pos);
    d->mSourceList.removeAt(pos);
    endRemoveRows();
}

/*!
  Updates the PIM data source at \a index to \a source.  Because the model maintains
  a sort order, the new \a source may not appear at \a index.

  \sa addSource(), removeSource()
*/
void QPimSourceModel::updateSource(const QModelIndex &index, const QPimSource&source)
{
    removeSource(QPimSourceModel::source(index));
    addSource(source);
}

