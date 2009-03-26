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

#include <qtopialog.h>
#include <qirlocaldevice.h>
#include <qirremotedevice.h>
#include <qirnamespace.h>
#include "qirnamespace_p.h"

#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>

#include <errno.h>
#include <stdio.h>

#include <QStringList>
#include <QTextCodec>
#include <QVariant>
#include <QProcess>

class QIrLocalDevice_Private
{
public:
    QIrLocalDevice_Private(QIrLocalDevice *parent, const QString &devname);

    bool isUp();
    bool bringUp();
    bool bringDown();

    const QString &deviceName() const;

    QVariant queryRemoteAttribute(const QIrRemoteDevice &remote,
                                  const QString &className,
                                  const QString &attribName);
    bool discoverRemoteDevices(QIr::DeviceClasses classes);

private:
    QString m_devname;
    QIrLocalDevice *m_parent;
};

QIrLocalDevice_Private::QIrLocalDevice_Private(QIrLocalDevice *parent,
                                               const QString &devname)
{
    m_parent = parent;
    m_devname = devname;
}

/*!
    \internal

    Not quite sure if this will work on serial SIR devices, but hopefully
    all phones will be shipping with FIR/VFIR devices soon anyway.
 */
bool QIrLocalDevice_Private::isUp()
{
    // Try to open an Infrared socket
    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1) {
        qLog(Infrared) << "Could not open IR socket";
        return false;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, m_devname.toLatin1().constData());

    bool ret = false;

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        qLog(Infrared) << "Could not get IF Flags";
        goto error;
    }

    if ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING)) {
        ret = true;
    }

error:
    close(fd);
    return ret;
}

static void enable_discovery()
{
    qLog(Infrared) << "setting discovery to 1";
    QProcess sysctl;
    sysctl.start("/sbin/sysctl -w net.irda.discovery=1");
    sysctl.waitForFinished(1000);
}

/*!
    \internal

    Tries to bring up the infrared device.
    Not quite sure if this will work on serial SIR devices, but hopefully
    all phones will be shipping with FIR/VFIR devices soon anyway.
*/
bool QIrLocalDevice_Private::bringUp()
{
    // This function assumes that the irda driver has been loaded and that the irattach
    // daemon is running.  This in effect activates the Linux IrDA stack.  From then on
    // the irda device should show up as a regular network device that we can control
    // using ifconfig
    // Try to open an Infrared socket
    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1) {
        qLog(Infrared) << "Could not open IR socket";
        return false;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, m_devname.toLatin1().constData());

    bool ret = false;

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        qLog(Infrared) << "Could not get IF Flags";
        goto error;
    }

    strcpy(ifr.ifr_name, m_devname.toLatin1().constData());
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        qLog(Infrared) << "Could not bring up interface: " << strerror(errno);
        goto error;
    }

    ret = true;

    // Now make sure discoverable bit is set
    enable_discovery();

error:
    close(fd);
    return ret;
}

/*!
    \internal

    Tries to bring down a local infrared device.
    Not quite sure if this will work on serial SIR devices, but hopefully
    all phones will be shipping with FIR/VFIR devices soon anyway.
 */
bool QIrLocalDevice_Private::bringDown()
{
    // This function assumes that the irda driver has been loaded and that the irattach
    // daemon is running.  This in effect activates the Linux IrDA stack.  From then on
    // the irda device should show up as a regular network device that we can control
    // using ifconfig
    // Note that this most likely will not power down the device, if this is required
    // then custom code must be inserted to do this as there is no way to do this in
    // Linux at this time
    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1) {
        qLog(Infrared) << "Could not open IR socket";
        return false;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, m_devname.toLatin1().constData());

    bool ret = false;

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        qLog(Infrared) << "Could not get IF Flags";
        goto error;
    }

    strcpy(ifr.ifr_name, m_devname.toLatin1().constData());
    ifr.ifr_flags &= ~IFF_UP;

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        qLog(Infrared) << "Could not bring down interface: " << strerror(errno);
        goto error;
    }

    ret = true;

error:
    close(fd);
    return ret;
}

const QString &QIrLocalDevice_Private::deviceName() const
{
    return m_devname;
}

// This is defined in <linux/if_arp.h>
#define ARPHRD_IRDA 783

bool is_ir_device(const char *buf, int fd)
{
    struct ifreq ifr;

    strcpy(ifr.ifr_name, buf);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
        return false;

    if (ifr.ifr_hwaddr.sa_family == ARPHRD_IRDA)
        return true;

    return false;
}

#define PROC_NET_DEV "/proc/net/dev"

/*!
    \internal
    Seems to be the only way to find a list of devices.
*/
static QStringList get_local_ir_devices()
{
    qLog(Infrared) << "Getting IR Devices...";
    QStringList ret;

    qLog(Infrared) << "Opening AF_INET socket...";
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd == -1 )
        return ret;

    qLog(Infrared) << "Opening PROC_NET_DEV";
    FILE *f = fopen(PROC_NET_DEV, "r");
    if (f == NULL) {
        close(fd);
        return ret;
    }

    char buf[1024];

    // Eat the first two lines
    fgets(buf, sizeof(buf), f);
    fgets(buf, sizeof(buf), f);

    while(fgets(buf, sizeof(buf), f)) {

        int i = 0;
        while (buf[i] != ':')
            i++;

        buf[i] = '\0';

        i = 0;
        while (buf[i] == ' ')
            i++;

        qLog(Infrared) << "Checking device: " << &buf[i];
        if (is_ir_device(&buf[i], fd)) {
            ret.push_back(&buf[i]);
        }
    }

    fclose(f);
    close(fd);

    return ret;
}

QVariant QIrLocalDevice_Private::queryRemoteAttribute(const QIrRemoteDevice &remote,
                                                      const QString &className,
                                                      const QString &attribName)
{
    if (className.size() > IAS_MAX_CLASSNAME)
        return QVariant();

    if (attribName.size() >= IAS_MAX_ATTRIBNAME)
        return QVariant();

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return QVariant();

    qLog(Infrared) << "Successfully opened Infrared socket";

    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());
    entry.daddr = remote.address();
    socklen_t len = sizeof(entry);

    int status = getsockopt(fd, SOL_IRLMP, IRLMP_IAS_QUERY, &entry, &len);
    close(fd);

    if (status != 0) {
        perror("Could not get attribute:");
        return QVariant();
    }

    return convert_ias_entry(entry);
}

bool QIrLocalDevice_Private::discoverRemoteDevices(QIr::DeviceClasses classes)
{
    qLog(Infrared) << "Scanning for IR Devices";

    // Try to open an Infrared socket
    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return false;

    qLog(Infrared) << "Successfully opened Infrared socket";

    struct irda_device_list *list;

    // Allocate space for up to 10 devices
    int len = sizeof(struct irda_device_list) + sizeof(struct irda_device_info) * 10;

    list = reinterpret_cast<struct irda_device_list *>(malloc(len));

    if (!list)
        return false;

    qLog(Infrared) << "Allocation of irda_device_list successfull";

    struct irda_device_info *dev = list->dev;
    socklen_t actual_size = len;

    bool ret = false;
    QList<QIrRemoteDevice> remoteList;

    // Try to set the hint bits, this should almost always work.
    unsigned char hints[4];

    hints[0] = 0;
    hints[1] = 0;
    hints[2] = 0;
    hints[3] = 0;

    convert_to_hints(classes, hints);

    qLog(Infrared) << "Setting the hint mask bits to: " << classes;
    int status = setsockopt(fd, SOL_IRLMP, IRLMP_HINT_MASK_SET, hints, sizeof(hints));
    qLog(Infrared) << "Success: " << (status == 0);
    if (status != 0)
        goto error;

    qLog(Infrared) << "getting socket options";
    if ( getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, list, &actual_size) != 0 ) {
        qLog(Infrared) << "Could not get remote devices: " << strerror(errno);
        goto error;
    }

    emit m_parent->discoveryStarted();

    ret = true;

    qLog(Infrared) << "Got " << list->len << " IR devices";
    for (unsigned int i = 0; i < list->len ; i++) {
        QIr::DeviceClasses classes;
        if (dev[i].hints[0] & HINT_PNP)
            classes |= QIr::PlugNPlay;
        if (dev[i].hints[0] & HINT_PDA)
            classes |= QIr::PDA;
        if (dev[i].hints[0] & HINT_COMPUTER)
            classes |= QIr::Computer;
        if (dev[i].hints[0] & HINT_PRINTER)
            classes |= QIr::Printer;
        if (dev[i].hints[0] & HINT_MODEM)
            classes |= QIr::Modem;
        if (dev[i].hints[0] & HINT_FAX)
            classes |= QIr::Fax;
        if (dev[i].hints[0] & HINT_LAN)
            classes |= QIr::LAN;

        if ((dev[i].hints[0] & HINT_EXTENSION) && (dev[i].hints[1] & HINT_TELEPHONY))
            classes |= QIr::Telephony;
        if ((dev[i].hints[0] & HINT_EXTENSION) && (dev[i].hints[1] & HINT_FILE_SERVER))
            classes |= QIr::FileServer;
        if ((dev[i].hints[0] & HINT_EXTENSION) && (dev[i].hints[1] & HINT_COMM))
            classes |= QIr::Communications;
        if ((dev[i].hints[0] & HINT_EXTENSION) && (dev[i].hints[1] & HINT_MESSAGE))
            classes |= QIr::Message;
        if ((dev[i].hints[0] & HINT_EXTENSION) && (dev[i].hints[1] & HINT_HTTP))
            classes |= QIr::HTTP;
        if ((dev[i].hints[0] & HINT_EXTENSION) && (dev[i].hints[1] & HINT_OBEX))
            classes |= QIr::OBEX;

        QString name;

        if (dev[i].charset == CS_ASCII) {
            name = QString::fromAscii(dev[i].info);
        }
        else if (dev[i].charset == CS_ISO_8859_1) {
            name = QString::fromLatin1(dev[i].info);
        }
        else {
            QByteArray codecName =
                    QIr::convert_charset_to_string(dev[i].charset);
            QTextCodec *codec = QTextCodec::codecForName(codecName);
            name = codec->toUnicode(dev[i].info);
        }
        QIrRemoteDevice remote(name, classes, dev[i].daddr);
        emit m_parent->remoteDeviceFound(remote);
        remoteList.push_back(remote);
    }
    emit m_parent->remoteDevicesFound(remoteList);
    emit m_parent->discoveryCompleted();

error:
    free(list);
    ::close(fd);

    return ret;
}

/*!
    \class QIrLocalDevice

    \brief The QIrLocalDevice class encapsulates a local Infrared (IrDA) device.

    The QIrLocalDevice class can be used to control the state of the local
    IrDA device and query for remote Infrared devices.

    The user can control the state of the device by using the bringUp()
    and bringDown() functions.  The isUp() function returns the state of
    the device.

    The local infrared device can be used to discover remote infrared devices
    by calling the discoverRemoteDevices() function and hooking onto the
    remoteDeviceFound() or remoteDevicesFound() signals.

    This class can also be used to query the remote IAS database.

    \ingroup qtopiair
    \sa QIrIasDatabase
 */

/*!
    Constructs a local device for the given device name \a device. The
    \a device is usually of the form irdaN, e.g. irda0.

    \sa devices()
*/
QIrLocalDevice::QIrLocalDevice(const QString &device)
{
    m_data = new QIrLocalDevice_Private(this, device);
}

/*!
    Destroys the device object.
*/
QIrLocalDevice::~QIrLocalDevice()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns the device name of the infrared device. E.g. irda0.
*/
const QString &QIrLocalDevice::deviceName() const
{
    return m_data->deviceName();
}

/*!
    Returns true if the infrared device is up and running.

    \sa bringUp(), bringDown()
*/
bool QIrLocalDevice::isUp() const
{
    return m_data->isUp();
}

/*!
    Tries to bring up the infrared device.  Returns true if the device
    could be brought up successfully, and false otherwise.

    NOTE: Under linux this function requires administrator privileges.

    \sa isUp(), bringDown()
*/
bool QIrLocalDevice::bringUp()
{
    return m_data->bringUp();
}

/*!
    Tries to bring down the infrared device.  Returns true if the device
    could be brought down successfully, and false otherwise.

    NOTE: Under linux this function requires administrator privileges.

    \sa isUp(), bringUp()
*/
bool QIrLocalDevice::bringDown()
{
    return m_data->bringDown();
}

/*!
    Returns all the infrared devices found on this system.  The contents
    of the list could be passed to the QIrLocalDevice constructor.
*/
QStringList QIrLocalDevice::devices()
{
    return get_local_ir_devices();
}

/*!
    Tries to query the remote device given by \a remote
    for an IAS Database entry with class name given by \a className and
    attribute name given by \a attribName.  Returns a valid attribute
    if the device has an attribute specified, returns false if the
    device could not be contacted or the attribute does not exist.

    \code
        QIrIasAttribute ret =
                dev.queryRemoteAttribute(remote, "OBEX", "IrDA:TinyTP:LsapSel");
    \endcode

    The above code would attempt to query the remote device given by \c remote
    for whether it supports the OBEX service.

    \sa QIrIasDatabase
*/
QVariant QIrLocalDevice::queryRemoteAttribute(const QIrRemoteDevice &remote,
                                          const QString &className,
                                          const QString &attribName)
{
    return m_data->queryRemoteAttribute(remote, className, attribName);
}

/*!
    Starts a discovery of remote infrared devices.  The \a classes parameter
    specifies a filtering set.  Thus a client can request that only remote
    devices which have at least one device class present in the filter set
    be discovered.

    The clients can subscribe to the discovery information in one of two ways.
    If the client wants to receive information about a device as it is received,
    they should subscribe to the remoteDeviceFound() signal.  If the clients
    wish to receive the information wholesale, they should subscribe
    to the remoteDevicesFound() signal.

    Returns true if the discovery could be started successfully, and false
    otherwise.

    \sa discoveryStarted(), discoveryCompleted()
*/
bool QIrLocalDevice::discoverRemoteDevices(QIr::DeviceClasses classes)
{
    return m_data->discoverRemoteDevices(classes);
}

/*!
    \fn void QIrLocalDevice::discoveryStarted()

    This signal is emitted whenever a discovery of remote infrared devices
    is started.

    \sa discoveryCompleted()
*/

/*!
    \fn void QIrLocalDevice::discoveryCompleted()

    This signal is emitted whenever a discovery of remote infrared devices
    has completed.

    \sa discoveryStarted()
*/

/*!
    \fn void QIrLocalDevice::remoteDeviceFound(const QIrRemoteDevice &device)

    This signal is emitted during the discovery procedure.  The \a device
    parameter holds the remote device found.  Clients can subscribe to this
    signal if they wish to receive information about one device at a time.
    The can subscribe to the remoteDevicesFound() signal if they wish to receive
    all the devices wholesale.

    \sa discoveryStarted(), discoveryCompleted(), remoteDevicesFound()
 */

/*!
    \fn void QIrLocalDevice::remoteDevicesFound(const QList<QIrRemoteDevice> &devices)

    This signal is emitted during the discovery procedure.  The \a devices
    parameter holds the list of remote devices found.  Clients can
    subscribe to this signal if they wish to receive information about all
    devices at once.  They could subscribe to the remoteDeviceFound() signal
    if they wished to receive the information piecemeal.

    \sa discoveryStarted(), discoveryCompleted(), remoteDeviceFound()
 */
