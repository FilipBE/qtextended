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
#include "qmediacontent.h"

#include "qmediaseekcontrol.h"


// {{{ QMediaSeekControlPrivate
class QMediaSeekControlPrivate : public QMediaAbstractControl
{
    Q_OBJECT
public:
    QMediaSeekControlPrivate(QMediaContent* mediaContent, QString const& name);
    ~QMediaSeekControlPrivate();

    void seek(quint32 position, QtopiaMedia::Offset offset);
    void jumpForward(quint32 ms);
    void jumpBackwards(quint32 ms);
    void seekToStart();
    void seekToEnd();
    void seekForward();
    void seekBackward();

signals:
    void positionChanged(quint32 ms);
};

QMediaSeekControlPrivate::QMediaSeekControlPrivate(QMediaContent* mediaContent, QString const& name):
    QMediaAbstractControl(mediaContent, name)
{
    proxyAll();
}

QMediaSeekControlPrivate::~QMediaSeekControlPrivate()
{
}

void QMediaSeekControlPrivate::seek(quint32 position, QtopiaMedia::Offset offset)
{
    forward(SLOT(seek(quint32,Offset)), QMediaAbstractControl::SlotArgs() << uint(position) << uint(offset));
}

void QMediaSeekControlPrivate::jumpForward(quint32 ms)
{
    forward(SLOT(jumpForward(quint32)), QMediaAbstractControl::SlotArgs() <<  uint(ms));
}

void QMediaSeekControlPrivate::jumpBackwards(quint32 ms)
{
    forward(SLOT(jumpBackwards(quint32)), QMediaAbstractControl::SlotArgs() << uint(ms));
}

void QMediaSeekControlPrivate::seekToStart()
{
    forward(SLOT(seekToStart()));
}

void QMediaSeekControlPrivate::seekToEnd()
{
    forward(SLOT(seekToEnd()));
}

void QMediaSeekControlPrivate::seekForward()
{
    forward(SLOT(seekForward()));
}

void QMediaSeekControlPrivate::seekBackward()
{
    forward(SLOT(seekBackward()));
}
// }}}


// {{{ QMediaSeekControl
QMediaSeekControl::QMediaSeekControl(QMediaContent* mediaContent):
    QObject(mediaContent),
    d(new QMediaSeekControlPrivate(mediaContent, QMediaSeekControl::name()))
{
    connect(d, SIGNAL(positionChanged(quint32)), this, SIGNAL(positionChanged(quint32)));

    connect(d, SIGNAL(valid()), this, SIGNAL(valid()));
    connect(d, SIGNAL(invalid()), this, SIGNAL(invalid()));
}

QMediaSeekControl::~QMediaSeekControl()
{
}

QString QMediaSeekControl::name()
{
    return "Seek";
}

//public slots:
void QMediaSeekControl::seek(quint32 position, QtopiaMedia::Offset offset)
{
    d->seek(position, offset);
}

void QMediaSeekControl::jumpForward(quint32 ms)
{
    d->jumpForward(ms);
}

void QMediaSeekControl::jumpBackwards(quint32 ms)
{
    d->jumpBackwards(ms);
}

void QMediaSeekControl::seekToStart()
{
    d->seekToStart();
}

void QMediaSeekControl::seekToEnd()
{
    d->seekToEnd();
}

void QMediaSeekControl::seekForward()
{
    d->seekForward();
}

void QMediaSeekControl::seekBackward()
{
    d->seekBackward();
}
// }}}


#include "qmediaseekcontrol.moc"
