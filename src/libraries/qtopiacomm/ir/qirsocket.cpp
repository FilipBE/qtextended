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
#include <QDebug>
#include <QtAlgorithms>
#include <QPointer>
#include <qglobal.h>

#include <qirsocket.h>
#include "qirsocketengine_p.h"

static const int QIRSOCKET_DEFAULT_READBUFFERSIZE = 1024;
static const int QIRSOCKET_DEFAULT_WRITEBUFFERSIZE = 4096;

static const int QT_IR_CONNECT_TIMEOUT = 15000;

class QIrSocketPrivate : public QObject
{
    Q_OBJECT

public:
    QIrSocketPrivate(bool isBuffered);
    ~QIrSocketPrivate();

    bool initiateDisconnect();

    bool flush();

    bool readData();

    void resetNotifiers();
    void setupNotifiers();

    qint64 bytesAvailable() const;

    void setError(QIrSocket::SocketError error);
    bool readSocketParameters(int fd);
    void resetSocketParameters();
    bool initiateConnect(int socket, const QByteArray &service, quint32 remote);
    void handleConnectError(int error);

public slots:
    void testConnected();
    void abortConnectionAttempt();
    bool readActivated();
    bool writeActivated();

public:
    QIrSocket *m_parent;
    QIrSocket::SocketError m_error;
    QIrSocket::SocketState m_state;
    int m_fd;
    QSocketNotifier *m_readNotifier;
    QSocketNotifier *m_writeNotifier;
    QTimer *m_timer;

    QRingBuffer m_writeBuffer;
    QRingBuffer m_readBuffer;

    bool m_readSocketNotifierCalled;
    bool m_readSocketNotifierState;
    bool m_readSocketNotifierStateSet;
    bool m_emittedReadyRead;
    bool m_emittedBytesWritten;
    qint64 m_readBufferCapacity;

    bool m_isBuffered;

    quint32 m_remote;
    QIrSocketEngine *m_engine;
};

#define SOCKET_DATA(Class) Class##Private * const m_data = static_cast<Class##Private *>(QIrSocket::m_data)

QIrSocketPrivate::QIrSocketPrivate(bool isBuffered)
    : m_writeBuffer(QIRSOCKET_DEFAULT_WRITEBUFFERSIZE),
    m_readBuffer(QIRSOCKET_DEFAULT_READBUFFERSIZE),
    m_isBuffered(isBuffered)
{
    m_error = QIrSocket::NoError;
    m_state = QIrSocket::UnconnectedState;
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

    m_remote = 0;
    m_engine = new QIrSocketEngine;
}

QIrSocketPrivate::~QIrSocketPrivate()
{
    delete m_engine;
}

void QIrSocketPrivate::setupNotifiers()
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

void QIrSocketPrivate::resetNotifiers()
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

void QIrSocketPrivate::abortConnectionAttempt()
{
    if (m_writeNotifier)
        disconnect(m_writeNotifier, SIGNAL(activated(int)),
                   this, SLOT(testConnected()));
    m_engine->close(m_fd);
    m_fd = -1;
    m_state = QIrSocket::UnconnectedState;
    setError(QIrSocket::TimeoutError);
    QPointer<QIrSocket> that = m_parent;
    emit m_parent->error(m_error);
    if (that)
        emit m_parent->stateChanged(m_state);
}

void QIrSocketPrivate::testConnected()
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
        setError(m_engine->error());
        m_state = QIrSocket::UnconnectedState;
        m_engine->close(m_fd);
        m_fd = -1;
        QPointer<QIrSocket> that = m_parent;
        emit m_parent->error(m_error);
        if (that)
            emit m_parent->stateChanged(m_state);
        return;
    }

    readSocketParameters(m_fd);
    setupNotifiers();

    m_state = QIrSocket::ConnectedState;

    QIODevice::OpenMode mode = QIODevice::ReadWrite;
    if (!m_isBuffered)
        mode |= QIODevice::Unbuffered;
    m_parent->setOpenMode(mode);

    QPointer<QIrSocket> that = m_parent;
    emit m_parent->connected();
    if (that)
        emit m_parent->stateChanged(m_state);
}

bool QIrSocketPrivate::readData()
{
    do {
        qint64 bytesToRead = 4096;

        if (m_readBufferCapacity && (bytesToRead > (m_readBufferCapacity - m_readBuffer.size()))) {
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
            setError(m_engine->error());
            emit m_parent->error(m_error);
            return false;
        }

        m_readBuffer.chop(int(bytesToRead - (readBytes < 0 ? qint64(0) : readBytes)));

        if (readBytes == 0)
            return false;
    } while (1);

    return true;
}

bool QIrSocketPrivate::initiateConnect(int socket,
                                       const QByteArray &service,
                                       quint32 remote)
{
    m_engine->setSocketOption(socket, QIrSocketEngine::NonBlockingOption);
    m_state = m_engine->connect(socket, service, remote);

    m_writeBuffer.clear();
    m_readBuffer.clear();

    if (m_state == QIrSocket::ConnectingState) {
        if (!m_writeNotifier)
            m_writeNotifier = new QSocketNotifier(socket, QSocketNotifier::Write);

        QObject::connect(m_writeNotifier, SIGNAL(activated(int)),
                    this, SLOT(testConnected()));

        if (!m_timer) {
            m_timer = new QTimer(this);
            QObject::connect(m_timer, SIGNAL(timeout()),
                             this, SLOT(abortConnectionAttempt()));
        }
        m_timer->start(QT_IR_CONNECT_TIMEOUT);
    }

    if ((m_state != QIrSocket::ConnectedState) &&
        (m_state != QIrSocket::ConnectingState)) {
        m_engine->close(socket);
        setError(m_engine->error());
        emit m_parent->error(m_error);
        return false;
    }

    m_fd = socket;

    if (m_state == QIrSocket::ConnectedState) {
        QPointer<QIrSocket> that = m_parent;
        emit m_parent->stateChanged(m_state);
        if (that && (m_state == QIrSocket::ConnectedState))
            emit m_parent->connected();
    }

    return true;
}

bool QIrSocketPrivate::initiateDisconnect()
{
    if (m_readNotifier)
        m_readNotifier->setEnabled(false);
    if (m_state != QIrSocket::ClosingState) {
        m_state = QIrSocket::ClosingState;
        emit m_parent->stateChanged(m_state);
    }

    if (m_writeBuffer.size() > 0) {
        m_writeNotifier->setEnabled(true);
        return true;
    }

    resetNotifiers();
    m_state = QIrSocket::UnconnectedState;
    m_engine->close(m_fd);
    m_fd = -1;

    if (m_readBuffer.isEmpty())
        m_parent->QIODevice::close();

    QPointer<QIrSocket> that = m_parent;
    emit m_parent->disconnected();

    if (!that)
        return true;

    emit m_parent->stateChanged(m_state);

    if (!that)
        return true;

    resetSocketParameters();

    return true;
}

bool QIrSocketPrivate::readActivated()
{
    if (m_state == QIrSocket::UnconnectedState)
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

    if ((m_state == QIrSocket::UnconnectedState) ||
            (m_state == QIrSocket::ClosingState)) {
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

bool QIrSocketPrivate::writeActivated()
{
    int tmp = m_writeBuffer.size();
    if (!flush())
        return false;

    return m_writeBuffer.size() < tmp;
}

bool QIrSocketPrivate::flush()
{
    if (m_writeBuffer.isEmpty())
        return false;

    int nextSize = m_writeBuffer.nextDataBlockSize();

    const char *ptr = m_writeBuffer.readPointer();

    // Attempt to write it all in one chunk.
    qint64 written = m_engine->writeToSocket(m_fd, ptr, nextSize);
    if (written < 0) {
        setError(m_engine->error());
        emit m_parent->error(m_error);
        // an unexpected error so close the socket.
        qWarning("QIrSocket::flush: unexpected error");
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

    if (m_state == QIrSocket::ClosingState) {
        m_parent->close();
    }

    return true;
}

qint64 QIrSocketPrivate::bytesAvailable() const
{
    return m_engine->bytesAvailable(m_fd);
}

void QIrSocketPrivate::setError(QIrSocket::SocketError error)
{
    m_error = error;
    m_parent->setErrorString(QIrSocketEngine::getErrorString(error));
}

bool QIrSocketPrivate::readSocketParameters(int socket)
{
    m_engine->readSocketParameters(socket, &m_remote);
    return true;
}

void QIrSocketPrivate::resetSocketParameters()
{
    m_remote = 0;
}

/*!
    \class QIrSocket

    \brief The QIrSocket class represents an Infrared client socket.

    At any time, the QIrSocket has a state (returned by
    state()). Upon creation, the initial state is
    QIrSocket::UnconnectedState.

    After calling connect(), the socket enters
    the QIrSocket::ConnectingState. If connection is
    established, the socket enters QIrSocket::ConnectedState
    and emits connected().

    If an error occurs at any time, the error() signal is emitted.
    Whenever the state changes, stateChanged() is emitted.
    For convenience, isValid() returns true if the socket is ready for
    reading and writing.

    Read or write data to/from the socket by calling read() or write(),
    or use the convenience functions readLine() and readAll().
    QIrSocket also inherits getChar(), putChar(),
    and ungetChar() from QIODevice, which work on single bytes.
    For every chunk of data that has been written to the socket,
    the bytesWritten() signal is emitted.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. bytesAvailable() then returns the number of bytes
    that are available for reading. Typically, you would connect the
    readyRead() signal to a slot and read all available data there.
    If you don't read all the data at once, the remaining data will
    still be available later, and any new incoming data will be
    appended to QIrSocket's internal read buffer. To limit the
    size of the read buffer, call setReadBufferSize().

    To close the socket, call disconnect(). Once disconnect() is
    called, QIrSocket enters the
    QIrSocket::ClosingState and emits the stateChanged()
    signal.  After all pending data has been written to the socket,
    QIrSocket actually closes the socket, enters the
    QIrSocket::ClosedState,
    and emits disconnected(). If no data is pending when disconnect() is
    called, the connection is disconnected immediately.  If you want to
    abort a connection immediately, discarding all pending data, call
    abort() instead.

    QIrSocket provides a set of functions that suspend the
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

    \sa QIrServer

    \ingroup qtopiair
 */

/*!
    \enum QIrSocket::SocketState
    \brief State of the infrared socket.

    \value UnconnectedState The socket is not connected.
    \value ConnectingState The socket is being connected.
    \value ConnectedState The socket is connected.
    \value BoundState The socket has been bound.
    \value ClosingState The socket is being closed.
 */

/*!
    \enum QIrSocket::SocketError
    \brief Error that last occurred on the infrared socket.

    \value NoError No error has occurred.
    \value AccessError The client has inadequate permissions to access the socket.
    \value ResourceError The kernel has run out of sockets.
    \value BusyError Another connect is in progress or device is busy.
    \value HostUnreachableError The host is unreachable.  This can be caused by an invalid destination address or remote device not being present.
    \value ServiceUnavailableError The host was found, but did not provide
    the target service.
    \value ConnectionRefused The remote host has refused a connection.
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
    Constructs a new QIrSocket with \a parent. The socket is not
    connected.  The \a parent specifies the QObject parent.
 */
QIrSocket::QIrSocket(QObject *parent)
    : QIODevice(parent)
{
    setOpenMode(QIODevice::NotOpen);

    m_data = new QIrSocketPrivate(true);
    m_data->m_parent = this;
}

/*!
    Deconstructs a QIrSocket.
    If the socket is in any state other than UnconnectedState, the current
    connection is aborted.
 */
QIrSocket::~QIrSocket()
{
    if (m_data->m_state != QIrSocket::UnconnectedState)
        abort();

    if (m_data)
        delete m_data;
}

/*!
    Returns the socket descriptor for the socket if the socket is currently
    active (e.g. not in UnconnectedState).  Otherwise returns -1.

    \sa state()
 */
int QIrSocket::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    Initializes the QIrSocket with the native descriptor
    \a socketDescriptor.  The socket is put into the \a state and
    opened in \a openMode.

    Returns true on successful completion of the request; otherwise returns false.

    \sa socketDescriptor()
 */
bool QIrSocket::setSocketDescriptor(int socketDescriptor,
        QIrSocket::SocketState state,
        QIODevice::OpenMode openMode)
{
    m_data->resetNotifiers();
    m_data->m_fd = socketDescriptor;

    bool ret = m_data->readSocketParameters(socketDescriptor);

    if (!ret) {
        m_data->m_fd = -1;
        return false;
    }

    m_data->m_engine->setSocketOption(socketDescriptor,
                                      QIrSocketEngine::NonBlockingOption);

    m_data->setupNotifiers();
    setOpenMode(openMode);

    if (m_data->m_state != state) {
        m_data->m_state = state;
        emit stateChanged(m_data->m_state);
    }

    return true;
}

/*!
    Closes the socket. This is the same as disconnect()

    \sa abort(), disconnect()
 */
void QIrSocket::close()
{
    QIODevice::close();

    if (m_data->m_state != QIrSocket::UnconnectedState) {
        m_data->m_readBuffer.clear();
        m_data->m_writeBuffer.clear();
        m_data->initiateDisconnect();
    }
}

/*!
    Returns the last error that has occurred.

    \sa state()
 */
QIrSocket::SocketError QIrSocket::error() const
{
    return m_data->m_error;
}

/*!
    Returns the state of the socket.

    \sa error()
 */
QIrSocket::SocketState QIrSocket::state() const
{
    return m_data->m_state;
}

/*!
    Returns the number of bytes that are waiting to be read.

    \sa bytesToWrite(), read()
 */
qint64 QIrSocket::bytesAvailable() const
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
qint64 QIrSocket::bytesToWrite() const
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
qint64 QIrSocket::readBufferSize() const
{
    return m_data->m_readBufferCapacity;
}

/*!
    Sets the capacity of QIrSocket's internal read buffer to be
    \a size bytes.  If \a size is 0 the buffer has unlimited capacity.  This is
    the default.

    \sa readBufferSize()
 */
void QIrSocket::setReadBufferSize(qint64 size)
{
    Q_ASSERT(size > 0);
    m_data->m_readBufferCapacity = size;
}

/*!
    Returns true if a line of data can be read from the socket;
    otherwise returns false.

    \sa readLine()
 */
bool QIrSocket::canReadLine() const
{
    bool hasLine = m_data->m_readBuffer.canReadLine();

    return hasLine || QIODevice::canReadLine();
}

/*!
    \reimp
 */
bool QIrSocket::isSequential() const
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
bool QIrSocket::waitForReadyRead(int msecs)
{
    if (m_data->m_state == QIrSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QIrSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while ((msecs < 0) || (stopWatch.elapsed() < msecs)) {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        QIrSocketEngine::SelectTypes types =
                QIrSocketEngine::SelectRead;
        if (!m_data->m_writeBuffer.isEmpty())
            types = types | QIrSocketEngine::SelectWrite;

        int timeout = timeout_value(msecs, stopWatch.elapsed());
        if (!m_data->m_engine->waitFor(types, m_data->m_fd, timeout,
            &timedOut, &readyToRead, &readyToWrite)) {
            m_data->setError(m_data->m_engine->error());
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
bool QIrSocket::waitForBytesWritten(int msecs)
{
    if (m_data->m_state == QIrSocket::UnconnectedState)
        return false;

    if (m_data->m_writeBuffer.isEmpty())
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QIrSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while ((msecs < 0) || (stopWatch.elapsed() < msecs)) {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        QIrSocketEngine::SelectTypes types =
                QIrSocketEngine::SelectTypes(QIrSocketEngine::SelectRead |
                                             QIrSocketEngine::SelectWrite);

        int timeout = timeout_value(msecs, stopWatch.elapsed());
        if (!m_data->m_engine->waitFor(types, m_data->m_fd, timeout,
            &timedOut, &readyToRead, &readyToWrite)) {
            m_data->setError(m_data->m_engine->error());
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
qint64 QIrSocket::readData(char *data, qint64 maxsize)
{
    if (!m_data->m_isBuffered) {
        qint64 readBytes = m_data->m_engine->readFromSocket(m_data->m_fd, data, maxsize);

        if (readBytes == -1) {
            m_data->setError(m_data->m_engine->error());
            emit error(m_data->m_error);
        }
        else if (!m_data->m_readNotifier->isEnabled())
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

        if (m_data->m_state == QIrSocket::UnconnectedState &&
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

    if (m_data->m_state == QIrSocket::UnconnectedState &&
        m_data->m_readBuffer.isEmpty()) {
        QIODevice::close();
    }

    return readSoFar;
}

/*!
    \reimp
 */
qint64 QIrSocket::readLineData(char *data, qint64 maxsize)
{
    return QIODevice::readLineData(data, maxsize);
}

/*!
    \reimp
 */
qint64 QIrSocket::writeData(const char *data, qint64 size)
{
    if (m_data->m_state == QIrSocket::UnconnectedState) {
        m_data->setError(QIrSocket::UnknownError);
        return -1;
    }

    if (!m_data->m_isBuffered) {
        qint64 written = m_data->m_engine->writeToSocket(m_data->m_fd, data, size);

        if (written >= 0)
            emit bytesWritten(written);
        else {
            m_data->setError(m_data->m_engine->error());
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
bool QIrSocket::waitForConnected(int msecs)
{
    if (m_data->m_state == QIrSocket::UnconnectedState)
        return false;

    if (m_data->m_state == QIrSocket::ConnectedState)
        return true;

    bool timedOut = false;
    QIrSocketEngine::SelectTypes types =
            QIrSocketEngine::SelectWrite;

    if (!m_data->m_engine->waitFor(types, m_data->m_fd, msecs, &timedOut)) {
        m_data->setError(m_data->m_engine->error());
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
bool QIrSocket::waitForDisconnected(int msecs)
{
    if (m_data->m_state == QIrSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QIrSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while ((msecs < 0) || (stopWatch.elapsed() < msecs)) {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        QIrSocketEngine::SelectTypes types =
                QIrSocketEngine::SelectRead;

        if (!m_data->m_writeBuffer.isEmpty())
            types = types | QIrSocketEngine::SelectWrite;

        int timeout = timeout_value(msecs, stopWatch.elapsed());
        if (!m_data->m_engine->waitFor(types, m_data->m_fd, timeout,
            &timedOut, &readyToRead, &readyToWrite)) {
            m_data->setError(m_data->m_engine->error());
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
void QIrSocket::abort()
{
    if (m_data->m_state == QIrSocket::UnconnectedState)
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
bool QIrSocket::atEnd() const
{
    return QIODevice::atEnd() && (!isOpen() || m_data->m_readBuffer.isEmpty());
}

/*!
    This function writes as much as possible from the internal write buffer to
    the underlying network socket, without blocking. If any data was written,
    this function returns true; otherwise false is returned.

    \sa write(), waitForBytesWritten()
 */
bool QIrSocket::flush()
{
    return m_data->flush();
}

/*!
    Attempts to open an Infrared connection between the local device
    and the remote device with address \a remote.  The underlying
    channel will be selected based on \a service parameter.  The
    service parameter will be looked up in the remote device's IAS
    Database.  This function should generally return immediately,
    and the socket will enter into the \c ConnectingState.

    The function returns true if the connection process could be started,
    and false otherwise.

    Note that the connection could still fail, the state of the socket
    will be sent in the stateChanged() signal.

    \bold{NOTE:} Under LINUX, you can pass 0 for the \c remote parameter.
    In this case, the kernel will select the first available device
    which contains an IAS entry that matches the \c service parameter.

    \sa state(), connected(), waitForConnected()
*/
bool QIrSocket::connect(const QByteArray &service, quint32 remote)
{
    if (state() != QIrSocket::UnconnectedState)
        return false;

    m_data->resetSocketParameters();

    int sockfd = m_data->m_engine->socket();

    if (sockfd < 0) {
        m_data->setError(m_data->m_engine->error());
        return false;
    }

    return m_data->initiateConnect(sockfd, service, remote);
}


/*!
    Attempts to close the socket.  If there is pending data waiting to be
    written, the socket will enter ClosingState and wait until all data has
    been written.  Eventually it will enter UnconnectedState and emit
    the disconnected() signal.

    Returns true on successful completion of the request; otherwise returns false.

    \sa close()
 */
bool QIrSocket::disconnect()
{
    if (m_data->m_state == QIrSocket::UnconnectedState) {
        return false;
    }

    return m_data->initiateDisconnect();
}

/*!
    Returns the address of the remote device this socket is connected
    to.  If the socket is not connected, this returns 0.

    \sa connect()
*/
quint32 QIrSocket::remoteAddress() const
{
    return m_data->m_remote;
}

/*!
    \fn void QIrSocket::connected()

    This signal is emitted once the connect() has been called
    and the socket has been successfully connected.

    \sa connect(), disconnected()
 */

/*!
    \fn void QIrSocket::disconnected()

    This signal is emitted when the socket has been disconnected.

    \sa connect(), disconnect(), abort()
 */

/*!
    \fn void QIrSocket::error(QIrSocket::SocketError socketError)

    This signal is emitted after an error occurred. The \a socketError parameter
    describes the type of error that has occurred.

    \sa error(), errorString()
 */

/*!
    \fn void QIrSocket::stateChanged(QIrSocket::SocketState socketState)

    This signal is emitted when the state of the socket has changed.  The
    \a socketState parameter holds the new state.

    \sa state()
 */

#include "qirsocket.moc"
