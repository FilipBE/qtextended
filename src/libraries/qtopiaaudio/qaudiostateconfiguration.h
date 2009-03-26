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

#ifndef QAUDIOSTATECONFIGURATION_H
#define QAUDIOSTATECONFIGURATION_H

#include <QObject>
#include <qtopiaglobal.h>
#include <qaudionamespace.h>

class QByteArray;
class QAudioStateInfo;

class QAudioStateConfigurationPrivate;
class QTOPIAAUDIO_EXPORT QAudioStateConfiguration : public QObject
{
    Q_OBJECT

    friend class QAudioStateConfigurationPrivate;

public:
    explicit QAudioStateConfiguration(QObject *parent = 0);
    ~QAudioStateConfiguration();

    bool isInitialized() const;

    QSet<QByteArray> domains() const;
    QSet<QAudioStateInfo> states() const;
    QSet<QAudioStateInfo> states(const QByteArray &domain) const;

    bool isStateAvailable(const QAudioStateInfo &state) const;
    QAudio::AudioCapabilities availableCapabilities(const QAudioStateInfo &state) const;

    QAudioStateInfo currentState() const;
    QAudio::AudioCapability currentCapability() const;

signals:
    void configurationInitialized();
    void availabilityChanged();
    void currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability capability);

private:
    QAudioStateConfigurationPrivate *m_data;
};


#endif
