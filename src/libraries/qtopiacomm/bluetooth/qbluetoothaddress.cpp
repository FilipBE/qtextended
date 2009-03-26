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

#include <qbluetoothaddress.h>
#include <qhash.h>
#include <qsettings.h>

#include <string.h>
#include <stdio.h>

/*!
    \class QBluetoothAddress
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothAddress class represents a bluetooth address.

    Each Bluetooth device can have one and only one address.  There are certain
    special addresses defined, namely \c{any}, \c{local}, \c{all}.

    The Bluetooth address can be constructed from a string representation.
    For instance,

    \code
        QBluetoothAddress addr("00:00:FF:FF:00:00");
    \endcode

    If the address is a valid Bluetooth address, then isValid()
    will return true, and false otherwise.

    \ingroup qtopiabluetooth
    \sa QBluetoothLocalDevice, QBluetoothRemoteDevice
 */


static QChar getAddressSeparator()
{
    QSettings settings("Trolltech", "Bluetooth");
    QString sep = settings.value("AddressSeparatorChar").toString();
    if (sep.isEmpty())
        return ':';
    return sep[0].toAscii();
}

static QString constructAddress(char sep, const char *pt1, const char *pt2, const char *pt3, const char *pt4, const char *pt5, const char *pt6)
{
    size_t bufSize = 18;
    char buf[bufSize];
    ::snprintf(buf, bufSize, "%s%c%s%c%s%c%s%c%s%c%s",
            pt1, sep, pt2, sep, pt3, sep, pt4, sep, pt5, sep, pt6);
    return QString::fromAscii(buf);
}

static const char SEPARATOR = getAddressSeparator().toAscii();


/*!
    Constructs a new invalid bluetooth address.
 */
QBluetoothAddress::QBluetoothAddress()
{
    m_valid = false;
    m_bdaddr = constructAddress(SEPARATOR, "00", "00", "00", "00", "00", "00");
}

/*!
    Constructs a copy of a bluetooth address from \a other.
*/
QBluetoothAddress::QBluetoothAddress(const QBluetoothAddress &other)
{
    m_valid = other.m_valid;
    m_bdaddr = other.m_bdaddr;
}

/*!
    Constructs a bluetooth address based on string representation given by \a addr.
    The string should be in the format of XX:XX:XX:XX:XX:XX where XX represents
    a hexadecimal number.  If the string is in an invalid format, an invalid
    bluetooth address is constructed.
*/
QBluetoothAddress::QBluetoothAddress(const QString &addr)
{
    unsigned int baddr[6];

    QByteArray asciiArr = addr.toAscii();
    const char *buf = asciiArr.constData();

    static const QByteArray defaultSepFormatStr =
            constructAddress(SEPARATOR, "%x", "%x", "%x", "%x", "%x", "%x").toAscii();

    if ( (sscanf(buf, defaultSepFormatStr.constData(),
            &baddr[0], &baddr[1], &baddr[2], &baddr[3], &baddr[4], &baddr[5]) == 6) ||
         (sscanf(buf, "%x:%x:%x:%x:%x:%x",
            &baddr[0], &baddr[1], &baddr[2], &baddr[3], &baddr[4], &baddr[5]) == 6) ||
         (sscanf(buf, "%x-%x-%x-%x-%x-%x",
            &baddr[0], &baddr[1], &baddr[2], &baddr[3], &baddr[4], &baddr[5]) == 6) ) {
        m_valid = true;
        m_bdaddr = addr;

        // force strings to use the default separator
        if (addr[2] != SEPARATOR) {
            m_bdaddr.replace(addr[2], SEPARATOR);
        }
    }
    else {
        m_valid = false;
        m_bdaddr = QBluetoothAddress::invalid.m_bdaddr;
    };
}

/*!
    Destroys a Bluetooth address.
*/
QBluetoothAddress::~QBluetoothAddress()
{
}

/*!
    Assigns the contents of the Bluetooth address \a other to the current address.
*/
QBluetoothAddress &QBluetoothAddress::operator=(const QBluetoothAddress &other)
{
    if (this == &other)
        return *this;

    m_valid = other.m_valid;
    m_bdaddr = other.m_bdaddr;

    return *this;
}

/*!
    Returns the result of comparing the current Bluetooth address against a Bluetooth address given by \a other.
*/
bool QBluetoothAddress::operator==(const QBluetoothAddress &other) const
{
    return m_bdaddr == other.m_bdaddr;
}

/*!
    Converts the Bluetooth address into a QString.  The format will be of
    the form XX:XX:XX:XX:XX:XX where XX is a hexadecimal number.
    If the address is invalid, a null string is returned.
*/
QString QBluetoothAddress::toString() const
{
    if (!m_valid)
        return QString();

    return m_bdaddr;
}

/*!
    Returns whether the address is valid.
*/
bool QBluetoothAddress::isValid() const
{
    return m_valid;
}

/*!
    \variable QBluetoothAddress::invalid
    Invalid Bluetooth address.
*/
const QBluetoothAddress QBluetoothAddress::invalid = QBluetoothAddress();

/*!
    \variable QBluetoothAddress::any
    Bluetooth address that represents a special address \bold any
*/
const QBluetoothAddress QBluetoothAddress::any = QBluetoothAddress(
        constructAddress(SEPARATOR, "00", "00", "00", "00", "00", "00"));

/*!
    \variable QBluetoothAddress::all
    Bluetooth address that represents a special address \bold all
 */
const QBluetoothAddress QBluetoothAddress::all = QBluetoothAddress(
        constructAddress(SEPARATOR, "FF", "FF", "FF", "FF", "FF", "FF"));

/*!
    \variable QBluetoothAddress::local
    Bluetooth address that represents a special address \bold local
*/
const QBluetoothAddress QBluetoothAddress::local = QBluetoothAddress(
        constructAddress(SEPARATOR, "00", "00", "00", "FF", "FF", "FF"));

/*!
    \fn bool QBluetoothAddress::operator!=(const QBluetoothAddress &other) const

    Compares the current address to \a other.
    Returns true if the addresses are not equal, false if they are.
*/

/*!
    \internal
*/
uint qHash(const QBluetoothAddress &addr)
{
    return qHash(addr.m_bdaddr);
}

#ifdef QTOPIA_BLUETOOTH
/*!
    \internal
    \fn void QBluetoothAddress::serialize(Stream &stream) const
 */
template <typename Stream> void QBluetoothAddress::serialize(Stream &stream) const
{
    stream << m_bdaddr;
    stream << m_valid;
}

/*!
    \internal
    \fn void QBluetoothAddress::deserialize(Stream &stream)
 */
template <typename Stream> void QBluetoothAddress::deserialize(Stream &stream)
{
    stream >> m_bdaddr;
    stream >> m_valid;
}

Q_IMPLEMENT_USER_METATYPE(QBluetoothAddress)

#endif
