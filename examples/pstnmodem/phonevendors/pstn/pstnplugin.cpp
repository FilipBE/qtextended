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

#include "pstnplugin.h"
#include "vendor_pstn_p.h"

QTOPIA_EXPORT_PLUGIN( PstnPluginImpl )

PstnPluginImpl::PstnPluginImpl()
{
}

PstnPluginImpl::~PstnPluginImpl()
{
}

bool PstnPluginImpl::supports( const QString& manufacturer )
{
    // AT+CGMI will never return anything interesting for this modem type
    // because such modems are not GSM-compatible.  The plugin will need
    // to be explicitly enabled using QTOPIA_PHONE_VENDOR.
    Q_UNUSED(manufacturer);
    return false;
}

QModemService *PstnPluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new PstnModemService( service, mux, parent );
}
