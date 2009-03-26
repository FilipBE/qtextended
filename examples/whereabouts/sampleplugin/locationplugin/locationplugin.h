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

#ifndef LOCATIONPLUGIN_H
#define LOCATIONPLUGIN_H

#include <QWhereaboutsPlugin>
#include <QWhereabouts>

class QTimer;


class LocationProvider : public QWhereabouts
{
    Q_OBJECT
public:
    LocationProvider(QObject *parent = 0);

    void requestUpdate();
    void startUpdates();
    void stopUpdates();

private:
    QTimer *m_timer;
};


class QTOPIA_PLUGIN_EXPORT LocationPlugin : public QWhereaboutsPlugin
{
    Q_OBJECT
public:
    LocationPlugin(QObject *parent = 0);

    QWhereabouts *create(const QString &source);
};


#endif
