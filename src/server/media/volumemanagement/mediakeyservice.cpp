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


#include "audiovolumemanager.h"
#include "mediakeyservice.h"
#include "qtopiapowermanager.h"
#include <qevent.h>
#include <QValueSpaceItem>
#include <QAudioMixer>
#include <QTimer>


MediaKeyService::MediaKeyService(AudioVolumeManager* avm):
    m_increment(INCREMENT),
    m_repeatTimerId(-1),
    m_repeatKeyCode(-1),
    m_avm(avm)
{
    m_vs = new QValueSpaceItem("/UI", this);
    QtopiaInputEvents::addKeyboardFilter(this);
}

MediaKeyService::~MediaKeyService()
{
}

//private:
bool MediaKeyService::filter
(
 int unicode,
 int keycode,
 int modifiers,
 bool press,
 bool autoRepeat
)
{
    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);

    bool    rc = false;

    if (keyLocked())
        return rc;
    // TODO: Configurable key/function matching
    if (keycode == Qt::Key_VolumeUp || keycode == Qt::Key_VolumeDown)
    {
        if (m_repeatKeyCode != -1 && autoRepeat)
        {
            rc = true;
        }
        else
        {
            rc = m_avm->canManageVolume();
            if (rc)
            {
                if (autoRepeat)
                {
                    m_repeatKeyCode = keycode;
                    m_repeatTimerId = startTimer(200);
                }
                else
                {
                    if (press)
                    {
                        switch (keycode)
                        {
                        case Qt::Key_VolumeUp:
                            m_avm->increaseVolume(1);
                            break;

                        case Qt::Key_VolumeDown:
                            m_avm->decreaseVolume(1);
                            break;
                        }
                    }
                    else
                    {
                        if (m_repeatKeyCode != -1)
                        {
                            killTimer(m_repeatTimerId);
                            m_repeatKeyCode = -1;
                            m_repeatTimerId = -1;
                            m_increment = 1;
                        }
                    }
                }
            }
        }
    } else if (keycode == Qt::Key_VolumeMute) {
        rc = m_avm->canManageVolume();

        if (rc && press)
            m_avm->toggleMuted();
    }

    if (rc)
        QtopiaPowerManager::setActive(false);

    return rc;
}

void MediaKeyService::timerEvent(QTimerEvent* timerEvent)
{
    if (m_repeatTimerId == timerEvent->timerId())
    {
        switch (m_repeatKeyCode)
        {
        case Qt::Key_VolumeUp:
            m_avm->increaseVolume(1);
            break;

        case Qt::Key_VolumeDown:
            m_avm->decreaseVolume(1);
            break;
        }

        m_increment += INCREMENT;
    }
}

/*! \internal */
bool MediaKeyService::keyLocked()
{
    return m_vs->value("KeyLock").toBool() || m_vs->value("SimLock").toBool();
}

void MediaKeyService::setVolume(bool up)
{
    if ( up )
        m_avm->increaseVolume(m_increment);
    else
        m_avm->decreaseVolume(m_increment);
}
