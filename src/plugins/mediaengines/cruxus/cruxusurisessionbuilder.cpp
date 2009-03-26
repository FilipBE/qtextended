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
#include <QMimeType>
#include <QMediaCodecPlugin>
#include <QMediaSessionRequest>
#include <QMediaDecoder>

#include "contentdevice.h"
#include "cruxusengine.h"
#include "cruxussimplesession.h"
#include "cruxusurihandlers.h"
#include "cruxusoutputdevices.h"

#include "cruxusurisessionbuilder.h"

namespace cruxus
{

// {{{ UriSessionBuilderPrivate
class UriSessionBuilderPrivate
{
public:
    Engine*                             engine;
    QMediaSessionBuilder::Attributes    attributes;
    MediaCodecPluginMap                 mimeTypeMapping;
};
// }}}

// {{{ UriSessionBuilder

/*!
    \class cruxus::UriSessionBuilder
    \internal
*/

UriSessionBuilder::UriSessionBuilder
(
 Engine*            engine,
 MediaCodecPluginMap const& mimeTypeMapping
):
    QObject(engine),
    d(new UriSessionBuilderPrivate)
{
    d->engine = engine;
    d->mimeTypeMapping = mimeTypeMapping;

    // Supported URI Schems
    d->attributes.insert("uriSchemes", UriHandlers::supportedUris());

    // Supported mime types
    d->attributes.insert("mimeTypes", QStringList(mimeTypeMapping.keys()));
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
    return d->attributes;
}

QMediaServerSession* UriSessionBuilder::createSession(QMediaSessionRequest sessionRequest)
{
    QMediaServerSession*    session = 0;
    QUrl                    url;

    sessionRequest >> url;

    if (url.isValid()) {
        QString     path = url.path();
        QMimeType   mimeType(path);

        if (!mimeType.isNull()) {
            MediaCodecPluginMap::iterator it = d->mimeTypeMapping.find(mimeType.id());

            // Check for a plugin
            if (it != d->mimeTypeMapping.end()) {
                QMediaDevice*   inputDevice  = UriHandlers::createInputDevice(url.scheme(), path);
                QMediaDevice*   outputDevice = OutputDevices::createOutputDevice();

                if (inputDevice != 0 && outputDevice != 0) {
                    QMediaCodecPlugin*  plugin = it.value();
                    QMediaDecoder*      coder = plugin->decoder(mimeType.id());

                    session = new SimpleSession(QMediaHandle(sessionRequest.id()),
                                                inputDevice,
                                                coder,
                                                outputDevice);

                    d->engine->registerSession(session);
                }
                else {
                    UriHandlers::destroyInputDevice(inputDevice);
                    OutputDevices::destroyOutputDevice(outputDevice);
                }
            }
        }
    }

    return session;
}

void UriSessionBuilder::destroySession(QMediaServerSession* session)
{
    d->engine->unregisterSession(session);
    session->stop();
    delete session;
}
// }}}

} // ns cruxus
