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

#ifdef QT_QWS_GREENPHONE

#include "greenphonehardware.h"

#include "../include/omega_sysdevs.h"
#include "../include/omega_chgled.h"
#define CONFIG_ARCH_OMEGA
#include "../include/soundcard.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>
#include <QFile>
#include <QtGlobal>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>
#include <qpowersource.h>

#include <qtopiaserverapplication.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

QTOPIA_TASK(GreenphoneHardware, GreenphoneHardware);

GreenphoneHardware::GreenphoneHardware()
    : vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
      usbGadgetObject("/Hardware/UsbGadget"),
      usbDataCable(true),
      mountProc(NULL)
{
    //StandardDialogs::disableShutdownDialog();

    detectFd = ::open("/dev/omega_detect", O_RDONLY|O_NDELAY, 0);
    if (detectFd >= 0) {
        qLog(Hardware) << "Opened omega_detect as detect input";
        m_notifyDetect = new QSocketNotifier(detectFd, QSocketNotifier::Read, this);
        connect(m_notifyDetect, SIGNAL(activated(int)), this, SLOT(readDetectData()));
    } else {
        qWarning("Cannot open omega_detect for detect (%s)", strerror(errno));
    }

    //QObject::connect(QtopiaServerApplication::instance(), SIGNAL(shutdownRequested()), this, SLOT(shutdownRequested()));

    bootSource = new QBootSourceAccessoryProvider( "greenphone", this );

    batterySource = new QPowerSourceProvider(QPowerSource::Battery, "GreenphoneBattery", this);
    batterySource->setAvailability(QPowerSource::Available);
    connect(batterySource, SIGNAL(chargingChanged(bool)),
            this, SLOT(chargingChanged(bool)));
    connect(batterySource, SIGNAL(chargeChanged(int)),
            this, SLOT(chargeChanged(int)));

    wallSource = new QPowerSourceProvider(QPowerSource::Wall, "GreenphoneCharger", this);

    readDetectData(1 << CHARGER_DETECT |
                   1 << BOOTSRC_DETECT |
                   1 << LOWPOWER_DETECT);

    QTimer::singleShot(30*1000, this, SLOT(delayedRead()));
}

GreenphoneHardware::~GreenphoneHardware()
{
    if (detectFd >= 0) {
        ::close(detectFd);
        detectFd = -1;
    }
}

void GreenphoneHardware::mountSD()
{
    QFile partitions("/proc/partitions");
    if (!partitions.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    sdCardDevice.clear();

    QList<QByteArray> lines = partitions.readAll().split('\n');
    for (int i = 2; i < lines.count(); i++) {
        QStringList fields = QString(lines.at(i)).split(' ', QString::SkipEmptyParts);

        if (fields.count() == 4) {
            if (sdCardDevice.isEmpty() && fields.at(3) == "mmca") {
                sdCardDevice = "/dev/" + fields.at(3);
            } else if (fields.at(3).startsWith("mmca")) {
                sdCardDevice = "/dev/" + fields.at(3);
                break;
            }
        }
    }

    if (sdCardDevice.isEmpty())
        return;

    if (!mountProc)
        mountProc = new QProcess(this);

    if (mountProc->state() != QProcess::NotRunning)
        return;

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                       SLOT(fsckFinished(int,QProcess::ExitStatus)));

    QStringList arguments;
    arguments << "-a" << sdCardDevice;

    qLog(Hardware) << "Checking filesystem on" << sdCardDevice;
    mountProc->start("fsck", arguments);
}

void GreenphoneHardware::unmountSD()
{
    if (!mountProc)
        mountProc = new QProcess(this);

    if (mountProc->state() != QProcess::NotRunning) {
        qLog(Hardware) << "Previous (u)mount command failed to finished";
        mountProc->kill();
    }

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                       SLOT(mountFinished(int,QProcess::ExitStatus)));

    qLog(Hardware) << "Unmounting /media/sdcard";
    mountProc->start("umount -l /media/sdcard");
}

void GreenphoneHardware::fsckFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0)
        qLog(Hardware) << "Filesystem errors detected on" << sdCardDevice;

    disconnect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
               this, SLOT(fsckFinished(int,QProcess::ExitStatus)));

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                       SLOT(mountFinished(int,QProcess::ExitStatus)));

    QStringList arguments;
    arguments << sdCardDevice << "/media/sdcard";

    qLog(Hardware) << "Mounting" << sdCardDevice << "on /media/sdcard";
    mountProc->start("mount", arguments);
}

void GreenphoneHardware::mountFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0)
        qLog(Hardware) << "Failed to (u)mount" << sdCardDevice << "on /media/sdcard";

    mountProc->deleteLater();
    mountProc = NULL;

    QtopiaIpcEnvelope msg("QPE/Card", "mtabChanged()");
}

void GreenphoneHardware::chargingChanged(bool charging)
{
    if (charging)
        setLeds(101);
    else
        setLeds(batterySource->charge());
}

void GreenphoneHardware::chargeChanged(int charge)
{
    if (!batterySource->charging())
        setLeds(charge);
}

void GreenphoneHardware::setLeds(int charge)
{
    int ledFd = ::open("/dev/omega_chgled", O_RDWR);
    if (ledFd < 0) {
        perror("Failed to open /dev/omega_chgled");
        return;
    }

    if (charge == 101) {
        // red on
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 7);
    } else if (charge > 75) {
        // red flashing slow
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 7);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (charge > 50) {
        // red flashing slow
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 5);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (charge > 25) {
        // red flashing fast
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 3);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (charge > 0) {
        // red flashing fast
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 1);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (charge == 0) {
        // red flashing very fast
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 0);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 4);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else {
        // all off
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 0);
    }

    if (ledFd >= 0)
        ::close(ledFd);
}

void GreenphoneHardware::shutdownRequested()
{
    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

void GreenphoneHardware::delayedRead()
{
    readDetectData(1 << SDCARD_DETECT |
                   1 << AVHEADSET_DETECT);
}

void GreenphoneHardware::readDetectData(quint32 devices)
{
    union {
        quint32 changedDevices;
        detect_device_t detectData[26];
    };

    if (::read(detectFd, detectData, 104) < 104) {
        qWarning() << "Couldn't read from device omega_detect";
        return;
    }

    if (devices == 0)
        devices = changedDevices;

    quint32 mask = 0x00000001;
    int position = 0;
    while (mask) {
        if (devices & mask) {
            detect_device_t &device = detectData[position];

            switch (device.dev_id) {
            case CHARGER_DETECT:
                if (device.status == DEV_ON) {
                    wallSource->setAvailability(QPowerSource::Available);
                    batterySource->setCharging(true);
                    if (usbDataCable)
                        usbGadgetObject.setAttribute("cableConnected", true);
                    qLog(Hardware) << "Charger cable plugged in";
                } else if (device.status == DEV_OFF) {
                    wallSource->setAvailability(QPowerSource::NotAvailable);
                    batterySource->setCharging(false);
                    usbGadgetObject.setAttribute("cableConnected", false);
                    qLog(Hardware) << "Charger cable unplugged";
                } else {
                    wallSource->setAvailability(QPowerSource::Failed);
                    batterySource->setCharging(false);
                    usbGadgetObject.setAttribute("cableConnected", false);
                    qLog(Hardware) << "Unknown charger cable event";
                }
                break;
            case AVHEADSET_DETECT:
                if (device.status == HEADSET_ON) {
                    qLog(Hardware) << "Headset plugged in";
                    vsoPortableHandsfree.setAttribute("Present", true);
                } else if (device.status == DEV_OFF) {
                    qLog(Hardware) << "Headset unplugged";
                    vsoPortableHandsfree.setAttribute("Present", false);
                } else {
                    qLog(Hardware) << "Unknown AVHEADSET";
                }
                break;
            case SDCARD_DETECT:
                if (device.status == DEV_ON)
                    // Need to delay to give the kernel time scan the sd card and
                    // publish partitions in /proc/partitions before we can mount it.
                    QTimer::singleShot(1000, this, SLOT(mountSD()));
                else if (device.status == DEV_OFF)
                    unmountSD();
                else
                    qLog(Hardware) << "Unknown SD card event";
                break;
            case USBIN_DETECT:
                if (device.status == DEV_ON) {
                    usbDataCable = true;
                    qLog(Hardware) << "USB cable plugged in";
                } else if (device.status == DEV_OFF) {
                    usbDataCable = false;
                    qLog(Hardware) << "USB cable unplugged";
                } else {
                    qLog(Hardware) << "Unknown USB cable event";
                }
                break;
            case FLIPSENSOR3_DETECT:
                qLog(Hardware) << "Unknown FLIPSENSOR3";
                break;
            case FLIPSENSOR4_DETECT:
                qLog(Hardware) << "Unknown FLIPSENSOR4";
                break;
            case FLIPSENSOR1_DETECT:
                qLog(Hardware) << "Unknown FLIPSENSOR1";
                break;
            case MODEM_DETECT:
                qLog(Hardware) << "Unknown MODEM";
                break;
            case HANDFREE_DETECT:
                qLog(Hardware) << "Unknown HANDFREE";
                break;
            case LCDSYNC_DETECT:
                qLog(Hardware) << "Unknown LCDSYNC";
                break;
            case LCD_DETECT:
                qLog(Hardware) << "Unknown LCD";
                break;
            case CALLING_DETECT:
                qLog(Hardware) << "Unknown CALLING";
                break;
            case CAMERA_DETECT:
                qLog(Hardware) << "Unknown CAMERA";
                break;
            case BOOTSRC_DETECT:
                if (device.status == OMEGABOOT_NORM) {
                    qLog(Hardware) << "Boot source normal";
                    bootSource->setBootSource(QBootSourceAccessory::PowerKey);
                } else if (device.status == OMEGABOOT_CHG) {
                    qLog(Hardware) << "Boot source charger";
                    bootSource->setBootSource(QBootSourceAccessory::Charger);
                } else if (device.status == OMEGABOOT_ALRM) {
                    qLog(Hardware) << "Boot source alarm";
                    bootSource->setBootSource(QBootSourceAccessory::Alarm);
                } else if (device.status == OMEGABOOT_WDR) {
                    qLog(Hardware) << "Boot source watchdog";
                    bootSource->setBootSource(QBootSourceAccessory::Watchdog);
                } else if (device.status == OMEGABOOT_REBOOT) {
                    qLog(Hardware) << "Boot source reboot";
                    bootSource->setBootSource(QBootSourceAccessory::Software);
                } else {
                    qLog(Hardware) << "Boot source unknown";
                    bootSource->setBootSource(QBootSourceAccessory::Unknown);
                }
                break;
            case TV_DETECT:
                qLog(Hardware) << "Unknown TV";
                break;
            case PMMODE_DETECT:
                if (device.status == DEV_ON)
                    qLog(Hardware) << "Power button held";
                else if (device.status == DEV_OFF)
                    qLog(Hardware) << "Power button released?";
                else
                    qLog(Hardware) << "Unknown power buttom event";
                break;
            case LOWPOWER_DETECT:
            {
                batterySource->setCharge(25 * qBound<quint16>(0, device.extra, 4));

                if (device.status == NORMAL_POWER) {
                    qLog(Hardware) << "Normal power - level" << device.extra;
                } else if (device.status == LOW_POWER) {
                    qLog(Hardware) << "Low power - level" << device.extra;
                } else if (device.status == TOOLOW_POWER) {
                    qLog(Hardware) << "Too low power - level" << device.extra;
                    if (detectData[CHARGER_DETECT].status == DEV_OFF) {
                        QFile splashHeading("/tmp/splash-heading");
                        if (splashHeading.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            splashHeading.write("Low Battery");
                            splashHeading.close();
                        }
                        shutdownRequested();
                    }
                } else {
                    qLog(Hardware) << "Unknown low power event";
                }
                break;
            }
            case SYSTEM_SLEEP_DETECT:
                qLog(Hardware) << "Unknown SYSTEM_SLEEP";
                break;
            case SYSTEM_WAKEUP_DETECT:
                qLog(Hardware) << "Unknown SYSTEM_WAKEUP";
                break;
            case LCDBL_DETECT:
                qLog(Hardware) << "Unknown LCDBL";
                break;
            case CHGLED_DETECT:
                qLog(Hardware) << "Unknown CHGLED";
                break;
            case SYSLED_DETECT:
                qLog(Hardware) << "Unknown SYSLED";
                break;
            case KPBL_DETECT:
                qLog(Hardware) << "Unknown KPBL";
                break;
            case VIBRATOR_DETECT:
                qLog(Hardware) << "Unknown VIBRATOR";
                break;
            }
        }

        mask <<= 1;
        position++;
    }
}

#endif // QT_QWS_GREENPHONE
