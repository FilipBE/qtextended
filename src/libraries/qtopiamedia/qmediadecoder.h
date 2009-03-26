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

#ifndef QMEDIADECODER_H
#define QMEDIADECODER_H

#include <qtopiamedia/media.h>
#include <QMediaDevice>

#include <qtopiaglobal.h>


class QTOPIAMEDIA_EXPORT QMediaDecoder : public QMediaDevice
{
    Q_OBJECT

public:
    virtual ~QMediaDecoder();

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;

    virtual quint64 length() = 0;
    virtual bool seek(qint64 ms) = 0;

    virtual void setVolume(int volume) = 0;
    virtual int volume() = 0;

    virtual void setMuted(bool mute) = 0;
    virtual bool isMuted() = 0;

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);
};

#endif

