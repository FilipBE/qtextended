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

#include <qdebug.h>
#include <qstring.h>
#include <qtopiaipcenvelope.h>

#include "sessionmanager.h"
#include "mediavolumecontrol.h"

#include <qmediaserversession.h>
#include <QTimer>

namespace mediaserver
{

class MediaVolumeControlPrivate
{
public:
    SessionManager* sessionManager;
   
    void sendCurrentVolume(int val) 
    {
        QString volume;
        volume.setNum(val);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
    };
};

/*!
    \class mediaserver::MediaVolumeControl
    \internal
*/

MediaVolumeControl::MediaVolumeControl(SessionManager* sessionManager):
    QtopiaIpcAdaptor("QPE/MediaServer/MediaVolumeControl"),
    d(new MediaVolumeControlPrivate)
{
    // init
    d->sessionManager = sessionManager;

    //
    publishAll(Slots);

    //
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Media") << QString("QPE/MediaServer/MediaVolumeControl");

    setCallDomain(true);
}


MediaVolumeControl::~MediaVolumeControl()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "unregisterHandler(QString,QString)");

    e << QString("Media") << QString("QPE/MediaServer/MediaVolumeControl");
}


//public slots:
void MediaVolumeControl::setVolume(int volume)
{
    if(d->sessionManager->sessions().isEmpty())
        return;

    foreach(QMediaServerSession *s, d->sessionManager->sessions())
    {

        if(s)
        {
            d->sendCurrentVolume(volume);
            s->setVolume(volume);
        }
    }    
}

void MediaVolumeControl::increaseVolume(int increment)
{
    if(d->sessionManager->sessions().isEmpty())
        return;

    //Condition: there should only be one Session  playing
    foreach(QMediaServerSession *s, d->sessionManager->sessions())
    {
        int state = s->playerState();
        if( state == QtopiaMedia::Playing || state == QtopiaMedia::Buffering || state == QtopiaMedia::Paused)
        {
            int val;
            val = increment + s->volume();
            if(val > 0)
                s->setMuted(false);
            if(val >= 100)
                val = 100;

            d->sendCurrentVolume(val);
            s->setVolume(val);

        }    
    }    
}

void MediaVolumeControl::decreaseVolume(int decrement)
{
    if(d->sessionManager->sessions().isEmpty())
        return;

    //Condition: there should only be one active Session  playing
    foreach(QMediaServerSession *s, d->sessionManager->sessions())
    {
        int state = s->playerState();
        if( state == QtopiaMedia::Playing || state == QtopiaMedia::Buffering || state == QtopiaMedia::Paused)
        {
            int val;
            val = s->volume() - decrement;
            if(val <= 0)
            {
                s->setMuted(true);
                val = 0;    
            }    
            d->sendCurrentVolume(val);  
            s->setVolume(val);
        }
    }    
    
}

void MediaVolumeControl::setMuted(bool mute)
{
    if(d->sessionManager->sessions().isEmpty())
        return;

    foreach(QMediaServerSession *s,  d->sessionManager->sessions())
    {
        if(s)
        {
            s->setMuted(mute);
        }
    }
}

void MediaVolumeControl::setCallDomain(bool active)
{
    QString str; 
    
    if(active)
        str += "setActiveDomain(QString)";
    else
        str += "resetActiveDomain(QString)";

   QtopiaIpcEnvelope   e2("QPE/AudioVolumeManager", str);
   e2 << QString("Media");
}

}   // ns mediaserver



