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

#include <phonesim/server.h>
#ifndef PHONESIM_TARGET
    #include "control.h"
    #include <qapplication.h>
#else
    #include <qcoreapplication.h>
#endif
#include <qstring.h>
#include <qdebug.h>
#include <stdlib.h>

static void usage()
{
    qWarning() << "Usage:"
               << QFileInfo(QCoreApplication::instance()->applicationFilePath()).fileName().toLocal8Bit().constData()
               << "[-p port] [-gui] filename";
    exit(-1);
}

int main(int argc, char **argv)
{
#ifndef PHONESIM_TARGET
    QApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif
    QString filename;
    int port = 12345;
    int index;
    bool with_gui = false;

    // Parse the command-line.
    index = 1;
    for (index = 1; index < argc; index++) {
        if (strcmp(argv[index],"-p") == 0) {
            index++;
            if (index >= argc) {
                qWarning() << "ERROR: Got -p but missing port number";
                usage();
            } else {
                port = atoi(argv[index]);
            }
        } else if (strcmp(argv[index],"-gui") == 0) {
            // turn on gui option
            with_gui = true;
        } else if ( strcmp(argv[index],"-h") == 0
                || strcmp(argv[index],"-help") == 0 ) {
            usage();
        } else {
            // must be filename.  SHOULD be last argument.
            if (index != argc-1) {
                qWarning() << "ERROR: filename must be the last argument";
                usage();
            }
            filename = argv[index];
        }

    }

    PhoneSimServer *pss = new PhoneSimServer(filename, port, 0);

#ifndef PHONESIM_TARGET
    if (with_gui)
        pss->setHardwareManipulator(new ControlFactory);
#else
    Q_UNUSED(pss);
#endif

    return app.exec();
}

