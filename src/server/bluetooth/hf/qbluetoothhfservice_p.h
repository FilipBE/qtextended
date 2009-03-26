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

#ifndef QBLUETOOTHHFSERVICE_P_H
#define QBLUETOOTHHFSERVICE_P_H

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

#include <qabstractipcinterfacegroup.h>
#include <qtopiacomm/qbluetoothnamespace.h>
#include <qtopiacomm/qbluetoothabstractservice.h>
#include <qtopiacomm/qbluetoothrfcommsocket.h>
#include <qtopiaglobal.h>

class QBluetoothScoSocket;

class QBluetoothHandsfreeServicePrivate;
class QBluetoothHandsfreeService : public QBluetoothAbstractService
{
    Q_OBJECT

public:
    QBluetoothHandsfreeService(const QString &service, const QString &displayName, QObject *parent = 0);
    ~QBluetoothHandsfreeService();

    void start();
    void stop();

    void setSecurityOptions(QBluetooth::SecurityOptions options);

    // Methods from the Headset AG Interface
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

    // Internal helper methods
    void updateSpeakerVolume(int volume);
    void updateMicrophoneVolume(int volume);

signals:
    // Signals from the Headset AG interface
    void connectResult(bool success, const QString &msg);
    void newConnection(const QBluetoothAddress &addr);
    void disconnected();
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();

protected:
    virtual bool canConnectAudio();

private slots:
    // Server connection
    void newConnection();

    // Client connection
    void stateChanged(QBluetoothAbstractSocket::SocketState socketState);
    void error(QBluetoothAbstractSocket::SocketError error);

    // Sco connection
    void scoStateChanged(QBluetoothAbstractSocket::SocketState socketState);

    // Sco server connection
    void newScoConnection();

    // From ModemEmulator
    void serialPortsChanged();

    // From QCommDeviceSession
    void sessionOpen();
    void sessionFailed();

private:
    bool doConnectAudio();
    void hookupSocket(QBluetoothRfcommSocket *socket);
    bool setupTty(QBluetoothRfcommSocket *socket, bool incoming);
    void doDisconnect();
    void initializeAudioGateway();
    void scoConnectionEstablished(QBluetoothScoSocket *socket);

    QBluetoothHandsfreeServicePrivate *m_data;
};


class QBluetoothHandsfreeCommInterfacePrivate;
class QBluetoothHandsfreeCommInterface : public QAbstractIpcInterfaceGroup
{
    Q_OBJECT

public:
    QBluetoothHandsfreeCommInterface(const QByteArray &audioDev, QBluetoothHandsfreeService *parent);
    ~QBluetoothHandsfreeCommInterface();

    void initialize();

    void setValue(const QString &key, const QVariant &value);

    // Methods from the Headset AG Interface
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

signals:
    // Signals from the Headset AG interface
    void connectResult(bool success, const QString &msg);
    void newConnection(const QBluetoothAddress &addr);
    void disconnected();
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();

private:
    QBluetoothHandsfreeCommInterfacePrivate *m_data;
};

#endif

