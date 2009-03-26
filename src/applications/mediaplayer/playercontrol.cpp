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
#include <QValueSpaceObject>
#include <QMediaContent>
#include <QMediaControl>
#include <QtopiaApplication>

#include "playercontrol.h"


static const int VOLUME_MAX = 100;

class PlayerControlPrivate
{
public:
    int                     volume;
    int                     tmpVol;
    bool                    ismute;
    PlayerControl::State    state;
    QMediaContent*          content;
    QMediaControl*          control;
    QValueSpaceObject*      status;
    QValueSpaceItem*        server;
    QTimer*                 timer;
};


PlayerControl::PlayerControl(QObject* parent):
    QObject(parent),
    d(new PlayerControlPrivate)
{
    d->content = 0;
    d->control = 0;
    d->state = Stopped;
    d->ismute = false;

    d->status = new QValueSpaceObject("/Media/Player");

    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()), SLOT(timeout()));

    QSettings config("Trolltech", "MediaPlayer");
    d->volume = config.value("Volume", 100).toInt();
    d->tmpVol = d->volume;
}

PlayerControl::~PlayerControl()
{
    delete d->content;
    delete d->status;
    delete d;
}

void PlayerControl::open(const QUrl& url)
{
    QString content = url.toString();

    if (url.scheme().isEmpty()) {
        setMediaContent(new QMediaContent(QContent(url.path())));
        content.prepend("file://");
    }
    else
        setMediaContent(new QMediaContent(url));

    d->status->setAttribute("Content", content);
}

void PlayerControl::close()
{
    setState(Stopped);
    setMediaContent(0);
}

PlayerControl::State PlayerControl::state() const
{
    return d->state;
}

void PlayerControl::setState(State state)
{
    if (d->state != state) {
        d->state = state;

        if (d->control) {
            switch (d->state) {
            case Playing:
                d->control->start();
                QtopiaApplication::instance()->registerRunningTask( "Media Player", this );
                d->timer->start(8000);
                break;
            case Paused:
                d->control->pause();
                d->timer->stop();
                break;
            case Stopped:
                d->control->stop();
                QtopiaApplication::instance()->unregisterRunningTask( this );
                d->timer->stop();
                break;
            }
        }

        d->status->setAttribute("State", d->state);

        emit stateChanged(d->state);
    }
}

int PlayerControl::volume() const
{
    return d->volume;
}

void PlayerControl::setVolume(int volume)
{
    d->volume = volume;
}

void PlayerControl::setMute(bool mute)
{
    d->ismute = mute;
}

void PlayerControl::activate()
{
    connect(d->control, SIGNAL(volumeChanged(int)),
            this, SLOT(setVolume(int)));
    connect(d->control, SIGNAL(volumeMuted(bool)),
            this, SLOT(setMute(bool)));

    d->control->setVolume(d->volume);
    d->control->setMuted(d->ismute);

    if (state() == Playing)
        d->control->start();
}

void PlayerControl::timeout()
{
    if (d->tmpVol != d->volume) {
        d->tmpVol = d->volume;
        QSettings config("Trolltech", "MediaPlayer");
        config.setValue("Volume", d->volume);
    }
}

void PlayerControl::setMediaContent(QMediaContent* content)
{
    if (d->content == content)
        return;

    emit contentChanged(content);

    delete d->content;
    d->content = content;

    if (d->content) {
        d->control = new QMediaControl(d->content);
        connect(d->control, SIGNAL(valid()), SLOT(activate()));
    }
    else {
        d->control = NULL;
        d->status->removeAttribute("Content");
        d->status->removeAttribute("State");
    }
}
