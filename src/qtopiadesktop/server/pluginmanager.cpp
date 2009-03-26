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
#include "pluginmanager.h"
#include "mainwindow.h"
#include "qpluginmanager.h"

#include <qdplugin.h>
#include <private/qdplugin_p.h>

#include <qtopiadesktoplog.h>
#include <desktopsettings.h>

#include <QMap>
#include <QWidget>
#include <QStackedWidget>
#include <qdebug.h>

extern QList<qdPluginCreateFunc_t> *qdInternalPlugins();

/*!
    \relates PluginManager
    Returns the singleton PluginManager instance.
*/
PluginManager *qdPluginManager()
{
    static PluginManager *pm = 0;
    if ( !pm )
        pm = new PluginManager( 0 );
    return pm;
}

static bool qdpluginLessThan( QDPlugin *p1, QDPlugin *p2 )
{
    return ( p1->displayName() < p2->displayName() );
}

// ==========================================================================

class PluginManagerPrivate
{
public:
    // This is the Qtopia class that wraps up loading of plugins
    QPluginManager *pluginManager;
    // An evil(tm) array thinggy
    QMap<QDPlugin*,QDPluginData*> pluginData;
    QDPluginList allPlugins;
    QDAppPluginList appPlugins;
    QDConPluginList conPlugins;
    QDDevPluginList devPlugins;
    QDLinkPluginList linkPlugins;
    QDSyncPluginList syncPlugins;

    QStringList disabledPluginFiles;
    QStringList detectedPluginFiles;
    QStringList disabledPluginIds;
    QMap<QString,QString> detectedPluginIds;
};

// ==========================================================================

/*!
    \class PluginManager
    \brief The PluginManager class manages plugin loading.
*/

/*!
    \internal
*/
PluginManager::PluginManager( QObject *parent )
    : QObject( parent )
{
    TRACE(PM) << "PluginManager::PluginManager";
    d = new PluginManagerPrivate;

    DesktopSettings settings("settings");
    d->disabledPluginFiles = settings.value("DisabledPluginFiles").toStringList();
    d->disabledPluginIds = settings.value("DisabledPluginIds").toStringList();
    d->disabledPluginIds.removeAll("com.trolltech.plugin.app.infopage");
}

/*!
    \internal
*/
PluginManager::~PluginManager()
{
    TRACE(PM) << "PluginManager::~PluginManager";
    // Clean up the widgets nicely (if we don't do it here, they'll miss out)
    foreach ( QDAppPlugin *plugin, d->appPlugins ) {
        QDAppPluginData *dta = pluginData(plugin);
        if ( dta->appWidget )
            delete dta->appWidget;
        if ( dta->settingsWidget )
            delete dta->settingsWidget;
        if ( dta->mainWindow )
            delete dta->mainWindow;
    }
    foreach ( QDPlugin *plugin, d->allPlugins ) {
        QDPluginData *dta = pluginData( plugin );
        LOG() << "delete plugin";
        delete plugin;
        LOG() << "delete plugin data";
        delete dta;
    }
    delete d;
}

/*!
    Returns the active application plugins.
*/
QDAppPluginList PluginManager::activePlugins()
{
    return QDAppPluginList();
}

/*!
    Returns the current application plugin.
    Note that this is currently hard-coded to return 0!
*/
QDAppPlugin *PluginManager::currentPlugin()
{
    return 0;
}

/*!
    Returns the plugin data for \a plugin.
*/
QDPluginData *PluginManager::pluginData( QDPlugin *plugin )
{
    return d->pluginData[plugin];
}

/*!
    Returns the plugin data for \a plugin.
*/
QDAppPluginData *PluginManager::pluginData( QDAppPlugin *plugin )
{
    return (QDAppPluginData*)d->pluginData[plugin];
}

/*!
    \internal
*/
void PluginManager::setupPlugins( bool safeMode )
{
    TRACE(PM) << "PluginManager::setupPlugins" << "safeMode" << safeMode;

    DesktopSettings settings("settings");
    // Grab a list of the instances we can find
    QList<QObject*> instances;

    // Builtin plugins
    QList<qdPluginCreateFunc_t> internalPlugins( *qdInternalPlugins() );
    foreach( qdPluginCreateFunc_t createFunc, internalPlugins ) {
        QDPlugin *plugin = createFunc( this );
        LOG() << "Internal plugin" << plugin->displayName();
        instances.append( plugin );
    }

    // External factories
    QStringList loadedPluginFiles;
    d->pluginManager = new QPluginManager("qtopiadesktop");
    foreach ( const QString &name, d->pluginManager->list() ) {
        d->detectedPluginFiles << name;
        if ( safeMode || d->disabledPluginFiles.contains(name) )
            continue;
        loadedPluginFiles << name;
        LOG() << "Loading external plugin file" << name;
        QObject *instance = d->pluginManager->instance(name);
        if ( !instance ) {
            WARNING() << name << "is not a Qt plugin!";
            continue;
        }
        instance->setObjectName( name.toLatin1().constData() );
        instances.append( instance );
    }

    // Extract the QDPlugins from the factories
    foreach ( QObject *instance, instances ) {
        if ( QDPluginFactory *pf = qobject_cast<QDPluginFactory*>(instance) ) {
            foreach ( const QString &key, pf->keys() ) {
                LOG() << "Constructing" << key;
                QDPlugin *plugin = pf->create( key );
                if ( plugin )
                    internal_setupPlugin( plugin, safeMode, true );
                else
                    WARNING() << "plugin factory" << instance << "did not create advertised plugin" << key;
            }
        } else if ( QDPlugin *plugin = qobject_cast<QDPlugin*>(instance) ) {
            internal_setupPlugin( plugin, safeMode, true );
        } else {
            WARNING() << instance->objectName() << "is not a plug-in type that Qtopia Sync Agent recognizes";
            delete instance;
        }
    }

    // Sort the plugins in a consistent way
#define SORT(list)\
    qSort(d-> list ## Plugins .begin(), d-> list ## Plugins .end(), qdpluginLessThan)
    SORT(all);
    SORT(app);
    SORT(con);
    SORT(dev);
    SORT(link);
    SORT(sync);
#undef SORT

    QStringList loadedPlugins;
    foreach ( QDPlugin *p, d->allPlugins ) {
        loadedPlugins << p->id();
    }
    // This sorted because d->allPlugins is sorted
    settings.setValue("loadedPlugins", loadedPlugins);
    loadedPluginFiles.sort();
    settings.setValue("loadedPluginFiles", loadedPluginFiles);
}

/*!
    Initialize a \a plugin that has been manually constructed.
    For example, this is used to initialize dynamically created QDClientSyncPlugin instances.
    After calling this method you should call QtopiaDesktopApplication::initializePlugin().
*/
void PluginManager::setupPlugin( QDPlugin *plugin )
{
    internal_setupPlugin( plugin, false, false );
}

/*!
    \internal
*/
void PluginManager::internal_setupPlugin( QDPlugin *plugin, bool safeMode, bool addToLists )
{
    TRACE(PM) << "PluginManager::setupPlugin" << "plugin" << plugin << "safeMode" << safeMode;
    d->detectedPluginIds[plugin->id()] = plugin->displayName();
    if ( ( safeMode && plugin->id() != "com.trolltech.plugin.app.infopage" ) ||
         d->disabledPluginIds.contains(plugin->id()) ) {
        LOG() << "Plugin" << plugin->id() << "disabled.";
        delete plugin;
        return;
    }

    LOG() << "Setting up plugin" << plugin->id();
    QDPluginData *dta = 0;
    if ( addToLists ) d->allPlugins.append( plugin );
    if ( QDAppPlugin *p = qobject_cast<QDAppPlugin*>(plugin) ) {
        // Application plugins
        if ( addToLists ) d->appPlugins.append(p);
        dta = new QDAppPluginData;
    } else if ( QDConPlugin *p = qobject_cast<QDConPlugin*>(plugin) ) {
        // Connection plugins
        if ( addToLists ) d->conPlugins.append(p);
        dta = new QDPluginData;
    } else if ( QDDevPlugin *p = qobject_cast<QDDevPlugin*>(plugin) ) {
        // Device plugins
        if ( addToLists ) d->devPlugins.append(p);
        dta = new QDPluginData;
    } else if ( QDLinkPlugin *p = qobject_cast<QDLinkPlugin*>(plugin) ) {
        // Link plugins (eg. USB, Serial, LAN, QVfb)
        if ( addToLists ) d->linkPlugins.append(p);
        dta = new QDLinkPluginData;
    } else if ( QDSyncPlugin *p = qobject_cast<QDSyncPlugin*>(plugin) ) {
        // Sync plugins (eg. Outlook, Qtopia)
        if ( addToLists ) d->syncPlugins.append(p);
        dta = new QDPluginData;
    } else {
        // Any other plugins
        dta = new QDPluginData;
    }
    d->pluginData[plugin] = dta;
    plugin->d = dta;
}

/*!
    Returns the plugins that match \a filter.
*/
QDPluginList PluginManager::plugins( QDPluginFilter *filter )
{
    if ( filter ) {
        QDPluginList list;
        foreach ( QDPlugin *plugin, d->appPlugins )
            if ( filter->filter( plugin ) )
                list << plugin;
        return list;
    } else {
        return d->allPlugins;
    }
}

/*!
    Returns the application plugins that match \a filter.
*/
QDAppPluginList PluginManager::appPlugins( QDAppPluginFilter *filter )
{
    if ( filter ) {
        QDAppPluginList list;
        foreach ( QDAppPlugin *plugin, d->appPlugins )
            if ( filter->filter( plugin ) )
                list << plugin;
        return list;
    } else {
        return d->appPlugins;
    }
}

/*!
    Returns the connection plugins.
*/
QDConPluginList PluginManager::conPlugins()
{
    return d->conPlugins;
}

/*!
    Returns the device plugins.
*/
QDDevPluginList PluginManager::devPlugins()
{
    return d->devPlugins;
}

/*!
    Returns the link plugins.
*/
QDLinkPluginList PluginManager::linkPlugins()
{
    return d->linkPlugins;
}

/*!
    Returns the sync plugins.
*/
QDSyncPluginList PluginManager::syncPlugins()
{
    return d->syncPlugins;
}

/*!
    Returns the plugin files that were detected on the system.
*/
QStringList PluginManager::detectedPluginFiles()
{
    return d->detectedPluginFiles;
}

/*!
    Returns the plugin identifiers that were detected on the system.
*/
QMap<QString,QString> PluginManager::detectedPluginIds()
{
    return d->detectedPluginIds;
}

// ==========================================================================

/*!
    \class QDPluginFilter
    \brief The QDPluginFilter class is used to filter the plugins returned by PluginManager::plugins().
*/

/*!
    Construct a QDPluginFilter.
*/
QDPluginFilter::QDPluginFilter()
    : pluginType(0)
{
}

/*!
    Destructor.
*/
QDPluginFilter::~QDPluginFilter()
{
}

/*!
    Filter \a plugin based on the criteria.
    Returns true if the plugin matches, false otherwise.
*/
bool QDPluginFilter::filter( QDPlugin *plugin )
{
    if ( (pluginType & QDPluginFilter::App) && qobject_cast<QDAppPlugin*>(plugin) == 0 )
        return false;
    if ( (pluginType & QDPluginFilter::Link) && qobject_cast<QDLinkPlugin*>(plugin) == 0 )
        return false;
    if ( (pluginType & QDPluginFilter::Con) && qobject_cast<QDConPlugin*>(plugin) == 0 ) 
        return false;
    if ( (pluginType & QDPluginFilter::Dev) && qobject_cast<QDDevPlugin*>(plugin) == 0 )
        return false;
    if ( (pluginType & QDPluginFilter::Sync) && qobject_cast<QDSyncPlugin*>(plugin) == 0 )
        return false;
    return true;
}

/*!
    \enum QDPluginFilter::PluginTypeFlags
    \value App
    \value Con
    \value Dev
    \value Link
    \value Sync
*/

/*!
    \enum QDPluginFilter::FilterOption
    \value Set
    \value NotSet
    \value Either
*/

/*
    \variable QDPluginFilter::pluginType;
*/

// ==========================================================================

/*!
    \class QDAppPluginFilter
    \brief The QDPluginFilter class is used to filter the plugins returned by PluginManager::appPlugins().
*/

/*!
    Construct a QDAppPluginFilter.
*/
QDAppPluginFilter::QDAppPluginFilter()
    : appWidget(Either), settingsWidget(Either)
{
    pluginType = QDPluginFilter::App;
}

/*!
    Destructor.
*/
QDAppPluginFilter::~QDAppPluginFilter()
{
}

/*!
    \reimp
*/
bool QDAppPluginFilter::filter( QDPlugin *plugin )
{
    if ( ! QDPluginFilter::filter(plugin) )
        return false;
    QDAppPluginData *data = qdPluginManager()->pluginData((QDAppPlugin*)plugin);
    if ( (appWidget == Set && data->appWidget == 0 ) ||
         (appWidget == NotSet && data->appWidget != 0 ) )
        return false;
    if ( (settingsWidget == Set && data->settingsWidget == 0 ) ||
         (settingsWidget == NotSet && data->settingsWidget != 0 ) )
        return false;
    return true;
}

