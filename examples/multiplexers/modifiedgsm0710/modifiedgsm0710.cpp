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

#include "modifiedgsm0710.h"
#include <qgsm0710multiplexer.h>

bool ModifiedGsm0710MultiplexerPlugin::detect( QSerialIODevice *device )
{
    // Determine if the module supports a frame size of 256.
    return QGsm0710Multiplexer::cmuxChat( device, 256 );
}

QSerialIODeviceMultiplexer *ModifiedGsm0710MultiplexerPlugin::create
        ( QSerialIODevice *device )
{
    // Create the GSM 07.10 multiplexer with a frame size of 256.
    return new QGsm0710Multiplexer( device, 256 );
}

QTOPIA_EXPORT_PLUGIN( ModifiedGsm0710MultiplexerPlugin )
