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
#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <qlist.h>
#include <qlibrary.h>
#include <mediarecorderplugininterface.h>


class QPluginManager;


class MediaRecorderPlugin
{
public:
    MediaRecorderCodecPlugin *plugin;
    MediaRecorderEncoder *encoder;
    QString format;
    QString formatName;
};


class MediaRecorderPluginList
{
public:
    MediaRecorderPluginList();
    ~MediaRecorderPluginList();

    // Walk the encoder list.
    uint count() const { return pluginList.count(); }
    MediaRecorderEncoder *at( uint n ) const { return pluginList[n].encoder; }
    QString formatAt( uint n ) const { return pluginList[n].format; }
    QString formatNameAt( uint n ) const { return pluginList[n].formatName; }

    // Get the plugin with a specific MIME type and format tag.
    MediaRecorderEncoder *fromType( const QString& type, const QString& tag );

    // Get the index of a plugin with a specific MIME type and format tag.
    int indexFromType( const QString& type, const QString& tag );

private:
    QList<MediaRecorderPlugin> pluginList;
    QPluginManager *loader;

    void addFormats( MediaRecorderCodecPlugin *iface );

};

#endif

