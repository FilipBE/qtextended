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
#include "pluginlist.h"

#include <qtopiaapplication.h>

#include <qpluginmanager.h>
#include <qtopialog.h>
#include <qdir.h>


#ifdef QT_NO_COMPONENT
// Plugins that are compiled in when no plugin architecture available.
#include "../../plugins/codecs/wavrecord/wavrecordimpl.h"

#endif // QT_NO_COMPONENT


MediaRecorderPluginList::MediaRecorderPluginList()
{
#ifndef QT_NO_COMPONENT
    loader = new QPluginManager( "codecs" );
    foreach( QString name, loader->list() ) {
        MediaRecorderCodecPlugin *plugin = 0;
        QObject *obj = loader->instance( name );

        if( ( plugin = qobject_cast<MediaRecorderCodecPlugin*>( obj ) ) != 0 ) {
            if( plugin->keys().contains( "MediaRecorderCodecPlugin" ) ) {
                addFormats( plugin );
            }
        }
    }
    if ( pluginList.count() == 0 )
        qWarning() << "No encoders found"; // No tr
#else
    // No component support - load the compiled-in encoders.
    loader = 0;
    MediaRecorderPluginInterface *iface = new WavRecorderPluginImpl();
    addFormats( iface );
#endif
}


MediaRecorderPluginList::~MediaRecorderPluginList()
{
#ifndef QT_NO_COMPONENT
    if ( loader ) {
        foreach( MediaRecorderPlugin codec, pluginList ) {
            if ( codec.plugin )
                delete codec.plugin;
        }
        pluginList.clear();
        delete loader;
        loader = 0;
    }
#else
    pluginList.clear();
#endif
}


MediaRecorderEncoder *MediaRecorderPluginList::fromType( const QString& type, const QString& tag )
{
    uint numPlugins;
    uint plugin;
    MediaRecorderEncoder *encoder;
    QString format;

    numPlugins = count();

    for ( plugin = 0; plugin < numPlugins; ++plugin ) {
        encoder = pluginList[plugin].encoder;
        format = pluginList[plugin].format;
        if ( type == encoder->pluginMimeType() && tag == format ) {
            return encoder;
        }
    }

    return 0;
}


int MediaRecorderPluginList::indexFromType( const QString& type, const QString& tag )
{
    uint numPlugins;
    uint plugin;
    MediaRecorderEncoder *encoder;
    QString format;

    numPlugins = count();

    for ( plugin = 0; plugin < numPlugins; ++plugin ) {
        encoder = pluginList[plugin].encoder;
        format = pluginList[plugin].format;
        if ( type == encoder->pluginMimeType() && tag == format ) {
            return (int)plugin;
        }
    }

    return -1;
}


void MediaRecorderPluginList::addFormats( MediaRecorderCodecPlugin *iface )
{
    MediaRecorderPlugin plugin;
    MediaRecorderEncoder *encoder;
    int nformats, format;

    encoder = iface->encoder();
    nformats = encoder->pluginNumFormats();

    for ( format = 0; format < nformats; ++format ) {

        plugin.plugin = iface;
        plugin.encoder = encoder;
        plugin.format = encoder->pluginFormatTag( format );
        plugin.formatName = encoder->pluginFormatName( format );
        pluginList.append( plugin );
        iface = 0;
    }
}
