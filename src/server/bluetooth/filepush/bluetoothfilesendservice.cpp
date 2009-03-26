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

#include "qabstractmessagebox.h"
#include "bluetoothfilesendservice.h"
#include "qtopiaserverapplication.h"
#include "sessionedobexclient.h"
#include "obexpushrequestsender.h"

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpquery.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothremotedevicedialog.h>
#include <qbluetoothrfcommsocket.h>

#include <qcontent.h>
#include <qcontact.h>
#include <qdsactionrequest.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qwaitwidget.h>

#include <QPointer>


static void bluetoothfilesendservice_showWarningDialog(const QString &title, const QString &text)
{
    // don't run as modal dialog or else the soft menu buttons cannot
    // be used to close the dialog
    QAbstractMessageBox *dlg = qtopiaWidget<QAbstractMessageBox>(0);
    if (dlg) {
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setIcon(QAbstractMessageBox::Warning);
        dlg->setWindowTitle(title);
        dlg->setText(text);
        QtopiaApplication::showDialog(dlg);
    }
}


class BluetoothFileSendServicePrivate
{
public:
    ObexPushRequestSender *requestSender;
};


/*!
    \class BluetoothFileSendService
    \inpublicgroup QtBluetoothModule
    \brief The BluetoothFileSendService class runs a BluetoothPush service and provides updates regarding the progress of each file transfer request that is sent through the service.
    \ingroup QtopiaServer::Task::Bluetooth

    A unique ID is generated for each request that is sent through the
    BluetoothPush service. This ID is used to provide transfer updates
    through the outgoingTransferStarted(), transferProgress() and
    transferFinished() signals. The ID can also be used to cancel transfers
    using abortTransfer().

    \bold{Note:} This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
*/

/*!
    Creates a service with parent object \a parent.
*/
BluetoothFileSendService::BluetoothFileSendService(QObject *parent)
    : FileTransferTask(parent),
      d(new BluetoothFileSendServicePrivate)
{
    qLog(Bluetooth) << "Starting BluetoothFileSendService...";

    d->requestSender = new ObexPushRequestSender(this);
    connect(d->requestSender, SIGNAL(requestStarted(int,QString,QString,QString)),
            SIGNAL(outgoingTransferStarted(int,QString,QString,QString)));
    connect(d->requestSender, SIGNAL(requestProgress(int,qint64,qint64)),
            SIGNAL(transferProgress(int,qint64,qint64)));
    connect(d->requestSender, SIGNAL(requestFinished(int,bool,bool)),
            SIGNAL(transferFinished(int,bool,bool)));

    new BluetoothPushingService(d->requestSender, this);
}

/*!
    Destroys the service.
*/
BluetoothFileSendService::~BluetoothFileSendService()
{
    delete d;
}

/*!
    \reimp
*/
QContentId BluetoothFileSendService::transferContentId(int id) const
{
    return d->requestSender->requestContentId(id);
}

/*!
    \reimp
*/
void BluetoothFileSendService::abortTransfer(int id)
{
    d->requestSender->abortRequest(id);
}

/*!
    \fn void BluetoothFileSendService::outgoingTransferStarted(int id, const QString &name, const QString &mimeType, const QString &description)

    This signal is emitted when a transfer request begins. The \a id uniquely
    identifies the transfer, as provided by FileTransferTask::BluetoothFileSendService::nextTransferId().
    The \a name, \a mimeType and \a description are respectively set to the
    Name, Type and Description metadata values that will be sent in the request.
*/

/*!
    \fn void BluetoothFileSendService::transferProgress(int id, qint64 done, qint64 total)

    This signal is emitted to indicate the progress of the transfer identified
    by \a id. \a done is the amount of data that has already been sent, and
    \a total is the total amount of data to be sent. If the total amount
    cannot be determined, \a total is set to 0.
*/

/*!
    \fn void BluetoothFileSendService::transferFinished(int id, bool error, bool aborted)

    This signal is emitted when the transfer identified by \a id is finished.
    \a error is \c true if the transfer failed, and \a aborted is
    \c true if the transfer was aborted following a call to abortRequest().
*/


//================================================================


class SessionedBluetoothObexClient : public SessionedObexClient
{
    Q_OBJECT
public:
    SessionedBluetoothObexClient(const QBluetoothAddress &remoteAddress = QBluetoothAddress(),
                                 QObject *parent = 0);
    ~SessionedBluetoothObexClient();

    virtual void begin(const QObexHeader &header, QIODevice *dataToSend);
    virtual void abort();

protected:
    virtual void openedDeviceSession(bool error);
    virtual void closedDeviceSession();
    virtual void forceAbort();

private slots:
    void sdpQueryComplete(const QBluetoothSdpQueryResult &result);
    void rfcommSocketConnected();
    void rfcommSocketError(QBluetoothAbstractSocket::SocketError error);

private:
    QBluetoothAddress m_remoteAddr;
    QBluetoothSdpQuery *m_sdpQuery;
    QPointer<QBluetoothRfcommSocket> m_socket;
    bool m_connecting;
    bool m_aborted;
    QWaitWidget *m_wait;
};


SessionedBluetoothObexClient::SessionedBluetoothObexClient(const QBluetoothAddress &remoteAddress, QObject *parent)
    : SessionedObexClient(QBluetoothLocalDeviceManager().defaultDevice().toLatin1(), parent),
      m_remoteAddr(remoteAddress),
      m_sdpQuery(0),
      m_socket(0),
      m_connecting(false),
      m_aborted(false),
      m_wait(new QWaitWidget(0))
{
}

SessionedBluetoothObexClient::~SessionedBluetoothObexClient()
{
    delete m_wait;
}

void SessionedBluetoothObexClient::begin(const QObexHeader &header, QIODevice *dataToSend)
{
    m_wait->setText(tr("Preparing to send..."));
    m_wait->show();
    SessionedObexClient::begin(header, dataToSend);
}

void SessionedBluetoothObexClient::abort()
{
    m_wait->setText(tr("Closing..."));
    m_aborted = true;
    if (m_sdpQuery)
        m_sdpQuery->cancelSearch();
    SessionedObexClient::abort();
}

void SessionedBluetoothObexClient::openedDeviceSession(bool error)
{
    qLog(Bluetooth) << "SessionedBluetoothObexClient: Bluetooth session opened OK?"
            << error;

    if (error) {
        bluetoothfilesendservice_showWarningDialog(tr("Bluetooth Error"),
            tr("<P>The Bluetooth device is currently unavailable."));
        abort();
        return;
    }

    if (!m_remoteAddr.isValid()) {
        QSet<QBluetooth::SDPProfile> profiles;
        profiles.insert(QBluetooth::ObjectPushProfile);
        m_remoteAddr = QBluetoothRemoteDeviceDialog::getRemoteDevice(0, profiles);
        if (!m_remoteAddr.isValid()) {
            abort();
            return;
        }
    }

    m_sdpQuery = new QBluetoothSdpQuery(this);
    connect(m_sdpQuery, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
            SLOT(sdpQueryComplete(QBluetoothSdpQueryResult)));

    m_wait->setText(tr("Connecting..."));

    QBluetoothLocalDevice local;
    if (!m_sdpQuery->searchServices(m_remoteAddr, local,
                QBluetooth::ObjectPushProfile)) {
        bluetoothfilesendservice_showWarningDialog(tr("Bluetooth Error"),
            tr("<P>Unable to set up the Bluetooth connection."));
        abort();
    }
}

void SessionedBluetoothObexClient::closedDeviceSession()
{
    qLog(Bluetooth) << "SessionedBluetoothObexClient: Bluetooth session closed";
    m_wait->hide();

    if (m_socket && m_socket->state() == QBluetoothAbstractSocket::ConnectedState) {
        m_socket->setParent(0);
        connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
        m_socket->disconnect();
    } else {
        delete m_socket;
        m_socket = 0;
    }
}

void SessionedBluetoothObexClient::forceAbort()
{
    if (m_socket)
        m_socket->disconnect();
}

void SessionedBluetoothObexClient::sdpQueryComplete(const QBluetoothSdpQueryResult &result)
{
    if (m_aborted) {
        qLog(Bluetooth) << "SessionedBluetoothObexClient: request cancelled";
        abort();
        return;
    }

    if (!result.isValid() || result.services().isEmpty()) {
        bluetoothfilesendservice_showWarningDialog(tr("Transfer Error"),
            tr("<P>The selected device does not allow file transfer requests."));
        abort();
        return;
    }

    qLog(Bluetooth) << "SessionedBluetoothObexClient: found"
            << result.services().count()
            << "OBEX Push services on" << m_remoteAddr.toString();

    // the SDP query was specifically for Object Push services, so any services
    // in the given results are guaranteed to be Object Push services
    int channel = QBluetoothSdpRecord::rfcommChannel(result.services().first());

    m_socket = new QBluetoothRfcommSocket(this);
    connect(m_socket, SIGNAL(connected()), SLOT(rfcommSocketConnected()));
    connect(m_socket, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
            SLOT(rfcommSocketError(QBluetoothAbstractSocket::SocketError)));
    m_connecting = true;
    m_socket->connect(QBluetoothAddress::any, m_remoteAddr, channel);

    delete m_sdpQuery;
    m_sdpQuery = 0;
}

void SessionedBluetoothObexClient::rfcommSocketConnected()
{
    qLog(Bluetooth) << "SessionedBluetoothObexClient: RFCOMM socket connected";
    m_connecting = false;

    if (m_aborted) {
        qLog(Bluetooth) << "SessionedBluetoothObexClient: request cancelled";
        abort();
        return;
    }

    m_wait->hide();
    sendRequest(m_socket);
}

void SessionedBluetoothObexClient::rfcommSocketError(QBluetoothAbstractSocket::SocketError error)
{
    qLog(Bluetooth) << "SessionedBluetoothObexClient: RFCOMM socket error:"
            << error;

    if (m_connecting) {
        bluetoothfilesendservice_showWarningDialog(tr("Bluetooth Error"),
            tr("<P>Could not connect to the selected bluetooth device. "
               "Make sure the selected device is turned on and within range."));
        delete m_socket;
        m_socket = 0;
    }
    abort();
}


//=====================================================

class BluetoothPushingServicePrivate
{
public:
    ObexPushRequestSender *requestSender;

    SessionedBluetoothObexClient *createDefaultClient() const;
};

SessionedBluetoothObexClient *BluetoothPushingServicePrivate::createDefaultClient() const
{
    QSet<QBluetooth::SDPProfile> profiles;
    profiles.insert(QBluetooth::ObjectPushProfile);
    return new SessionedBluetoothObexClient;
}


/*!
    \service BluetoothPushingService BluetoothPush
    \inpublicgroup QtBluetoothModule
    \brief The BluetoothPushingService class provides the BluetoothPush service.

    The \i BluetoothPush service enables applications to send
    files over the Bluetooth link. This service will present a user interface
    to ask the user to choose the Bluetooth device that should receive the
    file, establish the connection to the remote device, and provide the user
    with progress and status information during the transmission.

    This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
 */

/*!
    \internal
*/
BluetoothPushingService::BluetoothPushingService(ObexPushRequestSender *requestSender, QObject *parent)
    : QtopiaAbstractService("BluetoothPush", parent),
      d(new BluetoothPushingServicePrivate)
{
    d->requestSender = requestSender;
    publishAll();
}

/*!
    \internal
*/
BluetoothPushingService::~BluetoothPushingService()
{
    delete d;
}

/*!
    Sends the user's personal business card. If the user has not set a
    personal business card, the user will be notified appropriately.

    \sa pushBusinessCard()
*/
void BluetoothPushingService::pushPersonalBusinessCard()
{
    SessionedBluetoothObexClient *client = d->createDefaultClient();
    if (client) {
        d->requestSender->sendPersonalBusinessCard(client,
            BluetoothFileSendService::nextTransferId());
    }
}

/*!
    Sends the user's personal business card to the device specified by
    \a address. If the user has not set a personal business card, the user
    will be notified appropriately.

    \sa pushBusinessCard()
*/
void BluetoothPushingService::pushPersonalBusinessCard(const QBluetoothAddress &address)
{
    d->requestSender->sendPersonalBusinessCard(
            new SessionedBluetoothObexClient(address),
            BluetoothFileSendService::nextTransferId());
}

/*!
    Sends the business card represented by \a contact.

    \sa pushPersonalBusinessCard()
*/
void BluetoothPushingService::pushBusinessCard(const QContact &contact)
{
    SessionedBluetoothObexClient *client = d->createDefaultClient();
    if (client) {
        d->requestSender->sendBusinessCard(client,
                BluetoothFileSendService::nextTransferId(), contact);
    }
}

/*!
    Sends the business card represented by \a request. The
    request object should contain raw serialized QContact data.

    \sa pushPersonalBusinessCard()
*/
void BluetoothPushingService::pushBusinessCard(const QDSActionRequest& request)
{
    SessionedBluetoothObexClient *client = d->createDefaultClient();
    if (client) {
        d->requestSender->sendBusinessCard(client,
                BluetoothFileSendService::nextTransferId(), request);
    }
}

/*!
    Sends the file stored at \a filePath. The specified
    \a mimeType and \a description (both optional) will be sent in the
    request to provide additional metadata for the file.

    Set \a autoDelete to \c true if the file stores at \a filePath should
    be deleted after it has been sent.
*/
void BluetoothPushingService::pushFile(const QString &filePath, const QString &mimeType, const QString &description, bool autoDelete)
{
    SessionedBluetoothObexClient *client = d->createDefaultClient();
    if (client) {
        d->requestSender->sendFile(client,
                BluetoothFileSendService::nextTransferId(), filePath,
                mimeType, description, autoDelete);
    }
}

/*!
    Sends the document identified by \a contentId.
 */
void BluetoothPushingService::pushFile(const QContentId &contentId)
{
    SessionedBluetoothObexClient *client = d->createDefaultClient();
    if (client) {
        d->requestSender->sendContent(client,
                BluetoothFileSendService::nextTransferId(), contentId);
    }
}

/*!
    Sends the document identified by \a contentId to the device specified by
    \a address.
 */
void BluetoothPushingService::pushFile(const QBluetoothAddress &address, const QContentId &contentId)
{
    d->requestSender->sendContent(new SessionedBluetoothObexClient(address),
            BluetoothFileSendService::nextTransferId(), contentId);
}

/*!
    Sends the vCalendar object represented by \a request.  The
    request object should contain raw serialized QTask or QAppointment data.
 */
void BluetoothPushingService::pushCalendar(const QDSActionRequest &request)
{
    SessionedBluetoothObexClient *client = d->createDefaultClient();
    if (client) {
        d->requestSender->sendCalendar(client,
                BluetoothFileSendService::nextTransferId(), request);
    }
}


QTOPIA_TASK(BluetoothFileSendService, BluetoothFileSendService);
QTOPIA_TASK_PROVIDES(BluetoothFileSendService, FileTransferTask);

#include "bluetoothfilesendservice.moc"
