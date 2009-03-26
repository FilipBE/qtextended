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

#include <QValueSpaceItem>
#include <QVideoSurface>
#include <QDebug>

#include "qmediahandle_p.h"
#include "qmediaabstractcontrolserver.h"
#include "qmediavideocontrolserver.h"
#include "qtopiamedia/media.h"

/*!
    \class QMediaVideoControlServer
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaVideoControlServer class is used by Media Engines to
            inform clients that video is available for the associated media session.

    A Media Engine using the QtopiaMedia API constructs a
    QMediaVideoControlServer when it would like to inform and enable a client
    to access a QWidget based video widget in the server.

    \code
    {
        VideoWidget* videoWidget = new VideoWidget(videoSource, parent);

        QMediaVideoControlServer* videoControlServer = new QMediaVideoControlServer(id, parent);

        videoControlServer->setRenderTarget(videoWidget);
    }
    \endcode

    \sa QMediaVideoControl, QMediaAbstractControlServer

    \ingroup multimedia
*/

class QMediaVideoControlServerPrivate: public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    QMediaVideoControlServerPrivate(QMediaHandle const& handle);
    ~QMediaVideoControlServerPrivate();

public:
    void setRenderTarget(WId wid);

signals:
    void videoTargetAvailable();
    void videoTargetRemoved();

public:
    QValueSpaceItem* videoRotation;
    QValueSpaceItem* videoScaleMode;
};

QMediaVideoControlServerPrivate::QMediaVideoControlServerPrivate(QMediaHandle const& handle):
    QMediaAbstractControlServer(handle, "Video")
{
    proxyAll();

    setValue("rotation", QVariant(int(QtopiaVideo::Rotate0)));
    setValue("scaleMode", QVariant(int(QtopiaVideo::FitWindow)));

    videoRotation = new QValueSpaceItem("/Media/Control/" + handle.toString() + "/Video/rotation", this);
    videoScaleMode = new QValueSpaceItem( "/Media/Control/" + handle.toString() + "/Video/scaleMode", this);
}

QMediaVideoControlServerPrivate::~QMediaVideoControlServerPrivate()
{
}

void QMediaVideoControlServerPrivate::setRenderTarget(WId wid)
{
    if (wid == WId()) {
        setValue("hasVideo", false);
        setValue("windowId", quint32(0));

        emit videoTargetRemoved();
    }
    else {
        setValue("hasVideo", true);
        setValue("windowId", quint32(wid));

        emit videoTargetAvailable();
    }
}


/*!
    Constructs a QMediaVideoControlServer with the session \a handle, and
    optionally the video \a target widget for sharing with the client and the
    \a parent QObject.
*/

QMediaVideoControlServer::QMediaVideoControlServer
(
 QMediaHandle const& handle,
 QWidget    *target,
 QObject    *parent
):
    QObject(parent),
    d(new QMediaVideoControlServerPrivate(handle))
{
    setRenderTarget(target);

    connect(d->videoRotation, SIGNAL(contentsChanged()), SLOT(updateVideoRotation()));
    connect(d->videoScaleMode, SIGNAL(contentsChanged()), SLOT(updateVideoScaleMode()));
}

/*!
    Destroy the QMediaVideoControlServer object.
*/

QMediaVideoControlServer::~QMediaVideoControlServer()
{
    delete d;
}

/*!
    Set the video target widget to \a renderTarget.

    The Media Engine calls this function when it has a widget that it would
    like to make available to a client application.

    The QWidget will exist in the Media Server process, but will be made
    available to the session client.  A session client will be able to control
    different widget states, for example whether the widget is hidden or visible.
*/

void QMediaVideoControlServer::setRenderTarget(QWidget* renderTarget)
{
    setRenderTarget(renderTarget == 0 ? WId() : renderTarget->winId());
}

/*!
    Set the video target to the window with \a wid.

    The Media Engine calls this function when it has a widget that it would
    like to make available to a client application.

    The QWidget will exist in the Media Server process, but will be made
    available to the session client.  A session client will be able to control
    different widget states, for example whether the widget is hidden or visible.
*/

void QMediaVideoControlServer::setRenderTarget(WId wid)
{
    d->setRenderTarget(wid);
}

/*!
    Remove the video target widget.

    The Media Engine should call this function when it will no longer be rendering video
    into the QWidget set in setRenderTarget(). The Session client will be notified that the
    widget is no longer valid.
*/
void QMediaVideoControlServer::unsetRenderTarget()
{
    d->setRenderTarget(WId());
}

/*!
    Return the current video rotation value.
*/
QtopiaVideo::VideoRotation QMediaVideoControlServer::videoRotation() const
{
    return QtopiaVideo::VideoRotation(d->videoRotation->value().toInt());
}

/*!
    Return the current video scale mode value.
*/
QtopiaVideo::VideoScaleMode QMediaVideoControlServer::videoScaleMode() const
{
    return QtopiaVideo::VideoScaleMode(d->videoScaleMode->value().toInt());
}


void QMediaVideoControlServer::updateVideoRotation()
{
    emit rotationChanged(videoRotation());
}

void QMediaVideoControlServer::updateVideoScaleMode()
{
    emit scaleModeChanged(videoScaleMode());
}


/*!
    \fn void QMediaVideoControlServer::rotationChanged(QtopiaVideo::VideoRotation rotation);

    This signal is emitted when video rotation value was changed, the value is
    given by \a rotation.

    \sa videoRotation()
    \sa QtopiaVideo::VideoRotation
*/

/*!
    \fn void QMediaVideoControlServer::scaleModeChanged(QtopiaVideo::VideoScaleMode scaleMode);

    This signal is emitted when video scale mode was changed, the value is given by
    \a scaleMode.

    \sa videoScaleMode()
    \sa QtopiaVideo::VideoScaleMode

*/


#include "qmediavideocontrolserver.moc"
