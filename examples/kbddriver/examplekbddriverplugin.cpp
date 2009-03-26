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

#include "examplekbddriverplugin.h"
#include "examplekbdhandler.h"

#include <qtopiaglobal.h>

#include <qtopialog.h>

ExampleKbdDriverPlugin::ExampleKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

ExampleKbdDriverPlugin::~ExampleKbdDriverPlugin()
{
}

QWSKeyboardHandler* ExampleKbdDriverPlugin::create(const QString& driver,
                                                   const QString& device)
{
    if (driver.toLower() == "examplekbdhandler") {
        qLog(Input) << "Before call ExampleKbdHandler()";
        return new ExampleKbdHandler(device);
    }
    return 0;
}

QWSKeyboardHandler* ExampleKbdDriverPlugin::create(const QString& driver)
{
    if (driver.toLower() == "examplekbdhandler") {
        qLog(Input) << "Before call ExampleKbdHandler()";
        return new ExampleKbdHandler();
    }
    return 0;
}

QStringList ExampleKbdDriverPlugin::keys() const
{
    return QStringList() << "examplekbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(ExampleKbdDriverPlugin)
