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

#include <QMediaContent>
#include <QMediaControl>

#include "backend.h"

#include "audiooutput.h"


namespace Phonon
{

namespace qtopiamedia
{

class AudioOutputPrivate
{
public:
    Backend*        backend;
    QMediaContent*  content;
    QMediaControl*  control;
    qreal           volume;
};

/*!
    \class Phonon::qtopiamedia::AudioOutput
    \internal
*/

AudioOutput::AudioOutput(Backend* backend, QObject* parent):
    QObject(parent),
    d(new AudioOutputPrivate)
{
    d->backend = backend;
    d->content = 0;
    d->control = 0;

    d->volume = -1;
}

AudioOutput::~AudioOutput()
{
    delete d;
}

qreal AudioOutput::volume() const
{
    return d->control != 0 ? d->control->volume() / 100 : 0;
}

void AudioOutput::setVolume(qreal volume)
{
    d->volume = volume;

    if (d->control != 0) {
        if (volume == 0)
            d->control->setMuted(true);
        else {
            if (d->control->isMuted())
                d->control->setMuted(false);

            d->control->setVolume(qRound(volume * 100));
        }
    }
}

int AudioOutput::outputDevice() const
{
    return 0;
}

bool AudioOutput::setOutputDevice(int)
{
    return true;
}

bool AudioOutput::connectNode(MediaNode* node)
{
    Q_UNUSED(node);
    return false;
}

bool AudioOutput::disconnectNode(MediaNode* node)
{
    Q_UNUSED(node);
    return false;
}

void AudioOutput::setContent(QMediaContent* content)
{
    if (d->content != content) {
        d->content = content;
        if (d->content != 0) {
            d->control = new QMediaControl(d->content);
            connect(d->control, SIGNAL(volumeChanged(int)), SLOT(changeVolume(int)));
            connect(d->control, SIGNAL(volumeMuted(bool)), SLOT(volumeMuted(bool)));

            if (d->volume != -1)
                setVolume(d->volume);
        }
    }
}

void AudioOutput::changeVolume(int volume)
{
    emit volumeChanged(volume / 100);
}

void AudioOutput::volumeMuted(bool muted)
{
    Q_UNUSED(muted);
    emit volumeChanged(0.0);
}

}

}

