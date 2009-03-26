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

#include <QMediaDevice>
#include <QMediaDecoder>
#include <QDebug>

#include "cruxusurihandlers.h"
#include "cruxusoutputdevices.h"

#include "cruxussimplesession.h"

#include <qtopialog.h>


//#define DEBUG_SESSION

namespace cruxus
{

// {{{ SimpleSessionPrivate
class SimpleSessionPrivate
{
public:
    bool                opened;
    bool                connected;
    QMediaDevice*       source;
    QMediaDevice*       sink;
    QMediaDecoder*      coder;
    OutputThread*       thread;

    QMediaHandle        handle;
    QString             domain;
    QString             errorString;
    QtopiaMedia::State  state;
    QtopiaMedia::State  preSuspendState;
    bool isSuspended;
};
// }}}

// {{{ SimpleSession

/*!
    \class cruxus::SimpleSession
    \internal
*/

SimpleSession::SimpleSession
(
 QMediaHandle const&    handle,
 QMediaDevice*          source,
 QMediaDecoder*         coder,
 QMediaDevice*          sink
):
    d(new SimpleSessionPrivate)
{
    d->opened = false;
    d->connected = false;
    d->handle = handle;
    d->source = source;
    d->sink = sink;
    d->coder = coder;
    d->state = QtopiaMedia::Stopped;
    d->preSuspendState = QtopiaMedia::Stopped;
    d->isSuspended = false;

    connect(d->coder, SIGNAL(playerStateChanged(QtopiaMedia::State)), SIGNAL(playerStateChanged(QtopiaMedia::State)));
    connect(d->coder, SIGNAL(positionChanged(quint32)), SIGNAL(positionChanged(quint32)));
    connect(d->coder, SIGNAL(lengthChanged(quint32)), SIGNAL(lengthChanged(quint32)));
    connect(d->coder, SIGNAL(volumeChanged(int)),SIGNAL(volumeChanged(int)));
    connect(d->coder, SIGNAL(volumeMuted(bool)),SIGNAL(volumeMuted(bool)));

    connect(d->coder, SIGNAL(playerStateChanged(QtopiaMedia::State)), SLOT(stateChanged(QtopiaMedia::State)));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),SLOT(timeout()));

    timer->start(3500);

#ifdef DEBUG_SESSION
    qLog(Media) <<"SimpleSession::SimpleSession() "<< this;
#endif
}

SimpleSession::~SimpleSession()
{
#ifdef DEBUG_SESSION
    qLog(Media) <<"SimpleSession::~SimpleSession() "<<this;
#endif
    timer->stop();

    d->connected = false;
    d->coder->disconnectFromInput(d->source);
    d->sink->disconnectFromInput(d->coder);

    if (d->opened) {
        d->source->close();
        d->sink->close();
    }

    UriHandlers::destroyInputDevice(d->source);
    //OutputDevices::destroyOutputDevice(d->sink);

    d->coder->deleteLater();

    delete d;
}

void SimpleSession::start()
{
#ifdef DEBUG_SESSION
    qLog(Media) << "SimpleSession::start() begin " << this;
#endif

    if (d->isSuspended || d->state == QtopiaMedia::Playing || d->state == QtopiaMedia::Error)
        return;

#ifdef DEBUG_SESSION
    qLog(Media) << "SimpleSession::start() kickoff " << this;
#endif
    if (!d->opened) {
#ifdef DEBUG_SESSION
        qLog(Media) << "SimpleSession::start() not opened, open " << this;
#endif
        if(!d->sink->isOpen())
            d->sink->open(QIODevice::WriteOnly | QIODevice::Unbuffered);

        if (!d->source->open(QIODevice::ReadOnly)) {
            d->errorString = "Cruxus; Unable to open devices for media session";
            qWarning() << d->errorString;
            emit playerStateChanged(d->state = QtopiaMedia::Error);
        }
        else
            d->opened = true;
    }

    if (d->opened && !d->connected) {
#ifdef DEBUG_SESSION
        qLog(Media) <<"SimpleSession::start() no input, connect " << this;
#endif
        d->coder->connectToInput(d->source);
        d->sink->connectToInput(d->coder);

        d->connected = true;
    }
    if (d->connected) {
#ifdef DEBUG_SESSION
        qLog(Media) << "SimpleSession::start() decoder begin " << this;
#endif
        d->coder->start();
    }
}

void SimpleSession::pause()
{
    if (d->connected) {
#ifdef DEBUG_SESSION
        qLog(Media) <<"SimpleSession::pause() " << this;
#endif
        d->coder->pause();
    }
#ifdef DEBUG_SESSION
    else {
        qLog(Media) << "SimpleSession::pause() not connected???" << this;
    }
#endif
}

void SimpleSession::stop()
{
    if (d->connected) {
#ifdef DEBUG_SESSION
        qLog(Media) << "SimpleSession::stop() " << this;
#endif
        d->coder->stop();
        d->coder->disconnectFromInput(d->source);
        d->sink->disconnectFromInput(d->coder);
        d->connected = false;
    }
#ifdef DEBUG_SESSION
    else {
        qLog(Media) << "SimpleSession::stop() not connected???" << this;
    }
#endif
    d->connected = false;
}

void SimpleSession::suspend()
{
#ifdef DEBUG_SESSION
    qLog(Media) << "SimpleSession::suspend()" << this;
#endif
    if ( !d->isSuspended ) {
        d->isSuspended = true;
        d->preSuspendState = d->state;
        if ( d->state == QtopiaMedia::Playing ) {
            pause();
        }
    }
}

void SimpleSession::resume()
{
#ifdef DEBUG_SESSION
    qLog(Media) << "SimpleSession::resume()" << this;
#endif

    if (d->isSuspended) {
        d->isSuspended = false;
        if ( d->preSuspendState == QtopiaMedia::Playing ) {
            start();
            d->state = QtopiaMedia::Playing;
        }
    }
}

void SimpleSession::seek(quint32 ms)
{
    d->coder->seek(ms);
}

quint32 SimpleSession::length()
{
    return d->coder->length();
}

void SimpleSession::setVolume(int volume)
{
    d->coder->setVolume(volume);
}

int SimpleSession::volume() const
{
    return d->coder->volume();
}

void SimpleSession::setMuted(bool mute)
{
    d->coder->setMuted(mute);
}

bool SimpleSession::isMuted() const
{
    return d->coder->isMuted();
}

QtopiaMedia::State SimpleSession::playerState() const
{
    return d->state;
}

QString SimpleSession::errorString()
{
    return QString();
}

void SimpleSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString SimpleSession::domain() const
{
    return d->domain;
}

QStringList SimpleSession::interfaces()
{
    return QStringList("Basic");
}

QString SimpleSession::id() const
{
    return d->handle.id().toString();
}

QString SimpleSession::reportData() const
{
    return QString("engine:Cruxus");
}

void SimpleSession::stateChanged(QtopiaMedia::State state)
{
    d->state = state;
}

void SimpleSession::timeout()
{
#ifdef DEBUG_SESSION
    qLog(Media) << "SimpleSession::timeout() " << this;
#endif

    timer->stop();

    if ((d->state == QtopiaMedia::Error) ||
            ((!d->connected && !d->opened) && (d->state == QtopiaMedia::Playing)))
        resume();
}

// }}}

}   // ns cruxus
