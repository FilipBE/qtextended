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

#include "defaultobexpushservice.h"

#include <qtopiaipcenvelope.h>
#include <qcontent.h>
#include <qappointment.h>
#include <qtask.h>
#include <qtopiaservices.h>
#include <qmimetype.h>
#include <qtopialog.h>
#include <qcontact.h>
#include <qcontactmodel.h>
#include <qtopiaservices.h>
#include <qphoneprofile.h>
#include <qtopiaapplication.h>

#include <QTemporaryFile>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QFileSystem>
#include <QTimer>

#include <sys/vfs.h>


static QString pretty_print_size(qint64 fsize)
{
    static const char *size_suffix[] = {
        QT_TRANSLATE_NOOP("CustomPushService", "B"),
        QT_TRANSLATE_NOOP("CustomPushService", "KB"),
        QT_TRANSLATE_NOOP("CustomPushService", "MB"),
        QT_TRANSLATE_NOOP("CustomPushService", "GB"),
    };

    double max = fsize;

    int i = 0;
    for (; i < 4; i++) {
        if (max > 1024.0) {
            max /= 1024.0;
        }
        else {
            break;
        }
    }

    // REALLY big file?
    if (i == 4)
        i = 0;

    if (fsize < 1024) {
        return QString::number(fsize) + qApp->translate("CustomPushService", size_suffix[i]);
    } else {
        return QString::number(max, 'f', 2)
                + qApp->translate("CustomPushService", size_suffix[i]);
    }
}


//=================================================================

class PutRequestHandler : public QObject
{
    Q_OBJECT
public:
    PutRequestHandler(QObject *parent = 0);

    virtual QIODevice *handleRequestStarted(const QString &name,
            const QString &type, qint64 size, const QString &description) = 0;
    virtual bool finalizeDataTransfer() = 0;

public slots:
    virtual void disconnected();
};


PutRequestHandler::PutRequestHandler(QObject *parent)
    : QObject(parent)
{
}

void PutRequestHandler::disconnected()
{
}

//=================================================================

class VObjectPutRequestHandler : public PutRequestHandler
{
    Q_OBJECT
public:
    VObjectPutRequestHandler(QObject *parent = 0);
    ~VObjectPutRequestHandler();

    virtual QIODevice *handleRequestStarted(const QString &name,
            const QString &type, qint64 size, const QString &description);
    virtual bool finalizeDataTransfer();

private:
    QString getIpcChannel(const QString &service, const QMimeType &mimeType);
    void vcalInfo(const QString &filename, bool *todo, bool *cal);

    QString m_type;
    QTemporaryFile *m_file;
    bool m_handled;
};


VObjectPutRequestHandler::VObjectPutRequestHandler(QObject *parent)
    : PutRequestHandler(parent),
      m_file(0),
      m_handled(false)
{
}

VObjectPutRequestHandler::~VObjectPutRequestHandler()
{
    if (m_file && !m_handled)
        m_file->remove();
    delete m_file;
}

QIODevice *VObjectPutRequestHandler::handleRequestStarted(const QString &/*name*/, const QString &type, qint64, const QString &)
{
    qLog(Obex) << "VObjectPutRequestHandler: creating temp file for vCard/vCal";

    m_type = type;
    m_file = new QTemporaryFile(Qtopia::tempDir());
    m_file->setAutoRemove(false);   // Contacts app needs to read file later

    if (!m_file->open()) {
        qWarning("OBEX Push service error: cannot open temp file for writing");
        return 0;
    }
    return m_file;
}

bool VObjectPutRequestHandler::finalizeDataTransfer()
{
    qLog(Obex) << "VObjectPutRequestHandler: figuring out where to send"
            << m_type << "file";

    if (!m_file)
        return false;
    m_handled = false;

    QMimeType mimeType(m_type);
    QString service = "Receive/" + mimeType.id();
    QString channel = getIpcChannel(service, mimeType);

    if (channel.isEmpty()) {
        qLog(Obex) << "VObjectPutRequestHandler: no channel found for" << m_type;
        m_handled = false;
    } else {
        // Send immediately
        qLog(Obex) << "VObjectPutRequestHandler: sending QtopiaIpcEnvelope to"
                << channel << service;
        QContent lnk(QtopiaService::app(service));
        QtopiaIpcEnvelope e(channel, "receiveData(QString,QString)");
        e << m_file->fileName() << mimeType.id();
        m_handled = true;
    }

    delete m_file;
    m_file = 0;
    return m_handled;
}

QString VObjectPutRequestHandler::getIpcChannel(const QString &service, const QMimeType &mimeType)
{
    QString receiveChannel = QtopiaService::channel(service);
    if (receiveChannel.isEmpty()) {
        // Special cases...
        // ##### should split file, or some other full fix
        if (mimeType.id().toLower() == "text/x-vcalendar") {
            bool calendar, todo;
            vcalInfo(m_file->fileName(), &todo, &calendar);
            if (todo)
                receiveChannel = QtopiaService::channel(service + "-Tasks");
            else
                receiveChannel = QtopiaService::channel(service + "-Events");
        }
    }
    return receiveChannel;
}

void VObjectPutRequestHandler::vcalInfo(const QString &filename, bool *todo, bool *cal)
{
    *cal = *todo = false;

    QList<QAppointment> events = QAppointment::readVCalendar(filename);
    if (events.count())
        *cal = true;

    QList<QTask> tasks = QTask::readVCalendar( filename );
    if (tasks.count())
        *todo = true;
}

//=================================================================

class GenericPutRequestHandler : public PutRequestHandler
{
    Q_OBJECT
public:
    GenericPutRequestHandler(QObject *parent = 0);
    ~GenericPutRequestHandler();

    virtual QIODevice *handleRequestStarted(const QString &name,
            const QString &type, qint64 size, const QString &description);
    virtual bool finalizeDataTransfer();

    inline QContent *content() const { return m_content; }

public slots:
    virtual void disconnected();

private slots:
    void stopMessageRingtone();

private:
    QMessageBox *m_msgBox;
    QContent *m_content;
    QIODevice *m_device;
};


GenericPutRequestHandler::GenericPutRequestHandler(QObject *parent)
    : PutRequestHandler(parent),
      m_msgBox(new QMessageBox(0)),
      m_content(0),
      m_device(0)
{
    m_msgBox->setWindowTitle(tr("Accept file?"));
    m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_msgBox->setDefaultButton(QMessageBox::No);
    m_msgBox->setWindowModality(Qt::WindowModal);
}

GenericPutRequestHandler::~GenericPutRequestHandler()
{
    if (m_content) {
        // finalizeDataTransfer() was not called? Remove from content system.
        if (m_content->role() == QContent::Data)
            m_content->removeFiles();
    }

    delete m_msgBox;
    delete m_content;
    delete m_device;
}

QIODevice *GenericPutRequestHandler::handleRequestStarted(const QString &name, const QString &mimeType, qint64 size, const QString &description)
{
    qLog(Obex) << "GenericPutRequestHandler: incoming file" << name;

    qint64 availableStorage = 0;

    struct statfs fs;
    QString savePath = QFileSystem::documentsFileSystem().documentsPath();
    if (statfs( savePath.toLocal8Bit(), &fs ) == 0) {
        availableStorage = fs.f_bavail * fs.f_bsize;
        if (size >= availableStorage) {
            qWarning("OBEX Push service error: not enough disk space for incoming file, refusing Put request");
            return 0;
        }
    }

    // brighten the screen
    QtopiaServiceRequest e1("QtopiaPowerManager", "setBacklight(int)");
    e1 << -1; // read brightness setting from config
    e1.send();

    // wake up (otherwise the next keypress is swallowed even though the
    // screen has been brightened)
    QtopiaServiceRequest e2("QtopiaPowerManager", "setActive(bool)");
    e2 << false;
    e2.send();

    // sound a message tone
    QtopiaServiceRequest e3("Ringtone", "startMessageRingtone()");
    e3.send();
    QPhoneProfileManager manager;
    QPhoneProfile profile = manager.activeProfile();
    QTimer::singleShot(profile.msgAlertDuration(), this,
            SLOT(stopMessageRingtone()));

    QString baseFileName = QFileInfo(name).fileName();  // strip path
    QString fileDesc = ( description.isEmpty() ? baseFileName : description );

    QString msgText;
    if (size > 0) {
        msgText = tr("Accept incoming file <b>%1</b> of size %2?",
                     "%1=filename %2=filesize").arg(fileDesc).arg(pretty_print_size(size));
    } else {
        msgText = tr("Accept incoming file <b>%1</b>?", "%1=filename").arg(fileDesc);
    }
    if (availableStorage > 0) {
        msgText += QChar(' ') + tr(" You have %1 disk space remaining.",
                      "%1=size").arg(pretty_print_size(availableStorage));
    }
    m_msgBox->setText(msgText);

    // stop ringtone if msg box has been accepted/rejected
    int result = QtopiaApplication::execDialog(m_msgBox);
    stopMessageRingtone();

    if (result != QMessageBox::Yes)
        return 0;

    // create QContent, but don't commit it until request is done
    m_content = new QContent;
    m_content->setType(mimeType);
    m_content->setName(baseFileName);

    // don't show file in Documents list until finalizeDataTransfer() is called
    m_content->setRole(QContent::Data);

    m_device = m_content->open(QIODevice::WriteOnly);
    if (!m_device) {
        qWarning("OBEX Push service error: cannot open file for writing");
        delete m_content;
        m_content = 0;
        return 0;
    }

    // If there's a Description value, use that as the QContent name instead
    // of using the Name value, cos it's probably a more user-friendly name
    // whereas the Name is probably a filename.
    // Must do this after QContent::open() so that the file has already been
    // created under the filename in the Name value.
    if (!description.isEmpty())
        m_content->setName(description);

    qLog(Obex) << "GenericPutRequestHandler: will save file to"
            << m_content->fileName();
    return m_device;
}

void GenericPutRequestHandler::disconnected()
{
    qLog(Obex) << "GenericPutRequestHandler: lost connection";
    if (m_msgBox->isVisible()) {
        // might be disconnected while the "Accept file?" dialog is showing
        m_msgBox->reject();
    }
}

bool GenericPutRequestHandler::finalizeDataTransfer()
{
    if (!m_content)
        return false;

    // make visible in Documents list
    m_content->setRole(QContent::Document);

    bool saved = m_content->commit();
    if (!saved)
        m_content->removeFiles();

    qLog(Obex) << "GenericPutRequestHandler: saved received file OK?"
            << saved;
    return saved;
}

void GenericPutRequestHandler::stopMessageRingtone()
{
    QtopiaServiceRequest e("Ringtone", "stopMessageRingtone()");
    e.send();
}

//=================================================================

struct DefaultObexPushServicePrivate
{
    PutRequestHandler *requestHandler;
};


/*!
    \class DefaultObexPushService
    \inpublicgroup QtInfraredModule
    \inpublicgroup QtBluetoothModule
    \brief The DefaultObexPushService class provides an OBEX Push server.
    \ingroup QtopiaServer

    The DefaultObexPushService provides a default implementation of an OBEX
    Push server. It handles Put requests in the following manner:

    \list
    \o If a vCard or vCalendar file is received, the file will automatically
    be accepted. When finalizeDataTransfer() is called, the file will be sent
    to the appropriate Qt Extended application.
    \o If any other type of file is received, the user will be prompted to
    confirm whether the file should be accepted. If the file is accepted, once
    finalizeDataTransfer() is called, the file will be saved into the
    Document System.
    \endlist

    If the server receives a Get request for the owner's business card,
    the server will respond with the contents of
    QContactModel::personalDetails().

    This class enables the BluetoothObexPushService and the IrObexPushService.

    This class is part of the Qt Extended server and cannot be used
    by other Qt Extended applications.
*/

/*!
    Constructs a Push server that will run on the specified \a socket and
    parent object \a parent.
*/
DefaultObexPushService::DefaultObexPushService(QIODevice *socket, QObject *parent)
    : QObexPushService(socket, parent),
      d(new DefaultObexPushServicePrivate)
{
    d->requestHandler = 0;
}

/*!
    Destroys the service.
*/
DefaultObexPushService::~DefaultObexPushService()
{
    if (d->requestHandler)
        delete d->requestHandler;
    delete d;
}

/*!
    Completes the processing of the transferred data. If a vCard or vCalendar
    was received, the file will be sent to the appropriate Qt Extended application.
    If any other type of file was received, the file will be saved into
    the Document System.

    This should be called if the requestFinished() signal is emitted with
    \c error and and \c aborted both set to \c false (as there is no point in
    saving or passing on the file if it was not successfully received).

    Returns \c true if the data was processed successfully. (The current
    implementation always returns \c true for business card requests.)
*/
bool DefaultObexPushService::finalizeDataTransfer()
{
    if (d->requestHandler) {
        return d->requestHandler->finalizeDataTransfer();
    } else {
        // this was a Get request (for the business card)
        return true;
    }
}

/*!
    Performs clean-up for the last received request. This will automatically
    be done when a new request is received, but you can call this function
    to ensure it is done even if the service does not receive another request.

    If this is called, calling currentContentId() will return
    QContent::InvalidId until the next request is received.
*/
void DefaultObexPushService::cleanUpRequest()
{
    delete d->requestHandler;
    d->requestHandler = 0;
}

/*!
    If the current request is a Put request that does not contain a vCard or
    vCalendar, and finalizeDataTransfer() has been called, this returns the
    QContentId for the QContent resource into which the received document has
    been saved. Otherwise, returns QContent::InvalidId.

    Returns QContent::InvalidId if cleanUpRequest() has been called.
*/
QContentId DefaultObexPushService::currentContentId() const
{
    GenericPutRequestHandler *h =
            qobject_cast<GenericPutRequestHandler*>(d->requestHandler);
    if (h && h->content() && h->content()->role() == QContent::Document)
        return h->content()->id();
    return QContent::InvalidId;
}

/*!
    \reimp
*/
QByteArray DefaultObexPushService::businessCard() const
{
    QContactModel model;
    if (!model.hasPersonalDetails())
        return QByteArray();

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly))
        return QByteArray();
    QContact myVCard = model.personalDetails();
    if (!QContact::writeVCard(&buffer, myVCard))
        return QByteArray();
    buffer.close();
    return bytes;
}

/*!
    \reimp
*/
QIODevice *DefaultObexPushService::acceptFile(const QString &name, const QString &type, qint64 size, const QString &description)
{
    qLog(Obex) << "DefaultObexPushService: incoming file" << name << type
            << size << description;
    if (d->requestHandler) {
        delete d->requestHandler;
        d->requestHandler = 0;
    }

    QString realMimeType = type.toLower();
    if (realMimeType.isEmpty()) {
        realMimeType = QMimeType::fromFileName(name).id().toLower();
        qLog(Obex) << "DefaultObexPushService: use revised mime type:" << realMimeType;
    }
    if (realMimeType == "text/x-vcard" || realMimeType == "text/x-vcalendar")
        d->requestHandler = new VObjectPutRequestHandler(this);
    else
        d->requestHandler = new GenericPutRequestHandler(this);

    connect(sessionDevice(), SIGNAL(destroyed(QObject*)),
            d->requestHandler, SLOT(disconnected()));
    return d->requestHandler->handleRequestStarted(name, realMimeType, size, description);
}


#include "defaultobexpushservice.moc"
