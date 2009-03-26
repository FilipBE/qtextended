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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/soundcard.h"

#include <QTimer>
#include <QDebug>

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>

#include "greenphonevolumeservice.h"

//XXX: removed from below class due to qdoc sillyness
void sendCurrentVolume()
{    
        QString volume;
        volume.setNum(currVolume);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
}
//////////////////////////////////////////////////////////////

class GreenphoneVolumeServicePrivate 
{
public:
    
    void sendCurrentVolume()
    {
        QString volume;
        volume.setNum(currVolume);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
    }

    int currVolume;
};

GreenphoneVolumeService::GreenphoneVolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/GreenphoneVolumeService")
{
    publishAll(Slots);

    m_d = new GreenphoneVolumeServicePrivate;
    m_d->currVolume = 0;

    QTimer::singleShot(0, this, SLOT(registerService()));
}

GreenphoneVolumeService::~GreenphoneVolumeService()
{
    delete m_d;
}

//public slots:
void GreenphoneVolumeService::setVolume(int volume)
{
    adjustVolume(volume, volume, Absolute);
}

void GreenphoneVolumeService::setVolume(int leftChannel, int rightChannel)
{
    adjustVolume(leftChannel, rightChannel, Absolute);
}

void GreenphoneVolumeService::increaseVolume(int increment)
{
    adjustVolume(increment, increment, Relative);
    m_d->sendCurrentVolume();
}

void GreenphoneVolumeService::decreaseVolume(int decrement)
{
    decrement *= -1;

    adjustVolume(decrement, decrement, Relative);
    m_d->sendCurrentVolume();

}

void GreenphoneVolumeService::setMute(bool)
{
}

void GreenphoneVolumeService::registerService()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Headset") << QString("QPE/AudioVolumeManager/GreenphoneVolumeService");

    QTimer::singleShot(0, this, SLOT(setCallDomain()));
}

void GreenphoneVolumeService::setCallDomain()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Headset");
}


void GreenphoneVolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    int mixerFd = open("/dev/mixer", O_RDWR);
    if (mixerFd >= 0) {
        unsigned int leftright;
        int left;
        int right;

        if (adjust == Relative) {
            ioctl(mixerFd, SOUND_MIXER_READ_VOLUME, &leftright);

            left = (leftright & 0xff00) >> 8;
            right = (leftright & 0x00ff);

            left = qBound(0, left + leftChannel, 100);
            right = qBound(0, right + rightChannel, 100);
        } else {
            left = leftChannel;
            right = rightChannel;
        }

        leftright = (left << 8) | right;
        ioctl(mixerFd, SOUND_MIXER_WRITE_VOLUME, &leftright);       
        m_d->currVolume = (int)(left+right)>>1;
        close(mixerFd);
    }
}

QTOPIA_TASK(GreenphoneVolumeService, GreenphoneVolumeService);

