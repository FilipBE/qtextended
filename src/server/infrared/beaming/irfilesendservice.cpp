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

#include "irfilesendservice.h"
#include "qtopiaserverapplication.h"
#include "qabstractmessagebox.h"
#include "sessionedobexclient.h"
#include "obexpushrequestsender.h"

#include <qirlocaldevice.h>
#include <qirserver.h>
#include <qirnamespace.h>
#include <qirsocket.h>
#include <qirremotedevice.h>

#include <qcontent.h>
#include <qcontact.h>
#include <qdsactionrequest.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <QPointer>


class IrFileSendServicePrivate
{
public:
    ObexPushRequestSender *requestSender;
};

/*!
    \class IrFileSendService
    \inpublicgroup QtInfraredModule
    \brief The IrFileSendService class runs the InfraredBeaming service and provides updates regarding the progress of each file transfer request that is sent through the service.
    \ingroup QtopiaServer::Task

    A unique ID is generated for each request that is sent through the
    InfraredBeaming service. This ID is used to provide transfer updates
    through the outgoingTransferStarted(), transferProgress() and
    transferFinished() signals. The ID can also be used to cancel transfers
    using abortTransfer().

    \bold{Note:} This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
*/

/*!
    Creates a service with parent object \a parent.
*/
IrFileSendService::IrFileSendService(QObject *parent)
    : FileTransferTask(parent),
      d(new IrFileSendServicePrivate)
{
    qLog(Infrared) << "Starting IrFileSendService...";

    d->requestSender = new ObexPushRequestSender(this);
    connect(d->requestSender, SIGNAL(requestStarted(int,QString,QString,QString)),
            SIGNAL(outgoingTransferStarted(int,QString,QString,QString)));
    connect(d->requestSender, SIGNAL(requestProgress(int,qint64,qint64)),
            SIGNAL(transferProgress(int,qint64,qint64)));
    connect(d->requestSender, SIGNAL(requestFinished(int,bool,bool)),
            SIGNAL(transferFinished(int,bool,bool)));

    new IrBeamingService(d->requestSender, this);
}

/*!
    Destroys the service.
*/
IrFileSendService::~IrFileSendService()
{
    delete d;
}

/*!
    \reimp
*/
QContentId IrFileSendService::transferContentId(int id) const
{
    return d->requestSender->requestContentId(id);
}

/*!
    \reimp
*/
void IrFileSendService::abortTransfer(int id)
{
    d->requestSender->abortRequest(id);
}

/*!
    \fn void IrFileSendService::outgoingTransferStarted(int id, const QString &name, const QString &mimeType, const QString &description)

    This signal is emitted when a transfer request begins. The \a id uniquely
    identifies the transfer, as provided by FileTransferTask::IrFileSendService::nextTransferId().
    The \a name, \a mimeType and \a description are respectively set to the
    Name, Type and Description metadata values that will be sent in the request.
*/

/*!
    \fn void IrFileSendService::transferProgress(int id, qint64 done, qint64 total)

    This signal is emitted to indicate the progress of the transfer identified
    by \a id. \a done is the amount of data that has already been sent, and
    \a total is the total amount of data to be sent. If the total amount
    cannot be determined, \a total is set to 0.
*/

/*!
    \fn void IrFileSendService::transferFinished(int id, bool error, bool aborted)

    This signal is emitted when the transfer identified by \a id is finished.
    \a error is \c true if the transfer failed, and \a aborted is
    \c true if the transfer was aborted following a call to abortRequest().
*/


//=====================================================================


class SessionedIrObexClient : public SessionedObexClient
{
    Q_OBJECT
public:
    SessionedIrObexClient(const QByteArray &deviceName, QObject *parent = 0);

    virtual void abort();

protected:
    virtual void openedDeviceSession(bool error);
    virtual void closedDeviceSession();
    virtual void forceAbort();

private slots:
    void remoteDevicesFound(const QList<QIrRemoteDevice> &devices);
    void socketConnected();
    void socketError(QIrSocket::SocketError error);

private:
    QIrLocalDevice *m_local;
    QPointer<QIrSocket> m_socket;
    bool m_connecting;
    bool m_aborted;
};


SessionedIrObexClient::SessionedIrObexClient(const QByteArray &deviceName, QObject *parent)
    : SessionedObexClient(deviceName, parent),
      m_local(new QIrLocalDevice(deviceName)),
      m_socket(0),
      m_connecting(false),
      m_aborted(false)
{
    connect(m_local, SIGNAL(remoteDevicesFound(QList<QIrRemoteDevice>)),
        this, SLOT(remoteDevicesFound(QList<QIrRemoteDevice>)));
}

void SessionedIrObexClient::abort()
{
    m_aborted = true;
    SessionedObexClient::abort();
}

void SessionedIrObexClient::openedDeviceSession(bool error)
{
    qLog(Infrared) << "SessionedIrObexClient: IR session opened OK?" << error;

    if (error) {
        QAbstractMessageBox::warning(0, tr("Infrared Error"),
            tr("<P>The Infrared device is currently unavailable."));
        abort();
        return;
    }

    int result = QAbstractMessageBox::information(0, tr("Beam File"),
            tr("<P>Please align the infrared receivers and press OK when ready."),
                                    QAbstractMessageBox::Ok, QAbstractMessageBox::Cancel);

    if (result == QAbstractMessageBox::Cancel) {
        abort();
        return;
    }

    if (!m_local->discoverRemoteDevices(QIr::OBEX)) {
        // This should never happen in practice if we have an open session
        QAbstractMessageBox::warning(0, tr("Infrared Error"),
            tr("<P>There was an error while using the infrared device."));
        abort();
    }
}

void SessionedIrObexClient::closedDeviceSession()
{
    qLog(Infrared) << "SessionedIrObexClient: IR session closed";

    delete m_socket;
    m_socket = 0;
}

void SessionedIrObexClient::forceAbort()
{
    if (!m_socket)
        m_socket->disconnect();
}

void SessionedIrObexClient::remoteDevicesFound(const QList<QIrRemoteDevice> &devices)
{
    if (m_aborted) {
        qLog(Infrared) << "SessionedIrObexClient: request cancelled";
        abort();
        return;
    }

    if (devices.size() == 0) {
        QAbstractMessageBox::warning(0, tr("Infrared Error"),
            tr("<P>Could not connect to the other device. Make sure the Infrared ports are aligned and the other device's Infrared capabilities are turned on."));
        abort();
        return;
    }

    // Now make an Infrared Connection to the remote host
    m_socket = new QIrSocket(this);
    connect(m_socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(m_socket, SIGNAL(error(QIrSocket::SocketError)),
            SLOT(socketError(QIrSocket::SocketError)));

    qLog(Infrared) << "Found devices, connecting...";

    // Pick the first available device for now
    m_connecting = true;
    m_socket->connect("OBEX", devices[0].address());
}

void SessionedIrObexClient::socketConnected()
{
    qLog(Infrared) << "SessionedIrObexClient: socket connected";
    m_connecting = false;

    if (m_aborted) {
        qLog(Infrared) << "SessionedIrObexClient: request cancelled";
        abort();
        return;
    }

    sendRequest(m_socket);
}

void SessionedIrObexClient::socketError(QIrSocket::SocketError error)
{
    qLog(Infrared) << "Push request: infrared socket error:" << error;

    if (m_connecting) {
        QAbstractMessageBox::warning(0, tr("Infrared Error"),
            tr("<P>Could not connect to the other device. Make sure the Infrared ports are aligned and the other device's Infrared capabilities are turned on."));
        delete m_socket;
        m_socket = 0;
    }
    abort();
}


//=====================================================


class IrBeamingServicePrivate
{
public:
    bool checkDeviceAvailable();

    ObexPushRequestSender *requestSender;
    QByteArray deviceId;
};

bool IrBeamingServicePrivate::checkDeviceAvailable()
{
    //TODO if infrared device is removed, this variable should be cleared
    // to empty string
    if (!deviceId.isEmpty())
        return true;

    QStringList irDevices = QIrLocalDevice::devices();
    if (irDevices.isEmpty()) {
        QAbstractMessageBox::warning(0, QObject::tr("Infrared Error"),
            QObject::tr("<P>No infrared devices found."));
        return false;
    }
    deviceId = irDevices[0].toLatin1();
    return true;
}


/*!
    \service IrBeamingService InfraredBeaming
    \inpublicgroup QtInfraredModule
    \brief The IrBeamingService class provides the InfraredBeaming service.

    The \i InfraredBeaming service enables applications to send
    files over the Infrared link. The service takes care of
    establishing the Infrared connection and providing the user
    with progress and status information during the transmission.

    \bold{Note:} This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
 */

/*!
    \internal
 */
IrBeamingService::IrBeamingService(ObexPushRequestSender *requestSender, QObject *parent)
    : QtopiaAbstractService("InfraredBeaming", parent),
      d(new IrBeamingServicePrivate)
{
    d->requestSender = requestSender;
    publishAll();
}

/*!
    \internal
*/
IrBeamingService::~IrBeamingService()
{
    delete d;
}

/*!
    Sends the user's personal business card. If the user has not set a
    personal business card, the user will be notified appropriately.

    \sa beamBusinessCard()
*/
void IrBeamingService::beamPersonalBusinessCard()
{
    if (!d->checkDeviceAvailable())
        return;
    d->requestSender->sendPersonalBusinessCard(
            new SessionedIrObexClient(d->deviceId),
            IrFileSendService::nextTransferId());
}

/*!
    Sends the business card represented by \a contact.

    \sa beamPersonalBusinessCard()
*/
void IrBeamingService::beamBusinessCard(const QContact &contact)
{
    if (!d->checkDeviceAvailable())
        return;
    d->requestSender->sendBusinessCard(new SessionedIrObexClient(d->deviceId),
            IrFileSendService::nextTransferId(), contact);
}

/*!
    Sends the business card represented by \a request. The
    request object should contain raw serialized QContact data.

    \sa beamPersonalBusinessCard()
*/
void IrBeamingService::beamBusinessCard(const QDSActionRequest &request)
{
    if (!d->checkDeviceAvailable())
        return;
    d->requestSender->sendBusinessCard(new SessionedIrObexClient(d->deviceId),
            IrFileSendService::nextTransferId(), request);
}

/*!
    Sends the file stored at \a filePath. The specified
    \a mimeType and \a description (both optional) will be sent in the
    request to provide additional metadata for the file.

    Set \a autoDelete to \c true if the file stores at \a filePath should
    be deleted after it has been sent.
*/
void IrBeamingService::beamFile(const QString &filePath, const QString &mimeType, const QString &description, bool autoDelete)
{
    if (!d->checkDeviceAvailable())
        return;
    d->requestSender->sendFile(new SessionedIrObexClient(d->deviceId),
            IrFileSendService::nextTransferId(), filePath, mimeType,
            description, autoDelete);
}

/*!
    Sends the document identified by \a contentId.
 */
void IrBeamingService::beamFile(const QContentId &contentId)
{
    if (!d->checkDeviceAvailable())
        return;
    d->requestSender->sendContent(new SessionedIrObexClient(d->deviceId),
            IrFileSendService::nextTransferId(), contentId);
}

/*!
    Sends the vCalendar object represented by \a request.  The
    request object should contain raw serialized QTask or QAppointment data.
 */
void IrBeamingService::beamCalendar(const QDSActionRequest &request)
{
    if (!d->checkDeviceAvailable())
        return;
    d->requestSender->sendCalendar(new SessionedIrObexClient(d->deviceId),
            IrFileSendService::nextTransferId(), request);
}


QTOPIA_TASK(IrFileSendService, IrFileSendService);
QTOPIA_TASK_PROVIDES(IrFileSendService, FileTransferTask);

#include "irfilesendservice.moc"
