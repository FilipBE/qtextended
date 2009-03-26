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

#include "qbluetoothrfcommserialport.h"
#include "qbluetoothnamespace_p.h"
#include <qbluetoothlocaldevice.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

#include <QTime>
#include <QTimer>
#include <QString>
#include <QApplication>
#include <QDebug>

static bool release_dev_with_socket(int id, int socket)
{
    struct rfcomm_dev_req rqst;
    ::memset( &rqst, 0, sizeof( struct rfcomm_dev_req ) );
    rqst.dev_id = id;
    rqst.flags = (1 << RFCOMM_HANGUP_NOW );
    int ret = ::ioctl( socket, RFCOMMRELEASEDEV, &rqst );
    if ( ret < 0  && errno != ENODEV ) {
        perror( "QBluetoothRfcommSerialPort::release" );
        return false;
    }

    return true;
}

static const int MAX_RETRIES = 10;
static const int OPEN_TIME = 300;

static int create_rfcomm_tty(QBluetoothRfcommSocket *socket)
{
    int id;
    struct rfcomm_dev_req rqst;
    ::memset( &rqst, 0, sizeof( struct rfcomm_dev_req ) );
    str2bdaddr( socket->remoteAddress().toString(), &rqst.dst );
    str2bdaddr( socket->localAddress().toString(), &rqst.src );
    rqst.dev_id = -1;
    rqst.flags = 0;

    rqst.channel = socket->remoteChannel();
    rqst.flags = (1 << RFCOMM_REUSE_DLC ) |( 1 << RFCOMM_RELEASE_ONHUP );

    // Set the socket non-blocking
    int flags = fcntl(socket->socketDescriptor(), F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(socket->socketDescriptor(), F_SETFL, flags);
    id = ioctl( socket->socketDescriptor(), RFCOMMCREATEDEV, &rqst );

    return id;
}

static bool foreach_device_with_socket(int s, void *userData,
                                       void (*filter)(void *, struct rfcomm_dev_info *))
{
    struct rfcomm_dev_info* info;
    struct rfcomm_dev_list_req* allInfos;

    allInfos = (struct rfcomm_dev_list_req* ) malloc( RFCOMM_MAX_DEV*sizeof(struct rfcomm_dev_info) + sizeof(struct rfcomm_dev_list_req));
    allInfos->dev_num = RFCOMM_MAX_DEV;
    info = allInfos->dev_info;

    int ret = ::ioctl( s, RFCOMMGETDEVLIST, allInfos );
    if  ( ret < 0 ) {
        perror("QBluetoothRfcommSerialPort::listBindings: Cannot obtain list of rfcomm devices");
    } else {
        struct rfcomm_dev_info* next;
        for( int i = 0; i<allInfos->dev_num; i++ ) {
            next = info+i;
            filter(userData, next);
        }
    }
    free( allInfos );

    return ret != -1;
}

bool foreach_device(void *userData,
                    void (*filter)(void *, struct rfcomm_dev_info *))
{
    int s = socket( AF_BLUETOOTH, SOCK_RAW, BTPROTO_RFCOMM );
    if ( s < 0 ) {
        perror("QBluetoothRfcommSerialPort::listBindings");
        return false;
    }

    bool ret = foreach_device_with_socket(s, userData, filter);

    close(s);

    return ret;
}

class QBluetoothRfcommSerialPortPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothRfcommSerialPortPrivate(QBluetoothRfcommSerialPort *parent);
    ~QBluetoothRfcommSerialPortPrivate();

    // Max of 31 rfcomm devices
    int m_id;
    int m_fd;
    QString m_devName;
    QBluetoothRfcommSerialPort::Flags m_flags;
    QBluetoothRfcommSerialPort::Error m_error;
    QString m_errorString;
    QBluetoothRfcommSocket *m_socket;

    QBluetoothAddress m_remote;
    QBluetoothAddress m_local;
    int m_channel;

    bool initiateConnect(const QBluetoothAddress &local,
                         const QBluetoothAddress &remote,
                         int channel);

    bool connectPending() const;
    bool cancelConnect();
    bool disconnect();

private slots:
    void error(QBluetoothAbstractSocket::SocketError error);
    void stateChanged(QBluetoothAbstractSocket::SocketState state);
    void setupTtyRetry();

private:
    void setupTty();

    void readRfcommParameters();

    bool m_connectInProgress;
    bool m_openInProgress;
    bool m_cancelled;

    int m_retries;

    QTimer *m_timer;
    QBluetoothRfcommSerialPort *m_parent;
};

QBluetoothRfcommSerialPortPrivate::QBluetoothRfcommSerialPortPrivate(QBluetoothRfcommSerialPort *parent)
    : QObject(parent), m_socket(0), m_parent(parent)
{
    m_id = -1;
    m_fd = -1;
    m_flags = 0;
    m_error = QBluetoothRfcommSerialPort::NoError;
    m_connectInProgress = false;
    m_openInProgress = false;
    m_cancelled = false;

    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(setupTtyRetry()));
}

QBluetoothRfcommSerialPortPrivate::~QBluetoothRfcommSerialPortPrivate()
{
    delete m_socket;
    m_socket = 0;
}

static void read_serial_port_parameters(void *userData, struct rfcomm_dev_info *info)
{
    QBluetoothRfcommSerialPortPrivate *ptr =
            static_cast<QBluetoothRfcommSerialPortPrivate *>(userData);

    if (ptr->m_id == info->id) {
        bdaddr_t addr;
        memcpy(&addr, &info->src, sizeof(bdaddr_t));
        QString str = bdaddr2str(&addr);
        ptr->m_local = QBluetoothAddress(str);

        memcpy(&addr, &info->dst, sizeof(bdaddr_t));
        str = bdaddr2str(&addr);
        ptr->m_remote = QBluetoothAddress(str);

        ptr->m_channel = info->channel;
    }
}

void QBluetoothRfcommSerialPortPrivate::readRfcommParameters()
{
    foreach_device(this, read_serial_port_parameters);
}

bool QBluetoothRfcommSerialPortPrivate::initiateConnect(const QBluetoothAddress &local,
        const QBluetoothAddress &remote,
        int channel)
{
    // We are bound to a tty interface already
    if ( m_fd != -1)
        return false;

    if (m_connectInProgress)
        return false;

    if (m_openInProgress)
        return false;

    m_socket = new QBluetoothRfcommSocket();
    QObject::connect(m_socket, SIGNAL(stateChanged(QBluetoothAbstractSocket::SocketState)),
                     this, SLOT(stateChanged(QBluetoothAbstractSocket::SocketState)));
    QObject::connect(m_socket, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
                     this, SLOT(error(QBluetoothAbstractSocket::SocketError)));

    bool ret = m_socket->connect(local, remote, channel);

    m_connectInProgress = ret;
    m_openInProgress = false;
    m_cancelled = false;

    if (!ret) {
        delete m_socket;
        m_socket = 0;
    }

    return ret;
}

void QBluetoothRfcommSerialPortPrivate::setupTtyRetry()
{
    if (m_cancelled || (m_retries == MAX_RETRIES)) {
        m_timer->stop();
        QBluetoothRfcommSerialPort::releaseDevice(m_id);
        m_id = -1;
        m_openInProgress = false;

        if (m_cancelled) {
            m_parent->setError(QBluetoothRfcommSerialPort::ConnectionCancelled);
        } else {
            m_parent->setError(QBluetoothRfcommSerialPort::CreationError);
        }

        emit m_parent->error(m_error);
        return;
    }

    char devname[MAXPATHLEN];
    snprintf(devname, MAXPATHLEN - 1, "/dev/rfcomm%d", m_id);
    if ((m_fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
        snprintf(devname, MAXPATHLEN-1, "/dev/bluetooth/rfcomm/%d", m_id);
        m_fd = open(devname, O_RDONLY | O_NOCTTY);
        if ( m_fd < 0 )
            qWarning("Error opening RFCOMM serial port"); 
    }

    m_retries++;

    if (m_fd != -1) {
        m_timer->stop();
        m_openInProgress = false;

        ::fcntl(m_fd, F_SETFD, FD_CLOEXEC);
        m_devName = QString(devname);

        emit m_parent->connected(m_devName);
        return;
    }
}

void QBluetoothRfcommSerialPortPrivate::setupTty()
{
    m_connectInProgress = false;
    m_retries = 0;

    m_id = create_rfcomm_tty(m_socket);

    delete m_socket;
    m_socket = 0;

    if ( m_id < 0 ) {
        m_parent->setError(QBluetoothRfcommSerialPort::CreationError);

        emit m_parent->error(m_error);
        return;
    }

    readRfcommParameters();
    setupTtyRetry();

    if (m_fd == -1) {
        m_openInProgress = true;
        m_timer->start(OPEN_TIME);
    }
}

void QBluetoothRfcommSerialPortPrivate::error(QBluetoothAbstractSocket::SocketError)
{
    m_error = QBluetoothRfcommSerialPort::ConnectionFailed;
    m_errorString = m_socket->errorString();
}

void QBluetoothRfcommSerialPortPrivate::stateChanged(QBluetoothAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QBluetoothRfcommSocket::ConnectingState:
            break;

        case QBluetoothRfcommSocket::ConnectedState:
            setupTty();
            break;

        case QBluetoothRfcommSocket::ClosingState:
            break;

        case QBluetoothRfcommSocket::UnconnectedState:
            // We only need to catch connection failures here
            if (m_connectInProgress) {
                m_connectInProgress = false;
                m_socket->deleteLater();
                m_socket = 0;
                emit m_parent->error(m_error);
            }

            break;
        default:
            break;
    };
}

bool QBluetoothRfcommSerialPortPrivate::cancelConnect()
{
    // If we have no socket and no open is in progress
    if (!m_socket && !m_openInProgress)
        return false;

    if (m_openInProgress) {
        m_cancelled = true;
        return true;
    }

    m_parent->setError(QBluetoothRfcommSerialPort::ConnectionCancelled);
    m_socket->disconnect();

    return true;
}

bool QBluetoothRfcommSerialPortPrivate::connectPending() const
{
    return m_connectInProgress || m_openInProgress;
}

bool QBluetoothRfcommSerialPortPrivate::disconnect()
{
    bool ret = true;

    ::close(m_fd);
    m_fd = -1;

    if (!(m_flags & QBluetoothRfcommSerialPort::KeepAlive)) {
        ret = QBluetoothRfcommSerialPort::releaseDevice(m_id);
    }

    m_flags = 0;
    m_id = -1;
    m_devName = QString();

    emit m_parent->disconnected();

    return ret;
}

/*!
    \class QBluetoothRfcommSerialPort
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothRfcommSerialPort class represents a RFCOMM serial port device.

    This class enables conversion of RFCOMM sockets into serial
    tty devices. The class can be used in two major ways, by
    adapting an existing connected RFCOMM socket or by using the
    connect() function to establish a client connection to a remote
    device and adapt the resulting connection to a tty device.

    The resulting tty device can be used by other applications
    like a regular serial tty device.

    \code
        // Adapt a connected RFCOMM socket into a serial tty device

        if (rfcommSocket->state() != QBluetoothRfcommSocket::ConnectedState) {
            // Bail out
            return;
        }

        QBluetoothRfcommSerialPort *serial =
            new QBluetoothRfcommSerialPort(rfcommSocket);

        if (serial->id() == -1) {
            // Error creating serial port
            return;
        }

        rfcommSocket->close();

        // Use serial port
        openSerialPort(serial->device());
    \endcode

    \sa QBluetoothRfcommSocket, QSerialIODevice

    \ingroup qtopiabluetooth
*/

/*!
    \enum QBluetoothRfcommSerialPort::Flag
    Defines flags that alter the behavior of the QBluetoothRfcommSerialPort

    \value KeepAlive Deleting the serial port object will not necessarily shut down the serial port.  The serial port will be kept open until all open handles (file descriptors) to the serial port are closed.  You can use the release() method to forcibly delete a serial port.
*/

/*!
    \enum QBluetoothRfcommSerialPort::Error
    Defines errors that could occur in QBluetoothRfcommSerialPort

    \value NoError No error has occurred.
    \value SocketNotConnected The given socket is not connected.
    \value ConnectionFailed Could not connect to the remote device.
    \value ConnectionCancelled Connection has been cancelled.
    \value CreationError Could not create the serial port device.
*/

/*!
    Constructs a new RFCOMM serial port object.  The \a parent is passed to the
    QObject constructor.
*/
QBluetoothRfcommSerialPort::QBluetoothRfcommSerialPort( QObject* parent)
    : QObject( parent )
{
    d = new QBluetoothRfcommSerialPortPrivate( this );
}

/*!
    Destroys a Bluetooth serial port object.  If the instance is managing a
    serial port, disconnect() will be called first.
*/
QBluetoothRfcommSerialPort::~QBluetoothRfcommSerialPort()
{
    if ( d->m_fd != -1 )
        d->disconnect();

    delete d;
}

/*!
    Converts a connected RFCOMM \a socket to an RFCOMM serial port.

    This constructor is useful in situations where you already have a connected RFCOMM socket
    and wish to convert it to a serial port.  This is true of connections returned by
    QBluetoothRfcommServer::nextPendingConnection() or RFCOMM socket connections
    which require non-blocking connections.

    Note: The caller should call QBluetoothRfcommSocket::close() on \a socket
    immediately after calling this constructor.  Before the created
    QBluetoothRfcommSerialPort device can be used, the \a socket should be
    closed.   The \a deviceFlags parameter holds the
    optional flags that modify the serial port behavior.

    In case the device could not be created or an error occurred,
    the id() method will return -1.  The caller should check the return
    value of id() before using the serial port device.

    \sa id()
*/
QBluetoothRfcommSerialPort::QBluetoothRfcommSerialPort(QBluetoothRfcommSocket* socket,
        QBluetoothRfcommSerialPort::Flags deviceFlags, QObject *parent)
    : QObject(parent)
{
    d = new QBluetoothRfcommSerialPortPrivate( this );

    struct sockaddr_rc saddr;
    socklen_t len = sizeof( saddr );

    if ( getsockname( socket->socketDescriptor(), (struct sockaddr*)&saddr, &len ) < 0 ) {
        setError(QBluetoothRfcommSerialPort::SocketNotConnected);
        return;
    }

    d->m_id = create_rfcomm_tty(socket);
    if ( d->m_id < 0 ) {
        qWarning() << "RfcommSerialPort::copyConstructor( socket ): " << strerror(errno);
        setError(QBluetoothRfcommSerialPort::CreationError);
        return;
    }

    char devname[MAXPATHLEN];

    QTime started = QTime::currentTime();
    QTime now = started;

    do {
        snprintf(devname, MAXPATHLEN - 1, "/dev/rfcomm%d", d->m_id);
        if ((d->m_fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
            snprintf(devname, MAXPATHLEN-1, "/dev/bluetooth/rfcomm/%d", d->m_id);
            if ((d->m_fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
                now = QTime::currentTime();
                if (started > now) // crossed midnight
                    started = now;

                // sleep 100 ms, so we don't use up CPU cycles all the time.
                struct timeval usleep_tv;
                usleep_tv.tv_sec = 0;
                usleep_tv.tv_usec = 100000;
                select(0, 0, 0, 0, &usleep_tv);
            }
        }
    } while ((d->m_fd < 0) && (started.msecsTo(now) < 1000));

    if (d->m_fd < 0) {
        qWarning() << "RFCOMM Tty created successfully, but no /dev entry found!";
        release_dev_with_socket(d->m_id, socket->socketDescriptor());
        d->m_id = -1;

        setError(QBluetoothRfcommSerialPort::CreationError);
        return;
    }

    ::fcntl(d->m_fd, F_SETFD, FD_CLOEXEC);
    d->m_devName = QString(devname);
    d->m_flags = deviceFlags;
}

/*!
    This method tries to establish a connection to the remote device with address \a remote
    on \a channel.  The local device is given by \a local.  This method will return true
    if the request could be started, and false otherwise.  The connected() signal
    will be sent once the operation completes.

    You can cancel the connection process at any time by calling disconnect().  If the
    process is cancelled, the error() signal will be sent with the
    QBluetoothRfcommSerialPort::ConnectionCancelled error set.

    \sa error(), connected()
*/
bool QBluetoothRfcommSerialPort::connect(const QBluetoothAddress &local,
                                         const QBluetoothAddress& remote, int channel)
{
    // This function deliberately does not take any flags
    // This is so that if the hcid Rfcomm hierarchy is ever moved to the
    // non-experimental state, we could just use that interface over
    // dbus instead of implementing the same code here

    return d->initiateConnect(local, remote, channel);
}

/*!
    Returns true if able to disconnects the serial port; otherwise returns false. The underlying device will be removed unless the
    QBluetoothRfcommSerialPort::KeepAlive flag is set.  The disconnected() signal
    will be sent once the disconnection is complete.

    \sa disconnected()
*/
bool QBluetoothRfcommSerialPort::disconnect()
{
    if ((d->m_fd == -1) && (!d->connectPending())) {
        return false;
    }

    if (d->connectPending()) {
        return d->cancelConnect();
    }

    return d->disconnect();
}

/*!
    Returns the RFCOMM tty device which is being managed by the current instance of this class.
    If the instance is not managing a device, an empty string is returned.

    \sa id()
 */
QString QBluetoothRfcommSerialPort::device() const
{
    if ( d->m_id < 0 )
        return QString();

    return d->m_devName;
}

/*!
    Returns the device id of the associated RFCOMM device.  If no device is found,
    returns -1.  The device id is usually in the range of 0-31.

    \sa device()
*/
int QBluetoothRfcommSerialPort::id() const
{
    return d->m_id;
}

/*!
    Returns the flags that the serial port has been opened with.
*/
QBluetoothRfcommSerialPort::Flags QBluetoothRfcommSerialPort::flags() const
{
    return d->m_flags;
}

/*!
    Returns the last error that has occurred.

    \sa errorString()
*/
QBluetoothRfcommSerialPort::Error QBluetoothRfcommSerialPort::error() const
{
    return d->m_error;
}

/*!
    Returns the human readable form of the last error that has occurred.

    \sa error()
*/
QString QBluetoothRfcommSerialPort::errorString() const
{
    return d->m_errorString;
}

/*!
    \internal

    Sets the error to \a error.  This also updates the error string.

    \sa error(), errorString()
*/
void QBluetoothRfcommSerialPort::setError(QBluetoothRfcommSerialPort::Error error)
{
    d->m_error = error;

    switch (error) {
        case NoError:
            d->m_errorString = QString();
            break;

        case SocketNotConnected:
            d->m_errorString = qApp->translate("QBluetoothRfcommSerialPort", "Socket is not connected");
            break;

        case ConnectionFailed:
            d->m_errorString = qApp->translate("QBluetoothRfcommSerialPort", "Connection failed");
            break;

        case ConnectionCancelled:
            d->m_errorString = qApp->translate("QBluetoothRfcommSerialPort", "Connection cancelled");
            break;

        case CreationError:
            d->m_errorString = qApp->translate("QBluetoothRfcommSerialPort", "Serial port could not be created");
            break;

        default:
            d->m_errorString = qApp->translate("QBluetoothRfcommSerialPort", "Unknown error");
            break;
    };
}

/*!
    Returns the address of the remote device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa localAddress(), remoteChannel()
 */
QBluetoothAddress QBluetoothRfcommSerialPort::remoteAddress() const
{
    if (d->m_fd == -1)
        return QBluetoothAddress::invalid;

    return d->m_remote;
}

/*!
    Returns the address of the local device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa remoteAddress()
 */
QBluetoothAddress QBluetoothRfcommSerialPort::localAddress() const
{
    if (d->m_fd == -1)
        return QBluetoothAddress::invalid;

    return d->m_local;
}

/*!
    Returns the RFCOMM channel of the remote device.  If the socket is not
    currently connected, returns -1.

    \sa remoteAddress()
 */
int QBluetoothRfcommSerialPort::remoteChannel() const
{
    if (d->m_fd == -1)
        return -1;

    return d->m_channel;
}

/*!
    Returns true if able to release the RFCOMM device with \a id; otherwise returns false.

    \sa listDevices()
*/
bool QBluetoothRfcommSerialPort::releaseDevice(int id)
{
    int sockfd = ::socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_RFCOMM );
    if ( sockfd < 0 ) {
        perror("QBluetoothRfcommSerialPort::release");
        return false;
    }

    release_dev_with_socket(id, sockfd);

    ::close(sockfd);

    return true;
}

struct filter_data
{
    QBluetoothAddress local;
    QList<int> result;
};

static void no_filter(void *userData, struct rfcomm_dev_info *info)
{
    filter_data *ret = static_cast<filter_data *>(userData);
    ret->result.append(info->id);
}

/*!
    Returns a list of all RFCOMM device bindings.

    \sa releaseDevice()
*/
QList<int> QBluetoothRfcommSerialPort::listDevices()
{
    filter_data data;
    foreach_device(&data, no_filter);
    return data.result;
}

static void addr_filter(void *userData, struct rfcomm_dev_info *info)
{
    filter_data *ret = static_cast<filter_data *>(userData);

    QBluetoothAddress actual = QBluetoothAddress(bdaddr2str(&info->src));

    if (ret->local == actual)
        ret->result.append(info->id);
}

/*!
    Returns a list of all RFCOMM devices for a particular Bluetooth
    adapter given by \a local.

    \sa releaseDevice()
*/
QList<int> QBluetoothRfcommSerialPort::listDevices(const QBluetoothLocalDevice &local)
{
    filter_data data;
    data.local = local.address();
    foreach_device(&data, addr_filter);

    return data.result;
}

/*!
    \fn void QBluetoothRfcommSerialPort::connected(const QString &device)

    This signal is emitted whenever the RFCOMM serial port is connected.  The \a device
    parameter holds the device name of the created serial port.

    \sa connect()
*/

/*!
    \fn void QBluetoothRfcommSerialPort::disconnected()

    This signal is emitted whenever the RFCOMM serial port is disconnected.

    \sa disconnect()
*/

/*!
    \fn void QBluetoothRfcommSerialPort::error(QBluetoothRfcommSerialPort::Error err)

    This signal is emitted whenever an error occurs.  The \a err parameter holds the
    error that has occurred.

    \sa connect()
*/

#include "qbluetoothrfcommserialport.moc"
