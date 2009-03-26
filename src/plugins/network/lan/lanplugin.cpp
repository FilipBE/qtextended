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

#include "lanplugin.h"

#include <QPointer>
#include <qtopialog.h>
#include <qtopiaapplication.h>

LanPlugin::LanPlugin()
{
    //Load translation for libqtopianetwork
    QtopiaApplication::loadTranslations( "libqtopianetwork" );
}

LanPlugin::~LanPlugin()
{
    qLog(Network) << "Deleting LanPlugin (" << instances.count()
        << " instances)";
    //cleanup all interface instances
    while ( !instances.isEmpty() )
    {
        QPointer<QtopiaNetworkInterface> impl = instances.takeFirst();
        if (impl)
            delete impl;
    }
}

QPointer<QtopiaNetworkInterface> LanPlugin::network( const QString& confFile)
{
    qLog(Network) << "new Lan interface instance requested -> " << confFile;
    QPointer<QtopiaNetworkInterface> impl = new LanImpl( confFile );
    instances.append(impl);

    return impl;
}

QtopiaNetwork::Type LanPlugin::type() const
{
    return ( QtopiaNetwork::LAN |
#ifndef NO_WIRELESS_LAN
                QtopiaNetwork::WirelessLAN |
#endif
                QtopiaNetwork::PCMCIA );
}

QByteArray LanPlugin::customID() const
{
    return QByteArray();
}

QTOPIA_EXPORT_PLUGIN( LanPlugin );
