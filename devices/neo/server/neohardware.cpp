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

#ifdef QT_QWS_NEO

#include "neohardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>
#include <QtopiaIpcAdaptor>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>

#include <qtopiaserverapplication.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input.h>

#include <sys/ioctl.h>

QTOPIA_TASK(NeoHardware, NeoHardware);

NeoHardware::NeoHardware()
    : vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
      vsoUsbCable("/Hardware/UsbGadget"),
      vsoNeoHardware("/Hardware/Neo")
{
    adaptor = new QtopiaIpcAdaptor("QPE/NeoHardware");

    qLog(Hardware) << "neohardware";

    cableConnected(getCableStatus());

    vsoPortableHandsfree.setAttribute("Present", false);
    vsoPortableHandsfree.sync();

// Handle Audio State Changes
    audioMgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);


    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(headphonesInserted(bool)),
                              this, SLOT(headphonesInserted(bool)));

    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(cableConnected(bool)),
                              this, SLOT(cableConnected(bool)));
    findHardwareVersion();
}

NeoHardware::~NeoHardware()
{
}

void NeoHardware::findHardwareVersion()
{
    QFile cpuinfo( "/proc/cpuinfo");
    QString inStr;
    cpuinfo.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&cpuinfo);
    QString line;
    do {
        line  = in.readLine();
        if (line.contains("Hardware") ){
            QStringList token = line.split(":");
            inStr = token.at(1).simplified();
        }
    } while (!line.isNull());

    cpuinfo.close();
    qLog(Hardware)<<"Neo"<< inStr;

    vsoNeoHardware.setAttribute("Device", inStr);
    vsoNeoHardware.sync();
}

void NeoHardware::headphonesInserted(bool b)
{
    qLog(Hardware)<< __PRETTY_FUNCTION__ << b;
    vsoPortableHandsfree.setAttribute("Present", b);
    vsoPortableHandsfree.sync();
    if (b) {
        QByteArray mode("Headphone");
        audioMgr->send("setProfile(QByteArray)", mode);
    } else {
        QByteArray mode("MediaSpeaker");
        audioMgr->send("setProfile(QByteArray)", mode);
    }


}

void NeoHardware::cableConnected(bool b)
{
    qLog(Hardware)<< __PRETTY_FUNCTION__ << b;
    vsoUsbCable.setAttribute("cableConnected", b);
    vsoUsbCable.sync();
}

void NeoHardware::shutdownRequested()
{
    qLog(PowerManagement)<< __PRETTY_FUNCTION__;

    QFile powerFile;
    QFile btPower;

    if ( QFileInfo("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on").exists()) {
//neo
        powerFile.setFileName("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on");
        btPower.setFileName("/sys/bus/platform/devices/gta01-pm-bt.0/power_on");
    } else {
//ficgta02
        powerFile.setFileName("/sys/bus/platform/devices/neo1973-pm-gsm.0/power_on");
        btPower.setFileName("/sys/bus/platform/devices/neo1973-pm-bt.0/power_on");
    }

    if( !powerFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerFile);
        out << "0";
        powerFile.close();
    }

        if( !btPower.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&btPower);
        out <<  "0";
        powerFile.close();
    }


    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

bool NeoHardware::getCableStatus()
{
    // since the Neo's do not let us know if we startup with usb connected
    // any other way..
    qLog(PowerManagement) << __PRETTY_FUNCTION__;
    QString chgState;
    if (QFileInfo("/sys/devices/platform/s3c2440-i2c/i2c-adapter/i2c-0/0-0073/chgstate").exists()) {
         //freerunner
        chgState = "/sys/devices/platform/s3c2440-i2c/i2c-adapter/i2c-0/0-0073/chgstate";
    } else {
        //1973
        chgState = "/sys/devices/platform/s3c2410-i2c/i2c-adapter/i2c-0/0-0008/chgstate";
    }
    QString inStr;

    QFile chgstate( chgState);
    chgstate.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&chgstate);
    in >> inStr;

    chgstate.close();
    qLog(PowerManagement) << inStr;

    if (inStr.contains("enabled")) {
        return true;
    } else {
        return false;
    }
}

#endif // QT_QWS_NEO

