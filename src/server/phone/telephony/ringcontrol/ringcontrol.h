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

#ifndef RINGCONTROL_H
#define RINGCONTROL_H

#include <QObject>
#include <QTime>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include "qtopiaserverapplication.h"

class QSoundControl;
class QWSSoundClient;

class RingControlPrivate;
class RingControl : public QObject
{
Q_OBJECT
public:
    RingControl(QObject *parent = 0);
    virtual ~RingControl();

    enum RingType {
        NotRinging,
        Call,
        Msg
    };

    void setCallRingEnabled(bool);
    bool callRingEnabled() const;
    void setMessageRingEnabled(bool);
    bool messageRingEnabled() const;

    RingType ringType() const;
    int ringTime() const;
    void playSound( const QString &soundFile );
    void setVolume(int vol);

    void setVibrateDuration(int);
    int vibrateDuration() const;
    void setMsgRingTime(int);
    int msgRingTime() const;
    void stopRing();

public slots:
    void muteRing();
    void stopMessageAlert();

signals:
    void ringTypeChanged(RingControl::RingType);

private slots:
    void stateChanged();
    void nextRing();
    void profileChanged();
    void videoRingtoneFailed();
    void startMessageRingtone();
    void stopMessageRingtone();
    void startRingtone(const QString&);
    void stopRingtone(const QString&);

private:
    virtual void timerEvent(QTimerEvent *e);
    void startRinging(RingType);
    void initSound();
    void setSoundPriority(bool priorityPlay);

    QString findRingTone();

    RingControlPrivate *d;
};

QTOPIA_TASK_INTERFACE(RingControl);
#endif
