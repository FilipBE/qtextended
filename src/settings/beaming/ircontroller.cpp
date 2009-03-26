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

#include "ircontroller.h"

#include <qtranslatablesettings.h>
#include <qtopiaapplication.h>

#include <qirlocaldevice.h>

#include <QFile>
#include <QDir>

void IRController::powerStateChanged(QCommDeviceController::PowerState state)
{
    switch (state) {
        case QCommDeviceController::On:
            m_st = IRController::On;
            break;
        case QCommDeviceController::Off:
            m_st = IRController::Off;
            break;
        case QCommDeviceController::OnOneItem:
            m_st = IRController::On1Item;
            break;
        case QCommDeviceController::OnTimed:
            m_st = IRController::On5Mins;
            break;
    }

    emit stateChanged(m_st);
}

IRController::IRController(QObject* parent) : QObject(parent), m_device(0), m_st(IRController::Off)
{
    // Load protocols
    QString tdir = Qtopia::qtopiaDir()+"/etc/beam/targets";
    QDir dir(tdir,"*.conf");
    protocount=0;
    QSettings cfgout("Trolltech","Beam");
    cfgout.beginGroup("Send");
    QString curdev=cfgout.value("DeviceConfig").toString();
    curproto = 0;
    for (int i=0; i<(int)dir.count(); i++) {
        QString t=tdir+"/"+dir[i];
        QTranslatableSettings cfg(t, QSettings::IniFormat);
        if ( cfg.status()==QSettings::NoError ) {
            cfg.beginGroup("Device");
            names.append(cfg.value("Name").toString());
            icons.append(cfg.value("Icon").toString());
            targets.append(t);
            if ( curdev.isEmpty() )
                curdev=t;
            if ( curdev == t )
                curproto = protocount;
            protocount++;
        }
    }

    QStringList devices = QIrLocalDevice::devices();

    if (devices.size() > 0) {
        m_device = new QCommDeviceController(devices[0].toLatin1());
        connect(m_device, SIGNAL(powerStateChanged(QCommDeviceController::PowerState)),
                SLOT(powerStateChanged(QCommDeviceController::PowerState)));
        powerStateChanged(m_device->powerState());
    }
}

IRController::State IRController::state() const
{
    return m_st;
}

int IRController::currentProtocol() const
{
    return curproto;
}

int IRController::protocolCount() const
{
    return protocount;
}

IRController::~IRController()
{
    if (m_device)
        delete m_device;
}

QString IRController::stateDescription(State s)
{
    switch ( s ) {
        case Off: return tr("Receiver off");
        case On: return tr("Receiver on");
        case On5Mins: return tr("On for 5 minutes");
        case On1Item: return tr("On for 1 item");
    }
    return QString();
}

QString IRController::protocolName(int i) const
{
    return names[i];
}

QIcon IRController::protocolIcon(int i) const
{
    QString ic = icons[i];
    if ( !ic.isEmpty() )
        return QIcon(":icon/"+ic);
    return QIcon();
}

bool IRController::setProtocol(int i)
{
    // read target config (validity test)
    QString dev = targets[i];
    QSettings cfgin(dev, QSettings::IniFormat);
    if ( cfgin.status()==QSettings::NoError ) {
        // write Beam config
        QSettings cfgout("Trolltech","Beam");
        cfgout.beginGroup("Send");
        cfgout.setValue("DeviceConfig",dev);
        return true;
    } else {
        return false;
    }
}

void IRController::setState(State s)
{
    if (!m_device)
        return;

    m_st = s;
    switch (s) {
        case IRController::Off:
            m_device->bringDown();
            break;
        case IRController::On:
            m_device->bringUp();
            break;
        case IRController::On5Mins:
            m_device->bringUpTimed(300);
            break;
        case IRController::On1Item:
            m_device->bringUpOneItem();
            break;
    }
}
