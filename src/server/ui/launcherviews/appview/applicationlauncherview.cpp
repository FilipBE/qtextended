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
#include "applicationlauncherview.h"
#include "uifactory.h"

#include <QMenu>
#include <QSettings>

#include <Qtopia>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QSoftMenuBar>
#include <QFavoriteServicesModel>
#include <QtopiaServiceDescription>


/*!
  \class ApplicationLauncherView
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::GeneralUI
  \brief The ApplicationLauncherView class provides a specialized view for application lists.

  It allows the user to browse through a list of applications and starts the selected application
  if required.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa LauncherView
*/

/*!
  Creates a new ApplicationLauncherView class with the given \a parent and \a flags. To increase 
  the component separation an instance of this class should be created via LauncherView::createLauncherView().

  \code
    LauncherView* view = LauncherView::createLauncherView("ApplicationLauncherView", p, fl);
  \endcode
  */
ApplicationLauncherView::ApplicationLauncherView(QWidget *parent, Qt::WFlags flags)
    : LauncherView(parent, flags), rightMenu(0)
{
#ifndef QTOPIA_HOMEUI
    QMenu * softMenu = QSoftMenuBar::menuFor(this);
    rightMenu = new QMenu(this);
    QAction *a_fav = new QAction(QIcon(":icon/favorite"),
                                   tr("Add to Favorites"), this );
    connect( a_fav, SIGNAL(triggered()), this, SLOT(addFavorite()));
    softMenu->addAction(a_fav);
    rightMenu->addAction(a_fav);

    prop = new QAction(tr("FastLoad"), this );
    prop->setCheckable( true );
    connect( prop, SIGNAL(triggered()), this, SLOT(properties()));
    softMenu->addAction(prop);
    rightMenu->addAction(prop);

    QObject::connect(this, SIGNAL(rightPressed(QContent)),
                     this, SLOT(launcherRightPressed(QContent)));
#endif
    setViewMode(QListView::ListMode);
}


/*!
  \internal
  */
void ApplicationLauncherView::launcherRightPressed(QContent lnk)
{
#ifndef QTOPIA_HOMEUI
    if(!lnk.isValid())
        return;

    rightMenu->popup(QCursor::pos());
#else
    Q_UNUSED(lnk);
#endif
}


void ApplicationLauncherView::addFavorite()
{
    const QContent lnk(currentItem());
    QtopiaServiceRequest sreq;
    sreq = QtopiaServiceRequest("Launcher","execute(QString)");
    sreq << lnk.executableName();
    QtopiaServiceRequest req("Favorites","add(QtopiaServiceDescription)");
    req << QtopiaServiceDescription(sreq, Qtopia::dehyphenate(lnk.name()), lnk.iconName());
    req.send();
}

void ApplicationLauncherView::properties()
{
#ifndef QTOPIA_HOMEUI
    const QContent doc(currentItem());
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("AppLoading");
        QStringList apps = cfg.value("PreloadApps").toStringList();
        QString exe = doc.executableName();

        if ( (apps.contains(exe) > 0) != prop->isChecked() ) {
            if ( prop->isChecked() ) {
                apps.append(exe);
                QtopiaIpcEnvelope e("QPE/Application/"+exe,
                               "enablePreload()");
            } else {
                apps.removeAll(exe);
                QtopiaIpcEnvelope("QPE/Application/"+exe,
                               "disablePreload()");
                QtopiaIpcEnvelope("QPE/Application/"+exe,
                               "quitIfInvisible()");
            }
            cfg.setValue("PreloadApps", apps);
        }
    }
#endif
}

void ApplicationLauncherView::currentChanged(const QModelIndex& current, const QModelIndex&  )
{
#ifndef QTOPIA_HOMEUI
    if ( current.isValid() ) {
        const QContent doc(currentItem());
        if ( doc.id() != QContent::InvalidId && doc.isValid() ) {
            bool fastLoadDisabled = (doc.property("CanFastLoad") == QLatin1String("0"));
            prop->setVisible( !fastLoadDisabled );
            prop->setChecked( doc.isPreloaded() );
        }
    }
#else
    Q_UNUSED(current);
#endif
}
UIFACTORY_REGISTER_WIDGET(ApplicationLauncherView);
