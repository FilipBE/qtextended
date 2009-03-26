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

#include "btdialupservice.h"

#include <qtopianamespace.h>
#include <qbluetoothsdprecord.h>
#include <QFile>

/*!
  \class BtDialupServiceTask
    \inpublicgroup QtBluetoothModule
  \brief The BtDialupServiceTask class provides server side support for the Bluetooth
  DUN profile.
  \ingroup QtopiaServer::Task::Bluetooth

  This task listens for incoming Bluetooth DUN connections, forwards the request to
  the modem emulator and manages the life time of these connections. This task relies
  on QBluetoothSerialPortService.

  The BtDialupServiceTask class provides the \c {BtDialupServiceTask} task.
  This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.

  \sa QBluetoothSerialPortService
  */

/*!
  Constructs the BtDialupServiceTask instance with the given \a parent.
  */
BtDialupServiceTask::BtDialupServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Bluetooth DialupService";

    QFile file(Qtopia::qtopiaDir() + "etc/bluetooth/sdp/dun.xml");
    file.open(QIODevice::ReadOnly);
    QBluetoothSdpRecord record = QBluetoothSdpRecord::fromDevice(&file);
    if (record.isNull())
        qWarning() << "BtDialupServiceTask: cannot read" << file.fileName();

    provider = new QBluetoothSerialPortService( QLatin1String("DialupNetworking"),
            tr("Dial-up Networking"),
            record,
            this );
}

/*!
  Destroys the BtDialupServiceTask instance.
  */
BtDialupServiceTask::~BtDialupServiceTask()
{
}

QTOPIA_TASK(BtDialupServiceTask,BtDialupServiceTask);
#include "btdialupservice.moc"

