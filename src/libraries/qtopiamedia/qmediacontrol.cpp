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

#include <qtopiaservices.h>

#include "qmediaabstractcontrol.h"
#include "qmediacontent.h"

#include "qmediacontrol.h"


// {{{ QMediaControlPrivate
class QMediaControlPrivate : public QMediaAbstractControl
{
    Q_OBJECT

public:
    QMediaControlPrivate(QMediaContent* mediaContent);
    ~QMediaControlPrivate();

    QtopiaMedia::State playerState() const;

    quint32 length() const;
    quint32 position() const;

    bool isMuted();
    int volume() const;

    QString errorString() const;

public slots:
    void start();
    void pause();
    void stop();
    void seek(quint32 ms);

    void setVolume(int volume);
    void setMuted(bool mute);

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);
};

QMediaControlPrivate::QMediaControlPrivate(QMediaContent* mediaContent):
    QMediaAbstractControl(mediaContent, QMediaControl::name())
{
    proxyAll();
}

QMediaControlPrivate::~QMediaControlPrivate()
{
}

QtopiaMedia::State QMediaControlPrivate::playerState() const
{
    return static_cast<QtopiaMedia::State>(value(QLatin1String("playerState")).toUInt());
}

quint32 QMediaControlPrivate::length() const
{
    return static_cast<quint32>(value(QLatin1String("length")).toUInt());
}

quint32 QMediaControlPrivate::position() const
{
    return static_cast<quint32>(value(QLatin1String("position")).toUInt());
}

bool QMediaControlPrivate::isMuted()
{
    return value(QLatin1String("muted")).toBool();
}

int QMediaControlPrivate::volume() const
{
    return value(QLatin1String("volume")).toInt();
}

QString QMediaControlPrivate::errorString() const
{
    return value(QLatin1String("errorString")).toString();
}

//public slots:
void QMediaControlPrivate::start()
{
    forward(SLOT(start()));
}

void QMediaControlPrivate::pause()
{
    forward(SLOT(pause()));
}

void QMediaControlPrivate::stop()
{
    forward(SLOT(stop()));
}

void QMediaControlPrivate::seek(quint32 ms)
{
    forward(SLOT(seek(quint32)), QMediaAbstractControl::SlotArgs() << ms);
}

void QMediaControlPrivate::setVolume(int volume)
{
    forward(SLOT(setVolume(int)), QMediaAbstractControl::SlotArgs() << volume);
}

void QMediaControlPrivate::setMuted(bool mute)
{
    forward(SLOT(setMuted(bool)), QMediaAbstractControl::SlotArgs() << mute);
}
// }}}


/*!
    \class QMediaControl
    \inpublicgroup QtMediaModule

    \brief The QMediaControl class is used to manipulate a media resource in the
    Qt Extended media system.

    \ingroup multimedia
*/


// {{{ QMediaControl

/*!
    Construct a QMediaControl.

    Construct a QMediaControl for controlling \a mediaContent. The
    QMediaControl can be constructed only after the \a mediaContent instance has
    signaled that it is available via the QMediaContent::controlAvailable()
    signal.

    \sa QMediaContent::controlAvailable()
*/

QMediaControl::QMediaControl(QMediaContent* mediaContent):
    QObject(mediaContent)
{
    d = new QMediaControlPrivate(mediaContent);

    connect(d, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SIGNAL(playerStateChanged(QtopiaMedia::State)));

    connect(d, SIGNAL(positionChanged(quint32)),
            this, SIGNAL(positionChanged(quint32)));

    connect(d, SIGNAL(lengthChanged(quint32)),
            this, SIGNAL(lengthChanged(quint32)));

    connect(d, SIGNAL(volumeChanged(int)),
            this, SIGNAL(volumeChanged(int)));

    connect(d, SIGNAL(volumeMuted(bool)),
            this, SIGNAL(volumeMuted(bool)));

    connect(d, SIGNAL(valid()), this, SIGNAL(valid()));
    connect(d, SIGNAL(invalid()), this, SIGNAL(invalid()));
}

/*!
    Destroy the QMediaControl object.
*/

QMediaControl::~QMediaControl()
{
}

/*!
    Return the current state of the media content
*/

QtopiaMedia::State QMediaControl::playerState() const
{
    return d->playerState();
}

/*!
    Return the length of the media content.
*/

quint32 QMediaControl::length() const
{
    return d->length();
}

/*!
    Return the current position in the media content.
*/

quint32 QMediaControl::position() const
{
    return d->position();
}

/*!
    Mute or unmute the volume for the media content.

    The \a mute parameter enables or disable muting of the media.
*/

void QMediaControl::setMuted(bool mute)
{
    d->setMuted(mute);
}

/*!
    Return the mute state of the media content.
*/

bool QMediaControl::isMuted() const
{
    return d->isMuted();
}

/*!
    Set the volume of the current media content. The acceptable
    volume range is 1-100.

    The \a volume is the value of the volume to be set for this media.

*/

void QMediaControl::setVolume(int volume)
{
    return d->setVolume(volume);
}

/*!
    Return the current volume for the media content.
*/

int QMediaControl::volume() const
{
    return d->volume();
}

/*!
    When an error occurs, indicated by the QtopiaMedia::Error state,
    this function will return a QString with appropriate information
    regarding the error.
*/

QString QMediaControl::errorString() const
{
    return d->errorString();
}

/*!
    Returns the name of this control.
*/

QString QMediaControl::name()
{
    return "Basic";
}


/*!
    Start playing the media content.
*/

void QMediaControl::start()
{
    d->start();
}


/*!
    Pause the media content.
*/

void QMediaControl::pause()
{
    d->pause();
}

/*!
    Stop playing the media content.
*/

void QMediaControl::stop()
{
    d->stop();
}

/*!
    Seek to the location in the media content. The location is specified with
    the \a ms parameter which is the number of milliseconds from the beginning
    of the media.
*/

void QMediaControl::seek(quint32 ms)
{
    d->seek(ms);
}

/*!
    \fn void QMediaControl::valid();

    Signal that is emitted when the control is valid and available for use.
*/

/*!
    \fn void QMediaControl::invalid();

    Signal that is emitted when the control is invalid and no longer available for use.
*/

/*!
    \fn QMediaControl::playerStateChanged(QtopiaMedia::State state);

    Signals a change in the state of the media content. The new state
    is represented by the \a state parameter.
*/

/*!
    \fn QMediaControl::positionChanged(quint32 ms);

    Signals an update to the current position of a media content.  This signal
    is emitted while the media is playing and represents the current
    milliseconds \a ms from the beginning of the media.
*/

/*!
    \fn QMediaControl::lengthChanged(quint32 ms);

    Signals a change in the length of the media. For some media content the
    length may not be available until after playback has started, it is
    advisable to hook onto this signal to ensure clients have the correct
    length information, The length information is available as the total number
    of milliseconds from the parameter \a ms.
*/

/*!
    \fn QMediaControl::volumeChanged(int volume);

    Signals a change in volume of the media content. The new volume is has the
    value \a volume, ranging from 1 - 100.
*/

/*!
    \fn QMediaControl::volumeMuted(bool muted);

    Signals if the media content has been muted or not. The \a muted paramter
    is true when muted, false when not.
*/

// }}}


#include "qmediacontrol.moc"

