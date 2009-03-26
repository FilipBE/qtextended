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

#ifndef BTAUDIOVOLUMEMANAGER_H
#define BTAUDIOVOLUMEMANAGER_H

#include <QObject>

class BluetoothAudioVolumeControl;
class QBluetoothAddress;

class BtAudioVolumeManager : public QObject
{
    Q_OBJECT

public:
    BtAudioVolumeManager(const QString &service, QObject *parent = 0);
    ~BtAudioVolumeManager();

private slots:
    void serviceAdded(const QString &service);
    void serviceRemoved(const QString &service);

    void audioGatewayConnected(bool success, const QString &msg);
    void audioGatewayDisconnected();
    void audioDeviceConnected(const QBluetoothAddress &addr);
    void audioStateChanged();

private:
    void createVolumeControl();
    void removeVolumeControl();

    QString m_service;
    BluetoothAudioVolumeControl *m_volumeControl;
};

#endif
