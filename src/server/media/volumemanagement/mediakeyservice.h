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

#ifndef MEDIAKEYSERVICE_H
#define MEDIAKEYSERVICE_H

#include <qobject.h>
#include "qtopiainputevents.h"

class AudioVolumeManager;
class QValueSpaceItem;

class MediaKeyService :
    public QObject,
    public QtopiaKeyboardFilter
{
    Q_OBJECT

    static const int INCREMENT = 2;

public:
    MediaKeyService(AudioVolumeManager* avm);
    ~MediaKeyService();

    void setVolume(bool up);

signals:
    void volumeChanged(bool up);

private:
    bool filter(int unicode, int keycode, int modifiers,
                bool press, bool autoRepeat);

    void timerEvent(QTimerEvent* timerEvent);

    bool keyLocked();

    int                 m_increment;
    int                 m_repeatTimerId;
    int                 m_repeatKeyCode;
    AudioVolumeManager* m_avm;
    bool                m_keyLocked;
    QValueSpaceItem*    m_vs;
};

#endif
