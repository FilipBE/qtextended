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
#include "qtopiadesktopapplication.h"

#include <qtopiadesktoplog.h>

#include <QStringList>

int main( int argc, char **argv )
{
    QtopiaDesktopApplication instance( argc, argv );
    if ( instance.isRunning() ) {
        qDebug() << "Qtopia Sync Agent is already running";
        if ( instance.sendMessage( instance.arguments().join("!!!") ) ) {
            return 0;
        } else {
            qDebug() << "Could not communicate with the previous instance";
            return 1;
        }
    }

    instance.initialize();
    int ret = instance.exec();
    return ret;
}

