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

#include "obexpushrequestsender.h"
#include "sessionedobexclient.h"
#include "qabstractmessagebox.h"

#include <qobexheader.h>
#include <qdsactionrequest.h>
#include <qcontent.h>
#include <qcontact.h>
#include <qcontactmodel.h>
#include <qdsdata.h>
#include <qtopialog.h>

#include <QBuffer>
#include <QFile>
#include <QHash>
#include <QDataStream>


class PushRequest
{
public:
    PushRequest(int id, QIODevice *device);
    ~PushRequest();

    QString name;
    QString type;
    QString description;
    QContentId contentId;
    QPointer<QDSActionRequest> qdsActionRequest; // deleted on destruction

    inline int id() const { return m_id; }
    inline QIODevice *device() const { return m_device; }

    inline void setAutoDeleteFile(bool autoDelete) { m_autoDeleteFile = autoDelete; }

private:
    int m_id;
    QIODevice *m_device;
    bool m_autoDeleteFile;
};

// takes ownership of given device
PushRequest::PushRequest(int id, QIODevice *device)
    : contentId(QContent::InvalidId),
      m_id(id),
      m_device(device),
      m_autoDeleteFile(false)
{
}

PushRequest::~PushRequest()
{
    if (m_device) {
        m_device->close();
        if (m_autoDeleteFile) {
            QFile *file = qobject_cast<QFile *>(m_device);
            if (file) {
                bool b = file->remove();
                qLog(Obex) << "ObexPushRequestSender auto-deleted sent file:" << b;
            }
        }
        delete m_device;
        m_device = 0;
    }

    if (qdsActionRequest) {
        delete qdsActionRequest;
        qdsActionRequest = 0;
    }
}

//==================================================

class ObexPushRequestSenderPrivate : public QObject
{
    Q_OBJECT
public:
    ObexPushRequestSenderPrivate(ObexPushRequestSender *parent);
    ~ObexPushRequestSenderPrivate();

    void performRequest(PushRequest *request, SessionedObexClient *client);

    bool abortRequest(int id);
    QContentId requestContentId(int id) const;

private slots:
    void requestStarted();
    void dataSendProgress(qint64 done, qint64 total);
    void requestFinished(bool error, bool aborted);

private:
    ObexPushRequestSender *m_parent;
    QHash<SessionedObexClient*, PushRequest*> m_requests;
};

ObexPushRequestSenderPrivate::ObexPushRequestSenderPrivate(ObexPushRequestSender *parent)
    : QObject(parent),
      m_parent(parent)
{
}

ObexPushRequestSenderPrivate::~ObexPushRequestSenderPrivate()
{
    // delete any stored PushRequest* values
    QHashIterator<SessionedObexClient*, PushRequest*> i(m_requests);
    while (i.hasNext())
        delete i.next().value();
}

void ObexPushRequestSenderPrivate::performRequest(PushRequest *request, SessionedObexClient *client)
{
    // This method may be called again before a previous request has
    // finished.

    qLog(Obex) << "ObexPushRequestSender: starting Push request:"
            << request->id() << request->name << request->type;

    if (!client) {
        qLog(Obex) << "ObexPushRequestSender: internal error (given null client)";
        delete request;
        return;
    }

    connect(client, SIGNAL(requestStarted()),
            SLOT(requestStarted()));
    connect(client, SIGNAL(dataSendProgress(qint64,qint64)),
            SLOT(dataSendProgress(qint64,qint64)));
    connect(client, SIGNAL(requestFinished(bool,bool)),
            SLOT(requestFinished(bool,bool)));
    connect(client, SIGNAL(done()),
            client, SLOT(deleteLater()));   // delete when done
                // TODO just deleteLater() from requestFinished instead?

    m_requests.insert(client, request);

    QObexHeader header;
    header.setName(QFileInfo(request->name).fileName());    // remove path
    header.setType(request->type);
    header.setDescription(request->description);
    QIODevice *device = request->device();
    if ( device && (device->isOpen() || device->open(QIODevice::ReadOnly)) ) {
        if (!device->isSequential())
            header.setLength(device->size());
    }
    client->begin(header, request->device());
}

bool ObexPushRequestSenderPrivate::abortRequest(int id)
{
    qLog(Obex) << "ObexPushRequestSender: abort request" << id;

    QList<SessionedObexClient *> clients = m_requests.keys();
    for (int i=0; i<clients.size(); i++) {
        if (m_requests[clients[i]]->id() == id) {
            qLog(Obex) << "ObexPushRequestSender found push client, calling abort()";
            clients[i]->abort();
            return true;
        }
    }
    return false;
}

QContentId ObexPushRequestSenderPrivate::requestContentId(int id) const
{
    QList<SessionedObexClient *> clients = m_requests.keys();
    for (int i=0; i<clients.size(); i++) {
        if (m_requests[clients[i]]->id() == id) {
            return m_requests[clients[i]]->contentId;
        }
    }
    return QContent::InvalidId;
}

void ObexPushRequestSenderPrivate::requestStarted()
{
    qLog(Obex) << "ObexPushRequestSender: request started";

    SessionedObexClient *client = qobject_cast<SessionedObexClient*>(sender());
    if (!client || !m_requests.contains(client)) {
        qLog(Obex) << "ObexPushRequestSender: internal error (client not found)";
        return;
    }

    const PushRequest *request = m_requests[client];
    emit m_parent->requestStarted(request->id(), request->name,
            request->type, request->description);
}

void ObexPushRequestSenderPrivate::dataSendProgress(qint64 done, qint64 total)
{
    qLog(Obex) << "ObexPushRequestSender: sent"
            << done << "/" << total;

    SessionedObexClient *client = qobject_cast<SessionedObexClient*>(sender());
    if (!client || !m_requests.contains(client)) {
        qLog(Obex) << "ObexPushRequestSender: internal error (client not found)";
        return;
    }

    emit m_parent->requestProgress(m_requests[client]->id(), done, total);
}

void ObexPushRequestSenderPrivate::requestFinished(bool error, bool aborted)
{
    qLog(Obex) << "ObexPushRequestSender: done request"
            << error << aborted;

    SessionedObexClient *client = qobject_cast<SessionedObexClient*>(sender());
    if (!client || !m_requests.contains(client)) {
        qLog(Obex) << "ObexPushRequestSender: internal error (client not found)";
        return;
    }

    PushRequest *request = m_requests[client];

    if (request->qdsActionRequest) {
        if (aborted)
            request->qdsActionRequest->respond(tr("Transmission canceled"));
        else if (error)
            request->qdsActionRequest->respond(tr("Transmission error"));
        else
            request->qdsActionRequest->respond();
    }

    emit m_parent->requestFinished(request->id(), error, aborted);
    delete m_requests.take(client);
}


//==================================================

/*!
    \class ObexPushRequestSender
    \inpublicgroup QtInfraredModule
    \inpublicgroup QtBluetoothModule
    \brief The ObexPushRequestSender class provides convenience methods for sending a variety of OBEX Push requests and also provides updates regarding the progress of each request.
    \ingroup QtopiaServer

    ObexPushRequestSender sends files that are commonly transferred using
    the OBEX Object Push Profile, including business cards (vCards),
    calendars and ordinary files.

    A unique ID is generated for each request and is used to provide request
    updates through the requestStarted(), requestProgress() and
    requestFinished() signals. Requests can be canceled using abortRequest().

    This class uses SessionedObexClient to send each OBEX request
    inside of a device session for a particular hardware device. This ensures
    the Qt Extended device manager will try to keep the device open while a request
    is in progress.

    This class enables the BluetoothFileSendService and the IrFileSendService.

    This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
*/

/*!
    Constructs a sender with parent object \a parent.
*/
ObexPushRequestSender::ObexPushRequestSender(QObject *parent)
    : QObject(parent),
      d(new ObexPushRequestSenderPrivate(this))
{
}

/*!
    Sends the file stored at \a filePath using \a client. The specified
    \a mimeType and \a description (both optional) will be sent in the
    request to provide additional metadata for the file. Set \a autoDeleteFile
    to \c true if the file stores at \a filePath should be deleted after it
    has been sent.

    The specified \a id will be used to emit the requestStarted(),
    requestProgress() and requestFinished() signals for this request.

    This sender takes ownership of \a client and deletes it when the
    request is finished.

    \sa createFile()
*/
void ObexPushRequestSender::sendFile(SessionedObexClient *client, int id, const QString &filePath, const QString &mimeType, const QString &description, bool autoDeleteFile)
{
    qLog(Obex) << "ObexPushRequestSender::sendFile()";

    QFile *file = createFile(filePath);
    if (!file) {
        qLog(Obex) << "Can't send file, ObexPushRequestSender::createFile() returned null QFile";
        return;
    }

    PushRequest *request = new PushRequest(id, file);
    request->name = filePath;
    request->type = mimeType;
    request->description = description;
    request->setAutoDeleteFile(autoDeleteFile);

    d->performRequest(request, client);
}

/*!
    Sends the document identified by \a contentId using \a client.

    The specified \a id will be used to emit the requestStarted(),
    requestProgress() and requestFinished() signals for this request.

    This sender takes ownership of \a client and deletes it when the
    request is finished.

    \sa createDevice()
 */
void ObexPushRequestSender::sendContent(SessionedObexClient *client, int id, const QContentId &contentId)
{
    qLog(Obex) << "ObexPushRequestSender::sendContent()";

    QIODevice *device = createDevice(contentId);
    if (!device) {
        qLog(Obex) << "Can't send QContent document, ObexPushRequestSender::createDevice() returned null QIODevice";
        return;
    }

    QContent content(contentId);
    PushRequest *request = new PushRequest(id, device);
    request->contentId = contentId;
    request->name = content.fileName();
    request->type = content.type();
    request->description = content.name();

    d->performRequest(request, client);
}

/*!
    Sends the user's personal business card using \a client. If the user has
    not set a personal business card, the user will be notified appropriately.

    The specified \a id will be used to emit the requestStarted(),
    requestProgress() and requestFinished() signals for this request.

    This sender takes ownership of \a client and deletes it when the
    request is finished.

    \sa sendBusinessCard(), ownerBusinessCard()
*/
void ObexPushRequestSender::sendPersonalBusinessCard(SessionedObexClient *client, int id)
{
    qLog(Obex) << "ObexPushRequestSender::sendPersonalBusinessCard()";

    bool hasBusinessCard = false;
    QContact contact = ownerBusinessCard(&hasBusinessCard);
    if (!hasBusinessCard) {
        qLog(Obex) << "ObexPushRequestSender: no owner business card to send";
        return;
    }
    sendBusinessCard(client, id, contact);
}

/*!
    Sends the business card represented by \a contact using \a client.

    The specified \a id will be used to emit the requestStarted(),
    requestProgress() and requestFinished() signals for this request.

    This sender takes ownership of \a client and deletes it when the
    request is finished.

    \sa sendPersonalBusinessCard()
*/
void ObexPushRequestSender::sendBusinessCard(SessionedObexClient *client, int id, const QContact &contact)
{
    qLog(Obex) << "ObexPushRequestSender::sendBusinessCard(QContact)";

    QIODevice *device = createDevice(contact);
    if (!device) {
        qLog(Obex) << "Can't send business card, ObexPushRequestSender::createDevice() returned null device";
        return;
    }

    PushRequest *request = new PushRequest(id, device);
    request->name = contact.nickname();
    request->type = "text/x-vcard";

    d->performRequest(request, client);
}

/*!
    Sends the business card represented by \a actionRequest using \a client. The
    request object should contain raw serialized QContact data.

    The specified \a id will be used to emit the requestStarted(),
    requestProgress() and requestFinished() signals for this request.

    This sender takes ownership of \a client and deletes it when the
    request is finished.

    \sa sendPersonalBusinessCard()
*/
void ObexPushRequestSender::sendBusinessCard(SessionedObexClient *client, int id, const QDSActionRequest &actionRequest)
{
    qLog(Obex) << "ObexPushRequestSender::sendBusinessCard(QDSActionRequest)";

    QBuffer *buffer = new QBuffer;
    buffer->setData(actionRequest.requestData().data());
    buffer->open(QIODevice::ReadOnly);

    PushRequest *request = new PushRequest(id, buffer);
    request->name = "BusinessCard.vcf";
    request->type = "text/x-vcard";
    request->qdsActionRequest = new QDSActionRequest(actionRequest);

    d->performRequest(request, client);
}

/*!
    Sends the vCalendar object represented by \a actionRequest using \a client.  The
    request object should contain raw serialized QTask or QAppointment data.

    The specified \a id will be used to emit the requestStarted(),
    requestProgress() and requestFinished() signals for this request.

    This sender takes ownership of \a client and deletes it when the
    request is finished.
 */
void ObexPushRequestSender::sendCalendar(SessionedObexClient *client, int id, const QDSActionRequest &actionRequest)
{
    qLog(Obex) << "ObexPushRequestSender::sendCalendar()";

    QBuffer *buffer = new QBuffer;
    buffer->setData(actionRequest.requestData().data());
    buffer->open(QIODevice::ReadOnly);

    PushRequest *request = new PushRequest(id, buffer);
    request->name = "vcal.vcs";
    request->type = "text/x-vcalendar";
    request->qdsActionRequest = new QDSActionRequest(actionRequest);

    d->performRequest(request, client);
}

/*!
    Aborts the request identified by \a id.

    If the request is successfully aborted, requestFinished() is emitted
    with \c aborted set to \c true. Due to timing issues, it is possible that
    the request has already finished. In this case, the request is not aborted,
    and requestFinished() will be emitted as normal.

    Returns \c true if \a id references a request that is in progress; that
    is, requestFinished() has not yet been emitted for the request. Otherwise,
    this method does nothing, and returns \c false.
*/
bool ObexPushRequestSender::abortRequest(int id)
{
    return d->abortRequest(id);
}

/*!
    Returns the QContent ID for the request identified by \a id if the
    request was started by a call to sendContent(). Otherwise, returns
    QContent::InvalidId.
*/
QContentId ObexPushRequestSender::requestContentId(int id) const
{
    return d->requestContentId(id);
}

/*!
    Returns an opened QIODevice that can read from the contents of the
    resource identified by \a contentId. Returns 0 if there was an error.

    The default implementation displays an error message if the resource is
    not accessible for reading.

    This sender takes ownership of the returned device and deletes it
    when the request is finshed.
*/
QIODevice *ObexPushRequestSender::createDevice(const QContentId &contentId) const
{
    QContent content(contentId);
    QIODevice *device = content.open(QIODevice::ReadOnly);
    if (!device) {
        QAbstractMessageBox::warning(0, tr("File Error"),
                tr("<P>The file you are trying to send could not be opened."));
        return 0;
    }
    return device;
}

/*!
    Returns an opened QIODevice that can read the raw vCard data for the
    business card specified by \a contact. Returns 0 if there was an error.

    The default implementation displays an error message if the contact data
    cannot be processed.

    This sender takes ownership of the returned device and deletes it
    when the request is finshed.
*/
QIODevice *ObexPushRequestSender::createDevice(const QContact &contact) const
{
    QBuffer *buffer = new QBuffer;
    buffer->open(QIODevice::ReadWrite);
    if (!QContact::writeVCard(buffer, contact)) {
        QAbstractMessageBox::warning(0, tr("File Error"),
                tr("<P>The business card you are trying to send could not be processed"));
        delete buffer;
        return 0;
    }
    buffer->seek(0);
    return buffer;
}

/*!
    Returns an opened file that can read from the contents of the file at
    \a filePath. Returns 0 if there was an error.

    The default implementation displays an error message if the file does not
    exist or cannot be opened.

    This sender takes ownership of the returned file and deletes it
    when the request is finshed.
*/
QFile *ObexPushRequestSender::createFile(const QString &filePath) const
{
    QFile *file = new QFile(filePath);
    if (!file->exists() || !file->open(QIODevice::ReadOnly)) {
        QAbstractMessageBox::warning(0, tr("File Error"),
                tr("<P>The file you are trying to send could not be opened."));
        delete file;
        return 0;
    }
    return file;
}

/*!
    Returns a QContact object that contains the details of the owner's
    business card. \a hasBusinessCard is set to \c true if the owner's
    business card is available, otherwise it is set to \c false.

    The default implementation returns the result of
    QContactModel::personalDetails(). The owner's business card is determined
    to be available if QContactModel::hasPersonalDetails() returns \c true.
*/
QContact ObexPushRequestSender::ownerBusinessCard(bool *hasBusinessCard) const
{
    QContactModel model;
    if (!model.hasPersonalDetails()) {
        QAbstractMessageBox::warning(0, tr("Transfer Error"),
                tr("<P>You have not set a personal business card. "
                   "Please set a personal business card in the Contacts application."));
        *hasBusinessCard = false;
        return QContact();
    }

    *hasBusinessCard = true;
    return model.personalDetails();
}

/*!
    \fn void ObexPushRequestSender::requestStarted(int id, const QString &name, const QString &mimeType, const QString &description)

    This signal is emitted when an OBEX request begins. The \a id uniquely
    identifies the request for this sender. The \a name, \a mimeType and
    \a description are respectively set to the Name, Type and Description
    metadata values that will be sent in the request.
*/

/*!
    \fn void ObexPushRequestSender::requestProgress(int id, qint64 done, qint64 total)

    This signal is emitted to indicate the progress of the request identified
    by \a id. \a done is the amount of data that has already been sent, and
    \a total is the total amount of data to be sent. If the total amount
    cannot be determined, \a total is set to 0.
*/

/*!
    \fn void ObexPushRequestSender::requestFinished(int id, bool error, bool aborted)

    This signal is emitted when the request identified by \a id is finished.
    \a error is \c true if the request has failed, and \a aborted is
    \c true if the request was aborted following a call to abortRequest().
*/

#include "obexpushrequestsender.moc"
