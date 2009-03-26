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

#ifndef GENERICVOLUMESERVICE_H
#define GENERICVOLUMESERVICE_H

#include <qtopiaipcadaptor.h>

class GenericVolumeServicePrivate;

class GenericVolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    GenericVolumeService();
    ~GenericVolumeService();

public slots:
    void setVolume(int volume);
    void setVolume(int leftChannel, int rightChannel);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

private slots:
    void registerService();
    void setCallDomain();
    void updateVolume();
    void timeout();

private:
    void adjustVolume(int leftChannel, int rightChannel, AdjustType);

    int m_leftChannelVolume;
    int m_rightChannelVolume;

    GenericVolumeServicePrivate *m_d;
};

#endif
