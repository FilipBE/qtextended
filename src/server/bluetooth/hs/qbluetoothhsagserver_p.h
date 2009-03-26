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

#ifndef QBLUETOOTHHSAGSERVER_P_H
#define QBLUETOOTHHSAGSERVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiacomm/qbluetoothaudiogateway.h>
#include <qtopiaglobal.h>

class QBluetoothHeadsetCommInterface;

class QBluetoothHeadsetAudioGatewayServer : public QBluetoothAudioGateway
{
    Q_OBJECT
    friend class QBluetoothHeadsetAudioGatewayServerPrivate;

public:
    QBluetoothHeadsetAudioGatewayServer(QBluetoothHeadsetCommInterface *parent,
                                        const QString &audioDev,
                                        const QString& service);
    ~QBluetoothHeadsetAudioGatewayServer();

    using QCommInterface::setValue;

public slots:
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

private:
    QBluetoothHeadsetCommInterface *m_parent;
};

#endif
