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

#include <QList>
#include <QMap>
#include <QTimer>
#include <QDebug>

#include <QValueSpaceObject>
#include <QMediaSessionRequest>
#include <QMediaSessionBuilder>
#include <QMediaServerSession>
#include <QMediaEngine>
#include <QMediaEngineInformation>

#include <qtopialog.h>

#include "engineloader.h"
#include "buildermanager.h"
#include "mediaagentsession.h"

#include "mediaagent.h"

namespace mediaserver
{

typedef QList<MediaAgentSession*>   ActiveSessions;
typedef QMap<QString,QMediaEngine*> QMediaEnginesMap;

// {{{ MediaAgentPrivate
class MediaAgentPrivate
{
public:
    bool                        wrapSessions;
    QValueSpaceObject*          info;
    EngineLoader                engineLoader;
    QMediaEngineList            engines;
    QMediaEnginesMap            enginesMap;
    BuilderManager*             foreman;
    ActiveSessions              activeSessions;
    QMediaEngine*               activeEngine;
    QTimer                      sessionsUpdateTimer;
};
// }}}

/*!
    \class mediaserver::MediaAgent
    \internal
*/

// {{{ MediaAgent
MediaAgent::MediaAgent():
    d(new MediaAgentPrivate)
{
    d->wrapSessions = false;
    d->info = new QValueSpaceObject("/Media/Engines");
    d->foreman = new BuilderManager;
    d->activeEngine = 0;

    // Special case for mediaserver should always resume! 255 priority
    audioMgr = new QAudioInterface("MediaServer", this);
    connect(audioMgr,SIGNAL(audioStarted()),this,SLOT(resume()));
    connect(audioMgr,SIGNAL(audioStopped()),this,SLOT(suspend()));
    audioMgr->startAudio();

    d->sessionsUpdateTimer.setSingleShot( false );
    connect( &d->sessionsUpdateTimer, SIGNAL(timeout()), this, SLOT(updateSessions()) );
}

MediaAgent::~MediaAgent()
{
    d->engineLoader.unload();

    delete d->foreman;
    delete d->info;
    delete d;
    delete audioMgr;
}

QMediaServerSession* MediaAgent::createSession(QMediaSessionRequest const& sessionRequest)
{
    QMediaServerSession*    mediaSession;
    MediaAgentSession*      sessionWrapper = 0;
    QString engineName;

    mediaSession = d->foreman->createSession(sessionRequest, &engineName);
    if (mediaSession != 0)
    {
        if (d->wrapSessions)
        {
            sessionWrapper = new MediaAgentSession(this, mediaSession, engineName);
            d->activeSessions.append(sessionWrapper);

            mediaSession = sessionWrapper;

            if ( !d->sessionsUpdateTimer.isActive() )
                d->sessionsUpdateTimer.start(1000);
        }
    }

    return mediaSession;
}

void MediaAgent::destroySession(QMediaServerSession* mediaSession)
{
    QMediaServerSession*    original = mediaSession;

    if (d->wrapSessions)
    {
        MediaAgentSession*  sessionWrapper = static_cast<MediaAgentSession*>(mediaSession);

        d->activeSessions.removeAll(sessionWrapper);

        original = sessionWrapper->wrappedSession();

        delete sessionWrapper;
        if ( d->activeSessions.isEmpty() )
            d->sessionsUpdateTimer.stop();
    }

    d->foreman->destroySession(original);

    updateSessions();
}

MediaAgent* MediaAgent::instance()
{
    static bool         initialized = false;
    static MediaAgent   self;

    if (!initialized)
    {
        self.initialize();

        initialized = true;
    }

    return &self;
}

// private:
void MediaAgent::initialize()
{
    bool        anExclusiveEngine = false;

    // Load all plugins
    d->engineLoader.load();

    // Sort out capabilities
    d->engines = d->engineLoader.engines();

    d->info->setAttribute("Registered", d->engines.size());

    if (d->engines.size() > 0)
    {
        // Initialize engines
        foreach (QMediaEngine* mediaEngine, d->engines)
        {
            // init
            mediaEngine->initialize();

            // Store {
            QMediaEngineInformation const*  engineInfo = mediaEngine->engineInformation();
            QString     engineName = engineInfo->name();

            // Add builders
            d->foreman->addBuilders(engineName, engineInfo->sessionBuilders());

            // Info in valuespace
            d->info->setAttribute(engineName + "/Version", engineInfo->version());
            d->info->setAttribute(engineName + "/IdleTime", engineInfo->idleTime());
            // }

            d->enginesMap[ engineInfo->name() ] = mediaEngine;

            // Exclusive
            anExclusiveEngine = anExclusiveEngine || engineInfo->hasExclusiveDeviceAccess();
        }
    }

    // Should we manage engines?
    d->wrapSessions = d->engines.size() > 1 && anExclusiveEngine;
}

void MediaAgent::sessionStarting(MediaAgentSession* newSession)
{
    qLog(Media) << "Starting session of engine" << newSession->engineName();
    // Stop others not in this engine
    foreach( MediaAgentSession* session, d->activeSessions ) {
        if ( session->engineName() != newSession->engineName() )
            if ( !session->isSuspended() )
                session->wrappedSession()->suspend();
    }

    // Stop other media engines and start the current one
    activateMediaEngine( d->enginesMap[newSession->engineName()] );

    newSession->wrappedSession()->start();

    // Resume other sessions from the current engine
    foreach( MediaAgentSession* session, d->activeSessions ) {
        if ( session->engineName() == newSession->engineName() )
            if ( !session->isSuspended() )
                session->wrappedSession()->resume();
    }
}

void MediaAgent::activateMediaEngine( QMediaEngine* mediaEngine )
{
    if ( d->wrapSessions ) {
        if (d->activeEngine != mediaEngine) {
            if (d->activeEngine)
                d->activeEngine->suspend();
            d->activeEngine = mediaEngine;
            if (d->activeEngine)
                d->activeEngine->resume();
        }
    }
}

void MediaAgent::updateSessions()
{
    if (!d->wrapSessions || d->activeSessions.isEmpty())
        return;

    QMap<QString,int> notSuspendedSessions;
    QMap<QString,int> activeSessions;

    foreach( MediaAgentSession* session, d->activeSessions ) {
        if ( !session->isSuspended() ) {
            notSuspendedSessions[session->engineName()]++;
            if ( session->isActive() )
                activeSessions[session->engineName()]++;
            //qDebug() << "player state:" <<  session->engineName() << session->playerState();
        }
    }

    //qLog(Media) << "Not suspended sessions:" << notSuspendedSessions << "active:" << activeSessions;

    if (notSuspendedSessions.isEmpty())
        return;

    bool changeEngine = d->activeEngine == 0;

    if (d->activeEngine) {
        QString engineName = d->activeEngine->engineInformation()->name();
        if ( notSuspendedSessions.value(engineName) == 0 ||
             (!activeSessions.isEmpty() && activeSessions.value(engineName) == 0) ) {
            changeEngine = true;
        }
    }

    if ( changeEngine ) {
        //change active engine, suspend all currents sessions first:
        foreach( MediaAgentSession* session, d->activeSessions ) {
            session->wrappedSession()->suspend();
        }
        QString engineName = activeSessions.isEmpty() ? notSuspendedSessions.begin().key() : activeSessions.begin().key();
        qLog(Media) << "Set active media engine to" << engineName;
        activateMediaEngine( d->enginesMap[engineName] );
    }

    if (d->activeEngine) {
        QString engineName = d->activeEngine->engineInformation()->name();

        foreach( MediaAgentSession* session, d->activeSessions ) {
            if ( session->engineName() == engineName ) {
                if ( !session->isSuspended() ) {
                    session->wrappedSession()->resume();
                }
            }
        }
    }
}

void MediaAgent::suspend()
{
    if (!d->wrapSessions) {
        foreach (QMediaEngine* mediaEngine, d->engines) {
            if(mediaEngine) {
                mediaEngine->suspend();
            }
        }
    } else {
        foreach( MediaAgentSession* session, d->activeSessions ) {
            session->wrappedSession()->suspend();
        }
        if ( d->activeEngine )
            d->activeEngine->suspend();
        d->activeEngine = 0;
    }
}

void MediaAgent::resume()
{
    if (!d->wrapSessions) {
        foreach (QMediaEngine* mediaEngine, d->engines) {
            if(mediaEngine) {
                mediaEngine->resume();
            }
        }
    } else {
        updateSessions();
    }
}

// }}}

}   // ns mediaserver

