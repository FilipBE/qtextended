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

#include "taskmanagerlauncherview.h"
#include "taskmanagerentry.h"
#include "uifactory.h"

#include <QSoftMenuBar>
#include <QtopiaChannel>
#include <QtopiaIpcEnvelope>
#include <QtopiaItemDelegate>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QContent>


class TaskManagerModel : public QContentSetModel 
{
public:
    TaskManagerModel(const QContentSet* set, QObject *parent = 0)
        : QContentSetModel(set, parent)
    {
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        if (role == Qtopia::AdditionalDecorationRole) {
            if (isNotResponding(content(index)))
                return QIcon(":icon/wait");
            else
                return QVariant();
        } else {
            return QContentSetModel::data(index, role);
        }
    }

    void setNotResponding(const QContent& c, bool isBusy)
    {
        bool changed = false;
        if (isBusy && !isNotResponding(c)) {
            busyIds.append(c);
            changed = true;
        } else if ( !isBusy && isNotResponding(c) ) {
            busyIds.removeAll(c);
            changed = true;
        }

        if (changed) {
            QModelIndex index = indexForContent(c);
            if (index.isValid())
                emit dataChanged(index, index);
        }
    }

    bool isNotResponding(const QContent& c) const
    {
        return busyIds.contains(c);
    }
    
    QModelIndex indexForContent(const QContent& c) const
    {
        for (int i=0; i< rowCount(); i++) {
            if (content(i)==c)
                return index(i);
        }
        return QModelIndex();
    }

    
private:
    QList<QContent> busyIds;
};

/*!
   \class TaskManagerLauncherView
    \inpublicgroup QtBaseModule
   \brief The TaskManagerLauncherView class provides the list of running applications.
   \ingroup QtopiaServer::GeneralUI

    An application is automatically added to the list once it changes its state to \c Running.
    This is helpfull in situations whereby an application is backgrounded.

    In addition it is possible to insert artifical entries to the list. Refer to TaskManagerEntry
    for more details about this feature.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa LauncherView, TaskManagerEntry 
*/

const QString TaskManagerLauncherView::LAUNCH_MSG_PREFIX = "launch_";

/*!
  Creates a new TaskManagerLauncherView instance with the given \a parent and \a flags. To increase 
  the component separation an instance of this class should be created via LauncherView::createLauncherView().

  \code
    LauncherView* view = LauncherView::createLauncherView("TaskManagerLauncherView", p, fl);
  \endcode
  */
TaskManagerLauncherView::TaskManagerLauncherView(QWidget *parent, Qt::WFlags flags)
: LauncherView(parent,flags)
{
    setObjectName(QLatin1String("taskmanager"));
    
    QMenu* contextmenu = QSoftMenuBar::menuFor(this); //ensures help
    rightMenu = new QMenu(this);

    a_kill = new QAction(QIcon(":icon/reset"), tr("Terminate application"), this);
    connect(a_kill, SIGNAL(triggered(bool)), this, SLOT(killRequested()));
    contextmenu->addAction(a_kill);
    rightMenu->addAction(a_kill);

    m_contentSet->setSortOrder(QStringList());
    tmodel = new TaskManagerModel(m_contentSet, this);
    setModel(tmodel);

    m_channel = new QtopiaChannel("QPE/RunningAppsLauncherViewService");
    connect(m_channel, SIGNAL(received(QString,QByteArray)),
            SLOT(receivedLauncherServiceMessage(QString,QByteArray)));

    TaskManagerEntry *homeItem =
            new TaskManagerEntry(tr("Home"), "home", this);
    connect(homeItem, SIGNAL(activated()), SLOT(activatedHomeItem()));
    homeItem->show();

    applicationStateChanged("",UIApplicationMonitor::NotRunning);
    QObject::connect(&monitor, SIGNAL(applicationStateChanged(QString,UIApplicationMonitor::ApplicationState)), 
        this, SLOT(applicationStateChanged(QString,UIApplicationMonitor::ApplicationState)));

    QtopiaChannel::send(m_channel->channel(), "runningApplicationsViewLoaded()");
    QObject::connect(this, SIGNAL(rightPressed(QContent)),
            this, SLOT(launcherRightPressed(QContent)));

}

/*!
    \internal
      */
void TaskManagerLauncherView::launcherRightPressed(QContent lnk)
{
    if(!lnk.isValid())
        return;

    rightMenu->popup(QCursor::pos());
}

/*!
  Destroys the TaskManagerLauncherView instance.
  */
TaskManagerLauncherView::~TaskManagerLauncherView()
{
    QList<QString> keys = m_dynamicallyAddedItems.keys();
    for (int i=0; i<keys.size(); i++)
        delete m_dynamicallyAddedItems[keys[i]];
}

void TaskManagerLauncherView::activatedHomeItem()
{
    QtopiaIpcEnvelope env("QPE/System", "showHomeScreen()");
}

// XXX still need to remove Running apps from display when they all exit

void TaskManagerLauncherView::applicationStateChanged( const QString& app, UIApplicationMonitor::ApplicationState state)
{
    QStringList apps = monitor.runningApplications();

    // TODO: now that the model is smart, redo this so that it doesn't recreate it all from scratch.
    QContentSetModel csModel(m_contentSet);
    for (int i=0; i< csModel.rowCount(); i++) {
        QContent c = csModel.content(i);
        if (!m_dynamicallyAddedItems.contains(c.type()) &&
             !apps.contains(c.executableName())) {
            removeItem(c);
        }
    }

    QContentFilter filter(QContent::Application);
    QContentSet set(filter);
    for (int ii=0; ii < apps.count(); ii++) {
        QContent app = set.findExecutable(apps.at(ii));
        if(app.isValid() && !m_contentSet->contains(app)) {
            addItem(new QContent(app), false);
        }
    }

    QContent c = m_contentSet->findExecutable(app);
    if (c.isValid() && 
            (tmodel->isNotResponding(c) != (state & UIApplicationMonitor::NotResponding))) {
        tmodel->setNotResponding(c, state & UIApplicationMonitor::NotResponding);
        
    }
}

void TaskManagerLauncherView::receivedLauncherServiceMessage(const QString &msg, const QByteArray &args)
{
    if (msg == QLatin1String("addDynamicLauncherItem(int,QString,QString)")) {
        QDataStream ds(args);
        int id;
        QString name;
        QString iconPath;
        ds >> id >> name >> iconPath;
        addDynamicLauncherItem(id, name, iconPath);
    } else if (msg == QLatin1String("removeDynamicLauncherItem(int)")) {
        QDataStream ds(args);
        int id;
        ds >> id;
        removeDynamicLauncherItem(id);
    } else if (msg.startsWith(LAUNCH_MSG_PREFIX)) {
        // expect msg to look like "launch_X()" where X is the item's integer ID
        int launchPrefixLength = LAUNCH_MSG_PREFIX.length();
        QByteArray bytes;
        QDataStream ds(&bytes, QIODevice::WriteOnly);
        int id = msg.mid(launchPrefixLength, msg.length() - launchPrefixLength - 2).toInt();
        ds << id;
        QtopiaChannel::send(m_channel->channel(), "activatedLaunchItem(int)", bytes);
    }
}

/*
    Returns the channel/message string that will be used to send an IPC
    message in order to signal that the item with the given \a itemId should
    now be launched.

    This string should be set as the "type" for the QContent that corresponds
    to the given \a itemId.
*/
QString TaskManagerLauncherView::itemActivationIpcMessage(int itemId)
{
    // make a string like "Ipc/QPE/RunningAppsLauncherViewService/launch_X"
    // where X is the item's integer ID
    return QString("Ipc/%1::%2%3()")
            .arg(m_channel->channel())
            .arg(LAUNCH_MSG_PREFIX)
            .arg(itemId);
}

void TaskManagerLauncherView::addDynamicLauncherItem(int id, const QString &name, const QString &iconPath)
{
    QString ipcMsgString = itemActivationIpcMessage(id);

    // don't add same entry twice
    if (m_dynamicallyAddedItems.contains(ipcMsgString))
        return;

    QContent *c = new QContent;
    c->setName(name);
    c->setType(ipcMsgString);
    c->setIcon(iconPath);


    // Use the type string (i.e. the IPC message) as the key instead of the ID
    // so the applicationStateChanged() method in this class can easily tell
    // whether an item a dynamically added launch item, by looking at the
    // content type().
    // (The ID is unique, so the IPC message should also be unique among items.)
    m_dynamicallyAddedItems.insert(ipcMsgString, c);

    addItem(c);
}

void TaskManagerLauncherView::removeDynamicLauncherItem(int id)
{
    QString ipcMsgString = itemActivationIpcMessage(id);
    if (m_dynamicallyAddedItems.contains(ipcMsgString)) {
        QContent *c = m_dynamicallyAddedItems.take(ipcMsgString);
        removeItem(*c);
        delete c;
    }
}

void TaskManagerLauncherView::killRequested()
{
    QContent c = currentItem();
    QString exeName = c.executableName();
    if (c.role() == QContent::Application && !exeName.isEmpty()) {
        ApplicationLauncher* launcher = qtopiaTask<ApplicationLauncher>();
        if (!launcher) return;
        a_kill->setVisible(false);
        bool killed = launcher->kill(exeName);
        if (!killed)
            qWarning() << "Cannot kill" << c.name();
    }
}

/*!
    \reimp
*/
void TaskManagerLauncherView::currentChanged(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    if (!current.isValid())
        return;

    //TaskManagerEntry's are built in and should not have a kill option
    QContent c = tmodel->content(current);

    QList<QContent*> values = m_dynamicallyAddedItems.values();
    foreach(QContent* item, values) {
        if (item->name() == c.name()) {
            a_kill->setVisible(false);
            return;
        }
    }

    a_kill->setVisible(true);
}

/*!
    \reimp
*/
void TaskManagerLauncherView::showEvent(QShowEvent* event)
{
    QtopiaChannel::send(m_channel->channel(), "runningApplicationsViewLoaded()");
    LauncherView::showEvent(event);
}
UIFACTORY_REGISTER_WIDGET(TaskManagerLauncherView);
