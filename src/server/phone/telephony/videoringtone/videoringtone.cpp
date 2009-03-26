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

#include "videoringtone.h"

#include <qmediacontent.h>
#include <qmediacontrol.h>
#include <qmediavideocontrol.h>
#include <QtopiaIpcAdaptor>


class VideoRingtonePrivate
{
public:
    QMediaContent*      videoContent;
    QMediaVideoControl* videoControl;
    QtopiaIpcAdaptor*  adaptor;
};

/*!
  \class VideoRingtone
    \inpublicgroup QtTelephonyModule
  \brief The VideoRingtone class provides an interface
  to the Qt Extended media system to play the video tones for incoming calls.
  \ingroup QtopiaServer::Telephony

  The VideoRingtone acts as a bridge between RingControl
  and other part of the system that is interested in displaying the video widget,
  for example, CallScreen.

  The RingControl class communicates with this class via messages on the \c QPE/VideoRingtone
  QCop channel. The following messages are supported.
  \table
    \header \o Message \o Description
    \row    \o \c playVideo(QString)   \o This message is forwarded to playVideo().
    \row    \o \c stopVideo()          \o This message is forwarded to stopVideo().
    \row    \o \c videoRingtoneFailed() \o This message is equivalent to the videoRingtoneFailed() signal.
  \endtable

  RingControl calls playVideo() when the video ringtone is preferred.
  The CallScreen should be listening to the videoWidgetReady() signal
  to retreive the video widget object by calling videoWidget().

  The signal videoRingtoneStopped() is emitted when the tone finished playing.

  \code
        CallScreen::CallScreen() {
            ...
            VideoRingtone *vrt = qtopiaTask<VideoRingtone>();
            if ( vrt )
                connect( vrt, SIGNAL(videoWidgetReady()),
                    this, SLOT(showVideo()) );
            ...
        }

        CallScreen::showVideo() {
            VideoRingtone *vrt = qtopiaTask<VideoRingtone>();
            if (!vrt)
                return;
            QWidget *widget = vrt->videoWidget();

            // set the new parent to manage resource
            widget->setParent( this );

            layout()->addWidget( widget );
        }
  \endcode

  This class is a Qt Extended server task and is part of the Qt Extended server. It cannot be used by other Qt Extended applications.

  \sa RingControl
*/

/*!
  Creates a new VideoRingtone instance with the specified \a parent.
 */
VideoRingtone::VideoRingtone( QObject *parent)
    : QObject(parent)
{
    d = new VideoRingtonePrivate;

    d->videoContent = 0;
    d->videoControl = 0;

    d->adaptor = new QtopiaIpcAdaptor( "QPE/VideoRingtone", this );
    QtopiaIpcAdaptor::connect(d->adaptor, MESSAGE(playVideo(QString)), 
                               this, SLOT(playVideo(QString)) );
    QtopiaIpcAdaptor::connect(d->adaptor, MESSAGE(stopVideo()),
                               this, SLOT(stopVideo()) );
    QtopiaIpcAdaptor::connect(this, SIGNAL(videoRingtoneFailed()),
                              d->adaptor, MESSAGE(videoRingtoneFailed()) );
}


/*!
  Destroys the VideoRingtone object.
*/
VideoRingtone::~VideoRingtone()
{
    delete d;
}

/*!
  Attempts to play the video \a fileName.
*/
void VideoRingtone::playVideo(const QString& fileName)
{
    d->videoContent = new QMediaContent( QContent( fileName ),"RingTone" );
    connect(d->videoContent, SIGNAL(mediaError(QString)), this, SIGNAL(videoRingtoneFailed()));

    QMediaControl* mediaControl = new QMediaControl(d->videoContent);
    connect(mediaControl, SIGNAL(valid()), mediaControl, SLOT(start()));

    d->videoControl = new QMediaVideoControl(d->videoContent);
    connect(d->videoControl, SIGNAL(valid()), this, SIGNAL(videoWidgetReady()));
    emit videoRingtoneFailed();
}


/*!
  Stops the media content.
*/
void VideoRingtone::stopVideo()
{
    if ( d->videoContent )
        delete d->videoContent;

    d->videoControl = 0;

    emit videoRingtoneStopped();
}

/*!
  Returns the video widget instance.
*/
QWidget* VideoRingtone::videoWidget()
{
    if (d->videoControl != 0)
        return d->videoControl->createVideoWidget();

    return NULL;
}

/*!
  \fn void VideoRingtone::videoRingtoneFailed()

  This signal is emitted when the media system fails to play the video tone.
*/

/*!
  \fn void VideoRingtone::videoWidgetReady()

  This signal is emitted when the video widget is created and ready to be used.
*/

/*!
  \fn void VideoRingtone::videoRingtoneStopped()

  This signal is emitted when the video tone is stopped.
*/

QTOPIA_TASK(VideoRingtone,VideoRingtone);
QTOPIA_TASK_PROVIDES(VideoRingtone, VideoRingtone);
