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

#include "bthandsfreetask.h"
#include "qtopiaserverapplication.h"
#include "qbluetoothhfservice_p.h"
#include "btaudiovolumemanager.h"
#include <qtopialog.h>

/*!
    \class BtHandsfreeServiceTask
    \inpublicgroup QtBluetoothModule
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The BtHandsfreeServiceTask class provides an implementation of the Bluetooth Handsfree Service.

    The BtHandsfreeServiceTask manages the lifetime of a
    QBluetoothHandsfreeService object.
  
    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.
 */

/*!
    Create a BtHandsfreeService task with \a parent.
*/
BtHandsfreeServiceTask::BtHandsfreeServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Handsfree Service";
    m_hfService = new QBluetoothHandsfreeService( "BluetoothHandsfree", tr("Handsfree Audio Gateway"), this );
    new BtAudioVolumeManager( "BluetoothHandsfree", this );
}

/*!
    Destructor.
*/
BtHandsfreeServiceTask::~BtHandsfreeServiceTask()
{
    delete m_hfService;
}

QTOPIA_TASK( BtHandsfreeServiceTask, BtHandsfreeServiceTask );
