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

#include "qmailaccountkey.h"
#include "qmailaccountkey_p.h"

using namespace QMailDataComparator;

/*!
    \class QMailAccountKey
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailAccountKey class defines the parameters used for querying a subset of 
    all available accounts from the mail store.
    \ingroup messaginglibrary

    A QMailAccountKey is composed of an account property, an optional comparison operator
    and a comparison value. The QMailAccountKey class is used in conjunction with the 
    QMailStore::queryAccounts() and QMailStore::countAccounts() functions to filter results 
    which meet the criteria defined by the key.

    QMailAccountKey's can be combined using the logical operators (&), (|) and (~) to 
    build more sophisticated queries.
    
    For example:

    To create a query for all email accounts:
    \code
    QMailAccountKey emailKey(QMailAccountKey::MessageType, QMailMessage::Email);
    QMailAccountIdList results = QMailStore::instance()->queryAccounts(emailKey);
    \endcode
    
    To query all accounts handling SMS or MMS messages:
    \code
    QMailAccountKey mmsAccount(QMailAccountKey::MessageType, QMailMessage::Mms);
    QMailAccountKey smsAccount(QMailAccountKey::MessageType, QMailMessage::Sms);
    QMailAccountIdList results = QMailStore::instance()->queryAccounts(mmsAccount | smsAccount);
    \endcode 
    
    \sa QMailStore, QMailAccount
*/

/*!
    \enum QMailAccountKey::Property

    This enum type describes the queryable data properties of a QMailAccount.

    \value Id The ID of the account.
    \value Name The name of the account.
    \value MessageType The type of messages handled by the account.
    \value EmailAddress The email address associated with this account.

*/

/*!
    \typedef QMailAccountKey::IdType
    \internal
*/

Q_IMPLEMENT_USER_METATYPE(QMailAccountKey);

/*!
    Create a QMailAccountKey with specifying matching parameters.

    A default-constructed key (one for which isEmpty() returns true) matches all accounts. 
    The logical negation of an empty key also matches all accounts.

    The result of combining an empty key with a non-empty key is the same as the original 
    non-empty key. This is true regardless of whether the combination is formed by a 
    logical AND or a logical OR operation.

    The result of combining two empty keys is an empty key.
*/

QMailAccountKey::QMailAccountKey()
{
    d = new QMailAccountKeyPrivate();
}


/*!
    Construct a QMailAccountKey which defines a query parameter where
    QMailAccount::Property \a p is compared using comparison operator
    \a c with a value \a value.
*/

QMailAccountKey::QMailAccountKey(Property p, const QVariant& value, QMailDataComparator::Comparator c)
{
    sanityCheck(p, value, c);

    d = new QMailAccountKeyPrivate();

    // Convert Excludes to negated Includes
    if (c == QMailDataComparator::Excludes) {
        d->negated = true;
        c = QMailDataComparator::Includes;
    }

    d->arguments.append(QMailAccountKeyPrivate::Argument(p, c, value));
}

/*!
    Construct a QMailAccountKey which defines a query parameter where
    folder id's matching those in \a ids are returned.
*/

QMailAccountKey::QMailAccountKey(const QMailAccountIdList& ids)
{
    if (ids.isEmpty()) {
        *this = nonMatchingKey();
    } else {
        d = new QMailAccountKeyPrivate();
        d->arguments.append(QMailAccountKeyPrivate::Argument(QMailAccountKey::Id, QMailDataComparator::Equal, ids));
    }
}

/*!
    Create a copy of the QMailAccountKey \a other.
*/

QMailAccountKey::QMailAccountKey(const QMailAccountKey& other)
{
    d = other.d;
}


/*!
    Destroys this QMailAccountKey.
*/


QMailAccountKey::~QMailAccountKey()
{
}

/*!
    Returns a key that is the logical NOT of the value of this key.
*/

QMailAccountKey QMailAccountKey::operator~() const
{
    QMailAccountKey k(*this);
    if(!k.isEmpty())
        k.d->negated = !d->negated;
    return k;
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailAccountKey QMailAccountKey::operator&(const QMailAccountKey& other) const
{
    if (isEmpty() || other.isNonMatching())
        return other;
    else if (isNonMatching() || other.isEmpty())
        return *this;

    QMailAccountKey k;
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

QMailAccountKey QMailAccountKey::operator|(const QMailAccountKey& other) const
{
    if (isEmpty() || isNonMatching())
        return other;
    else if (other.isEmpty() || other.isNonMatching())
        return *this;

    QMailAccountKey k;
    k.d->combiner = Or;

    if(d->combiner != And && 
       !d->negated && 
       other.d->combiner != And && 
       !other.d->negated)
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

QMailAccountKey& QMailAccountKey::operator&=(const QMailAccountKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Performs a logical OR with this key and the key \a other and assigns the result
    to this key.
*/

QMailAccountKey& QMailAccountKey::operator|=(const QMailAccountKey& other) 
{
    *this = *this | other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailAccountKey::operator==(const QMailAccountKey& other) const
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

bool QMailAccountKey::operator!=(const QMailAccountKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailAccountKey \a other to this.
*/

QMailAccountKey& QMailAccountKey::operator=(const QMailAccountKey& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false. 

    \sa isNonMatching()
*/

bool QMailAccountKey::isEmpty() const
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

bool QMailAccountKey::isNonMatching() const
{
    return (*this == nonMatchingKey());
}

/*!
  Returns the QVariant representation of this QMailAccountKey. 
*/

QMailAccountKey::operator QVariant() const
{
	return QVariant::fromValue(*this);
}

/*!
    \fn QMailAccountKey::serialize(Stream &stream) const

    Writes the contents of a QMailAccountKey to a \a stream.
*/

template <typename Stream> void QMailAccountKey::serialize(Stream &stream) const
{
    stream << d->combiner;          
    stream << d->negated;

    stream << d->arguments.count();
    foreach (const QMailAccountKeyPrivate::Argument& a, d->arguments)
        a.serialize(stream);

    stream << d->subKeys.count();
    foreach (const QMailAccountKey& k, d->subKeys)
        k.serialize(stream);
}

/*!
    \fn QMailAccountKey::deserialize(Stream &stream)

    Reads the contents of a QMailAccountKey from \a stream.
*/

template <typename Stream> void QMailAccountKey::deserialize(Stream &stream)
{
    int enumInt = 0;
    stream >> enumInt;
    d->combiner = static_cast<QMailDataComparator::Combiner>(enumInt);
    stream >> d->negated;

    int argumentCount = 0;
    stream >> argumentCount;
    for(int i = 0; i < argumentCount; ++i) {
        QMailAccountKeyPrivate::Argument a;
        a.deserialize(stream);
        d->arguments.append(a);
    }

    int subKeyCount = 0;
    stream >> subKeyCount;
    for(int i = 0; i < subKeyCount; ++i) {
        QMailAccountKey subKey;
        subKey.deserialize(stream);
        d->subKeys.append(subKey);
    }
}

/*!
    Returns a key that does not match any accounts (unlike an empty key).

    \sa isNonMatching(), isEmpty()
*/
QMailAccountKey QMailAccountKey::nonMatchingKey()
{
    return QMailAccountKey(QMailAccountKey::Id, QMailAccountId(), QMailDataComparator::Equal);
}

/*! \internal */
void QMailAccountKey::sanityCheck(QMailAccountKey::Property p, const QVariant &value, QMailDataComparator::Comparator) 
{
    switch (p) 
    {
    case QMailAccountKey::Id:

        // The value must be either a QMailFolderId, list or key
        if (!qVariantCanConvert<QMailAccountId>(value) && 
            !qVariantCanConvert<QMailAccountIdList>(value) &&
            !qVariantCanConvert<QMailAccountKey>(value))
            qWarning() << "QMailAccountKey - Invalid folder ID comparison!";
        break;

    case QMailAccountKey::Name:
    case QMailAccountKey::EmailAddress:

        // The value must be a string
        if (!qVariantCanConvert<QString>(value))
            qWarning() << "QMailAccountKey - Invalid string comparison!";
        break;

    case QMailAccountKey::MessageType:

        // The value must be an int
        if (!qVariantCanConvert<int>(value))
            qWarning() << "QMailAccountKey - Invalid message type comparison!";
        break;
    }
}

