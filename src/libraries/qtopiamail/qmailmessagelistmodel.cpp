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

#include "qmailmessagelistmodel.h"
#include "qmailstore.h"
#include <QIcon>
#include <QTimeString>
#include <QDebug>
#include <QCache>
#include <QContactModel>
#include <QtAlgorithms>

#include <QCollectivePresenceInfo>

#ifdef QTOPIA_HOMEUI
#  include <private/homewidgets_p.h>
#endif

static const int nameCacheSize = 50;
static const int fullRefreshCutoff = 10;

class QMailMessageListModelPrivate
{
public:
    class Item : private QPair<QMailMessageId, bool>
    {
    public:
        explicit Item(const QMailMessageId& id, bool f = false) : QPair<QMailMessageId, bool>(id, f) {}

        QMailMessageId id() const { return first; }

        bool isChecked() const { return second; }
        void setChecked(bool f) { second = f; }

        // Two instances of the same QMailMessageId are the same Item, regardless of the checked state
        bool operator== (const Item& other) { return (first == other.first); }
    };

    QMailMessageListModelPrivate(const QMailMessageKey& key,
                                 const QMailMessageSortKey& sortKey,
                                 bool sychronizeEnabled);
    ~QMailMessageListModelPrivate();

    const QList<Item>& items() const;

    int indexOf(const QMailMessageId& id) const;

    template<typename Comparator>
    QList<Item>::iterator lowerBound(const QMailMessageId& id, Comparator& cmp) const;

    QString messageAddressText(const QMailMessageMetaData& m, bool incoming);

    void invalidateCache();

public:
    QMailMessageKey key;
    QMailMessageSortKey sortKey;
    bool ignoreUpdates;
    mutable QList<Item> itemList;
    mutable bool init;
    mutable bool needSynchronize;
    QCache<QString,QString> nameCache;
    QContactModel contactModel;
};

class LessThanFunctor
{
public:
    typedef QMailMessageListModelPrivate::Item Item;

    LessThanFunctor(const QMailMessageSortKey& sortKey);
    ~LessThanFunctor();

    bool operator()(const QMailMessageId& lhs, const QMailMessageId& rhs);

    bool operator()(const Item& lhs, const Item& rhs) { return operator()(lhs.id(), rhs.id()); }
    bool operator()(const Item& lhs, const QMailMessageId& rhs) { return operator()(lhs.id(), rhs); }
    bool operator()(const QMailMessageId& lhs, const Item& rhs) { return operator()(lhs, rhs.id()); }

    bool invalidatedList() const;

private:
    QMailMessageSortKey mSortKey;
    bool mInvalidatedList;
};

LessThanFunctor::LessThanFunctor(const QMailMessageSortKey& sortKey)
:
    mSortKey(sortKey),
    mInvalidatedList(false)
{
}

LessThanFunctor::~LessThanFunctor(){}

bool LessThanFunctor::operator()(const QMailMessageId& lhs, const QMailMessageId& rhs)
{
    QMailMessageKey firstKey(QMailMessageKey::Id,lhs);
    QMailMessageKey secondKey(QMailMessageKey::Id,rhs);

    // TODO: we have to do this in a more efficient manner:
    QMailMessageIdList results = QMailStore::instance()->queryMessages(firstKey | secondKey, mSortKey);
    if(results.count() != 2)
    {
        mInvalidatedList = true;
        return false;
    }
    return results.first() == lhs;
}

bool LessThanFunctor::invalidatedList() const
{
    return mInvalidatedList;
}

QMailMessageListModelPrivate::QMailMessageListModelPrivate(const QMailMessageKey& key,
                                                           const QMailMessageSortKey& sortKey,
                                                           bool ignoreUpdates)
:
    key(key),
    sortKey(sortKey),
    ignoreUpdates(ignoreUpdates),
    init(false),
    needSynchronize(true),
    nameCache(nameCacheSize)
{
}

QMailMessageListModelPrivate::~QMailMessageListModelPrivate()
{
}

const QList<QMailMessageListModelPrivate::Item>& QMailMessageListModelPrivate::items() const
{
    if(!init)
    {
        itemList.clear();
        QMailMessageIdList ids = QMailStore::instance()->queryMessages(key,sortKey);
        foreach (const QMailMessageId& id, ids)
            itemList.append(QMailMessageListModelPrivate::Item(id, false));

        init = true;
        needSynchronize = false;
    }

    return itemList;
}

int QMailMessageListModelPrivate::indexOf(const QMailMessageId& id) const
{
    Item item(id, false);

    // Will return the matching item regardless of boolean state due to Item::operator==
    return itemList.indexOf(item);
}

template<typename Comparator>
QList<QMailMessageListModelPrivate::Item>::iterator QMailMessageListModelPrivate::lowerBound(const QMailMessageId& id, Comparator& cmp) const
{
    return qLowerBound(itemList.begin(), itemList.end(), id, cmp);
}

QString QMailMessageListModelPrivate::messageAddressText(const QMailMessageMetaData& m, bool incoming) 
{
    //message sender or recipients
    if ( incoming ) 
    {
        QMailAddress fromAddress(m.from());
        if(!nameCache.contains(fromAddress.toString()))
        {
            QString displayName = fromAddress.displayName(contactModel);
            nameCache.insert(fromAddress.toString(), new QString(displayName));
        }
        return *nameCache.object(fromAddress.toString());
    }
    else 
    {
        QMailAddressList toAddressList(m.to());
        if (!toAddressList.isEmpty())
        {
            QMailAddress firstRecipient(toAddressList.first());

            if(!nameCache.contains(firstRecipient.toString()))
            {
                QString displayName = firstRecipient.displayName(contactModel);
                nameCache.insert(firstRecipient.toString(), new QString(displayName));
            }
            QString text = *nameCache.object(firstRecipient.toString());
            bool multipleRecipients(toAddressList.count() > 1);
            if(multipleRecipients)
                text += ", ...";
            return text;
        }
        else 
            return QObject::tr("Draft Message");
    }
}

void QMailMessageListModelPrivate::invalidateCache()
{
    nameCache.clear();
}


/*!
  \class QMailMessageListModel 
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

  \preliminary
  \ingroup messaginglibrary 
  \brief The QMailMessageListModel class provides access to a list of stored messages. 

  The QQMailMessageListModel presents a list of all the messages currently stored in
  the message store. By using the setKey() and sortKey() functions it is possible to have the model
  represent specific user filtered subsets of messages sorted in a particular order.

  The QMailMessageListModel is a descendant of QAbstractListModel, so it is suitable for use with
  the Qt View classes such as QListView to visually represent lists of messages. 
 
  The model listens for changes reported by the QMailStore, and automatically synchronizes
  its content with that of the store.  This behaviour can be optionally or temporarily disabled 
  by calling the setIgnoreMailStoreUpdates() function.

  Messages can be extracted from the view with the idFromIndex() function and the resultant id can be 
  used to load a message from the store. 

  For filters or sorting not provided by the QMailMessageListModel it is recommended that
  QSortFilterProxyModel is used to wrap the model to provide custom sorting and filtering. 

  \sa QMailMessage, QSortFilterProxyModel
*/

/*!
  \enum QMailMessageListModel::Roles

  Represents common display roles of a message. These roles are used to display common message elements 
  in a view and its attached delegates.

  \value MessageAddressTextRole 
    The address text of a message. This a can represent a name if the address is tied to a contact in the addressbook and 
    represents either the incoming or outgoing address depending on the message direction.
  \value MessageSubjectTextRole  
    The subject of a message. For-non email messages this may represent the body text of a message.
  \value MessageFilterTextRole 
    The MessageAddressTextRole concatenated with the MessageSubjectTextRole. This can be used by filtering classes to filter
    messages based on the text of these commonly displayed roles. 
  \value MessageTimeStampTextRole
    The timestamp of a message. "Recieved" or "Sent" is prepended to the timestamp string depending on the message
    direction.
  \value MessageTypeIconRole
    An Icon representing the type of the message.
  \value MessageStatusIconRole
    An Icon representing the status of the message. e.g Read, Unread, Downloaded
  \value MessageDirectionIconRole
    An Icon representing the incoming or outgoing direction of a message.
  \value MessagePresenceIconRole
    An Icon representing the presence status of the contact associated with the MessageAddressTextRole.
  \value MessageBodyTextRole  
    The body of a message represented as text.
  \value MessageIdRole
    The QMailMessageId value identifying the message.
*/

/*!
    Constructs a QMailMessageListModel with a parent \a parent.

    By default, the model will match all messages in the database, and display them in
    the order they were submitted, and mail store updates are not ignored.

    \sa setKey(), setSortKey(), setIgnoreMailStoreUpdates()
*/

QMailMessageListModel::QMailMessageListModel(QObject* parent)
:
    QAbstractListModel(parent),
    d(new QMailMessageListModelPrivate(QMailMessageKey::nonMatchingKey(),QMailMessageSortKey(),false))
{
    connect(QMailStore::instance(),
            SIGNAL(messagesAdded(QMailMessageIdList)),
            this,
            SLOT(messagesAdded(QMailMessageIdList)));
    connect(QMailStore::instance(),
            SIGNAL(messagesRemoved(QMailMessageIdList)),
            this,
            SLOT(messagesRemoved(QMailMessageIdList)));
    connect(QMailStore::instance(),
            SIGNAL(messagesUpdated(QMailMessageIdList)),
            this,
            SLOT(messagesUpdated(QMailMessageIdList)));
    connect(&d->contactModel,
            SIGNAL(modelReset()),
            this,
            SLOT(contactModelReset()));
}

/*!
    Deletes the QMailMessageListModel object.
*/

QMailMessageListModel::~QMailMessageListModel()
{
    delete d; d = 0;
}

/*!
    \reimp
*/

int QMailMessageListModel::rowCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return d->items().count();
}

/*!
    Returns true if the model contains no messages.
*/
bool QMailMessageListModel::isEmpty() const
{
    return d->items().isEmpty();
}

/*!
    \reimp
*/

QVariant QMailMessageListModel::data(const QModelIndex& index, int role) const
{
#ifdef QTOPIA_HOMEUI
    static QIcon outgoingIcon(":icon/phone/outgoingcall");
    static QIcon incomingIcon(":icon/phone/incomingcall");
#else
    static QIcon outgoingIcon(":icon/qtmail/sendmail");
    static QIcon incomingIcon(":icon/qtmail/getmail");
#endif
    
    static QIcon readIcon(":icon/qtmail/flag_normal");
    static QIcon unreadIcon(":icon/qtmail/flag_unread");
    static QIcon toGetIcon(":icon/qtmail/flag_toget");
    static QIcon toSendIcon(":icon/qtmail/flag_tosend");
    static QIcon unfinishedIcon(":icon/qtmail/flag_unfinished");
    static QIcon removedIcon(":icon/qtmail/flag_removed");

    static QIcon noPresenceIcon(":icon/presence-none");
    static QIcon offlineIcon(":icon/presence-offline");
    static QIcon awayIcon(":icon/presence-away");
    static QIcon busyIcon(":icon/presence-busy");
    static QIcon onlineIcon(":icon/presence-online");

#ifdef QTOPIA_HOMEUI
    static QIcon messageIcon(":icon/home/message");
    static QIcon videomailIcon(":icon/home/video");
    static QIcon voicemailIcon(":icon/home/voicemail");
    static QIcon emailIcon(":icon/home/email");
#else
    static QIcon messageIcon(":icon/txt");
    static QIcon mmsIcon(":icon/multimedia");
    static QIcon emailIcon(":icon/email");
    static QIcon instantMessageIcon(":icon/im");
#endif

    if(!index.isValid())
        return QVariant();

    QMailMessageId id = idFromIndex(index);
    if (!id.isValid())
        return QVariant();

    // Some items can be processed without loading the message data
    switch(role)
    {
    case MessageIdRole:
        return id;
        break;

    case Qt::CheckStateRole:
        return (d->itemList[index.row()].isChecked() ? Qt::Checked : Qt::Unchecked);
        break;

    default:
        break;
    }

    // Otherwise, load the message data
    QMailMessageMetaData message(id);

    bool sent(message.status() & QMailMessage::Sent);
    bool incoming(message.status() & QMailMessage::Incoming);

    switch(role)
    {
        case Qt::DisplayRole:
        case MessageAddressTextRole:
            return d->messageAddressText(message,incoming);
            break;

        case MessageTimeStampTextRole:
        {
            QString action;
            if(incoming)
                action = tr("Received");
            else
            {
                if(!sent)
                    action = tr("Last edited");
                else
                    action = tr("Sent");
            }

            QDateTime messageTime(message.date().toLocalTime());
            QString date(QTimeString::localMD(messageTime.date()));
            QString time(QTimeString::localHM(messageTime.time(), QTimeString::Short));
            QString sublabel(QString("%1 %2 %3").arg(action).arg(date).arg(time));
            return sublabel;

        }break;

        case MessageSubjectTextRole:
            return message.subject();
        break;

        case MessageFilterTextRole:
            return d->messageAddressText(message,incoming) + " " + message.subject();
        break;

        case Qt::DecorationRole:
        case MessageTypeIconRole:
        {
            QIcon icon = messageIcon;
#ifdef QTOPIA_HOMEUI
            if (message.content() == QMailMessage::VideomailContent) {
                icon = videomailIcon;
            } else if (message.content() == QMailMessage::VoicemailContent) {
                icon = voicemailIcon;
            } else if (message.messageType() == QMailMessage::Email) {
                icon = emailIcon;
            }
#else
            if (message.messageType() == QMailMessage::Mms) {
                icon = mmsIcon;
            } else if (message.messageType() == QMailMessage::Email) {
                icon = emailIcon;
            } else if (message.messageType() == QMailMessage::Instant) {
                icon = instantMessageIcon;
            }
#endif
            return icon;

        }break;

        case MessageDirectionIconRole:
        {
            QIcon mainIcon = incoming ? incomingIcon : outgoingIcon;
            return mainIcon;
        }break;

        case MessageStatusIconRole:
        {
            if (incoming){ 
                quint64 status = message.status();
                if ( status & QMailMessage::Removed ) {
                    return removedIcon;
                } else if ( status & QMailMessage::Downloaded ) {
                    if ( status & QMailMessage::Read || status & QMailMessage::ReadElsewhere ) {
                        return readIcon;
                    } else {
                        return unreadIcon;
                    }
                } else {
                    return toGetIcon;
                }
            } else {
                if (sent) {
                    return readIcon;
                } else if ( message.to().isEmpty() ) {
                    // Not strictly true - there could be CC or BCC addressees
                    return unfinishedIcon;
                } else {
                    return toSendIcon;
                }
            }
        }break;

        case MessagePresenceIconRole:
        {
            QIcon icon = noPresenceIcon;

#ifdef QTOPIA_HOMEUI
            QContact contact;
            if (incoming) {
                contact = message.from().matchContact();
            } else {
                if (!message.to().isEmpty())
                    contact = message.to().first().matchContact();
            }

            if (!contact.uid().isNull()) {
                QCollectivePresenceInfo::PresenceType type = QtopiaHome::bestPresence(contact);

                switch (type) {
                    case QCollectivePresenceInfo::None:
                        break;

                    case QCollectivePresenceInfo::Hidden:
                    case QCollectivePresenceInfo::Offline:
                        icon = offlineIcon;
                        break;

                    case QCollectivePresenceInfo::Away:
                    case QCollectivePresenceInfo::ExtendedAway:
                        icon = awayIcon;
                        break;

                    case QCollectivePresenceInfo::Busy:
                        icon = busyIcon;
                        break;

                    case QCollectivePresenceInfo::Online:
                        icon = onlineIcon;
                        break;
                }
            }
#endif
            return icon;

        }break;

        case MessageBodyTextRole:
        {
            if ((message.messageType() == QMailMessage::Instant) && !message.subject().isEmpty()) {
                // For IMs that contain only text, the body is replicated in the subject
                return message.subject();
            } else {
                QMailMessage fullMessage(id);

                // Some items require the entire message data
                if (fullMessage.hasBody())
                    return fullMessage.body().data();

                return QString();
            }
        }
        break;
    }
    return QVariant();
}

/*!
    \reimp
*/

bool QMailMessageListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid()) {
        // The only role we allow to be changed is the check state
        if (role == Qt::CheckStateRole || role == Qt::EditRole) {
            Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());

            // No support for partial checking in this model
            if (state != Qt::PartiallyChecked) {
                int row = index.row();
                if (row < rowCount()) {
                    d->itemList[row].setChecked(state == Qt::Checked);
                    emit dataChanged(index, index);
                    return true;
                }
            }
        }
    }

    return false;
}

/*!
    Returns the QMailMessageKey used to populate the contents of this model.
*/

QMailMessageKey QMailMessageListModel::key() const
{
    return d->key; 
}

/*!
    Sets the QMailMessageKey used to populate the contents of the model to \a key.
    If the key is empty, the model is populated with all the messages from the 
    database.
*/

void QMailMessageListModel::setKey(const QMailMessageKey& key) 
{
    d->key = key;
    fullRefresh(true);
}

/*!
    Returns the QMailMessageSortKey used to sort the contents of the model.
*/

QMailMessageSortKey QMailMessageListModel::sortKey() const
{
   return d->sortKey;
}

/*!
    Sets the QMailMessageSortKey used to sort the contents of the model to \a sortKey.
    If the sort key is invalid, no sorting is applied to the model contents and messages
    are displayed in the order in which they were added into the database.
*/

void QMailMessageListModel::setSortKey(const QMailMessageSortKey& sortKey) 
{
    d->sortKey = sortKey;
    fullRefresh(true);
}

/*! \internal */

void QMailMessageListModel::messagesAdded(const QMailMessageIdList& ids)
{
    d->needSynchronize = true;
    if(d->ignoreUpdates)
        return;

    //TODO change this code to use our own searching and insertion routines
    //for more control
    //use id sorted indexes
    
    if(!d->init)
        return;
    
    QMailMessageKey passKey = d->key & QMailMessageKey(ids);
    QMailMessageIdList results = QMailStore::instance()->queryMessages(passKey);

    if(results.isEmpty())
        return;

    if(results.count() > fullRefreshCutoff)
        fullRefresh(false);

    if(!d->sortKey.isEmpty())
    { 
        foreach(const QMailMessageId &id,results)
        {
            LessThanFunctor lessThan(d->sortKey);

            //if sorting the list fails, then resort to a complete refresh
            if(lessThan.invalidatedList())
                fullRefresh(false);
            else
            {
                QList<QMailMessageListModelPrivate::Item>::iterator itr = d->lowerBound(id, lessThan);
                int newIndex = (itr - d->itemList.begin());

                beginInsertRows(QModelIndex(),newIndex,newIndex);
                d->itemList.insert(newIndex, QMailMessageListModelPrivate::Item(id));
                endInsertRows();
            }
        }
    }
    else
    {
        int index = d->itemList.count();

        beginInsertRows(QModelIndex(),index,(index + results.count() - 1));
        foreach(const QMailMessageId &id,results)
            d->itemList.append(QMailMessageListModelPrivate::Item(id));
        endInsertRows();
    }
    d->needSynchronize = false;
}

/*! \internal */

void QMailMessageListModel::messagesUpdated(const QMailMessageIdList& ids)
{
    d->needSynchronize = true;
    if(d->ignoreUpdates)
        return;

    //TODO change this code to use our own searching and insertion routines
    //for more control
    //use id sorted indexes

    if(!d->init)
        return;

    QMailMessageKey idKey(ids);

    QMailMessageIdList validIds = QMailStore::instance()->queryMessages(idKey & d->key);
    if(validIds.count() > fullRefreshCutoff)
    {
        fullRefresh(false);
        return;
    }

    //if the key is empty the id's will be returned valid and invalid
    if(!d->key.isEmpty())
    {
        QMailMessageIdList invalidIds = QMailStore::instance()->queryMessages(idKey & ~d->key);

        foreach(const QMailMessageId &id,invalidIds)
        {
            //get the index
            int index = d->indexOf(id);
            if(index == -1) 
                continue;

            beginRemoveRows(QModelIndex(),index,index);
            d->itemList.removeAt(index);
            endRemoveRows();
        }
    }

    LessThanFunctor lessThan(d->sortKey);

    foreach(const QMailMessageId &id, validIds)
    {
        int index = d->indexOf(id);
        if(index == -1) //insert
        {
            if(lessThan.invalidatedList())
                fullRefresh(false);
            else
            {
                QList<QMailMessageListModelPrivate::Item>::iterator itr = d->lowerBound(id, lessThan);
                int newIndex = (itr - d->itemList.begin());

                beginInsertRows(QModelIndex(),newIndex,newIndex);
                d->itemList.insert(itr, QMailMessageListModelPrivate::Item(id));
                endInsertRows();
            }

        }
        else //update
        {
            if(lessThan.invalidatedList())
                fullRefresh(false);
            else
            {
                QList<QMailMessageListModelPrivate::Item>::iterator itr = d->lowerBound(id, lessThan);
                int newIndex = (itr - d->itemList.begin());

                if((newIndex == index) || (newIndex == index + 1))
                {
                    // This item would be inserted either immediately before or after itself
                    QModelIndex modelIndex = createIndex(index,0);
                    emit dataChanged(modelIndex,modelIndex);
                }
                else
                {
                    beginRemoveRows(QModelIndex(),index,index);
                    d->itemList.removeAt(index);
                    endRemoveRows();

                    if (newIndex > index)
                        --newIndex;

                    beginInsertRows(QModelIndex(),newIndex,newIndex);
                    d->itemList.insert(newIndex, QMailMessageListModelPrivate::Item(id));
                    endInsertRows();
                }
            }
        }
    }
    d->needSynchronize = false;
}

/*! \internal */

void QMailMessageListModel::messagesRemoved(const QMailMessageIdList& ids)
{
    d->needSynchronize = true;
    if(d->ignoreUpdates)
        return;

    if(!d->init)
        return;

    QList<int> indexes;

    foreach (const QMailMessageId &id, ids) {
        int index = d->indexOf(id);
        if (index != -1 && !indexes.contains(index))
            indexes.append(index);
    }

    if (!indexes.isEmpty()) {
        qSort(indexes.begin(), indexes.end(), qGreater<int>());

        foreach (int index, indexes) {
            beginRemoveRows(QModelIndex(), index, index);
            d->itemList.removeAt(index);
            endRemoveRows();
        }
    }

    d->needSynchronize = false;
}

/*!
    Returns the QMailMessageId of the message represented by the QModelIndex \a index.
    If the index is not valid an invalid QMailMessageId is returned.
*/

QMailMessageId QMailMessageListModel::idFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return QMailMessageId();

    int row = index.row();
    if (row >= rowCount()) {
        qWarning() << "QMailMessageListModel: valid index" << row << "is out of bounds:" << rowCount();
        return QMailMessageId();
    }

    return d->items()[row].id();
}

/*!
    Returns the QModelIndex that represents the message with QMailMessageId \a id.
    If the id is not conatained in this model, an invalid QModelIndex is returned.
*/

QModelIndex QMailMessageListModel::indexFromId(const QMailMessageId& id) const
{
    if (id.isValid()) {
        //if the id does not exist return null
        int index = d->indexOf(id);
        if(index != -1)
            return createIndex(index,0);
    }

    return QModelIndex();
}

/*!
    Returns true if the model has been set to ignore updates emitted by 
    the mail store; otherwise returns false.
*/
bool QMailMessageListModel::ignoreMailStoreUpdates() const
{
    return d->ignoreUpdates;
}

/*!
    Sets whether or not mail store updates are ignored to \a ignore.

    If ignoring updates is set to true, the model will ignore updates reported 
    by the mail store.  If set to false, the model will automatically synchronize 
    its content in reaction to updates reported by the mail store.


    If updates are ignored, signals such as rowInserted and dataChanged will not 
    be emitted; instead, the modelReset signal will be emitted when the model is
    later changed to stop ignoring mail store updates, and detailed change 
    information will not be accessible.
*/
void QMailMessageListModel::setIgnoreMailStoreUpdates(bool ignore)
{
    d->ignoreUpdates = ignore;
    if (!ignore && d->needSynchronize)
        fullRefresh(false);
}

/*!
    \fn QMailMessageListModel::modelChanged()

    Signal emitted when the data set represented by the model is changed. Unlike modelReset(),
    the modelChanged() signal can not be emitted as a result of changes occurring in the 
    current data set.
*/

/*! \internal */

void QMailMessageListModel::fullRefresh(bool changed) 
{
    d->init = false;
    reset();

    if (changed)
        emit modelChanged();
}

/*! \internal */

void QMailMessageListModel::contactModelReset()
{
    d->invalidateCache();
    fullRefresh(false);
}

