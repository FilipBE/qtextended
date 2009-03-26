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

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothremotedevice.h>
#include "qbluetoothnamespace_p.h"

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QList>

using namespace QBluetooth;

/*!
    \class QBluetooth
    \inpublicgroup QtBluetoothModule

    \brief The QBluetooth namespace contains miscellaneous Bluetooth functionality.

    The QBluetooth namespace defines various functions and enums
    that are used globally by the Bluetooth library.
*/

/*!
    \enum QBluetooth::SecurityOption

    Defines possible security settings for Bluetooth L2CAP and RFCOMM
    connections.

    \value Authenticated The connection is authenticated.
    \value Encrypted The connection is encrypted.
    \value Secure The is secure.  Generally this means that the
    connection is authenticated and encrypted.  Please
    note that the meaning of this value is implementation dependent, it
    is more portable to use the Authenticated and Encrypted options.
*/

/*!
    \enum QBluetooth::SDPProfile
    Defines possible profiles in use by the system.

    \value SerialPortProfile Represents Serial Port profile.
    \value DialupNetworkingProfile Represents Dialup Networking profile.
    \value LanAccessProfile Represents LAN Access profile.
    \value FaxProfile Represents FAX profile.
    \value ObjectPushProfile Represents OBEX Object Push profile.
    \value FileTransferProfile Represents OBEX File Transfer profile.
    \value DirectPrintingProfile Represents the OBEX Simple Printing Profile (SPP), Direct Printing service.
    \value HeadsetProfile Represents the Headset profile.
    \value HandsFreeProfile Represents the Hands Free profile.
    \value SimAccessProfile Represents the SIM Access profile.
    \value NetworkAccessPointProfile Represents the PAN NAP profile.
    \value GroupAdHocNetworkProfile Represents the PAN GN profile.
    \value PersonalAreaNetworkUserProfile Represents the PAN PANU profile.
    \value HardCopyReplacementProfile Represents the HCRP profile.
    \value AdvancedAudioSourceProfile Represents the A2DP Source profile.
    \value AdvancedAudioSinkProfile Represents the A2DP Sink profile.
    \value AudioVideoRemoteControlProfile Represents the AVRCP Controller (CT) profile.
    \value AudioVideoTargetProfile Represents the AVRCP Target (TG) profile.
    \value HeadsetAudioGatewayProfile Represents the HS AG profile.
    \value HandsFreeAudioGatewayProfile Represents the HF AG profile.
 */

/*!
    \enum QBluetooth::ServiceClass
    Defines the service classes.

    \value Positioning Device has Positioning services (Location identification)
    \value Networking Device has Networking services (LAN, Ad hoc, ...)
    \value Rendering Device has Rendering services (Printing, Speaker, ...)
    \value Capturing Device has Capturing services (Scanner, Microphone, ...)
    \value ObjectTransfer Device has Object Transfer services (Object Push, FTP)
    \value Audio Device has Audio services (Speaker, Microphone, Headset service, ...)
    \value Telephony Device has Telephony services (Cordless telephony, Modem, Headset service, ...)
    \value Information Device has Information services (WEB-server, WAP-server, ...)
    \value AllServiceClasses Special value that represents all service     classes.
*/

/*!
    \enum QBluetooth::DeviceMajor
    Defines the major class of a Bluetooth device.

    \value Miscellaneous Miscellaneous device
    \value Computer Computer device
    \value Phone Phone device
    \value LANAccess Some form of a local area network router
    \value AudioVideo Audio / Video device
    \value Peripheral Mouse, Joystick, Keyboard
    \value Imaging Camera or Scanner device
    \value Wearable Wearable Device
    \value Toy Toy device
    \value Uncategorized Uncategorized device
*/


// Device class consists of 24 bits
// 11 LSB bits are Major service class
// Followed by 5 MajorNumber bits
// Followed by 6 MinorNumber bits
// Followed by 2 format bits
// e.g.

// 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
// .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..

// Bits 23-13 Are reserved for service class
// Bits 12-8 Are reserved for major device number
// Bits 7-2 Are reserved for minor device number
// Bits 1-0 are unused?
//
// Stolen from hcid manual page, with comments by DK
// Major service class byte allocation (from LSB to MSB):
//       Bit 1 (16): Positioning (Location identification)
//       Bit 2 (17): Networking (LAN, Ad hoc, ...)
//       Bit 3 (18): Rendering (Printing, Speaker, ...)
//       Bit 4 (19): Capturing (Scanner, Microphone, ...)
//       Bit 5 (20): Object Transfer (v-Inbox, v-Folder, ...)
//       Bit 6 (21): Audio (Speaker, Microphone, Headset service, ...)
//       Bit 7 (22): Telephony (Cordless telephony, Modem, Headset service, ...)
//       Bit 8 (23): Information (WEB-server, WAP-server, ...)

// These are bits 7, 6, 5, 4, 3, 2
const char *computer_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized"),
    QT_TRANSLATE_NOOP("QBluetooth","Desktop workstation"),
    QT_TRANSLATE_NOOP("QBluetooth","Server-class computer"),
    QT_TRANSLATE_NOOP("QBluetooth","Laptop"),
    QT_TRANSLATE_NOOP("QBluetooth","Handheld PC/PDA (clam shell)"),
    QT_TRANSLATE_NOOP("QBluetooth","Palm sized PC/PDA"),
    QT_TRANSLATE_NOOP("QBluetooth","Wearable computer (Watch sized)"),
    NULL
};

// These are bits 7, 6, 5, 4, 3, 2
const char *phone_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized"),
    QT_TRANSLATE_NOOP("QBluetooth","Cellular"),
    QT_TRANSLATE_NOOP("QBluetooth","Cordless"),
    QT_TRANSLATE_NOOP("QBluetooth","Smart phone"),
    QT_TRANSLATE_NOOP("QBluetooth","Wired modem or voice gateway"),
    QT_TRANSLATE_NOOP("QBluetooth","Common ISDN Access"),
    NULL
};

// These are bits 7, 6, 5
const char *lan_access_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Fully available"),
    QT_TRANSLATE_NOOP("QBluetooth","17% utilized"),
    QT_TRANSLATE_NOOP("QBluetooth","17-33% utilized"),
    QT_TRANSLATE_NOOP("QBluetooth","33-50% utilized"),
    QT_TRANSLATE_NOOP("QBluetooth","50-67% utilized"),
    QT_TRANSLATE_NOOP("QBluetooth","67-83% utilized"),
    QT_TRANSLATE_NOOP("QBluetooth","83-99% utilized"),
    QT_TRANSLATE_NOOP("QBluetooth","No service available"),
    NULL
};

// These are bits 7, 6, 5, 4, 3, 2
const char *audio_video_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized, code not assigned"),
    QT_TRANSLATE_NOOP("QBluetooth","Wearable Headset Device"),
    QT_TRANSLATE_NOOP("QBluetooth","Hands-free Device"),
    QT_TRANSLATE_NOOP("QBluetooth","(Reserved)"),
    QT_TRANSLATE_NOOP("QBluetooth","Microphone"),
    QT_TRANSLATE_NOOP("QBluetooth","Loudspeaker"),
    QT_TRANSLATE_NOOP("QBluetooth","Headphones"),
    QT_TRANSLATE_NOOP("QBluetooth","Portable Audio"),
    QT_TRANSLATE_NOOP("QBluetooth","Car audio"),
    QT_TRANSLATE_NOOP("QBluetooth","Set-top box"),
    QT_TRANSLATE_NOOP("QBluetooth","HiFi Audio Device"),
    QT_TRANSLATE_NOOP("QBluetooth","VCR"),
    QT_TRANSLATE_NOOP("QBluetooth","Video Camera"),
    QT_TRANSLATE_NOOP("QBluetooth","Camcorder"),
    QT_TRANSLATE_NOOP("QBluetooth","Video Monitor"),
    QT_TRANSLATE_NOOP("QBluetooth","Video Display and Loudspeaker"),
    QT_TRANSLATE_NOOP("QBluetooth","Video Conferencing"),
    QT_TRANSLATE_NOOP("QBluetooth","(Reserved)"),
    QT_TRANSLATE_NOOP("QBluetooth","Gaming/Toy"),
    NULL
};

//Bits 7,6
const char *peripheral_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Not Keyboard / Not Pointing Device"),
    QT_TRANSLATE_NOOP("QBluetooth","Keyboard"),
    QT_TRANSLATE_NOOP("QBluetooth","Pointing device"),
    QT_TRANSLATE_NOOP("QBluetooth","Combo keyboard/pointing device"),
    NULL
};

// Bits 5, 4, 3, 2 (combination with the above)
// E.g. can be Keyboard + Gamepad or something
const char *peripheral_device_field_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized device"),
    QT_TRANSLATE_NOOP("QBluetooth","Joystick"),
    QT_TRANSLATE_NOOP("QBluetooth","Gamepad"),
    QT_TRANSLATE_NOOP("QBluetooth","Remote control"),
    QT_TRANSLATE_NOOP("QBluetooth","Sensing device"),
    QT_TRANSLATE_NOOP("QBluetooth","Digitizer tablet"),
    QT_TRANSLATE_NOOP("QBluetooth","Card Reader"),
    NULL
};

// Bits 7, 6, 5, 4
const char *imaging_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized"),
    QT_TRANSLATE_NOOP("QBluetooth","Display"),
    QT_TRANSLATE_NOOP("QBluetooth","Camera"),
    QT_TRANSLATE_NOOP("QBluetooth","Camera"),
    QT_TRANSLATE_NOOP("QBluetooth","Scanner"),
    QT_TRANSLATE_NOOP("QBluetooth","Scanner"),
    QT_TRANSLATE_NOOP("QBluetooth","Scanner"),
    QT_TRANSLATE_NOOP("QBluetooth","Scanner"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    QT_TRANSLATE_NOOP("QBluetooth","Printer"),
    NULL
};

// These are bits 7, 6, 5, 4, 3, 2
const char *wearable_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized"),
    QT_TRANSLATE_NOOP("QBluetooth","Wrist Watch"),
    QT_TRANSLATE_NOOP("QBluetooth","Pager"),
    QT_TRANSLATE_NOOP("QBluetooth","Jacket"),
    QT_TRANSLATE_NOOP("QBluetooth","Helmet"),
    QT_TRANSLATE_NOOP("QBluetooth","Glasses"),
    NULL
};

const char *toy_minor_classes[] = {
    QT_TRANSLATE_NOOP("QBluetooth","Uncategorized"),
    QT_TRANSLATE_NOOP("QBluetooth","Robot"),
    QT_TRANSLATE_NOOP("QBluetooth","Vehicle"),
    QT_TRANSLATE_NOOP("QBluetooth","Doll / Action Figure"),
    QT_TRANSLATE_NOOP("QBluetooth","Controller"),
    QT_TRANSLATE_NOOP("QBluetooth","Game"),
    NULL
};

static const char *minor_to_str(qint8 minor, const char *minors[])
{
    int i = 0;
    while (minors[i]) {
        i++;
    }

    if (minor < i) {
        return minors[minor];
    }

    return NULL;
}

/*!
    \internal
    Converts a device minor class to a String.  This is here since both
    QBluetoothLocalDevice and QBluetoothRemoteDevice use this.
*/
QString convertDeviceMinorToString(QBluetooth::DeviceMajor major, qint8 minor)
{
    const char *m;
    QString ret = QObject::tr("Uncategorized");

    switch(major) {
        case Miscellaneous:
            break;
        case Computer:
            m = minor_to_str(minor & 0x3F, computer_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case Phone:
            m = minor_to_str(minor & 0x3F, phone_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case LANAccess:
            m = minor_to_str((minor >> 3) & 0x7, lan_access_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case AudioVideo:
            m = minor_to_str(minor & 0x3F, audio_video_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case Peripheral:
            m = minor_to_str((minor >> 4) & 0x3, peripheral_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case Imaging:
            m = minor_to_str((minor >> 2) & 0xF, imaging_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case Wearable:
            m = minor_to_str(minor & 0x3F, wearable_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case Toy:
            m = minor_to_str(minor & 0x3F, toy_minor_classes);
            if (m != NULL) {
                ret = qApp->translate("QBluetooth",m);
            }
            break;
        case Uncategorized:
            ret = QObject::tr("Uncategorized");
            break;
        default:
            ret = QObject::tr("Invalid");
            break;
    };

    return ret;
}

/*! \internal
    Converts a device class to a String.  This is here since both QBluetoothLocalDevice
    and QBluetoothRemoteDevice use this.

    \ingroup qtopiabluetooth
 */
QString convertDeviceMajorToString(QBluetooth::DeviceMajor dev_class)
{
    QString ret;

    switch(dev_class) {
        case Miscellaneous:
            ret = QObject::tr("Miscellaneous");
            break;
        case Computer:
            ret = QObject::tr("Computer");
            break;
        case Phone:
            ret = QObject::tr("Phone");
            break;
        case LANAccess:
            ret = QObject::tr("LAN Access");
            break;
        case AudioVideo:
            ret = QObject::tr("Audio / Video");
            break;
        case Peripheral:
            ret = QObject::tr("Peripheral");
            break;
        case Imaging:
            ret = QObject::tr("Imaging");
            break;
        case Wearable:
            ret = QObject::tr("Wearable");
            break;
        case Toy:
            ret = QObject::tr("Toy");
            break;
        case Uncategorized:
            ret = QObject::tr("Uncategorized");
            break;
        default:
            ret = QObject::tr("Invalid");
            break;
    };

    return ret;
}

/*! \internal
    Converts a union of service classes to a list of strings.  This service class
    is different from the Major class.  This is used as a rudimentary form of the
    service discovery protocol, it is meant to provide a rudimentary understanding
    of what services the device might provide.

    \ingroup qtopiabluetooth
 */
QStringList convertServiceClassesToString(QBluetooth::ServiceClasses classes)
{
    QStringList ret;

    if (classes & Positioning) {
        ret.append(QObject::tr("Positioning"));
    }

    if (classes & Networking) {
        ret.append(QObject::tr("Networking"));
    }

    if (classes & Rendering) {
        ret.append(QObject::tr("Rendering"));
    }

    if (classes & Capturing) {
        ret.append(QObject::tr("Capturing"));
    }

    if (classes & ObjectTransfer) {
        ret.append(QObject::tr("Object Transfer"));
    }

    if (classes & Audio) {
        ret.append(QObject::tr("Audio"));
    }

    if (classes & Telephony) {
        ret.append(QObject::tr("Telephony"));
    }

    if (classes & Information) {
        ret.append(QObject::tr("Information"));
    }

    return ret;
}

QBluetooth::DeviceMajor QTOPIA_AUTOTEST_EXPORT major_to_device_major(quint8 major)
{
    if (major > 9)
        return QBluetooth::Uncategorized;

    return static_cast<QBluetooth::DeviceMajor>(major);
}

#ifdef QTOPIA_BLUETOOTH
//TODO: Need to move this to the BluetoothGui Library
/*!
    \internal

    Returns the device icon for a particular remote device.

    This uses the deviceMajor and deviceMinor information
    to come up with the icon.

    This ideally should be moved to the bluetooth ui library.
*/
QIcon find_device_icon(const QBluetoothRemoteDevice &remote)
{
    return find_device_icon(remote.deviceMajor(), remote.deviceMinor(), remote.serviceClasses());
}

/*!
    \internal
    \overload

    Returns the device icon for the device represented by the given \a major,
    \a minor and \a serviceClasses.
*/
QIcon find_device_icon(QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses)
{
    Q_UNUSED(serviceClasses);   // would this ever be used for the icon?

    if (major == QBluetooth::Computer)
        return QIcon(":icon/icons/computer");
    if (major == QBluetooth::Phone)
        return QIcon(":icon/phone/phone");

    // until device minor are available in QBluetooth namespace,
    // just compare the untranslated device minor strings to return
    // more specific device icons
    const char *untranslatedMinor;
    switch (major) {
        case AudioVideo:
            untranslatedMinor = minor_to_str(minor & 0x3F, audio_video_minor_classes);
            if (strcmp(untranslatedMinor, "Headphones") == 0)
                return QIcon(":icon/icons/headset");
            else if (strcmp(untranslatedMinor, "Wearable Headset Device") == 0)
                return QIcon(":icon/icons/headset");
            break;
        case Imaging:
            untranslatedMinor = minor_to_str((minor >> 2) & 0xF, imaging_minor_classes);
            if (strcmp(untranslatedMinor, "Printer") == 0)
                return QIcon(":icon/icons/print");
            else if (strcmp(untranslatedMinor, "Camera") == 0)
                return QIcon(":icon/icons/camera");
            break;
        default:
            break;
    }

    return QIcon();
}
#endif

/*!
    \class qint128
    \inpublicgroup QtBluetoothModule
    \internal
*/

/*!
    \internal
*/
qint128::qint128()
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = 0;
}

/*!
    \internal
*/
qint128::qint128(const quint8 indata[16])
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = indata[i];
}

/*!
    \class quint128
    \inpublicgroup QtBluetoothModule
    \internal
*/

/*!
    \internal
*/
quint128::quint128()
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = 0;
}

/*!
    \internal
*/
quint128::quint128(const quint8 indata[16])
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = indata[i];
}

/*!
    \internal

    Converts a QString representation of a Bluetooth address to a bdaddr_t structure
    Used by BlueZ
 */
void str2bdaddr(const QString &addr, bdaddr_t *bdaddr)
{
    unsigned char b[6];

    b[5] = addr.mid(0, 2).toUShort(0, 16);
    b[4] = addr.mid(3, 2).toUShort(0, 16);
    b[3] = addr.mid(6, 2).toUShort(0, 16);
    b[2] = addr.mid(9, 2).toUShort(0, 16);
    b[1] = addr.mid(12, 2).toUShort(0, 16);
    b[0] = addr.mid(15, 2).toUShort(0, 16);

    memcpy(bdaddr, b, 6);
}

static const char hex_table[]={'0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'};

#define CONVERT(dest, offset, byte)\
    dest[offset+1] = hex_table[byte & 0x0f]; \
    dest[offset] = hex_table[(byte >> 4) & 0x0f]; \

/*!
    \internal

    Converts a bdaddr_t structure used by BlueZ to a string representation.
 */
QString bdaddr2str(const bdaddr_t *bdaddr)
{
    QByteArray arr("00:00:00:00:00:00");
    unsigned char b[6];
    memcpy(b, bdaddr, 6);

    CONVERT(arr, 15, b[0])
    CONVERT(arr, 12, b[1])
    CONVERT(arr, 9, b[2])
    CONVERT(arr, 6, b[3])
    CONVERT(arr, 3, b[4])
    CONVERT(arr, 0, b[5])

    return arr;
}

/*!
    \internal

    Get the security options.
*/
bool _q_getSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options)
{
    int lm = 0;
    socklen_t len = sizeof(lm);

    if (getsockopt(sockfd, SOL_RFCOMM, RFCOMM_LM, &lm, &len) < 0) {
        options = 0;
        return false;
    }

    if (lm & RFCOMM_LM_AUTH)
        options |= QBluetooth::Authenticated;
    if (lm & RFCOMM_LM_ENCRYPT)
        options |= QBluetooth::Encrypted;
    if (lm & RFCOMM_LM_SECURE)
        options |= QBluetooth::Secure;

    return options;
}

/*!
    \internal

    Set the security options.
 */
bool _q_setSecurityOptions(int sockfd, QBluetooth::SecurityOptions options)
{
    int lm = 0;

    if (options & QBluetooth::Authenticated)
        lm |= RFCOMM_LM_AUTH;
    if (options & QBluetooth::Encrypted)
        lm |= RFCOMM_LM_ENCRYPT;
    if (options & QBluetooth::Secure)
        lm |= RFCOMM_LM_SECURE;

    if (setsockopt(sockfd, SOL_RFCOMM, RFCOMM_LM, &lm, sizeof(lm)) < 0)
    {
        return false;
    }

    return true;
}

/*!
    \internal

    Get the L2CAP security options.
 */
bool _q_getL2CapSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options)
{
    int lm = 0;
    socklen_t len = sizeof(lm);

    if (getsockopt(sockfd, SOL_L2CAP, L2CAP_LM, &lm, &len) < 0) {
        options = 0;
        return false;
    }

    if (lm & L2CAP_LM_AUTH)
        options |= QBluetooth::Authenticated;
    if (lm & L2CAP_LM_ENCRYPT)
        options |= QBluetooth::Encrypted;
    if (lm & L2CAP_LM_SECURE)
        options |= QBluetooth::Secure;

    return options;
}

/*!
    \internal

    Set the L2CAP security options.
 */
bool _q_setL2CapSecurityOptions(int sockfd, QBluetooth::SecurityOptions options)
{
    int lm = 0;

    if (options & QBluetooth::Authenticated)
        lm |= L2CAP_LM_AUTH;
    if (options & QBluetooth::Encrypted)
        lm |= L2CAP_LM_ENCRYPT;
    if (options & QBluetooth::Secure)
        lm |= L2CAP_LM_SECURE;

    if (setsockopt(sockfd, SOL_L2CAP, L2CAP_LM, &lm, sizeof(lm)) < 0)
    {
        return false;
    }

    return true;
}

class RegisterMetaTypes {
public:
    RegisterMetaTypes();
};

RegisterMetaTypes::RegisterMetaTypes()
{
//    qRegisterMetaType<QBluetoothSdpUuid>("QBluetoothSdpUuid");
    qRegisterMetaType<qint128>("qint128");
    qRegisterMetaType<quint128>("quint128");
    qRegisterMetaType<qint8>("qint8");
}

RegisterMetaTypes sdpMetaTypes;

#ifdef QTOPIA_BLUETOOTH
Q_IMPLEMENT_USER_METATYPE_ENUM(QBluetooth::SDPProfile)
Q_IMPLEMENT_USER_METATYPE_ENUM(QBluetooth::SecurityOptions)
#endif
