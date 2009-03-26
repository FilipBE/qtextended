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

#include "multiportmultiplexer.h"
#include <qmultiportmultiplexer.h>
#include <qserialport.h>

bool MultiPortMultiplexerPlugin::detect( QSerialIODevice * )
{
    return true;
}

QSerialIODeviceMultiplexer *MultiPortMultiplexerPlugin::create( QSerialIODevice *device )
{
    // The primary AT command device, /dev/mux0, is configured
    // in the custom.h file as QTOPIA_PHONE_DEVICE and then passed
    // down to us in the "device" parameter.
    QMultiPortMultiplexer *mux = new QMultiPortMultiplexer( device );

    // Add the secondary channel.
    QSerialPort *secondary = QSerialPort::create( "/dev/mux1" );
    mux->addChannel( "secondary", secondary );

    // Add the data channel.
    QSerialPort *data = QSerialPort::create( "/dev/mux2" );
    mux->addChannel( "data", data );

    // Add the data setup channel, which is the same as "data".
    mux->addChannel( "datasetup", data );
    return mux;
}

QTOPIA_EXPORT_PLUGIN( MultiPortMultiplexerPlugin )
