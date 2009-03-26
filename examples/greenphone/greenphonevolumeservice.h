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

#ifndef GREENPHONEVOLUMESERVICE_H
#define GREENPHONEVOLUMSERVICER_H

#include <qtopiaipcadaptor.h>

class GreenphoneVolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    GreenphoneVolumeService();
    ~GreenphoneVolumeService();

public slots:
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

private slots:
    void registerService();
    void setCallDomain();

private:
    void adjustVolume(int leftChannel, int rightChannel, AdjustType);

    int m_leftChannelVolume;
    int m_rightChannelVolume;

    GreenphoneVolumeServicePrivate *m_d;
};

#endif
