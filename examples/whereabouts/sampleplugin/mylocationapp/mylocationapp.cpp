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

#include "mylocationapp.h"

#include <QWhereabouts>
#include <QWhereaboutsFactory>

#include <QDebug>


MyLocationApp::MyLocationApp(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    QWhereabouts *whereabouts = QWhereaboutsFactory::create("locationplugin");
    if (!whereabouts) {
        qWarning("MyLocationApp: unable to load LocationPlugin! Have you installed the project located in examples/whereabouts/sampleplugin/locationplugin?");
        return;
    }

    connect(whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
            SLOT(updated(QWhereaboutsUpdate)));

    whereabouts->startUpdates();
}

void MyLocationApp::updated(const QWhereaboutsUpdate &update)
{
    // Respond to update here
}

