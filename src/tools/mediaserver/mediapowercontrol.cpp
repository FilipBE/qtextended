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

#include "mediapowercontrol.h"


namespace mediaserver
{

/*!
    \class mediaserver::MediaPowerControl
    \internal
*/

MediaPowerControl::MediaPowerControl(QObject* parent):
    QObject(parent),
    m_powerConstraint(QtopiaApplication::Enable)
{
}

MediaPowerControl::~MediaPowerControl()
{
}

void MediaPowerControl::activeSessionCount(int activeSessions)
{
    if (activeSessions == 0)
        QtopiaApplication::setPowerConstraint(m_powerConstraint = QtopiaApplication::Enable);
    else
    {
        if (activeSessions > 0 && m_powerConstraint != QtopiaApplication::DisableSuspend)
            QtopiaApplication::setPowerConstraint(m_powerConstraint = QtopiaApplication::DisableSuspend);
    }
}

} // ns mediaserver
