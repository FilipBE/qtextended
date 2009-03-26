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

#include "qmediaabstractcontrol.h"
#include "private/mediaserverproxy_p.h"
#include "qmediacontentplayer_p.h"
#include "qmediahandle_p.h"

#include "qmediacontent.h"

using namespace mlp;


// {{{ MediaSession
class MediaSession : public QMediaAbstractControl
{
    Q_OBJECT

public:
    MediaSession(QMediaContent* content);

    QStringList controls() const;

signals:
    void controlAvailable(const QString& name);
    void controlUnavailable(const QString& name);
};

MediaSession::MediaSession(QMediaContent* content):
    QMediaAbstractControl(content, "Session")
{
    proxyAll();
}

QStringList MediaSession::controls() const
{
    return value("controls").toStringList();
}
// }}}


// {{{ QMediaContentPrivate
class QMediaContentPrivate : public MediaServerCallback
{
public:
    QMediaHandle        handle;
    QMediaContent*      content;
    QString             domain;
    MediaSession*       session;

    void mediaReady()
    {
        session = new MediaSession(content);

        foreach (QString const& controlName, session->controls())
        {
            content->controlAvailable(controlName);
        }

        QObject::connect(session, SIGNAL(controlAvailable(QString)),
                         content, SIGNAL(controlAvailable(QString)));

        QObject::connect(session, SIGNAL(controlUnavailable(QString)),
                         content, SIGNAL(controlUnavailable(QString)));
    }

    void mediaError(QString const& errorString)
    {
        content->mediaError(errorString);
    }
};
// }}}


/*!
    \class QMediaContent
    \inpublicgroup QtMediaModule

    \brief The QMediaContent class is used to prepare a media resource
    for playing in Qtopia.

    A QMediaContent is constructed with a URL, that may refer to a local or
    remote resource, or a QContent that refers to a local media resource.

    If the QMediaContent is constructed with a valid piece of media a signal
    will be emitted with for each of the controls available to manipulate that
    media.

    During the life-time of a piece of media, the controls available may
    change, due to different formats being available in the content, each time
    a control is usable or unusable a controlAvailable or controlUnavailable
    signal will be emitted.

    If there is an error preparing the media resource, the mediaError signal will
    be emitted, with a QString detailing the error information.

    \code
    Example:
    {
        QContent        beep = findBeepSound();

        m_mediaContent = new QMediaContent(beep);

        connect(mediaContent, SIGNAL(controlAvailable(QString)),
                this, SLOT(mediaControlAvailable(QString)));
    }

    void mediaControlAvailable(QString const& id)
    {
        if (id == QMediaControl::name())
        {
            // The basic media control is available, all media content
            // will have this control available.
            m_mediaControl = new QMediaControl(m_mediaContent);

            // Now do something play/get length
            m_mediaControl->start();
        }
    }
    \endcode

    \ingroup multimedia
*/

/*!
    Create a QMediaContent from a URL.

    The URL may point to a local or remote media resource.

    The \a url paramter is a URL representing the location of the media content.
    The \a domain is the audio domain in which this media content should exist.
    The \a parent is the Parent QObject.
*/

QMediaContent::QMediaContent
(
 QUrl const&    url,
 QString const& domain,
 QObject*       parent
):
    QObject(parent),
    d(new QMediaContentPrivate)
{
    d->content = this;
    d->domain = domain;
    d->session = 0;

    d->handle = MediaServerProxy::instance()->prepareContent(d, d->domain, url);
}

/*!
    Create a QMediaContent from a QContent.

    This creates a QMediaContent from a local resource known by the
    Document System.

    The \a content is the QContent representing the location of the media content.
    The \a domain is the audio domain in which this media content should exist.
    The \a parent is the Parent QObject.
*/

QMediaContent::QMediaContent
(
 QContent const& content,
 QString const& domain,
 QObject* parent
):
    QObject(parent),
    d(new QMediaContentPrivate)
{
    d->content = this;
    d->domain = domain;
    d->session = 0;

    QString url = content.fileName();

    if (content.drmState() == QContent::Protected)
        url.prepend("qtopia://");
    else
        url.prepend("file://");

    d->handle = MediaServerProxy::instance()->prepareContent(d, d->domain, url);
}

/*!
    Destroy the QMediaContent object.
*/

QMediaContent::~QMediaContent()
{
    MediaServerProxy::instance()->destroySession(d->handle);
    delete d;
}


/*!
    \fn QMediaContent::handle() const;
    Return a handle that can be used to construct a control
    to interact with the media resource.
*/

QMediaHandle QMediaContent::handle() const
{
    return d->handle;
}

/*!
    \fn QMediaContent::controls() const;
    Return a list of Controls that this Media supports.
*/

QStringList QMediaContent::controls() const
{
    if (d->session != 0)
        return d->session->controls();

    return QStringList();
}

/*!
    \fn QMediaContent::controlAvailable(const QString& id);

    Signals that a control with identity \a id is now available for use.
*/

/*!
    \fn QMediaContent::controlUnavailable(const QString& id);

    Signals that a control with identity \a id is no longer usable.
*/

/*!
    \fn QMediaContent::mediaError(const QString& mediaError);

    Signals that there was an error trying to access a piece of media content.
    Details of the error are contained in the \a mediaError parameter.
*/


/*!
    Returns a list of Mime Types handled by the Qt Extended Media system.
*/

QStringList QMediaContent::supportedMimeTypes()
{
    QStringList         rc;
    MediaServerProxy*   proxy = MediaServerProxy::instance();

    foreach (QString const& engine, proxy->simpleProviderTags())
    {
        rc += proxy->simpleMimeTypesForProvider(engine);
    }

    return rc;
}

/*!
    Returns a list of URI schemes that can be used in combination with the \a
    mimeType.
*/

QStringList QMediaContent::supportedUriSchemes(QString const& mimeType)
{
    QStringList         rc;
    MediaServerProxy*   proxy = MediaServerProxy::instance();

    foreach (QString const& engine, proxy->simpleProviderTags())
    {
        if (proxy->simpleMimeTypesForProvider(engine).contains(mimeType))
        {
            rc += proxy->simpleUriSchemesForProvider(engine);
        }
    }

    return rc;
}

/*!
    Play immediately, and to the end, the content specified by \a url in the
    media \a domain.
*/

void QMediaContent::playContent(QUrl const& url, QString const& domain)
{
    new QMediaContentPlayer(url, domain);
}

/*!
    Play immediately, and to the end, the content specified by \a content in
    the media \a domain.
*/

void QMediaContent::playContent(QContent const& content, QString const& domain)
{
    new QMediaContentPlayer(content, domain);
}

#include "qmediacontent.moc"

