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

#include <qtopiaipcenvelope.h>
#include <QValueSpaceObject>
#include <QSettings>
#include "audiovolumemanager.h"

#include <QTimer>
#include <QDebug>

AudioVolumeManager::AudioVolumeManager():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager")
{
    publishAll(Slots);

    m_vsVolume = new QValueSpaceObject("/System/Volume");

    audioConf = new QAudioStateConfiguration(this);
    connect(audioConf, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),this,SLOT(domainChanged(QAudioStateInfo,QAudio::AudioCapability)));

    m_vsVolume->setAttribute("CurrentVolume",100);

    m_vsVolume->setAttribute("CurrentDomain","Media");

    QSettings cfg("Trolltech", "Sound");
    cfg.beginGroup("System");
    m_vsVolume->setAttribute("Attenuation",
            cfg.value("Attenuation",30).toInt());
    m_vsVolume->setAttribute("Amplification",
            cfg.value("Amplification",30).toInt());
    m_vsVolume->setAttribute("Fade",
            cfg.value("Fade",true).toBool());
}

AudioVolumeManager::~AudioVolumeManager()
{
    delete m_vsVolume;
}

bool AudioVolumeManager::canManageVolume() const
{
    if(m_domains.isEmpty())
        return false;

    return true;
}

bool AudioVolumeManager::isMuted() const
{
    if (m_domains.isEmpty())
        return false;
    else
        return m_muted.contains(m_domains.front());
}

//public slots:
void AudioVolumeManager::setVolume(int volume)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "setVolume(int)");
        e << volume;
    }
}

void AudioVolumeManager::increaseVolume(int increment)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "increaseVolume(int)");
        e << increment;
    }
}

void AudioVolumeManager::decreaseVolume(int decrement)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "decreaseVolume(int)");
        e << decrement;
    }
}

void AudioVolumeManager::setMuted(bool mute)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "setMuted(bool)");
        e << mute;
    }
}

void AudioVolumeManager::toggleMuted()
{
    setMuted(!isMuted());
}

void AudioVolumeManager::currentVolume(QString const&  volume)
{
    // We get beck here the actual volume
    m_vsVolume->setAttribute("CurrentVolume",volume.toInt());
}

void AudioVolumeManager::currentMuted(QString const& domain, bool muted)
{
    if (muted)
        m_muted += domain;
    else
        m_muted -= domain;
    if (!m_domains.isEmpty() && domain == m_domains.front()) {
        m_vsVolume->setAttribute("CurrentMuted", muted);
    }
}

void AudioVolumeManager::registerHandler(QString const& domain, QString const& channel)
{
    m_vsps[domain] = channel;
    m_vspsvol[domain] = 100;
}

void AudioVolumeManager::unregisterHandler(QString const& domain, QString const& channel)
{
    VolumeServiceProviders::iterator it = m_vsps.find(domain);
    if (it != m_vsps.end() && (*it).compare(channel) == 0)
        m_vsps.erase(it);
}

void AudioVolumeManager::setActiveDomain(QString const& activeDomain)
{
    m_vsVolume->setAttribute("CurrentDomain",activeDomain);

    m_domains.push_front(activeDomain);
    updateMuted();
}

void AudioVolumeManager::resetActiveDomain(QString const& oldDomain)
{
    m_domains.removeAll(oldDomain);
    updateMuted();
}

QString AudioVolumeManager::findProvider() const
{
    QString     domain;
    QString     provider;

    if (!m_domains.isEmpty())
    {
        domain = m_domains.front();

        VolumeServiceProviders::const_iterator it = m_vsps.find(domain);
        if (it != m_vsps.end())
            provider = *it;
        else {
            it = m_vsps.find("Generic");
            if(it != m_vsps.end()) {
                provider = *it;
            } else {
                it = m_vsps.begin();
                if(it != m_vsps.end())
                    provider = *it;
            }
        }
        if(it == m_vsps.end()) {
            it = m_vsps.find("Media");
            if(it != m_vsps.end())
                provider = *it;
        }
    }
    return provider;
}

void AudioVolumeManager::updateMuted()
{
    if (!m_domains.isEmpty()) {
        m_vsVolume->setAttribute("CurrentMuted", m_muted.contains(m_domains.front()));
    } else {
        m_vsVolume->setAttribute("CurrentMuted", false);
    }
}

void AudioVolumeManager::domainChanged(const QAudioStateInfo &state,QAudio::AudioCapability capability)
{
    Q_UNUSED(capability)
    bool match = false;
    QMapIterator<QString, int> i(m_vspsvol);

    while(i.hasNext()) {
        i.next();
        if(i.key().contains(QString(state.domain().data()))) {
            m_domains.push_front(QString(state.domain().data()));
            match = true;
            break;
        }
    }
    if(!match) {
        while(i.hasNext()) {
            i.next();
            if(i.key().contains("Generic")) {
                m_domains.push_front("Generic");
                break;
            }
        }
    }

    updateMuted();
}

