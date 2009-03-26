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

#include <sys/types.h>
#include <unistd.h>

#include <QHash>

#include <QCoreApplication>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qvaluespace.h>
#include <qmediasessionrequest.h>

#include "media.h"
#include "qmediahandle_p.h"
#include "mediaserverproxy_p.h"


namespace mlp
{

/*!
    \class mlp::MediaServerCallback
    \internal
*/

// {{{ MediaServerCallback
MediaServerCallback::~MediaServerCallback()
{
}
// }}}


// {{{ MediaServerProxy
MediaServerProxy::~MediaServerProxy()
{
}

// {{ Info
QStringList MediaServerProxy::simpleProviderTags() const
{
    return m_simpleInfo->subPaths();
}

QStringList MediaServerProxy::simpleMimeTypesForProvider(QString const& providerTag)
{
    return m_simpleInfo->value(providerTag + "/mimeTypes").toStringList();
}

QStringList MediaServerProxy::simpleUriSchemesForProvider(QString const& providerTag)
{
    return m_simpleInfo->value(providerTag + "/uriSchemes").toStringList();
}
// }}}

// {{{ QMediaContent
QMediaHandle MediaServerProxy::prepareContent
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QUrl const&            url
)
{
    QMediaSessionRequest    request(domain, "com.trolltech.qtopia.uri");
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "createSession(QString,QMediaSessionRequest)");

    // Keep record
    m_callbackMap.insert(request.id(), callback);

    // Send request
    request << url;

    envelope << m_channel << request;
    envelope.send();

    return QMediaHandle(request.id());
}
// }}}

void MediaServerProxy::destroySession(QMediaHandle const& handle)
{
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "destroySession(QUuid)");

    envelope << handle.id();
    envelope.send();

    m_callbackMap.remove(handle.id());

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}


MediaServerProxy* MediaServerProxy::instance()
{
    static MediaServerProxy proxy;

    return &proxy;
}

// private slots:
void MediaServerProxy::sessionCreated(QUuid const& id)
{
    CallbackMap::iterator it = m_callbackMap.find(id);

    if (it != m_callbackMap.end())
        (*it)->mediaReady();
}

void MediaServerProxy::sessionError(QUuid const& id, QString const& error)
{
    CallbackMap::iterator it = m_callbackMap.find(id);

    if (it != m_callbackMap.end())
        (*it)->mediaError(error);
}

void MediaServerProxy::simpleInfoChanged()
{
    if (m_simpleInfo->subPaths().empty()) {
        // We'll take it no providers means something bad happened
        // Tell any of our sessions they are now in error.
        for (CallbackMap::iterator it = m_callbackMap.begin();
             it != m_callbackMap.end();
             ++it) {

            (*it)->mediaError(QString("Connection lost"));
        }
    }
}

MediaServerProxy::MediaServerProxy()
{
    // channel
    m_channel = QString(QTOPIA_MEDIALIBRARY_CHANNEL).arg(getpid());
    m_adaptor = new QtopiaIpcAdaptor(m_channel, this);

    QtopiaIpcAdaptor::connect(m_adaptor, MESSAGE(sessionCreated(QUuid)),
            this, SLOT(sessionCreated(QUuid)));

    QtopiaIpcAdaptor::connect(m_adaptor, MESSAGE(sessionError(QUuid,QString)),
            this, SLOT(sessionError(QUuid,QString)));

    // values
    m_simpleInfo = new QValueSpaceItem("/Media/Library/Info/Simple", this);
    connect(m_simpleInfo, SIGNAL(contentsChanged()), SLOT(simpleInfoChanged()));
}
// }}}

}   // ns mlp

