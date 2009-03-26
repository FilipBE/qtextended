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

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>

#include <phonon/audiooutputinterface.h>

#include "medianode.h"


namespace Phonon
{

namespace qtopiamedia
{

class Backend;

class AudioOutputPrivate;

class AudioOutput :
    public QObject,
    public AudioOutputInterface,
    public MediaNode
{
    Q_OBJECT
    Q_INTERFACES(Phonon::AudioOutputInterface Phonon::qtopiamedia::MediaNode)

public:
    AudioOutput(Backend* backend, QObject* parent);
    ~AudioOutput();

    qreal volume() const;
    void setVolume(qreal);

    int outputDevice() const;
    bool setOutputDevice(int);

    // Medianode
    bool connectNode(MediaNode* node);
    bool disconnectNode(MediaNode* node);
    void setContent(QMediaContent* content);

signals:
    void volumeChanged(qreal newVolume);
    void audioDeviceFailed();

private slots:
    void changeVolume(int);
    void volumeMuted(bool);

private:
    AudioOutputPrivate* d;
};

}

}

#endif  // AUDIOOUTPUT_H
