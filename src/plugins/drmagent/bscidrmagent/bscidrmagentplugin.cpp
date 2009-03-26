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

#include "bscidrmagentplugin.h"
#include "bscidrmcontentplugin.h"
#include "bscidrmagentservice.h"
#include "bscirightsmanager.h"
#include "bsciprompts.h"
#include <qtopiaapplication.h>
#include <QtDebug>

BSciDrmAgentPlugin::BSciDrmAgentPlugin()
{
#ifndef QT_NO_TRANSLATION
    QStringList trans;
    trans << "libqtopiaomadrm";
    QtopiaApplication::loadTranslations(trans);
#endif

    if( !BSciDrm::context )
    {
        QString appId = QLatin1String( "QtopiaDrm-" ) + qApp->applicationName();

        BSciDrm::initialiseAgent( appId, BSciPrompts::instance()->callbacks() );
    }
    else
        qWarning() << "Multiple BSciDrmAgentPlugin instances";
}

BSciDrmAgentPlugin::~BSciDrmAgentPlugin()
{
    BSciDrm::releaseAgent();
}

QDrmContentPlugin *BSciDrmAgentPlugin::createDrmContentPlugin()
{
    return new BSciDrmContentPlugin();
}

void BSciDrmAgentPlugin::createService( QObject *parent )
{
    new BSciDrmAgentService( parent );
}

QList< QWidget * > BSciDrmAgentPlugin::createManagerWidgets( QWidget *parent )
{
    QList< QWidget * > widgets;

    BSciRightsView *view = new BSciRightsView( parent );

    view->setModel( new BSciRightsModel( view ) );

    view->setWindowTitle( tr( "Licenses" ) );

    widgets.append( view );

    BSciSettings *settings = new BSciSettings( parent );

    settings->setWindowTitle( tr( "Settings" ) );

    widgets.append( settings );

    return widgets;
}

QTOPIA_EXPORT_PLUGIN(BSciDrmAgentPlugin);

