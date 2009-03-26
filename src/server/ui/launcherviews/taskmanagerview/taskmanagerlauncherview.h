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

#ifndef TASKMANAGERLAUNCHERVIEW_H
#define TASKMANAGERLAUNCHERVIEW_H

#include "launcherview.h"
#include "applicationmonitor.h"

#include <QString>
#include <QByteArray>
#include <QHash>
#include <QEvent>


class QtopiaChannel;
class TaskManagerModel;
class QAction;
class TaskManagerLauncherView : public LauncherView
{
Q_OBJECT
public:
    TaskManagerLauncherView(QWidget * = 0, Qt::WFlags fl = 0);
    ~TaskManagerLauncherView();

protected:
    void showEvent(QShowEvent* event);

protected slots:
    void currentChanged(const QModelIndex &current, const QModelIndex & previous);

private slots:
    void applicationStateChanged(const QString&, UIApplicationMonitor::ApplicationState);
    void receivedLauncherServiceMessage(const QString &msg, const QByteArray &args);
    void launcherRightPressed(QContent lnk);
    void activatedHomeItem();
    void killRequested();

private:
    QString itemActivationIpcMessage(int itemId);
    void addDynamicLauncherItem(int id, const QString &name, const QString &iconPath);
    void removeDynamicLauncherItem(int id);

    static const QString LAUNCH_MSG_PREFIX;
    UIApplicationMonitor monitor;
    QtopiaChannel *m_channel;
    QHash<QString, QContent *> m_dynamicallyAddedItems;

    TaskManagerModel *tmodel;
    QAction *a_kill;
    QMenu* rightMenu;
};

#endif
