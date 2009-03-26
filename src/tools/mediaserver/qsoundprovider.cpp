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

#include <QUrl>

#include <qcontent.h>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif
#include <qsoundcontrol.h>
#include <qtopialog.h>
#include <qmediaserversession.h>
#include <qmediasessionrequest.h>

#include "sessionmanager.h"
#include "domainmanager.h"

#include "qsoundprovider.h"


namespace mediaserver
{

/*!
    \class mediaserver::QSoundPlayer
    \internal
*/

// {{{ QSoundPlayer
QSoundPlayer::QSoundPlayer(QUuid const& id, SessionManager* sessionManager):
    m_id(id),
    m_domain("Media"),
    m_sessionManager(sessionManager),
    m_mediaSession(NULL)
{
}

QSoundPlayer::~QSoundPlayer()
{
    if (m_mediaSession)
        m_sessionManager->destroySession(m_mediaSession);
}

void QSoundPlayer::open(QString const& filePath)
{
    QUrl                    url;
    QContent                content(filePath, false);
    QMediaSessionRequest    sessionRequest(m_domain, "com.trolltech.qtopia.uri");

    // Make a URL
    if (content.drmState() == QContent::Protected)
        url.setScheme("qtopia");
    else
        url.setScheme("file");

    url.setPath(filePath);

    sessionRequest << url;

    m_mediaSession = m_sessionManager->createSession(sessionRequest);

    if (m_mediaSession)
    {
        connect(m_mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
                this, SLOT(playerStateChanged(QtopiaMedia::State)));
    }
}

void QSoundPlayer::setVolume(int volume)
{
    if (m_mediaSession)
        m_mediaSession->setVolume(volume);
}

void QSoundPlayer::setPriority(int priority)
{
    switch (priority)
    {
    case QSoundControl::Default:
        m_domain = "Media";
        break;

    case QSoundControl::RingTone:
        m_domain = "RingTone";
        break;
    }

    if (m_mediaSession)
        m_mediaSession->setDomain(m_domain);
}

void QSoundPlayer::play()
{
    if (m_mediaSession)
        m_mediaSession->start();
    else
        playerStateChanged(QtopiaMedia::Error);
}

void QSoundPlayer::stop()
{
    if (m_mediaSession)
        m_mediaSession->stop();
    else
        playerStateChanged(QtopiaMedia::Stopped);
}

void QSoundPlayer::playerStateChanged(QtopiaMedia::State state)
{
    switch (state)
    {
    case QtopiaMedia::Stopped:
    case QtopiaMedia::Error:
        QCopChannel::send(QString("QPE/QSound/").append(m_id), "done()");
        break;

    default:
        ;
    }
}
// }}}

/*!
    \class mediaserver::QSoundProvider
    \internal
*/

// {{{ QSoundProvider
QSoundProvider::QSoundProvider(SessionManager* sessionManager):
    QtopiaIpcAdaptor(QT_MEDIASERVER_CHANNEL),
    m_sessionManager(sessionManager)
{
    qLog(Media) << "QSoundProvider starting";

    publishAll(Slots);
}

QSoundProvider::~QSoundProvider()
{
}

void QSoundProvider::setPriority(int priority)
{
    switch (priority)
    {
    case QSoundControl::Default:
        DomainManager::instance()->deactivateDomain("RingTone");
        break;

    case QSoundControl::RingTone:
        DomainManager::instance()->activateDomain("RingTone");
        break;
    }
}

void QSoundProvider::subscribe(const QUuid& id)
{
     m_playerMap.insert(id, new QSoundPlayer(id, SessionManager::instance()));
}

void QSoundProvider::open(const QUuid& id, const QString& url)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->open(url);
}

void QSoundProvider::setVolume(const QUuid& id, int volume)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->setVolume(volume);
}

void QSoundProvider::setPriority(const QUuid& id, int priority)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->setPriority(priority);
}

void QSoundProvider::play(const QUuid& id)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->play();
}

void QSoundProvider::stop(const QUuid& id)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->stop();
}

void QSoundProvider::revoke(const QUuid& id)
{
    PlayerMap::iterator it = m_playerMap.find(id);

    if (it != m_playerMap.end())
    {
        (*it)->stop();

        delete *it;

        m_playerMap.erase(it);
    }
}

QSoundPlayer* QSoundProvider::player(QUuid const& id)
{
    PlayerMap::iterator it = m_playerMap.find(id);

    return it == m_playerMap.end() ? 0 : *it;
}
// }}}

}   // ns mediaserver
