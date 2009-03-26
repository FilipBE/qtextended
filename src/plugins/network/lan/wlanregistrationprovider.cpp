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

#include "wlanregistrationprovider.h"

#ifndef NO_WIRELESS_LAN

#include <QAbstractIpcInterfaceGroup>

class WlanRegistrationInterface : public QWlanRegistration
{
    Q_OBJECT
public:

    WlanRegistrationInterface( const QString& serviceName, QObject* parent )
        : QWlanRegistration( serviceName, parent, QAbstractIpcInterface::Server )
    {
        setValue( "currentESSID", "" );
    }

    void setNewAccessPoint( const QString& essid )
    {
        if ( essid != value("currentESSID").toString() ) {
            setValue( "currentESSID", essid );
            emit accessPointChanged(); 
        }
    }
};

/*
   This class provides the back-end support for QWlanRegistration. It monitors the current ESSID
   and MAC and sends notifications when they change.
   */
WlanRegistrationProvider::WlanRegistrationProvider( const QString& serviceName, QObject* parent )
    : QAbstractIpcInterfaceGroup( serviceName, parent ), 
        wri(0), servName(serviceName)
{
}

WlanRegistrationProvider::~WlanRegistrationProvider()
{
}

/*
   Initializes this wlan provider
   */
void WlanRegistrationProvider::initialize()
{
    if ( !supports<QWlanRegistration>() ) {
        wri = new WlanRegistrationInterface( servName, this );
        addInterface( wri );
    }
    
    QAbstractIpcInterfaceGroup::initialize(); 
}

void WlanRegistrationProvider::setAccessPoint( const QString& sid )
{
    essid = sid;
}

void WlanRegistrationProvider::notifyClients()
{
    if ( wri )
        wri->setNewAccessPoint( essid );
}

#include "wlanregistrationprovider.moc"

#endif //NO_WIRELESS_LAN
