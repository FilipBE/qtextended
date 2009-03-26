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

#ifndef QMEDIACONTROL_H
#define QMEDIACONTROL_H

#include <QObject>
#include <QString>

#include "media.h"


class QMediaContent;


class QMediaControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaControl(QMediaContent* mediaContent);
    ~QMediaControl();

    QtopiaMedia::State playerState() const;
    quint32 length() const;
    quint32 position() const;

    bool isMuted() const;
    int volume() const;

    QString errorString() const;

    static QString name();

public slots:
    void start();
    void pause();
    void stop();
    void seek(quint32 ms);

    void setVolume(int volume);
    void setMuted(bool mute);

signals:
    void valid();
    void invalid();

    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);

private:
    Q_DISABLE_COPY(QMediaControl);

    QMediaControlPrivate*   d;
};

#endif
