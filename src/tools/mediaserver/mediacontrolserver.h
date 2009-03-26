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

#ifndef MEDIACONTROLSERVER_H
#define MEDIACONTROLSERVER_H

#include <QMediaAbstractControlServer>

#include <media.h>

#include <private/qmediahandle_p.h>


class QMediaServerSession;

namespace mediaserver
{


class MediaControlServer : public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    MediaControlServer(QMediaServerSession* mediaSession,
                       QMediaHandle const& handle);

    ~MediaControlServer();

    QMediaServerSession* mediaSession() const;

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

private slots:
    void stateChanged(QtopiaMedia::State state);
    void posChanged(quint32 ms);
    void lenChanged(quint32 ms);
    void volChanged(int volume);
    void volMuted(bool muted);

private:
    QMediaServerSession* m_mediaSession;
};


}   // ns mediaserver

#endif
