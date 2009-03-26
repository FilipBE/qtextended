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

#include <QtopiaApplication>
#include <QDebug>
#include "dbmigrateservice.h"
#include <private/qcontent_p.h>

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,dbmigrate)
#define MAIN_FUNC main_dbmigrate
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY
int MAIN_FUNC( int argc, char** argv )
{
    QSXE_SET_APP_KEY(argv[0]);

    int i;

    QStringList args;
    for(i=1; i<argc; i++)
        args.append(argv[i]);
    qLog(DocAPI) << "dbmigrate called with parameters..." << args;
    if(args.contains("--systemupgrade"))
    {
        if(args.contains("--qws"))
        {
            qWarning() << "\"dbmigrate --systemupgrade\" will not work with the --qws command line option\n"
                       << " as it is designed to be run in this mode as a standalone application";
            return -1;
        }
        args.clear();
        QApplication app(argc, argv, false);
        app.setApplicationName( QLatin1String( "dbmigrate" ) );
        if(app.arguments().count() > 2)
            for(i=1; i<app.arguments().count(); i++)
                if(app.arguments().at(i) != "--systemupgrade")
                    args.append(app.arguments().at(i));
        if(MigrationEngineService::doMigrate(args))
            return 0;
        else
            return -1;
    }
    else
    {
        QtopiaApplication app(argc, argv);
        MigrationEngineService service(NULL);
        return app.exec();
    }
}
