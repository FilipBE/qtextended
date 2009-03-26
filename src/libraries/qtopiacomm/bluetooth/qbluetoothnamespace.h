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

#ifndef QBLUETOOTHNAMESPACE_H
#define QBLUETOOTHNAMESPACE_H

#include <QtGlobal>
#include <QMetaType>
#include <QFlags>

#include <qbluetoothglobal.h>
#ifdef QTOPIA_BLUETOOTH
#include <qtopiaipcmarshal.h>
#endif

#include <string.h>

class QString;
#ifndef Q_QDOC
namespace QBluetooth
{
#else
class QBluetooth
{
public:
#endif
    enum DeviceMajor {
        Miscellaneous = 0,
        Computer = 1,
        Phone = 2,
        LANAccess = 3,
        AudioVideo = 4,
        Peripheral = 5,
        Imaging = 6,
        Wearable = 7,
        Toy = 8,
        Uncategorized = 9,
    };

    enum SDPProfile {
        SerialPortProfile,
        DialupNetworkingProfile,
        LanAccessProfile,
        FaxProfile,
        ObjectPushProfile,
        FileTransferProfile,
        DirectPrintingProfile,
        HeadsetProfile,
        HeadsetAudioGatewayProfile,
        HandsFreeProfile,
        HandsFreeAudioGatewayProfile,
        SimAccessProfile,
        NetworkAccessPointProfile,
        GroupAdHocNetworkProfile,
        PersonalAreaNetworkUserProfile,
        HardCopyReplacementProfile,
        AdvancedAudioSourceProfile,
        AdvancedAudioSinkProfile,
        AudioVideoRemoteControlProfile,
        AudioVideoTargetProfile
    };

    enum ServiceClass {
        Positioning = 0x1,
        Networking = 0x2,
        Rendering = 0x4,
        Capturing = 0x8,
        ObjectTransfer = 0x10,
        Audio = 0x20,
        Telephony = 0x40,
        Information = 0x80,
        AllServiceClasses = 0xffff
    };

    Q_DECLARE_FLAGS(ServiceClasses, ServiceClass)

    enum SecurityOption {
        Authenticated = 0x1,
        Encrypted = 0x2,
        Secure = 0x4
    };

    Q_DECLARE_FLAGS(SecurityOptions, SecurityOption)
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetooth::ServiceClasses)
Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetooth::SecurityOptions)

struct QBLUETOOTH_EXPORT quint128 {
    quint128();
    quint128(const quint8 indata[16]);

    quint8 data[16];

    bool operator==(const quint128 &other) const
    {
        return memcmp(data, other.data, 16) == 0;
    }

    bool operator!=(const quint128 &other) const
    {
        return memcmp(data, other.data, 16) != 0;
    }
};

struct QBLUETOOTH_EXPORT qint128 {
    qint128();
    qint128(const quint8 indata[]);

    quint8 data[16];

    bool operator==(const qint128 &other) const
    {
        return memcmp(data, other.data, 16) == 0;
    }

    bool operator!=(const qint128 &other) const
    {
        return memcmp(data, other.data, 16) != 0;
    }
};

#ifdef QTOPIA_BLUETOOTH
Q_DECLARE_USER_METATYPE_ENUM(QBluetooth::SDPProfile)
Q_DECLARE_USER_METATYPE_ENUM(QBluetooth::SecurityOptions)
#endif

Q_DECLARE_METATYPE(qint128)
Q_DECLARE_METATYPE(quint128)
Q_DECLARE_METATYPE(qint8) //TODO: Should Qt support this out of the box?

#endif
