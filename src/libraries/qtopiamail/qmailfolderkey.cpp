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

#include "qmailfolderkey.h"
#include "qmailfolderkey_p.h"

#include <QMailAccountKey>

using namespace QMailDataComparator;


/*!
    \class QMailFolderKey
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailFolderKey class defines the parameters used for querying a subset of
    all available folders from the mail store.
    \ingroup messaginglibrary

    A QMailFolderKey is composed of a folder property, an optional comparison operator
    and a comparison value. The QMailFolderKey class is used in conjunction with the 
    QMailStore::queryFolders() and QMailStore::countFolders() functions to filter results 
    which meet the criteria defined by the key.

    QMailFolderKey's can be combined using the logical operators (&), (|) and (~) to
    build more sophisticated queries.

    For example:

    To create a query for all folders named "inbox" or "sms":
    \code
    QMailFolderKey inboxKey(QMailFolderKey::Name, "inbox");
    QMailFolderKey smsKey(QMailFolderKey::Name, "sms");
    QMailFolderIdList results = QMailStore::instance()->queryFolders(inboxKey | smsKey);
    \endcode

    To query all folders with name "foo" for a specified account:
    \code
    QMailFolderIdList fooFolders(const QMailAccountId& accountId)
    {
        QMailFolderKey nameKey(QMailFolderKey::Name, "foo");
        QMailFolderKey accountKey(QMailFolderKey::ParentAccountId, accountId);

        return QMailStore::instance()->queryFolders(nameKey & accountKey);
    }
    \endcode

    \sa QMailStore, QMailFolder
*/

/*!
    \enum QMailFolderKey::Property

    This enum type describes the queryable data properties of a QMailFolder.

    \value Id The ID of the folder.
    \value Name The name of the folder in native form.
    \value ParentId The ID of the parent folder for a given folder.
    \value ParentAccountId The ID of the parent account for this folder.
    \value DisplayName The name of the folder, designed for display to users.
    \value Status The status value of the folder.
    \value AncestorFolderIds The set of IDs of folders which are direct or indirect parents of this folder.
*/

/*!
    \typedef QMailFolderKey::IdType
    \internal
*/

Q_IMPLEMENT_USER_METATYPE(QMailFolderKey);

/*!
    Create a QMailFolderKey with specifying matching parameters.

    A default-constructed key (one for which isEmpty() returns true) matches all folders.
    The logical negation of an empty key also matches all folders.

    The result of combining an empty key with a non-empty key is the same as the original
    non-empty key. This is true regardless of whether the combination is formed by a
    logical AND or a logical OR operation.

    The result of combining two empty keys is an empty key.
*/

QMailFolderKey::QMailFolderKey()
{
    d = new QMailFolderKeyPrivate();
}

/*!
    Construct a QMailFolderKey which defines a query parameter where
    QMailFolder::Property \a p is compared using comparison operator
    \a c with a value \a value.
*/

QMailFolderKey::QMailFolderKey(Property p, const QVariant& value, QMailDataComparator::Comparator c)
{
    sanityCheck(p, value, c);

    d = new QMailFolderKeyPrivate();

    // Convert Excludes to negated Includes
    if (c == QMailDataComparator::Excludes) {
        d->negated = true;
        c = QMailDataComparator::Includes;
    }

    d->arguments.append(QMailFolderKeyPrivate::Argument(p, c, value));
}

/*!
    Construct a QMailFolderKey which defines a query parameter where
    folder id's matching those in \a ids are returned.
*/

QMailFolderKey::QMailFolderKey(const QMailFolderIdList& ids)
{
    if (ids.isEmpty()) {
        *this = nonMatchingKey();
    } else {
        d = new QMailFolderKeyPrivate();
        d->arguments.append(QMailFolderKeyPrivate::Argument(QMailFolderKey::Id, QMailDataComparator::Equal, ids));
    }
}

/*!
    Create a copy of the QMailFolderKey \a other.
*/

QMailFolderKey::QMailFolderKey(const QMailFolderKey& other)
{
    d = other.d;
}

/*!
    Destroys this QMailFolderKey.
*/


QMailFolderKey::~QMailFolderKey()
{
}

/*!
    Returns a key that is the logical NOT of the value of this key.
*/

QMailFolderKey QMailFolderKey::operator~() const
{
    QMailFolderKey k(*this);
    if(!k.isEmpty())
        k.d->negated = !d->negated;
    return k;
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailFolderKey QMailFolderKey::operator&(const QMailFolderKey& other) const
{
    if (isEmpty() || other.isNonMatching())
        return other;
    else if (isNonMatching() || other.isEmpty())
        return *this;

    QMailFolderKey k;
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

QMailFolderKey QMailFolderKey::operator|(const QMailFolderKey& other) const
{
    if (isEmpty() || isNonMatching())
        return other;
    else if (other.isEmpty() || other.isNonMatching())
        return *this;

    QMailFolderKey k;
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

QMailFolderKey& QMailFolderKey::operator&=(const QMailFolderKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Performs a logical OR with this key and the key \a other and assigns the result
    to this key.
*/

QMailFolderKey& QMailFolderKey::operator|=(const QMailFolderKey& other) 
{
    *this = *this | other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailFolderKey::operator==(const QMailFolderKey& other) const
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

bool QMailFolderKey::operator!=(const QMailFolderKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailFolderKey \a other to this.
*/

QMailFolderKey& QMailFolderKey::operator=(const QMailFolderKey& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false. 

    \sa isNonMatching()
*/

bool QMailFolderKey::isEmpty() const
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

bool QMailFolderKey::isNonMatching() const
{
    return (*this == nonMatchingKey());
}

/*!
  Returns the QVariant representation of this QMailFolderKey. 
*/

QMailFolderKey::operator QVariant() const
{
	return QVariant::fromValue(*this);
}

/*!
    \fn QMailFolderKey::serialize(Stream &stream) const

    Writes the contents of a QMailFolderKey to a \a stream.
*/

template <typename Stream> void QMailFolderKey::serialize(Stream &stream) const
{
    stream << d->combiner;          
    stream << d->negated;

    stream << d->arguments.count();
    foreach (const QMailFolderKeyPrivate::Argument& a, d->arguments)
        a.serialize(stream);

    stream << d->subKeys.count();
    foreach (const QMailFolderKey& k, d->subKeys)
        k.serialize(stream);
}

/*!
    \fn QMailFolderKey::deserialize(Stream &stream)

    Reads the contents of a QMailFolderKey from \a stream.
*/

template <typename Stream> void QMailFolderKey::deserialize(Stream &stream)
{
    int enumInt = 0;
    stream >> enumInt;
    d->combiner = static_cast<QMailDataComparator::Combiner>(enumInt);
    stream >> d->negated;

    int argumentCount = 0;
    stream >> argumentCount;
    for(int i = 0; i < argumentCount; ++i) {
        QMailFolderKeyPrivate::Argument a;
        a.deserialize(stream);
        d->arguments.append(a);
    }

    int subKeyCount = 0;
    stream >> subKeyCount;
    for(int i = 0; i < subKeyCount; ++i) {
        QMailFolderKey subKey;
        subKey.deserialize(stream);
        d->subKeys.append(subKey);
    }
}

/*!
    Returns a key that does not match any folders (unlike an empty key).

    \sa isNonMatching(), isEmpty()
*/
QMailFolderKey QMailFolderKey::nonMatchingKey()
{
    return QMailFolderKey(QMailFolderKey::Id, QMailFolderId(), QMailDataComparator::Equal);
}

/*! \internal */
void QMailFolderKey::sanityCheck(QMailFolderKey::Property p, const QVariant &value, QMailDataComparator::Comparator) 
{
    switch (p) 
    {
    case QMailFolderKey::Id:
    case QMailFolderKey::ParentId:
    case QMailFolderKey::AncestorFolderIds:

        // The value must be either a QMailFolderId, list or key
        if (!qVariantCanConvert<QMailFolderId>(value) && 
            !qVariantCanConvert<QMailFolderIdList>(value) &&
            !qVariantCanConvert<QMailFolderKey>(value))
            qWarning() << "QMailFolderKey - Invalid folder ID comparison!";
        break;

    case QMailFolderKey::Name:
    case QMailFolderKey::DisplayName:

        // The value must be a string
        if (!qVariantCanConvert<QString>(value))
            qWarning() << "QMailFolderKey - Invalid folder string comparison!";
        break;

    case QMailFolderKey::ParentAccountId:

        // The value must be either a QMailAccountId, list or key
        if (!qVariantCanConvert<QMailAccountId>(value) && 
            !qVariantCanConvert<QMailAccountIdList>(value) &&
            !qVariantCanConvert<QMailAccountKey>(value))
            qWarning() << "QMailFolderKey - Invalid account ID comparison!";
        break;

    case QMailFolderKey::Status:

        // The value must be a quint64
        if (!qVariantCanConvert<quint64>(value))
            qWarning() << "QMailFolderKey - Invalid folder status comparison!";
        break;
    }
}

