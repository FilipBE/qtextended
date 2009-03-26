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

#ifndef QBLUETOOTHAUDIOGATEWAY_H
#define QBLUETOOTHAUDIOGATEWAY_H

#include <qbluetoothglobal.h>
#include <qcomminterface.h>

class QBluetoothAddress;
class QBluetoothRfcommSocket;

class QBLUETOOTH_EXPORT QBluetoothAudioGateway : public QCommInterface
{
    Q_OBJECT

public:
    explicit QBluetoothAudioGateway(const QString& service = QString(),
                                    QObject *parent = 0,
                                    QAbstractIpcInterface::Mode mode = Client );
    ~QBluetoothAudioGateway();

    int speakerVolume() const;
    int microphoneVolume() const;
    bool isConnected() const;
    QBluetoothAddress remotePeer() const;
    bool audioEnabled() const;

public slots:
    virtual void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    virtual void disconnect();
    virtual void setSpeakerVolume(int volume);
    virtual void setMicrophoneVolume(int volume);
    virtual void releaseAudio();
    virtual void connectAudio();

signals:
    void connectResult(bool success, const QString &msg);
    void newConnection(const QBluetoothAddress &addr);
    void headsetDisconnected();
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();
};

#endif
