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

#include "qmailfoldersortkey.h"
#include "qmailfoldersortkey_p.h"


/*!
    \class QMailFolderSortKey
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailFolderSortKey class defines the parameters used for sorting a subset of 
    queried folders from the mail store.
    \ingroup messaginglibrary

    A QMailFolderSortKey is composed of a folder property to sort and a sort order. 
    The QMailFolderSortKey class is used in conjunction with the QMailStore::queryFolders() 
    function to sort folder results according to the criteria defined by the sort key.

    For example:
    To create a query for all folders sorted by the name in ascending order:
    \code
    QMailFolderSortKey sortNameKey(QMailFolderSortKey::Name,Qt::Ascending);
    QMailIdList results = QMailStore::instance()->queryFolders(QMailFolderKey(),sortNameKey);
    \endcode
    
    \sa QMailStore, QMailFolderKey
*/

/*!
    \enum QMailFolderSortKey::Property

    This enum type describes the sortable data properties of a QMailFolder.

    \value Id The ID of the folder.
    \value Name The name of the folder in native form.
    \value ParentId The ID of the parent folder for a given folder.
    \value ParentAccountId The ID of the parent account for a given folder.
    \value DisplayName The name of the folder, designed for display to users.
    \value Status The status value of the folder.
*/

/*!
    Create a QMailFolderSortKey with specifying matching parameters.

    A default-constructed key (one for which isEmpty() returns true) sorts no folders. 

    The result of combining an empty key with a non-empty key is the same as the original 
    non-empty key.

    The result of combining two empty keys is an empty key.
*/

QMailFolderSortKey::QMailFolderSortKey()
{
    d = new QMailFolderSortKeyPrivate();
}

/*!
    Construct a QMailFolderSortKey which sorts a set of results based on the  
    QMailFolderSortKey::Property \a p and the Qt::SortOrder \a order 
*/

QMailFolderSortKey::QMailFolderSortKey(Property p, Qt::SortOrder order)
{
    d = new QMailFolderSortKeyPrivate();
    QMailFolderSortKeyPrivate::Argument a(p,order);
    d->arguments.append(a);
}

/*!
    Create a copy of the QMailFolderSortKey \a other.
*/

QMailFolderSortKey::QMailFolderSortKey(const QMailFolderSortKey& other)
{
    d = other.d;
}

/*!
    Destroys this QMailFolderSortKey.
*/

QMailFolderSortKey::~QMailFolderSortKey()
{
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailFolderSortKey QMailFolderSortKey::operator&(const QMailFolderSortKey& other) const
{
    QMailFolderSortKey k;
    k.d->arguments = d->arguments + other.d->arguments;
    return k;
}

/*!
    Performs a logical AND with this key and the key \a other and assigns the result
    to this key.
*/

QMailFolderSortKey& QMailFolderSortKey::operator&=(const QMailFolderSortKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailFolderSortKey::operator==(const QMailFolderSortKey& other) const
{
    return d->arguments == other.d->arguments;
}
/*!
    Returns \c true if the value of this key is not the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailFolderSortKey::operator!=(const QMailFolderSortKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailFolderSortKey \a other to this.
*/

QMailFolderSortKey& QMailFolderSortKey::operator=(const QMailFolderSortKey& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false.
*/

bool QMailFolderSortKey::isEmpty() const
{
    return d->arguments.isEmpty();
}

