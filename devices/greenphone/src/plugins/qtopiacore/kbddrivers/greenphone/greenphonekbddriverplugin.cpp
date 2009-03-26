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

#include "greenphonekbddriverplugin.h"
#include "greenphonekbdhandler.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

GreenphoneKbdDriverPlugin::GreenphoneKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

GreenphoneKbdDriverPlugin::~GreenphoneKbdDriverPlugin()
{}

QWSKeyboardHandler* GreenphoneKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qLog(Input) << "GreenphoneKbdDriverPlugin:create()";
    return create( driver );
}

QWSKeyboardHandler* GreenphoneKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "greenphonekbdhandler" ) {
        qLog(Input) << "Before call GreenphoneKbdHandler()";
        return new GreenphoneKbdHandler();
    }
    return 0;
}

QStringList GreenphoneKbdDriverPlugin::keys() const
{
    return QStringList() << "greenphonekbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(GreenphoneKbdDriverPlugin)
