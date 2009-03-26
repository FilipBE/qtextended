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

#ifndef GREENPHONEHARDWARE_H
#define GREENPHONEHARDWARE_H

#ifdef QT_QWS_GREENPHONE

#include <QObject>
#include <QProcess>

#include <QValueSpaceObject>

class QSocketNotifier;
class QBootSourceAccessoryProvider;
class QPowerSourceProvider;

class GreenphoneHardware : public QObject
{
    Q_OBJECT

public:
    GreenphoneHardware();
    ~GreenphoneHardware();

private:
    QValueSpaceObject vsoPortableHandsfree;
    QValueSpaceObject usbGadgetObject;

    QSocketNotifier *m_notifyDetect;
    int detectFd;
    bool usbDataCable;

    QProcess *mountProc;
    QString sdCardDevice;

    QBootSourceAccessoryProvider *bootSource;

    QPowerSourceProvider *batterySource;
    QPowerSourceProvider *wallSource;

    void setLeds(int charge);

private slots:
    void readDetectData(quint32 devices = 0);

    void delayedRead();

    void shutdownRequested();

    void mountSD();
    void unmountSD();
    void fsckFinished(int, QProcess::ExitStatus);
    void mountFinished(int, QProcess::ExitStatus);

    void chargingChanged(bool charging);
    void chargeChanged(int charge);
};

#endif // QT_QWS_GREENPHONE

#endif
