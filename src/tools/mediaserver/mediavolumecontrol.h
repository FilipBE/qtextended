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

#ifndef MEDIAVOLUMECONTROL_H
#define MEDIAVOLUMECONTROL_H

#include <qtopiaipcadaptor.h>

namespace mediaserver
{

class SessionManager;

class MediaVolumeControlPrivate;

class MediaVolumeControl : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    MediaVolumeControl(SessionManager* sessionManager);
    ~MediaVolumeControl();

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMuted(bool mute);
    void setCallDomain(bool active);

private:
    MediaVolumeControlPrivate*  d;
};

} // ns mediaserver

#endif
