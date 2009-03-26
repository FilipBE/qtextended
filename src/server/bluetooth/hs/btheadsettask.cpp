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

#include "btheadsettask.h"
#include "qtopiaserverapplication.h"
#include "qbluetoothhsservice_p.h"
#include "btaudiovolumemanager.h"
#include <qtopialog.h>

/*!
    \class BtHeadsetServiceTask
    \inpublicgroup QtBluetoothModule
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The BtHeadsetServiceTask class provides the Bluetooth Headset Service.

    The BtHeadsetService task manages the lifetime of a
    QBluetoothHeadsetService object.
  
    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.
 */

/*!
    Create a BtHeadsetService task with \a parent.
*/
BtHeadsetServiceTask::BtHeadsetServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Headset Service";
    m_hsService = new QBluetoothHeadsetService( "BluetoothHeadset", tr("Headset Audio Gateway"), this );
    new BtAudioVolumeManager( "BluetoothHeadset", this );
}

/*!
    Destructor.
*/
BtHeadsetServiceTask::~BtHeadsetServiceTask()
{
    delete m_hsService;
}

QTOPIA_TASK( BtHeadsetServiceTask, BtHeadsetServiceTask );
