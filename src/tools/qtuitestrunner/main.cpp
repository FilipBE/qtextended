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

#include "qscriptsystemtest.h"
#include <gracefulquit.h>
#include <QApplication>

// Check if we need to update qtestlib
#include "QtTest/qtest_global.h"
#ifdef FOOBAR
#if QTEST_PATCH < 0x1
# error "--------------------------------------------------------"
# error "   One or more patches are available for QTestLib       "
# error "           and it needs to be updated!                  "
# error "To patch:                                               "
# error "  - Run $QTOPIA_DEPOT_DIR/scripts/patch_qtestlib        "
# error "    from the root of the $QTOPIA_BUILD_DIR.             "
# error "  - Then run 'qbuild && qbuild iamge' in:                         "
# error "    $QTOPIA_BUILD_DIR/src/libraries/qt/qtestlib and     "
# error "    $QTOPIA_BUILD_DIR/src/libraries/qtopiacore/qtestlib "
# error "--------------------------------------------------------"
#endif
#endif

int main(int argc, char *argv[])
{
    bool guiEnabled = true;

    // If in auto mode, don't need a GUI connection.
    // This allows us to run system tests without an X server running.
    for (int i = 1; i < argc; ++i) {
        if (!strcasecmp(argv[i], "-auto")) {
            guiEnabled = false;
        }
    }

    QApplication app(argc, argv, guiEnabled);
    GracefulQuit::install(&app);
    QScriptSystemTest tc;
    return tc.exec(argc, argv);
}
