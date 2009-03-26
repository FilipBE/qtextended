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

#include <QMap>
#include <QUrl>
#include <QtopiaIpcEnvelope>
#include <qtopialog.h>
#include <media.h>

#include <QMediaServerSession>

#include "mediacontrolserver.h"
#include "mediacontentserver.h"
#include "sessionmanager.h"

#include <private/qmediahandle_p.h>

#include "qtopiamediaprovider.h"


namespace mediaserver
{

struct Session
{
    Session(MediaContentServer* contentServer,
            MediaControlServer* controlServer):
        content(contentServer),
        control(controlServer)
    {
    }

    MediaContentServer* content;
    MediaControlServer* control;
};

typedef QMap<QUuid, Session>   SessionMap;


class QtopiaMediaProviderPrivate
{
public:
    SessionManager*   sessionManager;
    SessionMap        sessionMap;
};

/*!
    \class mediaserver::QtopiaMediaProvider
    \internal
*/

QtopiaMediaProvider::QtopiaMediaProvider(SessionManager* sessionManager):
    QtopiaAbstractService(QTOPIA_MEDIASERVER_CHANNEL),
    d(new QtopiaMediaProviderPrivate)
{
    qLog(Media) << "QtopiaMediaProvider starting";

    d->sessionManager = sessionManager;

    publishAll();
}

QtopiaMediaProvider::~QtopiaMediaProvider()
{
    delete d;
}

void QtopiaMediaProvider::createSession
(
 QString const& responseChannel,
 QMediaSessionRequest const& request
)
{
    qLog(Media) << "QtopiaMediaProvider::createSession()" << request.id() << request.domain() << request.type();

    QMediaServerSession*    mediaSession = d->sessionManager->createSession(request);

    if (mediaSession != 0)
    {
        // Wrap in Media API content & control objects
        MediaContentServer* contentServer = new MediaContentServer(mediaSession,
                                                                   QMediaHandle(request.id()));
        MediaControlServer* controlServer = new MediaControlServer(mediaSession,
                                                                   QMediaHandle(request.id()));

        d->sessionMap.insert(request.id(), Session(contentServer, controlServer));

        QtopiaIpcEnvelope   e(responseChannel, "sessionCreated(QUuid)");
        e << request.id();
    }
    else
    {
        QtopiaIpcEnvelope   e(responseChannel, "sessionError(QUuid,QString)");
        e << request.id() << tr("The media system is not configured to handle this request");
    }
}

void QtopiaMediaProvider::destroySession(QUuid const& id)
{
    qLog(Media) << "QtopiaMediaProvider::destroySession()" << id;

    SessionMap::iterator it = d->sessionMap.find(id);

    if (it != d->sessionMap.end())
    {
        Session&    session = *it;

        d->sessionManager->destroySession(session.control->mediaSession());

        delete session.content;
        delete session.control;

        d->sessionMap.erase(it);
    }
}

}   // ns mediaserver

