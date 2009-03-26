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

#include "gpsdwhereabouts_p.h"

#include <QTcpSocket>
#include <QBasicTimer>
#include <QDateTime>
#include <QTimerEvent>
#include <QDebug>

// when in 'watcher' mode, GPSd sends updates whenever they are available
static const QByteArray Q_GPSD_WATCHER_MODE_ON("w+\n");
static const QByteArray Q_GPSD_WATCHER_MODE_OFF("w-\n");
static const QByteArray Q_GPSD_GET_FIX("o\n");
static const QByteArray Q_GPSD_GET_STATUS("x\n");

/*!
    \internal
    \class QGpsdWhereabouts
    \inpublicgroup QtLocationModule
    \ingroup whereabouts
    \brief The QGpsdWhereabouts class reads and distributes the positional data received from a GPSd daemon.

    GPSd is a service daemon that connects to a GPS device and then serves the
    data to clients over TCP. It is available at http://gpsd.berlios.de.
*/

/*!
    \enum QGpsdWhereabouts::ActionWhenConnected
    \internal
*/

/*!
    Constructs a QGpsdWhereabouts instance with the given \a parent that
    connects to the GPSd daemon on the given \a addr and \a port.
*/
QGpsdWhereabouts::QGpsdWhereabouts(QObject *parent, const QHostAddress &addr, quint16 port)
    : QWhereabouts(QWhereabouts::TerminalBasedUpdate, parent),
      m_addr(addr),
      m_port(port),
      m_sock(new QTcpSocket(this)),
      m_queryTimer(0)
{
    connect(m_sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_sock, SIGNAL(readyRead()), SLOT(socketReadyRead()));
}

/*!
    Destroys the instance.
*/
QGpsdWhereabouts::~QGpsdWhereabouts()
{
    delete m_queryTimer;
}

/*!
    \reimp
*/
void QGpsdWhereabouts::startUpdates()
{
    if (m_sock->state() == QAbstractSocket::ConnectedState) {

        // reset if startUpdates() was previously called
        stopUpdates();

        if (updateInterval() > 0) {
            if (!m_queryTimer)
                m_queryTimer = new QBasicTimer;
            m_queryTimer->start(updateInterval(), this);
        } else {
            m_sock->write(Q_GPSD_WATCHER_MODE_ON);
        }

    } else {
        m_actionsWhenConnected.enqueue(PeriodicUpdates);
        initialize();
    }
}

/*!
    \reimp
*/
void QGpsdWhereabouts::stopUpdates()
{
    if (m_queryTimer)
        m_queryTimer->stop();
    if (m_sock->state() == QAbstractSocket::ConnectedState) {
        m_sock->write(Q_GPSD_WATCHER_MODE_OFF);
        m_sock->flush();    // stop updates ASAP
    }
}

/*!
    \reimp
*/
void QGpsdWhereabouts::requestUpdate()
{
    if (m_sock->state() == QAbstractSocket::ConnectedState) {
        m_sock->write(Q_GPSD_GET_FIX);
        m_sock->write(Q_GPSD_GET_STATUS);
    } else {
        m_actionsWhenConnected.enqueue(SingleUpdate);
        initialize();
    }
}

void QGpsdWhereabouts::initialize()
{
    if (m_sock->state() == QAbstractSocket::UnconnectedState) {
        setState(QWhereabouts::Initializing);
        m_sock->connectToHost(m_addr, m_port);
    }
}

/*!
    \internal
*/
void QGpsdWhereabouts::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    requestUpdate();
}

void QGpsdWhereabouts::socketError(QAbstractSocket::SocketError)
{
    if (m_sock->state() == QAbstractSocket::UnconnectedState)
        setState(QWhereabouts::NotAvailable);
}

void QGpsdWhereabouts::socketStateChanged(QAbstractSocket::SocketState state)
{
    switch (state) {
        case QAbstractSocket::UnconnectedState:
            setState(QWhereabouts::NotAvailable);
            m_actionsWhenConnected.clear();
            break;
        case QAbstractSocket::ConnectedState:
            setState(QWhereabouts::Available);
            while (m_actionsWhenConnected.size() > 0) {
                switch (m_actionsWhenConnected.dequeue()) {
                    case PeriodicUpdates:
                        startUpdates();
                        break;
                    case SingleUpdate:
                        requestUpdate();
                        break;
                }
            }
            break;
        default:
            break;
    }
}

void QGpsdWhereabouts::socketReadyRead()
{
    QByteArray reply = m_sock->readLine();
    while (!reply.isEmpty()) {
        if (reply.length() > 6) {
            switch (reply.at(5)) {
                case 'O':
                    parseFix(reply.mid(7));
                    break;
                case 'X':
                    parseDeviceStatus(reply.mid(7));
                    break;
                default:
                    break;
            }
        }
        reply = m_sock->readLine();
    }
}


#define Q_UPDATE_FLOAT_IF_PRESENT(func, value) if (value[0] != '?') func(value.toFloat());
#define Q_UPDATE_DOUBLE_IF_PRESENT(func, value) if (value[0] != '?') func(value.toDouble());

void QGpsdWhereabouts::parseFix(const QByteArray &fix)
{
    if (fix.length() > 0 && fix[0] == '?') {
        setState(QWhereabouts::Available);
        return;
    }

    QWhereaboutsUpdate update;
    QTextStream in(fix);

    QByteArray tag, time, terr, lat, lng, alt, herr, verr, course, speed, climb,
        courseerr, speederr, climberr, mode;
    in >> tag >> time >> terr >> lat >> lng >> alt >> herr >> verr >> course
            >> speed >> climb >> courseerr >> speederr >> climberr >> mode;

    QWhereaboutsCoordinate coord;
    Q_UPDATE_DOUBLE_IF_PRESENT(coord.setLatitude, lat);
    Q_UPDATE_DOUBLE_IF_PRESENT(coord.setLongitude, lng);
    Q_UPDATE_DOUBLE_IF_PRESENT(coord.setAltitude, alt);
    update.setCoordinate(coord);

    Q_UPDATE_FLOAT_IF_PRESENT(update.setUpdateTimeAccuracy, terr);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setHorizontalAccuracy, herr);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setVerticalAccuracy, verr);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setCourse, course);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setGroundSpeed, speed);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setVerticalSpeed, climb);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setCourseAccuracy, courseerr);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setGroundSpeedAccuracy, speederr);
    Q_UPDATE_FLOAT_IF_PRESENT(update.setVerticalSpeedAccuracy, climberr);

    // time is guaranteed to be present
    QDateTime dt = QDateTime::fromTime_t(uint(time.toDouble()));
    int timeSepIndex = time.indexOf(".");
    if (timeSepIndex != -1)
        dt = dt.addMSecs(time.mid(timeSepIndex + 1, 3).toInt());
    dt = dt.toUTC();
    update.setUpdateDateTime(dt);

    if (mode.length() > 0) {
        if (mode[0] == '1')
            setState(QWhereabouts::Available);
        else if (mode[0] == '2' || mode[0] == '3')
            setState(QWhereabouts::PositionFixAcquired);
    }

    if (!update.isNull())
        emitUpdated(update);
}

void QGpsdWhereabouts::parseDeviceStatus(const QByteArray &status)
{
    if (status.length() > 0 && status[0] == '0') {
        setState(QWhereabouts::NotAvailable);
    } else {
        if (state() == QWhereabouts::NotAvailable)
            setState(QWhereabouts::Available);
    }
}
