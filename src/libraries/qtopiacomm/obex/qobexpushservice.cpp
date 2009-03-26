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
#include "qobexpushservice.h"
#include <qobexserversession.h>
#include <qobexheader.h>

#include <qobexnamespace.h>
#include <qtopialog.h>

#include <QFile>
#include <QPointer>
#include <QTimer>
#include <QDir>
#include <QFileInfo>

#include <unistd.h>


#define OBEX_STREAM_BUF_SIZE 4096


class QObexPushServicePrivate : public QObexServerSession
{
    Q_OBJECT

public slots:
    QObex::ResponseCode connect(const QObexHeader &header);
    QObex::ResponseCode disconnect( const QObexHeader &header );
    QObex::ResponseCode put(const QObexHeader &header);
    QObex::ResponseCode get(const QObexHeader &header);

private slots:
    void requestFinished();
    void sentFinalResponse(QObex::Request request);
    void emitDone();

protected:
    virtual void error(QObexServerSession::Error error, const QString &errorString);
    QObex::ResponseCode dataAvailable( const char *data, qint64 size );
    QObex::ResponseCode provideData( const char **data, qint64 *size );

public:
    QObexPushServicePrivate(QIODevice *device, QObexPushService *parent);
    ~QObexPushServicePrivate();

    void close();

    QObexPushService::State serviceState() const;
    QObexPushService::Error serviceError() const;

    QObexPushService *m_parent;

    QObexPushService::Error m_error;
    QObexPushService::State m_state;

    QByteArray m_vcard;
    bool m_cleanUpPutDevice;
    QPointer<QIODevice> m_device;
    bool m_abortPending;

private:
    void updateState(QObexPushService::State state);
    void cleanUpPut(bool aborted);

    int m_total;
    int m_bytes;
    bool m_callingAcceptFile;
};


QObexPushServicePrivate::QObexPushServicePrivate(QIODevice *device, QObexPushService *parent)
    : QObexServerSession(device, parent),
      m_parent(parent)
{
    m_state = QObexPushService::Ready;
    m_error = QObexPushService::NoError;
    m_device = 0;
    m_abortPending = false;
    m_cleanUpPutDevice = false;
    m_callingAcceptFile = false;

    QObject::connect(this, SIGNAL(finalResponseSent(QObex::Request)),
                     SLOT(sentFinalResponse(QObex::Request)));
}

QObexPushServicePrivate::~QObexPushServicePrivate()
{
    cleanUpPut(false);
}

QObexPushService::State QObexPushServicePrivate::serviceState() const
{
    return m_state;
}

QObexPushService::Error QObexPushServicePrivate::serviceError() const
{
    return m_error;
}

QObex::ResponseCode QObexPushServicePrivate::get(const QObexHeader &header)
{
    // OPP spec 5.6 - reject GET if name is not empty
    if (!header.name().isEmpty())
        return QObex::Forbidden;

    // be confused if a Get request is not for a v-card
    if (header.type().compare("text/x-vCard", Qt::CaseInsensitive) != 0) {
        qLog(Obex) << "QObexPushService: received Get request that wasn't for a v-card";
        return QObex::Forbidden;
    }

    // OPP spec 5.6 - NotFound if there is no default object
    if (m_vcard.isEmpty()) {
        m_vcard = m_parent->businessCard();
        if (m_vcard.isEmpty()) {
            qLog(Obex) << "QObexPushService: default vCard requested, but none set!";
            return QObex::NotFound;
        }
    }

    m_total = m_vcard.size();
    m_bytes = 0;
    m_abortPending = false;

    emit m_parent->businessCardRequested();
    updateState(QObexPushService::Streaming);
    return QObex::Success;
}

QObex::ResponseCode QObexPushServicePrivate::provideData( const char **data, qint64 *size )
{
    if (m_abortPending) {
        m_error = QObexPushService::Aborted;
        m_abortPending = false;
        return QObex::Forbidden;
    }

    char *streambuf = &m_vcard.data()[m_bytes];

    int len = 0;

    if ( (m_total - m_bytes) > OBEX_STREAM_BUF_SIZE) {
        len = OBEX_STREAM_BUF_SIZE;
    } else {
        len = m_total - m_bytes;
    }

    m_bytes += len;

    if (len > 0) {
        *data = streambuf;
        *size = len;
        emit m_parent->dataTransferProgress( m_bytes, m_total );
    } else {
        *size = 0;
    }

    return QObex::Success;
}

QObex::ResponseCode QObexPushServicePrivate::put(const QObexHeader &header)
{
    QPointer<QObexPushServicePrivate> ptr(this);
    m_abortPending = false;
    m_cleanUpPutDevice = false;
    m_callingAcceptFile = true;
    m_device = m_parent->acceptFile(header.name(), header.type(),
                                    header.length(), header.description());

    if (ptr.isNull()) {
        qLog(Obex) << "QObexPushService: self deleted during acceptFile()";
        return QObex::InternalServerError;
    }

    m_callingAcceptFile = false;

    // If user has started a reentrant loop during the acceptFile() callback
    // by showing a UI dialog box, etc. then it's possible the connection was
    // broken during the callback.
    if (m_error != QObexPushService::NoError) {
        qLog(Obex) << "QObexPushService: error during acceptFile():" << m_error;

        // emit done when this function has returned
        QTimer::singleShot(0, this, SLOT(emitDone()));
        return QObex::InternalServerError;
    }

    // acceptFile() returned 0, so reject the request
    if (!m_device)
        return QObex::Forbidden;

    if (!m_device->isOpen() && !m_device->open(QIODevice::WriteOnly)) {
        qWarning("QObexPushService: unable to open QIODevice for OBEX Put request");
        return QObex::Forbidden;
    }

    m_total = ( header.length() > 0 ? header.length() : 0 );
    m_bytes = 0;

    emit m_parent->putRequested(header.name(), header.type(), header.length(), header.description());

    updateState(QObexPushService::Streaming);
    return QObex::Success;
}

QObex::ResponseCode QObexPushServicePrivate::dataAvailable( const char *data, qint64 size )
{
    if (m_abortPending) {
        m_error = QObexPushService::Aborted;
        m_abortPending = false;
        return QObex::Forbidden;
    }

    m_bytes += size;

    if (size > 0) {
        if (m_device && m_device->isOpen()) {
            if (m_device->write(data, size) > 0) {
                emit m_parent->dataTransferProgress( m_bytes, m_total );
                return QObex::Success;
            }
        }

        qWarning("QObexPushService: error writing received data to QIODevice for OBEX Put request");
        m_error = QObexPushService::UnknownError;
    }

    if (m_error == QObexPushService::NoError)
        return QObex::Success;
    return QObex::InternalServerError;
}

void QObexPushServicePrivate::sentFinalResponse(QObex::Request request)
{
    switch (request) {
        case QObex::Connect:
            updateState(QObexPushService::Ready);
            break;
        case QObex::Disconnect:
            updateState(QObexPushService::Closed);
            break;
        case QObex::Put:
        case QObex::Get:
            if (m_state == QObexPushService::Streaming) {
                requestFinished();
            }
        default:
            break;
    }

    m_abortPending = false;
}

void QObexPushServicePrivate::requestFinished()
{
    bool wasStreaming = (m_state == QObexPushService::Streaming);

    if (m_error == QObexPushService::NoError ||
            m_error == QObexPushService::Aborted ) {
        updateState(QObexPushService::Ready);
    }

    QPointer<QObexPushServicePrivate> ptr(this);
    if (wasStreaming)
        emit m_parent->requestFinished(m_error != QObexPushService::NoError);

    if (ptr.isNull())
        return;

    if (m_error == QObexPushService::Aborted) {
        cleanUpPut(true);
    } else {
        cleanUpPut(false);
    }

    if (m_error == QObexPushService::ConnectionError ||
            m_error == QObexPushService::UnknownError) {
        emit m_parent->done(m_error != QObexPushService::NoError);
    }
}

QObex::ResponseCode QObexPushServicePrivate::connect(const QObexHeader &)
{
    updateState(QObexPushService::Connecting);
    return QObex::Success;
}

QObex::ResponseCode QObexPushServicePrivate::disconnect(const QObexHeader &)
{
    updateState(QObexPushService::Disconnecting);
    return QObex::Success;
}

void QObexPushServicePrivate::updateState(QObexPushService::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit m_parent->stateChanged(m_state);
    if (m_state == QObexPushService::Closed) {
        // all done...
        cleanUpPut(false);
        QObexServerSession::close();

        emit m_parent->done(m_error != QObexPushService::NoError);
    }
}

void QObexPushServicePrivate::emitDone()
{
    emit m_parent->done(m_error != QObexPushService::NoError);
}

void QObexPushServicePrivate::error(QObexServerSession::Error error, const QString &errorString)
{
    qLog(Obex) << "QObexPushService: error" << error << errorString
            << "- during state:" << m_state;

    if (m_state == QObexPushService::Closed)
        return;

    switch (error) {
        case QObexServerSession::Aborted:
            m_error = QObexPushService::Aborted;
            requestFinished();
            break;

        default:
            if (error == QObexServerSession::ConnectionError) {
                m_error = QObexPushService::ConnectionError;
            } else {
                m_error = QObexPushService::UnknownError;
            }

            if (m_state == QObexPushService::Ready) {
                // don't emit done() yet if acceptFile() has not returned (e.g.
                // if connection is lost while class is blocking synchronously
                // in acceptFile() by running a UI dialog box)
                if (!m_callingAcceptFile) {
                    emit m_parent->done(m_error != QObexPushService::NoError);
                }
            } else {
                requestFinished();
            }
            break;
    }

}

void QObexPushServicePrivate::cleanUpPut(bool aborted)
{
    if (m_cleanUpPutDevice && m_device) {
        // device was opened by default acceptFile() implementation, so
        // close it now
        m_device->close();

        if (aborted) {
            QFile *file = qobject_cast<QFile*>(m_device);
            if (file)
                file->remove();
        }
    }
    m_device = 0;
    m_cleanUpPutDevice = false;
}

//==================================================================

/*!
    \class QObexPushService
    \inpublicgroup QtBaseModule

    \brief The QObexPushService class provides an OBEX Push service.

    The QObexPushService class can be used to provide OBEX Push services
    over Bluetooth, Infrared or any other OBEX capable transport.
    This class implements the Bluetooth Object Push Profile, and can also
    be used to implement the Infrared IrXfer service.

    If you want to control whether an incoming file should be accepted,
    subclass QObexPushService and override the acceptFile() function.
    Subclasses can also override businessCard() to provide the default
    business card object that may be requested by an OBEX Push client.

    When an OBEX client sends a file to the service, the acceptFile() function
    is called to determine whether the file transfer should be allowed. If
    the transfer is accepted, the putRequested() signal is emitted with
    the details of the incoming file, and the dataTransferProgress() signal will be
    emitted at intervals to show the progress of the file transfer operation.
    The requestFinished() signal is emitted when the transfer is complete.

    Similarly, when an OBEX client requests the default business card, the
    businessCardRequested() signal is emitted, and the
    dataTransferProgress() signal will be emitted at intervals. The requestFinished()
    signal is emitted when the transfer is complete.

    Finally, the done() signal is emitted when the OBEX client disconnects or
    if the connection is terminated.


    \section1 Handling socket disconnections

    You should ensure that the QIODevice provided in the constructor emits
    QIODevice::aboutToClose() or QObject::destroyed() when the associated
    transport connection is disconnected. If one of these signals
    are emitted, QObexPushService will know the transport connection has
    been lost, and will emit done() (and also requestFinished() if a \c Put or
    \c Get request is in progress) with \c error set to \c true, and error()
    will return ConnectionError.

    This is particularly an issue for socket classes such as QTcpSocket that
    do not emit QIODevice::aboutToClose() when a \c disconnected() signal is
    emitted. In these cases, QObexPushService will not know that the
    transport has been disconnected. To avoid this, you can make the socket
    emit QIODevice::aboutToClose() when it is disconnected:

    \code
    // make the socket emit aboutToClose() when disconnected() is emitted
    QObject::connect(socket, SIGNAL(disconnected()), socket, SIGNAL(aboutToClose()));
    \endcode

    Or, if the socket can be discarded as soon as it is disconnected:

    \code
    // delete the socket when the transport is disconnected
    QObject::connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    \endcode

    \ingroup qtopiaobex
*/

/*!
    \enum QObexPushService::State
    Defines the possible states for a push service.

    \value Ready The service is ready to receive requests from an OBEX client. This is the default state.
    \value Connecting A client is connecting.
    \value Disconnecting A client is disconnecting.
    \value Streaming A file transfer operation is in progress.
    \value Closed The service has been closed and cannot process any more requests. The service will change to this state if a client sends a \c Disconnect command, or a ConnectionError occurs.
 */

/*!
    \enum QObexPushService::Error
    Defines the possible errors for a push service.

    \value NoError No error has occurred.
    \value ConnectionError The client is unable to send data, or the client-server communication process is otherwise disrupted. If this happens, the state will change to QObexPushService::Closed.
    \value Aborted The request was aborted (either by the client, or by a call to abort()).
    \value UnknownError An error other than those specified above occurred.
 */

/*!
    Constructs an OBEX Push service that uses \a device for the transport
    connection. The \a parent is the QObject parent.

    The \a device must be opened, or else the service will be unable
    to receive any client requests.
*/
QObexPushService::QObexPushService(QIODevice *device, QObject *parent)
    : QObject(parent)
{
    m_data = new QObexPushServicePrivate(device, this);
}

/*!
    Destroys the service.
*/
QObexPushService::~QObexPushService()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns the error for the last completed request.
*/
QObexPushService::Error QObexPushService::error() const
{
    return m_data->serviceError();
}

/*!
    Returns the current state of the service.
*/
QObexPushService::State QObexPushService::state() const
{
    return m_data->serviceState();
}

static QString qobexpushservice_getSaveFileName(const QString &path, const QString &baseName)
{
    QString pathToCheck = ( path.endsWith(QDir::separator()) ?
            path + baseName : path + QDir::separator() + baseName );
    if (!QFile::exists(pathToCheck))
        return pathToCheck;

    int index = baseName.lastIndexOf('_');
    if (index == -1)
        return qobexpushservice_getSaveFileName(path, baseName + QLatin1String("_1"));
    if (index == baseName.size() - 1)
        return qobexpushservice_getSaveFileName(path, baseName + '1');

    int count = baseName.mid(index + 1).toInt();
    if (count == 0) {   // not a number after underscore
        return qobexpushservice_getSaveFileName(path, baseName + QLatin1String("_1"));
    } else {
        return qobexpushservice_getSaveFileName(path,
                    baseName.mid(0, index) + '_' + QString::number(count + 1));
    }
}

/*!
    Called when a client makes a \c Put request to send a file to this service.
    The \a name, \a type, \a size and \a description parameters respectively
    hold the name, MIME type, size and description of the file, as sent by the
    client. The \a size will be 0 if the client did not send any information
    about the size of the file.

    This function can be overridden to intercept the request and determine
    whether the request should be allowed to continue (for example, by asking
    the end user, or by checking some file threshold) and where the file data
    should be saved. To allow the request, return a QIODevice pointer, and the
    received file data will be written to this QIODevice. To refuse the request,
    return 0.

    If the request is allowed to continue, the putRequested() signal is
    emitted, and the requestFinished() signal will be emitted when the
    the request has finished. The returned QIODevice pointer is accessible
    from currentDevice().

    The default implementation attempts to save the incoming file into
    QDir::homePath(), using \a name as the name of the file. (If \a name
    contains a path, only the filename at the end of the
    path will be used, and if \a name is an empty string, the file will be
    saved as "received_file".) If the file is successfully created, the \c Put
    request will be accepted; otherwise, the request will be refused.

    \sa putRequested(), currentDevice(), dataTransferProgress(), requestFinished()
*/
QIODevice *QObexPushService::acceptFile(const QString &name, const QString &, qint64, const QString &)
{
    QString fname = name;
    if (fname.contains(QDir::separator())) {
        fname = QFileInfo(fname).fileName();
    }

    // get unique filename in home directory
    fname = QFileInfo(qobexpushservice_getSaveFileName(
            QDir::homePath(), fname.isEmpty() ? tr("received_file", "placeholder name for a received file with no name") : fname)).fileName();

    QFile *file = 0;
    if (QDir::homePath().endsWith(QDir::separator())) {
        file = new QFile(QDir::homePath() + fname, this);
    } else {
        file = new QFile(QDir::homePath() + QDir::separator() + fname, this);
    }

    if (file->open(QIODevice::WriteOnly)) {
        // since file is created by the default implementation, we need to do
        // clean up the file later
        m_data->m_cleanUpPutDevice = true;
        return file;
    } else {
        qWarning("Unable to open file %s, refusing OBEX Put request with name '%s'",
                 file->fileName().toLatin1().constData(), name.toLatin1().constData());
        delete file;
        return 0;
    }
}

/*!
    Sets \a vCard to be the business card (in vCard format) that will be sent to
    OBEX clients who request it, as per the Business Card exchange feature of the
    Bluetooth Object Push Profile.

    If no business card is set, this service will refuse all client requests
    for business cards.

    \sa businessCard()
*/
void QObexPushService::setBusinessCard(const QByteArray &vCard)
{
    m_data->m_vcard = vCard;
}

/*!
    Returns the business card that will be sent to OBEX clients who request
    it, as per the Business Card exchange feature of the Bluetooth Object Push
    Profile.

    \sa setBusinessCard()
*/
QByteArray QObexPushService::businessCard() const
{
    return m_data->m_vcard;
}

/*!
    If a \c Put request is in progress, this function returns the QIODevice
    pointer that is used to store the data from the request. If there is no
    \c Put request in progress, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the requestFinished() signal.

    \sa acceptFile()
*/
QIODevice *QObexPushService::currentDevice() const
{
    return m_data->m_device;
}

/*!
    Returns the device used by this OBEX Push server session, as provided in
    the constructor.
 */
QIODevice *QObexPushService::sessionDevice() const
{
    return m_data->sessionDevice();
}

/*!
    Aborts the current client request if a \c Put or \c Get request is in
    progress. If the request was successfully aborted, requestFinished() will
    be emitted with \c error set to \c true and error() will return
    QObexPushService::Aborted.

    Due to timing issues, the request may finish before it can be aborted, in
    which case requestFinished() is emitted normally.
*/
void QObexPushService::abort()
{
    if (m_data->m_device)
        m_data->m_abortPending = true;
}

/*!
    \fn void QObexPushService::done(bool error);

    This signal is emitted when the OBEX client has disconnected from the service
    or if the connection has been terminated. The \a error value is \c true if an
    error occurred during the processing; otherwise \a error is \c false.
 */

/*!
    \fn void QObexPushService::dataTransferProgress(qint64 done, qint64 total);

    This signal is emitted during file transfer operations to
    indicate the progress of the transfer. The \a done value is the
    number of bytes that have been sent or received so far, and \a total
    is the total number of bytes to be sent or received.

    If the total number of bytes is not known, \a total will be 0.
 */

/*!
    \fn void QObexPushService::stateChanged(QObexPushService::State state)

    This signal is emitted when the state of the service changes. The \a state
    is the new state of the client.
 */

/*!
    \fn void QObexPushService::putRequested(const QString &name, const QString &type, qint64 size, const QString &description)

    This signal is emitted when an OBEX client has made a \c Put request to the
    service and acceptFile() has accepted the request. The \a name, \a type,
    \a size and \a description parameters respectively hold the name, MIME
    type, size and description of the file, as sent by the client. The \a size
    will be 0 if the client did not send any information about the size of the
    file.

    The requestFinished() signal is emitted when the \c Put request is
    completed.

    \sa currentDevice()
*/

/*!
    \fn void QObexPushService::businessCardRequested()

    This signal is emitted when an OBEX client has requested the business
    card from the service, and businessCard() returns a non-empty
    QByteArray. (If businessCard() returns an empty QByteArray, the signal
    is not emitted.)

    The requestFinished() signal is emitted when the request is
    completed.

    \sa setBusinessCard(), businessCard()
*/

/*!
    \fn void QObexPushService::requestFinished(bool error)

    This signal is emitted when a client \c Put or \c Get request is completed.
    The \a error value is \c true if an error occurred during the request;
    otherwise \a error is \c false.
*/


#include "qobexpushservice.moc"
