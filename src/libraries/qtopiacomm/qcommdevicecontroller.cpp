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

#include "qcommdevicecontroller.h"

#include <qvaluespace.h>

#include <QString>
#include <QVariant>
#include <QByteArray>

#include <private/qunixsocket_p.h>

class QCommDeviceController_Private : public QObject
{
    Q_OBJECT

public:
    QCommDeviceController_Private(const QByteArray &devId, QCommDeviceController *parent);
    ~QCommDeviceController_Private();

    bool isUp() const;
    void bringUp();
    void bringDown();
    void bringUpTimed(int secs);
    void bringUpOneItem();
    bool sessionsActive() const;

    QCommDeviceController::PowerState powerState() const;

    QUnixSocket *m_sock;
    QCommDeviceController *m_parent;

    QValueSpaceItem *m_upDown;
    QValueSpaceItem *m_state;
    QValueSpaceItem *m_sessionsActive;
    QByteArray m_devId;
    QByteArray m_path;

private slots:
    void statusChanged();
    void powerStateChanged();
};

QCommDeviceController_Private::QCommDeviceController_Private(const QByteArray &devId,
        QCommDeviceController *parent) : QObject(parent), m_sock(0)
{
    m_parent = parent;
    m_devId = devId;

    QByteArray p("/Hardware/Devices/");
    p.append(m_devId);

    // Find out the path to connect the UNIX socket to
    QValueSpaceItem *vPath = new QValueSpaceItem(p);
    QVariant path = vPath->value("Path");
    m_path = path.toByteArray();
    delete vPath;

    // ValueSpaceItem for up/down information
    QByteArray ud(p);
    ud.append("/Status");
    m_upDown = new QValueSpaceItem(ud);
    connect(m_upDown, SIGNAL(contentsChanged()),
            this, SLOT(statusChanged()));

    // ValueSpaceItem for Power State information
    QByteArray powerState(p);
    powerState.append("/PowerState");
    m_state = new QValueSpaceItem(powerState);
    connect(m_state, SIGNAL(contentsChanged()),
            this, SLOT(powerStateChanged()));

    // ValueSpaceItem for whether there are any active sessions
    QByteArray sessionsActive(p);
    sessionsActive.append("/ActiveSessions");
    m_sessionsActive = new QValueSpaceItem(sessionsActive);

    m_sock = new QUnixSocket();

    if (!m_sock->connect(m_path)) {
        delete m_sock;
        m_sock = 0;
        return;
    }
}

QCommDeviceController_Private::~QCommDeviceController_Private()
{
    if (m_state)
        delete m_state;

    if (m_upDown)
        delete m_upDown;

    if (m_sessionsActive)
        delete m_sessionsActive;

    if (m_sock) {
        m_sock->close();
        delete m_sock;
    }
}

void QCommDeviceController_Private::powerStateChanged()
{
    QCommDeviceController::PowerState state = powerState();

    emit m_parent->powerStateChanged(state);
}

QCommDeviceController::PowerState QCommDeviceController_Private::powerState() const
{
    QByteArray value = m_state->value().toByteArray();

    QCommDeviceController::PowerState state = QCommDeviceController::Off;

    if (value == "Off")
        state = QCommDeviceController::Off;
    else if (value == "On")
        state = QCommDeviceController::On;
    else if (value == "OnOneItem")
        state = QCommDeviceController::OnOneItem;
    else if (value == "OnMinutes")
        state = QCommDeviceController::OnTimed;

    return state;
}

void QCommDeviceController_Private::statusChanged()
{
    QByteArray value = m_upDown->value().toByteArray();

    if (value == "Down")
        emit m_parent->down();
    else if (value == "Up")
        emit m_parent->up();
}

bool QCommDeviceController_Private::isUp() const
{
    QByteArray value = m_upDown->value().toByteArray();

    if (value == "Down")
        return false;
    else if (value == "Up")
        return true;

    return false;
}

void QCommDeviceController_Private::bringUp()
{
    if (!m_sock)
        return;

    m_sock->write("UP\r\n");
}

void QCommDeviceController_Private::bringDown()
{
    if (!m_sock)
        return;

    m_sock->write("DOWN\r\n");
}

bool QCommDeviceController_Private::sessionsActive() const
{
    return m_sessionsActive->value().toBool();
}

void QCommDeviceController_Private::bringUpTimed(int secs)
{
    if (!m_sock)
        return;

    QByteArray msg("UP_TIMED ");
    msg.append(QByteArray::number(secs));
    msg.append("\r\n");

    m_sock->write(msg);
}

void QCommDeviceController_Private::bringUpOneItem()
{
    if (!m_sock)
        return;

    m_sock->write("UP_ONE_ITEM\r\n");
}

/*!
    \class QCommDeviceController
    \inpublicgroup QtBaseModule

    \brief The QCommDeviceController class provides facilities to control the power state of a hardware communications device

    Using the QCommDeviceController class it is possible to control the
    power state of a Bluetooth or Infrared hardware device.  This class is
    generally useful only to settings applications which need to control the
    device state directly.  For typical useage, please see the
    QCommDeviceSession class.

    The device can be in four power states: on, off, on for one item,
    and on for a period of time.

    \sa QCommDeviceSession

    \ingroup hardware
 */

/*!
    \enum QCommDeviceController::PowerState

    The current state of the device.

    \value On The device is on permanently.
    \value Off The device is off.
    \value OnOneItem The device is on until the last device session is closed.
    \value OnTimed The device is on for a specified period of time, at which time it is either turned off if there are no sessions open, or enters the OnOneItem state.
*/

/*!
    Constructs a new QCommDeviceController object.  The \a devId specifies the device id.
    This is usually equivalent to the hardware device id of the device.  E.g. irdaX for
    Infrared devices and hciX for Bluetooth devices.

    The \a parent parameter is passed to the QObject constructor.

    \sa deviceId()
*/
QCommDeviceController::QCommDeviceController(const QByteArray &devId, QObject *parent) :
        QObject(parent)
{
    m_data = new QCommDeviceController_Private(devId, this);
}

/*!
    Destructor.
*/
QCommDeviceController::~QCommDeviceController()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns the id of the device.
*/
const QByteArray &QCommDeviceController::deviceId() const
{
    return m_data->m_devId;
}

/*!
    Brings up the device.  The device will attempt to enter the \bold{On} mode.

    \sa bringDown(), bringUpTimed(), bringUpOneItem()
*/
void QCommDeviceController::bringUp()
{
    m_data->bringUp();
}

/*!
    Brings up the device in timed mode.  The device will attempt to enter the
    \bold{OnTimed} mode.  The device will remain in this mode for \a secs seconds.

    \sa bringUp(), bringUpOneItem()
*/
void QCommDeviceController::bringUpTimed(int secs)
{
    m_data->bringUpTimed(secs);
}

/*!
    Brings up the device for one item.  The device will attempt to enter the
    \bold{OnOneItem} mode.

    \sa bringUp(), bringUpTimed()
*/
void QCommDeviceController::bringUpOneItem()
{
    m_data->bringUpOneItem();
}

/*!
    Brings down the device.  The device will attempt to enter the \bold{Off} mode.

    \sa bringUp()
*/
void QCommDeviceController::bringDown()
{
    m_data->bringDown();
}

/*!
    Returns true if there are applications using this device.
*/
bool QCommDeviceController::sessionsActive() const
{
    return m_data->sessionsActive();
}

/*!
    Returns true if the device is currently turned on.  E.g. it is in
    On, OnTimed or OnOneItem mode.

    \sa bringUp(), bringDown(), powerState()
*/
bool QCommDeviceController::isUp() const
{
    return m_data->isUp();
}

/*!
    Returns the current power state of the device.

    \sa isUp()
*/
QCommDeviceController::PowerState QCommDeviceController::powerState() const
{
    return m_data->powerState();
}

/*!
    \fn void QCommDeviceController::up()

    This signal is emitted when the device is turned on.

    \sa bringUp()
*/

/*!
    \fn void QCommDeviceController::down()

    This signal is emitted when the device is turned off.

    \sa bringDown()
*/

/*!
    \fn void QCommDeviceController::powerStateChanged(QCommDeviceController::PowerState state)

    This signal is emitted when a device's power state has been changed.  The
    \a state parameter holds the new power state.

    \sa powerState()
*/

#include "qcommdevicecontroller.moc"
