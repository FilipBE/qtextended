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

#include "qmailaccountsortkey.h"
#include "qmailaccountsortkey_p.h"

/*!
    \class QMailAccountSortKey
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailAccountSortKey class defines the parameters used for sorting a subset of 
    queried accounts from the store.
    \ingroup messaginglibrary

    A QMailAccountSortKey is composed of an account property to sort and a sort order. 
    The QMailAccountSortKey class is used in conjunction with the QMailStore::query() 
    function to sort account results according to the criteria defined by the sort key.

    For example:
    To create a query for all accounts sorted by the name in ascending order:
    \code
    QMailAccountSortKey sortNameKey(QMailAccountSortKey::Name,Qt::Ascending);
    QMailAccountIdList results = QMailStore::instance()->query(sortNameKey);
    \endcode
    
    \sa QMailStore
*/

/*!
    \enum QMailAccountSortKey::Property

    This enum type describes the sortable data properties of a QMailFolder.

    \value Id The ID of the account.
    \value Name The name of the account.
    \value MessageType The type of messages handled by the account.
    \value EmailAddress The email address that is tied to the account.
    
*/

/*!
    Create a QMailAccountSortKey with specifying matching parameters.

    A default-constructed key (one for which isEmpty() returns true) sorts no folders. 

    The result of combining an empty key with a non-empty key is the same as the original 
    non-empty key.

    The result of combining two empty keys is an empty key.
*/

QMailAccountSortKey::QMailAccountSortKey()
{
    d = new QMailAccountSortKeyPrivate();
}

/*!
    Construct a QMailAccountSortKey which sorts a set of results based on the  
    QMailAccountSortKey::Property \a p and the Qt::SortOrder \a order 
*/

QMailAccountSortKey::QMailAccountSortKey(Property p, Qt::SortOrder order)
{
    d = new QMailAccountSortKeyPrivate();
    QMailAccountSortKeyPrivate::Argument a(p,order);
    d->arguments.append(a);
}

/*!
    Create a copy of the QMailAccountSortKey \a other.
*/

QMailAccountSortKey::QMailAccountSortKey(const QMailAccountSortKey& other)
{
    d = other.d;
}

/*!
    Destroys this QMailAccountSortKey.
*/

QMailAccountSortKey::~QMailAccountSortKey()
{
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailAccountSortKey QMailAccountSortKey::operator&(const QMailAccountSortKey& other) const
{
    QMailAccountSortKey k;
    k.d->arguments = d->arguments + other.d->arguments;
    return k;
}

/*!
    Performs a logical AND with this key and the key \a other and assigns the result
    to this key.
*/

QMailAccountSortKey& QMailAccountSortKey::operator&=(const QMailAccountSortKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailAccountSortKey::operator==(const QMailAccountSortKey& other) const
{
    return d->arguments == other.d->arguments;
}
/*!
    Returns \c true if the value of this key is not the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailAccountSortKey::operator!=(const QMailAccountSortKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailAccountSortKey \a other to this.
*/

QMailAccountSortKey& QMailAccountSortKey::operator=(const QMailAccountSortKey& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false.
*/

bool QMailAccountSortKey::isEmpty() const
{
    return d->arguments.isEmpty();
}

