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

#include "nokiascreendriverplugin.h"
#include "nokiascreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

NokiaScreenDriverPlugin::NokiaScreenDriverPlugin( QObject *parent )
: QScreenDriverPlugin( parent )
{
}

NokiaScreenDriverPlugin::~NokiaScreenDriverPlugin()
{
}

QScreen* NokiaScreenDriverPlugin::create(const QString& key, int displayId)
{
    if (key.toLower() == "nokia")
        return new NokiaScreen(displayId);

    return 0;
}

QStringList NokiaScreenDriverPlugin::keys() const
{
    return QStringList() << "nokia";
}

QTOPIA_EXPORT_QT_PLUGIN(NokiaScreenDriverPlugin)

