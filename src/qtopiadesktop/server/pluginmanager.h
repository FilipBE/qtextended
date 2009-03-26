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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <qdplugindefs.h>

#include <QObject>
#include <QFlags>
#include <QString>
#include <QMap>

class PluginManagerPrivate;
class QDPluginFilter;
class QDAppPluginFilter;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    PluginManager( QObject *parent = 0 );
    ~PluginManager();

    void setupPlugins( bool safeMode );
    void setupPlugin( QDPlugin *plugin );

    QDAppPluginList activePlugins();
    QDAppPlugin *currentPlugin();
    QDPluginData *pluginData( QDPlugin *plugin );
    QDAppPluginData *pluginData( QDAppPlugin *plugin );

    QDPluginList plugins( QDPluginFilter *filter = 0 );
    QDAppPluginList appPlugins( QDAppPluginFilter *filter = 0 );
    QDConPluginList conPlugins();
    QDDevPluginList devPlugins();
    QDLinkPluginList linkPlugins();
    QDSyncPluginList syncPlugins();

    QStringList detectedPluginFiles();
    QMap<QString,QString> detectedPluginIds();

private:
    void internal_setupPlugin( QDPlugin *plugin, bool safeMode, bool addToLists );

    PluginManagerPrivate *d;
};

extern PluginManager *qdPluginManager();


class QDPluginFilter
{
public:
    QDPluginFilter();
    virtual ~QDPluginFilter();

    enum PluginTypeFlags {
        App  = 0x01,
        Link = 0x02,
        Con  = 0x04,
        Dev  = 0x08,
        Sync = 0x10,
    };
    Q_DECLARE_FLAGS(PluginType,PluginTypeFlags)
    enum FilterOption { Either, Set, NotSet };

    virtual bool filter( QDPlugin *plugin );

    PluginType pluginType;
};

class QDAppPluginFilter : public QDPluginFilter
{
public:
    QDAppPluginFilter();
    virtual ~QDAppPluginFilter();

    virtual bool filter( QDPlugin *plugin );

    FilterOption appWidget;
    FilterOption settingsWidget;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDPluginFilter::PluginType)

#endif
