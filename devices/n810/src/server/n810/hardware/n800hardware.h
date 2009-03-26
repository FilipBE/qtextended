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

#ifndef N800HARDWARE_H
#define N800HARDWARE_H

#ifdef QT_QWS_N810

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>

class QSocketNotifier;
class QBootSourceAccessoryProvider;
class QPowerSourceProvider;
class QDBusInterface;

class N800Hardware : public QObject
{
    Q_OBJECT

public:
    N800Hardware();
    ~N800Hardware();
    QDBusInterface *m_iface;
private:
    bool charging;
    int percent;
    int visualCharge;
    int chargeId;

    QValueSpaceObject vsoPortableHandsfree;

    QSocketNotifier *m_notifyDetect;
    int detectFd;

    QBootSourceAccessoryProvider *bootSource;

    QPowerSourceProvider *batterySource;
    QPowerSourceProvider *wallSource;
    void setLeds(int charge);

    QProcess *mountProc;
    QString sdCardDevice;
    int sdCardState;
    int sdSuccessState;
    int sdErrorState;
    int sdWaitForCount;
    enum cardState {
        CLOSEMOUNTED,
        CLOSEUNMOUNTED,
        CLOSEEMPTY,
        OPENMOUNTED,
        OPENUNMOUNTED,
        OPENEMPTY,
        INITMOUNTED,
        INITUNMOUNTED,
        INITEMPTY
    };
    void sdCardDeviceInitialisation();
    QString getMountPoint();
    void sdCardStateUpdate(bool);

private slots:
    void shutdownRequested();

    void chargingChanged(bool charging);
    void chargeChanged(int charge);
    void getCoverProperty(QString, QString);

    void getSDCoverState();
    void addSDCard();
    void removeSDCard();
    void coverOpen();
    void coverClose();
    void mountSDCard();
    void mountFinished(int, QProcess::ExitStatus);
    void unmountSDCard(bool);
    void unmountFinished(int, QProcess::ExitStatus);
};

#endif // QT_QWS_N800

#endif
