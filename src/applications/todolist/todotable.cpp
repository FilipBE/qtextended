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

#include "todotable.h"
#include "qtaskview.h"
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QPainter>
#include "qsoftmenubar.h"

TodoTable::TodoTable(QWidget *parent)
    : QListView(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setItemDelegate(new QTaskDelegate(this));
    setLayoutMode(Batched);
    setBatchSize(10);
    setUniformItemSizes(true);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(showTask(QModelIndex)));
}

TodoTable::~TodoTable() {}

void TodoTable::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QListView::currentChanged(current, previous);
    selectionModel()->select(current, QItemSelectionModel::Select);
    /* and save the uid, so we can reselect this if we change things */
    if ( current.isValid() ) {
        lastSelectedTaskId = taskModel()->id(current);
    } else {
        lastSelectedTaskId = QUniqueId();
    }
    emit currentItemChanged(current);
}

void TodoTable::reset()
{
    /* base class stuff */
    QListView::reset();

    /* Select the last selected task, if any, otherwise the first task, if any  */
    if ( !currentIndex().isValid() ) {
        QModelIndex newSel = taskModel()->index(lastSelectedTaskId);
        if ( !newSel.isValid() ) {
            newSel = taskModel()->index(0,0);
        }
        if( newSel.isValid() )
            selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::SelectCurrent);
    }

    /* Emit this if we failed to set the current index */
    if (!currentIndex().isValid())
        emit currentItemChanged(QModelIndex());
}

void TodoTable::showTask(const QModelIndex &i)
{
    if (taskModel()) {
        QTask t = taskModel()->task(i);
        emit taskActivated(t);
    }
}

void TodoTable::setModel( QAbstractItemModel * model )
{
    QTaskModel *tm = qobject_cast<QTaskModel *>(model);
    if (!tm)
        return;
    QListView::setModel(model);
}

void TodoTable::toggleTaskCompleted(const QModelIndex &i)
{
    // Make sure this task is the current one.
    setCurrentIndex(i);

    // And toggle it
    bool current = model()->data(i, Qt::EditRole).toBool();
    model()->setData(i, QVariant(!current), Qt::EditRole);
}


QList<QTask> TodoTable::selectedTasks() const
{
    QList<QTask> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(taskModel()->task(i));
    }
    return res;
}

QList<QUniqueId> TodoTable::selectedTaskIds() const
{
    QList<QUniqueId> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    // selection model for a table will list each cell in the row
    foreach(QModelIndex i, list) {
        QUniqueId id = taskModel()->id(i);
        if (!res.contains(id))
            res.append(id);
    }
    return res;
}

void TodoTable::paintEvent(QPaintEvent *pe)
{
    QListView::paintEvent(pe);
    QTaskModel *tm = taskModel();
    if (tm->rowCount() == 0) {
        QWidget *vp = viewport();
        QPainter p(vp);
        if (tm->categoryFilter().acceptAll()) {
            p.drawText(0, 0, vp->width(), vp->height(), Qt::AlignCenter,
                   tr("No tasks"));
        } else {
            p.drawText(0, 0, vp->width(), vp->height(), Qt::AlignCenter,
                   tr("No matching tasks"));
        }
    }
}

void TodoTable::keyPressEvent(QKeyEvent *e)
{
    QPersistentModelIndex oldCurrent = currentIndex();

    QItemSelectionModel * smodel = selectionModel();
    QModelIndex ci = smodel->currentIndex();

    switch ( e->key() ) {
        case Qt::Key_Select:
            showTask(ci);
            return;

        case Qt::Key_Up:
        case Qt::Key_Down:
            /* Ignore these here, they get handled in the base class */
            break;

        default:
            QModelIndex newmi = ci.sibling(ci.row(), QTaskModel::Description);
            setCurrentIndex(newmi);
            smodel->select(newmi, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }

    QListView::keyPressEvent(e);

    // work around to ensure that we have wrap-around (since QListView doesn't wrap)
    if (model()->rowCount() > 1) {
        if ( (oldCurrent.row() == 0) && (e->key() == Qt::Key_Up) ) {
            // At the beginning of the list, keying down.
            setCurrentIndex(model()->sibling(model()->rowCount()-1,currentIndex().column(),currentIndex()));
        } else if ( (oldCurrent.row() == model()->rowCount()-1) && (e->key() == Qt::Key_Down) ) {
            // At the end of the list, keying up.
            setCurrentIndex(model()->sibling(0,currentIndex().column(),currentIndex()));
        }
    }
}
