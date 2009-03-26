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

#include "locationplugin.h"

#include <QWhereabouts>
#include <QTimer>


LocationProvider::LocationProvider(QObject *parent)
    : QWhereabouts(QWhereabouts::TerminalBasedUpdate, parent),
      m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), SLOT(requestUpdate()));
}

void LocationProvider::requestUpdate()
{
    QWhereaboutsUpdate update;
    update.setCoordinate(QWhereaboutsCoordinate(0.0, 0.0));
    update.setUpdateDateTime(QDateTime::currentDateTime());
    emitUpdated(update);
}

void LocationProvider::startUpdates()
{
    if (updateInterval() > 0)
        m_timer->start(updateInterval());
    else
        m_timer->start(1000);
}

void LocationProvider::stopUpdates()
{
    m_timer->stop();
}


//=========================================================

LocationPlugin::LocationPlugin(QObject *parent)
    : QWhereaboutsPlugin(parent)
{
}

QWhereabouts *LocationPlugin::create(const QString &source)
{
    Q_UNUSED(source);
    return new LocationProvider;
}


QTOPIA_EXPORT_PLUGIN(LocationPlugin)
