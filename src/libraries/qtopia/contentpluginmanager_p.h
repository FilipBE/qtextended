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

#ifndef CONTENTPLUGINMANAGER_P_H
#define CONTENTPLUGINMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qcontentplugin.h>
#include <QMultiHash>
#include <QList>
#include <QSettings>
#include <qpluginmanager.h>
#include <QMutex>


class DotDesktopContentPlugin : public QContentPlugin
{
public:
    DotDesktopContentPlugin();
    virtual ~DotDesktopContentPlugin();

    virtual QStringList keys() const;

    virtual bool installContent( const QString &filePath, QContent *content );
    virtual bool updateContent( QContent *content );
};


class ContentPluginManager
{
public:
    ContentPluginManager();
    ~ContentPluginManager();

    QList< QContentPlugin * > findPlugins( const QString &type );
    QList< QContentPropertiesPlugin * > findPropertiesPlugins( const QString &type );

    void loadPlugins();
private:

    QPluginManager *manager;

    QMultiHash< QString, QContentPlugin * > typePluginMap;
    QMultiHash< QString, QContentPropertiesPlugin* > typePropertiesPluginMap;

    QMutex loadMutex;

    DotDesktopContentPlugin dotDesktopPlugin;
};

class QContentFactory
{
public:
    static void loadPlugins();
    static bool installContent( const QString &fileName, QContent *content );
    static bool updateContent( QContent *content );
    static QImage thumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode );
    static QContentPropertiesEngine *createPropertiesEngine( const QContent &content );
};

#endif
