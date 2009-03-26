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

#include "irobexpushservice.h"
#include "defaultobexpushservice.h"

#include <qirlocaldevice.h>
#include <qirserver.h>
#include <qirnamespace.h>
#include <qirsocket.h>
#include <qirremotedevice.h>

#include <qcommdevicesession.h>
#include <qtopialog.h>

#include <QStringList>
#include <QSet>
#include <QTimer>


class IrObexPushServer : public QObject
{
    Q_OBJECT
public:
    IrObexPushServer(QObject *parent = 0);

signals:
    void newConnection(QIrSocket *socket);

private slots:
    void newConnection();
    void socketDisconnected();

private:
    QCommDeviceSession *m_irSession;
    QIrServer *m_server;
    int m_numIrSessions;
};


IrObexPushServer::IrObexPushServer(QObject *parent)
    : QObject(parent),
      m_irSession(0),
      m_server(0),
      m_numIrSessions(0)
{
    qLog(Infrared) << "IrObexPushServer: starting...";

    QStringList irDevices = QIrLocalDevice::devices();
    qLog(Infrared) << "IrObexPushServer: found local Infrared devices:" << irDevices;

    if (irDevices.size() > 0) {
        m_server = new QIrServer(this);
        m_server->listen("OBEX:IrXfer", QIr::OBEX | QIr::Telephony);
        connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));
    }
    else {
        qLog(Infrared) << "No IRDA devices found!";
    }
}

void IrObexPushServer::newConnection()
{
    QIrSocket *socket = m_server->nextPendingConnection();
    if (!socket)
        return;

    qLog(Infrared) << "IrObexPushServer: new infared connection from"
            << socket->remoteAddress();

    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    m_numIrSessions++;
    qLog(Infrared) << "IrObexPushServer: num IR Sessions:" << m_numIrSessions;
    if (m_numIrSessions == 1) { // First session
        // We don't need to receive the notification of session start here
        // as that just notifies us that the device is up.  We would not be receiving
        // connections here if the device wasn't up though
        qLog(Infrared) << "IrObexPushServer: starting IR Session";
        m_irSession = QCommDeviceSession::session("irda0");
        qLog(Infrared) << "Got session:" << m_irSession;
    }

    emit newConnection(socket);
}

void IrObexPushServer::socketDisconnected()
{
    // make sure the OBEX service knows the socket has disconnected
    QIrSocket *socket = qobject_cast<QIrSocket*>(sender());
    if (socket)
        socket->deleteLater();

    m_numIrSessions--;
    qLog(Infrared) << "IrObexPushServer: socket disconnected, numIrSessions:"
            << m_numIrSessions;

    if (m_numIrSessions == 0) {
        qLog(Infrared) << "IrObexPushServer: ending Infrared session";
        if (m_irSession) {
            m_irSession->endSession();
            delete m_irSession;
            m_irSession = 0;
        }
    }
}


//====================================================================


class IrObexPushServicePrivate : public QObject
{
    Q_OBJECT
public:
    IrObexPushServicePrivate(IrObexPushService *parent);

    IrObexPushService *m_parent;
    QHash<DefaultObexPushService*, int> m_services;
    QSet<int> m_abortedRequests;

public slots:
    void newConnection(QIrSocket *socket);

private slots:
    void putRequested(const QString &name, const QString &type, qint64 size,
                const QString &description);
    void businessCardRequested();
    void dataTransferProgress(qint64 done, qint64 total);
    void requestFinished(bool error);
    void done(bool error);
};

IrObexPushServicePrivate::IrObexPushServicePrivate(IrObexPushService *parent)
    : QObject(parent),
      m_parent(parent)
{
}

void IrObexPushServicePrivate::newConnection(QIrSocket *socket)
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

void IrObexPushServicePrivate::putRequested(const QString &name, const QString &type, qint64 size, const QString &description)
{
    qLog(Obex) << "IrObexPushService: Put started:" << name << type
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

void IrObexPushServicePrivate::businessCardRequested()
{
    qLog(Obex) << "IrObexPushService: businessCardRequested";

    DefaultObexPushService *service =
            qobject_cast<DefaultObexPushService*>(sender());
    if (service) {
        int id = m_parent->nextTransferId();
        m_services.insert(service, id);
        emit m_parent->outgoingTransferStarted(id, QString(),
                "text/x-vcard", tr("My business card"));
    }
}

void IrObexPushServicePrivate::dataTransferProgress(qint64 done, qint64 total)
{
    qLog(Obex) << "IrObexPushService: progress" << done << "/" << total;
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (service && m_services.contains(service)) {
        emit m_parent->transferProgress(m_services[service], done, total);
    }
}

void IrObexPushServicePrivate::requestFinished(bool error)
{
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (!service)
        return;

    qLog(Obex) << "IrObexPushService: request done, error?"
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

void IrObexPushServicePrivate::done(bool error)
{
    qLog(Obex) << "IrObexPushService: service done, error?" << error;
    DefaultObexPushService *service = qobject_cast<DefaultObexPushService*>(sender());
    if (service) {
        m_services.remove(service); // in case requestFinished() wasn't emitted
        service->deleteLater();
    }
}

//===============================================================

/*!
    \class IrObexPushService
    \inpublicgroup QtInfraredModule
    \brief The IrObexPushService class runs an Infrared OBEX Object Push server and provides notifications of received requests and their status.
    \ingroup QtopiaServer::Task

    A unique ID is generated for each received request and is used to provide
    request updates through the requestStarted(), requestProgress() and
    requestFinished() signals.

    \bold{Note:} This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
*/


/*!
    Creates a service with parent object \a parent.
*/
IrObexPushService::IrObexPushService(QObject *parent)
    : FileTransferTask(parent),
      d(new IrObexPushServicePrivate(this))
{
    qLog(Obex) << "Starting IrObexPushService...";

    IrObexPushServer *pushServer = new IrObexPushServer(this);
    connect(pushServer, SIGNAL(newConnection(QIrSocket*)),
            d, SLOT(newConnection(QIrSocket*)));
}

/*!
    \reimp
*/
QContentId IrObexPushService::transferContentId(int id) const
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
void IrObexPushService::abortTransfer(int id)
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
    \fn void IrObexPushService::incomingTransferStarted(int id, const QString &name, const QString &type, const QString &description)

    This signal is emitted when the service responds to an OBEX Put request.
    The \a id uniquely identifies the transfer, as provided by
    FileTransferTask::nextTransferId(). The \a name, \a type and \a description
    are set to the Name, Type and Description metadata values received in the
    request.
*/

/*!
    \fn void IrObexPushService::outgoingTransferStarted(int id, const QString &name, const QString &type, const QString &description)

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
    \fn void IrObexPushService::transferProgress(int id, qint64 done, qint64 total)

    This signal is emitted to indicate the progress of the transfer identified
    by \a id. \a done is the amount of data that has already been sent, and
    \a total is the total amount of data to be sent. If the total amount
    cannot be determined, \a total is set to 0.
*/

/*!
    \fn void IrObexPushService::transferFinished(int id, bool error, bool aborted)

    This signal is emitted when the transfer identified by \a id is finished.
    \a error is \c true if the transfer failed, and \a aborted is
    \c true if the transfer was aborted by a call to abortRequest().
*/


QTOPIA_TASK(IrObexPushService, IrObexPushService);
QTOPIA_TASK_PROVIDES(IrObexPushService, FileTransferTask);

#include "irobexpushservice.moc"
