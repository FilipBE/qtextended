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

#include "n810gpsplugin.h"

#include <QFile>
#include <QSettings>
#include <QSocketNotifier>
#include <QDebug>

#include <QWhereabouts>
#include <QNmeaWhereabouts>
#include <QMessageBox>
#include <qtopialog.h>
#include <QLocalSocket>
#include <QtopiaApplication>



#define NMEA_GPS_DEVICE "/dev/pgps"
/*
 This plugin only works for n810
*/
N810GpsPlugin::N810GpsPlugin(QObject *parent)
    : QWhereaboutsPlugin(parent)
{
    qLog(Hardware) << __PRETTY_FUNCTION__;
    if ( ! enableGps())
        exit(1);
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
}

N810GpsPlugin::~N810GpsPlugin()
{
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
    disableGps();
}

QWhereabouts *N810GpsPlugin::create(const QString &source)
{
    qLog(Hardware) << __PRETTY_FUNCTION__;
    QString path = source;
    if (path.isEmpty()) {
#ifdef NMEA_GPS_DEVICE
        path = NMEA_GPS_DEVICE;
#endif
        QSettings cfg("Trolltech",  "Whereabouts");
        cfg.beginGroup("Hardware");
        path = cfg.value("Device", path).toString();
    }

    QFile *sourceFile = new QFile(path, this);
    if (!sourceFile->open(QIODevice::ReadOnly)) {
        QMessageBox::warning( 0,tr("Mappingdemo"),tr("Cannot open GPS device at %1").arg(path),
            QMessageBox::Ok,  QMessageBox::Ok);
        qWarning() << "Cannot open GPS device at" << path;
        delete sourceFile;
        return 0;
    }

    QNmeaWhereabouts *whereabouts = new QNmeaWhereabouts(QNmeaWhereabouts::RealTimeMode, this);
    whereabouts->setSourceDevice(sourceFile);

    // QFile does not emit readyRead(), so we must call
    // newDataAvailable() when necessary
    QSocketNotifier *notifier = new QSocketNotifier(sourceFile->handle(), QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)),
              whereabouts, SLOT(newDataAvailable()));

    return whereabouts;
}

bool N810GpsPlugin::sendToGpsDriverCtrl(QString cmd)
{
    QLocalSocket *gpsCtrlSocket;
    gpsCtrlSocket = new QLocalSocket(this);

    QByteArray socketPath = ( "/var/lib/gps/gps_driver_ctrl");
    gpsCtrlSocket->connectToServer(socketPath.data());

    if(gpsCtrlSocket->waitForConnected()) {
        qLog(Hardware)<< __PRETTY_FUNCTION__ << "connected" << cmd;

        QByteArray data;
        data.append(cmd);
        data.append("\n");
        gpsCtrlSocket->write(data);


        gpsCtrlSocket->flush();
        gpsCtrlSocket->disconnectFromServer();
        return true;
    } else {
        qLog(Hardware) << __PRETTY_FUNCTION__ << "Could not connect to socket" << gpsCtrlSocket;
    }
    return false;
}

bool N810GpsPlugin::enableGps()
{
    return sendToGpsDriverCtrl("P 3");
}

bool  N810GpsPlugin::disableGps()
{
    return sendToGpsDriverCtrl("P 0");
}

QTOPIA_EXPORT_PLUGIN(N810GpsPlugin)
