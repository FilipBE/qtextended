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

#include "phoneserver.h"
#include <QModemService>


class ModemTelephonyServiceFactory : public TelephonyServiceFactory
{
public:
    ModemTelephonyServiceFactory( QObject *parent = 0 ) ;

    QTelephonyService* service();
    QByteArray serviceName() const;
};

ModemTelephonyServiceFactory::ModemTelephonyServiceFactory( QObject* /*parent*/ )
{
}

QTelephonyService* ModemTelephonyServiceFactory::service()
{
    return QModemService::createVendorSpecific
            ("modem", QString(), 0 );
}

QByteArray ModemTelephonyServiceFactory::serviceName() const
{
    //synchronize with phoneserver.cpp 
    return QByteArray("ATModemService");
}

QTOPIA_TASK(ModemTelephonyServiceFactory, ModemTelephonyServiceFactory );
QTOPIA_TASK_PROVIDES(ModemTelephonyServiceFactory, TelephonyServiceFactory);
