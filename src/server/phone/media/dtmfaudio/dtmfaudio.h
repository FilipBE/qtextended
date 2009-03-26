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

#ifndef DTMFAUDIO_H
#define DTMFAUDIO_H

#include "qtopiaserverapplication.h"

#include <QTelephonyTones>

class DtmfAudioPrivate;
class DtmfAudio : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool exclusivePlay READ exclusivePlay WRITE setExclusivePlay)
    Q_PROPERTY(int duration READ duration WRITE setDuration)
public:
    DtmfAudio(QObject *parent = 0);
    ~DtmfAudio();

    bool exclusivePlay() const;
    void setExclusivePlay(bool exclusive);

    int duration() const;
    void setDuration( int duration );

    QString playDtmfTone(QTelephonyTones::Tone tone, int duration = -1);
    void stopDtmfTone(const QString& toneId);

public slots:
    void playDtmfKeyTone( int key );
    void stopAllTones();

signals:
    void toneStopped(const QString&);

private:
    DtmfAudioPrivate *d;
    friend class DtmfAudioPrivate;
    Q_PRIVATE_SLOT(d, void _q_servicesChanged());
    Q_PRIVATE_SLOT(d, void _q_toneStopped(QString));
};

QTOPIA_TASK_INTERFACE(DtmfAudio);

#endif
