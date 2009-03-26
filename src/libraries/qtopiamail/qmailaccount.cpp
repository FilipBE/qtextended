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

#include "qmailaccount.h"
#include "qmailid.h"
#include "qmailstore.h"
#include "qmailmessageremovalrecord.h"
#include <private/accountconfiguration_p.h>

// Temporary:
#include "qmailstore_p.h"

#include <qtimer.h>
#include <qsettings.h>
#include <qtopialog.h>
#include <qmailcodec.h>
#include <QApplication>
#include <QDir>
#include <QMailFolder>


class QMailAccountPrivate : public QSharedData
{
public:
    QMailAccountPrivate() : QSharedData(),
                            _accountType(QMailAccount::POP)
    {};

    ~QMailAccountPrivate()
    {
    }

    QMailAccountId _id;
    QMailAccount::AccountType _accountType;
    QString _accountName;
};

/*!
    \class QMailAccount
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailAccount class represents a messaging account in the mail store.

    A QMailAccount is a logical entity that groups messages according to the
    method by which they are sent and received.  An account can be configured
    to support one more message sources, from which messages are imported into
    the mail store, and one or more message sinks by which messages are transmitted
    to external messaging services.  Although an account can support multiple 
    source or sinks, this facility is for grouping those that are logically equivalent;
    for example, using one of multiple connectivity options to retrieve messages from 
    the same external server.

    The QMailAccount class is used for accessing properties of the account related
    to dealing with the account's folders and messages, rather than for modifying 
    the account itself.  The AccountConfiguration class allows for the configuration 
    of the account itself to be modified.  A newly created account must also
    have an AccountConfiguration defined, in order to be used for transfer of
    messages to or from Qt Extended.

    QMailAccount allows the communications properties of the account to be tested.
    The canSendMail() and canCollectMail() functions allow the basic transfer
    capabilities of the account to be queried.  The messageSources() and messageSinks() 
    functions return the protocol tags for each message source or message sink 
    implementations configured for the account.  These tags can be used to identify the 
    implementation details of the account if necessary:

    \code 
    void someFunction(const QMailMessage &message) 
    {
        QMailAccount msgAccount(message.parentAccountId());
        if (msgAccount.messageSources().contains("imap4", Qt::CaseInsensitive)) {
            // This account uses IMAP
            ...
        }
    }
    \endcode

    The QMailAccount class also provides functions which help clients to access 
    the resources of the account.  The mailboxes() function returns a list of 
    each folder associated with the account, while the getMailbox() function
    allows a mailbox to be located by name.  The deletedMessages() and serverUids() 
    functions are primarily used in synchronizing the account's contents with 
    those present on an external server.

    \sa AccountConfiguration, QMailStore::account()
*/


/*!
    Creates an uninitialised account object.
*/
QMailAccount::QMailAccount()
{
    d = new QMailAccountPrivate;
}

/*!
  Convenience constructor that creates a \c QMailAccount by loading the data from the store as
  specified by the QMailAccountId \a id. If the account does not exist in the store, then this constructor
  will create an empty and invalid QMailAccount.
*/

QMailAccount::QMailAccount(const QMailAccountId& id)
{
    d = new QMailAccountPrivate;
    *this = QMailStore::instance()->account(id);
}

/*!
    Creates a copy of the QMailAccount \a other.
*/

QMailAccount::QMailAccount(const QMailAccount& other)
{
    d = other.d;
}

/*!
   Assigns the value of this account to the account \a other
*/

QMailAccount& QMailAccount::operator=(const QMailAccount& other)
{
    if(&other != this)
        d = other.d;
    return *this;
}

/*!
  Destroys the account object.
*/
QMailAccount::~QMailAccount()
{
}

/*!
  Returns the name of the account for display purposes.

  \sa setAccountName()
*/
QString QMailAccount::accountName() const
{
    return qApp->translate( "AccountList", d->_accountName.toLatin1() );
}

/*!
  Sets the name of the account for display purposes to \a str.

  \sa accountName()
*/
void QMailAccount::setAccountName(const QString &str)
{
    d->_accountName = str;
}

/*!
  Returns the account type.

  \sa setAccountType()
*/
QMailAccount::AccountType QMailAccount::accountType() const
{
    return d->_accountType;
};

/*!
  Sets the account type to \a at.

  \sa accountType()
*/
void QMailAccount::setAccountType(AccountType at)
{
    d->_accountType = at;
};

/*!
  Returns true if the account can be used to collect mail. That is if it is an
  IMAP or POP account; otherwise returns false.
*/
bool QMailAccount::canCollectMail() const
{
    if (accountType() == QMailAccount::POP || accountType() == QMailAccount::IMAP)
        return true;

    return false;
}

/*!
  Returns true if the account can be used to send mail.
*/
bool QMailAccount::canSendMail() const
{
    //email accounts can send only if smtp servers and email addresses have been defined.
    if (accountType() == QMailAccount::POP || 
        accountType() == QMailAccount::IMAP ||
        accountType() == QMailAccount::MMS ||
        accountType() == QMailAccount::Collective) {
        AccountConfiguration config(d->_id);
        return config.canSendMail();
    }

    return (accountType() != QMailAccount::System);
}

/*!
    Return the list of messages removed from this account.
*/
const QStringList QMailAccount::deletedMessages() const
{
    QStringList serverUidList;

    foreach (const QMailMessageRemovalRecord& r, QMailStore::instance()->messageRemovalRecords(id()))
        serverUidList.append(r.serverUid());

    return serverUidList;
}

/*!
    Return the list of messages removed from the mailbox identified by \a folderId.
*/
const QStringList QMailAccount::deletedMessages(const QMailFolderId &folderId) const
{
    QStringList serverUidList;

    QMailFolder folder(folderId);
    foreach (const QMailMessageRemovalRecord& r, QMailStore::instance()->messageRemovalRecords(id(), folder.name()))
        serverUidList.append(r.serverUid());

    return serverUidList;
}

/*!
  Returns the list of server UIDs for messages owned by this account.
*/

const QStringList QMailAccount::serverUids() const
{
    return serverUids(QMailMessageKey(QMailMessageKey::ParentAccountId, id()));
}

/*!
    Returns the list of message server UIDs for this account, that are located in the folder identified by \a folderId.
*/

const QStringList QMailAccount::serverUids(const QMailFolderId &folderId) const
{
    // We need to list both the messages in the mailbox, and those moved to the
    // Trash folder which are still in the mailbox as far as the server is concerned
    return serverUids(messagesKey(folderId) | trashKey(folderId));
}

/*!
    Returns the list of message server UIDs located in the folder identified by 
    \a folderId, whose status field yields \a set when logically ANDed with \a messageStatusFilter.
*/

const QStringList QMailAccount::serverUids(const QMailFolderId &folderId, quint64 messageStatusFilter, bool set) const
{
    QMailMessageKey statusKey(QMailMessageKey::Status, messageStatusFilter, QMailDataComparator::Includes);
    return serverUids((messagesKey(folderId) | trashKey(folderId)) & (set ? statusKey : ~statusKey));
}

/*!
  Returns the subfolder with the given \a name if any; other returns an invalid ID.

  \sa mailboxes()
*/
QMailFolderId QMailAccount::getMailbox(const QString &name)
{
    QMailFolderKey key(QMailFolderKey::ParentAccountId, d->_id);
    key &= QMailFolderKey(QMailFolderKey::Name, name);

    QMailFolderIdList folderIds = QMailStore::instance()->queryFolders(key);
    if (folderIds.count() == 1)
        return folderIds.first();
    
    return QMailFolderId();
}

/*!
  Returns the list of identifiers for the mailboxes of this account.

  \sa getMailbox()
*/
const QMailFolderIdList QMailAccount::mailboxes() const
{
    QMailFolderKey key(QMailFolderKey::ParentAccountId, d->_id);
    QMailFolderSortKey sortKey(QMailFolderSortKey::Name, Qt::AscendingOrder);

    return QMailStore::instance()->queryFolders(key, sortKey);
}

/*!
  Returns the storage id for this account.
 */
QMailAccountId QMailAccount::id() const
{
    return d->_id;
}

/*!
  Sets the storage id for this account to \a id.
 */

void QMailAccount::setId(const QMailAccountId& id)
{
    d->_id = id;
}

/*!
  Returns true if the account has configuration file settings; otherwise
  false. Currently only returns false for SMS accounts.

  \sa saveSettings(), readSettings()
*/
bool QMailAccount::hasSettings() const
{
    // TODO: should probably return false for Collective, also...
    switch( accountType() ) {
        case QMailAccount::SMS:
            return false;
        default:
            return true;
    }
}

/*!
  Serialize the account information to the current group of
  the given QSettings file \a conf.

  \sa readSettings(), hasSettings()
*/
void QMailAccount::saveSettings(QSettings *conf) const
{
    conf->setValue("accountname", d->_accountName );
    conf->setValue("type", d->_accountType );
}

/*!
    Deserialize the account information from the current group of
    the given QSettings file \a conf.

  \sa saveSettings(), hasSettings()
*/
void QMailAccount::readSettings(QSettings *conf)
{
    d->_accountName = conf->value("accountname").toString();
    d->_accountType = static_cast<AccountType>( conf->value("type", POP ).toInt() );
}

/*!
  Returns the name of the account as suitable for displaying in the user
  interface. Currently simply returns the account name.
*/
QString QMailAccount::displayName() const
{
    return accountName();
};

/*!
  Returns the storage key used to query for messages owned by this account.
*/
QMailMessageKey QMailAccount::messagesKey() const
{
    QMailMessageKey accountKey(QMailMessageKey::ParentAccountId,id());
    QMailMessageKey trashFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::TrashFolder));

    return (accountKey & ~trashFolderKey);
}

/*!
  Returns the storage key used to query for messages moved from this account's folders to the Trash folder.
*/
QMailMessageKey QMailAccount::trashKey() const
{
    QMailMessageKey trashFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::TrashFolder));
    QMailMessageKey accountKey(QMailMessageKey::ParentAccountId, id());

    return (trashFolderKey & accountKey);
}

/*!
  Returns the storage key used to query for messages owned by this account, in the mailbox folder identified by \a folderId.
*/
QMailMessageKey QMailAccount::messagesKey(const QMailFolderId &folderId) const
{
    QMailMessageKey accountKey(QMailMessageKey::ParentAccountId, id());
    QMailMessageKey mailboxKey(QMailMessageKey::ParentFolderId, folderId);

    return (accountKey & mailboxKey);
}

/*!
  Returns the storage key used to query for messages moved from the mailbox folder identified by \a folderId to the Trash folder.
*/
QMailMessageKey QMailAccount::trashKey(const QMailFolderId &folderId) const
{
    QMailMessageKey trashFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::TrashFolder));
    QMailMessageKey accountKey(QMailMessageKey::ParentAccountId, id());
    QMailMessageKey mailboxKey(QMailMessageKey::PreviousParentFolderId, folderId);

    return (trashFolderKey & accountKey & mailboxKey);
}

/*! \internal */
QStringList QMailAccount::serverUids(QMailMessageKey key) const
{
    QStringList uidList;

    foreach (const QMailMessageMetaData& r, QMailStore::instance()->messagesMetaData(key, QMailMessageKey::ServerUid))
        uidList.append(r.serverUid());

    return uidList;
}

/*!
  Returns the types of messsages this account deals with.
*/
QMailMessage::MessageType QMailAccount::messageType() const
{
    switch (accountType())
    {
        case QMailAccount::POP:
        case QMailAccount::IMAP:
            return QMailMessage::Email;
        case QMailAccount::SMS:
            return QMailMessage::Sms;
        case QMailAccount::MMS:
            return QMailMessage::Mms;
        case QMailAccount::Collective:
            return QMailMessage::Instant;
        case QMailAccount::System:
            return QMailMessage::System;
    }

    return QMailMessage::None;
}

/*!
    Returns the list of protocol tags identifying the message source implementations
    that provide the messages for this account.
*/
QStringList QMailAccount::messageSources() const
{
    QStringList result;

    if (accountType() == QMailAccount::IMAP) {
        result.append("imap4");
    } else if (accountType() == QMailAccount::POP) {
        result.append("pop3");
    } else if (accountType() == QMailAccount::SMS) {
        result.append("sms");
    } else if (accountType() == QMailAccount::MMS) {
        result.append("mms");
    } else if (accountType() == QMailAccount::Collective) {
        // What goes here?
        result.append("jabber");
    } else if (accountType() == QMailAccount::System) {
        result.append("qtopia-system");
    }

    return result;
}

/*! 
    Returns true if this account has incoming messages sourced from external messaging services.
*/
bool QMailAccount::isMessageSource() const
{
    return !messageSources().isEmpty();
}

/*!
    Returns the list of protocol tags identifying the message sink implementations
    that can transmit messages for this account.
*/
QStringList QMailAccount::messageSinks() const
{
    QStringList result;

    if ((accountType() == QMailAccount::IMAP) || (accountType() == QMailAccount::POP)) {
        result.append("smtp");
    } else if (accountType() == QMailAccount::SMS) {
        result.append("sms");
    } else if (accountType() == QMailAccount::MMS) {
        result.append("mms");
    } else if (accountType() == QMailAccount::Collective) {
        // What goes here?
        result.append("jabber");
    }

    return result;
}

/*! 
    Returns true if this account can transmit outgoing messages to external messaging services.
*/
bool QMailAccount::isMessageSink() const
{
    return !messageSinks().isEmpty();
}

/*! \internal */
QMailAccount QMailAccount::accountFromSource(const QString &sourceType)
{
    QMailAccount account;
    
    const QString type(sourceType.toLower());
    if (type == "imap4") {
        account.setAccountType(QMailAccount::IMAP);
    } else if (type == "pop3") {
        account.setAccountType(QMailAccount::POP);
    } else if (type == "sms") {
        account.setAccountType(QMailAccount::SMS);
    } else if (type == "mms") {
        account.setAccountType(QMailAccount::MMS);
    } else if (type == "jabber") {
        account.setAccountType(QMailAccount::Collective);
    } else if (type == "qtopia-system") {
        account.setAccountType(QMailAccount::System);
    } else {
        qWarning() << "Unable to create account! Unknown account source type:" << sourceType;
    }

    return account;
}

/*! \internal */
QList<int> QMailAccount::matchingAccountTypes(QMailMessage::MessageType type)
{
    QList<int> accountTypes;

    switch (type)
    {
    case QMailMessage::Email:
        accountTypes.append(QMailAccount::POP);
        accountTypes.append(QMailAccount::IMAP);
        break;

    case QMailMessage::Sms:
        accountTypes.append(QMailAccount::SMS);
        break;

    case QMailMessage::Mms:
        accountTypes.append(QMailAccount::MMS);
        break;

    case QMailMessage::Instant:
        accountTypes.append(QMailAccount::Collective);
        break;

    case QMailMessage::System:
        accountTypes.append(QMailAccount::System);
        break;

    case QMailMessage::None:
    case QMailMessage::AnyType:
        break;
    }

    return accountTypes;
}

