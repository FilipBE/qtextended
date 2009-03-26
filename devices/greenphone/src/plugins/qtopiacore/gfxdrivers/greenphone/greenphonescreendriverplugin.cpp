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

#include "greenphonescreendriverplugin.h"
#include "greenphonescreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

GreenphoneScreenDriverPlugin::GreenphoneScreenDriverPlugin( QObject *parent )
: QScreenDriverPlugin( parent )
{
}

GreenphoneScreenDriverPlugin::~GreenphoneScreenDriverPlugin()
{
}

QScreen* GreenphoneScreenDriverPlugin::create(const QString& key, int displayId)
{
    if (key.toLower() == "greenphone") 
        return new GreenphoneScreen(displayId);
    
    return 0;
}

QStringList GreenphoneScreenDriverPlugin::keys() const
{
    return QStringList() << "greenphone";
}

QTOPIA_EXPORT_QT_PLUGIN(GreenphoneScreenDriverPlugin)

