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

#ifndef QVPNCLIENTPRIVATE_P_H
#define QVPNCLIENTPRIVATE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qvpnclient.h"

#include <QProcess>
#include <QString>
#include <qvaluespace.h>

#include <qtopianamespace.h>

class QVPNClientPrivate {
public:
    QVPNClientPrivate()
        : vpnProcess( 0 ), vSpace( 0 ), vItem( 0 )
    {
        serverMode = false;
    }

    ~QVPNClientPrivate() {
        if ( vpnProcess ) {
            delete vpnProcess;
            vpnProcess = 0;
        }
        if ( vSpace )  {
            delete vSpace;
            vSpace = 0;
        }
        if ( vItem ) {
            delete vItem;
            vItem = 0;
        }
    }

    /*!
      Created by non-server mode object.
      */
    void createVPNConfig(const QString& baseName)
    {
        if ( serverMode )
            return;

        const QString path = Qtopia::applicationFileName( "Network", "vpn" );
        QDir settingsDir(path);
        QString filename;
        int n = 0;
        do {
            filename = settingsDir.filePath( baseName +
                    QString::number(n++)+ QLatin1String(".conf") );
        } while ( QFile::exists( filename ) );

        config = filename;
    }

    QProcess* vpnProcess;
    QString config;
    bool serverMode;
    QString errorString;
    QValueSpaceObject* vSpace;
    QValueSpaceItem* vItem;
};

#endif
