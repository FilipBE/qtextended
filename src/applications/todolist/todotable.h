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

#ifndef TODOTABLE_H
#define TODOTABLE_H

#include <QListView>
#include <QList>
#include "qtask.h"
#include "qtaskmodel.h"

class TodoTable : public QListView
{
    Q_OBJECT

public:
    TodoTable( QWidget *parent = 0 );
    ~TodoTable();

    void setModel( QAbstractItemModel * model );

    QTask currentTask() const
    {
        if (taskModel() && currentIndex().isValid())
            return taskModel()->task(currentIndex());
        return QTask();
    }

    QList<QTask> selectedTasks() const;
    QList<QUniqueId> selectedTaskIds() const;

    QTaskModel *taskModel() const { return qobject_cast<QTaskModel *>(model()); }

    void paintEvent(QPaintEvent *pe);

signals:
    void currentItemChanged(const QModelIndex &);
    void taskActivated(const QTask &);

protected:
    void keyPressEvent(QKeyEvent *);

protected slots:
    void currentChanged(const QModelIndex &, const QModelIndex &);
    void showTask(const QModelIndex &);
    void reset();
    void toggleTaskCompleted(const QModelIndex &i);

private:
    // since needs to treat as list, not spreadsheet.
    void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)
    { QListView::setSelectionBehavior(behavior); }

    QUniqueId lastSelectedTaskId;
};

#endif
