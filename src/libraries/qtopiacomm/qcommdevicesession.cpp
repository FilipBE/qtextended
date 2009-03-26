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

#include "qcommdevicesession.h"

#include <qvaluespace.h>

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QEventLoop>

#include <private/qunixsocket_p.h>

class QCommDeviceSession_Private : public QObject
{
    Q_OBJECT
public:
    QCommDeviceSession_Private(const QByteArray &devId, QCommDeviceSession *parent);
    ~QCommDeviceSession_Private();

    void startSession();
    void endSession();

    QValueSpaceItem *m_valueSpace;
    QByteArray m_devId;
    QByteArray m_path;

    QCommDeviceSession *m_parent;
    QUnixSocket *m_socket;

    bool m_isOpen;
    bool m_inRequest;

    static QCommDeviceSession * session(const QByteArray &devId,
                                        QCommDeviceSession::WaitType type,
                                        QObject *parent);

private slots:
    void readyRead();
};

QCommDeviceSession_Private::QCommDeviceSession_Private(const QByteArray &devId,
        QCommDeviceSession *parent) : QObject(parent), m_socket(0),
                                      m_isOpen(false), m_inRequest(false)
{
    m_parent = parent;
    m_devId = devId;

    QByteArray p("/Hardware/Devices/");
    p.append(m_devId);

    m_valueSpace = new QValueSpaceItem(p);

    QVariant path = m_valueSpace->value("Path");
    m_path = path.toByteArray();
}

QCommDeviceSession_Private::~QCommDeviceSession_Private()
{
    if (m_socket) {
        m_socket->close();
        delete m_socket;
    }

    if (m_valueSpace)
        delete m_valueSpace;
}

void QCommDeviceSession_Private::startSession()
{
    if (m_isOpen)
        return;

    if (m_inRequest)
        return;

    if (!m_socket) {
        m_socket = new QUnixSocket();

        if (!m_socket->connect(m_path)) {
            delete m_socket;
            m_socket = 0;
            emit m_parent->sessionFailed();
            return;
        }

        connect(m_socket, SIGNAL(readyRead()),
                this, SLOT(readyRead()));
    }

    m_socket->write("SESSION_OPEN\r\n");
    m_inRequest = true;
}

void QCommDeviceSession_Private::endSession()
{
    if (!m_socket)
        return;

    if (m_inRequest)
        return;

    m_socket->write("SESSION_CLOSE\r\n");

    m_isOpen = false;
    emit m_parent->sessionClosed();

    // ensure new socket is created for next session, or else won't
    // work after device is suspended
    delete m_socket;
    m_socket = 0;
}

QCommDeviceSession * QCommDeviceSession_Private::session(const QByteArray &devId,
                                                         QCommDeviceSession::WaitType type,
                                                         QObject *parent)
{
    QCommDeviceSession *session = new QCommDeviceSession(devId, parent);

    if (type == QCommDeviceSession::Block) {
        session->startSession();

        if (!session->m_data->m_socket) {
            delete session;
            return NULL;
        }

        session->m_data->m_socket->waitForReadyRead(-1);
        session->m_data->readyRead();
    }
    else if (type == QCommDeviceSession::BlockWithEventLoop) {
        QEventLoop *evLoop = new QEventLoop(session);
        QObject::connect(session, SIGNAL(sessionOpen()),
                         evLoop, SLOT(quit()));
        QObject::connect(session, SIGNAL(sessionFailed()),
                         evLoop, SLOT(quit()));
        session->startSession();
        evLoop->exec();
        delete evLoop;
    }

    // Session failed
    if (!session->m_data->m_isOpen) {
        delete session;
        return NULL;
    }

    return session;
}

void QCommDeviceSession_Private::readyRead()
{
    if (!m_socket->canReadLine())
        return;

    QByteArray line = m_socket->readLine();

    if (line == "SESSION_FAILED\r\n") {
        m_inRequest = false;
        emit m_parent->sessionFailed();
    }
    else if (line == "SESSION_STARTED\r\n") {
        m_isOpen = true;
        m_inRequest = false;
        emit m_parent->sessionOpen();
    }
    else if (line == "SESSION_CLOSED\r\n") {
        // The server will only send this to us on a forced close
        m_isOpen = false;
        emit m_parent->sessionClosed();

        // ensure new socket is created for next session, or else won't
        // work after device is suspended
        delete m_socket;
        m_socket = 0;
    }
}

/*!
    \class QCommDeviceSession
    \inpublicgroup QtBaseModule

    \brief The QCommDeviceSession class provides facilities to initiate a device session.

    The QCommDeviceSession class provides facilities to initiate a new session on a hardware
    device.  The system will attempt to keep the device open for the duration of the open
    session, unless the user explicitly shuts it down or another unexpected event occurs.

    Sessions are thus used by applications to give a hint to the device manager that the
    device is currently in use.  E.g. a client that is trying to send a vCard over bluetooth
    to a remote device would wrap all Bluetooth related operations by using the
    QCommDeviceSession object with a given Bluetooth device.

    \sa QCommDeviceController

    \ingroup hardware
    \ingroup telephony
 */

/*!
    \enum QCommDeviceSession::WaitType

    \value Block Block, not using the event loop.
    \value BlockWithEventLoop Block, but using the event loop.
*/

/*!
    Constructs a new QCommDeviceSession object.  The he \a deviceId specifies the device id.
    This is usually equivalent to the hardware device id of the device.  E.g. irdaX for
    Infrared devices and hciX for Bluetooth devices.

    The \a parent parameter is passed to the QObject constructor.

    \sa deviceId()
*/
QCommDeviceSession::QCommDeviceSession(const QByteArray &deviceId, QObject *parent)
    : QObject(parent)
{
    m_data = new QCommDeviceSession_Private(deviceId, this);
}

/*!
    Destructor.
*/
QCommDeviceSession::~QCommDeviceSession()
{
    if (m_data) {
        delete m_data;
        m_data = NULL;
    }
}

/*!
    Attempts to initiate a session.  Once the session is established,
    the sessionOpen() signal will be sent.  Otherwise sessionFailed()
    signal will be sent.

    \sa sessionOpen(), sessionFailed(), endSession()
*/
void QCommDeviceSession::startSession()
{
    m_data->startSession();
}

/*!
    Closes the current session.

    \sa startSession(), sessionClosed()
*/
void QCommDeviceSession::endSession()
{
    m_data->endSession();
}

/*!
    Returns the id of the device.
 */
const QByteArray &QCommDeviceSession::deviceId() const
{
    return m_data->m_devId;
}

/*!
    Returns a new session for device given by \a deviceId.  The type of wait
    to use (blocking or recursive event loop) is specified by \a type.  The
    \a parent is used to specify the QObject parent of the created session.

    If the session could not be established, a NULL is returned.
*/
QCommDeviceSession * QCommDeviceSession::session(const QByteArray &deviceId,
                                         WaitType type, QObject *parent)
{
    return QCommDeviceSession_Private::session(deviceId, type, parent);
}

/*!
    \fn void QCommDeviceSession::sessionOpen()

    This signal is emitted whenever a session has been opened successfully and it is
    safe to use the device.

    \sa startSession()
*/

/*!
    \fn void QCommDeviceSession::sessionFailed();

    This signal is emitted when there was a problem opening the device session.

    \sa startSession()
*/

/*!
    \fn void QCommDeviceSession::sessionClosed()

    This signal is emitted whenever the session has been terminated (perhaps forcefully)

    \sa endSession()
*/

#include "qcommdevicesession.moc"
