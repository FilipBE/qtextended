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

#include "perftestlinuxfbscreen.h"

#ifndef QT_NO_QWS_LINUXFB

#include <QScreenDriverFactory>
#include <QWSServer>
#include <qtopialog.h>
#include <QDebug>


PerftestLinuxFbScreen::PerftestLinuxFbScreen(int display_id) : QLinuxFbScreen(display_id)
{
    qLog(UI) << "Created Perftest LINUXFB screen driver";
}

PerftestLinuxFbScreen::~PerftestLinuxFbScreen()
{
}

bool PerftestLinuxFbScreen::connect(const QString &displaySpec)
{
    QString spec(displaySpec);
    if (spec.startsWith("perftestlinuxfb:"))
        spec = spec.mid(16);

    qLog(UI) << "Attempting to connect with spec" << ((spec.isEmpty()) ? "(none)" : spec);

    return QLinuxFbScreen::connect(spec);
}

void PerftestLinuxFbScreen::exposeRegion(QRegion region, int windowIndex)
{
    if (qLogEnabled(UI)) {
        QWSWindow *changed = 0;
        if (windowIndex >= 0 && QWSServer::instance() && QWSServer::instance()->clientWindows().count() > windowIndex)
            changed = QWSServer::instance()->clientWindows().at(windowIndex);
        if(changed && !changed->client()->identity().isEmpty()) {
            QRect r = region.boundingRect();
            qLog(UI)
                << changed->client()->identity()
                << ": expose_region"
                << QString("QRect(%1,%2 %3x%4)").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height());
        }
    }

    return QLinuxFbScreen::exposeRegion(region, windowIndex);
}

#endif
