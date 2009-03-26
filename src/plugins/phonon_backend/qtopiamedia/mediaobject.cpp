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

#include <QQueue>
#include <QTimer>

#include <QContent>
#include <QMediaContent>
#include <QMediaControl>
#include <QMediaVideoControl>

#include "mediaobject.h"

/*!
    \class Phonon::qtopiamedia::MediaNode
    \internal
*/

namespace Phonon
{

namespace qtopiamedia
{

inline Phonon::State qtopiaStateToPhononState(QtopiaMedia::State state)
{
    switch (state) {
    case QtopiaMedia::Stopped: return Phonon::StoppedState;
    case QtopiaMedia::Playing: return Phonon::PlayingState;
    case QtopiaMedia::Paused: return Phonon::PausedState;
    case QtopiaMedia::Error: return Phonon::ErrorState;
    default:
        ;
    }

    return Phonon::ErrorState;
}


class MediaObjectPrivate
{
public:
    Backend*            backend;
    QMediaContent*      content;
    QMediaControl*      control;
    Phonon::ErrorType   error;
    QTimer*             tickTimer;
    QList<MediaNode*>   peers;
    QQueue<MediaSource> sources;

    bool                valid;
    bool                hasVideo;
    bool                isSeekable;
    qint32              tickInterval;
    qint32              transitionTime;
    qint32              prefinishMark;
    QString             errorString;
    QtopiaMedia::State  state;
    QtopiaMedia::State  requiredState;
};

/*!
    \class Phonon::qtopiamedia::MediaObject
    \internal
*/

MediaObject::MediaObject(Backend* backend, QObject* parent):
    QObject(parent),
    d(new MediaObjectPrivate)
{
    d->backend = backend;
    d->content = 0;
    d->control = 0;
    d->error = Phonon::NormalError;

    d->valid = false;
    d->hasVideo = false;
    d->isSeekable = true;
    d->tickInterval = 1000;
    d->transitionTime = 0;
    d->prefinishMark = 0;
    d->state = QtopiaMedia::Stopped;

    d->tickTimer = new QTimer(this);
    connect(d->tickTimer, SIGNAL(timeout()), SLOT(tickTimeout()));
}

MediaObject::~MediaObject()
{
    stop();

    delete d;
}

void MediaObject::play()
{
    d->requiredState = QtopiaMedia::Playing;
    if (d->valid)
        d->control->start();
}

void MediaObject::pause()
{
    d->requiredState = QtopiaMedia::Paused;
    if (d->valid)
        d->control->pause();
}

void MediaObject::stop()
{
    d->requiredState = QtopiaMedia::Stopped;
    if (d->valid)
        d->control->stop();
}

void MediaObject::seek(qint64 milliseconds)
{
    if (d->valid)
        d->control->seek(static_cast<quint32>(milliseconds));
}

qint32 MediaObject::tickInterval() const
{
    return d->tickInterval;
}

void MediaObject::setTickInterval(qint32 interval)
{
    d->tickInterval = interval;

    d->tickTimer->setInterval(interval);
}

bool MediaObject::hasVideo() const
{
    return d->hasVideo;
}

bool MediaObject::isSeekable() const
{
    return d->isSeekable;
}

qint64 MediaObject::currentTime() const
{
    if (d->control != 0)
        return d->control->position();

    return 0;
}

Phonon::State MediaObject::state() const
{
    return qtopiaStateToPhononState(d->state);
}

QString MediaObject::errorString() const
{
    return d->errorString;
}

Phonon::ErrorType MediaObject::errorType() const
{
    return d->error;
}

qint64 MediaObject::totalTime() const
{
    if (d->control != 0)
        return d->control->length();

    return 0;
}

Phonon::MediaSource MediaObject::source() const
{
    return d->sources.empty() ? MediaSource() : d->sources.head();
}

void MediaObject::setSource(const Phonon::MediaSource& source)
{
    switch (source.type()) {
    case MediaSource::LocalFile:
    case MediaSource::Url:
        d->sources.push_front(source);
        readyContent();
        break;

    case MediaSource::Invalid:
    case MediaSource::Disc:
    default:
        // Set error code
        return;
    }
}

void MediaObject::setNextSource(const Phonon::MediaSource& source)
{
    d->sources.enqueue(source);
}

qint32 MediaObject::prefinishMark() const
{
    return d->prefinishMark;
}

void MediaObject::setPrefinishMark(qint32 mark)
{
    d->prefinishMark = mark;
}

qint32 MediaObject::transitionTime() const
{
    return d->transitionTime;
}

void MediaObject::setTransitionTime(qint32 transitionTime)
{
    d->transitionTime = transitionTime;
}

//
bool MediaObject::connectNode(MediaNode* node)
{
    d->peers.push_back(node);
    if (d->valid)
        node->setContent(d->content);

    return true;
}

bool MediaObject::disconnectNode(MediaNode* node)
{
    if (d->peers.contains(node)) {
        d->peers.removeAll(node);
        node->setContent(0);
        return true;
    }

    return false;
}

void MediaObject::setContent(QMediaContent*)
{
    qWarning() << "MedaObject; Attempting to make an invalid connection";
}

//
void MediaObject::readyContent()
{
    d->tickTimer->stop();
    if (d->content)
        delete d->content;

    if (d->sources.empty()) {
        emit finished();
        return;
    }

    MediaSource const& h = d->sources.head();

    if (h.type() == MediaSource::Invalid) {
        d->state = d->requiredState = QtopiaMedia::Stopped;
        d->sources.dequeue();
        emit finished();
        return;
    }

    d->content = new QMediaContent(h.url(), "Media", this);
    connect(d->content, SIGNAL(mediaError(QString)), SLOT(mediaError(QString)));
    connect(d->content, SIGNAL(controlAvailable(QString)), SLOT(controlAvailable(QString)));
    connect(d->content, SIGNAL(controlUnavailable(QString)), SLOT(controlUnavailable(QString)));

    d->valid = false;

    emit currentSourceChanged(h);
    emit stateChanged(Phonon::LoadingState, qtopiaStateToPhononState(d->state));
}

// qtopiamedia signal handling
void MediaObject::mediaError(QString const& errorString)
{
    d->errorString = errorString;

    emit stateChanged(Phonon::ErrorState, qtopiaStateToPhononState(d->state));
}

void MediaObject::controlAvailable(QString const& name)
{
    if (name == QMediaControl::name() && !d->valid) {
        d->control = new QMediaControl(d->content);
        connect(d->control, SIGNAL(playerStateChanged(QtopiaMedia::State)), SLOT(playerStateChanged(QtopiaMedia::State)));
        connect(d->control, SIGNAL(lengthChanged(quint32)), SLOT(lengthChanged(quint32)));

        d->valid = true;

        // Connect peers
        foreach (MediaNode* peer, d->peers)
            peer->setContent(d->content);

        switch (d->requiredState) {
        case QtopiaMedia::Playing:
            d->control->start();
            break;
        default:
            ;
        }
    }

    bool hasVideo = d->content->controls().contains(QMediaVideoControl::name());

    if (hasVideo != d->hasVideo)
        emit hasVideoChanged(d->hasVideo = hasVideo);
}

void MediaObject::controlUnavailable(QString const&)
{
}

void MediaObject::playerStateChanged(QtopiaMedia::State state)
{
    emit stateChanged(qtopiaStateToPhononState(state), qtopiaStateToPhononState(d->state));

    d->state = state;

    switch (d->state) {
    case QtopiaMedia::Playing:
        d->tickTimer->start(d->tickInterval);
        break;
    case QtopiaMedia::Stopped:
        d->tickTimer->stop();
        if (!d->sources.empty()) {
            d->sources.dequeue();
            readyContent();
        }
        break;
    default:
        ;
    }
}

void MediaObject::positionChanged(quint32 position)
{
    if (d->prefinishMark > 0 && static_cast<qint32>(position) >= d->prefinishMark)
        emit prefinishMarkReached(d->prefinishMark);
}

void MediaObject::lengthChanged(quint32 ms)
{
    emit totalTimeChanged(ms);
}

void MediaObject::tickTimeout()
{
    emit tick(d->control->position());
}

}   // ns qtopiamedia

}   // Phonon

