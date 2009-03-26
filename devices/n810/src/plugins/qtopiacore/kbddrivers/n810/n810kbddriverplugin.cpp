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

#include "n810kbddriverplugin.h"
#include "n810kbdhandler.h"

#include <qtopiaglobal.h>

N810KbdDriverPlugin::N810KbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

N810KbdDriverPlugin::~N810KbdDriverPlugin()
{
}

QWSKeyboardHandler* N810KbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("N810KbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* N810KbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "n810kbdhandler" ) {
        qWarning("Before call N810KbdHandler()");
        return new N810KbdHandler();
    }
    return 0;
}

QStringList N810KbdDriverPlugin::keys() const
{
    return QStringList() << "n810kbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(N810KbdDriverPlugin)
