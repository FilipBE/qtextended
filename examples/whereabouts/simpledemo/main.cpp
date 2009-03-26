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

#include <QWhereabouts>
#include <QWhereaboutsFactory>

#include <QtopiaApplication>

/*
    This is a simple example of how to use QWhereabouts.

    This example calls QWhereaboutsFactory::create() without any arguments,
    indicating that the default whereabouts plugin should be used for
    retrieving location data. Therefore, this example will only be able to
    get location data if the default plugin's data source has been set up
    correctly.

    For most Qtopia device configurations, the default whereabouts plugin
    is set to the built-in plugin, "gpsd", which receives location data from
    a GPSd daemon (see http://gpsd.berlios.de), so the daemon must be running
    in order to use this plugin. The default plugin can be set through the
    "Plugins/Default" value in \c etc/Settings/Trolltech/Whereabouts.conf.

    See the Location Services documentation and the QWhereaboutsFactory class
    documentation for more details.
*/


class SimpleLocationDemo : public QObject
{
    Q_OBJECT
public:
    SimpleLocationDemo(QObject *parent = 0)
        : QObject(parent)
    {
        QWhereabouts *whereabouts = QWhereaboutsFactory::create();
        connect(whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
                SLOT(updated(QWhereaboutsUpdate)));

        whereabouts->startUpdates();
    }

private slots:
    void updated(const QWhereaboutsUpdate &update)
    {
        // respond to update here
    }
};


int main(int argc, char **argv)
{
    QtopiaApplication app(argc, argv);

    SimpleLocationDemo demo;

    return app.exec();
}

#include "main.moc"
