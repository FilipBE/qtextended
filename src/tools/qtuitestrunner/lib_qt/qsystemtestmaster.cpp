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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qsystemtestmaster_p.h"

#include <qsystemtest.h>
#include "qsystemtest_p.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QIODevice>

// **************************************************************************************
// **************************************************************************************

QSystemTestMaster::QSystemTestMaster( QSystemTestPrivate *testCase )
    : QTestProtocol()
    , app_name()
    , test_case(testCase)
{
}

QSystemTestMaster::~QSystemTestMaster()
{
}

void QSystemTestMaster::queryName()
{
    QTestMessage reply;
    sendMessage(QTestMessage("appName"), reply, 40000);
    app_name = reply["appName"].toString();
}

void QSystemTestMaster::processMessage( QTestMessage *msg )
{
    if (msg->event() == "APP_NAME") {
        app_name = (*msg)["appName"].toString();
    } else {
        // Leave all processing to the testcase
        if (test_case != 0) {
            test_case->p->processMessage( *msg );
        }
    }
}

QString QSystemTestMaster::appName()
{
    if (app_name.isEmpty() ||
        app_name.indexOf("quicklauncher") >= 0 ||
        app_name.indexOf("ERR") >= 0 ) {
        queryName();
    }
    return app_name;
}

void QSystemTestMaster::onReplyReceived( QTestMessage * /*reply*/ )
{
}

