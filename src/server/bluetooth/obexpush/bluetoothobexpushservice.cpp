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

#include "bluetoothobexpushservice.h"
#include "defaultobexpushservice.h"

#include <qbluetoothabstractservice.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothrfcommsocket.h>
#include <qbluetoothrfcommserver.h>
#include <qbluetoothsdprecord.h>

#include <qcommdevicesession.h>
#include <qtopialog.h>

#include <QHash>
#include <QFile>
#include <QSet>
#include <QTimer>


class BluetoothObexPushServer : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    BluetoothObexPushServer(const QString &serviceName, const QBluetoothSdpRecord &sdpRecord, QObject *parent = 0);

    virtual void start();
    virtual void stop();
    virtual void setSecurityOptions(QBluetooth::SecurityOptions options);

signals:
    void newConnection(QBluetoothRfcommSocket *socket);

private slots:
    void newConnection();
    void socketDisconnected();
    void socketDestroyed(QObject *socket);
    void deviceSessionClosed();

private:
    void close();

    QBluetoothRfcommServer *m_server;
    QBluetoothLocalDevice *m_local;
    QBluetoothSdpRecord m_sdpRecord;
    quint32 m_sdpRecordHandle;
    QBluetooth::SecurityOptions m_securityOptions;
    QCommDeviceSession *m_deviceSession;
    QSet<QBluetoothRfcommSocket *> m_activeSockets;
    int m_numBtSessions;
};


BluetoothObexPushServer::BluetoothObexPushServer(const QString &serviceName, const QBluetoothSdpRecord &sdpRecord, QObject *parent)
    : QBluetoothAbstractService("ObexObjectPush", serviceName, parent),
      m_server(0),
      m_local(0),
      m_sdpRecord(sdpRecord),
      m_sdpRecordHandle(0),
      m_securityOptions(0),
      m_deviceSession(0),
      m_numBtSessions(0)
{
}

void BluetoothObexPushServer::close()
{
    qLog(Bluetooth) << "BluetoothObexPushServer: closing...";

    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = 0;
    }
}

void BluetoothObexPushServer::start()
{
    qLog(Bluetooth) << "BluetoothObexPushServer: starting...";

    if (!m_server) {
        m_server = new QBluetoothRfcommServer(this);
        connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));
    }

    if (!m_local) {
        m_local = new QBluetoothLocalDevice(this);
        if (!m_local->isValid()) {
            emit started(true, tr("Cannot access local bluetooth device"));
            return;
        }
    }

    m_sdpRecordHandle = 0;
    if (!m_sdpRecord.isNull())
        m_sdpRecordHandle = registerRecord(m_sdpRecord);
    if (m_sdpRecordHandle == 0) {
        emit started(true, tr("Error registering with SDP server"));
        return;
    }

    int channel = QBluetoothSdpRecord::rfcommChannel(m_sdpRecord);
    if (!m_server->listen(m_local->address(), channel)) {
        unregisterRecord(m_sdpRecordHandle);
        close();
        emit started(true, tr("Error listening on OBEX Push Server"));
        return;
    }

    m_server->setSecurityOptions(m_securityOptions);
    emit started(false, QString());

    if (!m_deviceSession) {
        QBluetoothLocalDeviceManager manager;
        m_deviceSession = new QCommDeviceSession(manager.defaultDevice().toLatin1(), this);
        connect(m_deviceSession, SIGNAL(sessionClosed()),
                SLOT(deviceSessionClosed()));
    }
}

void BluetoothObexPushServer::stop()
{
    qLog(Bluetooth) << "BluetoothObexPushServer: stopping...";
    if (m_server && m_server->isListening())
        close();
    if (!unregisterRecord(m_sdpRecordHandle))
        qLog(Bluetooth) << "BluetoothObexPushServer::stop() error unregistering SDP service";

    emit stopped();
}

void BluetoothObexPushServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_securityOptions = options;
    if (m_server && m_server->isListening())
        m_server->setSecurityOptions(options);
}

void BluetoothObexPushServer::newConnection()
{
    QBluetoothRfcommSocket *socket =
            qobject_cast<QBluetoothRfcommSocket*>(m_server->nextPendingConnection());
    if (!socket)
        return;
    m_activeSockets.insert(socket);
    connect(socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
    connect(socket, SIGNAL(destroyed(QObject*)), SLOT(socketDestroyed(QObject*)));

    m_numBtSessions++;
    if (m_numBtSessions == 1) { // First session
        qLog(Bluetooth) << "BluetoothObexPushServer: starting Bluetooth Session";
        m_deviceSession->startSession();
    }

    emit newConnection(socket);
}

void BluetoothObexPushServer::socketDestroyed(QObject *socket)
{
    m_numBtSessions--;
    qLog(Bluetooth) << "BluetoothObexPushServer: RFCOMM socket destroyed,"
            << "m_numBtSessions: " << m_numBtSessions;

    QBluetoothRfcommSocket *rfcomm = qobject_cast<QBluetoothRfcommSocket*>(socket);
    if (rfcomm)
        m_activeSockets.remove(rfcomm);

    if (m_numBtSessions == 0) {
        qLog(Bluetooth) << "BluetoothObexPushServer: ending Bluetooth session";
        m_deviceSession->endSession();
    }
}

void BluetoothObexPushServer::socketDisconnected()
{
    qLog(Bluetooth) << "BluetoothObexPushServer: RFCOMM socket disconnected";
    QBluetoothRfcommSocket *rfcomm = qobject_cast<QBluetoothRfcommSocket*>(sender());
    if (rfcomm) {
        m_activeSockets.remove(rfcomm);

        // make sure the OBEX service knows when socket has disconnected
        rfcomm->deleteLater();
    }
}

/*
    Called when BT device session has closed.
    This may be called as a result of the session being closed in socketDestroyed(),
    or as a result of the session being closed from elsewhere, e.g. if Bluetooth
    is turned off in Bluetooth settings.
*/
void BluetoothObexPushServer::deviceSessionClosed()
{
    qLog(Bluetooth) << "BluetoothObexPushServer: device session closed";
    QSetIterator<QBluetoothRfcommSocket *> i(m_activeSockets);
    while (i.hasNext())
        i.next()->disconnect();
}

//====================================================================


class BluetoothObexPushServicePrivate : public QObject
{
    Q_OBJECT
public:
    BluetoothObexPushServicePrivate(BluetoothObexPushService *parent);

    BluetoothObexPushService *m_parent;
    QHash<DefaultObexPushService*, int> m_services;
    QSet<int> m_abortedRequests;

public slots:
    void newConnection(QBluetoothRfcommSocket *socket);

private slots:
    void putRequested(const QString &name, const QString &type, qint64 size,
                const QString &description);
    void businessCardRequested();
    void dataTransferProgress(qint64 done, qint64 total);
    void requestFinished(bool error);
    void done(bool error);
};

BluetoothObexPushServicePrivate::BluetoothObexPushServicePrivate(BluetoothObexPushService *parent)
    : QObject(parent),
      m_parent(parent)
{
}

void BluetoothObexPushServicePrivate::newConnection(QBluetoothRfcommSocket *socket)
{
    DefaultObexPushService *service = new DefaultObexPushService(socket, this);
    connect(service, SIGNAL(putRequested(QString,QString,qint64,QString)),
            SLOT(putRequested(QString,QString,qint64,QString)));
    connect(service, SIGNAL(businessCardRequested()),
            SLOT(businessCardRequested()));
    connect(service, SIGNAL(dataTransferProgress(qint64,qint64)),
            SLOT(dataTransferProgress(qint64,qint64)));
    connect(service, SIGNAL(requestFinished(bool)),
            SLOT(requestFinished(bool)));
    connect(service, SIGNAL(done(bool)),
            SLOT(done(bool)));
}

void BluetoothObexPushServicePrivate::putRequested(const QString &name, const QString &type, qint64 size, const QString &description)
{
    qLog(Obex) << "BluetoothObexPushService: Put started:" << name << type
            << size << description;

    // a QObexPushService can only receive 1 file at a time,
    // we can generate a new ID for each new request that comes
    // from the service
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (service) {
        int id = m_parent->nextTransferId();
        m_services.insert(service, id);
        emit m_parent->incomingTransferStarted(id, name, type, description);
    }
}

void BluetoothObexPushServicePrivate::businessCardRequested()
{
    qLog(Obex) << "BluetoothObexPushService: businessCardRequested";

    DefaultObexPushService *service =
            qobject_cast<DefaultObexPushService*>(sender());
    if (service) {
        int id = m_parent->nextTransferId();
        m_services.insert(service, id);
        emit m_parent->outgoingTransferStarted(id, QString(),
                "text/x-vcard", tr("My business card"));
    }
}

void BluetoothObexPushServicePrivate::dataTransferProgress(qint64 done, qint64 total)
{
    qLog(Obex) << "BluetoothObexPushService: progress" << done << "/" << total;
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (service && m_services.contains(service)) {
        emit m_parent->transferProgress(m_services[service], done, total);
    }
}

void BluetoothObexPushServicePrivate::requestFinished(bool error)
{
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (!service)
        return;

    qLog(Obex) << "BluetoothObexPushService: request done, error?"
            << error << service->error();
    if (m_services.contains(service)) {
        // 'aborted' value is determined by whether abortTransfer() was
        // called, not whether the client sent an Abort request
        int id = m_services[service];
        bool aborted = (service->error() == DefaultObexPushService::Aborted) &&
                (m_abortedRequests.contains(id));
        if (!error && !aborted) {
            if (!service->finalizeDataTransfer())
                error = true;
        }

        emit m_parent->transferFinished(id, error, aborted);

        service->cleanUpRequest();
        m_abortedRequests.remove(id);
        m_services.remove(service);
    }
}

void BluetoothObexPushServicePrivate::done(bool error)
{
    qLog(Obex) << "BluetoothObexPushService: service done, error?" << error;
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (service) {
        m_services.remove(service); // in case requestFinished() wasn't called
        service->deleteLater();
    }
}


//===============================================================


/*!
    \class BluetoothObexPushService
    \inpublicgroup QtBluetoothModule
    \brief The BluetoothObexPushService class runs a Bluetooth OBEX Object Push server and provides notifications of received requests and their status.
    \ingroup QtopiaServer::Task::Bluetooth

    A unique ID is generated for each received request and is used to provide
    request updates through the incomingTransferStarted(),
    outgoingTransferStarted(), transferProgress() and transferFinished()
    signals.

    \bold{Note:} This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
*/


/*!
    Creates a service with parent object \a parent.
*/
BluetoothObexPushService::BluetoothObexPushService(QObject *parent)
    : FileTransferTask(parent),
      d(new BluetoothObexPushServicePrivate(this))
{
    qLog(Bluetooth) << "Starting BluetoothObexPushService...";

    QFile sdpRecordFile(Qtopia::qtopiaDir() + "etc/bluetooth/sdp/opp.xml");
    QBluetoothSdpRecord record = QBluetoothSdpRecord::fromDevice(&sdpRecordFile);
    if (record.isNull())
        qWarning() << "BluetoothObexPushService: cannot read" << sdpRecordFile.fileName();

    BluetoothObexPushServer *pushServer =
            new BluetoothObexPushServer(tr("OBEX Object Push"), record, this);
    connect(pushServer, SIGNAL(newConnection(QBluetoothRfcommSocket*)),
            d, SLOT(newConnection(QBluetoothRfcommSocket*)));
}

/*!
    \reimp
*/
QContentId BluetoothObexPushService::transferContentId(int id) const
{
    QHashIterator<DefaultObexPushService*, int> i(d->m_services);
    while (i.hasNext()) {
        if (i.next().value() == id) {
            return i.key()->currentContentId();
        }
    }
    return QContent::InvalidId;
}

/*!
    \reimp
*/
void BluetoothObexPushService::abortTransfer(int id)
{
    QHashIterator<DefaultObexPushService*, int> i(d->m_services);
    while (i.hasNext()) {
        if (i.next().value() == id) {
            d->m_abortedRequests.insert(id);
            i.key()->abort();

            // force disconnection if other side doesn't respond
            QTimer::singleShot(1000, i.key()->sessionDevice(), SLOT(deleteLater()));
            break;
        }
    }
}

/*!
    \fn void BluetoothObexPushService::incomingTransferStarted(int id, const QString &name, const QString &type, const QString &description)

    This signal is emitted when the service responds to an OBEX Put request.
    The \a id uniquely identifies the transfer, as provided by
    FileTransferTask::nextTransferId(). The \a name, \a type and \a description
    are set to the Name, Type and Description metadata values received in the
    request.
*/

/*!
    \fn void BluetoothObexPushService::outgoingTransferStarted(int id, const QString &name, const QString &type, const QString &description)

    This signal is emitted when the service responds to an OBEX Get request.
    (The request will be for the owner's business card, as OBEX Push servers
    cannot respond to any other types of Get requests.) The \a id uniquely
    identifies the transfer, as provided by FileTransferTask::nextTransferId().

    The \a name, \a type and \a description arguments are provided as a
    convenience for consistency with incomingTransferStarted(). \a name is set
    to an empty string, \a type is set to "text/x-vcard" and \a description is
    set to a user-friendly description of the business card request.
*/

/*!
    \fn void BluetoothObexPushService::transferProgress(int id, qint64 done, qint64 total)

    This signal is emitted to indicate the progress of the transfer identified
    by \a id. \a done is the amount of data that has already been sent, and
    \a total is the total amount of data to be sent. If the total amount
    cannot be determined, \a total is set to 0.
*/

/*!
    \fn void BluetoothObexPushService::transferFinished(int id, bool error, bool aborted)

    This signal is emitted when the transfer identified by \a id is finished.
    \a error is \c true if the transfer failed, and \a aborted is
    \c true if the transfer was aborted by a call to abortRequest().
*/


QTOPIA_TASK(BluetoothObexPushService, BluetoothObexPushService);
QTOPIA_TASK_PROVIDES(BluetoothObexPushService, FileTransferTask);

#include "bluetoothobexpushservice.moc"
