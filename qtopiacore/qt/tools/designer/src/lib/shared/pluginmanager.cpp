/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "pluginmanager_p.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>
#include <QtCore/QPluginLoader>
#include <QtCore/QLibrary>
#include <QtCore/QLibraryInfo>
#include <QtCore/qdebug.h>
#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QCoreApplication>

QT_BEGIN_NAMESPACE

static QStringList unique(const QStringList &lst)
{
    const QSet<QString> s = QSet<QString>::fromList(lst);
    return s.toList();
}

QStringList QDesignerPluginManager::defaultPluginPaths()
{
    QStringList result;

    const QStringList path_list = QCoreApplication::libraryPaths();

    const QString designer = QLatin1String("designer");
    foreach (const QString &path, path_list) {
        QString libPath = path;
        libPath += QDir::separator();
        libPath += designer;
        result.append(libPath);
    }

    QString homeLibPath = QDir::homePath();
    homeLibPath += QDir::separator();
    homeLibPath += QLatin1String(".designer");
    homeLibPath += QDir::separator();
    homeLibPath += QLatin1String("plugins");

    result.append(homeLibPath);
    return result;
}

// ---------------- QDesignerPluginManagerPrivate

class QDesignerPluginManagerPrivate {
    public:
    QDesignerPluginManagerPrivate(QDesignerFormEditorInterface *core);

    QDesignerFormEditorInterface *m_core;
    QStringList m_pluginPaths;
    QStringList m_registeredPlugins;
    QStringList m_disabledPlugins;

    typedef QMap<QString, QString> FailedPluginMap;
    FailedPluginMap m_failedPlugins;

    typedef QList<QDesignerCustomWidgetInterface*> CustomWidgetList;
    CustomWidgetList m_customWidgets;

    QStringList defaultPluginPaths() const;
};

QDesignerPluginManagerPrivate::QDesignerPluginManagerPrivate(QDesignerFormEditorInterface *core) :
   m_core(core)
{
}


// ---------------- QDesignerPluginManager
// As of 4.4, the header will be distributed with the Eclipse plugin.

QDesignerPluginManager::QDesignerPluginManager(QDesignerFormEditorInterface *core) :
    QObject(core),
    m_d(new QDesignerPluginManagerPrivate(core))
{
    QSettings settings;

    settings.beginGroup(QLatin1String("PluginManager"));

    m_d->m_pluginPaths = defaultPluginPaths();
    m_d->m_disabledPlugins
        = unique(settings.value(QLatin1String("DisabledPlugins")).toStringList());
    updateRegisteredPlugins();

    settings.endGroup();
}

QDesignerPluginManager::~QDesignerPluginManager()
{
    syncSettings();
    delete m_d;
}

QDesignerFormEditorInterface *QDesignerPluginManager::core() const
{
    return m_d->m_core;
}

QStringList QDesignerPluginManager::findPlugins(const QString &path)
{
    const QDir dir(path);
    if (!dir.exists())
        return QStringList();

    const QFileInfoList infoList = dir.entryInfoList(QDir::Files);
    if (infoList.empty())
        return QStringList();

    // Load symbolic links but make sure all file names are unique as not
    // to fall for something like 'libplugin.so.1 -> libplugin.so'
    QStringList result;
    const QFileInfoList::const_iterator icend = infoList.constEnd();
    for (QFileInfoList::const_iterator it = infoList.constBegin(); it != icend; ++it) {
        QString fileName;
        if (it->isSymLink()) {
            const QFileInfo linkTarget = QFileInfo(it->symLinkTarget());
            if (linkTarget.exists() && linkTarget.isFile())
                fileName = linkTarget.absoluteFilePath();
        } else {
            fileName = it->absoluteFilePath();
        }
        if (!fileName.isEmpty() && QLibrary::isLibrary(fileName) && !result.contains(fileName))
            result += fileName;
    }
    return result;
}

void QDesignerPluginManager::setDisabledPlugins(const QStringList &disabled_plugins)
{
    m_d->m_disabledPlugins = disabled_plugins;
    updateRegisteredPlugins();
}

void QDesignerPluginManager::setPluginPaths(const QStringList &plugin_paths)
{
    m_d->m_pluginPaths = plugin_paths;
    updateRegisteredPlugins();
}

QStringList QDesignerPluginManager::disabledPlugins() const
{
    return m_d->m_disabledPlugins;
}

QStringList QDesignerPluginManager::failedPlugins() const
{
    return m_d->m_failedPlugins.keys();
}

QString QDesignerPluginManager::failureReason(const QString &pluginName) const
{
    return m_d->m_failedPlugins.value(pluginName);
}

QStringList QDesignerPluginManager::registeredPlugins() const
{
    return m_d->m_registeredPlugins;
}

QStringList QDesignerPluginManager::pluginPaths() const
{
    return m_d->m_pluginPaths;
}

QObject *QDesignerPluginManager::instance(const QString &plugin) const
{
    if (m_d->m_disabledPlugins.contains(plugin))
        return 0;

    QPluginLoader loader(plugin);
    return loader.instance();
}

void QDesignerPluginManager::updateRegisteredPlugins()
{
    m_d->m_registeredPlugins.clear();
    foreach (QString path,  m_d->m_pluginPaths)
        registerPath(path);
}

bool QDesignerPluginManager::registerNewPlugins()
{
    const int before = m_d->m_registeredPlugins.size();
    foreach (QString path,  m_d->m_pluginPaths)
        registerPath(path);
    const bool newPluginsFound = m_d->m_registeredPlugins.size() > before;
    if (newPluginsFound)
        ensureInitialized();
    return newPluginsFound;
}

void QDesignerPluginManager::registerPath(const QString &path)
{
    QStringList candidates = findPlugins(path);

    foreach (QString plugin, candidates)
        registerPlugin(plugin);
}

void QDesignerPluginManager::registerPlugin(const QString &plugin)
{
    if (m_d->m_disabledPlugins.contains(plugin))
        return;
    if (m_d->m_registeredPlugins.contains(plugin))
        return;

    QPluginLoader loader(plugin);
    if (loader.isLoaded() || loader.load()) {
        m_d->m_registeredPlugins += plugin;
        QDesignerPluginManagerPrivate::FailedPluginMap::iterator fit = m_d->m_failedPlugins.find(plugin);
        if (fit != m_d->m_failedPlugins.end())
            m_d->m_failedPlugins.erase(fit);
        return;
    }

    const QString errorMessage = loader.errorString();
    m_d->m_failedPlugins.insert(plugin, errorMessage);
}

bool QDesignerPluginManager::syncSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("PluginManager"));
    settings.setValue(QLatin1String("DisabledPlugins"), m_d->m_disabledPlugins);
    settings.endGroup();
    return settings.status() == QSettings::NoError;
}

void QDesignerPluginManager::ensureInitialized()
{
    m_d->m_customWidgets.clear();
    // Complete list of plugins, static and dynamic
    QObjectList pluginObjects = QPluginLoader::staticInstances();

    const QStringList plugins = registeredPlugins();
    foreach (const QString &plugin, plugins)
        if (QObject *o = instance(plugin))
            pluginObjects.push_back(o);

    foreach(QObject *o, pluginObjects) {
        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            m_d->m_customWidgets.append(c);
        } else if (QDesignerCustomWidgetCollectionInterface *coll = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(o)) {
            m_d->m_customWidgets += coll->customWidgets();
        }
    }

    foreach (QDesignerCustomWidgetInterface *c, m_d->m_customWidgets) {
        if (!c->isInitialized()) {
            c->initialize(core());
        }
    }
}

QList<QDesignerCustomWidgetInterface*> QDesignerPluginManager::registeredCustomWidgets() const
{
    const_cast<QDesignerPluginManager*>(this)->ensureInitialized();
    return m_d->m_customWidgets;
}

QList<QObject*> QDesignerPluginManager::instances() const
{
    QStringList plugins = registeredPlugins();

    QList<QObject*> lst;
    foreach (QString plugin, plugins) {
        if (QObject *o = instance(plugin))
            lst.append(o);
    }

    return lst;
}

QT_END_NAMESPACE
