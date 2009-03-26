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

#include "taskmanager.h"
#include "taskmanagerservice.h"
#include "applicationlauncher.h"
#include "qabstractmessagebox.h"
#include "launcherview.h"
#include "favoritesservice.h"
#include "qfavoriteserviceslist.h"

#include <QSmoothList>
#include <QtopiaServiceHistoryModel>
#include <QRegExp>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <windowmanagement.h>
#include <QTabWidget>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QtopiaItemDelegate>
#include <QStyleOptionViewItem>
#include <QSize>
#include <QtopiaStyle>
#include <qtopialog.h>
#include <qsoftmenubar.h>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>


// declare MultiTaskProxy
class MultiTaskProxy : public TaskManagerService
{
Q_OBJECT
public:
    MultiTaskProxy(DefaultTaskManager *parent = 0);

protected:
    virtual void multitask();
    virtual void showRunningTasks();

private:
    DefaultTaskManager *tm;
};


// define MultiTaskProxy
MultiTaskProxy::MultiTaskProxy(DefaultTaskManager *parent)
: TaskManagerService(parent), tm( parent )
{
}

void MultiTaskProxy::multitask()
{
    emit tm->multitaskRequested();
}

void MultiTaskProxy::showRunningTasks()
{
    emit tm->showRunningTasks();
}

class TaskManagerDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
public:
    TaskManagerDelegate(QSmoothList *view,QObject *parent=0);
    virtual QSize sizeHint ( const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index */) const;
private:
    int m_height;
    QSmoothList *m_view;
};

/*!
    \class DefaultTaskManager
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer::GeneralUI
    \ingroup QtopiaServer::PhoneUI
    \brief The DefaultTaskManager class provides the standard Qt Extended task manager.

    An image of this taskmanager can be found in the \l{Server Widget Classes}{server widget gallery}.
    It provides quick access to running applications and other recent actions.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractTaskManager
*/

/*!
    Constrcuts a new DefaultTaskManager instance with the specified \a parent and window \a flags.
*/
DefaultTaskManager::DefaultTaskManager (QWidget *parent,Qt::WFlags flags)
    : QAbstractTaskManager(parent,flags)
{
    setWindowTitle("Task Manager");
    setObjectName("taskmanager");
    deferred = false;
    deferredDescription = QtopiaServiceDescription();

    tabs = new QTabWidget(this);
    listRecent = new HistoryLauncherList(QtopiaServiceHistoryModel::Recent, tabs);
    listFrequent = new HistoryLauncherList(QtopiaServiceHistoryModel::Frequency, tabs);
    listFavorites = new QFavoriteServicesList(tabs);
    ralv = LauncherView::createLauncherView( "TaskManagerLauncherView", tabs );
    if (!ralv)
        qLog(Component) << "TaskManager: TaskManagerLauncherView not available";

    td = new TaskManagerDelegate(listRecent,this);
    if (ralv) //don't assume TaskManagerLauncherView is part of the build
        ralv->setItemDelegate(td);
    listRecent->setItemDelegate(td);
    listFrequent->setItemDelegate(td);

    tabs->addTab(listFavorites,QIcon(":icon/favorite"),tr("Favorites"));
    tabs->addTab(listRecent,QIcon(":icon/clock/Clock"),tr("Recent"));
    tabs->addTab(listFrequent,QIcon(":icon/qpe/DocsIcon"),tr("Frequent"));
    if (ralv) //don't assume TaskManagerLauncherView is part of the build
        tabs->addTab(ralv,QIcon(":icon/home"),tr("Running"));
    listFavorites->setFocus();

    QFont listFont(font());
    listFont.setBold(true);
    listRecent->setFont(listFont);
    listFrequent->setFont(listFont);
    listFavorites->setFont(listFont);

    windowManagement = new WindowManagement(this);
    connect(windowManagement, SIGNAL(windowActive(QString,QRect,WId)),
            this, SLOT(windowActive(QString,QRect,WId)));
    connect(listRecent, SIGNAL(requestSent(QtopiaServiceDescription)),
            this, SLOT(setDeferred(QtopiaServiceDescription)));
    connect(listFrequent, SIGNAL(requestSent(QtopiaServiceDescription)),
            this, SLOT(setDeferred(QtopiaServiceDescription)));
    connect(listFavorites, SIGNAL(selected(QModelIndex)),
            this, SLOT(setDeferred()));
    if (ralv)
        connect(ralv, SIGNAL(clicked(QContent)), this, SLOT(launch(QContent)));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->setMargin(0);

    (void)new MultiTaskProxy( this );
    (void)new FavoritesService( this );
}

/*!
  Destroys the DefaultTaskManager instance.
*/
DefaultTaskManager::~DefaultTaskManager()
{
}

void DefaultTaskManager::setDeferred()
{
    deferred = true;
}

void DefaultTaskManager::setDeferred(const QtopiaServiceDescription &desc)
{
    deferred = true;
    deferredDescription = desc;
}

void DefaultTaskManager::launch(QContent content)
{
      QRegExp ipc("Ipc/([^:]*)::(.*)");
      if (!content.executableName().isNull() ) {
          content.execute();
      } else if (ipc.exactMatch(content.type())) {
          QtopiaIpcEnvelope env(ipc.cap(1),ipc.cap(2));
      }
      hide();
}

/*!
    \reimp
*/
bool DefaultTaskManager::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate) {
        listFrequent->resetHistory();
        listRecent->resetHistory();
        listRecent->blockModelSignals(false);
        listFrequent->blockModelSignals(false);
    } else if (e->type() == QEvent::Hide) {
        listRecent->blockModelSignals(true);
        listFrequent->blockModelSignals(true);
    } else if (e->type() == QEvent::Show) {
        if(!iconSize.isValid()) {
            iconSize = LauncherView::listIconSize(listFavorites);
            listFavorites->setIconSize(iconSize);
            listRecent->setIconSize(iconSize);
            listFrequent->setIconSize(iconSize);
        }

    }

    return QAbstractTaskManager::event(e);
}

void DefaultTaskManager::windowActive(const QString& caption, const QRect&, WId)
{
    if (deferred && caption != "Task Manager") {
        hide();
        if(!deferredDescription.isNull()){
            QtopiaServiceHistoryModel::insert(deferredDescription);
            deferredDescription = QtopiaServiceDescription();
        }
        deferred = false;
    }
}

TaskManagerDelegate::TaskManagerDelegate(QSmoothList *view, QObject *parent):QtopiaItemDelegate(parent)
{
    m_view = view;
    m_height  = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize)+2;
}

QSize TaskManagerDelegate::sizeHint ( const QStyleOptionViewItem &/*option*/
        , const QModelIndex &/*index*/ ) const
{
    QSize sz(m_view->parentWidget()->width(),m_view->iconSize().height()+4);
    return sz;
}
//HistoryLauncherList Implementation
/*!
    \class HistoryLauncherList
    \inpublicgroup QtBaseModule
    \internal
    Used primarily because the recent and frequent tabs are practically identical, and making
    this class facilitates code reuse. Also done so as to reimplement mousepressevent() for
    press and hold menu functionality.
*/
HistoryLauncherList::HistoryLauncherList(QtopiaServiceHistoryModel::SortFlags order, QWidget *parent)
    :QSmoothList(parent), model(new QtopiaServiceHistoryModel(this))
{
    model->setSorting(order);
    model->blockSignals(true);
    setModel(model);
    setCurrentIndex(model->index(0,0));

    QMenu *bottomMenu = QSoftMenuBar::menuFor(this);
    if(Qtopia::mousePreferred()) {
        QtopiaApplication::setStylusOperation (this, QtopiaApplication::RightOnHold);
        contextMenu = new QMenu(this);
    } else {
        contextMenu = bottomMenu;
    }
    QAction *favoritesAction = new QAction(tr("Add to Favorites"), this);
    contextMenu->addAction(favoritesAction);
    connect(favoritesAction, SIGNAL(triggered()),
            this, SLOT(favoritesAdd()));
    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(exec(QModelIndex)));
    connect(model, SIGNAL(modelReset()),
            this, SLOT(resetHistory()));
}

HistoryLauncherList::~HistoryLauncherList()
{
}

void HistoryLauncherList::resetHistory()
{
    setCurrentIndex(model->index(0,0));
}

void HistoryLauncherList::blockModelSignals(bool block)
{
    model->blockSignals(block);
}

void HistoryLauncherList::exec(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    QtopiaServiceRequest req(model->serviceRequest(index));
    if(!req.send()){
        QAbstractMessageBox::critical(this,tr("Failed"),tr("Service request unsuccessful."));
    }else{
        emit requestSent(model->serviceDescription(index));
    }
}

void HistoryLauncherList::favoritesAdd()
{
    QtopiaServiceRequest req("Favorites","add(QtopiaServiceDescription)");
    req << model->serviceDescription(currentIndex());
    req.send();
}

void HistoryLauncherList::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton){
        setCurrentIndex(indexAt(event->pos()));
        contextMenu->exec(event->globalPos());
        event->accept();
    } else {
        QSmoothList::mousePressEvent(event);
    }
}

// "taskmanager" builtin
static QWidget *taskmanager()
{
    QtopiaServiceRequest( "TaskManager", "showRunningTasks()" ).send();
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(taskmanager, taskmanager);

QTOPIA_REPLACE_WIDGET(QAbstractTaskManager,DefaultTaskManager)
#include "taskmanager.moc"
