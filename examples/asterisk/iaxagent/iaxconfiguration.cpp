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

#include "iaxconfiguration.h"
#include "iaxtelephonyservice.h"

IaxConfiguration::IaxConfiguration( IaxTelephonyService *service )
    : QTelephonyConfiguration( service->service(), service, Server )
{
    this->service = service;
}

IaxConfiguration::~IaxConfiguration()
{
}

void IaxConfiguration::update( const QString& name, const QString& )
{
    // Process messages from the "iaxsettings" program for config updates.
    if ( name == "registration" )
        service->updateRegistrationConfig();
    else if ( name == "callerid" )
        service->updateCallerIdConfig();
}

void IaxConfiguration::request( const QString& name )
{
    // Not supported - just return an empty value.
    emit notification( name, QString() );
}
