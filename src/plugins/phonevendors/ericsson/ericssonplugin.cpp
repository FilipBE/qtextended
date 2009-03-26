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
#include "ericssonplugin.h"
#include "vendor_ericsson_p.h"


QTOPIA_EXPORT_PLUGIN( EricssonPluginImpl )

EricssonPluginImpl::EricssonPluginImpl()
{
    trolltechExtensions = false;
}


EricssonPluginImpl::~EricssonPluginImpl()
{
}


bool EricssonPluginImpl::supports( const QString& manufacturer )
{
    trolltechExtensions = manufacturer.contains( "QT EXTENDED" );
    return manufacturer.contains( "Ericsson" ) ||
           manufacturer.contains( "QT EXTENDED" );    // for phonesim support
}

QModemService *EricssonPluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new EricssonModemService( service, mux, parent, trolltechExtensions );
}
