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

#include "nokiaaudioplugin.h"

#include <QAudioState>
#include <qaudionamespace.h>
#include <QDebug>
#include <qplugin.h>
#include <QAudioStateInfo>
#include <qtopialog.h>

typedef bool (*callback_t)();

class GenericState : public QAudioState
{
    Q_OBJECT

public:
    explicit GenericState(const QAudioStateInfo &info, QAudio::AudioCapabilities capabilities,
                            callback_t avail,
                            callback_t enter,
                            callback_t leave,
                            QObject *parent = 0);
    virtual ~GenericState();

    virtual QAudioStateInfo info() const;
    virtual QAudio::AudioCapabilities capabilities() const;

    virtual bool isAvailable() const;
    virtual bool enter(QAudio::AudioCapability capability);
    virtual bool leave();

private:
    QAudioStateInfo m_info;
    QAudio::AudioCapabilities m_caps;
    callback_t m_avail;
    callback_t m_enter;
    callback_t m_leave;
};

GenericState::GenericState(const QAudioStateInfo &info, QAudio::AudioCapabilities capabilities,
                            callback_t avail,
                            callback_t enter,
                            callback_t leave,
                            QObject *parent)
    : QAudioState(parent)
{
    m_info = info;
    m_caps = capabilities;
    m_avail = avail;
    m_enter = enter;
    m_leave = leave;
}

GenericState::~GenericState()
{

}

QAudioStateInfo GenericState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities GenericState::capabilities() const
{
    return m_caps;
}

bool GenericState::isAvailable() const
{
    qLog(AudioState) << "GenericState::isAvailable called for" << m_info;
    return m_avail();
}

bool GenericState::enter(QAudio::AudioCapability capability)
{
    qLog(AudioState) << "GenericState::enter called for" << m_info << "with capability:" << capability;
    return m_enter();
}

bool GenericState::leave()
{
    qLog(AudioState) << "GenericState::leave called for" << m_info;
    return m_leave();
}

static bool set_stereo_output()
{
    return true;
}

static bool is_stereo_output_avail()
{
    return true;
}

static bool leave_stereo_output()
{
    return true;
}

static bool dummy()
{
    return true;
}

#ifdef QTOPIA_BLUETOOTH
#include <QBluetoothAudioGateway>
#include <QBluetoothAddress>

class BluetoothAudioState : public QAudioState
{
    Q_OBJECT
public:
    explicit BluetoothAudioState(bool isPhone, QObject *parent = 0);
    virtual ~BluetoothAudioState();

    virtual QAudioStateInfo info() const;
    virtual QAudio::AudioCapabilities capabilities() const;

    virtual bool isAvailable() const;
    virtual bool enter(QAudio::AudioCapability capability);
    virtual bool leave();

private slots:
    void bluetoothAudioStateChanged();
    void headsetDisconnected();
    void headsetConnected();

private:
    bool resetCurrAudioGateway();

private:
    QList<QBluetoothAudioGateway *> m_audioGateways;
    bool m_isPhone;
    QBluetoothAudioGateway *m_currAudioGateway;
    QtopiaIpcAdaptor *adaptor;
    QAudioStateInfo m_info;
    bool m_isActive;
    bool m_isAvail;
};

BluetoothAudioState::BluetoothAudioState(bool isPhone, QObject *parent)
    : QAudioState(parent)
{
    m_isPhone = isPhone;
    m_currAudioGateway = 0;

    QBluetoothAudioGateway *hf = new QBluetoothAudioGateway("BluetoothHandsfree");
    m_audioGateways.append(hf);
    qLog(AudioState) << "Handsfree audio gateway: " << hf;

    QBluetoothAudioGateway *hs = new QBluetoothAudioGateway("BluetoothHeadset");
    m_audioGateways.append(hs);
    qLog(AudioState) << "Headset audio gateway: " << hs;

    for (int i=0; i<m_audioGateways.size(); i++) {
        QBluetoothAudioGateway *gateway = m_audioGateways.at(i);
        connect(gateway, SIGNAL(audioStateChanged()), SLOT(bluetoothAudioStateChanged()));
        connect(gateway, SIGNAL(headsetDisconnected()), SLOT(headsetDisconnected()));
        connect(gateway, SIGNAL(connectResult(bool,QString)),
                SLOT(headsetConnected()));
        connect(gateway, SIGNAL(newConnection(QBluetoothAddress)),
                SLOT(headsetConnected()));
    }

    if (isPhone) {
        m_info.setDomain("Phone");
        m_info.setProfile("PhoneBluetoothHeadset");
        m_info.setPriority(25);
    } else {
        m_info.setDomain("Media");
        m_info.setProfile("MediaBluetoothHeadset");
        m_info.setPriority(150);
    }

    m_info.setDisplayName(tr("Bluetooth Headset"));

    adaptor = new QtopiaIpcAdaptor("QPE/Nokia", this );

    m_isAvail = false;
    if(resetCurrAudioGateway())
        m_isAvail = true;
}

BluetoothAudioState::~BluetoothAudioState()
{
    for (int i = 0; i < m_audioGateways.size(); i++) {
        delete m_audioGateways.at(i);
    }
}

bool BluetoothAudioState::resetCurrAudioGateway()
{
    for (int i=0; i<m_audioGateways.size(); i++) {
        QBluetoothAudioGateway *gateway = m_audioGateways.at(i);
        if (gateway->isConnected()) {
            m_currAudioGateway = gateway;
            qLog(AudioState) << "Returning audiogateway to be:" << m_currAudioGateway;
            return true;
        }
    }

    qLog(AudioState) << "No current audio gateway found";
    return false;
}

void BluetoothAudioState::bluetoothAudioStateChanged()
{
    qLog(AudioState) << "bluetoothAudioStateChanged" << m_isActive << m_currAudioGateway;

    if (m_isActive && (m_currAudioGateway || resetCurrAudioGateway())) {
        if (!m_currAudioGateway->audioEnabled()) {
            emit doNotUseHint();
        }
    }
    else if (!m_isActive && (m_currAudioGateway || resetCurrAudioGateway())) {
        if (m_currAudioGateway->audioEnabled()) {
            emit useHint();
        }
    }
}

void BluetoothAudioState::headsetConnected()
{
    if (!m_isAvail && resetCurrAudioGateway()) {
        m_isAvail = true;
        emit availabilityChanged(true);
    }
}

void BluetoothAudioState::headsetDisconnected()
{
    if (!resetCurrAudioGateway()) {
        m_isAvail = false;
        emit availabilityChanged(false);
    }
}

QAudioStateInfo BluetoothAudioState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities BluetoothAudioState::capabilities() const
{
    // Cannot record Bluetooth convos,
    if (m_isPhone) {
        return QAudio::OutputOnly;
    }
    else {
        return QAudio::OutputOnly | QAudio::InputOnly;
    }
}

bool BluetoothAudioState::isAvailable() const
{
    return m_isAvail;
}

bool BluetoothAudioState::enter(QAudio::AudioCapability /*capability*/)
{
    if (m_currAudioGateway || resetCurrAudioGateway()) {
        m_currAudioGateway->connectAudio();
        m_isActive = true;

        return true;
    }

    return false;
}

bool BluetoothAudioState::leave()
{
    if (m_currAudioGateway || resetCurrAudioGateway()) {
        m_currAudioGateway->releaseAudio();
        m_isActive = false;
        return true;
    }

    return false;
}
#endif

class NokiaAudioPluginPrivate
{
public:
    QList<QAudioState *> m_states;
};

#define ADD_STATE(dom, prof, display, pri, cap, avail, enter, leave) \
    m_data->m_states.push_back(new GenericState(QAudioStateInfo(dom, prof, display, pri), \
                                cap, avail, enter, leave, this))

NokiaAudioPlugin::NokiaAudioPlugin(QObject *parent)
    : QAudioStatePlugin(parent)
{
    m_data = new NokiaAudioPluginPrivate;

    ADD_STATE("Media", "MediaSpeaker", tr("Speaker"), 100,
                QAudio::OutputOnly | QAudio::InputOnly,
                is_stereo_output_avail, set_stereo_output, leave_stereo_output);

    ADD_STATE("Media", "MediaHeadphones", tr("Headphones"), 50,
                QAudio::OutputOnly | QAudio::InputOnly,
                dummy, dummy, dummy);

    /*
    // Can play media through bluetooth.  Can record through bluetooth as well.
    ADD_STATE("Media", "MediaBluetoothHeadset", tr("Bluetooth"), 25,
                QAudio::InputOnly | QAudio::OutputOnly,
                dummy, dummy, dummy);
    */

#ifdef QTOPIA_BLUETOOTH
    m_data->m_states.push_back(new BluetoothAudioState(false, this));
    m_data->m_states.push_back(new BluetoothAudioState(true, this));
#endif

    ADD_STATE("Phone", "PhoneSpeaker", tr("Speaker"), 200,
                QAudio::InputOnly | QAudio::OutputOnly,
                dummy, dummy, dummy);

    ADD_STATE("Phone", "PhoneHeadset", tr("Headset"), 50,
                QAudio::InputOnly | QAudio::OutputOnly,
                dummy, dummy, dummy);

    ADD_STATE("RingTone", "RingSpeaker", tr("Speaker"), 100,
                QAudio::OutputOnly,
                dummy, dummy, dummy);

    ADD_STATE("RingTone", "RingHeadphones", tr("Headphones"), 50,
                QAudio::OutputOnly,
                dummy, dummy, dummy);
}

NokiaAudioPlugin::~NokiaAudioPlugin()
{
    delete m_data;
}

QList<QAudioState *> NokiaAudioPlugin::statesProvided() const
{
    return m_data->m_states;
}

Q_EXPORT_PLUGIN2(nokiaaudio_plugin, NokiaAudioPlugin)

#include "nokiaaudioplugin.moc"
