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

#include "qmailfolder.h"
#include "qmailstore.h"
#include "qtopialog.h"

static quint64 synchronizationEnabledFlag = 0;
static quint64 synchronizedFlag = 0;

class QMailFolderPrivate : public QSharedData
{
public:
    QMailFolderPrivate() 
        : QSharedData(),
          status(0) 
    {
    }

    QMailFolderId id;
    QString name;
    QString displayName;
    QMailFolderId parentFolderId;
    QMailAccountId parentAccountId;
    quint64 status;

    static void initializeFlags()
    {
        static bool flagsInitialized = false;
        if (!flagsInitialized) {
            flagsInitialized = true;

            synchronizationEnabledFlag = registerFlag("SynchronizationEnabled");
            synchronizedFlag = registerFlag("Synchronized");
        }
    }

private:
    static quint64 registerFlag(const QString &name)
    {
        if (!QMailStore::instance()->registerFolderStatusFlag(name)) {
            qLog(Messaging) << "Unable to register folder status flag:" << name << "!";
        }

        return QMailFolder::statusMask(name);
    }
};

/*!
    \class QMailFolder
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailFolder class represents a folder for mail messages in the mail store.
    \ingroup messaginglibrary

    QMailFolder represents a folder of mail messages, either defined internally for
    application use, or to represent a folder object held by an external message
    service, such as an IMAP account.

    A QMailFolder object has an optional parent of the same type, allowing folders
    to be arranged in tree structures.  Messages may be associated with folders, 
    allowing for simple classification and access by their 
    \l{QMailMessage::parentFolderId()}{parentFolderId} property.

    \sa QMailMessage, QMailStore::folder()
*/

/*!
    \variable QMailFolder::SynchronizationEnabled

    The status mask needed for testing the value of the registered status flag named 
    \c "SynchronizationEnabled" against the result of QMailFolder::status().

    This flag indicates that a folder should be included during account synchronization.
*/

/*!
    \variable QMailFolder::Synchronized

    The status mask needed for testing the value of the registered status flag named 
    \c "Synchronized" against the result of QMailFolder::status().

    This flag indicates that a folder has been synchronized during account synchronization.
*/

/*!
    \enum QMailFolder::StandardFolder

    This enum type describes the standard standard storage folders.
    These folders cannot be removed or updated.

    \value InboxFolder Represents the standard inbox folder.
    \value OutboxFolder Represents the standard outbox folder.
    \value DraftsFolder Represents the standard drafts folder.
    \value SentFolder Represents the standard sent folder.
    \value TrashFolder Represents the standard trash folder.
*/


const quint64 &QMailFolder::SynchronizationEnabled = synchronizationEnabledFlag;
const quint64 &QMailFolder::Synchronized = synchronizedFlag;

/*!
  Constructor that creates an empty and invalid \c QMailFolder.
  An empty folder is one which has no name, no parent folder and no parent account.
  An invalid folder does not exist in the database and has an invalid id.
*/

QMailFolder::QMailFolder()
{
    d = new QMailFolderPrivate();
}

/*!
  Constructor that loads a standard QMailFolder specified by \a sf from the message store.
*/

QMailFolder::QMailFolder(const StandardFolder& sf)
{
    *this = QMailStore::instance()->folder(QMailFolderId(static_cast<quint64>(sf)));
}

/*!
  Constructor that creates a QMailFolder by loading the data from the message store as
  specified by the QMailFolderId \a id. If the folder does not exist in the message store, 
  then this constructor will create an empty and invalid QMailFolder.
*/

QMailFolder::QMailFolder(const QMailFolderId& id)
{
    *this = QMailStore::instance()->folder(id);
}


/*!
  Creates a QMailFolder object with name \a name and  parent folder ID \a parentFolderId,
  that is linked to a parent account \a parentAccountId.
*/

QMailFolder::QMailFolder(const QString& name, const QMailFolderId& parentFolderId, const QMailAccountId& parentAccountId)
    : d(new QMailFolderPrivate())
{
    d->name = name;
    d->parentFolderId = parentFolderId;
    d->parentAccountId = parentAccountId;
}

/*!
  Creates a copy of the \c QMailFolder object \a other.
*/

QMailFolder::QMailFolder(const QMailFolder& other)
{
    d = other.d;
}


/*!
  Destroys the \c QMailFolder object.
*/

QMailFolder::~QMailFolder()
{
}

/*!
  Assigns the value of the \c QMailFolder object \a other to this.
*/

QMailFolder& QMailFolder::operator=(const QMailFolder& other)
{
    d = other.d;
    return *this;
}

/*!
  Returns the \c ID of the \c QMailFolder object. A \c QMailFolder with an invalid ID
  is one which does not yet exist on the message store.
*/

QMailFolderId QMailFolder::id() const
{
    return d->id;
}

/*!
  Sets the ID of this folder to \a id
*/

void QMailFolder::setId(const QMailFolderId& id)
{
    d->id = id;
}

/*!
  Returns the name of the folder.
*/
QString QMailFolder::name() const
{
    return d->name;
}

/*!
  Sets the name of this folder to \a name.
*/

void QMailFolder::setName(const QString& name)
{
    d->name = name;
}

/*!
  Returns the display name of the folder.
*/
QString QMailFolder::displayName() const
{
    if (!d->displayName.isNull())
        return d->displayName;

    return d->name;
}

/*!
  Sets the display name of this folder to \a displayName.
*/

void QMailFolder::setDisplayName(const QString& displayName)
{
    d->displayName = displayName;
}

/*!
  Returns the ID of the parent folder. This folder is a root folder if the parent
  ID is invalid.
*/

QMailFolderId QMailFolder::parentId() const
{
    return d->parentFolderId;
}

/*!
 Sets the parent folder id to \a id. \bold{Warning}: it is the responsibility of the
 application to make sure that no circular folder refernces are created.
*/

void QMailFolder::setParentId(const QMailFolderId& id)
{
    d->parentFolderId = id;
}

/*!
  Returns the id of the account which owns the folder. If the folder
  is not linked to an account an invalid id is returned.
*/

QMailAccountId QMailFolder::parentAccountId() const
{
    return d->parentAccountId;
}

/*!
  Sets the id of the account which owns the folder to \a id.
*/

void QMailFolder::setParentAccountId(const QMailAccountId& id)
{
    d->parentAccountId = id;
}

/*! 
    Returns the status value for the folder.

    \sa setStatus(), statusMask()
*/
quint64 QMailFolder::status() const
{
    return d->status;
}

/*! 
    Sets the status value for the folder to \a newStatus.

    \sa status(), statusMask()
*/
void QMailFolder::setStatus(quint64 newStatus)
{
    d->status = newStatus;
}

/*! 
    Sets the status flags indicated in \a mask to \a set.

    \sa status(), statusMask()
*/
void QMailFolder::setStatus(quint64 mask, bool set)
{
    if (set)
        d->status |= mask;
    else
        d->status &= ~mask;
}

/*!
  Returns \c true if the folder is a root folder or \c false otherwise. A root folder
  is one which has not parent.
*/

bool QMailFolder::isRoot() const
{
    return (!d->parentFolderId.isValid());
}

/*!
    Returns the status bitmask needed to test the result of QMailFolder::status() 
    against the QMailFolder status flag registered with the identifier \a flagName.

    \sa status(), QMailStore::folderStatusMask()
*/
quint64 QMailFolder::statusMask(const QString &flagName)
{
    return QMailStore::instance()->folderStatusMask(flagName);
}

/*! \internal */
void QMailFolder::initStore()
{
    QMailFolderPrivate::initializeFlags();
}

