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

#include <qtopialog.h>

#include <qmediaserversession.h>

#include "mediacontrolserver.h"


namespace mediaserver
{

/*!
    \class mediaserver::MediaControlServer
    \internal
*/

MediaControlServer::MediaControlServer
(
 QMediaServerSession* mediaSession,
 QMediaHandle const& handle
):
    QMediaAbstractControlServer(handle, "Basic"),
    m_mediaSession(mediaSession)
{
    // connect
    connect(m_mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(stateChanged(QtopiaMedia::State)));

    connect(m_mediaSession, SIGNAL(positionChanged(quint32)),
            this, SLOT(posChanged(quint32)));

    connect(m_mediaSession, SIGNAL(lengthChanged(quint32)),
            this, SLOT(lenChanged(quint32)));

    connect(m_mediaSession, SIGNAL(volumeChanged(int)),
            this, SLOT(volChanged(int)));

    connect(m_mediaSession, SIGNAL(volumeMuted(bool)),
            this, SLOT(volMuted(bool)));

    // set initial values
    setValue(QLatin1String("playerState"), QtopiaMedia::Stopped);
    setValue(QLatin1String("length"), m_mediaSession->length());
    setValue(QLatin1String("volume"), m_mediaSession->volume());
    setValue(QLatin1String("muted"), m_mediaSession->isMuted());
    setValue(QLatin1String("errorString"), QString());

    proxyAll();
}

MediaControlServer::~MediaControlServer()
{
}

QMediaServerSession* MediaControlServer::mediaSession() const
{
    return m_mediaSession;
}

// public slots:
void MediaControlServer::start()
{
    m_mediaSession->start();
}

void MediaControlServer::pause()
{
    m_mediaSession->pause();
}

void MediaControlServer::stop()
{
    m_mediaSession->stop();
}

void MediaControlServer::seek(quint32 ms)
{
    m_mediaSession->seek(ms);
}

void MediaControlServer::setVolume(int volume)
{
    m_mediaSession->setVolume(volume);
}

void MediaControlServer::setMuted(bool mute)
{
    m_mediaSession->setMuted(mute);
}

void MediaControlServer::stateChanged(QtopiaMedia::State state)
{
    setValue("playerState", state);

    if (state == QtopiaMedia::Error)
    {
        setValue("errorString", m_mediaSession->errorString());
    }

    emit playerStateChanged(state);
}

void MediaControlServer::posChanged(quint32 ms)
{
    setValue("position", ms);

    emit positionChanged(ms);
}

void MediaControlServer::lenChanged(quint32 ms)
{
    setValue("length", ms);

    emit lengthChanged(ms);
}

void MediaControlServer::volChanged(int volume)
{
    setValue("volume", volume);

    emit volumeChanged(volume);
}

void MediaControlServer::volMuted(bool muted)
{
    setValue("muted", muted);

    emit volumeMuted(muted);
}

}   // ns mediaserver


