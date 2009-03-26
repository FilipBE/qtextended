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

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

#include <qmediaengineinformation.h>
#include <qmediasessionbuilder.h>

#include "ffmpegengineinformation.h"
#include "ffmpegengine.h"


namespace ffmpeg
{

class EnginePrivate
{
public:
    EngineInformation*          engineInformation;
    QMediaSessionBuilderList    sessionBuilders;
};

/*!
    \class ffmpeg::Engine
    \internal
*/

Engine::Engine():
    s(0),
    d(new EnginePrivate)
{
    d->engineInformation = new EngineInformation(this);
}

Engine::~Engine()
{
    delete d;
}


void Engine::initialize()
{
    av_register_all();
}

void Engine::start()
{
}

void Engine::stop()
{
}

void Engine::suspend()
{
    if(s)
        s->suspend();
}

void Engine::resume()
{
    if(s)
        s->resume();
}

void Engine::registerSession(QMediaServerSession* session)
{
    s = static_cast<PlaybinSession*>(session);
}

void Engine::unregisterSession(QMediaServerSession* session)
{
    Q_UNUSED(session);
}

QMediaEngineInformation const* Engine::engineInformation()
{
    return d->engineInformation;
}

}   // ns ffmpeg

