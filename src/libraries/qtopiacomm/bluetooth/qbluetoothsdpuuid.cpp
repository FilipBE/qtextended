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

#include <qbluetoothsdpuuid.h>

#include <QString>
#include <QList>

#include <assert.h>
#include <netinet/in.h>

static const quint16 PUBLIC_BROWSE_GROUP = 0x1002;
QBluetoothSdpUuid QBluetoothSdpUuid::PublicBrowseGroup(PUBLIC_BROWSE_GROUP);

static const quint16 L2CAP = 0x0100;
QBluetoothSdpUuid QBluetoothSdpUuid::L2cap(L2CAP);

/*!
    \class QBluetoothSdpUuid
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothSdpUuid class encapsulates Unique Identifiers defined and used by the Bluetooth Service Discovery Protocol.

    There are three UUID sizes: 16 bit, 32 bit and 128 bit.  The 16
    bit and 32 bit UUIDs can be upward converted to a 128 bit UUID.

    All values to this class should be given in host byte order.
    They will be converted to the appropriate byte
    ordering automatically.

    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothSdpUuid::Type
    Defines the size of the UUID.

    \value UUID16 16 bit UUID.
    \value UUID32 32 bit UUID.
    \value UUID128 128 bit UUID.
 */

/*!
    Constructs a new invalid UUID.

    \sa isValid()
 */
QBluetoothSdpUuid::QBluetoothSdpUuid()
{
    m_uuid = QVariant::fromValue(static_cast<quint16>(0));
    m_type = UUID16;
    m_isValid = false;
}

/*!
    Constructs a new 16 bit UUID with the value given by \a data.
    The type of the UUID will be \c UUID16

    \sa isValid()
*/
QBluetoothSdpUuid::QBluetoothSdpUuid(quint16 data)
{
    m_uuid = QVariant::fromValue(data);
    m_type = UUID16;
    m_isValid = true;
}

/*!
    Constructs a new 32 bit UUID with the value given by \a data.
    The type of the UUID will be \c UUID32

    \sa isValid()
*/
QBluetoothSdpUuid::QBluetoothSdpUuid(quint32 data)
{
    m_uuid = QVariant::fromValue(data);
    m_type = UUID32;
    m_isValid = true;
}

/*!
    Constructs a new 128 bit UUID with the value given by \a data.
    The type of the UUID will be \c UUID128

    \sa isValid()
*/
QBluetoothSdpUuid::QBluetoothSdpUuid(quint128 data)
{
    m_uuid = QVariant::fromValue(data);
    m_type = UUID128;
    m_isValid = true;
}

/*!
    Constructs a new UUID from a string representation \a str.  The UUID type will be
    inferred from the size and format of the string.  If the string is not in a valid
    format, an invalid UUID is created.

    \sa isValid()
*/
QBluetoothSdpUuid::QBluetoothSdpUuid(const QString &str)
{
    bool ok;

    // Assume its valid unless an error occurs
    m_isValid = true;

    if (str.startsWith("0x")) {
        if (str.length() == 6) {
            m_type = UUID16;
            m_uuid = QVariant::fromValue(static_cast<quint16>(str.toUShort(&ok, 0)));
            if (!ok)
                goto error;
            return;
        }
        else if (str.length() == 10) {
            m_type = UUID32;
            m_uuid = QVariant::fromValue(static_cast<quint32>(str.toUInt(&ok, 0)));
            if (!ok)
                goto error;
            return;
        }
        else {
            goto error;
        }
    }
    else {
        if (str.length() == 4) { // Probably a UUID16
            m_type = UUID16;
            m_uuid = QVariant::fromValue(static_cast<quint16>(str.toUShort(&ok, 0)));
            if (!ok)
                goto error;
            return;
        }
        else if (str.length() == 8) { // Probably a UUID32
            m_type = UUID32;
            m_uuid = QVariant::fromValue(static_cast<quint32>(str.toUInt(&ok, 0)));
            if (!ok)
                goto error;
            return;
        }
        else if (str.length() == 36) { // Probably a UUID128
            quint32 d0;
            quint16 d1;
            quint16 d2;
            quint16 d3;
            quint8 d4[6];

            bool ok;

            d0 = htonl(str.mid(0, 8).toUInt(&ok, 16));
            if (!ok)
                goto error;

            d1 = htons(str.mid(9, 4).toUShort(&ok, 16));
            if (!ok)
                goto error;

            d2 = htons(str.mid(14, 4).toUShort(&ok, 16));
            if (!ok)
                goto error;

            d3 = htons(str.mid(19, 4).toUShort(&ok, 16));
            if (!ok)
                goto error;

            for (int i = 0; i < 6; i++) {
                d4[i] = str.mid(24+i*2, 2).toUShort(&ok, 16);
                if (!ok)
                    goto error;
            }

            quint128 uuid;
            memcpy(&uuid.data[0], &d0, 4);
            memcpy(&uuid.data[4], &d1, 2);
            memcpy(&uuid.data[6], &d2, 2);
            memcpy(&uuid.data[8], &d3, 2);
            memcpy(&uuid.data[10], &d4[0], 6);

            m_uuid = QVariant::fromValue(uuid);
            m_type = UUID128;

            return;
        }
    }

error:
    *this = QBluetoothSdpUuid();
}

/*!
    Constructs a UUID from a UUID given by \a other.
*/
QBluetoothSdpUuid::QBluetoothSdpUuid(const QBluetoothSdpUuid &other)
{
    m_type = other.m_type;
    m_uuid = other.m_uuid;
    m_isValid = other.m_isValid;
}

/*!
    Deconstructs a UUID.
*/
QBluetoothSdpUuid::~QBluetoothSdpUuid()
{

}

/*!
    Assigns the contents of UUID \a other to the current UUID object.
*/
QBluetoothSdpUuid &QBluetoothSdpUuid::operator=(const QBluetoothSdpUuid &other)
{
    if (this == &other)
        return *this;

    m_type = other.m_type;
    m_uuid = other.m_uuid;
    m_isValid = other.m_isValid;

    return *this;
}

static quint8 base_uuid[] = { 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00,
                              0x10, 0x00,
                              0x80, 0x00,
                              0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

static QBluetoothSdpUuid convert_16_to_128(quint16 in)
{
    quint128 uuid;
    memcpy(&uuid.data[0], &base_uuid[0], 16);

    // UUIDs are stored in Network Byte Order
    quint16 tmp = htons(in);
    memcpy(&uuid.data[2], &tmp, 2);

    return QBluetoothSdpUuid(uuid);
}

static QBluetoothSdpUuid convert_32_to_128(quint32 in)
{
    quint128 uuid;
    memcpy(&uuid.data[0], &base_uuid[0], 16);

    // UUIDs are stored in Network Byte Order
    quint32 tmp = htonl(in);
    memcpy(&uuid.data[0], &tmp, 4);

    return QBluetoothSdpUuid(uuid);
}

/*!
    Converts a 16 or 32 bit UUID to a 128 bit UUID and returns the result.
    If the UUID is already a 128 bit UUID, a copy of the current object is returned.

    \sa type()
*/
QBluetoothSdpUuid QBluetoothSdpUuid::toUuid128() const
{
    if (m_type == UUID128)
        return *this;

    else if (m_type == UUID16)
        return convert_16_to_128(m_uuid.value<quint16>());

    else
        return convert_32_to_128(m_uuid.value<quint32>());
}

/*!
    Compares the current UUID to uuid given by \a other.  If the types do
    not match, the UUIDs are converted to 128 bit UUIDs and are bitwise compared.
    The method returns true if uuids match, false otherwise.
*/
bool QBluetoothSdpUuid::operator==(const QBluetoothSdpUuid &other) const
{
    if (m_isValid != other.m_isValid)
        return false;

    if (m_type == other.m_type) {
        switch (m_type) {
            case UUID16:
                return m_uuid.value<quint16>() == other.m_uuid.value<quint16>();
            case UUID32:
                return m_uuid.value<quint16>() == other.m_uuid.value<quint16>();
            case UUID128:
                quint128 uuid = m_uuid.value<quint128>();
                quint128 otheruuid = other.m_uuid.value<quint128>();
                return memcmp(&uuid, &otheruuid, sizeof(quint128)) == 0;
        }
    }

    QBluetoothSdpUuid thisone = this->toUuid128();
    QBluetoothSdpUuid otherone = other.toUuid128();

    quint128 uuid = thisone.m_uuid.value<quint128>();
    quint128 otheruuid = otherone.m_uuid.value<quint128>();

    return memcmp(&uuid, &otheruuid, sizeof(quint128)) == 0;
}

/*!
    Converts a UUID into string form.

    If the UUID is a 16 bit UUID, the string returned will be in this format: 0xXXXX, where
    X represents a hexadecimal number.  The string will be 0-padded, e.g. \c 0x00FF.

    If the UUID is a 32 bit UUID, the string returned will be in this format: 0xXXXXXXXX where
    X represents a hexadecimal number.  The string will be 0-padded, e.g. \c 0x0000FFFFFF.

    If the UUID is a 128 bit UUID, the string returned will be in the format of:
    XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX where X is a hexadecimal number.
*/
QString QBluetoothSdpUuid::toString() const
{
    QString result;

    if (m_type == UUID16) {
        result = "0x";
        quint16 num = m_uuid.value<quint16>();
        result.append(QString::number(num, 16).rightJustified(4, QLatin1Char('0')));
    }
    else if (m_type == UUID32) {
        result = "0x";
        quint32 num = m_uuid.value<quint32>();
        result.append(QString::number(num, 16).rightJustified(8, QLatin1Char('0')));
    }
    else if (m_type == UUID128) {
        QChar dash = QLatin1Char('-');
        quint32 d0;
        quint16 d1;
        quint16 d2;
        quint16 d3;
        quint8 d4[6];

        quint128 uuid = m_uuid.value<quint128>();
        memcpy(&d0, &uuid.data[0], 4);
        memcpy(&d1, &uuid.data[4], 2);
        memcpy(&d2, &uuid.data[6], 2);
        memcpy(&d3, &uuid.data[8], 2);
        memcpy(&d4[0], &uuid.data[10], 6);

        result = QString::number(ntohl(d0), 16).rightJustified(8, QLatin1Char('0'));
        result += dash;
        result += QString::number(ntohs(d1), 16).rightJustified(4, QLatin1Char('0'));
        result += dash;
        result += QString::number(ntohs(d2), 16).rightJustified(4, QLatin1Char('0'));
        result += dash;
        result += QString::number(ntohs(d3), 16).rightJustified(4, QLatin1Char('0'));
        result += dash;
        for (int i = 0; i < 6; i++)
            result += QString::number(d4[i], 16).rightJustified(2, QLatin1Char('0'));
    }
    else {
        assert(false);
    }

    return result;
}

/*!
    Returns the uuid as a QVariant.  The QVariant should be interpreted based on the
    QBluetoothSdpUuid type.  E.g. \c quint16 if type is \c UUID16, \c quint32 if type is \c UUID32, and
    \c quint128 if type is \c UUID128.

    \sa type()
*/
QVariant QBluetoothSdpUuid::uuid()
{
    return m_uuid;
}

/*!
    Returns whether the UUID is valid.
*/
bool QBluetoothSdpUuid::isValid() const
{
    return m_isValid;
}

/*!
    Returns the type of the UUID.

    \sa uuid()
*/
QBluetoothSdpUuid::Type QBluetoothSdpUuid::type() const
{
    return m_type;
}

/*!
    This is a convenience method for creating 16 bit UUIDs.  The \a id parameter
    specifies the 16 bit id to create.

    \sa create32Bit(), create128Bit()
*/
QBluetoothSdpUuid QBluetoothSdpUuid::create16Bit(quint16 id)
{
    return QBluetoothSdpUuid(id);
}

/*!
    This is a convenience method for creating 32 bit UUIDs. The \a id parameter
    specifies the 32 bit id to create.

    \sa create16Bit(), create128Bit()
 */
QBluetoothSdpUuid QBluetoothSdpUuid::create32Bit(quint32 id)
{
    return QBluetoothSdpUuid(id);
}

/*!
    This is a convenience method for creating 128 bit UUIDs. The \a id parameter
    specifies the 128 bit id to create.

    \sa create16Bit(), create32Bit()
 */
QBluetoothSdpUuid QBluetoothSdpUuid::create128Bit(quint128 id)
{
    return QBluetoothSdpUuid(id);
}

/*!
    This is a convenience method for creating a QBluetoothSdpUuid from \a profile.
    The profile enumeration is converted to a Bluetooth assigned number
    corresponding to the the profile UUID.
*/
QBluetoothSdpUuid QBluetoothSdpUuid::fromProfile(QBluetooth::SDPProfile profile)
{
    switch (profile) {
        case QBluetooth::SerialPortProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1101));
        case QBluetooth::DialupNetworkingProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1103));
        case QBluetooth::LanAccessProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1102));
        case QBluetooth::FaxProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1111));
        case QBluetooth::ObjectPushProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1105));
        case QBluetooth::FileTransferProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1106));
        case QBluetooth::DirectPrintingProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1118));
        case QBluetooth::HeadsetProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1108));
        case QBluetooth::HeadsetAudioGatewayProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1112));
        case QBluetooth::HandsFreeProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x111e));
        case QBluetooth::HandsFreeAudioGatewayProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x111f));
        case QBluetooth::SimAccessProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x112d));
        case QBluetooth::NetworkAccessPointProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1116));
        case QBluetooth::GroupAdHocNetworkProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1117));
        case QBluetooth::PersonalAreaNetworkUserProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1115));
        case QBluetooth::HardCopyReplacementProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x1125));
        case QBluetooth::AdvancedAudioSourceProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x110a));
        case QBluetooth::AdvancedAudioSinkProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x110b));
        case QBluetooth::AudioVideoRemoteControlProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x110e));
        case QBluetooth::AudioVideoTargetProfile:
            return QBluetoothSdpUuid(static_cast<quint16>(0x110c));
        default:
            break;
    };

    return QBluetoothSdpUuid(static_cast<quint16>(0x0));
}

/*!
    \fn bool QBluetoothSdpUuid::operator!=(const QBluetoothSdpUuid &other) const

    This function returns true if UUID in the current does not match the UUID
    given by \a other.

    \sa operator==()
 */
