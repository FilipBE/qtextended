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

#include "qmediaseekcontrolserver.h"


// {{{ QMediaSeekDelegate
QMediaSeekDelegate::~QMediaSeekDelegate()
{
}
// }}}


// {{{ QMediaSeekControlServer
QMediaSeekControlServer::QMediaSeekControlServer
(
 QMediaHandle const& handle,
 QMediaSeekDelegate* seekDelegate,
 QObject* parent
):
    QMediaAbstractControlServer(handle, "Seek", parent),
    m_seekDelegate(seekDelegate)
{
    proxyAll();

    connect(m_seekDelegate, SIGNAL(positionChanged(quin32)),
            this, SIGNAL(positionChanged(quint32)));
}

QMediaSeekControlServer::~QMediaSeekControlServer()
{
}

//public slots:
void QMediaSeekControlServer::seek(quint32 position, QtopiaMedia::Offset offset)
{
    m_seekDelegate->seek(position, offset);
}

void QMediaSeekControlServer::jumpForward(quint32 ms)
{
    m_seekDelegate->jumpForward(ms);
}

void QMediaSeekControlServer::jumpBackwards(quint32 ms)
{
    m_seekDelegate->jumpBackwards(ms);
}

void QMediaSeekControlServer::seekToStart()
{
    m_seekDelegate->seekToStart();
}

void QMediaSeekControlServer::seekToEnd()
{
    m_seekDelegate->seekToEnd();
}

void QMediaSeekControlServer::seekForward()
{
    m_seekDelegate->seekForward();
}

void QMediaSeekControlServer::seekBackward()
{
    m_seekDelegate->seekBackward();
}
// }}}

