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

#include "n800kbddriverplugin.h"
#include "n800kbdhandler.h"

#include <qtopiaglobal.h>

N800KbdDriverPlugin::N800KbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

N800KbdDriverPlugin::~N800KbdDriverPlugin()
{
}

QWSKeyboardHandler* N800KbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("N800KbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* N800KbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "n800kbdhandler" ) {
        qWarning("Before call N800KbdHandler()");
        return new N800KbdHandler();
    }
    return 0;
}

QStringList N800KbdDriverPlugin::keys() const
{
    return QStringList() << "n800kbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(N800KbdDriverPlugin)
