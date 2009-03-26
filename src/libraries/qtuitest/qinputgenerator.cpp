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

#include "qinputgenerator_p.h"

#include <QApplication>
#include <QPoint>
#include <QWidget>

/*
    Given a co-ordinate relative to the application's currently active window,
    maps it to this input generator's co-ordinate system.
*/
QPoint QInputGenerator::mapFromActiveWindow(QPoint const& pos) const
{
    QPoint ret(pos);

    /*
        Qt Extended is treated as one big always-active always-fullscreen window.
        We only need to do the translation on other platforms.
    */
#ifndef QTOPIA_TARGET
    QWidget *w = QApplication::activeWindow();
    if (!w) w = QApplication::activePopupWidget();
    if (!w) w = QApplication::activeModalWidget();

    if (w) {
       ret = w->mapToGlobal(pos);
    }
#endif

    return ret;
}

