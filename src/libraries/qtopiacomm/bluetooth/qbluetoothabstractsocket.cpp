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

#include <private/qringbuffer_p.h>
#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QtAlgorithms>
#include <QPointer>
#include <qglobal.h>

#include <qbluetoothabstractsocket.h>
#include "qbluetoothabstractsocket_p.h"
#include <qbluetoothaddress.h>
#include "qbluetoothsocketengine_p.h"

static const int QBLUETOOTHRFCOMMSOCKET_DEFAULT_READBUFFERSIZE = 1024;
static const int QBLUETOOTHRFCOMMSOCKET_DEFAULT_WRITEBUFFERSIZE = 4096;

static const int QT_BLUETOOTH_CONNECT_TIMEOUT = 15000;

QBluetoothAbstractSocketPrivate::QBluetoothAbstractSocketPrivate(bool isBuffered)
    : m_writeBuffer(QBLUETOOTHRFCOMMSOCKET_DEFAULT_WRITEBUFFERSIZE),
    m_readBuffer(QBLUETOOTHRFCOMMSOCKET_DEFAULT_READBUFFERSIZE),
    m_isBuffered(isBuffered)
{
    m_error = QBluetoothAbstractSocket::NoError;
    m_state = QBluetoothAbstractSocket::UnconnectedState;
    m_fd = -1;
    m_readNotifier = 0;
    m_writeNotifier = 0;
    m_timer = 0;
    m_readBufferCapacity = 0;

    m_readSocketNotifierCalled = false;
    m_readSocketNotifierState = false;
    m_readSocketNotifierStateSet = false;
    m_emittedReadyRead = false;
    m_emittedBytesWritten = false;

    m_readMtu = 0;
    m_writeMtu = 0;

    m_engine = new QBluetoothSocketEngine;
}

QBluetoothAbstractSocketPrivate::~QBluetoothAbstractSocketPrivate()
{
    delete m_engine;
}

void QBluetoothAbstractSocketPrivate::setupNotifiers()
{
    if (!m_readNotifier) {
        m_readNotifier = new QSocketNotifier(m_fd,
                                             QSocketNotifier::Read,
                                             this);
    }

    m_readNotifier->setEnabled(true);
    QObject::connect(m_readNotifier, SIGNAL(activated(int)),
                     this, SLOT(readActivated()));

    if (!m_writeNotifier) {
        m_writeNotifier = new QSocketNotifier(m_fd,
                                              QSocketNotifier::Write,
                                              this);
    }

    m_writeNotifier->setEnabled(false);
    QObject::connect(m_writeNotifier, SIGNAL(activated(int)),
                     this, SLOT(writeActivated()));
}

void QBluetoothAbstractSocketPrivate::resetNotifiers()
{

    if (m_readNotifier) {
        delete m_readNotifier;
        m_readNotifier = 0;
    }

    if (m_writeNotifier) {
        delete m_writeNotifier;
        m_writeNotifier = 0;
    }

    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = 0;
    }

    m_readSocketNotifierCalled = false;
    m_readSocketNotifierState = false;
    m_readSocketNotifierStateSet = false;
    m_emittedReadyRead = false;
    m_emittedBytesWritten = false;
}

void QBluetoothAbstractSocketPrivate::abortConnectionAttempt()
{
    if (m_writeNotifier) {
        disconnect(m_writeNotifier, SIGNAL(activated(int)),
                   this, SLOT(testConnected()));
        delete m_writeNotifier;
        m_writeNotifier = 0;
    }
    m_engine->close(m_fd);
    m_fd = -1;
    m_state = QBluetoothAbstractSocket::UnconnectedState;
    m_parent->setError(QBluetoothAbstractSocket::TimeoutError);
    QPointer<QBluetoothAbstractSocket> that = m_parent;
    emit m_parent->error(m_error);
    if (that)
        emit m_parent->stateChanged(m_state);
}

void QBluetoothAbstractSocketPrivate::testConnected()
{
    if (m_writeNotifier)
        disconnect(m_writeNotifier, SIGNAL(activated(int)),
                   this, SLOT(testConnected()));

    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = 0;
    }

    if (!m_engine->testConnected(m_fd)) {
        m_parent->setError(m_engine->error());
        m_state = QBluetoothAbstractSocket::UnconnectedState;
        m_engine->close(m_fd);
        m_fd = -1;
        QPointer<QBluetoothAbstractSocket> that = m_parent;
        emit m_parent->error(m_error);
        if (that)
            emit m_parent->stateChanged(m_state);

        delete m_writeNotifier;
        m_writeNotifier = 0;
        return;
    }

    m_parent->readSocketParameters(m_fd);
    setupNotifiers();

    m_state = QBluetoothAbstractSocket::ConnectedState;

    QIODevice::OpenMode mode = QIODevice::ReadWrite;
    if (!m_isBuffered)
        mode |= QIODevice::Unbuffered;
    m_parent->setOpenMode(mode);

    QPointer<QBluetoothAbstractSocket> that = m_parent;
    emit m_parent->connected();
    if (that)
        emit m_parent->stateChanged(m_state);
}

bool QBluetoothAbstractSocketPrivate::readData()
{
    do {
        qint64 bytesToRead = m_readMtu ? m_readMtu : 4096;

        if (m_readBufferCapacity && (bytesToRead > (m_readBufferCapacity - m_readBuffer.size()))) {
            if (m_readMtu)
                bytesToRead = 0;
            else
                bytesToRead = m_readBufferCapacity - m_readBuffer.size();
        }

        if (bytesToRead == 0)
            break;

        char *ptr = m_readBuffer.reserve(bytesToRead);
        qint64 readBytes = m_engine->readFromSocket(m_fd, ptr, bytesToRead);

        if (readBytes == -2) {
            m_readBuffer.chop(bytesToRead);
            return true;
        }
        else if (readBytes == -1) {
            m_readBuffer.chop(bytesToRead);
            m_parent->setError(m_engine->error());
            emit m_parent->error(m_error);
            return false;
        }

        m_readBuffer.chop(int(bytesToRead - (readBytes < 0 ? qint64(0) : readBytes)));

        if (readBytes == 0)
            return false;
    } while (1);

    return true;
}

bool QBluetoothAbstractSocketPrivate::initiateDisconnect()
{
    if (m_state == QBluetoothAbstractSocket::UnconnectedState)
        return true;

    if (m_readNotifier)
        m_readNotifier->setEnabled(false);
    if (m_state != QBluetoothAbstractSocket::ClosingState) {
        m_state = QBluetoothAbstractSocket::ClosingState;
        emit m_parent->stateChanged(m_state);
    }

    if (m_writeBuffer.size() > 0) {
        m_writeNotifier->setEnabled(true);
        return true;
    }

    resetNotifiers();
    m_state = QBluetoothAbstractSocket::UnconnectedState;
    m_engine->close(m_fd);
    m_fd = -1;

    if (m_readBuffer.isEmpty()) {
        m_parent->QIODevice::close();
    }

    QPointer<QBluetoothAbstractSocket> that = m_parent;
    emit m_parent->disconnected();

    if (!that)
        return true;

    emit m_parent->stateChanged(m_state);

    if (!that)
        return true;

    m_parent->resetSocketParameters();

    return true;
}

bool QBluetoothAbstractSocketPrivate::readActivated()
{
    if (m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    if (m_readSocketNotifierCalled) {
        if (!m_readSocketNotifierStateSet) {
            m_readSocketNotifierStateSet = true;
            m_readSocketNotifierState = m_readNotifier->isEnabled();
            m_readNotifier->setEnabled(false);
        }
    }
    m_readSocketNotifierCalled = true;

    if (!m_isBuffered)
        m_readNotifier->setEnabled(false);

    qint64 newBytes = 0;
    bool shouldInitiateDisconnect = false;

    if (m_isBuffered) {
        if (m_readBufferCapacity && m_readBuffer.size() >= m_readBufferCapacity) {
            m_readSocketNotifierCalled = false;
            return false;
        }

        newBytes = m_readBuffer.size();

        if (!readData()) {
            // Even if the socket returned EOF
            // Should still emit ready read, etc
            shouldInitiateDisconnect = true;
        }

        newBytes = m_readBuffer.size() - newBytes;

        // If read buffer is full, disable the read notifier
        if (m_readBufferCapacity &&
            m_readBuffer.size() == m_readBufferCapacity) {
            m_readNotifier->setEnabled(false);
        }
    }

    if (!m_emittedReadyRead && (newBytes || !m_isBuffered)) {
        m_emittedReadyRead = true;
        emit m_parent->readyRead();
        m_emittedReadyRead = false;
    }

    if ((m_state == QBluetoothAbstractSocket::UnconnectedState) ||
            (m_state == QBluetoothAbstractSocket::ClosingState)) {
        m_readSocketNotifierCalled = false;
        return true;
    }

    if (m_readSocketNotifierStateSet && m_readNotifier && 
        m_readSocketNotifierState != m_readNotifier->isEnabled()) {
        m_readNotifier->setEnabled(m_readSocketNotifierState);
        m_readSocketNotifierStateSet = false;
    }

    m_readSocketNotifierCalled = false;

    if (shouldInitiateDisconnect) {
        initiateDisconnect();
        return false;
    }

    return true;
}

bool QBluetoothAbstractSocketPrivate::writeActivated()
{
    int tmp = m_writeBuffer.size();
    if (!flush())
        return false;

    return m_writeBuffer.size() < tmp;
}

bool QBluetoothAbstractSocketPrivate::flush()
{
    if (m_writeBuffer.isEmpty())
        return false;

    int nextSize = m_writeBuffer.nextDataBlockSize();

    if (m_writeMtu && (nextSize > m_writeMtu))
        nextSize = m_writeMtu;
    const char *ptr = m_writeBuffer.readPointer();

    // Attempt to write it all in one chunk.
    qint64 written = m_engine->writeToSocket(m_fd, ptr, nextSize);
    if (written < 0) {
        m_parent->setError(m_engine->error());
        emit m_parent->error(m_error);
        // an unexpected error so close the socket.
        qWarning("QBluetoothAbstractSocket::flush: unexpected error");
        m_parent->abort();
        return false;
    }

    m_writeBuffer.free(written);
    if (written > 0) {
        if (!m_emittedBytesWritten) {
            m_emittedBytesWritten = true;
            emit m_parent->bytesWritten(written);
            m_emittedBytesWritten = false;
        }
    }

    if (m_writeBuffer.isEmpty() && m_writeNotifier && m_writeNotifier->isEnabled()) {
        m_writeNotifier->setEnabled(false);
    }

    if (m_state == QBluetoothAbstractSocket::ClosingState) {
        m_parent->close();
    }

    return true;
}

qint64 QBluetoothAbstractSocketPrivate::bytesAvailable() const
{
    return m_engine->bytesAvailable(m_fd);
}

/*!
    \internal

    Clients of this class can set the \a mtu to be used on reading.  This
    value will be used as the maximum number of bytes to read on a socket.

    A value of 0 means there is no MTU.

    \sa readMtu()
*/
void QBluetoothAbstractSocket::setReadMtu(int mtu)
{
    m_data->m_readMtu = mtu;
}

/*!
    \internal

    Clients of this class can set the \a mtu to be used when writing.  This
    value will be used as the maximum number of bytes to write on a socket.

    A value of 0 means there is no MTU.

    \sa writeMtu()
*/
void QBluetoothAbstractSocket::setWriteMtu(int mtu)
{
    m_data->m_writeMtu = mtu;
}

/*!
    \internal

    Returns the current read MTU.

    \sa setReadMtu()
*/
int QBluetoothAbstractSocket::readMtu() const
{
    return m_data->m_readMtu;
}

/*!
    \internal

    Returns the current write MTU.

    \sa setWriteMtu()
*/
int QBluetoothAbstractSocket::writeMtu() const
{
    return m_data->m_writeMtu;
}

/*!
    \class QBluetoothAbstractSocket
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothAbstractSocket class represents an abstract Bluetooth client socket.

    QBluetoothAbstractSocket is a common base class used by all Bluetooth
    sockets.  This class implements functionality common to all socket types.

    Clients that need to interact with RFCOMM based services should
    use the QBluetoothRfcommSocket class.  For interacting with
    L2CAP based services, use the QBluetoothL2CapSocket class.
    Headset and Handsfree profile implementations might need to use
    the QBluetoothScoSocket class.

    At any time, the QBluetoothAbstractSocket has a state (returned by
    state()). Upon creation, the initial state is
    QBluetoothAbstractSocket::UnconnectedState.

    After calling connect(), the socket enters
    the QBluetoothAbstractSocket::ConnectingState. If connection is
    established, the socket enters QBluetoothAbstractSocket::ConnectedState
    and emits connected().

    If an error occurs at any time, the error() signal is emitted.
    Whenever the state changes, stateChanged() is emitted.
    For convenience, isValid() returns true if the socket is ready for
    reading and writing.

    Read or write data to/from the socket by calling read() or write(),
    or use the convenience functions readLine() and readAll().
    QBluetoothAbstractSocket also inherits getChar(), putChar(),
    and ungetChar() from QIODevice, which work on single bytes.
    For every chunk of data that has been written to the socket,
    the bytesWritten() signal is emitted.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. bytesAvailable() then returns the number of bytes
    that are available for reading. Typically, you would connect the
    readyRead() signal to a slot and read all available data there.
    If you don't read all the data at once, the remaining data will
    still be available later, and any new incoming data will be
    appended to QBluetoothAbstractSocket's internal read buffer. To limit the
    size of the read buffer, call setReadBufferSize().

    To close the socket, call disconnect(). Once disconnect() is
    called, QBluetoothAbstractSocket enters the
    QBluetoothAbstractSocket::ClosingState and emits the stateChanged()
    signal.  After all pending data has been written to the socket,
    QBluetoothAbstractSocket actually closes the socket, enters the
    QBluetoothAbstractSocket::ClosedState,
    and emits disconnected(). If no data is pending when disconnect() is
    called, the connection is disconnected immediately.  If you want to
    abort a connection immediately, discarding all pending data, call
    abort() instead.

    QBluetoothAbstractSocket provides a set of functions that suspend the
    calling thread until certain signals are emitted. These functions
    can be used to implement blocking sockets:

    \list
        \o waitForConnected() blocks until a connection has been established.

        \o waitForReadyRead() blocks until new data is available for
           reading.

        \o waitForBytesWritten() blocks until one payload of data has been
           written to the socket.

        \o waitForDisconnected() blocks until the connection has closed.
    \endlist

    \sa QBluetoothRfcommSocket, QBluetoothL2CapSocket, QBluetoothScoSocket
    \sa QBluetoothL2CapDatagramSocket

    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothAbstractSocket::SocketState
    \brief State of the rfcomm socket.

    \value UnconnectedState The socket is not connected.
    \value ConnectingState The socket is being connected.
    \value ConnectedState The socket is connected.
    \value BoundState The socket has been bound.
    \value ClosingState The socket is being closed.
 */

/*!
    \enum QBluetoothAbstractSocket::SocketError
    \brief Error that last occurred on the rfcomm socket.

    \value NoError No error has occurred.
    \value AccessError The client has inadequate permissions to access the socket.
    \value ResourceError The kernel has run out of sockets.
    \value BindError The socket could not be bound to a particular address.
    \value ConnectionRefused The remote host has refused a connection.
    \value HostDownError The remote host could not be contacted.
    \value NetworkError A network error has occurred, e.g. device moved out of range.
    \value TimeoutError Operation has timed out.
    \value RemoteHostClosedError Remote host has closed the connection.
    \value BusyError The system is busy.
    \value HostUnreachableError The remote host could not be reached.
    \value UnsupportedOperationError The operation is not supported.
    \value AddressInUseError Address is currently in use.
    \value AddressNotAvailableError Address is not available.
    \value UnknownError Unknown error has occurred.
 */

/*!
    \internal

    Constructs a new QBluetoothAbstractSocket with \a parent. The socket is not
    connected.  Stream sockets (RFCOMM) are buffered.
    Seqpacket and Datagram sockets (SCO, L2CAP) are not buffered.
 */
QBluetoothAbstractSocket::QBluetoothAbstractSocket(QBluetoothAbstractSocketPrivate *data,
                                                   QObject *parent)
    : QIODevice(parent)
{
    setOpenMode(QIODevice::NotOpen);

    m_data = data;
    m_data->m_parent = this;
}

/*!
    Deconstructs a QBluetoothAbstractSocket.
    If the socket is in any state other than UnconnectedState, the current
    connection is aborted.
 */
QBluetoothAbstractSocket::~QBluetoothAbstractSocket()
{
    if (m_data->m_state != QBluetoothAbstractSocket::UnconnectedState)
        abort();

    if (m_data)
        delete m_data;
}

/*!
    Returns the socket descriptor for the socket if the socket is currently
    active (e.g. not in UnconnectedState).  Otherwise returns -1.

    \sa state()
 */
int QBluetoothAbstractSocket::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    Initializes the QBluetoothAbstractSocket with the native descriptor
    \a socketDescriptor.  The socket is put into the \a state and
    opened in \a openMode.

    Returns true on successful completion of the request; otherwise returns false.

    \sa socketDescriptor()
 */
bool QBluetoothAbstractSocket::setSocketDescriptor(int socketDescriptor,
        QBluetoothAbstractSocket::SocketState state,
        QIODevice::OpenMode openMode)
{
    m_data->resetNotifiers();
    m_data->m_fd = socketDescriptor;

    // Update the local, remote and channel
    bool ret = readSocketParameters(socketDescriptor);

    if (!ret) {
        m_data->m_fd = -1;
        return false;
    }

    m_data->m_engine->setSocketOption(socketDescriptor,
                                      QBluetoothSocketEngine::NonBlockingOption);

    m_data->setupNotifiers();
    setOpenMode(openMode);

    if (m_data->m_state != state) {
        m_data->m_state = state;
        emit stateChanged(m_data->m_state);
    }

    return true;
}

/*!
    Closes the socket. Pending data will not be flushed first.
    The socket will be immediately closed.

    \sa abort(), disconnect()
 */
void QBluetoothAbstractSocket::close()
{
    if (QIODevice::isOpen())
        QIODevice::close();

    if (m_data->m_state != QBluetoothAbstractSocket::UnconnectedState) {
        m_data->m_readBuffer.clear();
        m_data->m_writeBuffer.clear();
        m_data->initiateDisconnect();
    }
}

/*!
    Returns the last error that has occurred.

    \sa state()
 */
QBluetoothAbstractSocket::SocketError QBluetoothAbstractSocket::error() const
{
    return m_data->m_error;
}

/*!
    Returns the state of the socket.

    \sa error()
 */
QBluetoothAbstractSocket::SocketState QBluetoothAbstractSocket::state() const
{
    return m_data->m_state;
}

/*!
    Returns the number of bytes that are waiting to be read.

    \sa bytesToWrite(), read()
 */
qint64 QBluetoothAbstractSocket::bytesAvailable() const
{
    qint64 available = QIODevice::bytesAvailable();

    if (m_data->m_isBuffered)
        available += qint64(m_data->m_readBuffer.size());
    else
        available += m_data->bytesAvailable();

    return available;
}

/*!
    Returns the number of bytes which are pending to be written.

    \sa bytesAvailable(), write()
 */
qint64 QBluetoothAbstractSocket::bytesToWrite() const
{
    return qint64(m_data->m_writeBuffer.size());
}

/*!
    Returns the size of the internal read buffer.  This limits the
    amount of data that the client can receive before you call read()
    or readAll().

    A read buffer size of 0 (the default) means that the buffer has no
    size limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
 */
qint64 QBluetoothAbstractSocket::readBufferSize() const
{
    return m_data->m_readBufferCapacity;
}

/*!
    Sets the capacity of QBluetoothAbstractSocket's internal read buffer to be
    \a size bytes.  If \a size is 0 the buffer has unlimited capacity.  This is
    the default.

    \sa readBufferSize()
 */
void QBluetoothAbstractSocket::setReadBufferSize(qint64 size)
{
    Q_ASSERT(size > 0);
    m_data->m_readBufferCapacity = size;
}

/*!
    Returns true if a line of data can be read from the socket;
    otherwise returns false.

    \sa readLine()
 */
bool QBluetoothAbstractSocket::canReadLine() const
{
    bool hasLine = m_data->m_readBuffer.canReadLine();

    return hasLine || QIODevice::canReadLine();
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::isSequential() const
{
    return true;
}

static int timeout_value(int timeout, int elapsed)
{
    if (timeout < 0)
        return -1;

    int msecs = timeout - elapsed;

    return msecs < 0 ? 0 : msecs;
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::waitForReadyRead(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while ((msecs < 0) || (stopWatch.elapsed() < msecs)) {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        QBluetoothSocketEngine::SelectTypes types =
                QBluetoothSocketEngine::SelectRead;
        if (!m_data->m_writeBuffer.isEmpty())
            types = types | QBluetoothSocketEngine::SelectWrite;

        int timeout = timeout_value(msecs, stopWatch.elapsed());
        if (!m_data->m_engine->waitFor(types, m_data->m_fd, timeout,
            &timedOut, &readyToRead, &readyToWrite)) {
            setError(m_data->m_engine->error());
            emit error(m_data->m_error);
            if (!timedOut)
                abort();
            return false;
        }

        if (readyToWrite)
            m_data->writeActivated();

        if (readyToRead && m_data->readActivated())
            return true;

        if (state() != ConnectedState)
            return false;
    }

    return false;
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::waitForBytesWritten(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    if (m_data->m_writeBuffer.isEmpty())
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while ((msecs < 0) || (stopWatch.elapsed() < msecs)) {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        QBluetoothSocketEngine::SelectTypes types =
                QBluetoothSocketEngine::SelectTypes(QBluetoothSocketEngine::SelectRead |
                                                    QBluetoothSocketEngine::SelectWrite);

        int timeout = timeout_value(msecs, stopWatch.elapsed());
        if (!m_data->m_engine->waitFor(types, m_data->m_fd, timeout,
            &timedOut, &readyToRead, &readyToWrite)) {
            setError(m_data->m_engine->error());
            emit error(m_data->m_error);
            if (!timedOut)
                abort();
            return false;
        }

        if (readyToRead)
            m_data->readActivated();

        if (readyToWrite && m_data->writeActivated())
            return true;

        if (state() != ConnectedState)
            return false;
    }

    return false;
}

/*!
    \reimp
 */
qint64 QBluetoothAbstractSocket::readData(char *data, qint64 maxsize)
{
    if (!m_data->m_isBuffered) {
        qint64 readBytes = m_data->m_engine->readFromSocket(m_data->m_fd, data, maxsize);

        if (readBytes == -1) {
            setError(m_data->m_engine->error());
            emit error(m_data->m_error);
        }

        if (!m_data->m_readNotifier->isEnabled())
            m_data->m_readNotifier->setEnabled(true);

        return readBytes;
    }

    if (m_data->m_readBuffer.isEmpty())
        return qint64(0);

    if (!m_data->m_readNotifier->isEnabled()) {
        m_data->m_readNotifier->setEnabled(true);
    }

    if (maxsize == 1) {
        *data = m_data->m_readBuffer.getChar();

        if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState &&
            m_data->m_readBuffer.isEmpty()) {
            QIODevice::close();
        }

        return 1;
    }

    qint64 bytesToRead = qMin(qint64(m_data->m_readBuffer.size()), maxsize);
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        const char *ptr = m_data->m_readBuffer.readPointer();
        int bytesToReadFromThisBlock = qMin(int(bytesToRead - readSoFar),
                                            m_data->m_readBuffer.nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        m_data->m_readBuffer.free(bytesToReadFromThisBlock);
    }

    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState &&
        m_data->m_readBuffer.isEmpty()) {
        QIODevice::close();
    }

    return readSoFar;
}

/*!
    \reimp
 */
qint64 QBluetoothAbstractSocket::readLineData(char *data, qint64 maxsize)
{
    return QIODevice::readLineData(data, maxsize);
}

/*!
    \reimp
 */
qint64 QBluetoothAbstractSocket::writeData(const char *data, qint64 size)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState) {
        setError(QBluetoothAbstractSocket::UnknownError);
        return -1;
    }

    if (!m_data->m_isBuffered) {
        qint64 written = m_data->m_engine->writeToSocket(m_data->m_fd, data, size);

        if (written >= 0)
            emit bytesWritten(written);
        else {
            setError(m_data->m_engine->error());
            emit error(m_data->m_error);
            abort();
        }

        return written;
    }

    char *ptr = m_data->m_writeBuffer.reserve(size);
    if (size == 1)
        *ptr = *data;
    else
        memcpy(ptr, data, size);

    if (!m_data->m_writeBuffer.isEmpty()) {
        m_data->m_writeNotifier->setEnabled(true);
    }

    return size;
}

/*!
    Waits until the socket is connected, up to \a msecs milliseconds.  If
    the connection has been established, this function returns true; otherwise
    returns false.  In the case where it returns false, you can call error()
    to determine the cause of the error.

    This is a blocking function call. Its use is not advised in a
    single-threaded GUI application, since the whole application will
    stop responding until the function returns.
    waitForNewConnected() is mostly useful when there is no event
    loop available.

    \sa connect(), connected()
 */
bool QBluetoothAbstractSocket::waitForConnected(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectedState)
        return true;

    bool timedOut = false;
    QBluetoothSocketEngine::SelectTypes types =
            QBluetoothSocketEngine::SelectWrite;

    if (!m_data->m_engine->waitFor(types, m_data->m_fd, msecs, &timedOut)) {
        setError(m_data->m_engine->error());
        emit error(m_data->m_error);
        if (!timedOut)
            abort();
        return false;
    }

    m_data->testConnected();
    if (state() == ConnectedState)
        return true;

    return false;
}

/*!
    Waits until the socket is disconnected, up to \a msecs milliseconds.  If
    the connection has been terminated, this function returns true; otherwise
    returns false.  In the case where it returns false, you can call error()
    to determine the cause of the error.

    This is a blocking function call. Its use is not advised in a
    single-threaded GUI application, since the whole application will
    stop responding until the function returns.
    waitForDisconnected() is mostly useful when there is no event
    loop available.

    \sa disconnect(), close(), disconnected()
 */
bool QBluetoothAbstractSocket::waitForDisconnected(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while ((msecs < 0) || (stopWatch.elapsed() < msecs)) {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        QBluetoothSocketEngine::SelectTypes types =
                QBluetoothSocketEngine::SelectRead;

        if (!m_data->m_writeBuffer.isEmpty())
            types = types | QBluetoothSocketEngine::SelectWrite;

        int timeout = timeout_value(msecs, stopWatch.elapsed());
        if (!m_data->m_engine->waitFor(types, m_data->m_fd, timeout,
            &timedOut, &readyToRead, &readyToWrite)) {
            setError(m_data->m_engine->error());
            emit error(m_data->m_error);
            if (!timedOut)
                abort();
            return false;
        }

        if (readyToRead)
            m_data->readActivated();

        if (readyToWrite)
            m_data->writeActivated();

        if (state() != UnconnectedState)
            return true;
    }

    return false;
}

/*!
    Aborts the current connection and resets the socket. Unlike
    disconnect(), this function immediately closes the socket, clearing
    any pending data in the write buffer.

    \sa disconnect(), close()
 */
void QBluetoothAbstractSocket::abort()
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return;

    if (m_data->m_timer) {
        m_data->m_timer->stop();
        delete m_data->m_timer;
        m_data->m_timer = 0;
    }

    m_data->m_writeBuffer.clear();
    close();
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::atEnd() const
{
    return QIODevice::atEnd() && (!isOpen() || m_data->m_readBuffer.isEmpty());
}

/*!
    This function writes as much as possible from the internal write buffer to
    the underlying network socket, without blocking. If any data was written,
    this function returns true; otherwise false is returned.

    \sa write(), waitForBytesWritten()
 */
bool QBluetoothAbstractSocket::flush()
{
    return m_data->flush();
}

/*!
    \internal

    Clients of this class can call this method to initiate connection
    procedures common to all Bluetooth socket types.  The \a socket parameter
    holds the socket file descriptor to use.  The \a addr structure holds the
    sockaddr structure of the remote peer to connect to and \a size holds the
    size of the sockaddr structure.

    Returns true on successful completion of the request; otherwise returns false.
*/
bool QBluetoothAbstractSocket::handleConnect(int socket,
                                             QBluetoothAbstractSocket::SocketState state)
{

    m_data->m_writeBuffer.clear();
    m_data->m_readBuffer.clear();
    m_data->m_state = state;

    m_data->m_fd = socket;

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!m_data->m_writeNotifier)
            m_data->m_writeNotifier = new QSocketNotifier(m_data->m_fd,
                QSocketNotifier::Write);

        QObject::connect(m_data->m_writeNotifier, SIGNAL(activated(int)),
                    m_data, SLOT(testConnected()));

        if (!m_data->m_timer) {
            m_data->m_timer = new QTimer(this);
            QObject::connect(m_data->m_timer, SIGNAL(timeout()),
                             m_data, SLOT(abortConnectionAttempt()));
        }
        m_data->m_timer->start(QT_BLUETOOTH_CONNECT_TIMEOUT);
    }

    if ((m_data->m_state != QBluetoothAbstractSocket::ConnectedState) &&
        (m_data->m_state != QBluetoothAbstractSocket::ConnectingState)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        setError(m_data->m_engine->error());
        emit error(m_data->m_error);
        return false;
    }

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectedState) {
        QPointer<QBluetoothAbstractSocket> that = this;
        emit stateChanged(m_data->m_state);
        if (that && (m_data->m_state == QBluetoothAbstractSocket::ConnectedState))
            emit connected();
    }

    return true;
}


/*!
    Attempts to close the socket.  If there is pending data waiting to be
    written, the socket will enter ClosingState and wait until all data has
    been written.  Eventually it will enter UnconnectedState and emit
    the disconnected() signal.

    Returns true on successful completion of the request; otherwise returns false.

    \sa close()
 */
bool QBluetoothAbstractSocket::disconnect()
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState) {
        return false;
    }

    return m_data->initiateDisconnect();
}

/*!
    \internal

    Can be used by the clients of this class to set the \a error that might have occurred.
    This function is generally used from the specific socket connect implementation.
*/
void QBluetoothAbstractSocket::setError(QBluetoothAbstractSocket::SocketError error)
{
    m_data->m_error = error;
    setErrorString(QBluetoothSocketEngine::getErrorString(error));
}

/*!
    \internal

    The clients of this class can override this method to read specific socket
    parameters from the socket given by \a sockfd.  This method is called
    when the socket initially enters the connected state.

    If the parameters could not be obtained, the clients should return false,
    and should return true otherwise.

    The default implementation returns true.
*/
bool QBluetoothAbstractSocket::readSocketParameters(int sockfd)
{
    Q_UNUSED(sockfd);
    return true;
}

/*!
    \internal

    The clients of this class should override this method to reset the
    specific socket parameters.  This method is called when a socket enters
    the disconnected state.
*/
void QBluetoothAbstractSocket::resetSocketParameters()
{

}

/*!
    \fn void QBluetoothAbstractSocket::connected()

    This signal is emitted once the connect() has been called
    and the rfcomm socket has been successfully connected.

    \sa connect(), disconnected()
 */

/*!
    \fn void QBluetoothAbstractSocket::disconnected()

    This signal is emitted when the socket has been disconnected.

    \sa connect(), disconnect(), abort()
 */

/*!
    \fn void QBluetoothAbstractSocket::error(QBluetoothAbstractSocket::SocketError socketError)

    This signal is emitted after an error occurred. The \a socketError parameter
    describes the type of error that has occurred.

    \sa error(), errorString()
 */

/*!
    \fn void QBluetoothAbstractSocket::stateChanged(QBluetoothAbstractSocket::SocketState socketState)

    This signal is emitted when the state of the socket has changed.  The
    \a socketState parameter holds the new state.

    \sa state()
 */
