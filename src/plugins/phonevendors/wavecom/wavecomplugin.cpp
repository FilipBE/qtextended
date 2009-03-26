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
#include "wavecomplugin.h"
#include "vendor_wavecom_p.h"


QTOPIA_EXPORT_PLUGIN( WavecomPluginImpl )

WavecomPluginImpl::WavecomPluginImpl() {}
WavecomPluginImpl::~WavecomPluginImpl() {}

bool WavecomPluginImpl::supports( const QString& manufacturer )
{
    return manufacturer.contains( "WAVECOM" );
}

QModemService *WavecomPluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new WavecomModemService( service, mux, parent );
}
