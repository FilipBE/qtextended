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

#ifndef AUDIOVOLUMEMANAGER_H
#define AUDIOVOLUMEMANAGER_H

#include <qstring.h>
#include <qlist.h>
#include <qmap.h>
#include <qset.h>
#include <qtopiaipcadaptor.h>

#include <qaudiostateconfiguration.h>
#include <qaudiostateinfo.h>

class QTimer;

class QValueSpaceObject;
class AudioVolumeManager : public QtopiaIpcAdaptor
{
    Q_OBJECT
    typedef QMap<QString, QString>  VolumeServiceProviders;
    typedef QMap<QString, int>      VolumeServiceProvidersVolume;
    typedef QList<QString>          VolumeDomains;
    typedef QSet<QString>           MutedDomains;

public:
    AudioVolumeManager();
    ~AudioVolumeManager();

    bool canManageVolume() const;
    bool isMuted() const;

signals:
    void volumeChanged(int volume);

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMuted(bool mute);
    void toggleMuted();

    void registerHandler(QString const& domain, QString const& channel);
    void unregisterHandler(QString const& domain, QString const& channel);
    void setActiveDomain(QString const& domain);
    void resetActiveDomain(QString const& domain);
    void currentVolume(QString const& volume);
    void currentMuted(QString const& domain, bool muted);

private slots:
    void domainChanged(const QAudioStateInfo &state,
            QAudio::AudioCapability capability);

private:
    QString findProvider() const;
    void updateMuted();

    VolumeDomains           m_domains;
    VolumeServiceProviders  m_vsps;
    VolumeServiceProvidersVolume m_vspsvol;
    MutedDomains            m_muted;
    QValueSpaceObject      *m_vsVolume;

    QAudioStateConfiguration *audioConf;
};

#endif
