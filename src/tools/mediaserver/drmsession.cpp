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

#include <QDrmContent>

#include <qtopialog.h>

#include "drmsession.h"

/*!
    \class mediaserver::DrmSession
    \internal
*/


namespace mediaserver
{


class DrmSessionPrivate
{
public:
    QMediaServerSession*    mediaSession;
    QDrmContent             content;
    bool                    drmContent;
    bool                    haveLicense;
};


DrmSession::DrmSession(QUrl const& url, QMediaServerSession* mediaSession):
    d(new DrmSessionPrivate)
{
    // init
    d->mediaSession = mediaSession;
    d->haveLicense = false;

    if (url.scheme() == "qtopia")
    {
        QContent content(url.path());   // sizeof("qtopia://")

        if (content.isValid() &&
            content.drmState() == QContent::Protected)
        {
            d->drmContent = true;
            d->content.setPermission(QDrmRights::Play);
            d->haveLicense = d->content.requestLicense(content);

            connect(&d->content, SIGNAL(rightsExpired(QDrmContent)),
                     this, SLOT(licenseExpired()) );
            connect(d->mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(changeLicenseState(QtopiaMedia::State)));
        }
    }

    // XXX: Re-emit
    connect(d->mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SIGNAL(playerStateChanged(QtopiaMedia::State)));
    connect(d->mediaSession, SIGNAL(positionChanged(quint32)),
            this, SIGNAL(positionChanged(quint32)));
    connect(d->mediaSession, SIGNAL(lengthChanged(quint32)),
            this, SIGNAL(lengthChanged(quint32)));
    connect(d->mediaSession, SIGNAL(volumeChanged(int)),
            this, SIGNAL(volumeChanged(int)));
    connect(d->mediaSession, SIGNAL(volumeMuted(bool)),
            this, SIGNAL(volumeMuted(bool)));
    connect(d->mediaSession, SIGNAL(interfaceAvailable(QString)),
            this, SIGNAL(interfaceAvailable(QString)));
    connect(d->mediaSession, SIGNAL(interfaceUnavailable(QString)),
            this, SIGNAL(interfaceUnavailable(QString)));
}

DrmSession::~DrmSession()
{
    delete d->mediaSession;
    delete d;
}

void DrmSession::start()
{
    if (!d->drmContent && d->haveLicense)
    {
        d->mediaSession->start();
    }
    else if( d->content.requestLicense( d->content.content() ) )
    {
        d->haveLicense = true;

        d->mediaSession->start();
    }
}

void DrmSession::pause()
{
    d->mediaSession->pause();
}

void DrmSession::stop()
{
    d->mediaSession->stop();
}

void DrmSession::suspend()
{
    d->mediaSession->suspend();
}

void DrmSession::resume()
{
    d->mediaSession->resume();
}

void DrmSession::seek(quint32 ms)
{
    d->mediaSession->seek(ms);
}

quint32 DrmSession::length()
{
    return d->mediaSession->length();
}

void DrmSession::setVolume(int volume)
{
    d->mediaSession->setVolume(volume);
}

int DrmSession::volume() const
{
    return d->mediaSession->volume();
}

void DrmSession::setMuted(bool mute)
{
    d->mediaSession->setMuted(mute);
}

bool DrmSession::isMuted() const
{
    return d->mediaSession->isMuted();
}

QtopiaMedia::State DrmSession::playerState() const
{
    return d->mediaSession->playerState();
}

QString DrmSession::errorString()
{
    return d->mediaSession->errorString();
}

void DrmSession::setDomain(QString const& domain)
{
    d->mediaSession->setDomain(domain);
}

QString DrmSession::domain() const
{
    return d->mediaSession->domain();
}

QStringList DrmSession::interfaces()
{
    return d->mediaSession->interfaces();
}

QString DrmSession::id() const
{
    return d->mediaSession->id();
}

QString DrmSession::reportData() const
{
    return d->mediaSession->reportData();
}

void DrmSession::licenseExpired()
{
    stop();
    d->content.releaseLicense();
    d->haveLicense = false;
}

void DrmSession::changeLicenseState(QtopiaMedia::State state)
{
    if (d->haveLicense)
    {
        switch (state)
        {
        case QtopiaMedia::Playing:
            d->content.renderStarted();
            break;
        case QtopiaMedia::Paused:
        case QtopiaMedia::Buffering:
            d->content.renderPaused();
            break;
        case QtopiaMedia::Stopped:
        case QtopiaMedia::Error:
            d->content.renderStopped();
        }
    }
}


}   // ns mediaserver
