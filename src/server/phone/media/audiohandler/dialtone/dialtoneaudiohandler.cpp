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

#include "dialtoneaudiohandler.h"

#include <QAudioStateConfiguration>
#include <QAudioStateInfo>
#include <QtopiaIpcEnvelope>
#include <QTelephonyTones>

#include "dtmfaudio.h"
#include "qabstractcallpolicymanager.h"


DialtoneAudioHandler::DialtoneAudioHandler(QObject *parent)
    : AbstractAudioHandler(parent), audioActive(false)
{
    connect(audioConf, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
            this, SLOT(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)));

    dtmf = qtopiaTask<DtmfAudio>();
    if (!dtmf)
        qLog(Component) << "DtmfAudio not available";
}


DialtoneAudioHandler::~DialtoneAudioHandler()
{
}

void DialtoneAudioHandler::initialize()
{
    QSet<QAudioStateInfo> states = audioConf->states("Phone");

    //must be in sync with DialtoneAudioHandler::audioProfileForKey(int)
    foreach (QAudioStateInfo state, states) {
        if ( QString(state.profile()).contains( "speaker", Qt::CaseInsensitive ) )
            keyAudioMap.insert( "speaker", state );
        else if ( QString(state.profile()).contains( "handset", Qt::CaseInsensitive ) )
            keyAudioMap.insert( "handset", state );
        else if ( QString(state.profile()).contains( "headset", Qt::CaseInsensitive ) )
            keyAudioMap.insert( "headset", state );
    }
}

void DialtoneAudioHandler::activateAudio(bool enableAudio)
{
    if (enableAudio == audioActive)
        return;

    audioActive = enableAudio;

    if (audioActive) {
        // Determine if we are registered to some network.  If not,
        // then play the NoService tone instead of the Dial tone.
        // This provides an audible indication to the user that their
        // call may not go through.
        QList<QAbstractCallPolicyManager *> managers;
        managers = qtopiaTasks<QAbstractCallPolicyManager>();
        QTelephonyTones::Tone tone = QTelephonyTones::NoService;
        foreach (QAbstractCallPolicyManager *manager, managers) {
            QTelephony::RegistrationState state = manager->registrationState();
            if (state == QTelephony::RegistrationHome ||
                state == QTelephony::RegistrationUnknown ||
                state == QTelephony::RegistrationRoaming) {
                tone = QTelephonyTones::Dial;
                break;
            }
        }

        // start playing the dial tone
        if (dtmf)
            dtmf->playDtmfTone( tone, -1 );

        QByteArray domain("Phone");
        int capability = static_cast<int>(QAudio::OutputOnly);
        ipcAdaptor->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
        e << QString("Phone");

    }
    else {
        // stop tone
        if (dtmf)
            dtmf->stopAllTones();

        //TODO: This needs to be fixed  up later to send the release
        // domain message instead
        QByteArray domain("Media");
        int capability = static_cast<int>(QAudio::OutputOnly);
        ipcAdaptor->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "resetActiveDomain(QString)");
        e << QString("Phone");

        setAudioProfile(QByteArray());
    }
}


void DialtoneAudioHandler::transferAudio(const QByteArray& profile)
{
    setAudioProfile( profile );
    ipcAdaptor->send("setProfile(QByteArray)", keyAudioMap[profile].profile());
}

void DialtoneAudioHandler::currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability)
{
    QHash<QByteArray, QAudioStateInfo>::const_iterator it = keyAudioMap.constBegin();
    while (it != keyAudioMap.constEnd()) {
        if (it.value() == state) {
            setAudioProfile(it.key());
            return;
        }
        ++it;
    }
}


QByteArray DialtoneAudioHandler::audioType()
{
    return QByteArray("DialtoneAudio");
}

QTOPIA_TASK_PROVIDES(DialtoneAudioHandler, AbstractAudioHandler);
QTOPIA_TASK(DialtoneAudioHandler, DialtoneAudioHandler);
