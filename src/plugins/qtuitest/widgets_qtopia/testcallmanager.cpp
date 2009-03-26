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

#include "testcallmanager.h"

#include <Qtopia>

bool TestCallManager::canSelect(const QString& item) const
{ return item == "accept" || item == "hangup"; }

bool TestCallManager::select(const QString& item)
{
    using namespace QtUiTest;

    if (!canSelect(item)) return false;

    if (!Qtopia::mousePreferred()) {
        if ("accept" == item) {
            keyClick( Qt::Key_Call );
        } else if ("hangup" == item) {
            keyClick( Qt::Key_Hangup );
        }
        return true;
    }

    // On touchscreen-preferred,
    // "accept call" is optionsMenu()->Answer,
    // "hangup" is optionsMenu()->Send Busy OR optionsMenu()->End
    QStringList selection;
    if ("accept" == item) {
        selection << "Answer";
    } else if ("hangup" == item) {
        selection << "Send Busy" << "End";
    }

    QObject* optionsMenu = findWidget(OptionsMenu);
    SelectWidget* sw = qtuitest_cast<SelectWidget*>(optionsMenu);
    if (!sw) {
        setErrorString("Could not find the options menu while selecting '" + item
                + "' from call manager.");
        return false;
    }
    foreach (QString const& s, selection) {
        if (sw->canSelect(s))
            return sw->select(s);
    }
    QString sel = (selection.count() > 1) ? QString("one of ('%1')").arg(selection.join("','")) : QString("'%1'").arg(selection.at(0));
    QString error("Expected options menu to contain " + sel + ", but it didn't.");
    if (ListWidget* lw = qtuitest_cast<ListWidget*>(optionsMenu)) {
        error += "\nAvailable items: " + lw->list().join(",");
    }
    setErrorString(error);
    return false;
}

