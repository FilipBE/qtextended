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

#include "messageviewer.h"
#include <qtopiaapplication.h>

// Comment out this line to use a manual main() function.
// Ensure you also remove CONFIG+=qtopia_main from qbuild.pro if you do this.
#define USE_THE_MAIN_MACROS



#ifdef USE_THE_MAIN_MACROS

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, MessageViewer)
QTOPIA_MAIN

#else

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, exampleapp)
#define MAIN_FUNC main_exampleapp
#else
#define MAIN_FUNC main
#endif

// This is the storage for the SXE key that uniquely identified this applicaiton.
// make will fail without this!
QSXE_APP_KEY

int MAIN_FUNC( int argc, char **argv )
{
    // This is required to load the SXE key into memory
    QSXE_SET_APP_KEY(argv[0]);

    QtopiaApplication a( argc, argv );

    // Set the preferred document system connection type
    QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION();

    MessageViewer *mw = new MessageViewer();
    a.setMainWidget(mw);
    if ( mw->metaObject()->indexOfSlot("setDocument(QString)") != -1 ) {
        a.showMainDocumentWidget();
    } else {
        a.showMainWidget();
    }
    int rv = a.exec();
    delete mw;
    return rv;
}

#endif

