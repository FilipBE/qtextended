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

#include <QtopiaApplication>
#include <QDebug>


#include "requesthandler.h"
#include "servicerequest.h"

#include "playmediaservice.h"


/*!
    \service PlayMediaService PlayMedia
    \inpublicgroup QtMediaModule
    \brief The PlayMediaService class provides the \i Playmedia service which allows 
    access to media playback facilities.

    This service plays an arbitrary media file as long as the format is supported 
    by the underlying media system.
*/

/*!
    \internal
*/
PlayMediaService::PlayMediaService(RequestHandler* handler, QObject* parent):
    QtopiaAbstractService("PlayMedia", parent),
    m_requestHandler(handler)
{
    publishAll();
}

/*!
    Plays the media file at \a url.
*/
void PlayMediaService::openURL(QString const& url)
{
    // We aren't shown when application was invoked for service request.
    QtopiaApplication::instance()->mainWidget()->showMaximized();

    m_requestHandler->execute(new OpenUrlRequest(url, true));
}


