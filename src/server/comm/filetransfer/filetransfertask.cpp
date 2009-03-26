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

#include "filetransfertask.h"

/*!
    \class FileTransferTask
    \inpublicgroup QtInfraredModule
    \inpublicgroup QtBluetoothModule
    \brief The FileTransferTask class provides an interface to receive notifications about file transfer operations.
    \ingroup QtopiaServer::Task::Interfaces


    FileTransferTask is a Qt Extended server task interface. If a class is to
    implement this interface, it must subclass FileTransferTask, be made
    into a task, and declare that it implements this interface using
    QTOPIA_TASK_PROVIDES. For example, if there is a subclass called
    \c MyTransferService, the code would look something similar to this:

    In the .h header file:
    \code
    class MyTransferService : public FileTransferTask
    {
        Q_OBJECT
    public:
        MyTransferService();
    }
    \endcode

    In the .cpp implementation file:
    \code
    MyTransferService::MyTransferService()
        : FileTransferTask()
    {
    }
    QTOPIA_TASK(MyTransferService, MyTransferService)
    QTOPIA_TASK_PROVIDES(MyTransferService, FileTransferTask)
    \endcode

    Refer to the various FileTransferTask subclasses for the various transfer tasks supported by Qtopia.

    Each existing FileTransferTask is automatically incorporated into the FileTransferWindow
    which monitors each transfer and presents a window to the user indicating the progress.

    This class is part of the QtopiaServer and cannot be used by other Qt Extended applications.

    \sa FileTransferWindow
*/

static QAtomicInt transferIdCounter(1);

/*!
    \internal
*/
FileTransferTask::FileTransferTask(QObject *parent)
    : QObject(parent)
{
}

/*!
    \internal
*/
FileTransferTask::~FileTransferTask()
{
}

/*!
    Returns the QContent ID for the transfer identified by \a id, or
    QContent::InvalidId if the transfer does not have an associated
    QContent ID.

    This value may change over the course of a file transfer. For example,
    an incoming transfer may not have an associated QContent ID to begin
    with, but a file transfer task might assign a QContent ID to the
    transfer once the transfer is finished and the data has been saved into
    the Qt Extended document system as an identifiable QContent resource.

    The specified transfer should be one that is still in progress or has only
    just finished (i.e. at the point when transferFinished() is emitted), as a
    file transfer task is not required to retain information for a transfer
    after transferFinished() has been emitted.

    The default implementation returns QContent::InvalidId.
*/
QContentId FileTransferTask::transferContentId(int ) const
{
    return QContent::InvalidId;
}

/*!
    Aborts the transfer identified by \a id. Does nothing if the request
    cannot be identified or if it has already finished.

    If the transfer is successfully aborted, transferFinished() is emitted
    with \c aborted set to \c true. Due to timing issues, it is possible that
    the transfer has already finished. In this case, the transfer is not aborted,
    and transferFinished() is emitted as normal.

    The default implementation does nothing.
*/
void FileTransferTask::abortTransfer(int)
{
}

/*!
    Generates and returns a new file transfer ID.

    Subclasses should use this to generate a file transfer ID that is unique
    across all file transfer classes that implement the FileTransferTask
    interface. The ID can then be used to emit the signals in this class.
 */
int FileTransferTask::nextTransferId()
{
    register int id;
    for (;;) {
        id = transferIdCounter;
        if (transferIdCounter.testAndSetOrdered(id, id + 1))
            break;
    }
    return id;
}

/*!
    \fn void FileTransferTask::incomingTransferStarted(int id, const QString &name, const QString &mimeType, const QString &description)

    This signal should be emitted by the subclass when an incoming file
    transfer has started. \a id should be a value that uniquely identifies the
    transfer, and \a name, \a mimeType should be set to the name and MIME type
    of the transferred object. \a description can be used to provide a
    user-friendly description of the transferred object, if necessary.

    \sa nextTransferId()
*/

/*!
    \fn void FileTransferTask::outgoingTransferStarted(int id, const QString &name, const QString &mimeType, const QString &description)

    This signal should be emitted by the subclass when an outgoing file
    transfer has started. \a id should be a value that uniquely identifies the
    transfer within this task, and \a name, \a mimeType should be set to the
    name and MIME type of the transferred object. \a description can be
    used to provide a user-friendly description of the transferred object, if
    necessary.

    \sa nextTransferId()
*/

/*!
    \fn void FileTransferTask::transferProgress(int id, qint64 done, qint64 total)

    This signal should be emitted by the subclass when some data has been
    sent or received for a file transfer. \a id is the value that identifies
    the transfer, \a done is the number of bytes that have been transferred
    so far, and \a total is the total number of bytes to be transferred.
    \a total can be set to 0 if the total is unknown.
*/

/*!
    \fn void FileTransferTask::transferFinished(int id, bool error, bool aborted);

    This signal should be emitted by the subclass when a transfer has
    finished. \a id is the value that identifies the transfer, \a error is
    \c true if an error occurred during the transfer (that is, the transfer
    was unsuccessful) and \a aborted is \c true if the transfer was
    canceled.
*/  // TODO i.e. ...canceled by a call to abortTransfer(), really (not necessarily canceled by other party?).
