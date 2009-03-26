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

#include "qtopiaservicehistorymodel.h"
#include "qtopiaservicehistorymodel_p.h"
#include <QIcon>


/*!
    \class QtopiaServiceHistoryModel
    \inpublicgroup QtBaseModule

    \brief The QtopiaServiceHistoryModel class provides a data model and insertion methods for the Qt Extended service history.

    This class provides methods to add service calls to the service history, and to retrieve
    service calls in the service history. If you wish to have a service call stored in the
    service history you must call this class to insert it, it will not be stored automatically.

    This class is also a data model suitable as the model for a QListView, so that it can display
    the entire service history.
*/
/*!
  
    \enum QtopiaServiceHistoryModel::SortFlags
    Determines sorting order.
    \value History List actions in reverse chronological order. The same action may appear more than once.
    \value Recent Sort actions by time performed, with the most recent at position 0. Each action will only appear once.
    \value Frequency Sort actions by frequency, with the most frequent at position 0. Each action will only appear once.
*/

/*!
    Construct a QtopiaServiceHistoryModel with \a parent.
*/
QtopiaServiceHistoryModel::QtopiaServiceHistoryModel(QObject* parent)
    : QAbstractListModel(parent)
{
    d = new QtopiaServiceHistoryModelPrivate(this);
}

/*!
    Destroys the QtopiaServiceHistoryModel
*/
QtopiaServiceHistoryModel::~QtopiaServiceHistoryModel()
{
    delete d;
}

/*!
    Set the sorting order to \a sort.
*/
void QtopiaServiceHistoryModel::setSorting(SortFlags sort)
{
    if (d->sorting() != sort) {
        d->setSorting(sort);
        reset();
    }
}

/*!
    Return the current sort order.
*/
QtopiaServiceHistoryModel::SortFlags QtopiaServiceHistoryModel::sorting() const
{
    return d->sorting();
}

/*!
    Returns the number of rows under the given \a parent.
*/
int QtopiaServiceHistoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->itemCount();
}

/*!
    Returns the data stored under the given \a role for the item referred
    to by the \a index.
*/
QVariant QtopiaServiceHistoryModel::data(const QModelIndex &index, int role) const
{
    QtopiaServiceDescription serviceDesc = d->getItem(index.row());
    switch (role) {
        case Qt::DecorationRole:
            return QIcon(":icon/"+serviceDesc.iconName());
        case Qt::DisplayRole:
            return serviceDesc.label();
    }

    return QVariant();
}

/*!
    Returns the QtopiaServiceRequest for the item referred
    to by the \a index.
*/
QtopiaServiceRequest QtopiaServiceHistoryModel::serviceRequest(const QModelIndex &index) const
{
    QtopiaServiceDescription serviceDesc = d->getItem(index.row());
    return serviceDesc.request();
}

/*!
    Returns the QtopiaServiceDescription for the item
    referred to by the \a index
*/
QtopiaServiceDescription QtopiaServiceHistoryModel::serviceDescription(const QModelIndex &index) const
{
    QtopiaServiceDescription serviceDesc = d->getItem(index.row());
    return serviceDesc;
}

/*!
    Inserts a new QtopiaServiceDescription \a desc in the history.
*/
void QtopiaServiceHistoryModel::insert(const QtopiaServiceDescription &desc)
{
    QtopiaServiceHistoryModelPrivate::insertItem(desc);
}

/*!
    Inserts a new QtopiaServiceRequest \a request in the history with
    \a label and icon named \a icon.
*/
void QtopiaServiceHistoryModel::insert(const QtopiaServiceRequest &request, const QString &label, const QString &icon)
{
    QtopiaServiceDescription desc(request, label, icon);
    QtopiaServiceHistoryModelPrivate::insertItem(desc);
}


