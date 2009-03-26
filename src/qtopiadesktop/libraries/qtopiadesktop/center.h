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
#ifndef CENTER_H
#define CENTER_H

#include <qdplugindefs.h>
#include <qdglobal.h>

// This is the interface presented to the plugins
class QD_EXPORT CenterInterface
{
public:
#ifdef Q_OS_WIN32
    // This is needed for DLL wierdness
    CenterInterface();
#endif
#ifdef Q_OS_UNIX
    virtual ~CenterInterface();
#endif

    virtual QDDevPlugin *currentDevice() = 0;
    virtual const QDLinkPluginList linkPlugins() = 0;
    virtual const QDDevPluginList devicePlugins() = 0;
    virtual QDPlugin *getPlugin( const QString &id ) = 0;
    virtual QObject *syncObject() = 0;
};

#endif
