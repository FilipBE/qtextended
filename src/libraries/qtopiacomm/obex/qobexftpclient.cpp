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

#include "qobexftpclient.h"
#include "qobexfolderlisting_p.h"

#include <qobexheader.h>
#include <qobexclientsession.h>

#include <QString>
#include <QFile>
#include <QMap>
#include <QBuffer>
#include <QPointer>

#include <QXmlSimpleReader>


const char *object_profile =
"<?xml version=\"1.0\"?>\n"
"<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\">\n"
"<folder-listing version=\"1.0\">\n"
"\t<parent-folder />\n"
"\t<folder name=\"\" size=\"\" modified=\"\" created=\"\" accessed=\"\" user-perm=\"\" group-perm=\"\" other-perm=\"\" group=\"\" owner=\"\" />\n"
"\t<file name=\"\" size=\"\" type=\"\" modified=\"\" created=\"\" accessed=\"\" user-perm=\"\" group-perm=\"\" other-perm=\"\" group=\"\" owner=\"\" />\n"
"</folder-listing>\n";

class QObexFtpClientPrivate : public QObject
{
    Q_OBJECT

public:
    QObexFtpClientPrivate(QIODevice *device, QObexFtpClient *parent);
    ~QObexFtpClientPrivate();

public:
    QObexFtpClient *m_parent;
    QObexClientSession *m_client;
    QMap<int, QObexFtpClient::Command> m_commands;
    quint32 m_connId;
    QObexFtpClient::State m_state;

    QBuffer m_listing;
    QXmlSimpleReader m_listingReader;
    QXmlInputSource *m_listingInput;
    QObexFolderListingHandler m_listingHandler;
    bool m_newListing;
    bool m_listingParseOk;
    bool m_listingFinished;

    QObexFtpClient::Error m_error;

public slots:
    void requestStarted(int);
    void requestFinished(int, bool);
    void responseHeaderReceived(const QObexHeader &header);

private slots:
    void parseListingChunk();
    void processDone(bool error);

private:
    void fillPutHeader(QObexHeader &, const QString &, const QString &);

    void processError();
};

static char target_uuid[] = {
    0xF9,
    0xEC,
    0x7B,
    0xC4,
    0x95,
    0x3C,
    0x11,
    0xD2,
    0x98,
    0x4E,
    0x52,
    0x54,
    0x00,
    0xDC,
    0x9E,
    0x09
};

QObexFtpClientPrivate::QObexFtpClientPrivate(QIODevice *device, QObexFtpClient *parent) :
        QObject(parent),
        m_client(new QObexClientSession(device, this))
{
    m_parent = parent;

    m_state = QObexFtpClient::Unconnected;

    QObject::connect(m_client, SIGNAL(requestStarted(int)),
                     this, SLOT(requestStarted(int)));
    QObject::connect(m_client, SIGNAL(requestFinished(int,bool)),
                     this, SLOT(requestFinished(int,bool)));

    QObject::connect(m_client, SIGNAL(done(bool)),
                     this, SLOT(processDone(bool)));

    QObject::connect(m_client, SIGNAL(readyRead()), m_parent, SIGNAL(readyRead()));

    m_listing.open(QIODevice::ReadWrite);
    connect(&m_listing, SIGNAL(readyRead()), this, SLOT(parseListingChunk()));

    m_listingInput = new QXmlInputSource;
    m_listingReader.setContentHandler(&m_listingHandler);
    m_listingReader.setErrorHandler(&m_listingHandler);
    connect(&m_listingHandler, SIGNAL(info(QObexFolderListingEntryInfo)),
             m_parent, SIGNAL(listInfo(QObexFolderListingEntryInfo)));
}

QObexFtpClientPrivate::~QObexFtpClientPrivate()
{
    delete m_listingInput;
    delete m_client;
}

void QObexFtpClientPrivate::parseListingChunk()
{
    if (m_listing.buffer().isEmpty())
        return;

    // Check for NULL Terminator
    int pos;
    if ((pos = m_listing.buffer().indexOf('\0')) != -1)
        m_listing.buffer().truncate(pos);

    m_listingInput->setData(m_listing.buffer());
    m_listing.buffer().clear();
    m_listing.seek(0);

    if (m_newListing) {
        m_listingParseOk = m_listingReader.parse(m_listingInput, !m_listingFinished);
        m_newListing = false;
    }
    else {
        if (m_listingParseOk)
            m_listingParseOk = m_listingReader.parseContinue();
    }

    if (!m_listingParseOk) {
        qWarning("Error parsing folder listing object!");
    }
}

void QObexFtpClientPrivate::requestStarted(int id)
{
    QObexFtpClient::Command type = m_commands.value(id);

    emit m_parent->commandStarted(id);

    switch (type) {
        case QObexFtpClient::Connect:
            m_state = QObexFtpClient::Connecting;
            emit m_parent->stateChanged(m_state);
            break;
        case QObexFtpClient::Disconnect:
            m_state = QObexFtpClient::Disconnecting,
            emit m_parent->stateChanged(m_state);
            break;
        case QObexFtpClient::List:
            m_newListing = true;
            m_listingFinished = false;
            m_listing.buffer().clear();
            break;
        case QObexFtpClient::Get:
            QObject::connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                             m_parent, SIGNAL(dataTransferProgress(qint64,qint64)));
            break;
        case QObexFtpClient::Put:
            QObject::connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                             m_parent, SIGNAL(dataTransferProgress(qint64,qint64)));
            break;
        default:
            break;
    };
}

void QObexFtpClientPrivate::processError()
{
    switch (m_client->error()) {
        case QObexClientSession::NoError:
            m_error = QObexFtpClient::NoError;
        case QObexClientSession::ConnectionError:
        case QObexClientSession::UnknownError:
            m_error = QObexFtpClient::UnknownError;
        case QObexClientSession::RequestFailed:
            m_error = QObexFtpClient::RequestFailed;
        case QObexClientSession::InvalidRequest:
            m_error = QObexFtpClient::InvalidRequest;
        case QObexClientSession::InvalidResponse:
            m_error = QObexFtpClient::InvalidResponse;
        case QObexClientSession::Aborted:
            m_error = QObexFtpClient::Aborted;
        case QObexClientSession::AuthenticationFailed:
            m_error = QObexFtpClient::AuthenticationFailed;
        default:
            m_error = QObexFtpClient::NoError;
    };
}

void QObexFtpClientPrivate::requestFinished(int id, bool error)
{
    QObexFtpClient::Command type = m_commands[id];

    if (error) {
        processError();
        goto finish;
    }

    switch (type) {
        case QObexFtpClient::Connect:
            if (error)
                m_state = QObexFtpClient::Unconnected;
            else
                m_state = QObexFtpClient::Connected;

            emit m_parent->stateChanged(m_state);
            break;

        case QObexFtpClient::Disconnect:
            m_state = QObexFtpClient::Unconnected;
            emit m_parent->stateChanged(m_state);
            break;

        case QObexFtpClient::List:
            m_listingFinished = true;
            parseListingChunk();

            if (!m_listingParseOk) {
                m_error = QObexFtpClient::ListingParseError;
                error = true;
            }
            break;

        case QObexFtpClient::Get:
            QObject::disconnect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                                m_parent, SIGNAL(dataTransferProgress(qint64,qint64)));
            QObject::disconnect(m_client, SIGNAL(responseHeaderReceived(QObexHeader)),
                                this, SLOT(responseHeaderReceived(QObexHeader)));

            break;

        case QObexFtpClient::Put:
            QObject::disconnect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                                m_parent, SIGNAL(dataTransferProgress(qint64,qint64)));
            break;

        default:
            break;
    };

finish:

    QPointer<QObexFtpClient> that(m_parent);
    emit m_parent->commandFinished(id, error);

    if (that)
        m_commands.remove(id);
}

void QObexFtpClientPrivate::responseHeaderReceived(const QObexHeader &header)
{
    QObject::disconnect(m_client, SIGNAL(responseHeaderReceived(QObexHeader)),
                        this, SLOT(responseHeaderReceived(QObexHeader)));

    if (m_parent->currentCommand() == QObexFtpClient::Get) {
        // In a get response we might get a bit more information
        // about the object being obtained, so parse it out

        QString mimetype = header.type();
        QString description = header.description();
        QDateTime lastModified = header.time();

        if ((!mimetype.isEmpty()) || (!description.isEmpty()) || (lastModified.isValid()))
            emit m_parent->getInfo(mimetype, description, lastModified);
    }
}

void QObexFtpClientPrivate::processDone(bool error)
{
    if (error) {
        emit m_parent->done(true);
        return;
    }

    emit m_parent->done(m_error != QObexFtpClient::NoError);
}

/*!
    \class QObexFtpClient
    \inpublicgroup QtBaseModule

    \brief The QObexFtpClient class provides an implementation of the OBEX file-transfer mechanism.

    The class works asynchronously, so there are no blocking functions. If an
    operation cannot be executed immediately, the function will still return
    straight away and the operation will be scheduled for later execution. The
    results of scheduled operations are reported via signals. This approach
    depends on the event loop being in operation.

    The operations that can be scheduled (they are called "commands" in the rest
    of the documentation) are the following: connect(), disconnect(),
    list(), cd(), cdUp(), get(), put(), remove(), mkdir(), rmdir()

    All of these commands return a unique identifier that allows you to keep
    track of the command that is currently being executed. When the execution of
    a command starts, the commandStarted() signal with the command's identifier
    is emitted. When the command is finished, the commandFinished() signal is
    emitted with the command's identifier and a bool that indicates whether the
    command finished with an error.

    When the last scheduled command has finished, a done() signal is emitted
    with a bool argument that tells you whether the sequence finished with an
    error.

    If an error occurs during the execution of one of the commands in a sequence
    of commands, all the pending commands (i.e. scheduled, but not yet executed
    commands) are cleared and no signals are emitted for them.

    Some commands, e.g. list(), emit additional signals to report their results.

    An example of using this class is:
    \code
        ftp->connect();                             // id == 1
        ftp->cd("foo");                             // id == 2
        ftp->get("bar");                            // id == 3
        ftp->disconnect();                          // id == 4
    \endcode

    get() and put() operations report the progress of the transfer using the
    dataTransferProgress() signal.  If no IODevice is associated with a get,
    then the readyRead() signal is emitted.  In this case the data can be
    obtained with the read() or readAll().

    You can then get details about the error with the error() and
    errorString() functions.

    The functions currentId() and currentCommand() provide more information
    about the currently executing command.

    The functions hasPendingCommands() and clearPendingCommands() allow you
    to query and clear the list of pending commands.

    \sa QObexFolderListingEntryInfo
    \ingroup qtopiaobex
*/

/*!
    \enum QObexFtpClient::Command

    This enum is used to represent the different types of commands
    supported by the QObexFtpClient.

    \value None No command is being processed.
    \value Connect is being processed.
    \value Cd Change directory command is being processed.
    \value CdUp Change to parent directory command is being processed.
    \value List Directory listing command is being processed.
    \value Get Get file command is being processed.
    \value Put Put file command is being processed.
    \value Mkdir Make directory command is being processed.
    \value Rmdir Remove directory command is being processed.
    \value Remove Remove file command is being processed.
    \value Disconnect is being processed.
*/

/*!
    \enum QObexFtpClient::State

    This enum is used to represent the states that a QObexFtpClient
    can be in.

    \value Unconnected The client is not connected.  CONNECT command has not been sent.
    \value Connecting The OBEX CONNECT command has been sent.
    \value Connected The client is connected and is processing commands.
    \value Disconnecting The client has sent the OBEX DISCONNECT command.
*/

/*!
    \enum QObexFtpClient::Error
    \brief The errors that may occur for an OBEX client.

    \value NoError No error has occurred.
    \value RequestFailed The request was refused by the server (i.e. the server responded with a non-success response code).
    \value InvalidRequest The client request is invalid.
    \value InvalidResponse The server sent an invalid or unreadable response.
    \value Aborted The request was aborted by a call to abort().
    \value ListingParseError The listing could not be parsed.
    \value AuthenticationFailed The request failed because the client or server could not be authenticated.
    \value UnknownError An error other than those specified above occurred.
*/

/*!
    Constructs a new OBEX FTP Client object.  The device to use
    is given by \a device.  The device should already be opened in order
    to perform requests.

    The \a parent parameter specifies the \c QObject parent.

    \sa state()
 */
QObexFtpClient::QObexFtpClient(QIODevice *device, QObject *parent) : QObject(parent)
{
    m_data = new QObexFtpClientPrivate(device, this);
}

/*!
    Deconstructs an OBEX FTP Client.
 */
QObexFtpClient::~QObexFtpClient()
{
    if (m_data)
        delete m_data;
}

/*!
    Requests the client to connect to the server.  The \c device will
    be used to establish the OBEX transport connection if the socket is not
    yet connected.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa stateChanged(), commandStarted(), commandFinished(), disconnect()
 */
int QObexFtpClient::connect()
{
    QObexHeader header;
    header.setTarget(QByteArray(target_uuid, sizeof(target_uuid)));

    int id = m_data->m_client->connect(header);
    m_data->m_commands.insert(id, QObexFtpClient::Connect);

    return id;
}

/*!
    Disconnect from the server.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa stateChanged(), commandStarted(), commandFinished(), connect()
 */
int QObexFtpClient::disconnect()
{
    int id = m_data->m_client->disconnect();

    m_data->m_commands.insert(id, QObexFtpClient::Disconnect);

    return id;
}

/*!
    Obtains the content listing of the directory given by \a dir.  If \a dir is an
    empty string, then the contents of the current working directory are returned.

    The listInfo() signal will be sent for all contents of the directory being
    listed.  

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa listInfo(), commandStarted(), commandFinished()
*/
int QObexFtpClient::list(const QString &dir)
{
    QObexHeader header;
    if (!dir.isEmpty())
        header.setName(dir);

    header.setType("x-obex/folder-listing");

    int id = m_data->m_client->get(header, &m_data->m_listing);

    m_data->m_commands.insert(id, QObexFtpClient::List);

    return id;
}

/*!
    Change the current working directory to a directory given by \a dir.  If \a dir
    is an empty string, then request the root directory to become the current
    working directory.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa commandStarted(), commandFinished()
*/
int QObexFtpClient::cd(const QString &dir)
{
    QObexHeader header;

    if (dir.isEmpty()) {
        header.setName("");
        int id = m_data->m_client->setPath(header, QObex::NoPathCreation);
        m_data->m_commands.insert(id, QObexFtpClient::Cd);
        return id;
    }

    header.setName(dir);
    int id = m_data->m_client->setPath(header, QObex::NoPathCreation);
    m_data->m_commands.insert(id, QObexFtpClient::Cd);

    return id;
}

/*!
    Changes the current working directory to be the parent of the current working
    directory.  Root directories do not have parent directories, and it is an error
    to cd to parent directory of the root directory.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa commandStarted(), commandFinished()
*/
int QObexFtpClient::cdUp()
{
    QObexHeader header;

    int id = m_data->m_client->setPath(header, QObex::BackUpOneLevel|QObex::NoPathCreation);
    m_data->m_commands.insert(id, QObexFtpClient::CdUp);

    return id;
}

/*!
    Create a new directory on the server.  The directory name is given by \a dir.  If
    the command succeeds, the current working directory will be the directory
    just created.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa commandStarted(), commandFinished()
*/
int QObexFtpClient::mkdir(const QString &dir)
{
    QObexHeader header;
    header.setName(dir);

    int id = m_data->m_client->setPath(header, 0);
    m_data->m_commands.insert(id, QObexFtpClient::Mkdir);

    return id;
}

/*!
    Remove the directory \a dir on the server.  Some servers do not allow deletion
    of non-empty directories.  The server should report a PreconditionFailed response
    code in this case.  In general the client should recursively delete all contents
    of the directory before using this command.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa commandStarted(), commandFinished()
 */
int QObexFtpClient::rmdir(const QString &dir)
{
    QObexHeader header;
    header.setName(dir);

    int id = m_data->m_client->putDelete(header);
    m_data->m_commands.insert(id, QObexFtpClient::Rmdir);

    return id;
}

/*!
    Removes \a file from the server.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa commandStarted(), commandFinished()
*/
int QObexFtpClient::remove(const QString &file)
{
    QObexHeader header;
    header.setName(file);

    int id = m_data->m_client->putDelete(header);
    m_data->m_commands.insert(id, QObexFtpClient::Remove);

    return id;
}

/*!
    Attempts to retrieve \a file from the server.  If \a dev is NULL,
    then the readyRead() signal is emitted when there is data available to read.
    You can then read the data with the read() or readAll() functions.

    If \a dev is not 0, the data is written directly to the device \a dev. Make sure that
    the \a dev pointer is valid for the duration of the operation (it is safe to delete
    it when the commandFinished() signal is emitted). In this case the readyRead() signal
    is not emitted and you cannot read data with the read() or readAll() functions.

    If you don't read the data immediately it becomes available, i.e. when the
    readyRead() signal is emitted, it is still available until the next command is started.

    The transfer progress is reported by the dataTransferProgress() signal.

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa readyRead(), dataTransferProgress(), commandStarted(), commandFinished(), getInfo()
*/
int QObexFtpClient::get(const QString &file, QIODevice *dev)
{
    QObexHeader header;
    header.setName(file);

    int id = m_data->m_client->get(header, dev);
    m_data->m_commands.insert(id, QObexFtpClient::Get);

    return id;
}

/*!
    Reads the data from \a dev and puts it in a file called \a file on the server.
    The data is read in chunks from the IO device, so this overload allows you to
    transmit large amounts of data without the need to read all the data
    into memory at once.

    If \a size is not 0, then it is used for OBEX Length header contents.  If
    \a mimetype is not empty, it is used for OBEX Type header contents.  If
    \a description is not empty, it is used for OBEX Description header contents.
    If \a lastModified is valid, it is used for OBEX Time header contents.

    Make sure that the dev pointer is valid for the duration of the operation
    (it is safe to delete it when the commandFinished() is emitted).

    The upload progress is reported by the dataTransferProgress() signal

    The function does not block and returns immediately. The command is
    scheduled, and its execution is performed asynchronously. The function
    returns a unique identifier which is passed by commandStarted() and
    commandFinished().

    When the command is started the commandStarted() signal is emitted. When it is
    finished the commandFinished() signal is emitted.

    \sa dataTransferProgress(), commandStarted(), commandFinished()
*/
int QObexFtpClient::put(QIODevice *dev, const QString &file,
                        qint64 size, const QString &mimetype,
                        const QString &description,
                        const QDateTime &lastModified)
{
    QObexHeader header;
    header.setName(file);

    if (size != 0)
        header.setLength(size);

    if (!mimetype.isEmpty())
        header.setType(mimetype);

    if (!description.isEmpty())
        header.setDescription(description);

    if (lastModified.isValid())
        header.setTime(lastModified);

    int id = m_data->m_client->put(header, dev);
    m_data->m_commands.insert(id, QObexFtpClient::Put);

    return id;
}

/*!
    This is an overloaded member function provided for convenience.  It is the same
    as the above function, except the data is obtained directory from \a data instead
    of an IO device.  The size information is obtained from the size of the byte
    array.  The \a file, \a mimetype, \a description and \a lastModified parameters
    have the same meaning as discussed previously.

    \sa dataTransferProgress(), commandStarted(), commandFinished()
*/
int QObexFtpClient::put(const QByteArray &data, const QString &file,
                        const QString &mimetype,
                        const QString &description,
                        const QDateTime &lastModified)
{
    QObexHeader header;
    header.setName(file);
    header.setLength(data.size());

    if (!mimetype.isEmpty())
        header.setType(mimetype);

    if (!description.isEmpty())
        header.setDescription(description);

    if (lastModified.isValid())
        header.setTime(lastModified);

    int id = m_data->m_client->put(header, data);
    m_data->m_commands.insert(id, QObexFtpClient::Put);

    return id;
}

/*!
    Attempts to abort the current operation.  This will also clear all pending
    operations.  If no operations are in progress or pending, then this function
    has no effect.

    \sa clearPendingCommands()
 */
void QObexFtpClient::abort()
{
    m_data->m_client->abort();
}

/*!
    Returns the device used by the Ftp Client, as provided in the constructor.
*/
QIODevice *QObexFtpClient::sessionDevice() const
{
    return m_data->m_client->sessionDevice();
}

/*!
    Returns the id of the operation currently in progress.  If no operation is in progress
    a 0 is returned.

    \sa currentCommand()
*/
int QObexFtpClient::currentId() const
{
    return m_data->m_client->currentId();
}

/*!
    Returns the current state of the client.

    \sa stateChanged()
*/
QObexFtpClient::State QObexFtpClient::state() const
{
    return m_data->m_state;
}

/*!
    Returns the command type of the current operation in progress.  If no operation is
    in progress, a QObexFtpClient::None is returned.

    \sa currentId()
*/
QObexFtpClient::Command QObexFtpClient::currentCommand() const
{
    int id = currentId();
    if (m_data->m_commands.contains(id))
        return m_data->m_commands.value(id);

    return QObexFtpClient::None;
}

/*!
    Returns the current QIODevice that is being used for a get / put operation.  If no
    device is being used, a NULL is returned.

    \sa put(), get()
*/
QIODevice *QObexFtpClient::currentDevice() const
{
    return m_data->m_client->currentDevice();
}

/*!
    Returns true if the Ftp Client has pending operations, and false otherwise.

    \sa clearPendingCommands(), currentId(), currentCommand()
*/
bool QObexFtpClient::hasPendingCommands() const
{
    return m_data->m_client->hasPendingRequests();
}

/*!
    Clears all pending commands.  If there are no pending commands, this method has no
    effect.

    \sa hasPendingCommands(), currentId(), currentCommand()
*/
void QObexFtpClient::clearPendingCommands()
{
    m_data->m_commands.clear();
    m_data->m_client->clearPendingRequests();
}

/*!
    Returns the last response from the remote service.  This could be used to find
    out the reason a command might have failed (e.g. unauthorized, unsupported, etc)
*/
QObex::ResponseCode QObexFtpClient::lastCommandResponse() const
{
    return m_data->m_client->lastResponseCode();
}

/*!
    Attempts to read \a maxlen bytes from the ftp stream and store them in
    a location pointed to by \a data.

    Returns the number of bytes read or -1 if an error occurred.

    \sa readAll(), bytesAvailable()
*/
qint64 QObexFtpClient::read(char *data, qint64 maxlen)
{
    return m_data->m_client->read(data, maxlen);
}

/*!
    Reads all the bytes available from the ftp stream and returns them as
    a byte array.

    \sa read(), bytesAvailable()
*/
QByteArray QObexFtpClient::readAll()
{
    return m_data->m_client->readAll();
}

/*!
    Returns the number of bytes available in the ftp stream.

    \sa read(), readAll()
*/
qint64 QObexFtpClient::bytesAvailable() const
{
    return m_data->m_client->bytesAvailable();
}

/*!
    Returns the last error that occurred. This is useful for finding out
    what happened when receiving an requestFinished() or done()
    signal that has the \c error argument set to \c true.

    \sa errorString()
 */
QObexFtpClient::Error QObexFtpClient::error() const
{
    return m_data->m_error;
}

/*!
    Returns a human-readable description of the last error that occurred.

    The error string is reset when a new request is started.

    \sa error()
*/
QString QObexFtpClient::errorString() const
{
    switch (m_data->m_error) {
        case QObexFtpClient::ListingParseError:
            return QString("Unable to parse the folder listing object!");
        default:
            return m_data->m_client->errorString();
    };
}

/*!
    \fn void QObexFtpClient::listInfo(const QObexFolderListingEntryInfo &info);

    This signal is emitted whenever a directory listing information is obtained
    from the remote service.  The directory entry is reported in \a info.

    \sa list()
*/

/*!
    \fn void QObexFtpClient::commandFinished(int id, bool error);

    This signal is emitted whenever a queued command has been performed.  The
    \a id parameter holds the id of the command finished.  The \a error parameter
    holds whether an error occurred.

    \sa commandStarted()
 */

/*!
    \fn void QObexFtpClient::commandStarted(int id);

    This signal is emitted whenever a queued command has been started.  The
    \a id parameter holds the id of the command.

    \sa commandFinished()
 */

/*!
    \fn void QObexFtpClient::done(bool error);

    This signal is emitted whenever all pending requests have been completed.
    The \a error parameter reports whether an error occurred during processing.

    \sa commandStarted(), commandFinished()
 */

/*!
    \fn void QObexFtpClient::dataTransferProgress(qint64 completed, qint64 total);

    This signal is emitted reports the progress of the file send operation.
    The \a completed parameter reports how many bytes were sent, and \a total
    parameter reports the total number of bytes.  If the total is not known,
    it will be reported as 0.

    \sa get(), put()
 */

/*!
    \fn void QObexFtpClient::getInfo(const QString &mimetype, const QString &description, const QDateTime &lastModified);

    Some OBEX servers can provide additional information about a file
    in the headers sent with the GET command response that might not
    appear in the directory listing.  These attributes are returned
    in this signal.  An empty, default-constructed value is given
    if the GET response did not specify any additional information.

    The \a mimetype parameter holds the contents of the Type header.
    The \a description parameter holds the contents of the Description
    header.  The \a lastModified holds the contents of the Time
    header.

    \sa get(), listInfo()
*/

/*!
    \fn void QObexFtpClient::readyRead()

    This signal is emitted whenever data is ready to be read from the ftp
    stream.

    \sa read(), readAll(), bytesAvailable()
*/

/*!
    \fn void QObexFtpClient::stateChanged(QObexFtpClient::State state);

    This signal is emitted whenever the Ftp Client has changed state.  The new
    state is reported in \a state.

    \sa state()
*/

#include "qobexftpclient.moc"
