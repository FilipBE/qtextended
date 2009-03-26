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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H
#include <qtopiaglobal.h>
#include <QContent>
#include <QtopiaServiceDescription>
#include <QtopiaServiceHistoryModel>
#include <QSmoothList>
#include "qabstracttaskmanager.h"

class LauncherView;
class QTabWidget;
class QFavoriteServicesList;
class TaskManagerDelegate;
class QModelIndex;
class QMenu;
class WindowManagement;
class QMouseEvent;
class HistoryLauncherList;

class QTOPIA_EXPORT DefaultTaskManager : public QAbstractTaskManager
{
    Q_OBJECT
public:
    DefaultTaskManager(QWidget *parent = 0, Qt::WFlags f=0);
    virtual ~DefaultTaskManager();

    bool event(QEvent *);
private slots:
    void launch(QContent content);
    void setDeferred();
    void setDeferred(const QtopiaServiceDescription&);
    void windowActive(const QString&,const QRect&, WId);

private:
    HistoryLauncherList *listRecent;
    HistoryLauncherList *listFrequent;
    QTabWidget *tabs;
    LauncherView *ralv;
    QFavoriteServicesList *listFavorites;
    TaskManagerDelegate *td;
    QtopiaServiceDescription deferredDescription;
    WindowManagement *windowManagement;
    QSize iconSize;
    bool deferred;

    friend class MultiTaskProxy;
};

class HistoryLauncherList : public QSmoothList
{
    Q_OBJECT
public:
    HistoryLauncherList(QtopiaServiceHistoryModel::SortFlags order, QWidget *parent=0);
    ~HistoryLauncherList();

public slots:
    void resetHistory();
    void blockModelSignals(bool block);
private slots:
    void exec(const QModelIndex &);
    void favoritesAdd();
signals:
    void requestSent(const QtopiaServiceDescription &desc);
protected:
    void mousePressEvent(QMouseEvent *event);
private:
    QtopiaServiceHistoryModel *model;
    QMenu *contextMenu;
};
#endif
