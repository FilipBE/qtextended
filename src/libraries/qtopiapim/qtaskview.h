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

#ifndef QTASKVIEW_H
#define QTASKVIEW_H

#include <qtask.h>
#include <qtaskmodel.h>
#include <qpimdelegate.h>

#include <QListView>
#include <QMap>
#include <QDialog>

class QFont;
class QTOPIAPIM_EXPORT QTaskDelegate : public QPimDelegate
{
    Q_OBJECT
public:
    explicit QTaskDelegate(QObject * parent = 0);
    virtual ~QTaskDelegate();

    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const;

    QList<StringPair> subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QFont secondaryHeaderFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;

private:
    QString formatDate(const QDate& date) const;
};

class QTOPIAPIM_EXPORT QTaskListView : public QListView
{
    Q_OBJECT

public:
    explicit QTaskListView(QWidget *parent);
    ~QTaskListView();

    void setModel( QAbstractItemModel * );

    QTask currentTask() const
    {
        if (taskModel() && currentIndex().isValid())
            return taskModel()->task(currentIndex());
        return QTask();
    }

    QList<QTask> selectedTasks() const;
    QList<QUniqueId> selectedTaskIds() const;

    QTaskModel *taskModel() const { return qobject_cast<QTaskModel *>(model()); }

    QTaskDelegate *taskDelegate() const { return qobject_cast<QTaskDelegate *>(itemDelegate()); }
};

class QTaskSelectorPrivate;
class QTOPIAPIM_EXPORT QTaskSelector : public QDialog
{
    Q_OBJECT
public:
    QTaskSelector(bool allowNew, QWidget *);
    ~QTaskSelector();
    void setModel(QTaskModel *);

    bool newTaskSelected() const;
    bool taskSelected() const;
    QTask selectedTask() const;

private slots:
    void setNewSelected();
    void setSelected(const QModelIndex&);
    void taskModelReset();

private:
    QTaskSelectorPrivate *d;
};

#endif
