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
#include "pairingagent.h"
#include <qbluetoothlocaldevice.h>

#include <qtopialog.h>

#include <QTimer>


PairingAgent::PairingAgent(QBluetoothLocalDevice *local, QObject *parent)
    : QObject(parent),
      m_local(local),
      m_running(false)
{
    connect(m_local, SIGNAL(pairingCreated(QBluetoothAddress)),
            SLOT(pairingCreated(QBluetoothAddress)));
    connect(m_local, SIGNAL(pairingFailed(QBluetoothAddress)),
            SLOT(pairingFailed(QBluetoothAddress)));
}

PairingAgent::~PairingAgent()
{
}

void PairingAgent::start(const QBluetoothAddress &remoteAddress)
{
    if (m_running) {
        qLog(Bluetooth) << "PairingAgent: already running";
        finish(true);
        return;
    }

    m_address = remoteAddress;
    m_canceled = false;
    m_delayedPairing = false;

    m_running = true;

    if (!m_address.isValid()) {
        qLog(Bluetooth) << "PairingAgent::start() device address is not valid!";
        finish(true);
        return;
    }

    qLog(Bluetooth) << "PairingAgent: start pairing";
    beginPairing();
}

void PairingAgent::beginPairing()
{
    if (m_canceled) {
        qLog(Bluetooth) << "PairingAgent: pairing cancelled by user"
                << "before it has actually started";
        return;
    }

    if (!m_local->isValid() || !m_local->requestPairing(m_address)) {
        qLog(Bluetooth) << "Unable to start pairing process";
        pairingFailed(m_address);
    }
}

void PairingAgent::pairingFailed(const QBluetoothAddress &)
{
    // ignore if pairing was not initiated by this object
    if (!m_running)
        return;

    qLog(Bluetooth) << "PairingAgent: pairing failed!"
            << m_local->error() << m_local->errorString();

    // if got InProgress error, try pairing again later (you can always 
    // call cancel())
    if (m_local->error() == QBluetoothLocalDevice::InProgress) {
        qLog(Bluetooth) << "PairingAgent: device busy, delaying pairing";
        m_delayedPairing = true;
        QTimer::singleShot(200, this, SLOT(beginPairing()));
        return;
    }

    m_delayedPairing = false;

    if (m_local->error() == QBluetoothLocalDevice::AuthenticationCancelled) {
        m_canceled = true;
        finish(false);
    } else {
        finish(true);
    }
}

void PairingAgent::pairingCreated(const QBluetoothAddress &)
{
    // ignore if pairing was not initiated by this object
    if (!m_running)
        return;

    qLog(Bluetooth) << "PairingAgent::pairingCreated()";
    finish(false);
}

void PairingAgent::cancel()
{
    qLog(Bluetooth) << "PairingAgent::cancel()";

    if (m_delayedPairing) {
        // haven't actually started the process, so stop everything now.
        m_canceled = true;
        finish(false);
    } else {
        // can't do much if error while cancelling, it will just keep going
        // ahead with the pairing
        if (!m_local->cancelPairing(m_address))
            qLog(Bluetooth) << "Unable to cancel pairing";
    }

    // if cancel is successful, pairingFailed() will be called
    // and device error will be AuthenticationCancelled
}

void PairingAgent::finish(bool error)
{
    qLog(Bluetooth) << "PairingAgent: done. Error?" << error;
    m_running = false;
    emit done(error);
}
