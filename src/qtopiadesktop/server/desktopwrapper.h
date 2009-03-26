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
#ifndef DESKTOPWRAPPER_H
#define DESKTOPWRAPPER_H

#include <qdplugindefs.h>
#include <center.h>

#include <QObject>

class QtopiaDesktopApplication;

class DesktopWrapper : public QObject, public CenterInterface
{
public:
    DesktopWrapper( QDPlugin *plugin );
    ~DesktopWrapper();

    QDDevPlugin *currentDevice();

    const QDLinkPluginList linkPlugins();
    const QDDevPluginList devicePlugins();
    QDPlugin *getPlugin( const QString &id );
    QObject *syncObject();

private:
    QDPlugin *plugin;
    QtopiaDesktopApplication *q;
};


#endif
