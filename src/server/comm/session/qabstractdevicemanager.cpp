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

#include "qabstractdevicemanager.h"

#include <qvaluespace.h>
#include <qglobal.h>
#include <QPhoneProfileManager>

#include <private/qunixsocketserver_p.h>
#include <private/qunixsocket_p.h>
#include <QSet>
#include <QMap>
#include <QTimer>
#include <QQueue>
#include <QFile>
#include <QDebug>

#include <unistd.h>

class DeviceConnection : public QObject
{
    Q_OBJECT
public:
    DeviceConnection(int sockfd, QAbstractCommDeviceManager_Private *data);
    ~DeviceConnection();

    QUnixSocket *sock() const;

public slots:
    void stateChanged();
    void readyRead();
    void sessionCallback(bool error);
    void closeSession();

private:
    QUnixSocket *m_sock;
    QAbstractCommDeviceManager_Private *m_data;
    int m_id;
};

class CommManagerEvent
{
public:
    CommManagerEvent();

    enum Type {
        SESSION_PRE_OPEN,
        SESSION_OPEN,
        SESSION_CLOSE,
        DEVICE_UP,
        DEVICE_DOWN,
        DEVICE_UP_TIMED,
        DEVICE_UP_ONE_ITEM,
        DEVICE_UP_SESSION  // Special BringUp event for session support
    };

    int data; // Either the id or the number of seconds
    Type type;
};

CommManagerEvent::CommManagerEvent()
    : data(0),
      type(SESSION_PRE_OPEN)
{}

// TODO: Later on this can be done using DBus
class QAbstractCommDeviceManager_Private : public QObject
{
    Q_OBJECT

public:
    enum State { Off, OnMinutes, OnOneItem, On };

    QAbstractCommDeviceManager_Private(QAbstractCommDeviceManager *parent,
                                        const QByteArray &path,
                                        const QByteArray &devId);
    ~QAbstractCommDeviceManager_Private();

    void bringUp();
    void bringUpTimed(int secs);
    void bringUpOneItem();
    void bringDown(int id);

    bool sessionOpen(int id);
    void openSession(int id);
    void closeSession(int id);

    void addConnection(int id, DeviceConnection *conn);
    bool removeConnection(int id);

    bool sessionsActive() const;

    bool start();
    bool isStarted();
    void stop();

    QByteArray m_path;
    QByteArray m_devId;

public slots:
    void timeout();
    void upStatus(bool error, const QString &msg);
    void downStatus(bool error, const QString &msg);
    void doPending();
    void setState(State state);
    void updateState();

private:
    QValueSpaceObject *m_valueSpace;
    QUnixSocketServer *m_server;

    QMap<int,DeviceConnection*> m_conns;  // Maps sockfds to DeviceConnection
    QSet<int> m_sessions;  // The set of DeviceConnections which have a session

    State m_state;
    QAbstractCommDeviceManager *m_parent;
    QTimer m_timer;

    QQueue<CommManagerEvent> m_events;

    void doBringUp();
    void doBringDown();
    void doBringUpTimed(int secs);
    void doBringUpOneItem();
    void doOpenSession(int id);
    void doPreOpenSession(int id);
    void doCloseSession(int id);

    void doUp();
    void doDown();

    void closeAllSessions();
};

DeviceConnection::DeviceConnection(int sockfd, QAbstractCommDeviceManager_Private *data) : m_sock(0)
{
    m_data = data;
    m_id = sockfd;
    m_sock = new QUnixSocket(this);
    m_sock->setSocketDescriptor(sockfd);

    connect(m_sock, SIGNAL(stateChanged(SocketState)),
            this, SLOT(stateChanged()));
    connect(m_sock, SIGNAL(readyRead()),
            this, SLOT(readyRead()));
}

DeviceConnection::~DeviceConnection()
{
    delete m_sock;
}

void DeviceConnection::stateChanged()
{
    QUnixSocket::SocketState state = m_sock->state();

    if ((state == QUnixSocket::UnconnectedState) ||
        (state == QUnixSocket::ClosingState)) {
        // If the session is open for our connection, close the session
        if (m_data->sessionOpen(m_id)) {
            m_data->closeSession(m_id);
        }

        m_data->removeConnection(m_id);
        deleteLater();
    }
}

void DeviceConnection::readyRead()
{
    while (m_sock->canReadLine()) {
        QByteArray line = m_sock->readLine();

        if (line.isEmpty())
            return;

        if (line == "SESSION_OPEN\r\n") {
            m_data->openSession(m_id);
        } else if (line == "SESSION_CLOSE\r\n") {
            if (m_data->sessionOpen(m_id)) {
                m_data->closeSession(m_id);
            }
        } else if (line == "DOWN\r\n") {
            m_data->bringDown(m_id);
        } else if (line == "UP_ONE_ITEM\r\n") {
            m_data->bringUpOneItem();
        } else if (line == "UP\r\n") {
            m_data->bringUp();
        } else if (line.startsWith("UP_TIMED")) {
            line.truncate(line.length() - 2);
            line = line.mid(9);
            int secs = line.toInt();
            if (secs > 0) {
                if (secs > 3600)
                    secs = 3600;

                m_data->bringUpTimed(secs);
            }
        }
    }
}

void DeviceConnection::sessionCallback(bool error)
{
    if (error) {
        m_sock->write("SESSION_FAILED\r\n");
    }
    else {
        m_sock->write("SESSION_STARTED\r\n");
    }
}

void DeviceConnection::closeSession()
{
    m_sock->write("SESSION_CLOSED\r\n");
}

QUnixSocket *DeviceConnection::sock() const
{
    return m_sock;
}

class DeviceManagerSocket : public QUnixSocketServer
{
public:
    DeviceManagerSocket(QAbstractCommDeviceManager_Private *data);
    virtual void incomingConnection(int socketDescriptor);

    QAbstractCommDeviceManager_Private *m_data;
};

DeviceManagerSocket::DeviceManagerSocket(QAbstractCommDeviceManager_Private *data) :
         QUnixSocketServer(data)
{
    m_data = data;
}

void DeviceManagerSocket::incomingConnection(int socketDescriptor)
{
    if (!m_data) {
        ::close(socketDescriptor);
        return;
    }

    // Create a connection and put on the list
    DeviceConnection *conn = new DeviceConnection(socketDescriptor, m_data);
    m_data->addConnection(socketDescriptor, conn);
}

QAbstractCommDeviceManager_Private::QAbstractCommDeviceManager_Private(QAbstractCommDeviceManager *parent,
        const QByteArray &path,
        const QByteArray &devId)
{
    m_path = path;
    m_devId = devId;
    m_server = new DeviceManagerSocket(this);
    m_parent = parent;
    m_valueSpace = 0;

    QFile f(path);
    f.open(QIODevice::WriteOnly|QIODevice::Truncate);
    f.close();

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(m_parent, SIGNAL(upStatus(bool,QString)),
            this, SLOT(upStatus(bool,QString)), Qt::QueuedConnection);
    connect(m_parent, SIGNAL(downStatus(bool,QString)),
            this, SLOT(downStatus(bool,QString)), Qt::QueuedConnection);
}

QAbstractCommDeviceManager_Private::~QAbstractCommDeviceManager_Private()
{
    stop();

    if (m_server)
        delete m_server;

    if (m_valueSpace)
        delete m_valueSpace;
}

bool QAbstractCommDeviceManager_Private::sessionsActive() const
{
    return m_sessions.size();
}

// Called once at startup
void QAbstractCommDeviceManager_Private::updateState()
{
    if (m_parent->isUp()) {
        setState(On);
    }
    else {
        setState(Off);
    }
}

/*!
    Notifies the manager that the device should be brought up.
 */
void QAbstractCommDeviceManager_Private::bringUp()
{
    CommManagerEvent ev;
    ev.type = CommManagerEvent::DEVICE_UP;

    m_events.enqueue(ev);

    if (m_events.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doBringUp()
{
    // Take care of the timer
    if (m_timer.isActive())
        m_timer.stop();

    doUp();
}

/*!
    Notifies the manager that the device should be brought up
    for a specified amount of time.  After this time the device will be automatically
    powered off by the manager.  The timeout is given by \c secs in seconds.

    If there are any opened sessions when the time is up, the service will wait for
    all sessions to terminate before shutting down the infrared device.
 */
void QAbstractCommDeviceManager_Private::bringUpTimed(int secs)
{
    CommManagerEvent ev;
    ev.type = CommManagerEvent::DEVICE_UP_TIMED;
    ev.data = secs;

    m_events.enqueue(ev);

    if (m_events.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doBringUpTimed(int secs)
{
    // Take care of the timer
    if (m_timer.isActive())
        m_timer.stop();
    m_timer.start(secs * 1000);

    doUp();
}

/*!
    Notifies the manager that the device should be brought up
    for just one session (outgoing or incoming).  After the connection
    has been closed, the device will be automatically powered off.

    If there are any opened sessions when the time is up, the service will wait for
    all sessions to terminate before shutting down the infrared device.
 */
void QAbstractCommDeviceManager_Private::bringUpOneItem()
{
    CommManagerEvent ev;
    ev.type = CommManagerEvent::DEVICE_UP_ONE_ITEM;

    m_events.enqueue(ev);

    if (m_events.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doBringUpOneItem()
{
    // Take care of the timer
    if (m_timer.isActive())
        m_timer.stop();

    doUp();
}

/*!
    Notifies the manager that the device should be brought down.
    Note that the manager will wait for any currently active sessions before
    shutting down the device.
 */
void QAbstractCommDeviceManager_Private::bringDown(int id)
{
    CommManagerEvent ev;
    ev.type = CommManagerEvent::DEVICE_DOWN;
    ev.data = id;

    m_events.enqueue(ev);

    if (m_events.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doBringDown()
{
    DeviceConnection *conn = 0;
    if ( !m_events.isEmpty() ) {
        CommManagerEvent ev = m_events.head();
        if (ev.data != 0) {
            conn = m_conns[ev.data];
        }
    }

    // Check that we should bring down the device
    if (conn && !m_parent->shouldBringDown(conn->sock())) {
        // Process next event
        if ( m_events.isEmpty() ) {
            qWarning("QAbstractCommDeviceManager::The queue should not be empty!");
            return;
        }
        m_events.dequeue();
        QTimer::singleShot(0, this, SLOT(doPending()));
        return;
    }

    // Take care of the timer
    if (m_timer.isActive())
        m_timer.stop();

    doDown();
}

void QAbstractCommDeviceManager_Private::doUp()
{
    if (m_parent->isUp()) {
        if ( m_events.isEmpty() ) {
            qWarning("QAbstractCommDeviceManager::doUp - The queue should not be empty!");
            return;
        }

        CommManagerEvent ev = m_events.dequeue();

        if (ev.type == CommManagerEvent::DEVICE_UP_TIMED)
            setState(OnMinutes);
        else if ((ev.type == CommManagerEvent::DEVICE_UP_ONE_ITEM) ||
                 (ev.type == CommManagerEvent::DEVICE_UP_SESSION))
            setState(OnOneItem);
        else
            setState(On);

        QTimer::singleShot(0, this, SLOT(doPending()));
        return;
    }

    m_parent->bringUp();
}

void QAbstractCommDeviceManager_Private::doDown()
{
    if (!m_parent->isUp()) {
        if ( m_events.isEmpty() ) {
            qWarning("QAbstractCommDeviceManager::doDown - The queue should not be empty!");
            return;
        }
        m_events.dequeue();
        QTimer::singleShot(0, this, SLOT(doPending()));
        return;
    }

    m_parent->bringDown();
}

bool QAbstractCommDeviceManager_Private::sessionOpen(int id)
{
    return m_sessions.contains(id);
}

/*!
    Notifies the manager that a new connection has been opened
    on the device.  The manager will take necessary action as to
    ensure that the device will not be shut down for the duration
    of the session.  The \c id parameter is a unique session Id.
 */
void QAbstractCommDeviceManager_Private::openSession(int id)
{
    if (!m_conns.contains(id))
        return;

    CommManagerEvent ev;
    ev.type = CommManagerEvent::SESSION_PRE_OPEN;
    ev.data = id;

    m_events.enqueue(ev);

    if (m_events.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doPreOpenSession(int id)
{
    if ( m_events.isEmpty() ) {
        qWarning("QAbstractCommDeviceManager::doPreOpenSession - The queue should not be empty!");
        return;
    }
    CommManagerEvent ev = m_events.dequeue();

    // Do we have a connection?
    if (!m_conns.contains(ev.data)) {
        QTimer::singleShot(0, this, SLOT(doPending()));
        return;
    }

    DeviceConnection *conn = m_conns[ev.data];

    // Check that we can create a session
    if (!m_parent->shouldStartSession(conn->sock())) {
        qWarning("Failed to start device session because plane mode is active");
        conn->sessionCallback(true);
        QTimer::singleShot(0, this, SLOT(doPending()));
        return;
    }

    CommManagerEvent sessOpenEv;
    sessOpenEv.type = CommManagerEvent::SESSION_OPEN;
    sessOpenEv.data = id;
    m_events.push_front(sessOpenEv);

    if (!m_parent->isUp()) {
        // Create a fake Bring Up event
        CommManagerEvent devUpEv;
        devUpEv.type = CommManagerEvent::DEVICE_UP_SESSION;
        m_events.push_front(devUpEv);
    }

    QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doOpenSession(int id)
{
    if ( m_events.isEmpty() ) {
        qWarning("QAbstractCommDeviceManager::doOpenSession - The queue should not be empty!");
        return;
    }

    m_events.dequeue();

    if (!m_conns.contains(id)) {
        return;
    }

    m_sessions.insert(id);

    m_valueSpace->setAttribute("ActiveSessions", QVariant(true));

    //Now need to notify the DeviceConnection
    DeviceConnection *conn = m_conns[id];
    conn->sessionCallback(false);

    QTimer::singleShot(0, this, SLOT(doPending()));
}

/*!
    Notifies the manager that a session uniquely identified by \c id
    has been ended.
 */
void QAbstractCommDeviceManager_Private::closeSession(int id)
{
    CommManagerEvent ev;
    ev.type = CommManagerEvent::SESSION_CLOSE;
    ev.data = id;

    m_events.enqueue(ev);

    if (m_events.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::doCloseSession(int id)
{
    if ( m_events.isEmpty() ) {
        qWarning("QAbstractCommDeviceManager::doCloseSession - The queue should not be empty!");
        return;
    }

    m_events.dequeue();

    if (!m_sessions.remove(id)) {
        return;
    }

    QTimer::singleShot(0, this, SLOT(doPending()));

    // There are still sessions open, don't do anything
    if (m_sessions.size())
        return;

    m_valueSpace->setAttribute("ActiveSessions", QVariant(false));

    switch( m_state ) {
        case On:
            break;
        case Off:
        case OnOneItem:
            bringDown(0);
            break;
        case OnMinutes:
            if ( !m_timer.isActive()) {
                bringDown(0);
            }
            break;
    }
}

void QAbstractCommDeviceManager_Private::timeout()
{
    m_timer.stop();

    if (m_state != OnMinutes)
        return;

    // If there are any pending events, don't turn off the device here
    if (m_events.size())
        return;

    // If there are sessions open, just set the state to OnOneItem
    if (m_sessions.size()) {
        setState(OnOneItem);
        return;
    }

    bringDown(0);
}

void QAbstractCommDeviceManager_Private::doPending()
{
    if (m_events.size() == 0)
        return;

    CommManagerEvent ev = m_events.head();

    switch (m_events.head().type) {
        case CommManagerEvent::SESSION_PRE_OPEN:
        {
            doPreOpenSession(ev.data);
            break;
        }

        case CommManagerEvent::SESSION_OPEN:
        {
            doOpenSession(ev.data);
            break;
        }

        case CommManagerEvent::SESSION_CLOSE:
        {
            doCloseSession(ev.data);
            break;
        }

        case CommManagerEvent::DEVICE_UP:
        {
            doBringUp();
            break;
        }

        case CommManagerEvent::DEVICE_DOWN:
        {
            doBringDown();
            break;
        }

        case CommManagerEvent::DEVICE_UP_TIMED:
        {
            doBringUpTimed(ev.data);
            break;
        }

        case CommManagerEvent::DEVICE_UP_ONE_ITEM:
        {
            doBringUpOneItem();
            break;
        }

        case CommManagerEvent::DEVICE_UP_SESSION:
        {
            doUp();
            break;
        }

        default:
        {
            qWarning("Unknown event!");
            QTimer::singleShot(0, this, SLOT(doPending()));
            break;
        }
    }
}

void QAbstractCommDeviceManager_Private::upStatus(bool error, const QString &msg)
{
    // Handle spurious events here
    if ( m_events.isEmpty() ) {
        setState(On);
        return;
    }

    CommManagerEvent ev = m_events.head();

    if ((ev.type != CommManagerEvent::DEVICE_UP) &&
         (ev.type != CommManagerEvent::DEVICE_UP_TIMED) &&
         (ev.type != CommManagerEvent::DEVICE_UP_ONE_ITEM) &&
         (ev.type != CommManagerEvent::DEVICE_UP_SESSION)) {
        setState(On);
        return;
    }

    ev = m_events.dequeue();

    if (error) {
        qWarning("Failed to bring up device %s with error: %s\n",
                 m_devId.constData(),
                 msg.toLatin1().constData());

        if (ev.type == CommManagerEvent::DEVICE_UP_SESSION) {
            // next event should be a session event
            // since we could not bring up the device
            // we need to notify failure here
            if ( m_events.isEmpty() )
                return;
            CommManagerEvent ev = m_events.dequeue();
            DeviceConnection *conn = m_conns[ev.data];
            conn->sessionCallback(true);
        }
    }
    else {
        if (ev.type == CommManagerEvent::DEVICE_UP_TIMED)
            setState(OnMinutes);
        else if ((ev.type == CommManagerEvent::DEVICE_UP_ONE_ITEM) ||
                 (ev.type == CommManagerEvent::DEVICE_UP_SESSION))
            setState(OnOneItem);
        else
            setState(On);
    }

    QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::downStatus(bool error, const QString &msg)
{
    // Handle spurious events here
    if ( m_events.isEmpty() ) {
        setState(Off);
        closeAllSessions();
        return;
    }

    CommManagerEvent ev = m_events.head();

    if (ev.type != CommManagerEvent::DEVICE_DOWN) {
        setState(Off);
        closeAllSessions();
        return;
    }

    if (error) {
        qWarning("Failed to bring down device %s with error: %s\n",
                 m_devId.constData(),
                 msg.toLatin1().constData());
    }

    if ( !m_events.isEmpty() )
        m_events.dequeue();

    setState(Off);
    closeAllSessions();
    QTimer::singleShot(0, this, SLOT(doPending()));
}

void QAbstractCommDeviceManager_Private::setState(State state)
{
    m_state = state;

    switch (m_state) {
        case Off:
            m_valueSpace->setAttribute("Status", QVariant(QByteArray("Down")));
            m_valueSpace->setAttribute("PowerState", QVariant(QByteArray("Off")));
            break;
        case On:
            m_valueSpace->setAttribute("Status", QVariant(QByteArray("Up")));
            m_valueSpace->setAttribute("PowerState", QVariant(QByteArray("On")));
            break;
        case OnMinutes:
            m_valueSpace->setAttribute("Status", QVariant(QByteArray("Up")));
            m_valueSpace->setAttribute("PowerState", QVariant(QByteArray("OnMinutes")));
            break;
        case OnOneItem:
            m_valueSpace->setAttribute("Status", QVariant(QByteArray("Up")));
            m_valueSpace->setAttribute("PowerState", QVariant(QByteArray("OnOneItem")));
            break;
    }
}

void QAbstractCommDeviceManager_Private::addConnection(int id, DeviceConnection *conn)
{
    m_conns.insert(id, conn);
}

bool QAbstractCommDeviceManager_Private::removeConnection(int id)
{
    return m_conns.remove(id) == 1;
}

bool QAbstractCommDeviceManager_Private::start()
{
    QByteArray p("/Hardware/Devices/");
    p.append(m_devId);
    m_valueSpace = new QValueSpaceObject(p);
    m_valueSpace->setAttribute("Path", QVariant(m_path));
    m_valueSpace->setAttribute("ActiveSessions", QVariant(false));

    QTimer::singleShot(0, this, SLOT(updateState()));

    return m_server->listen(m_path);
}

bool QAbstractCommDeviceManager_Private::isStarted()
{
    return m_server->isListening();
}

void QAbstractCommDeviceManager_Private::stop()
{
    m_timer.stop();
    m_server->close();
    m_sessions.clear();

    foreach (DeviceConnection *conn, m_conns) {
        delete conn;
    }

    m_conns.clear();

    if (m_valueSpace) {
        delete m_valueSpace;
        m_valueSpace = 0;
    }
}

void QAbstractCommDeviceManager_Private::closeAllSessions()
{
    foreach (int id, m_sessions) {
        DeviceConnection *conn = m_conns[id];
        conn->closeSession();
    }

    // Look ahead and see if there are any session_close events, remove them
    // then clear out the sessions
    QQueue<CommManagerEvent>::iterator iter = m_events.begin();
    while (iter != m_events.end()) {
        if ((*iter).type == CommManagerEvent::SESSION_CLOSE)
            iter = m_events.erase(iter);
    }
    m_sessions.clear();

    m_valueSpace->setAttribute("ActiveSessions", QVariant(false));
}

/*!
    \class QAbstractCommDeviceManager
    \inpublicgroup QtInfraredModule
    \inpublicgroup QtBluetoothModule
    \ingroup QtopiaServer
    \brief The QAbstractCommDeviceManager class manages power state of a communications device.

    The QAbstractCommDeviceManager implements functionality necessary to provide device session
    functionality for a communications device.  It also controls the power and state of a
    communications device.

    QAbstractCommDeviceManager manages the device availability for all applications which are
    use the communications device.  The applications must use the QCommDeviceSession class
    to notify the manager that they are or will be using the communications device.
    If a session is activated and the device is not in a correct state (e.g. turned off),
    the device is brought up automatically.  Once all sessions are closed, the device
    can be brought down automatically if required to conserve power.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
    \sa QCommDeviceSession, QCommDeviceController
 */

/*!
    Constructs a QAbstractCommDeviceManager.  The \a path argument contains
    the UNIX socket path for the manager to listen on.  This should usually be
    in Qt Extended temp directory.  The \a devId contains the unique device identifier,
    e.g. irda0 or hci0.  The \a parent contains the QObject parent.

    \sa serverPath(), deviceId()
*/
QAbstractCommDeviceManager::QAbstractCommDeviceManager(const QByteArray &path,
        const QByteArray &devId, QObject *parent) : QObject(parent)
{
    m_data = new QAbstractCommDeviceManager_Private(this, path, devId);
}

/*!
    Destructor.
*/
QAbstractCommDeviceManager::~QAbstractCommDeviceManager()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns if able to start the manager; otherwise returns false.

    \sa isStarted(), stop()
*/
bool QAbstractCommDeviceManager::start()
{
    return m_data->start();
}

/*!
    Returns true if the manager is started, false otherwise.

    \sa start(), stop()
*/
bool QAbstractCommDeviceManager::isStarted() const
{
    return m_data->isStarted();
}

/*!
    Stops the manager.

    \sa isStarted(), start()
*/
void QAbstractCommDeviceManager::stop()
{
    m_data->stop();
}

/*!
    Returns the UNIX server socket path of the device.

    \sa deviceId()
*/
const QByteArray &QAbstractCommDeviceManager::serverPath() const
{
    return m_data->m_path;
}

/*!
    Returns the unique device id of the device this manager is responsible for.

    \sa serverPath()
*/
const QByteArray &QAbstractCommDeviceManager::deviceId() const
{
    return m_data->m_devId;
}

/*!
    This method is called whenever a new session request is processed
    by the device manager.  It can be used by clients to specialize
    the logic of whether a given peer has the ability to start a session
    utilizing the device.  The \a socket parameter holds the socket of
    the peer.

    The default implementation returns false if Qt Extended is in plane mode;
    otherwise, it returns true.

    \sa QPhoneProfileManager::planeMode()
*/
bool QAbstractCommDeviceManager::shouldStartSession(QUnixSocket *socket) const
{
    Q_UNUSED(socket);

    // don't allow session to start in flight mode
    QPhoneProfileManager mgr;
    if (mgr.planeMode())
        return false;

    return true;
}

/*!
    Returns true if there are applications using this device.
*/
bool QAbstractCommDeviceManager::sessionsActive() const
{
    return m_data->sessionsActive();
}

/*!
    This method is called whenever a bring down request is processed by the
    device manager.  It can be used by clients to specialize the logic
    of whether a device should be brought down.  E.g. there are several
    applications that are using the device (e.g. there are several
    sessions open), and bringing up the device might interfere with
    these sessions.  The default implementation returns true if there
    are no active sessions, and false otherwise.

    The \a socket parameter holds the socket of the peer who is requesting
    the device be brought down.

    NOTE: It is safe to start a new EventLoop inside this method.
*/
bool QAbstractCommDeviceManager::shouldBringDown(QUnixSocket *socket) const
{
    Q_UNUSED(socket);

    if (sessionsActive())
        return false;

    return true;
}

/*!
    \fn void QAbstractCommDeviceManager::bringUp()

    Clients should reimplement this method to bring up the device.  If the operation
    is successful, clients should emit the upStatus signal.  If an error occurred,
    the upStatus signal should be emitted with error set to true.

    \sa upStatus()
*/

/*!
    \fn void QAbstractCommDeviceManager::bringDown()

    Clients should reimplement this method to bring down the device.  If the operation
    is successful, clients should emit the downStatus signal.  If an error occurred,
    the downStatus signal should be emitted with error set to true.

    \sa downStatus()
*/

/*!
    \fn bool QAbstractCommDeviceManager::isUp() const

    Clients should reimplement this method to return true if the device is currently
    available, and false otherwise.

    \sa bringUp(), bringDown()
*/

/*!
    \fn void QAbstractCommDeviceManager::upStatus(bool error, const QString &msg)

    Clients should emit this signal whenever the bringUp operation is successful or has failed.
    If the operation succeeds, the \a error parameter should be set to false.  Otherwise
    the \a error paramter should be set to true and the \a msg paramter can contain an
    optional error message.

    \sa bringUp()
*/

/*!
    \fn void QAbstractCommDeviceManager::downStatus(bool error, const QString &msg)

    Clients should emit this signal whenever the bringDown operation is successful or has failed.
    If the operation succeeds, the \a error parameter should be set to false.  Otherwise
    the \a error paramter should be set to true and the \a msg paramter can contain an
    optional error message.

    \sa bringDown()
*/

#include "qabstractdevicemanager.moc"
