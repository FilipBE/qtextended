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

#include <QDebug>

#include <QMediaControl>
#include <QMediaVideoControl>
#include <QMediaVideoControlServer>
#include <QVideoSurface>
#include <qtopiavideo.h>

#include <private/qmediahandle_p.h>

#include "helixplayer.h"
#include "qmediahelixsettingsserver.h"

#include "helixsession.h"

/*!
    \class qtopia_helix::HelixSession
    \internal
*/

namespace qtopia_helix
{


// {{{ HelixSessionPrivate
class HelixSessionPrivate
{
public:
    QUuid                       id;
    QString                     url;
    QString                     domain;
    QStringList                 interfaces;
    IHXClientEngine*            engine;
    HelixPlayer*                player;
    QMediaVideoControlServer*   videoControlServer;
    VideoWidget*                videoWidget;
    bool                        ismute;
    bool                        suspended;
    quint32                     position;
    quint32                     length;
    QtopiaMedia::State          state;
    QtopiaMedia::State          preSuspendState;
};
// }}}

// {{{ HelixSession
HelixSession::HelixSession(IHXClientEngine* engine, QUuid const& id, QString const& url):
    d(new HelixSessionPrivate)
{
    d->id = id;
    d->url = url;
    d->interfaces << QMediaControl::name();
    d->engine = engine;
    d->videoControlServer = 0;
    d->videoWidget = 0;
    d->ismute = false;
    d->suspended = false;
    d->position = 0;
    d->length = 0;
    d->state = QtopiaMedia::Stopped;
    d->preSuspendState = QtopiaMedia::Stopped;

    startupPlayer();
}

HelixSession::~HelixSession()
{
    stop();

    shutdownPlayer();

    delete d->videoControlServer;
    delete d->videoWidget;
}

void HelixSession::start()
{
    if (d->player->playerState() == QtopiaMedia::Stopped) {
        d->player->open(d->url);
    }

    d->player->play();
}

void HelixSession::pause()
{
    d->player->pause();
}

void HelixSession::stop()
{
    d->player->stop();
}

void HelixSession::suspend()
{
    if ( !d->suspended ) {
        d->suspended = true;
        d->preSuspendState = d->state;
        if (d->preSuspendState != QtopiaMedia::Stopped)
            pause();
    }
}

void HelixSession::resume()
{
    if ( d->suspended ) {
        d->suspended = false;
        if (d->preSuspendState == QtopiaMedia::Playing)
            start();
    }
}

bool HelixSession::isSuspended() const
{
    return d->suspended;
}

void HelixSession::seek(quint32 ms)
{
    d->player->seek(ms);
}

quint32 HelixSession::length()
{
    return d->player->length();
}

void HelixSession::setVolume(int volume)
{
    d->player->setVolume(volume);
}

int HelixSession::volume() const
{
    return d->player->volume();
}

void HelixSession::setMuted(bool mute)
{
    d->player->setMuted(mute);
}

bool HelixSession::isMuted() const
{
    return d->ismute;
}

QtopiaMedia::State HelixSession::playerState() const
{
    return d->state;
}

QString HelixSession::errorString()
{
    return d->player->errorString();
}

void HelixSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString HelixSession::domain() const
{
    return d->domain;
}

QStringList HelixSession::interfaces()
{
    return d->interfaces;
}

QString HelixSession::id() const
{
    return d->id;
}

QString HelixSession::reportData() const
{
    QStringList sl;

    sl << "engine:Helix" << ("uri:" + d->url);

    return sl.join(",");
}

// {{{ Observer
void HelixSession::update(Subject* subject)
{
    static const int MS_PER_S = 1000; // 1000 milliseconds per second

    PlayerState*    playerState;
    VideoRender*    videoRender;
    ErrorReport*    errorReport;

    if ((playerState = qobject_cast<PlayerState*>(d->player)) == subject)
    {
        QtopiaMedia::State state = playerState->playerState();

        if (d->state != state)
        {
            d->state = state;
            if (!d->suspended)
                emit playerStateChanged(d->state);
        }
    }
    else
    if (qobject_cast<PlaybackStatus*>(d->player) == subject)
    {
        quint32 position = (d->player->position() / MS_PER_S) * MS_PER_S;
        if( d->position != position ) {
            d->position = position;
            emit positionChanged(d->position);
        }

        quint32 length = d->player->length();
        if( d->length != length ) {
            d->length = length;
            emit lengthChanged(d->length);
        }
    }
    else
    if ((qobject_cast<VolumeControl*>(d->player)) == subject)
    {
        emit volumeChanged(d->player->volume());

        bool mute = d->player->isMuted();
        if( d->ismute != mute ) {
            d->ismute = mute;

            emit volumeMuted(d->ismute);
        }
    }
    else
    if ((videoRender = qobject_cast<VideoRender*>(d->player)) == subject)
    {
        if (videoRender->hasVideo())
        {
            d->videoWidget = videoRender->createVideoWidget();
            connect( d->videoWidget->videoSurface(), SIGNAL(formatsChanged()),
                     d->player, SLOT(updateVideoSurfaceFormats()) );
            connect( d->videoWidget->videoSurface(), SIGNAL(updateRequested()),
                     this, SLOT(repaintLastFrame()) );

            d->videoControlServer = new QMediaVideoControlServer( QMediaHandle(d->id) );
            d->videoControlServer->setRenderTarget(d->videoWidget->winId());
            QVideoSurface *surface = d->videoWidget->videoSurface();
            surface->setRotation(d->videoControlServer->videoRotation());
            surface->setScaleMode(d->videoControlServer->videoScaleMode());
            connect(d->videoControlServer, SIGNAL(rotationChanged(QtopiaVideo::VideoRotation)),
                    surface, SLOT(setRotation(QtopiaVideo::VideoRotation)));
            connect(d->videoControlServer, SIGNAL(scaleModeChanged(QtopiaVideo::VideoScaleMode)),
                    surface, SLOT(setScaleMode(QtopiaVideo::VideoScaleMode)));

            d->interfaces.append(QMediaVideoControl::name());

            emit interfaceAvailable(QMediaVideoControl::name());
        }
        else
        {
            d->interfaces.removeAll(QMediaVideoControl::name());
            emit interfaceUnavailable(QMediaVideoControl::name());

            delete d->videoWidget;
            d->videoWidget = 0;

            delete d->videoControlServer;
            d->videoControlServer = 0;
        }
    }
    else
    if ((errorReport = qobject_cast<ErrorReport*>(d->player)) == subject)
    {
        emit playerStateChanged(QtopiaMedia::Error);
    }
}
// }}}

void HelixSession::repaintLastFrame()
{
    if (  playerState() != QtopiaMedia::Playing && d->videoWidget )
        d->videoWidget->repaintLastFrame();
}

void HelixSession::startupPlayer()
{
    d->player = new HelixPlayer(d->engine);

    PlayerState *playerState = qobject_cast<PlayerState*>(d->player);
    if (playerState != 0)
        playerState->attach(this);

    ErrorReport *error = qobject_cast<ErrorReport*>(d->player);
    if (error != 0)
        error->attach(this);

    PlaybackStatus *status = qobject_cast<PlaybackStatus*>(d->player);
    if (status != 0)
        status->attach(this);

    VolumeControl *volume = qobject_cast<VolumeControl*>(d->player);
    if (volume != 0)
        volume->attach(this);

    VideoRender *videoRender = qobject_cast<VideoRender*>(d->player);
    if (videoRender != 0)
        videoRender->attach(this);
}

void HelixSession::shutdownPlayer()
{
    PlayerState *playerState = qobject_cast<PlayerState*>(d->player);
    if (playerState != 0)
        playerState->detach(this);

    ErrorReport *error = qobject_cast<ErrorReport*>(d->player);
    if (error != 0)
        error->detach(this);

    PlaybackStatus *status = qobject_cast<PlaybackStatus*>(d->player);
    if (status != 0)
        status->detach(this);

    VolumeControl *volume = qobject_cast<VolumeControl*>(d->player);
    if (volume != 0)
        volume->detach(this);

    VideoRender *videoRender = qobject_cast<VideoRender*>(d->player);
    if (videoRender != 0)
        videoRender->detach(this);

    delete d->player;
}
// }}}


}   // ns qtopia_helix

