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

#include <gst/gst.h>

#include <qmediaengineinformation.h>
#include <qmediasessionbuilder.h>

#include "gstreamerengineinformation.h"
#include "gstreamerengine.h"


namespace gstreamer
{

class EnginePrivate
{
public:
    EngineInformation*          engineInformation;
    QMediaSessionBuilderList    sessionBuilders;
};

/*!
    \class gstreamer::Engine
    \internal
*/

Engine::Engine():
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
    gst_init(NULL, NULL);
//    gst_debug_set_active(TRUE);
//    gst_debug_set_default_threshold(GST_LEVEL_INFO);
}

void Engine::start()
{
}

void Engine::stop()
{
}

void Engine::suspend()
{
    foreach( PlaybinSession *s, sessions ) {
        if ( !s->isSuspended() ) {
            s->suspend();
            suspendedSessions.append(s);
        }
    }
}

void Engine::resume()
{
    foreach( PlaybinSession *s, suspendedSessions ) {
        s->resume();
    }
    suspendedSessions.clear();
}

void Engine::registerSession(QMediaServerSession* session)
{
    PlaybinSession *s = static_cast<PlaybinSession*>(session);
    sessions.append(s);
}

void Engine::unregisterSession(QMediaServerSession* session)
{
    PlaybinSession *s = static_cast<PlaybinSession*>(session);
    sessions.removeAll(s);
}

QMediaEngineInformation const* Engine::engineInformation()
{
    return d->engineInformation;
}

}   // ns gstreamer

