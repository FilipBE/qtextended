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

#include "qmailmessagesortkey.h"
#include "qmailmessagesortkey_p.h"

/*!
    \class QMailMessageSortKey
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailMessageSortKey class defines the parameters used for sorting a subset of 
    queried messages from the mail store.
    \ingroup messaginglibrary

    A QMailMessageSortKey is composed of a message property to sort and a sort order. 
    The QMailMessageSortKey class is used in conjunction with the QMailStore::queryMessages() 
    function to sort message results according to the criteria defined by the sort key.

    For example:
    To create a query for all messages sorted by their timestamp in decending order:
    \code
    QMailMessageSortKey sortArrivalTimeKey(QMailMessageSortKey::TimeStamp,Qt::DescendingOrder);
    QMailIdList results = QMailStore::instance()->queryMessages(QMailMessageKey(),sortArrivalTimeKey);
    \endcode
    
    \sa QMailStore, QMailMessageKey
*/

/*!
    \enum QMailMessageSortKey::Property

    This enum type describes the sortable data properties of a QMailFolder.

    \value Id The ID of the message.
    \value Type The type of the message.
    \value ParentFolderId The parent folder ID this message is contained in.
    \value Sender The message sender address string.
    \value Recipients The message recipient address string.
    \value Subject The message subject string.
    \value TimeStamp The message timestamp
    \value Status The message status flags.
    \value FromMailbox The imap mailbox the message was downloaded from. 
    \value ServerUid The IMAP server UID of the message.
    \value ParentAccountId The ID of the account the mesasge was downloaded from.
    \value Size The size of the message.
    \value ContentType The type of data contained within the message.
    \value PreviousParentFolderId The parent folder ID this message was contained in, prior to moving to the current parent folder.
*/

/*!
    Create a QMailMessageSortKey with specifying matching parameters.

    A default-constructed key (one for which isEmpty() returns true) sorts no messages. 

    The result of combining an empty key with a non-empty key is the same as the original 
    non-empty key.

    The result of combining two empty keys is an empty key.
*/

QMailMessageSortKey::QMailMessageSortKey()
{
    d = new QMailMessageSortKeyPrivate();
}

/*!
    Construct a QMailMessageSortKey which sorts a set of results based on the  
    QMailMessageSortKey::Property \a p and the Qt::SortOrder \a order 
*/

QMailMessageSortKey::QMailMessageSortKey(Property p, Qt::SortOrder order)
{
    d = new QMailMessageSortKeyPrivate();
    QMailMessageSortKeyPrivate::Argument a(p,order);
    d->arguments.append(a);
}

/*!
    Create a copy of the QMailMessageSortKey \a other.
*/

QMailMessageSortKey::QMailMessageSortKey(const QMailMessageSortKey& other)
{
    d = other.d;
}

/*!
    Destroys this QMailMessageSortKey.
*/

QMailMessageSortKey::~QMailMessageSortKey()
{
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailMessageSortKey QMailMessageSortKey::operator&(const QMailMessageSortKey& other) const
{
    QMailMessageSortKey k;
    k.d->arguments = d->arguments + other.d->arguments;
    return k;
}

/*!
    Performs a logical AND with this key and the key \a other and assigns the result
    to this key.
*/

QMailMessageSortKey& QMailMessageSortKey::operator&=(const QMailMessageSortKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailMessageSortKey::operator==(const QMailMessageSortKey& other) const
{
    return d->arguments == other.d->arguments;
}
/*!
    Returns \c true if the value of this key is not the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailMessageSortKey::operator!=(const QMailMessageSortKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailMessageSortKey \a other to this.
*/

QMailMessageSortKey& QMailMessageSortKey::operator=(const QMailMessageSortKey& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false.
*/

bool QMailMessageSortKey::isEmpty() const
{
    return d->arguments.isEmpty();
}



