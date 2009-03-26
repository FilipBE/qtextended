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

#include <qdplugin.h>
#include <qmap.h>

static QMap<QString,qdPluginCreateFunc_t> *qdInternalPlugins()
{
    static QMap<QString,qdPluginCreateFunc_t> *internalPlugins = new QMap<QString,qdPluginCreateFunc_t>;
    return internalPlugins;
}

void qd_registerPlugin_AUTOPLUGIN_TARGET(const QString &id, qdPluginCreateFunc_t createFunc)
{
    (*qdInternalPlugins())[id] = createFunc;
}

class PluginFactory_AUTOPLUGIN_TARGET : public QDPluginFactory
{
    QD_CONSTRUCT_PLUGIN(PluginFactory_AUTOPLUGIN_TARGET,QDPluginFactory)
public:
    QString executableName() const
    {
        return QTOPIA_TARGET;
    }
    QStringList keys() const
    {
        return qdInternalPlugins()->keys();
    }
    QDPlugin *create( const QString &key )
    {
        if ( qdInternalPlugins()->contains( key ) ) {
            qdPluginCreateFunc_t func = (*qdInternalPlugins())[key];
            QDPlugin *plugin = func( this );
            return plugin;
        }
        return 0;
    }
};

Q_EXPORT_PLUGIN(PluginFactory_AUTOPLUGIN_TARGET)
