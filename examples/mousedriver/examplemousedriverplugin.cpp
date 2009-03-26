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

#include "examplemousedriverplugin.h"
#include "examplemousehandler.h"

#include <qtopiaglobal.h>

#include <qtopialog.h>

ExampleMouseDriverPlugin::ExampleMouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{
}

ExampleMouseDriverPlugin::~ExampleMouseDriverPlugin()
{
}

QWSMouseHandler* ExampleMouseDriverPlugin::create(const QString& driver, const QString& device)
{
    if ( driver.toLower() == "examplemousehandler" ) {
        qLog(Input) << "Before call ExampleMouseHandler()";
        return new ExampleMouseHandler(device);
    }
    return 0;
}

QWSMouseHandler* ExampleMouseDriverPlugin::create(const QString& driver)
{
    if( driver.toLower() == "examplemousehandler" ) {
        qLog(Input) << "Before call ExampleMouseHandler()";
        return new ExampleMouseHandler();
    }
    return 0;
}

QStringList ExampleMouseDriverPlugin::keys() const
{
    return QStringList() << "examplemousehandler";
}

QTOPIA_EXPORT_QT_PLUGIN(ExampleMouseDriverPlugin)
