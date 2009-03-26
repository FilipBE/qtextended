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

#ifndef PLAYERCONTROL_H
#define PLAYERCONTROL_H

#include <QObject>
#include <QTimer>


class QUrl;
class QContent;
class QMediaContent;

class PlayerControlPrivate;

class PlayerControl : public QObject
{
    Q_OBJECT

public:
    PlayerControl(QObject* parent = 0);
    ~PlayerControl();

    enum State { Playing, Paused, Stopped };

    void open(const QUrl& url);
    void close();

    State state() const;
    void setState(State state);

    int volume() const;

public slots:
    void setVolume(int volume);
    void setMute(bool mute);

signals:
    void contentChanged(QMediaContent* content);
    void stateChanged(PlayerControl::State state);

private slots:
    void activate();
    void timeout();

private:
    void setMediaContent(QMediaContent* content);

    PlayerControlPrivate*   d;
};

#endif
