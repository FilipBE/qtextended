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

#include "qmailaccountlistmodel.h"
#include "qmailstore.h"
#include "qmailaccount.h"
#include <QDebug>
#include <QCache>

static const int fullRefreshCutoff = 10;

class LessThanFunctorA
{
public:
    LessThanFunctorA(const QMailAccountSortKey& sortKey);
    ~LessThanFunctorA();

    bool operator()(const QMailAccountId& first, const QMailAccountId& second);
    bool invalidatedList() const;

private:
    QMailAccountSortKey mSortKey;
    bool mInvalidatedList;
};

LessThanFunctorA::LessThanFunctorA(const QMailAccountSortKey& sortKey)
:
    mSortKey(sortKey),
    mInvalidatedList(false)
{
}

LessThanFunctorA::~LessThanFunctorA(){}

bool LessThanFunctorA::operator()(const QMailAccountId& first, const QMailAccountId& second)
{
    QMailAccountKey firstKey(QMailAccountKey::Id,first);
    QMailAccountKey secondKey(QMailAccountKey::Id,second);

    QMailAccountIdList results = QMailStore::instance()->queryAccounts(firstKey | secondKey, mSortKey);
    if(results.count() != 2)
    {
        mInvalidatedList = true;
        return false;
    }
    return results.first() == first;
}

bool LessThanFunctorA::invalidatedList() const
{
    return mInvalidatedList;
}


class QMailAccountListModelPrivate
{
public:
    QMailAccountListModelPrivate(const QMailAccountKey& key,
                                 const QMailAccountSortKey& sortKey,
                                 bool synchronizeEnabled);
    ~QMailAccountListModelPrivate();

    const QMailAccountIdList& ids() const;

    int indexOf(const QMailAccountId& id) const;

    template<typename Comparator>
    QMailAccountIdList::iterator lowerBound(const QMailAccountId& id, Comparator& cmp) const;

public:
    QMailAccountKey key;
    QMailAccountSortKey sortKey;
    bool synchronizeEnabled;
    mutable QMailAccountIdList idList;
    mutable bool init;
    mutable bool needSynchronize;
};

QMailAccountListModelPrivate::QMailAccountListModelPrivate(const QMailAccountKey& key,
                                                           const QMailAccountSortKey& sortKey,
                                                           bool synchronizeEnabled)
:
    key(key),
    sortKey(sortKey),
    synchronizeEnabled(synchronizeEnabled),
    init(false),
    needSynchronize(true)
{
}

QMailAccountListModelPrivate::~QMailAccountListModelPrivate()
{
}

const QMailAccountIdList& QMailAccountListModelPrivate::ids() const
{
    if (!init) {
        idList = QMailStore::instance()->queryAccounts(key,sortKey);
        init = true;
        needSynchronize = false;
    }

    return idList;
}

int QMailAccountListModelPrivate::indexOf(const QMailAccountId& id) const
{
    return ids().indexOf(id);
}

template<typename Comparator>
QMailAccountIdList::iterator QMailAccountListModelPrivate::lowerBound(const QMailAccountId& id, Comparator& cmp) const
{
    return qLowerBound(idList.begin(), idList.end(), id, cmp);
}


/*!
  \class QMailAccountListModel 
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

  \preliminary
  \ingroup messaginglibrary 
  \brief The QMailAccountListModel class provides access to a list of stored accounts. 

  The QMailAccountListModel presents a list of all the accounts currently stored in
  the message store. By using the setKey() and sortKey() functions it is possible to have the model
  represent specific user filtered subsets of accounts sorted in a particular order.

  The QMailAccountListModel is a descendant of QAbstractListModel, so it is suitable for use with
  the Qt View classes such as QListView to visually represent lists of accounts. 

  The model listens for changes to the underlying storage system and sychronizes its contents based on
  the setSynchronizeEnabled() setting.
 
  Accounts can be extracted from the view with the idFromIndex() function and the resultant id can be 
  used to load an account from the store. 

  For filters or sorting not provided by the QMailAccountListModel it is recommended that
  QSortFilterProxyModel is used to wrap the model to provide custom sorting and filtering. 

  \sa QMailAccount, QSortFilterProxyModel
*/

/*!
  \enum QMailAccountListModel::Roles

  Represents common display roles of an account. These roles are used to display common account elements 
  in a view and its attached delegates.

  \value NameTextRole The name of the account 
  \value MessageTypeRole The type of the account 
  \value MessageSourcesRole The list of message sources for the account 
  \value MessageSinksRole The list of message sinks for the account 
*/

/*!
    Constructs a QMailAccountListModel with a parent \a parent.
    By default, the model will match all accounts in the database, and display them in
    the order they were submitted. Synchronization defaults to true.
*/

QMailAccountListModel::QMailAccountListModel(QObject* parent)
:
    QAbstractListModel(parent),
    d(new QMailAccountListModelPrivate(QMailAccountKey(),QMailAccountSortKey(),true))
{
    connect(QMailStore::instance(),
            SIGNAL(accountsAdded(QMailAccountIdList)),
            this,
            SLOT(accountsAdded(QMailAccountIdList)));
    connect(QMailStore::instance(),
            SIGNAL(accountsRemoved(QMailAccountIdList)),
            this,
            SLOT(accountsRemoved(QMailAccountIdList)));
    connect(QMailStore::instance(),
            SIGNAL(accountsUpdated(QMailAccountIdList)),
            this,
            SLOT(accountsUpdated(QMailAccountIdList)));
}

/*!
    Deletes the QMailMessageListModel object.
*/

QMailAccountListModel::~QMailAccountListModel()
{
    delete d; d = 0;
}

/*!
    \reimp
*/

int QMailAccountListModel::rowCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return d->ids().count();
}

/*!
    \reimp
*/

QVariant QMailAccountListModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return QVariant();

    int offset = index.row();
    QMailAccountId id = d->ids().at(offset);
    QMailAccount account(id);

    switch(role)
    {
       case Qt::DisplayRole:
       case NameTextRole:
#ifdef QTOPIA_HOMEUI
            // For the Home UI, the collective library is only used to configure GTalk
            if (account.messageType() == QMailMessage::Instant)
                return "GTalk";
#endif
            return account.accountName();
            break;

       case MessageTypeRole:
            return static_cast<int>(account.messageType());
            break;

       case MessageSourcesRole:
            return account.messageSources();
            break;

       case MessageSinksRole:
            return account.messageSinks();
            break;
    }

    return QVariant();
}

/*!
    Returns the QMailAccountKey used to populate the contents of this model.
*/

QMailAccountKey QMailAccountListModel::key() const
{
    return d->key; 
}

/*!
    Sets the QMailAccountKey used to populate the contents of the model to \a key.
    If the key is empty, the model is populated with all the accounts from the 
    database.
*/

void QMailAccountListModel::setKey(const QMailAccountKey& key) 
{
    d->key = key;
    d->init = false;
    reset(); 
}

/*!
    Returns the QMailAccountSortKey used to sort the contents of the model.
*/

QMailAccountSortKey QMailAccountListModel::sortKey() const
{
   return d->sortKey;
}

/*!
    Sets the QMailAccountSortKey used to sort the contents of the model to \a sortKey.
    If the sort key is invalid, no sorting is applied to the model contents and accounts 
    are displayed in the order in which they were added into the database.
*/

void QMailAccountListModel::setSortKey(const QMailAccountSortKey& sortKey) 
{
    d->sortKey = sortKey;
    d->init = false;
    reset();
}

/*! \internal */

void QMailAccountListModel::accountsAdded(const QMailAccountIdList& ids)
{
    d->needSynchronize = true;
    if(!d->synchronizeEnabled)
        return;

    //TODO change this code to use our own searching and insertion routines
    //for more control
    //use id sorted indexes
    
    if(!d->init)
        return;
    
    QMailAccountKey passKey = d->key & QMailAccountKey(ids);
    QMailAccountIdList results = QMailStore::instance()->queryAccounts(passKey);
    if(results.isEmpty())
        return;

    if(results.count() > fullRefreshCutoff)
        fullRefresh();

    if(!d->sortKey.isEmpty())
    { 
        foreach(const QMailAccountId &id,results)
        {
            LessThanFunctorA lessThan(d->sortKey);

            //if sorting the list fails, then resort to a complete refresh
            if(lessThan.invalidatedList())
                fullRefresh();
            else
            {
                QMailAccountIdList::iterator itr = d->lowerBound(id, lessThan);
                int newIndex = (itr - d->idList.begin());

                beginInsertRows(QModelIndex(),newIndex,newIndex);
                d->idList.insert(itr, id);
                endInsertRows();
            }
        }
    }
    else
    {
        int index = d->idList.count();

        beginInsertRows(QModelIndex(),index,(index + results.count() - 1));
        foreach(const QMailAccountId &id,results)
            d->idList.append(id);
        endInsertRows();
    }
    d->needSynchronize = false;
}

/*! \internal */

void QMailAccountListModel::accountsUpdated(const QMailAccountIdList& ids)
{
    d->needSynchronize = true;
    if(!d->synchronizeEnabled)
        return;

    //TODO change this code to use our own searching and insertion routines
    //for more control
    //use id sorted indexes

    if(!d->init)
        return;

    QMailAccountKey idKey(ids);

    QMailAccountIdList validIds = QMailStore::instance()->queryAccounts(idKey & d->key);
    if(validIds.count() > fullRefreshCutoff)
    {
        fullRefresh();
        return;
    }

    //if the key is empty the id's will be returned valid and invalid
    if(!d->key.isEmpty())
    {
        QMailAccountIdList invalidIds = QMailStore::instance()->queryAccounts(idKey & ~d->key);
        foreach(const QMailAccountId &id,invalidIds)
        {
            //get the index
            int index = d->idList.indexOf(id);
            if (index == -1) 
                continue;

            beginRemoveRows(QModelIndex(),index,index);
            d->idList.removeAt(index);
            endRemoveRows();
        }
    }

    LessThanFunctorA lessThan(d->sortKey);

    foreach(const QMailAccountId &id, validIds)
    {
        int index = d->idList.indexOf(id);
        if(index == -1) //insert
        {
            if(lessThan.invalidatedList())
                fullRefresh();
            else
            {
                QMailAccountIdList::iterator itr = d->lowerBound(id, lessThan);
                int newIndex = (itr - d->idList.begin());

                beginInsertRows(QModelIndex(),newIndex,newIndex);
                d->idList.insert(itr, id);
                endInsertRows();
            }
        }
        else //update
        {
            if(lessThan.invalidatedList())
                fullRefresh();
            else
            {
                QMailAccountIdList::iterator itr = d->lowerBound(id, lessThan);
                int newIndex = (itr - d->idList.begin());

                if((newIndex == index) || (newIndex == index + 1))
                {
                    // This item would be inserted either immediately before or after itself
                    QModelIndex modelIndex = createIndex(index,0);
                    emit dataChanged(modelIndex,modelIndex);
                }
                else
                {
                    beginRemoveRows(QModelIndex(),index,index);
                    d->idList.removeAt(index);
                    endRemoveRows();

                    if (newIndex > index)
                        --newIndex;

                    beginInsertRows(QModelIndex(),newIndex,newIndex);
                    d->idList.insert(newIndex, id);
                    endInsertRows();
                }
            }
        }
    }
    d->needSynchronize = false;
}

/*! \internal */

void QMailAccountListModel::accountsRemoved(const QMailAccountIdList& ids)
{
    d->needSynchronize = true;
    if(!d->synchronizeEnabled)
        return;

    if(!d->init)
        return;

    foreach(const QMailAccountId &id, ids)
    {
        int index = d->indexOf(id);
        if(index == -1)
            continue;

        beginRemoveRows(QModelIndex(),index,index);
        d->idList.removeAt(index);
        endRemoveRows();
    }
    d->needSynchronize = false;
}

/*!
    Returns the QMailAccountId of the account represented by the QModelIndex \a index.
    If the index is not valid an invalid QMailAccountId is returned.
*/

QMailAccountId QMailAccountListModel::idFromIndex(const QModelIndex& index) const
{
    if(!index.isValid())
        return QMailAccountId();

    return d->ids().at(index.row());
}

/*!
    Returns the QModelIndex that represents the account with QMailAccountId \a id.
    If the id is not conatained in this model, an invalid QModelIndex is returned.
*/

QModelIndex QMailAccountListModel::indexFromId(const QMailAccountId& id) const
{
    //if the id does not exist return null
    int index = d->indexOf(id);
    if(index != -1) {
        return createIndex(index,0);
    }

    return QModelIndex();
}

/*!
    Returns \c true if the model sychronizes its contents based on account changes
    in the database, otherwise returns \c false.
*/

bool QMailAccountListModel::synchronizeEnabled() const
{
    return d->synchronizeEnabled;
}

/*!
    Sets wheather the model synchronizes its contents based on account changes
    in the database to \a val.
*/

void QMailAccountListModel::setSynchronizeEnabled(bool val)
{
    d->synchronizeEnabled = val;
    if(val && d->needSynchronize)
        fullRefresh();
}

/*! \internal */

void QMailAccountListModel::fullRefresh()
{
    d->init = false;
    reset();
}

