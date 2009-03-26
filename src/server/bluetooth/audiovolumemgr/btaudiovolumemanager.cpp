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
#include "btaudiovolumemanager.h"
#include <qtopiaipcadaptor.h>
#include <qbluetoothaudiogateway.h>
#include <qtopiaipcenvelope.h>
#include <qtopialog.h>
#include <qcommservicemanager.h>
#include <qbluetoothaddress.h>

static const int VOLUME_MIN = 0;
static const int VOLUME_MAX = 15;


class BluetoothAudioVolumeControl : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    BluetoothAudioVolumeControl(const QString &service, const QString &ipcChannel, QObject *parent);
    ~BluetoothAudioVolumeControl();

    void setEnabled(bool enable);

    QBluetoothAudioGateway* m_gateway;
    QString m_ipcChannel;

    void notifyAudioVolumeManager(int val)
    {
        QString volume;
        volume.setNum(val);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
    }

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMuted(bool mute);
};

BluetoothAudioVolumeControl::BluetoothAudioVolumeControl(const QString &service, const QString &ipcChannel, QObject *parent)
    : QtopiaIpcAdaptor(ipcChannel, parent),
      m_gateway(new QBluetoothAudioGateway(service, this)),
      m_ipcChannel(ipcChannel)
{
    publishAll(Slots);
}

BluetoothAudioVolumeControl::~BluetoothAudioVolumeControl()
{
    // must unregister audio handler when no longer needed
    setEnabled(false);
}

void BluetoothAudioVolumeControl::setVolume(int volume)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: setVolume()" << volume;

    int value = qBound(VOLUME_MIN, volume, VOLUME_MAX);
    if (m_gateway->speakerVolume() != value)
    {    
        m_gateway->setSpeakerVolume(value);
        notifyAudioVolumeManager( qRound(100/VOLUME_MAX*value) );
    }    
}

void BluetoothAudioVolumeControl::increaseVolume(int increment)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: increaseVolume() by" << increment;

    int value = qMin(m_gateway->speakerVolume() + increment, VOLUME_MAX);
    if (m_gateway->speakerVolume() != value)
    {
        m_gateway->setSpeakerVolume(value);
        notifyAudioVolumeManager( qRound(100/VOLUME_MAX*value) );
    }    

}

void BluetoothAudioVolumeControl::decreaseVolume(int decrement)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: decreaseVolume() by" << decrement;

    int value = qMax(m_gateway->speakerVolume() - decrement, VOLUME_MIN);
    if (m_gateway->speakerVolume() != value)
    {
        m_gateway->setSpeakerVolume(value);
        notifyAudioVolumeManager( qRound(100/VOLUME_MAX*value) );
    }    

}

void BluetoothAudioVolumeControl::setMuted(bool mute)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: setMuted()" << mute;
    m_gateway->setSpeakerVolume( mute ? VOLUME_MIN : VOLUME_MAX );
}

void BluetoothAudioVolumeControl::setEnabled(bool enable)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: enable volume handler:" << enable;

    QtopiaIpcEnvelope e1("QPE/AudioVolumeManager",
                        enable ? "registerHandler(QString,QString)" :
                                 "unregisterHandler(QString,QString)" );
    e1 << QString("BluetoothAudio") << m_ipcChannel;

    QtopiaIpcEnvelope e2("QPE/AudioVolumeManager",
                         enable ? "setActiveDomain(QString)" :
                                  "resetActiveDomain(QString)" );
    e2 << QString("BluetoothAudio");
}

//==================================================================


BtAudioVolumeManager::BtAudioVolumeManager(const QString &service, QObject *parent)
    : QObject(parent),
      m_service(service),
      m_volumeControl(0)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: constructing for" << service;

    createVolumeControl();

    QCommServiceManager *serviceManager = new QCommServiceManager(this);
    QObject::connect(serviceManager, SIGNAL(serviceAdded(QString)),
                     this, SLOT(serviceAdded(QString)));
    QObject::connect(serviceManager, SIGNAL(serviceRemoved(QString)),
                     this, SLOT(serviceRemoved(QString)));
}

BtAudioVolumeManager::~BtAudioVolumeManager()
{
}

void BtAudioVolumeManager::createVolumeControl()
{
    if (m_volumeControl)
        removeVolumeControl();

    m_volumeControl = new BluetoothAudioVolumeControl(
            m_service, "QPE/" + m_service + "AudioControl", this);

    qLog(Bluetooth) << "BtAudioVolumeManager: creating" << m_service
            << "volume control...";

    connect(m_volumeControl->m_gateway, SIGNAL(connectResult(bool,QString)),
            SLOT(audioGatewayConnected(bool,QString)));
    connect(m_volumeControl->m_gateway, SIGNAL(disconnected()),
            SLOT(audioGatewayDisconnected()));
    connect(m_volumeControl->m_gateway, SIGNAL(newConnection(QBluetoothAddress)),
            SLOT(audioDeviceConnected(QBluetoothAddress)));
    connect(m_volumeControl->m_gateway, SIGNAL(audioStateChanged()),
            SLOT(audioStateChanged()));
}

void BtAudioVolumeManager::removeVolumeControl()
{
    qLog(Bluetooth) << "BtAudioVolumeManager: removing" << m_service
            << "volume control...";
    delete m_volumeControl;
    m_volumeControl = 0;
}

void BtAudioVolumeManager::serviceAdded(const QString &service)
{
    if (service == m_service && !m_volumeControl)
        createVolumeControl();
}

void BtAudioVolumeManager::serviceRemoved(const QString &service)
{
    if (service == m_service && m_volumeControl)
        removeVolumeControl();
}

void BtAudioVolumeManager::audioGatewayConnected(bool success, const QString &msg)
{
    qLog(Bluetooth) << "BtAudioVolumeManager:" << m_service 
            << "connect result:" << success << msg;
}

void BtAudioVolumeManager::audioGatewayDisconnected()
{
    qLog(Bluetooth) << "BtAudioVolumeManager: disconnected" << m_service;
}

void BtAudioVolumeManager::audioDeviceConnected(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "BtAudioVolumeManager: remote device" << addr.toString()
            << "connected to" << m_service;
}

void BtAudioVolumeManager::audioStateChanged()
{
    qLog(Bluetooth) << "BtAudioVolumeManager::audioStateChanged()";
    if (m_volumeControl)
        m_volumeControl->setEnabled(m_volumeControl->m_gateway->audioEnabled());
}

#include "btaudiovolumemanager.moc"
