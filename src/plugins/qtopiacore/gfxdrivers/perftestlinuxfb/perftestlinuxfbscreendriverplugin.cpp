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

#include "perftestlinuxfbscreendriverplugin.h"
#include "perftestlinuxfbscreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

PerftestLinuxFbScreenDriverPlugin::PerftestLinuxFbScreenDriverPlugin( QObject *parent )
    : QScreenDriverPlugin( parent )
{}

PerftestLinuxFbScreenDriverPlugin::~PerftestLinuxFbScreenDriverPlugin()
{}

QScreen* PerftestLinuxFbScreenDriverPlugin::create(const QString& key, int displayId)
{
#ifndef QT_NO_QWS_LINUXFB
    if (key.toLower() == "perftestlinuxfb") {
        qLog(Input) << "Creating PerftestScreen()";
        return new PerftestLinuxFbScreen(displayId);
    }
#endif
    return 0;
}

QStringList PerftestLinuxFbScreenDriverPlugin::keys() const
{
    return QStringList()
#ifndef QT_NO_QWS_LINUXFB
        << "perftestlinuxfb"
#endif
    ;
}

QTOPIA_EXPORT_QT_PLUGIN(PerftestLinuxFbScreenDriverPlugin)
