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

#ifndef QMEDIASERVERSESSION_H
#define QMEDIASERVERSESSION_H

#include <QObject>
#include <QStringList>

#include <qtopiaglobal.h>

#include <media.h>


class QTOPIAMEDIA_EXPORT QMediaServerSession : public QObject
{
    Q_OBJECT

public:
    virtual ~QMediaServerSession();

    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    virtual void suspend() = 0;
    virtual void resume() = 0;

    virtual void seek(quint32 ms) = 0;
    virtual quint32 length() = 0;

    virtual void setVolume(int volume) = 0;
    virtual int volume() const = 0;

    virtual void setMuted(bool mute) = 0;
    virtual bool isMuted() const = 0;

    virtual QtopiaMedia::State playerState() const = 0;

    virtual QString errorString() = 0;

    virtual void setDomain(QString const& domain) = 0;
    virtual QString domain() const = 0;

    virtual QStringList interfaces() = 0;

    virtual QString id() const = 0;
    virtual QString reportData() const = 0;

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);
    void interfaceAvailable(const QString& name);
    void interfaceUnavailable(const QString& name);
};

#endif
