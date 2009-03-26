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

#include "lazycontentstack.h"

#include <QtopiaServiceRequest>
#include <QtopiaServiceHistoryModel>
#include <QtopiaIpcEnvelope>


/*!
    \class LazyContentStack
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer::GeneralUI
    \internal
*/

LazyContentStack::LazyContentStack(Flags lcsFlags, QWidget *parent,
                            Qt::WFlags wflags)
: QWidget(parent, wflags), m_flags(lcsFlags)
{
    connect( &monitor, SIGNAL(applicationStateChanged(QString,UIApplicationMonitor::ApplicationState)), this, SLOT(appStateChanged(QString)) );
}

void LazyContentStack::reset()
{
    m_viewStack.clear();
    emit done();
}

void LazyContentStack::resetToView(const QString &view)
{
    m_viewStack.clear();
    addView(view, true);
}

void LazyContentStack::showView(const QString &view)
{
    if(m_flags && NoStack) {
        m_viewStack.clear();
        addView(view, false);
    } else {
        addView(view, false);
    }
}

QString LazyContentStack::currentView() const
{
    return m_viewStack.top();
}

void LazyContentStack::back()
{
    if(!m_viewStack.isEmpty())
        m_viewStack.pop();

    if(m_viewStack.isEmpty()) {
        emit done();
    } else {
        raiseView(m_viewStack.top(), false);
    }
    notBusy();
}

bool LazyContentStack::isDone() const
{
    return m_viewStack.isEmpty();
}

void LazyContentStack::noView(const QString &)
{
}

void LazyContentStack::busy(const QContent &content)
{
    busyContent = content;
}

void LazyContentStack::notBusy()
{
    if (!busyApp.isEmpty() && busyContent.isValid()) {
        //Store application launch in service history
        QtopiaServiceRequest req("Launcher", "execute(QString)");
        req << busyApp;
        QString label = busyContent.name();
        QString icon = busyContent.iconName();
        QtopiaServiceHistoryModel::insert(req, label, icon);
    }
    busyContent = QContent();
}

void LazyContentStack::addView(const QString &view, bool reset)
{
    if(m_views.contains(view)) {
        m_viewStack.append(view);
        raiseView(view, reset);
    } else {
        QObject *newView = createView(view);
        if(newView) {
            m_views.insert(view);
            m_viewStack.append(view);

            QObject::connect(newView, SIGNAL(clicked(QContent)),
                            this, SLOT(execContent(QContent)));

            raiseView(view, reset);
        } else {
            noView(view);
            if(m_viewStack.isEmpty())
                emit done();
        }
    }
}

void LazyContentStack::execContent(QContent content)
{
    if (!content.type().startsWith("Service/")
        && !content.type().startsWith("Ipc/")
        && !content.type().startsWith("Folder/")
        && content.id() == QContent::InvalidId)
    {
        qLog(DocAPI) << "Attempting to execute an invalid content link:" << content;
        return;
    }

    QString ltype = content.type();

    QRegExp qrs("Service/([^:]*)::(.*)");
    QRegExp ipc("Ipc/([^:]*)::(.*)");

    if (m_views.contains(ltype)) {
        busyApp = QString();
        busy(content);
        showView(ltype);
        notBusy();
    } else if (qrs.exactMatch(ltype)) {
        QtopiaServiceRequest req(qrs.cap(1),qrs.cap(2)); // 1=service, 2=message
        if ( qrs.cap(2).endsWith("(QString)") ) // only 1 string parameter supported
            req << content.fileName();
        req.send();
    } else if (ipc.exactMatch(ltype)) {
        QtopiaIpcEnvelope env(ipc.cap(1),ipc.cap(2));
    } else if (!content.executableName().isNull() ) {
        content.execute();
        QString app = content.executableName();
        if(UIApplicationMonitor::Starting != (monitor.applicationState(app) && UIApplicationMonitor::StateMask)) {
            busyApp = app;
            busy(content);
        }
    } else {
        busyApp = QString();
        busy(content);

        if(currentView()=="Folder/Documents" && !ltype.startsWith("Folder/"))
        {
            //okay we've gotten an error because of no file association, show the open-with dialog.
            if(currentViewObject()->inherits("DocumentLauncherView"))
            {
                //we don't want to reference DocumentLauncherView directly
                QMetaObject::invokeMethod( currentViewObject(), "showOpenWith", Qt::DirectConnection );
                return;
            }
        }
        QObject *newView = createView(ltype);
        if (!newView) {
            noView(ltype);
        } else {
            m_views.insert(ltype);

            QObject::connect(newView, SIGNAL(clicked(QContent)),
                            this, SLOT(execContent(QContent)));

            showView(ltype);
        }

        notBusy();
    }
}

void LazyContentStack::appStateChanged(const QString &app)
{
    if(app == busyApp && UIApplicationMonitor::Starting != (monitor.applicationState(app) && UIApplicationMonitor::StateMask)) {
        notBusy();
        busyApp = QString();
    }
}

