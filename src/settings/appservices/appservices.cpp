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

#include "appservices.h"
#include "applist.h"
#include "itemfactory.h"

#include <QListWidget>
#include <QLabel>
#include <QWaitWidget>
#include <QVBoxLayout>
#include <QSoftMenuBar>
#include <QMenu>
#include <QAction>

#include <QtopiaApplication>
#include <QTimer>

#include <QtopiaService>

#include <QDebug>

/*!
    \class AppServices
    \inpublicgroup QtDevToolsModule
    \inbrief The AppServices class allows the user to manage the services provided by several applications.

    It also displays a list of all services available with the applications that provide them.
    If a service is provided by several applications, the default application that should provide the service can be selected by checking its name
    in the tree view.

    \internal
*/

/*!
    Constructs a AppServices which is a child of 'parent'
    with widget flags set to 'f'.
*/
AppServices::AppServices(QWidget* parent, Qt::WFlags fl)
    : QDialog(parent, fl)
{
    setWindowTitle(tr("Application Services"));

    QVBoxLayout *vbox = new QVBoxLayout();

    QLabel *label = new QLabel(tr("Available application services"));
    label->setWordWrap(true);
    vbox->addWidget(label);

    m_services = new QListWidget();
    m_services->setWordWrap(true);
    m_services->setSortingEnabled(true);
    m_services->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_services, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(serviceSelected(QListWidgetItem*)));
    vbox->addWidget(m_services);

    setLayout(vbox);

    QAction *action = new QAction(tr("Show All Services"), this);
    action->setCheckable(true);
    action->setChecked(false);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(showAll(bool)));
    QSoftMenuBar::menuFor(this)->addAction(action);

    QTimer::singleShot(0, this, SLOT(loadState()));
}

/*!
    Destroys the AppServices instance.
*/
AppServices::~AppServices()
{
}

/*!
    \internal
*/
void AppServices::loadState()
{
    QWaitWidget *waitWidget = new QWaitWidget(this);
    waitWidget->show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    loadServices();

    delete waitWidget;
}

/*!
    loads the services and the applications that provide them.
*/
void AppServices::loadServices()
{
    QListWidgetItem *firstVisible = 0;

    foreach (const QString &service, QtopiaService::list()) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QListWidgetItem *item = createServiceItem(service);

        if (item) {
            m_services->addItem(item);

            bool hidden = item->data(Qt::UserRole).toInt() <= 1;
            item->setHidden(hidden);

            m_serviceDict.insert(item, service);

            if (!firstVisible && !hidden)
                firstVisible = item;
        }
    }

    m_services->setCurrentItem(firstVisible);
}

void AppServices::serviceSelected(QListWidgetItem *item)
{
    AppList *appList = new AppList(m_serviceDict.value(item));
    QtopiaApplication::execDialog(appList);
    delete appList;
}

void AppServices::showAll(bool all)
{
    QListWidgetItem *firstVisible = 0;

    for (int i = 0; i < m_services->count(); i++) {
        QListWidgetItem *item = m_services->item(i);

        bool hidden = !all && item->data(Qt::UserRole).toInt() <= 1;
        item->setHidden(hidden);

        if (!firstVisible && !hidden)
            firstVisible = item;
    }

    if (!m_services->currentItem() || m_services->currentItem()->isHidden())
        m_services->setCurrentItem(firstVisible);

    m_services->scrollToItem(m_services->currentItem());
}

