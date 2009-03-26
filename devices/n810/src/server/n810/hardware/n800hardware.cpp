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

#ifdef QT_QWS_N810

#include "n800hardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QFileSystem>
#include <QtGlobal>
#include <QStringList>

#include <qtopiaserverapplication.h>
#include <qtopialog.h>
#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>
#include <qpowersource.h>

#include <QtopiaServiceRequest>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <qdbusconnection.h>
#include <qdbusinterface.h>

QTOPIA_TASK(N800Hardware, N800Hardware);

N800Hardware::N800Hardware()
    : charging(false)
    , percent(-1)
    , chargeId(0)
    , vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree")
    , mountProc(0)
    , sdCardDevice("/dev/mmcblk1p1")
    , sdCardState(INITEMPTY)
    , sdSuccessState(INITEMPTY)
    , sdErrorState(INITEMPTY)
    , sdWaitForCount(0)
{
    bootSource = new QBootSourceAccessoryProvider( "N800", this );

    batterySource = new QPowerSourceProvider(QPowerSource::Battery, "N800Battery", this);
    batterySource->setAvailability(QPowerSource::Available);

    connect(batterySource, SIGNAL(chargingChanged(bool)),
            this, SLOT(chargingChanged(bool)));
    connect(batterySource, SIGNAL(chargeChanged(int)),
            this, SLOT(chargeChanged(int)));

    wallSource = new QPowerSourceProvider(QPowerSource::Wall, "N800Charger", this);

    QDBusConnection dbc = QDBusConnection::systemBus();

    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }

    dbc.connect(QString(), "/org/freedesktop/Hal/devices/platform_slide",
                "org.freedesktop.Hal.Device", "Condition",
                this, SIGNAL(getCoverProperty(QString,QString)));

    getCoverProperty(QString(), "cover");

    sdCardDeviceInitialisation();

    dbc.connect(QString(), "/org/freedesktop/Hal/devices/platform_mmci_omap_1_mmc_host",
                "org.freedesktop.Hal.Device", "Condition",
                this, SIGNAL(getSDCoverState()));

    dbc.connect(QString(), "/org/kernel/class/mmc_host/mmc0/mmc0_b368",
                "org.kernel.kevent", "add",
                this, SIGNAL(addSDCard()));

    dbc.connect(QString(), "/org/kernel/class/mmc_host/mmc0/mmc0_b368",
                "org.kernel.kevent", "remove",
                this, SIGNAL(removeSDCard()));
}

N800Hardware::~N800Hardware()
{
    if (detectFd >= 0) {
        ::close(detectFd);
        detectFd = -1;
    }
}

void N800Hardware::chargingChanged(bool charging)
{
    if (charging)
        setLeds(101);
    else
        setLeds(batterySource->charge());
}

void N800Hardware::chargeChanged(int charge)
{
    if (!batterySource->charging())
        setLeds(charge);
}

void N800Hardware::setLeds(int charge)
{
    Q_UNUSED(charge);
}

void N800Hardware::shutdownRequested()
{
    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

void N800Hardware::getCoverProperty(QString /*str*/,  QString str2)
{
    if (str2 == "cover") {
        QString slideState;

        QFile slideStateFile("/sys/devices/platform/gpio-switch/slide/state");
        slideStateFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream in(&slideStateFile);
        in >> slideState;
        slideStateFile.close();

        if ( slideState == "closed") {
            QtopiaServiceRequest svreq("RotationManager", "setCurrentRotation(int)");
            svreq << 270;
            svreq.send();
        } else {
            // cover open rotate 0
            QtopiaServiceRequest svreq("RotationManager", "defaultRotation()");
            svreq.send();
        }
    }
}

void N800Hardware::sdCardDeviceInitialisation()
{
    // Check if device is already mounted, if so return the mount point path and preform connection to database
    QString mountPoint = getMountPoint();

    if (mountPoint == "UNMOUNTED") {
        qLog(Hardware) << "SD device not initially mounted.";
        if (QFile::exists(sdCardDevice)) {
            sdCardState = INITUNMOUNTED;
        } else {
            sdCardState = INITEMPTY;
        }
    } else if (mountPoint != "ERROR") {
        QtopiaIpcEnvelope msg("QPE/QStorage", "mounting(QString)");
        msg << sdCardDevice;
        qLog(Hardware) << "SD device was initially mounted. Sent message to connect to database.";
        sdCardState = INITMOUNTED;
    }
    // allow getSDCoverState() to take care of any other state - being exact action(s) will be determined by SD cover state
    getSDCoverState();
}

// Take the one cover state slot and split it into two seperate slots.
void N800Hardware::getSDCoverState()
{
    QString sdCoverState;

    QFile sdCoverStateFile("/sys/class/mmc_host/mmc0/cover_switch");
    sdCoverStateFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&sdCoverStateFile);
    in >> sdCoverState;
    sdCoverStateFile.close();

    if (sdCoverState == "open") {
        emit coverOpen();
    } else if (sdCoverState == "closed") {
        emit coverClose();
    } else {
        qLog(Hardware) << "Unknown state for sdCardState: " << sdCardState;
    }
}

void N800Hardware::coverOpen()
{
    if (sdCardState == CLOSEMOUNTED || sdCardState == INITMOUNTED) {
        sdCardState = OPENMOUNTED;  // this is a valid state though very transient
        sdSuccessState = OPENUNMOUNTED;
        sdErrorState = OPENMOUNTED;
        unmountSDCard(false);
    } else if (sdCardState == INITUNMOUNTED || sdCardState == CLOSEUNMOUNTED) {
        sdCardState = OPENUNMOUNTED;
    } else if (sdCardState == CLOSEEMPTY || sdCardState == INITEMPTY) {
        sdCardState = OPENEMPTY;
    }
}

void N800Hardware::coverClose()
{
    if (sdCardState == OPENUNMOUNTED || sdCardState == INITUNMOUNTED) {
        sdSuccessState = CLOSEMOUNTED;
        sdErrorState = CLOSEUNMOUNTED;
        mountSDCard();
    } else if (sdCardState == OPENEMPTY || sdCardState == INITEMPTY) {
        sdCardState = CLOSEEMPTY;
    } else if (sdCardState == INITMOUNTED || sdCardState == OPENMOUNTED) {  // initalisation has issued connect to DB msg so only statechange
        sdCardState = CLOSEMOUNTED;
    }
}

void N800Hardware::addSDCard()
{
    // do both in case the change of state to CLOSEEMPTY doesn't occur before we evaluate
    // in order to the get the add signal, the door has to close anyway
    if (sdCardState == CLOSEEMPTY || sdCardState == OPENEMPTY) {
        sdSuccessState = CLOSEMOUNTED;
        sdErrorState = CLOSEUNMOUNTED;
        mountSDCard();
    }
}

void N800Hardware::removeSDCard()
{
    if (sdCardState == OPENUNMOUNTED) {
        sdCardState = OPENEMPTY;
    } else if (sdCardState == OPENMOUNTED) {  // should not get here but it could occur
        sdSuccessState = OPENEMPTY;
        sdErrorState = OPENEMPTY;               // card removed - empty+open no matter what
        unmountSDCard(true);
    }
}

void N800Hardware::sdCardStateUpdate(bool success)
{
    sdCardState = success ? sdSuccessState : sdErrorState;
}

QString N800Hardware::getMountPoint()
{
    QFile mounts("/proc/mounts");
    if (!mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qLog(Hardware) << "Unable to open /proc/mounts.";
        return QString("ERROR");
    }

    QList<QByteArray> lines = mounts.readAll().split('\n');
    for (int loopCount = 1; loopCount < lines.count(); loopCount++) {
        QStringList fields = QString(lines.at(loopCount-1)).split(' ', QString::SkipEmptyParts);
        if (fields.count() >= 2 && fields.at(0) == sdCardDevice) {
            return fields.at(1);
        }
    }
    return QString("UNMOUNTED");
}

void N800Hardware::mountSDCard()
{
    // in some conditions the device is yet to exist in /dev/... when we get to this point
    // use a oneshot to call ourselves and do a count so we don't loop forever
    if (sdWaitForCount >= 100) {
        qLog(Hardware) << "Timeout waiting for existance of" << sdCardDevice;
        sdCardStateUpdate(false);
        sdWaitForCount = 0;
        return;
    } else if (!QFile::exists(sdCardDevice)) {
        sdWaitForCount++;
        QTimer::singleShot(50, this, SLOT(mountSDCard()));
        return;
    }
    sdWaitForCount = 0;

    if (!mountProc)
        mountProc = new QProcess(this);

    if (mountProc->state() != QProcess::NotRunning) {
        qLog(Hardware) << "Previous (u)mount command failed to finish.";
        mountProc->kill();
    }

    QStringList arguments;
    arguments << sdCardDevice;

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                        SLOT(mountFinished(int,QProcess::ExitStatus)));

    qLog(Hardware) << "Mounting:" << sdCardDevice;
    mountProc->start("mount", arguments);
}

void N800Hardware::mountFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0) {
        qLog(Hardware) << "Failed to mount" << sdCardDevice;
        sdCardStateUpdate(false);
    } else if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        qLog(Hardware) << "Success:" << sdCardDevice << "Mounted.";
        QtopiaIpcEnvelope msg("QPE/QStorage", "mounting(QString)");
        msg << sdCardDevice;
        sdCardStateUpdate(true);
    } else {
        sdCardStateUpdate(false);
        qLog(Hardware) << "Unknown Error Mounting - State Undetermined for:" << sdCardDevice;
    }

    mountProc->deleteLater();
    mountProc = 0;
}

void N800Hardware::unmountSDCard(bool forceUnmount)
{
    {
        QtopiaIpcEnvelope msg("QPE/QStorage", "unmounting(QString)");
        msg << sdCardDevice;
    }

    QStringList arguments;
    // if true force the unmount, otherwise lazy unmount
    if (forceUnmount) {
        arguments << "-f";
        qLog(Hardware) << "Force argument passed to unmount.";
    } else {
        arguments << "-l";
    }

    if (!mountProc)
        mountProc = new QProcess(this);

    if (mountProc->state() != QProcess::NotRunning) {
        qLog(Hardware) << "Previous (u)mount command failed to finished";
        mountProc->kill();
    }

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                        SLOT(unmountFinished(int,QProcess::ExitStatus)));

    arguments << sdCardDevice;
    qLog(Hardware) << "Un-mounting using arguments:" << arguments;
    mountProc->start("umount", arguments);
}

void N800Hardware::unmountFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0) {
        qLog(Hardware) << "Failed to un-mount" << sdCardDevice;
        sdCardStateUpdate(false);
    } else if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        qLog(Hardware) << "Success:" << sdCardDevice << "Unmounted.";
        sdCardStateUpdate(true);
    } else {
        sdCardStateUpdate(false);
        qLog(Hardware) << "Unknown Error Unmounting - State Undetermined for:" << sdCardDevice;
    }

    mountProc->deleteLater();
    mountProc = 0;
}

#endif  // QT_QWS_N810
