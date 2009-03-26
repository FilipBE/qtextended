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

#ifndef PAIRINGAGENT_H
#define PAIRINGAGENT_H

#include <qbluetoothaddress.h>
#include <QObject>

class QBluetoothLocalDevice;

class PairingAgent : public QObject
{
    Q_OBJECT

public:
    PairingAgent(QBluetoothLocalDevice *local, QObject *parent = 0);
    ~PairingAgent();

    void start(const QBluetoothAddress &remoteAddress);
    inline bool wasCanceled() const { return m_canceled; }
    inline QBluetoothAddress remoteAddress() const { return m_address; }

signals:
    void done(bool error);

public slots:
    void cancel();

private slots:
    void pairingCreated(const QBluetoothAddress &addr);
    void pairingFailed(const QBluetoothAddress &addr);
    void beginPairing();

private:
    void finish(bool error);

    QBluetoothLocalDevice *m_local;
    QBluetoothAddress m_address;
    bool m_running;
    bool m_canceled;
    bool m_delayedPairing;

    Q_DISABLE_COPY(PairingAgent)
};

#endif
