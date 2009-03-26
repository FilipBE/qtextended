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
#include <QSettings>
#include <QMediaSessionRequest>
#include <QDebug>

#include "ffmpegengine.h"
#include "ffmpegplaybinsession.h"
#include "ffmpegurisessionbuilder.h"


namespace ffmpeg
{

// {{{ UriSessionBuilderPrivate
class UriSessionBuilderPrivate
{
public:
    Engine*                             engine;
    QMediaSessionBuilder::Attributes    attributes;
    QMediaDevice*                       outputDevice;
};
// }}}


// {{{ UriSessionBuilder

/*!
    \class ffmpeg::UriSessionBuilder
    \internal
*/

UriSessionBuilder::UriSessionBuilder(Engine* engine):
    d(new UriSessionBuilderPrivate)
{
    d->engine = engine;

    // ffmpeg does not support dynamically discovering the mime types and
    // uri schemes.  Grab them from QSettings
    QSettings   settings("Trolltech", "ffmpeg");
    settings.beginGroup("Simple");

    // Supported URI Schemes
    d->attributes.insert("uriSchemes", settings.value("UriSchemes").toStringList());

    // Supported mime types
    d->attributes.insert("mimeTypes", settings.value("MimeTypes").toStringList());
    d->outputDevice = 0;
}

UriSessionBuilder::~UriSessionBuilder()
{
    delete d;
}


QString UriSessionBuilder::type() const
{
    return "com.trolltech.qtopia.uri";
}

QMediaSessionBuilder::Attributes const& UriSessionBuilder::attributes() const
{
    qWarning()<<d->attributes;
    return d->attributes;
}

QMediaServerSession* UriSessionBuilder::createSession(QMediaSessionRequest sessionRequest)
{
    QUrl            url;
    PlaybinSession* mediaSession = 0;

    sessionRequest >> url;

    if (url.isValid())
    {
        mediaSession = new PlaybinSession(d->engine,
                                          sessionRequest.id(),
                                          url,d->outputDevice);

        if (!mediaSession->isValid())
        {
            delete mediaSession;
            mediaSession = 0;
        } else {
            QMediaServerSession* session;
            session = static_cast<QMediaServerSession*>(mediaSession);
            d->engine->registerSession(session);
        }
    }

    return mediaSession;
}

void UriSessionBuilder::destroySession(QMediaServerSession* mediaSession)
{
    delete mediaSession;
}
// }}}

}   // ns ffmpeg

