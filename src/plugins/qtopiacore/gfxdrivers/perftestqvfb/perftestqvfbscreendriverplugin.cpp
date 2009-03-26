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

#include "perftestqvfbscreendriverplugin.h"
#include "perftestqvfbscreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

PerftestQVFbScreenDriverPlugin::PerftestQVFbScreenDriverPlugin( QObject *parent )
    : QScreenDriverPlugin( parent )
{}

PerftestQVFbScreenDriverPlugin::~PerftestQVFbScreenDriverPlugin()
{}

QScreen* PerftestQVFbScreenDriverPlugin::create(const QString& key, int displayId)
{
#ifndef QT_NO_QWS_QVFB
    if (key.toLower() == "perftestqvfb") {
        qLog(Input) << "Creating PerftestScreen()";
        return new PerftestQVFbScreen(displayId);
    }
#endif
    return 0;
}

QStringList PerftestQVFbScreenDriverPlugin::keys() const
{
    return QStringList()
#ifndef QT_NO_QWS_QVFB
        << "perftestqvfb"
#endif
    ;
}

QTOPIA_EXPORT_QT_PLUGIN(PerftestQVFbScreenDriverPlugin)
