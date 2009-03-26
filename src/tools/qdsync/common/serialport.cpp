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
#include "serialport.h"

#include <trace.h>
QD_LOG_OPTION(Modem)

#include <QTimer>

SerialPort::SerialPort( const QString &port )
    : QSerialPort( port, 9600, true ), up( false )
{
    connect( this, SIGNAL(dsrChanged(bool)), this, SLOT(slotDsrChanged(bool)) );
    connect( this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
}

SerialPort::~SerialPort()
{
    TRACE(Modem) << "SerialPort::~SerialPort";
}

void SerialPort::slotDsrChanged( bool dsrUp )
{
    TRACE(Modem) << "SerialPort::slotDsrChanged" << "DSR" << dsrUp << "up" << up;
    if ( dsrUp ) {
        if ( !up ) {
            up = true;
            emit newConnection();
        }
    } else {
        if ( up ) {
            emit disconnected();
            up = false;
        }
    }
}

bool SerialPort::open( QIODevice::OpenMode mode )
{
    TRACE(Modem) << "SerialPort::open";
    bool ret;

    ret = QSerialPort::open( mode );
    up = false;

    if ( ret && dsr() ) {
        up = true;
        QTimer::singleShot( 0, this, SIGNAL(newConnection()) );
    }

    return ret;
}

void SerialPort::slotReadyRead()
{
    TRACE(Modem) << "SerialPort::slotReadyRead";
    // We might not get a DSR so bring up the connection now
    LOG() << "up" << up;
    if ( !up ) {
        up = true;
        LOG() << "coming up (data received), emit newConnection";
        emit newConnection();
        // We need to emit this again so the QIODeviceWrapper can see it!
        emit readyRead();
    }
    // We've brought the connection up so stop listening for the signal
    disconnect( this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
}

