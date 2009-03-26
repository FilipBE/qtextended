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

#include "sessionedobexclient.h"

#include <qobexclientsession.h>
#include <qcommdevicesession.h>
#include <qtopialog.h>

#include <QIODevice>
#include <QByteArray>
#include <QTimer>


class SessionedObexClientPrivate : public QObject
{
    Q_OBJECT
public:
    SessionedObexClientPrivate(const QByteArray &deviceId,
                               SessionedObexClient *parent = 0);
    void close();

    QCommDeviceSession *m_deviceSession;
    bool m_deviceSessionOpen;
    QObexClientSession *m_client;
    QIODevice *m_device;
    QObexHeader m_header;
    bool m_abortEnforced;

private slots:
    void deviceSessionOpen();
    void deviceSessionOpenFailed();
    void deviceSessionClosed();
    void clientRequestStarted(int id);
    void clientRequestFinished(int id, bool error);
    void clientDone();
    void enforceAbort();

private:
    SessionedObexClient *m_parent;
};

SessionedObexClientPrivate::SessionedObexClientPrivate(const QByteArray &deviceId, SessionedObexClient *parent)
    : QObject(parent),
      m_deviceSession(new QCommDeviceSession(deviceId, this)),
      m_deviceSessionOpen(false),
      m_client(0),
      m_device(0),
      m_abortEnforced(false),
      m_parent(parent)
{
    connect(m_deviceSession, SIGNAL(sessionOpen()), SLOT(deviceSessionOpen()));
    connect(m_deviceSession, SIGNAL(sessionFailed()), SLOT(deviceSessionOpenFailed()));
    connect(m_deviceSession, SIGNAL(sessionClosed()), SLOT(deviceSessionClosed()));
}

void SessionedObexClientPrivate::close()
{
    qLog(Obex) << "SessionedObexClient: closing...";
    if (m_deviceSessionOpen)
        m_deviceSession->endSession();
    else
        emit m_parent->done();
}

void SessionedObexClientPrivate::clientRequestStarted(int id)
{
    qLog(Obex) << "SessionedObexClient: request started" << id;

    // don't emit requestStarted() for Connect & Disconnect requests
    if (m_client->currentRequest() != QObex::Put) {
        qLog(Obex) << "SessionedObexClient: started a Connect or Disconnect request";
        return;
    }

    emit m_parent->requestStarted();
}

void SessionedObexClientPrivate::clientRequestFinished(int id, bool error)
{
    qLog(Obex) << "SessionedObexClient: request finished"
            << id << error;

    // don't emit requestFinished() for Connect & Disconnect requests
    if (m_client->currentRequest() != QObex::Put) {
        qLog(Obex) << "SessionedObexClient: finished a Connect or Disconnect request, error?" << error;
        return;
    }

    emit m_parent->requestFinished(error,
            (m_client->error() == QObexClientSession::Aborted) || m_abortEnforced);
}

void SessionedObexClientPrivate::clientDone()
{
    qLog(Obex) << "SessionedObexClient: internal client finished";
    close();
}

void SessionedObexClientPrivate::enforceAbort()
{
    if (m_client->currentRequest() != QObex::NoRequest) {
        qLog(Obex) << "SessionedObexClient: forcing request abort...";
        m_abortEnforced = true;
        m_parent->forceAbort();
    }
}

void SessionedObexClientPrivate::deviceSessionOpen()
{
    qLog(Obex) << "SessionedObexClient: device session opened";
    m_deviceSessionOpen = true;
    m_parent->openedDeviceSession(false);
}

void SessionedObexClientPrivate::deviceSessionOpenFailed()
{
    qLog(Obex) << "SessionedObexClient: device session open failed";
    m_deviceSessionOpen = false;
    m_parent->openedDeviceSession(true);
}

void SessionedObexClientPrivate::deviceSessionClosed()
{
    qLog(Obex) << "SessionedObexClient: device session closed";
    m_deviceSessionOpen = false;
    m_parent->closedDeviceSession();
    emit m_parent->done();
}


//====================================================

/*!
    \class SessionedObexClient
    \inpublicgroup QtInfraredModule
    \inpublicgroup QtBluetoothModule
    \brief The SessionedObexClient class provides a base implementation for performing an OBEX Put request inside of a device session for a particular hardware device.
    \ingroup QtopiaServer

    A SessionedObexClient will open a device session for the relevant hardware
    device before sending the specified OBEX request. Once the request is
    completed, the device session will be closed.

    The use of device sessions ensures the Qt Extended device manager is aware
    that a hardware device is currently in use, so that the device manager will
    try to keep the device open while the OBEX request is in progress.

    If you want to send OBEX requests without using device sessions, just
    use QObexClientSession. 

    This class enables the BluetoothFileSendService and the IrFileSendService.

    This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.


    \section1 Subclassing SessionedObexClient

    SessionedObexClient needs to be subclassed to provide the transport socket
    for the OBEX client request. When start() is called, the client will
    attempt to open a device session. When the session is opened (or fails to
    open), openedDeviceSession() is called. Subclasses should override
    this openedDeviceSession() to create an appropriate transport socket
    and then call sendRequest() with the socket object in order to begin the
    OBEX request.

    The subclass can call abort() if the session should be ended prematurely.


    \sa QCommDeviceSession, QAbstractCommDeviceManager
*/

/*!
    Creates a client that will operate on a device session for the hardware
    device with the given \a deviceId.

    The \a parent is the QObject parent object for the client.
*/
SessionedObexClient::SessionedObexClient(const QByteArray &deviceId,
QObject *parent)
    : QObject(parent),
      d(new SessionedObexClientPrivate(deviceId, this))
{
}

/*!
    Starts the client. Once the client is ready, it will send an OBEX Put
    request with \a header and the contents of \a dataToSend.
*/
void SessionedObexClient::begin(const QObexHeader &header, QIODevice *dataToSend)
{
    qLog(Obex) << "SessionedObexClient: starting...";

    d->m_header = header;
    d->m_device = dataToSend;
    d->m_deviceSession->startSession();
}

/*!
    Aborts the OBEX request and closes the device session.

    If the OBEX request has been sent (i.e. begin() and sendRequest() have
    been called) and the request has not yet finished, the client will
    attempt to abort the transfer to the server. If this is successful,
    requestFinished() is emitted with \c aborted set to \c true. Then,
    the device session is closed, closedDeviceSession() is called and done()
    is emitted.

    Otherwise, this will just close the device session, call
    closedDeviceSession() and emit done().
*/
void SessionedObexClient::abort()
{
    qLog(Obex) << "SessionedObexClient: abort current request";
    if (d->m_client) {
        qLog(Obex) << "SessionedObexClient: trying to abort current request";
        d->m_client->abort();

        // force abort if other side doesn't respond
        QTimer::singleShot(2000, d, SLOT(enforceAbort()));

    } else {
        d->close();
    }
}

/*!
    Sends the OBEX Put request specified in begin() over the given \a socket.
*/
void SessionedObexClient::sendRequest(QIODevice *socket)
{
    qLog(Obex) << "SessionedObexClient: sending request...";
    if (d->m_client) {
        qLog(Obex) << "SessionedObexClient: internal error (request already started)";
        return;
    }

    d->m_client = new QObexClientSession(socket, this);
    connect(d->m_client, SIGNAL(requestStarted(int)),
            d, SLOT(clientRequestStarted(int)));
    connect(d->m_client, SIGNAL(requestFinished(int,bool)),
            d, SLOT(clientRequestFinished(int,bool)));
    connect(d->m_client, SIGNAL(done(bool)),
            d, SLOT(clientDone()));

    // connect progress signal directly to ours
    connect(d->m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
            SIGNAL(dataSendProgress(qint64,qint64)));

    // do the request
    d->m_client->connect();
    d->m_client->put(d->m_header, d->m_device);
    d->m_client->disconnect();
}

/*!
    Called when the device session for this client has been closed.

    The device session could be closed because abort() has been called, or
    an external party has closed the session (e.g. by bringing down the
    associated hardware device).
*/
void SessionedObexClient::closedDeviceSession()
{
}

/*!
    \fn void SessionedObexClient::openedDeviceSession(bool error)

    When begin() is called, the client will open the device session. Once
    the device session is opened (or has failed to open), this method will be
    called. \a error is true if the session failed to open.

    Subclasses should override this to call sendRequest() with an appropriate
    socket object if the session was opened successfully.
*/

/*!
    \fn void SessionedObexClient::forceAbort()

    Called when abort() has been called but the OBEX server has not responded
    to the Abort request. In this case, the subclass should try to force the
    cancelation of the request - for example, by disconnecting the transport
    connection.
*/

/*!
    \fn void SessionedObexClient::requestStarted()

    This signal is emitted when the OBEX request has started. The request
    will not start until sendRequest() has been called by the subclass.
*/


/*!
    \fn void SessionedObexClient::dataSendProgress(qint64 done, qint64 total)

    This signal is emitted after the request has started to
    indicate the progress of the data transfer. The \a done value is the
    number of bytes that have been sent or received so far, and \a total
    is the total number of bytes to be sent or received. If the total amount
    cannot be determined, \a total is set to 0.
*/

/*!
    \fn void SessionedObexClient::requestFinished(bool error, bool aborted)

    This signal is emitted when the OBEX request has finished. \a error
    is set to \c true if an error occurred during the request, and
    \a aborted is set to \c true if the request was aborted by a call
    to abort().
*/

/*!
    \fn void SessionedObexClient::done()

    This signal is emitted once the device session has closed
    and closedDeviceSession() has been called.
*/

#include "sessionedobexclient.moc"
