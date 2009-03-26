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

#include <QMediaServerSession>

#include <private/qmediahandle_p.h>

#include "mediacontentserver.h"


namespace mediaserver
{

class MediaContentServerPrivate
{
public:
    QMediaServerSession*    session;
};

/*!
    \class mediaserver::MediaContentServer
    \internal
*/

MediaContentServer::MediaContentServer
(
 QMediaServerSession* session,
 QMediaHandle const& handle
):
    QMediaAbstractControlServer(handle, "Session"),
    d(new MediaContentServerPrivate)
{
    d->session = session;

    setValue("controls", d->session->interfaces() << "Session");

    connect(d->session, SIGNAL(interfaceAvailable(QString)),
            this, SLOT(interfaceAvailable(QString)));

    connect(d->session, SIGNAL(interfaceUnavailable(QString)),
            this, SLOT(interfaceUnavailable(QString)));

    proxyAll();
}

MediaContentServer::~MediaContentServer()
{
    delete d;
}

//private slots:
void MediaContentServer::interfaceAvailable(const QString& name)
{
    setValue("controls", d->session->interfaces() << "Session");

    emit controlAvailable(name);
}

void MediaContentServer::interfaceUnavailable(const QString& name)
{
    setValue("controls", d->session->interfaces() << "Session");

    emit controlUnavailable(name);
}


}   // ns mediaserver
