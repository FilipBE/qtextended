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

#include <QThemeItemFactory>
#include <QThemePageItem>
#include <QThemeTextItem>
#include <QThemeExclusiveItem>
#include <QThemeImageItem>
#include <QThemeStatusItem>
#include <QThemeLevelItem>
#include <QThemeLayoutItem>
#include <QThemeRectItem>
#include <QThemeGroupItem>
#include <QThemeWidgetItem>
#include <QThemeTemplateItem>
#include <QThemeListItem>
#include <QThemePluginItem>
#ifndef THEME_EDITOR
#include <QPluginManager>
#else
#include "qthemefakepluginitem.h"
#endif
/*!
  \class QThemeItemPlugin
    \inpublicgroup QtBaseModule
  \since 4.4
  \ingroup qtopiatheming
  \ingroup plugins

  \brief The QThemeItemPlugin class is a simple plugin interface that makes it easy to create custom theme items that can be loaded dynamically using the QThemeItemFactory class.

  Writing a theme item plugin is achieved by subclassing this base class, reimplementing the pure virtual keys() and create() functions, and exporting the class using the QTOPIA_EXPORT_PLUGIN() macro.

  \sa QThemeItem, QThemeItemFactory
*/

/*!
  \fn QStringList QThemeItemPlugin::keys() const
  Returns the list of theme items this plugin supports.
  These keys are usually the class names of the custom items that are implemented in the plugin.
*/

/*!
  \fn QThemeItem* QThemeItemPlugin::create(const QString& type, QThemeItem* parent)
  Creates and returns a QThemeItem object for the given \a type with parent \a parent.
  If a plugin cannot create a item, it should return 0 instead.
*/

/*!
  \class QThemeItemFactory
    \inpublicgroup QtBaseModule
  \since 4.4
  \ingroup qtopiatheming

  \brief The QThemeItemFactory class creates QThemeItem objects based on the name of the item.

  To create a new type of theme item that can be created via the QThemeItemFactory, implement a theme item that derives from QThemeItem and a plug-in that derives from QThemeItemPlugin.

  \sa QThemedView
*/

#ifndef THEME_EDITOR
QPluginManager *QThemeItemFactory::pluginManager = 0;
QList<QThemeItemPlugin *> QThemeItemFactory::builtins = QList<QThemeItemPlugin *>();
#endif

/*!
  Creates and returns a theme item of the given \a type with parent \a parent.

  If the factory cannot create an item, it returns 0.
*/
QThemeItem *QThemeItemFactory::create(const QString &type, QThemeItem *parent)
{
    if (type == "text") {
        return new QThemeTextItem(parent);
    } else if (type == "page") {
        return new QThemePageItem(parent);
    } else if (type == "exclusive") {
        return new QThemeExclusiveItem(parent);
    } else if (type == "image") {
        return new QThemeImageItem(parent);
    } else if (type == "status") {
        return new QThemeStatusItem(parent);
    } else if (type == "level") {
        return new QThemeLevelItem(parent);
    } else if (type == "layout") {
        return new QThemeLayoutItem(parent);
    } else if (type == "rect") {
        return new QThemeRectItem(parent);
    } else if (type == "group") {
        return new QThemeGroupItem(parent);
    } else if (type == "widget") {
        return new QThemeWidgetItem(parent);
    } else if (type == "template") {
        return new QThemeTemplateItem(parent);
    } else if (type == "list") {
        return new QThemeListItem(parent);
    } else if (type == "plugin") {
        return new QThemePluginItem(parent);
    } else {
#ifndef THEME_EDITOR
        QThemeItem *item = createBuiltin(type, parent);
        if (item)
            return item;
        return createPlugin(type, parent);
#else
        QThemeFakePluginItem *fakeItem = new QThemeFakePluginItem(type, parent);
        return fakeItem;
#endif
    }
}

#ifndef THEME_EDITOR
/*!
  Registers a QThemeItemPlugin \a plugin as a built-in.
*/
void QThemeItemFactory::registerBuiltin(QThemeItemPlugin *plugin)
{
    builtins << plugin;
}

/*!
  \internal
*/
QThemeItem *QThemeItemFactory::createBuiltin(const QString &type, QThemeItem *parent)
{
    foreach (QThemeItemPlugin *plugin, builtins) {
        foreach (QString s, plugin->keys()) {
            if (s == type)
                return plugin->create(type, parent);
        }
    }
    return 0;
}

/*!
  \internal
*/
QThemeItem *QThemeItemFactory::createPlugin(const QString &type, QThemeItem *parent)
{
    if (!pluginManager)
        pluginManager = new QPluginManager("themeitems");

    QStringList availablePlugins = pluginManager->list();
    foreach (QString plugin, availablePlugins) {
        QThemeItemPlugin *themePlugin = qobject_cast<QThemeItemPlugin *>(pluginManager->instance(plugin));
        if (!themePlugin)
            return 0;
        foreach (QString s, themePlugin->keys()) {
            if (s == type)
                return themePlugin->create(type, parent);
        }
    }
    return 0;
}
#endif

/*!
  Releases all resources allocated by the plugin manager
*/
void QThemeItemFactory::clear()
{
#ifndef THEME_EDITOR
    if (pluginManager)
        pluginManager->clear();
#endif
}

