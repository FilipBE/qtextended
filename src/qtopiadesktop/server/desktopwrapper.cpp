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
#include "desktopwrapper.h"
#include "qtopiadesktopapplication.h"
#include "pluginmanager.h"
#include "connectionmanager.h"
#include "syncmanager.h"

#include <qdplugin.h>

DesktopWrapper::DesktopWrapper( QDPlugin *_plugin )
    : QObject( _plugin ),
    plugin( _plugin )
{
    q = qobject_cast<QtopiaDesktopApplication*>(qApp);
    Q_ASSERT( q );
}

DesktopWrapper::~DesktopWrapper()
{
}

QDDevPlugin *DesktopWrapper::currentDevice()
{
    return q->connectionManager?q->connectionManager->currentDevice():0;
}

const QDDevPluginList DesktopWrapper::devicePlugins()
{
    if ( qobject_cast<QDConPlugin*>(plugin) )
        return qdPluginManager()->devPlugins();
    return QDDevPluginList();
}

const QDLinkPluginList DesktopWrapper::linkPlugins()
{
    if ( qobject_cast<QDConPlugin*>(plugin) )
        return qdPluginManager()->linkPlugins();
    return QDLinkPluginList();
}

QDPlugin *DesktopWrapper::getPlugin( const QString &id )
{
    foreach ( QDPlugin *plugin, qdPluginManager()->plugins() )
        if ( plugin->id() == id )
            return plugin;
    return 0;
}

QObject *DesktopWrapper::syncObject()
{
    if ( qobject_cast<QDSyncPlugin*>(plugin) || qobject_cast<QDClientSyncPluginFactory*>(plugin) ) {
        Q_ASSERT(q->syncManager);
        return q->syncManager->syncObject();
    }
    return 0;
}

