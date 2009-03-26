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

#include "bluetoothplugin.h"
#include "bluetoothimpl.h"

#include <QPointer>
#include <qtopialog.h>
#include <qtopiaapplication.h>

BluetoothPlugin::BluetoothPlugin()
{
    //Load translation for libqtopianetwork
    QtopiaApplication::loadTranslations( "libqtopianetwork" );
}

BluetoothPlugin::~BluetoothPlugin()
{
    qLog(Network) << "Deleting BluetoothPlugin (" << instances.count()
        << " instances)";
    //cleanup all interface instances
    while ( !instances.isEmpty() )
    {
        QPointer<QtopiaNetworkInterface> impl = instances.takeFirst();
        if (impl)
            delete impl;
    }
}

QPointer<QtopiaNetworkInterface> BluetoothPlugin::network( const QString& confFile)
{
    qLog(Network) << "new Bluetooth interface instance requested -> " << confFile;
    QPointer<QtopiaNetworkInterface> impl = new BluetoothImpl( confFile );
    instances.append(impl);

    return impl;
}

QtopiaNetwork::Type BluetoothPlugin::type() const
{
    return ( QtopiaNetwork::Bluetooth| QtopiaNetwork::BluetoothDUN );
}

QByteArray BluetoothPlugin::customID() const
{
    return QByteArray();
}
QTOPIA_EXPORT_PLUGIN( BluetoothPlugin );
