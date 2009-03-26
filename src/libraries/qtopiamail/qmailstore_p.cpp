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

#include "qmailstore_p.h"
#include "qmailstore.h"
#include "qmailfoldersortkey.h"
#include "qmailfoldersortkey_p.h"
#include "qmailmessagesortkey_p.h"
#include "qmailaccountsortkey.h"
#include "qmailaccountsortkey_p.h"
#include "qmailmessagekey.h"
#include "qmailmessagekey_p.h"
#include "qmailfolderkey.h"
#include "qmailfolderkey_p.h"
#include "qmailaccountkey.h"
#include "qmailaccountkey_p.h"
#include "qmailtimestamp.h"
#include "qmailaccount.h"
#include "qmailmessageremovalrecord.h"
#include "semaphore_p.h"
#include "accountconfiguration_p.h"

#include <QSettings>
#include <qtopialog.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDSAction>
#include <QDSServiceInfo>
#include <QTextCodec>
#include <QtopiaSql>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>

#define Q_USE_SQLITE

namespace { // none of this code is externally visible:

using namespace QMailDataComparator;

// We allow queries to be specified by supplying a list of message IDs against
// which candidates will be matched; this list can become too large to be 
// expressed directly in SQL.  Instead, we will build a temporary table to
// match against when required...
// The most IDs we can include in a query is currently 999; set an upper limit  
// below this to allow for other variables in the same query, bearing in mind 
// that there may be more than one clause containing this number of IDs in the 
// same query...
const int IdLookupThreshold = 256;


// Helper class for automatic unlocking
template<typename Mutex>
class Guard
{
    Mutex &mutex;
    bool locked;

public:
    enum { DefaultTimeout = 1000 };

    Guard(Mutex& m)
        : mutex(m),
          locked(false) 
    {
    }

    ~Guard()
    {
        unlock();
    }

    bool lock(int timeout = DefaultTimeout)
    {
        return (locked = mutex.lock(timeout));
    }

    void unlock()
    {
        if (locked) {
            mutex.unlock();
            locked = false; 
        }
    }
};

typedef Guard<ProcessMutex> MutexGuard;


typedef QPair<int,int> Segment; //start,end - end non inclusive
typedef QList<Segment> SegmentList;

// Process lists in size-limited batches
SegmentList createSegments(int numItems, int segmentSize)
{
    Q_ASSERT(segmentSize > 0);

    if(numItems <= 0)
        return SegmentList();

    int segmentCount = numItems % segmentSize ? 1 : 0;
    segmentCount += numItems / segmentSize;

    SegmentList segments;
    for(int i = 0; i < segmentCount ; ++i) {
        int start = segmentSize * i;
        int end = (i+1) == segmentCount ? numItems : (i+1) * segmentSize;
        segments.append(Segment(start,end)); 
    }

    return segments;
}


// Properties of the mailmessages table
static MessagePropertyMap messagePropertyMap() 
{
    MessagePropertyMap map; 

    map.insert(QMailMessageKey::Id,"id");
    map.insert(QMailMessageKey::Type,"type");
    map.insert(QMailMessageKey::ParentFolderId,"parentfolderid");
    map.insert(QMailMessageKey::Sender,"sender");
    map.insert(QMailMessageKey::Recipients,"recipients");
    map.insert(QMailMessageKey::Subject,"subject");
    map.insert(QMailMessageKey::TimeStamp,"stamp");
    map.insert(QMailMessageKey::Status,"status");
    map.insert(QMailMessageKey::ParentAccountId,"parentaccountid");
    map.insert(QMailMessageKey::FromMailbox,"frommailbox");
    map.insert(QMailMessageKey::ServerUid,"serveruid");
    map.insert(QMailMessageKey::Size,"size");
    map.insert(QMailMessageKey::ContentType,"contenttype");
    map.insert(QMailMessageKey::PreviousParentFolderId,"previousparentfolderid");

    return map;
}

static QString messagePropertyName(QMailMessageKey::Property property)
{
    static const MessagePropertyMap map(messagePropertyMap());

    MessagePropertyMap::const_iterator it = map.find(property);
    if (it != map.end())
        return it.value();

    if (property != QMailMessageKey::AncestorFolderIds)
        qWarning() << "Unknown message property:" << property;
    
    return QString();
}

typedef QMap<QMailAccountKey::Property, QString> AccountPropertyMap;

// Properties of the mailaccounts table
static AccountPropertyMap accountPropertyMap() 
{
    AccountPropertyMap map; 

    map.insert(QMailAccountKey::Id,"id");
    map.insert(QMailAccountKey::Name,"name");
    map.insert(QMailAccountKey::MessageType,"type");
    map.insert(QMailAccountKey::EmailAddress,"emailaddress");

    return map;
}

static QString accountPropertyName(QMailAccountKey::Property property)
{
    static const AccountPropertyMap map(accountPropertyMap());

    AccountPropertyMap::const_iterator it = map.find(property);
    if (it != map.end())
        return it.value();

    qWarning() << "Unknown account property:" << property;
    return QString();
}

typedef QMap<QMailFolderKey::Property, QString> FolderPropertyMap;

// Properties of the mailfolders table
static FolderPropertyMap folderPropertyMap() 
{
    FolderPropertyMap map; 

    map.insert(QMailFolderKey::Id,"id");
    map.insert(QMailFolderKey::Name,"name");
    map.insert(QMailFolderKey::ParentId,"parentid");
    map.insert(QMailFolderKey::ParentAccountId,"parentaccountid");
    map.insert(QMailFolderKey::DisplayName,"displayname");
    map.insert(QMailFolderKey::Status,"status");

    return map;
}

static QString folderPropertyName(QMailFolderKey::Property property)
{
    static const FolderPropertyMap map(folderPropertyMap());

    FolderPropertyMap::const_iterator it = map.find(property);
    if (it != map.end())
        return it.value();

    qWarning() << "Unknown folder property:" << property;
    return QString();
}

// Build lists of column names from property values

template<typename PropertyType>
QString fieldName(PropertyType property);

template<>
QString fieldName<QMailMessageKey::Property>(QMailMessageKey::Property property)
{
    return messagePropertyName(property);
}

template<>
QString fieldName<QMailFolderKey::Property>(QMailFolderKey::Property property)
{
    return folderPropertyName(property);
}

template<>
QString fieldName<QMailAccountKey::Property>(QMailAccountKey::Property property)
{
    return accountPropertyName(property);
}

template<typename SourceType, typename TargetType>
TargetType matchingProperty(SourceType source);

static QMap<QMailMessageSortKey::Property, QMailMessageKey::Property> messageSortMapInit()
{
    QMap<QMailMessageSortKey::Property, QMailMessageKey::Property> map;

    // Provide a mapping of sort key properties to the corresponding filter key
    map.insert(QMailMessageSortKey::Id, QMailMessageKey::Id);
    map.insert(QMailMessageSortKey::Type, QMailMessageKey::Type);
    map.insert(QMailMessageSortKey::ParentFolderId, QMailMessageKey::ParentFolderId);
    map.insert(QMailMessageSortKey::Sender, QMailMessageKey::Sender);
    map.insert(QMailMessageSortKey::Recipients, QMailMessageKey::Recipients);
    map.insert(QMailMessageSortKey::Subject, QMailMessageKey::Subject);
    map.insert(QMailMessageSortKey::TimeStamp, QMailMessageKey::TimeStamp);
    map.insert(QMailMessageSortKey::Status, QMailMessageKey::Status);
    map.insert(QMailMessageSortKey::ParentAccountId, QMailMessageKey::ParentAccountId);
    map.insert(QMailMessageSortKey::FromMailbox, QMailMessageKey::FromMailbox);
    map.insert(QMailMessageSortKey::ServerUid, QMailMessageKey::ServerUid);
    map.insert(QMailMessageSortKey::Size, QMailMessageKey::Size);
    map.insert(QMailMessageSortKey::ContentType, QMailMessageKey::ContentType);
    map.insert(QMailMessageSortKey::PreviousParentFolderId, QMailMessageKey::PreviousParentFolderId);

    return map;
}

template<>
QMailMessageKey::Property matchingProperty<QMailMessageSortKey::Property, QMailMessageKey::Property>(QMailMessageSortKey::Property source)
{
    static QMap<QMailMessageSortKey::Property, QMailMessageKey::Property> map(messageSortMapInit());
    return map.value(source);
}

static QMap<QMailFolderSortKey::Property, QMailFolderKey::Property> folderSortMapInit()
{
    QMap<QMailFolderSortKey::Property, QMailFolderKey::Property> map;

    // Provide a mapping of sort key properties to the corresponding filter key
    map.insert(QMailFolderSortKey::Id, QMailFolderKey::Id);
    map.insert(QMailFolderSortKey::Name, QMailFolderKey::Name);
    map.insert(QMailFolderSortKey::ParentId, QMailFolderKey::ParentId);
    map.insert(QMailFolderSortKey::ParentAccountId, QMailFolderKey::ParentAccountId);
    map.insert(QMailFolderSortKey::DisplayName, QMailFolderKey::DisplayName);
    map.insert(QMailFolderSortKey::Status, QMailFolderKey::Status);

    return map;
}

template<>
QMailFolderKey::Property matchingProperty<QMailFolderSortKey::Property, QMailFolderKey::Property>(QMailFolderSortKey::Property source)
{
    static QMap<QMailFolderSortKey::Property, QMailFolderKey::Property> map(folderSortMapInit());
    return map.value(source);
}

static QMap<QMailAccountSortKey::Property, QMailAccountKey::Property> accountSortMapInit()
{
    QMap<QMailAccountSortKey::Property, QMailAccountKey::Property> map;

    // Provide a mapping of sort key properties to the corresponding filter key
    map.insert(QMailAccountSortKey::Id, QMailAccountKey::Id);
    map.insert(QMailAccountSortKey::Name, QMailAccountKey::Name);
    map.insert(QMailAccountSortKey::MessageType, QMailAccountKey::MessageType);
    map.insert(QMailAccountSortKey::EmailAddress, QMailAccountKey::EmailAddress);

    return map;
}

template<>
QMailAccountKey::Property matchingProperty<QMailAccountSortKey::Property, QMailAccountKey::Property>(QMailAccountSortKey::Property source)
{
    static QMap<QMailAccountSortKey::Property, QMailAccountKey::Property> map(accountSortMapInit());
    return map.value(source);
}

template<>
QString fieldName<QMailMessageSortKey::Property>(QMailMessageSortKey::Property property)
{
    return messagePropertyName(matchingProperty<QMailMessageSortKey::Property, QMailMessageKey::Property>(property));
}

template<>
QString fieldName<QMailFolderSortKey::Property>(QMailFolderSortKey::Property property)
{
    return folderPropertyName(matchingProperty<QMailFolderSortKey::Property, QMailFolderKey::Property>(property));
}

template<>
QString fieldName<QMailAccountSortKey::Property>(QMailAccountSortKey::Property property)
{
    return accountPropertyName(matchingProperty<QMailAccountSortKey::Property, QMailAccountKey::Property>(property));
}

template<typename PropertyType>
QString fieldNames(const QList<PropertyType> &properties, const QString &separator)
{
    QStringList fields;
    foreach (const PropertyType &property, properties)
        fields.append(fieldName(property));

    return fields.join(separator);
}


template <typename Key, typename Argument>
class ArgumentExtractorBase
{
protected:
    const Argument &arg;
    
    ArgumentExtractorBase(const Argument &a) : arg(a) {}

    QString minimalString(const QString &s) const
    {
        // If the argument is a phone number, ensure it is in minimal form
        QMailAddress address(s);
        if (address.isPhoneNumber()) {
            QString minimal(address.minimalPhoneNumber());

            // Rather than compare exact numbers, we will only use the trailing
            // digits to compare phone numbers - otherwise, slightly different 
            // forms of the same number will not be matched
            static const int significantDigits = 8;

            int extraneous = minimal.length() - significantDigits;
            if (extraneous > 0)
                minimal.remove(0, extraneous);

            return minimal;
        }

        return s;
    }

    QString submatchString(const QString &s, bool valueMinimalised) const
    {
        if (!s.isEmpty()) {
            //delimit data for sql "LIKE" operator
            if ((arg.op == Includes) || ((arg.op == Equal) && valueMinimalised))
                return QString("\%" + s + "\%");
        }

        return s;
    }

    QString addressStringValue() const
    {
        return submatchString(minimalString(QMailStorePrivate::extractValue<QString>(arg.valueList.first())), true);
    }

    QString stringValue() const
    {
        return submatchString(QMailStorePrivate::extractValue<QString>(arg.valueList.first()), false);
    }

    template<typename ID>
    quint64 idValue() const
    {
        return QMailStorePrivate::extractValue<ID>(arg.valueList.first()).toULongLong(); 
    }

    template<typename ID>
    QVariantList idValues() const
    {
        QVariantList values;

        foreach (const QVariant &item, arg.valueList)
            values.append(QMailStorePrivate::extractValue<ID>(item).toULongLong());

        return values;
    }

    int intValue() const
    {
        return QMailStorePrivate::extractValue<int>(arg.valueList.first());
    }

    int quint64Value() const
    {
        return QMailStorePrivate::extractValue<quint64>(arg.valueList.first());
    }

    template <typename ClauseKey>
    QVariantList keyOrIdValue() const 
    {
        const QVariant& var = arg.valueList.first();

        if (qVariantCanConvert<ClauseKey>(var)) {
            return QMailStorePrivate::whereClauseValues(qVariantValue<ClauseKey>(var));
        } else {
            return QVariantList() << idValue<typename ClauseKey::IdType>();
        }
    }
};


template<typename PropertyType, typename BitmapType = int>
class RecordExtractorBase
{
protected:
    const QSqlRecord &record;
    const BitmapType bitmap;

    RecordExtractorBase(const QSqlRecord &r, BitmapType b = 0) : record(r), bitmap(b) {}
    virtual ~RecordExtractorBase() {}
    
    template<typename ValueType>
    ValueType value(const QString &field, const ValueType &defaultValue = ValueType()) const 
    { 
        int index(fieldIndex(field, bitmap));

        if (record.isNull(index))
            return defaultValue;
        else
            return QMailStorePrivate::extractValue<ValueType>(record.value(index), defaultValue);
    }
    
    template<typename ValueType>
    ValueType value(PropertyType p, const ValueType &defaultValue = ValueType()) const 
    { 
        return value(fieldName(p), defaultValue);
    }

    virtual int fieldIndex(const QString &field, BitmapType b) const = 0;

    int mappedFieldIndex(const QString &field, BitmapType bitmap, QMap<BitmapType, QMap<QString, int> > &fieldIndex) const
    {
        typename QMap<BitmapType, QMap<QString, int> >::iterator it = fieldIndex.find(bitmap);
        if (it == fieldIndex.end()) {
            it = fieldIndex.insert(bitmap, QMap<QString, int>());
        }

        QMap<QString, int> &fields(it.value());

        QMap<QString, int>::iterator fit = fields.find(field);
        if (fit != fields.end())
            return fit.value();

        int index = record.indexOf(field);
        fields.insert(field, index);
        return index;
    }
};


// Class to extract data from records of the mailmessages table
class MessageRecord : public RecordExtractorBase<QMailMessageKey::Property, QMailMessageKey::Properties>
{
public:
    MessageRecord(const QSqlRecord &r, QMailMessageKey::Properties props) 
        : RecordExtractorBase<QMailMessageKey::Property, QMailMessageKey::Properties>(r, props) {}

    QMailMessageId id() const { return QMailMessageId(value<quint64>(QMailMessageKey::Id)); }

    QMailMessage::MessageType messageType() const { return QMailMessage::MessageType(value<int>(QMailMessageKey::Type, QMailMessage::None)); }

    QMailFolderId parentFolderId() const { return QMailFolderId(value<quint64>(QMailMessageKey::ParentFolderId)); }

    QMailAddress from() const 
    { 
        static const QString smsTag("@sms");

        QString sender(value<QString>(QMailMessageKey::Sender));

        // Remove sms-origin tag, if present - the SMS client previously appended
        // "@sms" to the from address, which is no longer necesary
        if (sender.endsWith(smsTag))
            sender.chop(smsTag.length());

        return QMailAddress(sender);
    }

    QList<QMailAddress> to() const { return QMailAddress::fromStringList(value<QString>(QMailMessageKey::Recipients)); }

    QString subject() const { return value<QString>(QMailMessageKey::Subject); }

    QMailTimeStamp date() const { return QMailTimeStamp(value<QDateTime>(QMailMessageKey::TimeStamp)); }

    quint64 status() const { return value<quint64>(QMailMessageKey::Status, 0); }

    QMailAccountId parentAccountId() const { return QMailAccountId(value<quint64>(QMailMessageKey::ParentAccountId)); }

    QString fromMailbox() const { return value<QString>(QMailMessageKey::FromMailbox); }

    QString serverUid() const { return value<QString>(QMailMessageKey::ServerUid); }

    int size() const { return value<int>(QMailMessageKey::Size); }

    QMailMessage::ContentType content() const { return QMailMessage::ContentType(value<int>(QMailMessageKey::ContentType, QMailMessage::UnknownContent)); }

    QMailFolderId previousParentFolderId() const { return QMailFolderId(value<quint64>(QMailMessageKey::PreviousParentFolderId)); }

private:
    int fieldIndex(const QString &field, QMailMessageKey::Properties props) const
    {
        return mappedFieldIndex(field, props, _fieldIndex);
    }

    static QMap<QMailMessageKey::Properties, QMap<QString, int> > _fieldIndex;
};

QMap<QMailMessageKey::Properties, QMap<QString, int> > MessageRecord::_fieldIndex;

// Class to convert QMailMessageKey argument values to SQL bind values
class MessageKeyArgumentExtractor : public ArgumentExtractorBase<QMailMessageKey, QMailMessageKeyPrivate::Argument >
{
public:
    MessageKeyArgumentExtractor(const QMailMessageKeyPrivate::Argument &a) 
        : ArgumentExtractorBase<QMailMessageKey, QMailMessageKeyPrivate::Argument>(a) {}

    QVariantList id() const { return idValues<QMailMessageId>(); }

    QVariant messageType() const {  return intValue(); }

    QVariantList parentFolderId() const { return keyOrIdValue<QMailFolderKey>(); }

    QVariant ancestorFolderIds() const {  return idValue<QMailFolderId>(); }

    QVariant sender() const { return addressStringValue(); }

    QVariant recipients() const { return addressStringValue(); }

    QVariant subject() const { return stringValue(); }

    QVariant date() const
    { 
        return QMailStorePrivate::extractValue<QDateTime>(arg.valueList.first());
    }

    QVariant status() const
    {
        // The UnloadedData flag has no meaningful persistent value
        return (QMailStorePrivate::extractValue<quint64>(arg.valueList.first()) & ~QMailMessage::UnloadedData);
    }

    QVariantList parentAccountId() const { return keyOrIdValue<QMailAccountKey>(); }

    QVariant fromMailbox() const { return stringValue(); }

    QVariant serverUid() const { return stringValue(); }

    QVariant size() const { return intValue(); }

    QVariant content() const { return intValue(); }

    QVariantList previousParentFolderId() const { return keyOrIdValue<QMailFolderKey>(); }
};


// Class to extract data from records of the mailaccounts table
class AccountRecord : public RecordExtractorBase<QMailAccountKey::Property>
{
public:
    AccountRecord(const QSqlRecord &r) 
        : RecordExtractorBase<QMailAccountKey::Property>(r) {}

    QMailAccountId id() const { return QMailAccountId(value<quint64>(QMailAccountKey::Id)); }

    QString name() const { return value<QString>(QMailAccountKey::Name); }

    QMailMessage::MessageType messageType() const { return QMailMessage::MessageType(value<int>(QMailAccountKey::MessageType, -1)); }

    QString emailAddress() const { return value<QString>(QMailAccountKey::EmailAddress); }

private:
    int fieldIndex(const QString &field, int props) const
    {
        return mappedFieldIndex(field, props, _fieldIndex);
    }

    static QMap<int, QMap<QString, int> > _fieldIndex;
};

QMap<int, QMap<QString, int> > AccountRecord::_fieldIndex;

// Class to convert QMailAccountKey argument values to SQL bind values
class AccountKeyArgumentExtractor : public ArgumentExtractorBase<QMailAccountKey, QMailAccountKeyPrivate::Argument>
{
public:
    AccountKeyArgumentExtractor(const QMailAccountKeyPrivate::Argument &a)
        : ArgumentExtractorBase<QMailAccountKey, QMailAccountKeyPrivate::Argument>(a) {}

    QVariantList id() const { return idValues<QMailAccountId>(); }

    QVariant name() const { return stringValue(); }

    QVariant messageType() const { return intValue(); }

    QVariant emailAddress() const { return stringValue(); }
};


// Class to extract data from records of the mailfolders table
class FolderRecord : public RecordExtractorBase<QMailFolderKey::Property>
{
public:
    FolderRecord(const QSqlRecord &r)
        : RecordExtractorBase<QMailFolderKey::Property>(r) {}

    QMailFolderId id() const { return QMailFolderId(value<quint64>(QMailFolderKey::Id)); }

    QString name() const { return value<QString>(QMailFolderKey::Name); }

    QMailFolderId parentId() const { return QMailFolderId(value<quint64>(QMailFolderKey::ParentId)); }

    QMailAccountId parentAccountId() const { return QMailAccountId(value<quint64>(QMailFolderKey::ParentAccountId)); }

    QString displayName() const { return value<QString>(QMailFolderKey::DisplayName); }

    quint64 status() const { return value<quint64>(QMailFolderKey::Status); }

private:
    int fieldIndex(const QString &field, int props) const
    {
        return mappedFieldIndex(field, props, _fieldIndex);
    }

    static QMap<int, QMap<QString, int> > _fieldIndex;
};

QMap<int, QMap<QString, int> > FolderRecord::_fieldIndex;

// Class to convert QMailFolderKey argument values to SQL bind values
class FolderKeyArgumentExtractor : public ArgumentExtractorBase<QMailFolderKey, QMailFolderKeyPrivate::Argument>
{
public:
    FolderKeyArgumentExtractor(const QMailFolderKeyPrivate::Argument &a)
        : ArgumentExtractorBase<QMailFolderKey, QMailFolderKeyPrivate::Argument>(a) {}

    QVariantList id() const { return idValues<QMailFolderId>(); }

    QVariant name() const { return stringValue(); }

    QVariantList parentId() const { return keyOrIdValue<QMailFolderKey>(); }

    QVariant ancestorFolderIds() const {  return idValue<QMailFolderId>(); }

    QVariantList parentAccountId() const { return keyOrIdValue<QMailAccountKey>(); }

    QVariant displayName() const { return stringValue(); }

    QVariant status() const { return quint64Value(); }
};


// Class to extract data from records of the deletedmessages table
class MessageRemovalRecord : public RecordExtractorBase<int>
{
public:
    MessageRemovalRecord(const QSqlRecord &r)
        : RecordExtractorBase<int>(r) {}

    quint64 id() const { return value<quint64>("id"); }

    QMailAccountId parentAccountId() const { return QMailAccountId(value<quint64>("parentaccountid")); }

    QString serverUid() const { return value<QString>("serveruid"); }

    QString fromMailbox() const { return value<QString>("frommailbox"); }

private:
    int fieldIndex(const QString &field, int props) const
    {
        return mappedFieldIndex(field, props, _fieldIndex);
    }

    static QMap<int, QMap<QString, int> > _fieldIndex;
};

QMap<int, QMap<QString, int> > MessageRemovalRecord::_fieldIndex;


template<typename ArgumentListType>
QString buildOrderClause(const ArgumentListType &list)
{
    QStringList sortColumns;
    foreach (typename ArgumentListType::const_reference arg, list)
        sortColumns.append(fieldName(arg.first) + ' ' + (arg.second == Qt::AscendingOrder ? "ASC" : "DESC"));

    return QString(" ORDER BY ") + sortColumns.join(",");
}


template<typename OpType>
QString operatorString(OpType op, bool multipleArgs);

template<>
QString operatorString<QMailDataComparator::Comparator>(QMailDataComparator::Comparator op, bool multipleArgs)
{
    switch (op) 
    {
    case Equal:
        return (multipleArgs ? " IN " : " = ");
        break;

    case NotEqual:
        return " <> ";
        break;

    case LessThan:
        return " < ";
        break;

    case LessThanEqual:
        return " <= ";
        break;

    case GreaterThan:
        return " > ";
        break;

    case GreaterThanEqual:
        return " >= ";
        break;

    case Includes:
        return " LIKE ";
        break;

    case Excludes:
        // Should never occur due to conversion to !Includes
        break;
    }

    return QString();
}


template<typename OpType>
QString combineOperatorString(OpType op);

template<>
QString combineOperatorString<QMailDataComparator::Combiner>(QMailDataComparator::Combiner op)
{
    switch (op) 
    {
    case And:
        return " AND ";
        break;

    case Or:
        return " OR ";
        break;

    case None:
        break;
    }

    return QString();
}


template<typename ArgumentType, typename KeyType>
QString whereClauseItem(const ArgumentType &arg, const KeyType &key, const QMailStorePrivate &store);

template<>
QString whereClauseItem<QMailAccountKeyPrivate::Argument, QMailAccountKey>(const QMailAccountKeyPrivate::Argument &a, const QMailAccountKey &, const QMailStorePrivate &)
{
    QString item;
    {
        QTextStream q(&item);

        if (a.property == QMailAccountKey::MessageType) {
            // Currently, we have a table of account types - convert message type in query to an account type comparison
            QVariantList values;
            foreach (int type, QMailAccount::matchingAccountTypes(QMailMessage::MessageType(a.valueList.first().toInt())))
                values.append(QVariant(type));

            QString comparator = operatorString(a.op, values.count() > 1);
            q << fieldName(a.property) << comparator << QMailStorePrivate::expandValueList(values);
        } else {
            QString comparator = operatorString(a.op, a.valueList.count() > 1);
            q << fieldName(a.property) << comparator << QMailStorePrivate::expandValueList(a.valueList);
        }
    }
    return item;
}

template<>
QString whereClauseItem<QMailMessageKeyPrivate::Argument, QMailMessageKey>(const QMailMessageKeyPrivate::Argument &a, const QMailMessageKey &key, const QMailStorePrivate &store)
{
    QString item;
    {
        QTextStream q(&item);

        bool addressRelated(a.property == QMailMessageKey::Sender || a.property == QMailMessageKey::Recipients);
        bool multipleArgs(a.valueList.count() > 1);

        Comparator cmp(a.op);
        if (addressRelated && !multipleArgs && cmp == Equal) {
            // When matching addresses, we will force equality to actually use a content match
            cmp = Includes;
        }

        QString comparator = operatorString(cmp, multipleArgs);

        QString columnName;
        if (a.property == QMailMessageKey::AncestorFolderIds) {
            // AncestorFolderIds is a property derived from the ParentFolderId data
            columnName = fieldName(QMailMessageKey::ParentFolderId);
        } else {
            columnName = fieldName(a.property);
        }

        switch(a.property)
        {
        case QMailMessageKey::Id:
            if (a.valueList.count() >= IdLookupThreshold) {
                q << columnName << " IN ( SELECT id from " << QMailStorePrivate::temporaryTableName(key) << ")";
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList); 
            }
            break;

        case QMailMessageKey::Type:
            if (a.op == Includes) {
                q << columnName << " & ?";
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList); 
            }
            break;

        case QMailMessageKey::ParentFolderId:
        case QMailMessageKey::PreviousParentFolderId:
            if(a.valueList.first().canConvert<QMailFolderKey>()) {
                QMailFolderKey parentFolderKey = a.valueList.first().value<QMailFolderKey>();
                q << columnName << " IN ( SELECT id FROM mailfolders WHERE " << store.buildWhereClause(parentFolderKey) << ")"; 
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList);
            }
            break;

        case QMailMessageKey::AncestorFolderIds:
            // Only the 'includes' relationship is meaningful for this property
            if (a.op == Includes && a.valueList.count() == 1) {
                q << columnName << " IN ( SELECT DISTINCT descendantid FROM mailfolderlinks WHERE id = ? )";
            } 
            break;

        case QMailMessageKey::Status:
            if(a.op == Includes)
                q << columnName << " & ?";
            else
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList); 
            break;

        case QMailMessageKey::ParentAccountId:
            if(a.valueList.first().canConvert<QMailAccountKey>()) {
                QMailAccountKey parentAccountKey = a.valueList.first().value<QMailAccountKey>();
                q << columnName << " IN ( SELECT id FROM mailaccounts WHERE " << store.buildWhereClause(parentAccountKey) << ")"; 
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList);
            }
            break;

        case QMailMessageKey::Sender:
        case QMailMessageKey::Recipients:
        case QMailMessageKey::Subject:
        case QMailMessageKey::TimeStamp:
        case QMailMessageKey::FromMailbox:
        case QMailMessageKey::ServerUid:
        case QMailMessageKey::Size:
        case QMailMessageKey::ContentType:
            q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList); 
            break;     
        }
    }
    return item;
}

template<>
QString whereClauseItem<QMailFolderKeyPrivate::Argument, QMailFolderKey>(const QMailFolderKeyPrivate::Argument &a, const QMailFolderKey &, const QMailStorePrivate &store)
{
    QString item;
    {
        QTextStream q(&item);

        QString comparator = operatorString(a.op, a.valueList.count() > 1);
        QString columnName(fieldName(a.property));

        switch (a.property)
        {
        case QMailFolderKey::Id:
        case QMailFolderKey::Name:
        case QMailFolderKey::DisplayName:
            q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList);            
            break;

        case QMailFolderKey::ParentId:
            if(a.valueList.first().canConvert<QMailFolderKey>()) {
                QMailFolderKey folderSubKey = a.valueList.first().value<QMailFolderKey>();
                q << columnName << " IN ( SELECT id FROM mailfolders WHERE " << store.buildWhereClause(folderSubKey) << ")"; 
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList);
            }
            break;

        case QMailFolderKey::AncestorFolderIds:
            // Only the 'includes' relationship is meaningful for this property
            if (a.op == Includes && a.valueList.count() == 1) {
                q << fieldName(QMailFolderKey::Id) << " IN ( SELECT DISTINCT descendantid FROM mailfolderlinks WHERE id = ? )";
            } 
            break;

        case QMailFolderKey::ParentAccountId:
            if(a.valueList.first().canConvert<QMailAccountKey>()) {
                QMailAccountKey accountSubKey = a.valueList.first().value<QMailAccountKey>();
                q << columnName << " IN ( SELECT id FROM mailaccounts WHERE " << store.buildWhereClause(accountSubKey) << ")"; 
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList);
            }
            break;

        case QMailFolderKey::Status:
            if(a.op == Includes) {
                q << columnName << " & ?";
            } else {
                q << columnName << comparator << QMailStorePrivate::expandValueList(a.valueList); 
            }
            break;
        }
    }
    return item;
}

template<typename ImplType, typename KeyType, typename CombineType, typename ArgumentListType, typename KeyListType>
QString buildWhereClause(const KeyType &key, CombineType combine, const ArgumentListType &args, const KeyListType &subKeys, bool negated, const QMailStorePrivate& store)
{
    QString logicalOpString(combineOperatorString(combine));

    QString queryString;
    QTextStream q(&queryString);

    QString op = " ";
    foreach (typename ArgumentListType::const_reference a, args) {
        q << op << whereClauseItem(a, key, store);

        op = logicalOpString;
    }

    // subkeys
    if (queryString.isEmpty())
        op = " ";

    foreach (typename KeyListType::const_reference subkey, subKeys) {
        QString subquery = store.buildWhereClause(subkey);
        q << op << " ( " << subquery << " ) ";

        op = logicalOpString;
    }       

    if (negated)
        return " NOT (" + queryString + ")";
    else
        return queryString;
}

QPair<QString, qint64> tableInfo(const QString &name, qint64 version)
{
    return qMakePair(name, version);
}

QPair<quint64, QString> folderInfo(QMailFolder::StandardFolder id, const QString &name)
{
    return qMakePair(static_cast<quint64>(id), name);
}

} // namespace

bool QMailStorePrivate::init = false;

// QMailStorePrivate


// We need to support recursive locking, per-process
static volatile int mutexLockCount = 0;
static volatile int readLockCount = 0;

MailStoreTransaction::MailStoreTransaction(QMailStorePrivate* d)
    : m_d(d), 
      m_initted(false),
      m_committed(false)
{
    if (mutexLockCount > 0) {
        // Increase lock recursion depth
        ++mutexLockCount;
        m_initted = true;
    } else {
        // This process does not yet have a mutex lock
        if (m_d->databaseMutex().lock(10000)) {
            // Wait for any readers to complete
            if (m_d->databaseReadLock().wait(10000)) {
                if (m_d->transaction()) {
                    ++mutexLockCount;
                    m_initted = true;
                }
            } else {
                qWarning() << "Unable to wait for database read lock to reach zero!";
            }

            if (!m_initted) {
                m_d->databaseMutex().unlock();
            }
        } else {
            qWarning() << "Unable to lock database mutex for transaction!";
        }
    }
}

MailStoreTransaction::~MailStoreTransaction()
{
    if (m_initted && !m_committed) {
        m_d->rollback();

        --mutexLockCount;
        if (mutexLockCount == 0)
            m_d->databaseMutex().unlock();
    }
}

bool MailStoreTransaction::commit()
{
    if (m_initted && !m_committed) {
        if (m_committed = m_d->commit()) {
            --mutexLockCount;
            if (mutexLockCount == 0)
                m_d->databaseMutex().unlock();
        }
    }

    return m_committed;
}

bool MailStoreTransaction::committed() const
{
    return m_committed;
}


MailStoreReadLock::MailStoreReadLock(QMailStorePrivate* d)
    : m_d(d),
      m_locked(false)
{
    if (readLockCount > 0) {
        // Increase lock recursion depth
        ++readLockCount;
        m_locked = true;
    } else {
        // This process does not yet have a read lock
        // Lock the mutex to ensure no writers are active or waiting (unless we have already locked it)
        if ((mutexLockCount > 0) || m_d->databaseMutex().lock(10000)) {
            m_d->databaseReadLock().lock();
            ++readLockCount;
            m_locked = true;

            if (mutexLockCount == 0)
                m_d->databaseMutex().unlock();
        } else {
            qWarning() << "Unable to lock database mutex for read lock!";
        }
    }
}

MailStoreReadLock::~MailStoreReadLock()
{
    if (m_locked) {
        --readLockCount;
        if (readLockCount == 0)
            m_d->databaseReadLock().unlock();
    }
}


QMailMessageKey::Properties QMailStorePrivate::updatableMessageProperties()
{
    static QMailMessageKey::Properties p = QMailMessageKey::ParentFolderId |
                                           QMailMessageKey::Type |
                                           QMailMessageKey::Sender |
                                           QMailMessageKey::Recipients |
                                           QMailMessageKey::Subject |
                                           QMailMessageKey::TimeStamp |
                                           QMailMessageKey::Status |
                                           QMailMessageKey::ParentAccountId |
                                           QMailMessageKey::FromMailbox |
                                           QMailMessageKey::ServerUid |
                                           QMailMessageKey::Size |
                                           QMailMessageKey::ContentType |
                                           QMailMessageKey::PreviousParentFolderId;
    return p;
}

QMailMessageKey::Properties QMailStorePrivate::allMessageProperties()
{
    static QMailMessageKey::Properties p = QMailMessageKey::Id | updatableMessageProperties();
    return p;
}

const MessagePropertyMap& QMailStorePrivate::messagePropertyMap() 
{
    static const MessagePropertyMap map(::messagePropertyMap());
    return map;
}

const MessagePropertyList& QMailStorePrivate::messagePropertyList() 
{
    static const MessagePropertyList list(messagePropertyMap().keys());
    return list;
}

QString QMailStorePrivate::comparitorWarning()
{
    static QString s("Warning!! Queries with many comparitors can cause problems. "
                     "Prefer multiple calls with smaller keys when possible."); //no tr
    return s;
}

QString QMailStorePrivate::accountAddedSig()
{
    static QString s("accountAdded(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::accountRemovedSig()
{
    static QString s("accountRemoved(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::accountUpdatedSig()
{
    static QString s("accountUpdated(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::accountContentsModifiedSig()
{
    static QString s("accountContentsModified(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::messageAddedSig()
{
    static QString s("messageAdded(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::messageRemovedSig()
{
    static QString s("messageRemoved(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::messageUpdatedSig()
{
    static QString s("messageUpdated(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::folderAddedSig()
{
    static QString s("folderAdded(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::folderRemovedSig()
{
    static QString s("folderRemoved(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::folderUpdatedSig()
{
    static QString s("folderUpdated(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::folderContentsModifiedSig()
{
    static QString s("folderContentsModified(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::messageRemovalRecordsAddedSig()
{
    static QString s("messageRemovalRecordsAdded(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::messageRemovalRecordsRemovedSig()
{
    static QString s("messageRemovalRecordsRemoved(int,QList<quint64>)");
    return s;
}

QString QMailStorePrivate::messagesBodyPath() 
{
    static QString path = Qtopia::applicationFileName("qtopiamail", "mail");
    return path;
}

QString QMailStorePrivate::accountSettingsFileName() 
{
    static QString name("qtopiamail");
    return name;
}

QString QMailStorePrivate::accountSettingsPrefix()
{
    static QString prefix("Trolltech");
    return prefix;
}

QString QMailStorePrivate::globalAccountSettingsKey()
{
    static QString key("accountglobal");
    return key;
}

int QMailStorePrivate::accountSettingsFileIdentifier()
{
    static int id = 0;
    if (id == 0) {
        QSettings settings(accountSettingsPrefix(), accountSettingsFileName());
        id = static_cast<int>(::ftok(settings.fileName().toAscii(), 1));
    }

    return id;
}

QString QMailStorePrivate::messageFilePath(const QString &fileName)
{
    return messagesBodyPath() + "/" + fileName;
}

int QMailStorePrivate::messageFileIdentifier(const QString &filePath)
{
    return static_cast<int>(::ftok(filePath.toAscii(), 1));
}

int QMailStorePrivate::databaseIdentifier(int n) const
{
    return static_cast<int>(::ftok(database.databaseName().toAscii(), n));
}


ProcessMutex* QMailStorePrivate::settingsMutex = 0;
ProcessMutex* QMailStorePrivate::messageMutex = 0;

QMailStorePrivate::QMailStorePrivate(QMailStore* parent)
:
    QObject(parent),
    q(parent),
    headerCache(headerCacheSize),
    folderCache(folderCacheSize),
    accountCache(accountCacheSize),
    inTransaction(false),
    asyncEmission(false),
    lastError(0),
    mutex(0)
{
    Q_ASSERT(q);
    QtopiaChannel* ipcChannel = new QtopiaChannel("QPE/Qtopiamail",this);
    connect(ipcChannel,
            SIGNAL(received(QString,QByteArray)),
            this,
            SLOT(ipcMessage(QString,QByteArray)));
    flushTimer.setSingleShot(true);
    preFlushTimer.setSingleShot(true);
    queueTimer.setSingleShot(true);
    connect(&flushTimer,
            SIGNAL(timeout()),
            this,
            SLOT(notifyFlush()));
    connect(&queueTimer,
            SIGNAL(timeout()),
            this,
            SLOT(processIpcMessageQueue()));

    //make sure messages body path exists
    
    QDir dir(messagesBodyPath());
    if(!dir.exists())
        if(!dir.mkpath(messagesBodyPath()))
            qWarning() << "Unable to create messages storage directory " << messagesBodyPath();

    //open the database

    database = QtopiaSql::instance()->applicationSpecificDatabase("qtopiamail");

    mutex = new ProcessMutex(databaseIdentifier(1));
    readLock = new ProcessReadLock(databaseIdentifier(2));

    MutexGuard guard(databaseMutex());
    if (guard.lock(1000)) {
        if (settingsMutex == 0) {
            settingsMutex = new ProcessMutex(accountSettingsFileIdentifier());
            messageMutex = new ProcessMutex(messageFileIdentifier("."));
        }
    }
}

QMailStorePrivate::~QMailStorePrivate()
{
    delete mutex;
    delete readLock;
}

ProcessMutex& QMailStorePrivate::databaseMutex(void) const
{
    return *mutex;
}

ProcessReadLock& QMailStorePrivate::databaseReadLock(void) const
{
    return *readLock;
}

ProcessMutex& QMailStorePrivate::accountSettingsFileMutex(void)
{
    return *settingsMutex;
}

ProcessMutex& QMailStorePrivate::messageFileMutex(void)
{
    return *messageMutex;
}

bool QMailStorePrivate::transaction(void)
{
    if (inTransaction) {
        qLog(Messaging) << "(" << ::getpid() << ")" << "Transaction already exists at begin!";
        qWarning() << "Transaction already exists at begin!";
    }

    clearLastError();

    // Ensure any outstanding temp tables are removed before we begin this transaction
    destroyTemporaryTables();

    if (!database.transaction()) {
        setLastError(database.lastError(), "Failed to initiate transaction");
        return false;
    }

    inTransaction = true;
    return true;
}

QSqlQuery QMailStorePrivate::prepare(const QString& sql)
{
    if (!inTransaction) {
        // Ensure any outstanding temp tables are removed before we begin this query
        destroyTemporaryTables();
    }

    clearLastError();

    QSqlQuery query(database);
    
    // Create any temporary tables needed for this query
    while (!requiredTableKeys.isEmpty()) {
        const QMailMessageKey *key = requiredTableKeys.takeFirst();
        if (!temporaryTableKeys.contains(key)) {
            QString tableName = temporaryTableName(*key);

            bool ok = true;
            QSqlQuery tableQuery(database);
            if (!tableQuery.exec(QString("CREATE TEMP TABLE %1 ( id INTEGER PRIMARY KEY )").arg(tableName))) { 
                ok = false;
            } else {
                temporaryTableKeys.append(key);

                // Add the ID values to the temp table
                foreach (const QVariant &var, key->d->arguments.first().valueList) {
                    quint64 id = 0;

                    if (qVariantCanConvert<QMailMessageId>(var)) {
                        id = var.value<QMailMessageId>().toULongLong(); 
                    } else if (qVariantCanConvert<QMailFolderId>(var)) {
                        id = var.value<QMailFolderId>().toULongLong(); 
                    } else if (qVariantCanConvert<QMailAccountId>(var)) {
                        id = var.value<QMailAccountId>().toULongLong(); 
                    }

                    if (id != 0) {
                        tableQuery = QSqlQuery(database);
                        if (!tableQuery.exec(QString("INSERT INTO %1 VALUES (%2)").arg(tableName).arg(id))) { 
                            ok = false;
                            break;
                        }
                    } else {
                        qLog(Messaging) << "Unable to extract ID value from valuelist!";
                        ok = false;
                        break;
                    }
                }
            }
            
            if (!ok) {
                setLastError(tableQuery.lastError(), "Failed to create temporary table", tableQuery.lastQuery());
                qLog(Messaging) << "Unable to prepare query:" << sql;
                return query;
            }
        }
    }

    if (!query.prepare(sql)) {
        setLastError(query.lastError(), "Failed to prepare query", query.lastQuery());
    }

    return query;
}

bool QMailStorePrivate::execute(QSqlQuery& query, bool batch)
{
    bool success = (batch ? query.execBatch() : query.exec());
    if (!success) {
        setLastError(query.lastError(), "Failed to execute query", query.lastQuery());
        return false;
    }

#ifdef QMAILSTORE_LOG_SQL 
    qLog(Messaging) << "(" << ::getpid() << ")" << query.executedQuery().simplified();
#endif

    if (!inTransaction) {
        // We should be finished with these temporary tables
        expiredTableKeys = temporaryTableKeys;
        temporaryTableKeys.clear();
    }

    return true;
}

bool QMailStorePrivate::commit(void)
{
    if (!inTransaction) {
        qLog(Messaging) << "(" << ::getpid() << ")" << "Transaction does not exist at commit!";
        qWarning() << "Transaction does not exist at commit!";
    }
    
    if (!database.commit()) {
        setLastError(database.lastError(), "Failed to commit transaction");
        return false;
    } else {
        inTransaction = false;

        // Expire any temporary tables we were using
        expiredTableKeys = temporaryTableKeys;
        temporaryTableKeys.clear();
    }

    return true;
}

void QMailStorePrivate::rollback(void)
{
    if (!inTransaction) {
        qLog(Messaging) << "(" << ::getpid() << ")" << "Transaction does not exist at rollback!";
        qWarning() << "Transaction does not exist at rollback!";
    }
    
    inTransaction = false;

    if (!database.rollback()) {
        setLastError(database.lastError(), "Failed to rollback transaction");
    }
}

int QMailStorePrivate::lastErrorNumber() const
{
    return lastError;
}

void QMailStorePrivate::setLastError(const QSqlError &error, const QString &description, const QString &statement)
{
    QString s;
    QTextStream ts(&s);

    lastError = error.number();

    ts << qPrintable(description) << "; error:\"" << error.text() << '"';
    if (!statement.isEmpty())
        ts << "; statement:\"" << statement.simplified() << '"';

    qLog(Messaging) << "(" << ::getpid() << ")" << qPrintable(s);
    qWarning() << qPrintable(s);
}

void QMailStorePrivate::clearLastError(void) 
{
    lastError = 0;
}

template<bool PtrSizeExceedsLongSize>
QString numericPtrValue(const void *ptr)
{
    return QString::number(reinterpret_cast<unsigned long long>(ptr), 16).rightJustified(16, '0');
}

template<>
QString numericPtrValue<false>(const void *ptr)
{
    return QString::number(reinterpret_cast<unsigned long>(ptr), 16).rightJustified(8, '0');;
}

QString QMailStorePrivate::temporaryTableName(const QMailMessageKey& key)
{
    const QMailMessageKey *ptr = &key;
    return QString("qtopiamail_idmatch_%1").arg(numericPtrValue<(sizeof(void*) > sizeof(unsigned long))>(ptr));
}

void QMailStorePrivate::createTemporaryTable(const QMailMessageKey& key) const
{
    requiredTableKeys.append(&key);
}

void QMailStorePrivate::destroyTemporaryTables()
{
    while (!expiredTableKeys.isEmpty()) {
        const QMailMessageKey *key = expiredTableKeys.takeFirst();
        QString tableName = temporaryTableName(*key);

        QSqlQuery tableQuery(database);
        if (!tableQuery.exec(QString("DROP TABLE %1").arg(tableName))) {
            QString sql = tableQuery.lastQuery().simplified();
            QString err = tableQuery.lastError().text();

            qLog(Messaging) << "(" << ::getpid() << ")" << "Failed to drop temporary table - query:" << qPrintable(sql) << "; error:" << qPrintable(err);
            qWarning() << "Failed to drop temporary table - query:" << qPrintable(sql) << "; error:" << qPrintable(err);
        }
    }
}

bool QMailStorePrivate::idValueExists(quint64 id, const QString& table)
{
    QSqlQuery query(database);
    QString sql = "SELECT id FROM " + table + " WHERE id=?";
    if(!query.prepare(sql)) {
        setLastError(query.lastError(), "Failed to prepare idExists query", query.lastQuery());
        return false;
    }

    query.addBindValue(id);

    if(!query.exec()) {
        setLastError(query.lastError(), "Failed to execute idExists query", query.lastQuery());
        return false;
    }

    return (query.first());
}

bool QMailStorePrivate::idExists(const QMailAccountId& id, const QString& table)
{
    return idValueExists(id.toULongLong(), (table.isEmpty() ? "mailaccounts" : table));
}

bool QMailStorePrivate::idExists(const QMailFolderId& id, const QString& table)
{
    return idValueExists(id.toULongLong(), (table.isEmpty() ? "mailfolders" : table));
}

bool QMailStorePrivate::idExists(const QMailMessageId& id, const QString& table)
{
    return idValueExists(id.toULongLong(), (table.isEmpty() ? "mailmessages" : table));
}

void QMailStorePrivate::messageMetaData(const QSqlRecord& r,
                                        QMailMessageKey::Properties recordProperties,
                                        const QMailMessageKey::Properties& properties,
                                        QMailMessageMetaData* metaData) const
{
    // Record whether we have loaded all data for this message
    bool unloadedProperties = (properties != allMessageProperties());
    if (r.indexOf("mailfile") == -1) {
        // This record does not tell us whether there is a body or not...
        unloadedProperties = true;
    } else {
        QString mailfile = r.value("mailfile").toString();
        if (!mailfile.isEmpty()) {
            unloadedProperties = true;
        }
    }

    // Use wrapper to extract data items
    const MessageRecord messageRecord(r, recordProperties);

    foreach (QMailMessageKey::Property p, messagePropertyList())
    {
        switch(properties & p)
        {
            case QMailMessageKey::Id:
                metaData->setId(messageRecord.id());
                break;

            case QMailMessageKey::Type:
                metaData->setMessageType(messageRecord.messageType());
                break;

            case QMailMessageKey::ParentFolderId:
                metaData->setParentFolderId(messageRecord.parentFolderId());
                break;

            case QMailMessageKey::Sender:
                metaData->setFrom(messageRecord.from());
                break;

            case QMailMessageKey::Recipients:
                metaData->setTo(messageRecord.to());
                break;

            case QMailMessageKey::Subject:
                metaData->setSubject(messageRecord.subject());
                break;

            case QMailMessageKey::TimeStamp:
                metaData->setDate(messageRecord.date());
                break;

            case QMailMessageKey::Status:
                metaData->setStatus(messageRecord.status());
                break;

            case QMailMessageKey::ParentAccountId:
                metaData->setParentAccountId(messageRecord.parentAccountId());
                break;

            case QMailMessageKey::FromMailbox:
                metaData->setFromMailbox(messageRecord.fromMailbox());
                break;

            case QMailMessageKey::ServerUid:
                metaData->setServerUid(messageRecord.serverUid());
                break;

            case QMailMessageKey::Size:
                metaData->setSize(messageRecord.size());
                break;

            case QMailMessageKey::ContentType:
                metaData->setContent(messageRecord.content());
                break;

            case QMailMessageKey::PreviousParentFolderId:
                metaData->setPreviousParentFolderId(messageRecord.previousParentFolderId());
                break;
        }
    }
    
    if (unloadedProperties) {
        // This message is not completely loaded
        metaData->setStatus(QMailMessage::UnloadedData, true);
    }

    metaData->committed();
}

QMailMessageMetaData QMailStorePrivate::messageMetaData(const QSqlRecord& r, 
                                                        QMailMessageKey::Properties recordProperties,
                                                        const QMailMessageKey::Properties& properties) const
{
    QMailMessageMetaData metaData;

    messageMetaData(r, recordProperties, properties, &metaData);
    return metaData;
}

QMailMessage QMailStorePrivate::buildQMailMessage(const QSqlRecord& r,
                                                  const QMailMessageKey::Properties& properties) const
{
    QMailMessage newMessage;

    QString mailfile = r.value("mailfile").toString();
    if (!mailfile.isEmpty()) {
        if(!loadMessageBody(mailfile,&newMessage))
            qLog(Messaging) << "Could not load message body " << mailfile;
    }

    // Load the meta data items (note 'SELECT *' does not give the same result as 'SELECT expand(allMessageProperties())')
    messageMetaData(r, QMailMessageKey::Properties(0), properties, &newMessage);
    
    newMessage.committed();

    return newMessage;
}

QMailFolder QMailStorePrivate::buildQMailFolder(const QSqlRecord& r) const
{
    const FolderRecord record(r);

    QMailFolder result(record.name(), record.parentId(), record.parentAccountId());
    result.setId(record.id());
    result.setDisplayName(record.displayName());
    result.setStatus(record.status());

    return result;
}

QMailMessageRemovalRecord QMailStorePrivate::buildQMailMessageRemovalRecord(const QSqlRecord& r) const
{
    const MessageRemovalRecord record(r);

    QMailMessageRemovalRecord result(record.parentAccountId(), record.serverUid(), record.fromMailbox());
    return result;
}

QString QMailStorePrivate::buildOrderClause(const QMailAccountSortKey& key) const
{
    return ::buildOrderClause(key.d->arguments);
}

QString QMailStorePrivate::buildOrderClause(const QMailFolderSortKey& key) const
{
    return ::buildOrderClause(key.d->arguments);
}

QString QMailStorePrivate::buildOrderClause(const QMailMessageSortKey& key) const
{
    return ::buildOrderClause(key.d->arguments);
}

QString QMailStorePrivate::buildWhereClause(const QMailAccountKey& key) const
{
    return ::buildWhereClause<QMailAccountKeyPrivate>(key, key.d->combiner, key.d->arguments, key.d->subKeys, key.d->negated, *this);
}

bool QMailStorePrivate::containsProperty(const QMailMessageKey::Property& p,
                                         const QMailMessageKey& key) const
{
    foreach(const QMailMessageKeyPrivate::Argument &a, key.d->arguments)
        if(a.property == p)  
            return true;
       

    foreach(const QMailMessageKey &k, key.d->subKeys)
        if(containsProperty(p,k))
            return true;

    return false;
}

bool QMailStorePrivate::containsProperty(const QMailMessageSortKey::Property& p,
                                         const QMailMessageSortKey& key) const
{
    foreach(const QMailMessageSortKeyPrivate::Argument &a, key.d->arguments)
        if(a.first == p)  
            return true;

    return false;
}

QString QMailStorePrivate::buildWhereClause(const QMailMessageKey& key) const
{
    // See if we need to create any temporary tables to use in this query
    foreach (const QMailMessageKeyPrivate::Argument &a, key.d->arguments) {
        if (a.property == QMailMessageKey::Id && a.valueList.count() >= IdLookupThreshold) {
            createTemporaryTable(key);
        }
    }

    return ::buildWhereClause<QMailMessageKeyPrivate>(key, key.d->combiner, key.d->arguments, key.d->subKeys, key.d->negated, *this);
}

QVariantList QMailStorePrivate::whereClauseValues(const QMailAccountKey& key)
{
    QVariantList values;

    foreach (const QMailAccountKeyPrivate::Argument& a, key.d->arguments)
    {
        const AccountKeyArgumentExtractor extractor(a);

        switch(a.property)
        {
            case QMailAccountKey::Id:
                values += extractor.id();
                break;

            case QMailAccountKey::Name:
                values.append(extractor.name());
                break;

            case QMailAccountKey::MessageType:
                // Currently, we have a table of account types - convert message type in query to an account type comparison
                foreach (int type, QMailAccount::matchingAccountTypes(QMailMessage::MessageType(extractor.messageType().toInt())))
                    values.append(QVariant(type));
                break;

            case QMailAccountKey::EmailAddress:
                values.append(extractor.emailAddress());
                break;
        }
    }

    foreach (const QMailAccountKey& subkey, key.d->subKeys)
        values += whereClauseValues(subkey);

    return values;
}

void QMailStorePrivate::bindWhereData(const QMailAccountKey& key, QSqlQuery& query) const
{
    foreach (const QVariant& value, whereClauseValues(key))
        query.addBindValue(value);
}

QVariantList QMailStorePrivate::whereClauseValues(const QMailMessageKey& key)
{
    QVariantList values;

    foreach (const QMailMessageKeyPrivate::Argument& a, key.d->arguments)
    {
        const MessageKeyArgumentExtractor extractor(a);

        switch(a.property)
        { 
            case QMailMessageKey::Id:
                if (a.valueList.count() < IdLookupThreshold) {
                    values += extractor.id();
                } else {
                    // This value match has been replaced by a table lookup
                }
                break;

            case QMailMessageKey::Type:
                values.append(extractor.messageType());
                break;

            case QMailMessageKey::ParentFolderId:
                values += extractor.parentFolderId();
                break;

            case QMailMessageKey::AncestorFolderIds:
                values.append(extractor.ancestorFolderIds());
                break;

            case QMailMessageKey::Sender:
                values.append(extractor.sender());
                break;

            case QMailMessageKey::Recipients:
                values.append(extractor.recipients());
                break;    

            case QMailMessageKey::Subject:
                values.append(extractor.subject());
                break;    

            case QMailMessageKey::TimeStamp:
                values.append(extractor.date());
                break;    

            case QMailMessageKey::Status:
                values.append(extractor.status());
                break;    

            case QMailMessageKey::ParentAccountId:
                values += extractor.parentAccountId();
                break;

            case QMailMessageKey::FromMailbox:
                values.append(extractor.fromMailbox());
                break;    

            case QMailMessageKey::ServerUid:
                values.append(extractor.serverUid());
                break;   

            case QMailMessageKey::Size:
                values.append(extractor.size());
                break;     

            case QMailMessageKey::ContentType:
                values.append(extractor.content());
                break;     

            case QMailMessageKey::PreviousParentFolderId:
                values += extractor.previousParentFolderId();
                break;
        }
    }

    //subkeys

    foreach (const QMailMessageKey& subkey, key.d->subKeys)
        values += whereClauseValues(subkey);

    return values;
}

void QMailStorePrivate::bindWhereData(const QMailMessageKey& key, QSqlQuery& query) const
{
    foreach (const QVariant& value, whereClauseValues(key))
        query.addBindValue(value);
}

QString QMailStorePrivate::buildWhereClause(const QMailFolderKey& key) const
{
    return ::buildWhereClause<QMailFolderKeyPrivate>(key, key.d->combiner, key.d->arguments, key.d->subKeys, key.d->negated, *this);
}

QVariantList QMailStorePrivate::whereClauseValues(const QMailFolderKey& key)
{
    QVariantList values;

    foreach(const QMailFolderKeyPrivate::Argument& a, key.d->arguments)
    {
        FolderKeyArgumentExtractor extractor(a);

        switch(a.property)
        {
            case QMailFolderKey::Id:
                values += extractor.id();
                break;

            case QMailFolderKey::Name:
                values.append(extractor.name());
                break;

            case QMailFolderKey::ParentId:
                values += extractor.parentId();
                break;

            case QMailFolderKey::AncestorFolderIds:
                values.append(extractor.ancestorFolderIds());
                break;

            case QMailFolderKey::ParentAccountId:
                values += extractor.parentAccountId();
                break;

            case QMailFolderKey::DisplayName:
                values.append(extractor.displayName());
                break;

            case QMailFolderKey::Status:
                values += extractor.status();
                break;
        }
    }

    foreach (const QMailFolderKey& subkey, key.d->subKeys)
        values += whereClauseValues(subkey);

    return values;
}

void QMailStorePrivate::bindWhereData(const QMailFolderKey& key, QSqlQuery& query) const
{
    foreach (const QVariant& value, whereClauseValues(key))
        query.addBindValue(value);
}

// Class to convert QMailMessageMetaData properties to/from QVariant
class MessageValueExtractor
{
    const QMailMessageMetaData &data;
    
public:
    MessageValueExtractor(const QMailMessageMetaData &d) : data(d) {}

    // The types inserted into QVariant must match those extracted in the parallel operation!

    QVariant id() const { return data.id().toULongLong(); }
    static QMailMessageId id(const QVariant &var) { return QMailMessageId(QMailStorePrivate::extractValue<quint64>(var)); }

    QVariant messageType() const { return static_cast<int>(data.messageType()); }
    static QMailMessage::MessageType messageType(const QVariant &var) { return QMailMessage::MessageType(QMailStorePrivate::extractValue<int>(var)); }

    QVariant parentFolderId() const { return data.parentFolderId().toULongLong(); }
    static QMailFolderId parentFolderId(const QVariant &var) { return QMailFolderId(QMailStorePrivate::extractValue<quint64>(var)); }

    QVariant from() const { return data.from().toString(); }
    static QMailAddress from(const QVariant &var) { return QMailAddress(QMailStorePrivate::extractValue<QString>(var)); }

    QVariant to() const { return QMailAddress::toStringList(data.to()).join(","); }
    static QList<QMailAddress> to(const QVariant &var) { return QMailAddress::fromStringList(QMailStorePrivate::extractValue<QString>(var)); }

    QVariant subject() const { return data.subject(); }
    static QString subject(const QVariant &var) { return QMailStorePrivate::extractValue<QString>(var); }

    QVariant date() const { return data.date().toLocalTime(); }
    static QMailTimeStamp date(const QVariant &var) { return QMailTimeStamp(QMailStorePrivate::extractValue<QDateTime>(var)); }

    // Don't record the value of the UnloadedData flag:
    QVariant status() const { return (data.status() & ~QMailMessage::UnloadedData); }
    static quint64 status(const QVariant &var) { return QMailStorePrivate::extractValue<quint64>(var); }

    QVariant parentAccountId() const { return data.parentAccountId().toULongLong(); }
    static QMailAccountId parentAccountId(const QVariant &var) { return QMailAccountId(QMailStorePrivate::extractValue<quint64>(var)); }

    QVariant fromMailbox() const { return data.fromMailbox(); }
    static QString fromMailbox(const QVariant &var) { return QMailStorePrivate::extractValue<QString>(var); }

    QVariant serverUid() const { return data.serverUid(); }
    static QString serverUid(const QVariant &var) { return QMailStorePrivate::extractValue<QString>(var); }

    QVariant size() const { return data.size(); }
    static int size(const QVariant &var) { return QMailStorePrivate::extractValue<int>(var); }

    QVariant content() const { return static_cast<int>(data.content()); }
    static QMailMessage::ContentType content(const QVariant &var) { return QMailMessage::ContentType(QMailStorePrivate::extractValue<int>(var)); }

    QVariant previousParentFolderId() const { return data.previousParentFolderId().toULongLong(); }
    static QMailFolderId previousParentFolderId(const QVariant &var) { return QMailFolderId(QMailStorePrivate::extractValue<quint64>(var)); }
};

QVariantList QMailStorePrivate::updateValues(const QMailMessageKey::Properties& properties, const QMailMessageMetaData& data)
{
    QVariantList values;

    const MessageValueExtractor extractor(data);

    foreach (const QMailMessageKey::Property& p, messagePropertyList())
    {
        switch(properties & p)
        {
            case QMailMessageKey::Id:
                values.append(extractor.id());
                break;
            case QMailMessageKey::Type:
                values.append(extractor.messageType());
                break;
            case QMailMessageKey::ParentFolderId:
                values.append(extractor.parentFolderId());
                break;
            case QMailMessageKey::Sender:
                values.append(extractor.from());
                break;
            case QMailMessageKey::Recipients:
                values.append(extractor.to());
                break;
            case QMailMessageKey::Subject:
                values.append(extractor.subject());
                break;
            case QMailMessageKey::TimeStamp:
                values.append(extractor.date());
                break;
            case QMailMessageKey::Status:
                values.append(extractor.status());
                break;
            case QMailMessageKey::ParentAccountId:
                values.append(extractor.parentAccountId());
                break;
            case QMailMessageKey::FromMailbox:
                values.append(extractor.fromMailbox());
                break;
            case QMailMessageKey::ServerUid:
                values.append(extractor.serverUid());
                break;
            case QMailMessageKey::Size:
                values.append(extractor.size());
                break;
            case QMailMessageKey::ContentType:
                values.append(extractor.content());
                break;
            case QMailMessageKey::PreviousParentFolderId:
                values.append(extractor.previousParentFolderId());
                break;
        }
    }

    return values;
}

void QMailStorePrivate::bindUpdateData(const QMailMessageKey::Properties& properties,
                                       const QMailMessageMetaData& data,
                                       QSqlQuery& query) const
{
    foreach (const QVariant& value, updateValues(properties, data))
        query.addBindValue(value);
}

void QMailStorePrivate::updateMessageValues(const QMailMessageKey::Properties& properties, const QVariantList& values, QMailMessageMetaData& metaData)
{
    QVariantList::const_iterator it = values.constBegin();

    foreach (const QMailMessageKey::Property& p, messagePropertyList())
    {
        const QVariant& value(*it);
        bool valueConsumed(true);

        switch(properties & p)
        {
            case QMailMessageKey::Id:
                metaData.setId(MessageValueExtractor::id(value));
                break;

            case QMailMessageKey::Type:
                metaData.setMessageType(MessageValueExtractor::messageType(value));
                break;

            case QMailMessageKey::ParentFolderId:
                metaData.setParentFolderId(MessageValueExtractor::parentFolderId(value));
                break;

            case QMailMessageKey::Sender:
                metaData.setFrom(MessageValueExtractor::from(value));
                break;

            case QMailMessageKey::Recipients:
                metaData.setTo(MessageValueExtractor::to(value));
                break;

            case QMailMessageKey::Subject:
                metaData.setSubject(MessageValueExtractor::subject(value));
                break;

            case QMailMessageKey::TimeStamp:
                metaData.setDate(MessageValueExtractor::date(value));
                break;

            case QMailMessageKey::Status:
                metaData.setStatus(MessageValueExtractor::status(value));
                break;

            case QMailMessageKey::ParentAccountId:
                metaData.setParentAccountId(MessageValueExtractor::parentAccountId(value));
                break;

            case QMailMessageKey::FromMailbox:
                metaData.setFromMailbox(MessageValueExtractor::fromMailbox(value));
                break;

            case QMailMessageKey::ServerUid:
                metaData.setServerUid(MessageValueExtractor::serverUid(value));
                break;

            case QMailMessageKey::Size:
                metaData.setSize(MessageValueExtractor::size(value));
                break;

            case QMailMessageKey::ContentType:
                metaData.setContent(MessageValueExtractor::content(value));
                break;

            case QMailMessageKey::PreviousParentFolderId:
                metaData.setPreviousParentFolderId(MessageValueExtractor::previousParentFolderId(value));
                break;

            default:
                valueConsumed = false;
                break;
        }

        if (valueConsumed)
            ++it;
    }

    if (it != values.constEnd())
        qWarning() << QString("updateMessageValues: %1 values not consumed!").arg(values.constEnd() - it);

    // The target message is not completely loaded
    metaData.setStatus(QMailMessage::UnloadedData, true);
}

void QMailStorePrivate::initStore()
{
    {
        MutexGuard sMutex(databaseMutex());
        if (!sMutex.lock(1000)) {
            qLog(Messaging) << "Unable to acquire database mutex in initStore!";
            return;
        } 

        if (database.isOpenError()) {
            qLog(Messaging) << "Unable to open database in initStore!";
            return;
        }

        if (!ensureVersionInfo() ||
            !setupTables(QList<TableInfo>() << tableInfo("mailaccounts", 100)
                                            << tableInfo("mailfolders", 101)
                                            << tableInfo("mailfolderlinks", 100)
                                            << tableInfo("mailmessages", 100)
                                            << tableInfo("mailstatusflags", 100)
                                            << tableInfo("deletedmessages", 100)) ||
            !setupFolders(QList<FolderInfo>() << folderInfo(QMailFolder::InboxFolder, tr("Inbox"))
                                                << folderInfo(QMailFolder::OutboxFolder, tr("Outbox"))
                                                << folderInfo(QMailFolder::DraftsFolder, tr("Drafts"))
                                                << folderInfo(QMailFolder::SentFolder, tr("Sent"))
                                                << folderInfo(QMailFolder::TrashFolder, tr("Trash")))) {
            return;
        }
    }

    {
        MutexGuard settingsMutex(accountSettingsFileMutex());
        if (!settingsMutex.lock(1000)) {
            qLog(Messaging) << "Unable to acquire settings file mutex in initStore!";
            return;
        } 

        QSettings accountSettings(accountSettingsPrefix(), accountSettingsFileName());

        accountSettings.beginGroup(globalAccountSettingsKey());
        accountSettings.setValue("version",accountSettingsFileVersion);
        accountSettings.endGroup();
    }

    // We are now correctly initialized
    init = true;

    // Some classes have initialization code dependent on the store:
    QMailFolder::initStore();
    QMailMessage::initStore();

#if defined(Q_USE_SQLITE)
    // default sqlite cache_size of 2000*1.5KB is too large, as we only want
    // to cache 100 metadata records 
    QSqlQuery query( database );
    query.exec(QLatin1String("PRAGMA cache_size=50"));
#endif
}

bool QMailStorePrivate::initialized()
{
    return init;
}

bool QMailStorePrivate::ensureVersionInfo()
{
    if (!database.tables().contains("versioninfo", Qt::CaseInsensitive)) {
        // Use the same version scheme as dbmigrate, in case we need to cooperate later
        QString sql("CREATE TABLE versioninfo ("
                    "   tableName NVARCHAR (255) NOT NULL,"
                    "   versionNum INTEGER NOT NULL,"
                    "   lastUpdated NVARCHAR(20) NOT NULL,"
                    "   PRIMARY KEY(tableName, versionNum))");

        QSqlQuery query(database);
        if (!query.exec(sql)) {
            qLog(Messaging) << "Failed to create versioninfo table - query:" << sql << "- error:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

qint64 QMailStorePrivate::tableVersion(const QString &name) const
{
    QString sql("SELECT COALESCE(MAX(versionNum), 0) FROM versioninfo WHERE tableName=?");

    QSqlQuery query(database);
    query.prepare(sql);
    query.addBindValue(name);
    if (query.exec() && query.first())
        return query.value(0).value<qint64>();

    qLog(Messaging) << "Failed to query versioninfo - query:" << sql << "- error:" << query.lastError().text();
    return 0;
}

bool QMailStorePrivate::setTableVersion(const QString &name, qint64 version)
{
    QString sql("DELETE FROM versioninfo WHERE tableName=? AND versionNum=?");

    // Delete any existing entry for this table
    QSqlQuery query(database);
    query.prepare(sql);
    query.addBindValue(name);
    query.addBindValue(version);

    if (!query.exec()) {
        qLog(Messaging) << "Failed to delete versioninfo - query:" << sql << "- error:" << query.lastError().text();
        return false;
    } else {
        sql = "INSERT INTO versioninfo (tablename,versionNum,lastUpdated) VALUES (?,?,?)";

        // Insert the updated info
        query = QSqlQuery(database);
        query.prepare(sql);
        query.addBindValue(name);
        query.addBindValue(version);
        query.addBindValue(QDateTime::currentDateTime().toString());

        if (!query.exec()) {
            qLog(Messaging) << "Failed to insert versioninfo - query:" << sql << "- error:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool QMailStorePrivate::createTable(const QString &name)
{
    bool result = true;

    // load schema.
    QFile data(QLatin1String(":/QtopiaSql/") + database.driverName() + QLatin1String("/") + name);
    if (!data.open(QIODevice::ReadOnly)) {
        qLog(Messaging) << "Failed to load table resource:" << name;
        result = false;
    } else {
        QTextStream ts(&data);
        // read assuming utf8 encoding.
        ts.setCodec(QTextCodec::codecForName("utf8"));
        ts.setAutoDetectUnicode(true);
        
        QString sql = parseSql(ts);
        while (!sql.isEmpty()) {
            QSqlQuery query(database);
            if (!query.exec(sql)) {
                qLog(Messaging) << "Failed to exec table creation SQL query:" << sql << "- error:" << query.lastError().text();
                result = false;
            }
            sql = parseSql(ts);
        }
    }

    return result;
}

bool QMailStorePrivate::setupTables(const QList<TableInfo> &tableList)
{
    bool result = true;

    QStringList tables = database.tables();

    foreach (const TableInfo &table, tableList) {
        const QString &tableName(table.first);
        qint64 version(table.second);

        if (!tables.contains(tableName, Qt::CaseInsensitive)) {
            // Create the table
            result &= (createTable(tableName) && setTableVersion(tableName, version));
        } else {
            // Ensure the table does not have an incompatible version
            qint64 dbVersion = tableVersion(tableName);
            if (dbVersion == 0) {
                qWarning() << "No version for existing table:" << tableName;

                // No existing version info - set the current version
                // TODO: Post 4.4.1, it will be invalid to have no version for an extant table
                result &= setTableVersion(tableName, version);
            } else if (dbVersion != version) {
                qWarning() << "Incompatible version for table:" << tableName << "- existing" << dbVersion << "!=" << version;
                result = false;
            }
        }
    }
        
    return result;
}

bool QMailStorePrivate::setupFolders(const QList<FolderInfo> &folderList)
{
    QSqlQuery infoQuery(simpleQuery("SELECT id FROM mailfolders", 
                                    "folder ids query"));
    if (infoQuery.lastError().type() != QSqlError::NoError)
        return false;

    QSet<quint64> folderIds;
    while (infoQuery.next())
        folderIds.insert(infoQuery.value(0).toULongLong());

    foreach (const FolderInfo &folder, folderList) {
        if (folderIds.contains(folder.first))
            continue;

        QSqlQuery query(simpleQuery("INSERT INTO mailfolders (id,name,parentid,parentaccountid,displayname,status) VALUES (?,?,?,?,?,?)",
                                    QVariantList() << folder.first
                                                   << folder.second
                                                   << quint64(0)
                                                   << quint64(0)
                                                   << QString()
                                                   << quint64(0),
                                    "setupFolders insert query"));
        if (query.lastError().type() != QSqlError::NoError)
            return false;
    }

    return true;
}

QString QMailStorePrivate::parseSql(QTextStream& ts)
{
    QString qry = "";
    while(!ts.atEnd())
    {
        QString line = ts.readLine();
        // comment, remove.
        if (line.contains(QLatin1String("--")))
            line.truncate (line.indexOf (QLatin1String("--")));
        if (line.trimmed ().length () == 0)
            continue;
        qry += line;
        
        if ( line.contains( ';' ) == false) 
            qry += QLatin1String(" ");
        else
            return qry;
    }
    return qry;
}

QString QMailStorePrivate::expandValueList(const QVariantList& valueList)
{
    Q_ASSERT(!valueList.isEmpty());
    return expandValueList(valueList.count());
}

QString QMailStorePrivate::expandValueList(int valueCount)
{
    Q_ASSERT(valueCount > 0);

    if (valueCount == 1) {
        return "(?)";
    } else {
        QString inList = " (?";
        for (int i = 1; i < valueCount; ++i)
            inList += ",?";
        inList += ")";
        return inList;
    }
}

QString QMailStorePrivate::expandProperties(const QMailMessageKey::Properties& p, bool update) const 
{
    QString out;

    //if key contains FromAccount then we will be issuing a join, so 
    //prefix with table name
    
    const MessagePropertyMap& map = messagePropertyMap();

    foreach (QMailMessageKey::Property prop, messagePropertyList()) {
        if(p & prop)
        {
            if(!out.isEmpty())
                out += ",";                     
            out += map.value(prop);
            if(update)
                out += "=?";
        }
    }

    return out;
}

int QMailStorePrivate::numComparitors(const QMailAccountKey& key) const
{
    int total = 0;
    foreach(const QMailAccountKeyPrivate::Argument &a, key.d->arguments)
        total += a.valueList.count();
    foreach(const QMailAccountKey &k, key.d->subKeys)
        total += numComparitors(k);
    return total;
}

int QMailStorePrivate::numComparitors(const QMailMessageKey& key) const
{
    int total = 0;
    foreach(const QMailMessageKeyPrivate::Argument &a, key.d->arguments)
        total += a.valueList.count();
    foreach(const QMailMessageKey &k, key.d->subKeys)
        total += numComparitors(k);
    return total;
}

int QMailStorePrivate::numComparitors(const QMailFolderKey& key) const
{
    int total = 0;
    foreach(const QMailFolderKeyPrivate::Argument &a, key.d->arguments)
        total += a.valueList.count();
    foreach(const QMailFolderKey &k, key.d->subKeys)
        total += numComparitors(k);
    return total;

}

void QMailStorePrivate::checkComparitors(const QMailAccountKey& key) const
{
    if(numComparitors(key) > maxComparitorsCutoff) 
        qLog(Messaging) << comparitorWarning(); 
}

void QMailStorePrivate::checkComparitors(const QMailMessageKey& key) const
{
    if(numComparitors(key) > maxComparitorsCutoff) 
        qLog(Messaging) << comparitorWarning(); 
}

void QMailStorePrivate::checkComparitors(const QMailFolderKey& key) const
{
    if(numComparitors(key) > maxComparitorsCutoff) 
        qLog(Messaging) << comparitorWarning();

}

template<typename IDListType>
void emitIpcUpdates(const IDListType& ids, const QString& sig, int max = QMailStorePrivate::maxNotifySegmentSize)
{
    if (!sig.isEmpty()) {
        if (max > 0) {
            SegmentList segmentList = createSegments(ids.count(), max);
            foreach (const Segment& segment, segmentList) {
                IDListType idSegment = ids.mid(segment.first, (segment.second - segment.first));

                QtopiaIpcEnvelope e("QPE/Qtopiamail",sig); 
                e << ::getpid();
                e << idSegment; 
            }
        } else {
            QtopiaIpcEnvelope e("QPE/Qtopiamail",sig); 
            e << ::getpid();
            e << ids; 
        }
    } else {
        qWarning() << "No signature for IPC updates!";
    }
}

typedef QMap<QMailStorePrivate::ChangeType, QString> NotifyFunctionMap;

static NotifyFunctionMap initAccountFunctions()
{
    NotifyFunctionMap sig;
    sig[QMailStorePrivate::Added] = QMailStorePrivate::accountAddedSig();
    sig[QMailStorePrivate::Updated] = QMailStorePrivate::accountUpdatedSig();
    sig[QMailStorePrivate::Removed] = QMailStorePrivate::accountRemovedSig();
    sig[QMailStorePrivate::ContentsModified] = QMailStorePrivate::accountContentsModifiedSig();
    return sig;
}

static NotifyFunctionMap initMessageRemovalRecordFunctions()
{
    NotifyFunctionMap sig;
    sig[QMailStorePrivate::Added] = QMailStorePrivate::messageRemovalRecordsAddedSig();
    sig[QMailStorePrivate::Removed] = QMailStorePrivate::messageRemovalRecordsRemovedSig();
    return sig;
}

void QMailStorePrivate::notifyAccountsChange(const ChangeType& changeType,
                                             const QMailAccountIdList& ids)
{
    static NotifyFunctionMap sig(initAccountFunctions());
    if (preFlushTimer.isActive()) {
        QSet<QMailAccountId> idsSet = QSet<QMailAccountId>::fromList(ids);
        switch (changeType)
        {
        case Added:
            addAccountsBuffer += idsSet;
            break;
        case Removed:
            removeAccountsBuffer += idsSet;
            break;
        case Updated:
            updateAccountsBuffer += idsSet;
            break;
        case ContentsModified:
            accountContentsModifiedBuffer += idsSet;
            break;
        default:
            qLog(Messaging) << "Unhandled account notification";
            break;
        }
        if (!flushTimer.isActive())
            flushTimer.start(1000);
        preFlushTimer.start(1000);
        return;
    }
    preFlushTimer.start(1000);
    
    notifyFlush();
    emitIpcUpdates(ids, sig[changeType]);
}

static NotifyFunctionMap initMessageFunctions()
{
    NotifyFunctionMap sig;
    sig[QMailStorePrivate::Added] = QMailStorePrivate::messageAddedSig();
    sig[QMailStorePrivate::Updated] = QMailStorePrivate::messageUpdatedSig();
    sig[QMailStorePrivate::Removed] = QMailStorePrivate::messageRemovedSig();
    return sig;
}

void QMailStorePrivate::notifyMessagesChange(const ChangeType& changeType,
                                             const QMailMessageIdList& ids)
{
    static NotifyFunctionMap sig(initMessageFunctions());
    // Use the preFlushTimer to activate buffering when multiple messages 
    // arrive within one second of each other.
    if (preFlushTimer.isActive()) {
        QSet<QMailMessageId> idsSet = QSet<QMailMessageId>::fromList(ids);
        switch (changeType)
        {
        case Added:
            addMessagesBuffer += idsSet;
            break;
        case Removed:
            removeMessagesBuffer += idsSet;
            break;
        case Updated:
            updateMessagesBuffer += idsSet;
            break;
        default:
            qLog(Messaging) << "Unhandled message (content modified?) "
                "notification received";
            break;
        }
        
        if (!flushTimer.isActive())
            flushTimer.start(1000);
        preFlushTimer.start(1000);
        return;
    }
    preFlushTimer.start(1000);
    
    notifyFlush();
    emitIpcUpdates(ids, sig[changeType]);
}

static NotifyFunctionMap initFolderFunctions()
{
    NotifyFunctionMap sig;
    sig[QMailStorePrivate::Added] = QMailStorePrivate::folderAddedSig();
    sig[QMailStorePrivate::Updated] = QMailStorePrivate::folderUpdatedSig();
    sig[QMailStorePrivate::Removed] = QMailStorePrivate::folderRemovedSig();
    sig[QMailStorePrivate::ContentsModified] = QMailStorePrivate::folderContentsModifiedSig();
    return sig;
}

void QMailStorePrivate::notifyFoldersChange(const ChangeType& changeType,
                                            const QMailFolderIdList& ids)
{
    static NotifyFunctionMap sig(initFolderFunctions());
    if (preFlushTimer.isActive()) {
        QSet<QMailFolderId> idsSet = QSet<QMailFolderId>::fromList(ids);
        switch (changeType)
        {
        case Added:
            addFoldersBuffer += idsSet;
            break;
        case Removed:
            removeFoldersBuffer += idsSet;
            break;
        case Updated:
            updateFoldersBuffer += idsSet;
            break;
        case ContentsModified:
            folderContentsModifiedBuffer += idsSet;
            break;
        default:
            qLog(Messaging) << "Unhandled folder notification received";
            break;
        }

        if (!flushTimer.isActive())
            flushTimer.start(1000);
        preFlushTimer.start(1000);
        return;
    }
    preFlushTimer.start(1000);

    notifyFlush();
    emitIpcUpdates(ids, sig[changeType]);
}

void QMailStorePrivate::notifyMessageRemovalRecordsChange(const ChangeType& changeType,
                                                          const QMailAccountIdList& ids)
{
    static NotifyFunctionMap sig(initMessageRemovalRecordFunctions());
    if (preFlushTimer.isActive()) {
        QSet<QMailAccountId> idsSet = QSet<QMailAccountId>::fromList(ids);
        switch (changeType)
        {
        case Added:
            addMessageRemovalRecordsBuffer += idsSet;
            break;
        case Removed:
            removeMessageRemovalRecordsBuffer += idsSet;
            break;
        default:
            qLog(Messaging) << "Unhandled message removal record "
                "notification received";
            break;
        }

        if (!flushTimer.isActive())
            flushTimer.start(1000);
        preFlushTimer.start(1000);
        return;
    }
    preFlushTimer.start(1000);
    
    notifyFlush();
    emitIpcUpdates(ids, sig[changeType]);
}

QMailStorePrivate::AccountUpdateSignalMap QMailStorePrivate::initAccountUpdateSignals()
{
    AccountUpdateSignalMap sig;
    sig[QMailStorePrivate::accountAddedSig()] = &QMailStore::accountsAdded;
    sig[QMailStorePrivate::accountUpdatedSig()] = &QMailStore::accountsUpdated;
    sig[QMailStorePrivate::accountRemovedSig()] = &QMailStore::accountsRemoved;
    sig[QMailStorePrivate::accountContentsModifiedSig()] = &QMailStore::accountContentsModified;
    sig[QMailStorePrivate::messageRemovalRecordsAddedSig()] = &QMailStore::messageRemovalRecordsAdded;
    sig[QMailStorePrivate::messageRemovalRecordsRemovedSig()] = &QMailStore::messageRemovalRecordsRemoved;
    return sig;
}

QMailStorePrivate::FolderUpdateSignalMap QMailStorePrivate::initFolderUpdateSignals()
{
    FolderUpdateSignalMap sig;
    sig[QMailStorePrivate::folderAddedSig()] = &QMailStore::foldersAdded;
    sig[QMailStorePrivate::folderUpdatedSig()] = &QMailStore::foldersUpdated;
    sig[QMailStorePrivate::folderRemovedSig()] = &QMailStore::foldersRemoved;
    sig[QMailStorePrivate::folderContentsModifiedSig()] = &QMailStore::folderContentsModified;
    return sig;
}

QMailStorePrivate::MessageUpdateSignalMap QMailStorePrivate::initMessageUpdateSignals()
{
    MessageUpdateSignalMap sig;
    sig[QMailStorePrivate::messageAddedSig()] = &QMailStore::messagesAdded;
    sig[QMailStorePrivate::messageUpdatedSig()] = &QMailStore::messagesUpdated;
    sig[QMailStorePrivate::messageRemovedSig()] = &QMailStore::messagesRemoved;
    return sig;
}

void QMailStorePrivate::notifyFlush()
{
    //IPC Condensation
    static NotifyFunctionMap sigAccount(initAccountFunctions());
    static NotifyFunctionMap sigFolder(initFolderFunctions());
    static NotifyFunctionMap sigMessage(initMessageFunctions());
    static NotifyFunctionMap sigRemoval(initMessageRemovalRecordFunctions());
 
    if (!addAccountsBuffer.isEmpty())
        emitIpcUpdates(addAccountsBuffer.toList(),
                       sigAccount[Added]);
    if (!addFoldersBuffer.isEmpty())
        emitIpcUpdates(addFoldersBuffer.toList(),
                       sigFolder[Added]);
    if (!addMessagesBuffer.isEmpty())
        emitIpcUpdates(addMessagesBuffer.toList(),
                       sigMessage[Added]);
    if (!addMessageRemovalRecordsBuffer.isEmpty())
        emitIpcUpdates(addMessageRemovalRecordsBuffer.toList(),
                       sigRemoval[Added]);

    if (!updateMessagesBuffer.isEmpty())
        emitIpcUpdates(updateMessagesBuffer.toList(),
                       sigMessage[Updated]);
    if (!updateFoldersBuffer.isEmpty())
        emitIpcUpdates(updateFoldersBuffer.toList(),
                       sigFolder[Updated]);
    if (!updateAccountsBuffer.isEmpty())
        emitIpcUpdates(updateAccountsBuffer.toList(),
                       sigAccount[Updated]);
    
    if (!removeMessageRemovalRecordsBuffer.isEmpty())
        emitIpcUpdates(removeMessageRemovalRecordsBuffer.toList(), 
                       sigRemoval[Removed]);
    if (!removeMessagesBuffer.isEmpty())
        emitIpcUpdates(removeMessagesBuffer.toList(),
                       sigMessage[Removed]);
    if (!removeFoldersBuffer.isEmpty())
        emitIpcUpdates(removeFoldersBuffer.toList(),
                       sigFolder[Removed]);
    if (!removeAccountsBuffer.isEmpty())
        emitIpcUpdates(removeAccountsBuffer.toList(),
                       sigAccount[Removed]);

    folderContentsModifiedBuffer -= removeFoldersBuffer;
    accountContentsModifiedBuffer -= removeAccountsBuffer;
    if (!folderContentsModifiedBuffer.isEmpty())
        emitIpcUpdates(folderContentsModifiedBuffer.toList(),
                       sigFolder[ContentsModified]);
    if (!accountContentsModifiedBuffer.isEmpty())
        emitIpcUpdates(accountContentsModifiedBuffer.toList(),
                       sigAccount[ContentsModified]);
    
    addAccountsBuffer.clear();
    addFoldersBuffer.clear();
    addMessagesBuffer.clear();
    addMessageRemovalRecordsBuffer.clear();
    updateMessagesBuffer.clear();
    updateFoldersBuffer.clear();
    updateAccountsBuffer.clear();
    removeMessageRemovalRecordsBuffer.clear();
    removeMessagesBuffer.clear();
    removeFoldersBuffer.clear();
    removeAccountsBuffer.clear();
    folderContentsModifiedBuffer.clear();
    accountContentsModifiedBuffer.clear();
}

void QMailStorePrivate::flushIpcNotifications()
{
    // We need to emit all pending IPC notifications
    notifyFlush();

    // Tell the recipients to process the notifications synchronously
    QtopiaIpcEnvelope e("QPE/Qtopiamail", "forceIpcFlush"); 
    e << ::getpid();
}

bool QMailStorePrivate::emitIpcNotification()
{
    if (messageQueue.isEmpty())
        return false;
    
    const QPair<QString, QByteArray> &notification = messageQueue.first();
    const QString &message = notification.first;
    const QByteArray &data = notification.second;

    QDataStream ds(data);

    int pid;
    ds >> pid;

    static AccountUpdateSignalMap accountUpdateSignals(initAccountUpdateSignals());
    static FolderUpdateSignalMap folderUpdateSignals(initFolderUpdateSignals());
    static MessageUpdateSignalMap messageUpdateSignals(initMessageUpdateSignals());

    AccountUpdateSignalMap::const_iterator ait;
    FolderUpdateSignalMap::const_iterator fit;
    MessageUpdateSignalMap::const_iterator mit;

    if ((ait = accountUpdateSignals.find(message)) != accountUpdateSignals.end()) {
        QMailAccountIdList ids;
        ds >> ids;

        void (QMailStore::*sig)(const QMailAccountIdList&) = ait.value();
        if ((sig == &QMailStore::accountsUpdated) || (sig == &QMailStore::accountsRemoved)) {
            foreach (const QMailAccountId &id, ids)
                accountCache.remove(id);
        }

        asyncEmission = true;
        emit (q->*sig)(ids);
        asyncEmission = false;
    } else if ((fit = folderUpdateSignals.find(message)) != folderUpdateSignals.end()) {
        QMailFolderIdList ids;
        ds >> ids;

        void (QMailStore::*sig)(const QMailFolderIdList&) = fit.value();
        if ((sig == &QMailStore::foldersUpdated) || (sig == &QMailStore::foldersRemoved)) {
            foreach (const QMailFolderId &id, ids)
                folderCache.remove(id);
        }

        asyncEmission = true;
        emit (q->*sig)(ids);
        asyncEmission = false;
    } else if ((mit = messageUpdateSignals.find(message)) != messageUpdateSignals.end()) {
        QMailMessageIdList ids;
        ds >> ids;

        void (QMailStore::*sig)(const QMailMessageIdList&) = mit.value();
        if ((sig == &QMailStore::messagesUpdated) || (sig == &QMailStore::messagesRemoved)) {
            foreach (const QMailMessageId &id, ids)
                headerCache.remove(id);
        }

        asyncEmission = true;
        emit (q->*sig)(ids);
        asyncEmission = false;
    } else {
        qWarning() << "No update signal for message:" << message;
    }
    
    messageQueue.removeFirst();
    return !messageQueue.isEmpty();
}

void QMailStorePrivate::processIpcMessageQueue()
{
    if (messageQueue.isEmpty())
        return;
    
    if (emitIpcNotification())
        queueTimer.start(0);
}

void QMailStorePrivate::ipcMessage(const QString& message, const QByteArray& data) 
{
    QDataStream ds(data);

    int pid;
    ds >> pid;

    if(::getpid() == pid) //dont notify ourselves 
        return;

    if (message == "forceIpcFlush") {
        // We have been told to flush any pending ipc notifications
        queueTimer.stop();
        while (emitIpcNotification()) {}
    } else {
        messageQueue.append(qMakePair(message, data));
        queueTimer.start(0);
    }
}

QString QMailStorePrivate::generateUniqueFileName()
{
    //format: seconds_epoch.pid.randomchars

    bool exists = true;
    QString filename;
    
    while(exists)
    {
        qint64 pid = 0;
        pid = getpid();
        filename.sprintf("%ld.%ld.",(unsigned long)time(0),(long)pid);
        filename += randomString(5);
        //check if it exists
        exists = QFile::exists(messagesBodyPath() + "/" + filename);
    }

    return filename;
}

QString QMailStorePrivate::randomString(int length)
{
    if (length <=0 ) return QString();

    QString str;
    str.resize( length );

    int i = 0;
    while (length--){
        int r=qrand() % 62;
        r+=48;
        if (r>57) r+=7;
        if (r>90) r+=6;
        str[i++] =  char(r);
    }
    return str;
}

bool QMailStorePrivate::saveMessageBody(const QMailMessage& m, const QString& fileName)
{
    QString filePath = messageFilePath(fileName);

    MutexGuard lock(messageFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire message body mutex in saveMessageBody!";
        return false;
    } 

    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadWrite)) {
        qWarning("Could not open new mail file %s", qPrintable(fileName));
        return false;
    }

    QDataStream out(&file);
    m.toRfc2822(out, QMailMessage::StorageFormat); 
    if (out.status() != QDataStream::Ok)
    {
        qWarning("Could not save mail, removing temporary mail file..");

        //remove the mail file
        if(!QFile::remove(filePath))
            qWarning("Could not remove temporary mail file %s",qPrintable(fileName));
        return false;
    }

    return true;
}

bool QMailStorePrivate::removeMessageBody(const QString& fileName)
{
    //remove the mail file
    QString filePath = messageFilePath(fileName);

    MutexGuard lock(messageFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire message body mutex in removeMessageBody!";
        return false;
    } 

    bool result = false;
    if (QFile::exists(filePath))
        result = QFile::remove(filePath);

    return result;      
}

bool QMailStorePrivate::updateMessageBody(const QString& fileName, QMailMessage* data)
{
    QString filePath = messageFilePath(fileName);
    QString backupPath = filePath + ".tmp";

    MutexGuard lock(messageFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire message body mutex in updateMessageBody!";
        return false;
    } 

    if (QFile::exists(filePath)) {
        //backup the old file
        if(!QFile::rename(filePath,backupPath))
        {
            qWarning("Count not create temp backup file %s",qPrintable(backupPath));
            return false;
        }
    }

    QString detachedFile = data->headerField("X-qtopia-internal-filename").content();
    if (!detachedFile.isEmpty()) {
        data->removeHeaderField("X-qtopia-internal-filename");
        if (QFile::rename(detachedFile, filePath)) {
            //delete the temp mail file
            if(QFile::exists(backupPath))
                if(!QFile::remove(backupPath))
                    qWarning() << "Could not remove the backup file";

            return true;
        }

        qWarning("Could not rename mail..");
    } 
    
    //save the new file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite)) 
    {
        qWarning("Could not open new mail file %s", qPrintable(filePath));

        //rename the old file back
        if(QFile::exists(backupPath))
            if(!QFile::rename(backupPath,filePath))
                qWarning() << "Could not restore temp backup file %s",qPrintable(backupPath);

        return false;
    }

    QDataStream out(&file);
    data->toRfc2822(out, QMailMessage::StorageFormat); 
    if (out.status() != QDataStream::Ok)
    {
        qWarning("Could not save mail, removing temporary mail file..");

        //remove the mail file
        if(!QFile::remove(filePath))
            qWarning("Could not remove temporary mail file %s",qPrintable(filePath));

        //restore the backup file
        if(QFile::exists(backupPath))
            if(!QFile::rename(backupPath,filePath))
                qWarning() << "Could not restore temp backup file %s",qPrintable(backupPath);

        return false;
    }

    //delete the temp mail file
    if(QFile::exists(backupPath))
        if(!QFile::remove(backupPath))
            qWarning() << "Could not remove the backup file";

    return true;
}

bool QMailStorePrivate::loadMessageBody(const QString& fileName, QMailMessage* out) const
{
    QString filePath = messageFilePath(fileName);

    MutexGuard lock(messageFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire message body mutex in loadMessageBody!";
        return false;
    } 

    if(!QFile::exists( filePath ))
    {
        qWarning("Could not load mail file %s, does not exist",qPrintable(filePath));
        return false;
    }
    *out = QMailMessage::fromRfc2822File( filePath );

    static const QString smsTag("@sms");
    static const QString fromField("From");

    // Remove sms-origin tag, if present - the SMS client previously appended
    // "@sms" to the from address, which is no longer necesary
    QString sender(out->headerFieldText(fromField));
    if (sender.endsWith(smsTag)) {
        sender.chop(smsTag.length());
        out->setHeaderField(fromField, sender);
    }
    return true;
}

template<typename AccountType>
bool QMailStorePrivate::saveAccountSettings(const AccountType& account) 
{
    QString keyId = "account_" + QString::number(account.id().toULongLong());

    MutexGuard lock(accountSettingsFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire settings file mutex in saveAccountSettings!";
        return false;
    } 

    QSettings accountSettings(accountSettingsPrefix(), accountSettingsFileName());
    if (accountSettings.contains(keyId)) {
        qLog(Messaging) << "Settings file already contains account " << keyId; 
        return false;
    }

    accountSettings.beginGroup(keyId);
    account.saveSettings(&accountSettings);
    accountSettings.endGroup();
    
    return true;
}

template<typename AccountType>
bool QMailStorePrivate::updateAccountSettings(const AccountType& account, bool removeExisting)
{
    QString keyId = "account_" + QString::number(account.id().toULongLong());

    MutexGuard lock(accountSettingsFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire settings file mutex in updateAccountSettings!";
        return false;
    } 

    QSettings accountSettings(accountSettingsPrefix(), accountSettingsFileName());
    if (!accountSettings.childGroups().contains(keyId)) {
        qLog(Messaging) << "Account " << keyId << " does not exist in settings file";
        return true;
    }

    accountSettings.beginGroup(keyId);
    if (removeExisting)
        accountSettings.remove("");
    account.saveSettings(&accountSettings);
    accountSettings.endGroup();
    
    return true;
}

template<typename AccountType>
bool QMailStorePrivate::loadAccountSettings(AccountType* account)
{
    QString keyId = "account_" + QString::number(account->id().toULongLong());

    //lock the settings file
    MutexGuard lock(accountSettingsFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire settings file mutex in loadAccountSettings!";
        return false;
    } 

    QSettings accountSettings(accountSettingsPrefix(), accountSettingsFileName());
    if (!accountSettings.childGroups().contains(keyId)) {
        qLog(Messaging) << "Account " << keyId << " does not exist in settings file";
        return false;
    }

    accountSettings.beginGroup(keyId);
    account->readSettings(&accountSettings);
    accountSettings.endGroup();

    return true;
}

// Instantiate for the types we need to support:
template bool QMailStorePrivate::saveAccountSettings<QMailAccount>(const QMailAccount&);
template bool QMailStorePrivate::updateAccountSettings<QMailAccount>(const QMailAccount&, bool);
template bool QMailStorePrivate::loadAccountSettings<QMailAccount>(QMailAccount*);
template bool QMailStorePrivate::saveAccountSettings<AccountConfiguration>(const AccountConfiguration&);
template bool QMailStorePrivate::updateAccountSettings<AccountConfiguration>(const AccountConfiguration&, bool);
template bool QMailStorePrivate::loadAccountSettings<AccountConfiguration>(AccountConfiguration*);

bool QMailStorePrivate::removeAccountSettings(const QMailAccountId& accountId) 
{
    QString keyId = "account_" + QString::number(accountId.toULongLong());

    MutexGuard lock(accountSettingsFileMutex());
    if (!lock.lock(1000)) {
        qLog(Messaging) << "Unable to acquire settings file mutex in removeAccountSettings!";
        return false;
    } 

    QSettings accountSettings(accountSettingsPrefix(), accountSettingsFileName());
    if (!accountSettings.childGroups().contains(keyId)) {
        qLog(Messaging) << "Account " << keyId << " does not exist in settings file";
        return true;
    }

    accountSettings.beginGroup(keyId);
    accountSettings.remove("");
    accountSettings.endGroup();
    
    return true;
}

bool QMailStorePrivate::checkPreconditions(const QMailFolder& folder, bool update)
{
    //if the parent is valid, check that it exists 
    //if the account is valid, check that is exists 

    if(!update)
    {
        if(folder.id().isValid())
        {
            qLog(Messaging) << "Folder exists, use update instead of add.";
            return false;
        }
    }
    else 
    {
        if(!folder.id().isValid())
        {
            qLog(Messaging) << "Folder does not exist, use add instead of update.";
            return false;
        }

        if(folder.parentId().isValid() && folder.parentId() == folder.id())
        {
            qLog(Messaging) << "A folder cannot be a child to itself";
            return false;
        }
    }

    if(folder.parentId().isValid())
    {
        if(!idExists(folder.parentId(),"mailfolders"))
        {
            qLog(Messaging) << "Parent folder does not exist!";
            return false;
        }
    }

    if(folder.parentAccountId().isValid())
    {
        if(!idExists(folder.parentAccountId(),"mailaccounts"))
        {
            qLog(Messaging) << "Parent account does not exist!";
            return false;
        }
    }

    return true;
}

bool QMailStorePrivate::deleteMessages(const QMailMessageKey& key, 
                                       QMailStore::MessageRemovalOption option, 
                                       QMailMessageIdList& deletedMessages, 
                                       QStringList& expiredMailfiles, 
                                       QMailAccountIdList& modifiedAccounts, 
                                       QMailFolderIdList& modifiedFolders)
{
    QString elements("id,mailfile,parentaccountid,parentfolderId");
    if (option == QMailStore::CreateRemovalRecord)
        elements += ",serveruid,frommailbox";

    QVariantList removalAccountIds;
    QVariantList removalServerUids;
    QVariantList removalMailboxes;

    {
        // Get the information we need to delete these messages
        QSqlQuery infoQuery(simpleQuery("SELECT " + elements + " FROM mailmessages WHERE",
                                        key,
                                        "deleteMessages info query"));
        if (infoQuery.lastError().type() != QSqlError::NoError)
            return false;

        while (infoQuery.next()) {
            QMailMessageId messageId(extractValue<quint64>(infoQuery.value(0)));
            deletedMessages.append(messageId);
            
            expiredMailfiles.append(extractValue<QString>(infoQuery.value(1)));

            QMailAccountId parentAccountId(extractValue<quint64>(infoQuery.value(2)));
            if (!modifiedAccounts.contains(parentAccountId))
                modifiedAccounts.append(parentAccountId);

            QMailFolderId folderId(extractValue<quint64>(infoQuery.value(3)));
            if (!modifiedFolders.contains(folderId))
                modifiedFolders.append(folderId);

            if (option == QMailStore::CreateRemovalRecord) {
                // Extract the info needed to create removal records
                removalAccountIds.append(parentAccountId.toULongLong());
                removalServerUids.append(extractValue<QString>(infoQuery.value(4)));
                removalMailboxes.append(extractValue<QString>(infoQuery.value(5)));
            }
        }
    }

    // No messages? Then we're already done
    if (deletedMessages.isEmpty())
        return true;

    // Any ancestor folders of the directly modified folders are indirectly modified
    QVariantList folderIdValues;
    foreach (const QMailFolderId& id, modifiedFolders)
        folderIdValues.append(id.toULongLong());

    {
        QString sql("SELECT DISTINCT id FROM mailfolderlinks WHERE descendantid IN %1");
        QSqlQuery ancestorFolderQuery(simpleQuery(sql.arg(expandValueList(folderIdValues)),
                                                  folderIdValues,
                                                  "deleteMessages mailfolderlinks ancestor query"));
        if (ancestorFolderQuery.lastError().type() != QSqlError::NoError)
            return false;

        while (ancestorFolderQuery.next())
            modifiedFolders.append(QMailFolderId(extractValue<quint64>(ancestorFolderQuery.value(0))));
    }

    // Insert the removal records
    if (!removalAccountIds.isEmpty()) {
        // WARNING - QList::operator<<(QList) actually appends the list items to the object,
        // rather than insert the actual list!
        QSqlQuery insertRemovalQuery(simpleQuery("INSERT INTO deletedmessages (parentaccountid,serveruid,frommailbox) VALUES (?,?,?)",
                                                 QVariantList() << QVariant(removalAccountIds)
                                                                << QVariant(removalServerUids)
                                                                << QVariant(removalMailboxes),
                                                 "deleteMessages insert removal records query",
                                                 true));
        if (insertRemovalQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    {
        // Perform the message deletion
        QSqlQuery deleteMessagesQuery(simpleQuery("DELETE FROM mailmessages WHERE",
                                                  key,
                                                  "deleteMessages delete mailmessages query"));
        if (deleteMessagesQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    return true;
}

bool QMailStorePrivate::deleteFolders(const QMailFolderKey& key, 
                                      QMailStore::MessageRemovalOption option, 
                                      QMailFolderIdList& deletedFolders, 
                                      QMailMessageIdList& deletedMessages, 
                                      QStringList& expiredMailfiles, 
                                      QMailAccountIdList& modifiedAccounts)
{
    {
        // Get the identifiers for all the folders we're deleting
        QSqlQuery infoQuery(simpleQuery("SELECT id FROM mailfolders WHERE",
                                        key,
                                        "deleteFolders info query"));
        if (infoQuery.lastError().type() != QSqlError::NoError)
            return false;

        while (infoQuery.next())
            deletedFolders.append(QMailFolderId(extractValue<quint64>(infoQuery.value(0))));
    }

    // No folders? Then we're already done
    if (deletedFolders.isEmpty()) 
        return true;

    // Create a key to select messages in the folders to be deleted
    QMailMessageKey messagesKey(QMailMessageKey::ParentFolderId, key);
    
    // We won't report the modified folders, since they're about to be deleted
    QMailFolderIdList modifiedFolders;

    // Delete all the messages contained by the folders we're deleting
    if (!deleteMessages(messagesKey, option, deletedMessages, expiredMailfiles, modifiedAccounts, modifiedFolders))
        return false;
    
    // Delete any references to these folders in the mailfolderlinks table
    QString statement("DELETE FROM mailfolderlinks WHERE %1 IN ( SELECT id FROM mailfolders WHERE %2 )");

    {
        // Delete where target folders are ancestors
        QSqlQuery deleteIdLinksQuery(simpleQuery(statement.arg("id").arg(buildWhereClause(key)),
                                                 whereClauseValues(key), 
                                                 "deleteFolders mailfolderlinks ancestor query"));
        if (deleteIdLinksQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    {
        // Delete where target folders are descendants
        QSqlQuery deleteDescendantIdLinksQuery(simpleQuery(statement.arg("descendantid").arg(buildWhereClause(key)),
                                                           whereClauseValues(key),
                                                           "deleteFolders mailfolderlinks descendant query"));
        if (deleteDescendantIdLinksQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    {
        // Perform the folder deletion
        QSqlQuery deleteFoldersQuery(simpleQuery("DELETE FROM mailfolders WHERE",
                                                 key,
                                                 "deleteFolders delete mailfolders query"));
        if (deleteFoldersQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    return true;
}

bool QMailStorePrivate::deleteAccounts(const QMailAccountKey& key, 
                                       QMailAccountIdList& deletedAccounts, 
                                       QMailFolderIdList& deletedFolders, 
                                       QMailMessageIdList& deletedMessages, 
                                       QStringList& expiredMailfiles)
{
    {
        // Get the identifiers for all the accounts we're deleting
        QSqlQuery infoQuery(simpleQuery("SELECT id FROM mailaccounts WHERE",
                                        key,
                                        "deleteAccounts info query"));
        if (infoQuery.lastError().type() != QSqlError::NoError)
            return false;

        while (infoQuery.next())
            deletedAccounts.append(QMailAccountId(extractValue<quint64>(infoQuery.value(0))));
    }

    // No folders? Then we're already done
    if (deletedAccounts.isEmpty()) 
        return true;

    // Create a key to select folders from the accounts to be deleted
    QMailFolderKey foldersKey(QMailFolderKey::ParentAccountId, key);
    
    // We won't create new message removal records, since there will be no account to link them to
    QMailStore::MessageRemovalOption option(QMailStore::NoRemovalRecord);
    QMailAccountIdList modifiedAccounts;

    // Delete all the folders contained by the accounts we're deleting
    if (!deleteFolders(foldersKey, option, deletedFolders, deletedMessages, expiredMailfiles, modifiedAccounts))
        return false;
    
    {
        // Delete the removal records related to these accounts
        QSqlQuery removalRecordDeleteQuery(simpleQuery("DELETE FROM deletedmessages WHERE",
                                                       foldersKey,
                                                       "deleteAccounts removal record delete query"));
        if (removalRecordDeleteQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    {
        // Perform the account deletion
        QSqlQuery deleteAccountsQuery(simpleQuery("DELETE FROM mailaccounts WHERE",
                                                  key,
                                                  "deleteAccounts delete mailaccounts query"));
        if (deleteAccountsQuery.lastError().type() != QSqlError::NoError)
            return false;
    }

    return true;
}

bool QMailStorePrivate::asynchronousEmission() const
{
    return asyncEmission;
}

QSqlQuery QMailStorePrivate::prepareSimpleQuery(const QString& statement, const QString& descriptor)
{
    QSqlQuery query = prepare(statement);
    if (query.lastError().type() != QSqlError::NoError) {
        qLog(Messaging) << "Could not prepare query" << descriptor;
    }

    return query;
}

static const QVariantList noValues;

void QMailStorePrivate::executeSimpleQuery(QSqlQuery& query, const QVariantList& bindValues, const QVariantList& whereValues, const QString& descriptor, bool batch)
{
    // Bind any query values supplied
    foreach (const QVariant& value, bindValues)
        query.addBindValue(value);

    // Bind any values contained in the where clause
    foreach (const QVariant& value, whereValues)
        query.addBindValue(value);

    if (!execute(query, batch))
        qLog(Messaging) << "Could not execute query" << descriptor;
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QString& descriptor, bool batch)
{
    return simpleQuery(statement, noValues, descriptor, batch);
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QVariantList& bindValues, const QString& descriptor, bool batch)
{
    QSqlQuery query(prepareSimpleQuery(statement, descriptor));
    if (query.lastError().type() == QSqlError::NoError)
        executeSimpleQuery(query, bindValues, noValues, descriptor, batch);

    return query;
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QMailMessageKey& key, const QString& descriptor, bool batch)
{
    return simpleQuery(statement, noValues, key, descriptor, batch);
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QVariantList& bindValues, const QMailMessageKey& key, const QString& descriptor, bool batch)
{
    QSqlQuery query(prepareSimpleQuery(statement + buildWhereClause(key), descriptor));
    if (query.lastError().type() == QSqlError::NoError)
        executeSimpleQuery(query, bindValues, whereClauseValues(key), descriptor, batch);

    return query;
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QMailFolderKey& key, const QString& descriptor, bool batch)
{
    return simpleQuery(statement, noValues, key, descriptor, batch);
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QVariantList& bindValues, const QMailFolderKey& key, const QString& descriptor, bool batch)
{
    QSqlQuery query(prepareSimpleQuery(statement + buildWhereClause(key), descriptor));
    if (query.lastError().type() == QSqlError::NoError)
        executeSimpleQuery(query, bindValues, whereClauseValues(key), descriptor, batch);

    return query;
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QMailAccountKey& key, const QString& descriptor, bool batch)
{
    return simpleQuery(statement, noValues, key, descriptor, batch);
}

QSqlQuery QMailStorePrivate::simpleQuery(const QString& statement, const QVariantList& bindValues, const QMailAccountKey& key, const QString& descriptor, bool batch)
{
    QSqlQuery query(prepareSimpleQuery(statement + buildWhereClause(key), descriptor));
    if (query.lastError().type() == QSqlError::NoError)
        executeSimpleQuery(query, bindValues, whereClauseValues(key), descriptor, batch);

    return query;
}

