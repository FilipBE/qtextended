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

#include "qmailmessagekey.h"
#include "qmailmessagekey_p.h"

#include <QMailAccountKey>
#include <QMailFolderKey>


using namespace QMailDataComparator;

/*!
    \class QMailMessageKey
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailMessageKey class defines the parameters used for querying a subset of
    all available messages from the mail store.
    \ingroup messaginglibrary

    A QMailMessageKey is composed of a message property, an optional comparison operator
    and a comparison value. The QMailMessageKey class is used in conjunction with the 
    QMailStore::queryMessages() and QMailStore::countMessages() functions to filter results 
    which meet the criteria defined by the key.

    QMailMessageKeys can be combined using the logical operators (&), (|) and (~) to
    create more refined queries.

    For example:

    To create a query for all messages sent from "joe@user.com" with subject "meeting":
    \code
    QMailMessageKey subjectKey(QMailMessageKey::Subject,"meeting");
    QMailMessageKey senderKey(QMailMessageKey::Sender,"joe@user.com")
    QMailMessageIdList results = QMailStore::instance()->queryMessages(subjectKey & senderKey);
    \endcode

    To query all unread messages from a specific folder:
    \code
    QMailMessageIdList unreadMessagesInFolder(const QMailFolderId& folderId)
    {
        QMailMessageKey parentFolderKey(QMailMessageKey::ParentFolderId, folderId);
        QMailMessageKey unreadKey(QMailMessageKey::Status, QMailMessage::Read, QMailDataComparator::Excludes);

        return QMailStore::instance()->queryMessages(parentFolderKey & unreadKey);
    }
    \endcode

    \sa QMailStore, QMailMessage
*/

/*!
    \enum QMailMessageKey::Property

    This enum type describes the data query properties of a QMailMessage.

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
    \value Size The size of the message.
    \value ParentAccountId The ID of the account the message was downloaded from.
    \value AncestorFolderIds The set of IDs of folders which are direct or indirect parents of this message.
    \value ContentType The type of data contained within the message.
    \value PreviousParentFolderId The parent folder ID this message was contained in, prior to moving to the current parent folder.
*/

/*!
    \typedef QMailMessageKey::IdType
    \internal
*/

/*!
    Constructor which creates a default-constructed key (one for which isEmpty() returns true) 
    that matches all messages. The logical negation of an empty key also matches all messages.

    The result of combining an empty key with a non-empty key is the same as the original 
    non-empty key. This is true regardless of whether the combination is formed by a 
    logical AND or a logical OR operation.

    The result of combining two empty keys is an empty key.
*/

QMailMessageKey::QMailMessageKey()
{
    d = new QMailMessageKeyPrivate;
}

/*!
    Construct a QMailMessageKey which defines a query parameter where
    QMailMessage::Property \a p is compared using comparison operator
    \a c with a value \a value.
*/

QMailMessageKey::QMailMessageKey(Property p, const QVariant& value, QMailDataComparator::Comparator c)
{
    sanityCheck(p, value, c);

    d = new QMailMessageKeyPrivate;

    // Convert Excludes to negated Includes
    if (c == QMailDataComparator::Excludes) {
        d->negated = true;
        c = QMailDataComparator::Includes;
    }

    d->arguments.append(QMailMessageKeyPrivate::Argument(p, c, value));
}

/*!
    Construct a QMailMessageKey which defines a query parameter where messages 
    whose identifiers are in \a ids are returned.
*/

QMailMessageKey::QMailMessageKey(const QMailMessageIdList& ids)
{
    if (ids.isEmpty()) {
        *this = nonMatchingKey();
    } else {
        d = new QMailMessageKeyPrivate;
        d->arguments.append(QMailMessageKeyPrivate::Argument(QMailMessageKey::Id, QMailDataComparator::Equal, ids));
    }
}

/*!
    Create a copy of the QMailMessageKey \a other.
*/

QMailMessageKey::QMailMessageKey(const QMailMessageKey& other)
{
    d = other.d;
}


/*!
    Destroys the QMailMessageKey
*/

QMailMessageKey::~QMailMessageKey()
{
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false.

    \sa isNonMatching()
*/

bool QMailMessageKey::isEmpty() const
{
    return d->combiner == None &&
           d->negated == false &&
           d->subKeys.isEmpty() && 
           d->arguments.isEmpty();
}

/*!
    Returns true if the key is a non-matching key; otherwise returns false.

    \sa isEmpty()
*/

bool QMailMessageKey::isNonMatching() const
{
    return (*this == nonMatchingKey());
}

/*!
    Returns a key that is the logical NOT of the value of this key.
*/

QMailMessageKey QMailMessageKey::operator~() const
{
    QMailMessageKey k(*this);
    if(!isEmpty())
        k.d->negated = !d->negated;
    return k;
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailMessageKey QMailMessageKey::operator&(const QMailMessageKey& other) const
{
    if (isEmpty() || other.isNonMatching())
        return other;
    else if (isNonMatching() || other.isEmpty())
        return *this;

    QMailMessageKey k;
    k.d->combiner = And;

    if(d->combiner != Or && !d->negated && other.d->combiner != Or && !other.d->negated)
    {
        k.d->subKeys = d->subKeys + other.d->subKeys;
        k.d->arguments = d->arguments + other.d->arguments;
    }
    else
    {
        k.d->subKeys.append(*this);
        k.d->subKeys.append(other); 
    }
    return k;            
}

/*!
    Returns a key that is the logical OR of this key and the value of key \a other.
*/

QMailMessageKey QMailMessageKey::operator|(const QMailMessageKey& other) const
{
    if (isEmpty() || isNonMatching())
        return other;
    else if (other.isEmpty() || other.isNonMatching())
        return *this;

    QMailMessageKey k;
    k.d->combiner = Or;
    if(d->combiner != And && !d->negated && other.d->combiner != And && !other.d->negated)
    {
        k.d->subKeys = d->subKeys + other.d->subKeys;
        k.d->arguments = d->arguments + other.d->arguments;
    }
    else
    {
        k.d->subKeys.append(*this);    
        k.d->subKeys.append(other);
    }

    return k;
}

/*!
    Performs a logical AND with this key and the key \a other and assigns the result
    to this key.
*/

QMailMessageKey& QMailMessageKey::operator&=(const QMailMessageKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Performs a logical OR with this key and the key \a other and assigns the result
    to this key.
*/

QMailMessageKey& QMailMessageKey::operator|=(const QMailMessageKey& other) 
{
    *this = *this | other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailMessageKey::operator==(const QMailMessageKey& other) const
{
    return d->negated == other.d->negated &&
           d->combiner == other.d->combiner &&
           d->subKeys == other.d->subKeys && 
           d->arguments == other.d->arguments;
}

/*!
    Returns \c true if the value of this key is not the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailMessageKey::operator!=(const QMailMessageKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailMessageKey \a other to this.
*/

QMailMessageKey& QMailMessageKey::operator=(const QMailMessageKey& other)
{
    d = other.d;
    return *this;
}

/*!
  Returns the QVariant representation of this QMailMessageKey. 
*/

QMailMessageKey::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn QMailMessageKey::serialize(Stream &stream) const

    Writes the contents of a QMailMessageKey to a \a stream.
*/

template <typename Stream> void QMailMessageKey::serialize(Stream &stream) const
{
    stream << d->combiner;          
    stream << d->negated;

    stream << d->arguments.count();
    foreach (const QMailMessageKeyPrivate::Argument& a, d->arguments)
        a.serialize(stream);

    stream << d->subKeys.count();
    foreach (const QMailMessageKey& k, d->subKeys)
        k.serialize(stream);
}

/*!
    \fn QMailMessageKey::deserialize(Stream &stream)

    Reads the contents of a QMailMessageKey from \a stream.
*/

template <typename Stream> void QMailMessageKey::deserialize(Stream &stream)
{
    int enumInt = 0;
    stream >> enumInt;
    d->combiner = static_cast<QMailDataComparator::Combiner>(enumInt);
    stream >> d->negated;

    int argumentCount = 0;
    stream >> argumentCount;
    for (int i = 0; i < argumentCount; ++i) {
        QMailMessageKeyPrivate::Argument a;
        a.deserialize(stream);
        d->arguments.append(a);
    }

    int subKeyCount = 0;
    stream >> subKeyCount;
    for (int i = 0; i < subKeyCount; ++i) {
        QMailMessageKey subKey;
        subKey.deserialize(stream);
        d->subKeys.append(subKey);
    }
}

/*!
    Returns a key that does not match any messages (unlike an empty key).

    \sa isNonMatching(), isEmpty()
*/
QMailMessageKey QMailMessageKey::nonMatchingKey()
{
    return QMailMessageKey(QMailMessageKey::Id, QMailMessageId(), QMailDataComparator::Equal);
}

/*! \internal */
void QMailMessageKey::sanityCheck(QMailMessageKey::Property p, const QVariant &value, QMailDataComparator::Comparator) 
{
    switch (p) 
    {
    case QMailMessageKey::Id:

        // The value must be either a QMailMessageId, list or key
        if (!qVariantCanConvert<QMailMessageId>(value) && 
            !qVariantCanConvert<QMailMessageIdList>(value) &&
            !qVariantCanConvert<QMailMessageKey>(value))
            qWarning() << "QMailMessageKey - Invalid message ID comparison!";
        break;

    case QMailMessageKey::ParentFolderId:
    case QMailMessageKey::AncestorFolderIds:
    case QMailMessageKey::PreviousParentFolderId:

        // The value must be either a QMailFolderId, list or key
        if (!qVariantCanConvert<QMailFolderId>(value) && 
            !qVariantCanConvert<QMailFolderIdList>(value) &&
            !qVariantCanConvert<QMailFolderKey>(value))
            qWarning() << "QMailMessageKey - Invalid folder ID comparison!";
        break;

    case QMailMessageKey::Type:
    case QMailMessageKey::Status:
    case QMailMessageKey::ContentType:
    case QMailMessageKey::Size:

        // The value must be an int
        if (!qVariantCanConvert<int>(value))
            qWarning() << "QMailMessageKey - Invalid int comparison!";
        break;

    case QMailMessageKey::Sender:
    case QMailMessageKey::Recipients:
    case QMailMessageKey::Subject:
    case QMailMessageKey::FromMailbox:
    case QMailMessageKey::ServerUid:

        // The value must be a string
        if (!qVariantCanConvert<QString>(value))
            qWarning() << "QMailMessageKey - Invalid string comparison!";
        break;

    case QMailMessageKey::TimeStamp:

        // The value must be a string
        if (!qVariantCanConvert<QDateTime>(value))
            qWarning() << "QMailMessageKey - Invalid timestamp comparison!";
        break;

    case QMailMessageKey::ParentAccountId:

        // The value must be either a QMailAccountId, list or key
        if (!qVariantCanConvert<QMailAccountId>(value) && 
            !qVariantCanConvert<QMailAccountIdList>(value) &&
            !qVariantCanConvert<QMailAccountKey>(value))
            qWarning() << "QMailMessageKey - Invalid account ID comparison!";
        break;
    }
}

Q_IMPLEMENT_USER_METATYPE(QMailMessageKey);

