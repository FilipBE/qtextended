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

#ifndef QTOPIANETWORK_H
#define QTOPIANETWORK_H

#include <qtopiaglobal.h>
#include <QString>
#include <QPointer>
#include <QVariant>

class QStringList;
class QtopiaNetworkInterface;
class QtopiaNetworkProperties;

class QTOPIA_EXPORT QtopiaNetwork
{
public:
    enum TypeFlag
    {
        LAN             = 0x00000001,    //network of type LAN
        WirelessLAN     = 0x00000002,    //network of type WLAN


        Dialup          = 0x00000004,    //analog dialup
        GPRS            = 0x00000008,    //GPRS/UMTS/EDGE dialup
#ifdef QTOPIA_CELL
        PhoneModem      = 0x00000010,    //network device builtin into the device (Phone Edition only)
#endif
        NamedModem      = 0x00000020,    //network device is named e.g. /dev/ttyS0
        PCMCIA          = 0x00000040,    //network device is a PCMCIA card


        Bluetooth       = 0x00001000,    //general Bluetooth marker
        BluetoothDUN    = 0X00002000,    //Dial-up Networking profile (DNP) client for Bluetooth
        BluetoothPAN    = 0x00004000,    //Personal Area Network profile (PAN) client for Bluetooth

        Custom          = 0x20000000,    //Customised type -> cfg must match plugin exactly
                                         //(see QtopiaNetworkFactoryIface::customID() for more details)
        Hidden          = 0x10000000,    //hidden network interface
        Any             = 0x00000000     //unknown type
    };

    Q_DECLARE_FLAGS(Type, TypeFlag)

    static bool online();
    static void startInterface( const QString& handle, const QVariant& options = QVariant() );
    static void stopInterface( const QString& handle, bool deleteIface = false);
    static void setDefaultGateway( const QString& handle );
    static void unsetDefaultGateway( const QString& handle );
    static void extendInterfaceLifetime( const QString& handle, bool isExtended );
    static void privilegedInterfaceStop( const QString& handle );
    static void shutdown();
    static void lockdown( bool isLocked );
    static Type toType( const QString& config );

    static QString settingsDir();
    static QStringList availableNetworkConfigs(QtopiaNetwork::Type type = QtopiaNetwork::Any,
            const QString& dir = QString());

    static QPointer<QtopiaNetworkInterface> loadPlugin( const QString& handle );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtopiaNetwork::Type);

#endif
