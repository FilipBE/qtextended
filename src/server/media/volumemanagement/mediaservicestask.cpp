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
#include "mediaservicestask.h"


/*!
    \class VolumeControlTask
    \inpublicgroup QtMediaModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
    \ingroup QtopiaServer::Task

    \brief The VolumeControlTask class provides a task that manages services related to Media in Qt Extended This class is used to manage media related facilities in Qtopia. It watches
    media key events and forwards them to the appropriate party.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/


/*!
    \internal
*/

VolumeControlTask::VolumeControlTask()
{
    m_avm = new AudioVolumeManager();
    m_mks = new MediaKeyService(m_avm);
    connect(m_mks, SIGNAL(volumeChanged(bool)),
            this, SIGNAL(volumeChanged(bool)));
}

/*!
    \internal
*/

VolumeControlTask::~VolumeControlTask()
{
    delete m_mks;
    delete m_avm;
}

/*!
    \internal
    Connect key management with volume management
*/

void VolumeControlTask::setVolume(bool up)
{
    m_mks->setVolume(up);
}

/*!
    \fn VolumeControlTask::volumeChanged(bool)
    \internal
*/

QTOPIA_TASK(VolumeControlTask, VolumeControlTask);
QTOPIA_TASK_PROVIDES(VolumeControlTask, VolumeControlTask);

