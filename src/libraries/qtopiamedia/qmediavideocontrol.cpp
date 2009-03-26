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

#include <QObject>

#if defined(Q_WS_QWS)
#include <QWSEmbedWidget>
#elif defined(Q_WS_X11)
#include <QX11EmbedContainer>
#endif

#include <QDebug>

#include "qmediaabstractcontrol.h"
#include "qmediacontent.h"

#include "qmediavideocontrol.h"



// {{{ QMediaVideoControlPrivate
class QMediaVideoControlPrivate : public QMediaAbstractControl
{
    Q_OBJECT

public:
    QMediaVideoControlPrivate(QMediaContent* mediaContent):
        QMediaAbstractControl(mediaContent, QMediaVideoControl::name())
    {
        proxyAll();
    }

    bool hasVideo() const
    {
        return value("hasVideo").toBool();
    }

    WId getWindowId() const
    {
        return value("windowId").toInt();        // XXX: knowledge
    }

public slots:
    void setVideoRotation( quint32 );
    void setVideoScaleMode( quint32 );

signals:
    void videoTargetAvailable();
    void videoTargetRemoved();
};

void QMediaVideoControlPrivate::setVideoRotation(quint32 rotation)
{
    if ( value("rotation").toInt() != (int)rotation )
        setValue("rotation", QVariant(rotation));
}

void QMediaVideoControlPrivate::setVideoScaleMode(quint32 scaleMode)
{
    setValue("scaleMode", QVariant(scaleMode));
}

// }}}


/*!
    \class QMediaVideoControl
    \inpublicgroup QtMediaModule

    \brief The QMediaVideoControl class is an interface to videos playing through
    the Qt Extended Media System.

    If a prepared media resource has associated video content, when
    that content is available to be played, this inteface can be used to contruct
    a QWidget which will contain the rendering of the video content.

    Use the \l QMediaControlNotifier class to listen for when video content is
    available.

    \code
        QMediaControlNotifier *notifier = new QMediaControlNotifier( QMediaVideoControl::name() );
        connect( notifier, SIGNAL(valid()), this, SLOT(showVideo()) );

        QMediaContent *content = new QMediaContent( "video.3gp" );
        notifier->setMediaContent( content );
    \endcode

    If video content is available createVideoWidget() can be used to construct a
    new video widget for the media content.

    \code
        QMediaVideoControl control( content );

        QWidget *video = control.createVideoWidget();
        videolayout->addWidget( video );
    \endcode

    Simply delete video widgets when they are no longer needed or when the video
    control becomes invalid.

    \ingroup multimedia

    \sa QMediaContent, QMediaControlNotifier
*/

/*!
    Create a QMediaVideoControl with the handle to a prepared media resource

    The QMediaControl needs to be constructed with \a mediaContent that
    represents the video to be played.
*/

QMediaVideoControl::QMediaVideoControl(QMediaContent* mediaContent):
    QObject(mediaContent)
{
    d = new QMediaVideoControlPrivate(mediaContent);

    connect(d, SIGNAL(valid()), SIGNAL(valid()));
    connect(d, SIGNAL(invalid()), SIGNAL(invalid()));

    connect(d, SIGNAL(videoTargetAvailable()), SIGNAL(videoTargetAvailable()));
    connect(d, SIGNAL(videoTargetRemoved()), SIGNAL(videoTargetRemoved()));
}

/*!
    Destroy a QMediaVideoControl
*/

QMediaVideoControl::~QMediaVideoControl()
{
    delete d;
}


/*!
    Create a QWidget that displays the rendered video content.

    This function is used to create a QWidget that will display the video
    content, it is parented to the widget passed in with the \a parent
    parameter. This widget can be deleted when the application is finished with it.
*/

QWidget* QMediaVideoControl::createVideoWidget(QWidget* parent) const
{
    QWidget*    rc = 0;

    if (d->hasVideo())
    {
#if defined(Q_WS_QWS)
        rc = new QWSEmbedWidget(d->getWindowId(), parent);
#elif defined(Q_WS_X11)
        QX11EmbedContainer *embed = new QX11EmbedContainer(parent);
        embed->embedClient(d->getWindowId());
        rc = embed;
#endif
    }

    return rc;
}

/*!
    Return the name of this control.
*/

QString QMediaVideoControl::name()
{
    return "Video";
}

/*!
    Set video image \a rotation mode.
    \sa QtopiaVideo::VideoRotation
*/
void QMediaVideoControl::setVideoRotation(QtopiaVideo::VideoRotation rotation)
{
    d->setVideoRotation(quint32(rotation));
}

/*!
    Set the video using \a scaleMode to be scaled to fit the window size ( QtopiaVideo::FitWindow )
    or stay unchanged ( QtopiaMedia::NoScale ).

    \sa QtopiaVideo::VideoScaleMode
*/
void QMediaVideoControl::setVideoScaleMode(QtopiaVideo::VideoScaleMode scaleMode)
{
    d->setVideoScaleMode(quint32(scaleMode));
}

/*!
    \fn void QMediaVideoControl::valid();

    Signal that is emitted when the control is valid and available for use.
*/

/*!
    \fn void QMediaVideoControl::invalid();

    Signal that is emitted when the control is invalid and no longer available for use.
*/

/*!
    \fn void QMediaVideoControl::videoTargetAvailable();

    This signal is emitted when there is a video available to be viewed. The client can
    call createVideoWidget() at this time.

    \sa createVideoWidget()
*/

/*!
    \fn void QMediaVideoControl::videoTargetRemoved();

    This signal is emitted when there is no longer video available to be viewed. The client should
    delete any QWidget obtained from createVideoWidget()

    \sa createVideoWidget()
*/

#include "qmediavideocontrol.moc"

