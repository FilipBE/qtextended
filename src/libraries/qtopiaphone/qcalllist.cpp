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

#include <qcalllist.h>
#include <qtopianamespace.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <qtopiasql.h>
#include <QTimeZone>
#include <qtopialog.h>
#include <QtopiaIpcEnvelope>

class QCallListItemPrivate
{
public:
    QCallListItemPrivate()
        : count(1), type((QCallListItem::CallType)-1), number(QString()),
        start(QDateTime()), end(QDateTime()), originalTimeZoneStart(QDateTime()),
        originalTimeZoneEnd(QDateTime()), contact(QUniqueId()),
        serviceType(QString()), timeZoneId(QString()), simRecord(false)
        {
        }
    QCallListItemPrivate( QCallListItem::CallType _type,
            const QString& _number, const QDateTime& _start,
            const QDateTime& _end, const QUniqueId& _contact,
            const QString& _serviceType, const QString& _timeZoneId,
            const bool _simRecord )
        : count(1), type(_type), number(_number),
        start(_start), end(_end), originalTimeZoneStart(_start),
        originalTimeZoneEnd(_end),contact(_contact),
        serviceType(_serviceType), timeZoneId(_timeZoneId), simRecord(_simRecord)
        {
            if ( !_simRecord && _timeZoneId.isEmpty() )
                timeZoneId = QTimeZone().current().id();
        }
    QCallListItemPrivate( QCallListItem::CallType _type,
            const QString& _number, bool _simRecord )
        : count(1), type(_type), number(_number), simRecord(_simRecord)
        {
            if ( !_simRecord )
                timeZoneId = QTimeZone().current().id();
        }

    uint count;
    QCallListItem::CallType type;
    QString number;
    QDateTime start;
    QDateTime end;
    QDateTime originalTimeZoneStart;
    QDateTime originalTimeZoneEnd;
    QUniqueId contact;
    QString serviceType;
    QString timeZoneId;
    bool simRecord;

    void ref()          { count++; }
    bool deref()        { return !--count; }
};

// sql database
static QSqlDatabase *db = 0;

class QCallListPrivate
{
public:
    QCallListPrivate()
    {
        limit = -1;
    }

    static QSqlDatabase database()
    {
        if ( !db )
            db = new QSqlDatabase( QtopiaSql::instance()->systemDatabase() );
        if ( !db->isOpen() )
            qWarning() << "Failed to open system database()";
        return *db;
    }

    QList<QCallListItem> callList;
    int limit;

    void load();
    int timeZoneRecId( const QString & id );
    void record( QCallListItem item, bool permitDuplicates=false );
    void checkLimit();
    void clear();
    void removeAt( uint posn );
    bool removeByNumber( const QString& number );
    bool removeRecentDuplicate( QCallListItem item );
    void removeByType( QCallListItem::CallType type );
    QList<QCallListItem> searchCalls( QCallList::SearchOptions& options );

};

/*!
    \enum QCallListItem::CallType
    This enum defines the types of calls that may be recorded.

    \value Dialed An outgoing call that was dialed by the user.
    \value Received An incoming call that was answered by the user.
    \value Missed An incoming call that was not answered by the user.
*/

/*!
    \class QCallListItem
    \inpublicgroup QtTelephonyModule

    \brief The QCallListItem class specifies information about a single dialed, received or missed call in a QCallList set.

    \sa QCallList

    \ingroup telephony
*/

/*!
  Constructs a null QCallListItem.
*/
QCallListItem::QCallListItem()
{
    d = 0;
}

/*!
  Returns true if this is an invalid call list item.
  */
bool QCallListItem::isNull() const
{
    return 0 == d;
}

/*!
  Constructs a QCallListItem given the \a type of call, the phone \a number
  and the \a start and \a end times of the call.
  The contact can be optionally specified by \a contact.
  \a serviceType specifies the type of service such as Voice or VoIP.
  \a timeZoneId describes the time zone id at the time when the call is made e.g. Australia/Brisbane.
  If \a simRecord is true, then the call list item originated from a storage
  area on a SIM.
*/
QCallListItem::QCallListItem( CallType type, const QString& number,
        const QDateTime& start, const QDateTime &end,
        const QUniqueId& contact, const QString& serviceType,
        const QString& timeZoneId, bool simRecord )
{
    d = new QCallListItemPrivate( type, number, start, end, contact, serviceType, timeZoneId, simRecord );
}

/*!
  Constructs a QCallListItem given the \a type of call and the phone \a number.
  Use this constructor when reading SIM phone books as they contain only call type and phone number.
  If \a simRecord is true, then the call list item originated from a storage
  area on a SIM.
*/
QCallListItem::QCallListItem( CallType type, const QString& number, bool simRecord )
{
    d = new QCallListItemPrivate( type, number, simRecord );
}

/*!
  Constructs copy of \a other.
*/
QCallListItem::QCallListItem( const QCallListItem& other )
{
    d = other.d;
    if ( d ) {
        d->ref();
    }
}

/*!
  Destructs the QCallListItem.
*/
QCallListItem::~QCallListItem()
{
    if ( d && d->deref() ) {
        delete d;
    }
    d = 0;
}


/*!
  Returns true if this item is the same as \a other or false if they
  are different.

  The time properties such as start() and end()
  are converted to string using Qt::ISODate format for comparison.
*/
bool QCallListItem::operator== ( const QCallListItem& other ) const
{
    if ( !d || !(other.d) ) {
        return ( d == other.d );
    }
    return ( d->type == other.d->type &&
             d->number == other.d->number &&
             d->start.toString( Qt::ISODate ) == other.d->start.toString( Qt::ISODate ) &&
             d->end.toString( Qt::ISODate ) == other.d->end.toString( Qt::ISODate ) &&
             d->originalTimeZoneStart.toString( Qt::ISODate ) == other.d->originalTimeZoneStart.toString( Qt::ISODate ) &&
             d->originalTimeZoneEnd.toString( Qt::ISODate ) == other.d->originalTimeZoneEnd.toString( Qt::ISODate ) &&
             d->contact == other.d->contact &&
             d->serviceType == other.d->serviceType &&
             d->timeZoneId == other.d->timeZoneId
             );
}

/*!
    \fn bool QCallListItem::operator!= ( const QCallListItem& other ) const

    Returns true if this item is different than \a other or false if they
    are the same.
*/

/*!
  Assigns a copy of \a other to this item.
*/
QCallListItem& QCallListItem::operator= ( const QCallListItem& other )
{
    if ( other.d ) {
        other.d->ref();
    }
    if ( d && d->deref() ) {
        delete d;
    }
    d = other.d;
    return *this;
}

/*!
  Returns the type of call this item represents.
  If this item is null, then the return value of this function is undefined.

  \sa QCallListItem::CallType
*/
QCallListItem::CallType QCallListItem::type() const
{
    if ( d )
        return d->type;
    else
        return QCallListItem::Dialed;
}


/*!
  Returns the phone number associated with this call.
*/
QString QCallListItem::number() const
{
    if ( d )
        return d->number;
    else
        return "";
}


/*!
  Returns the starting time of the call.

  \sa end(), originalTimeZoneStart()
*/
QDateTime QCallListItem::start() const
{
    if ( d )
        return d->start;
    else
        return QDateTime();
}

/*!
  Returns end time of the call, or a null QDateTime if no connection was made for
  the call in the case of a missed call, or a dialed call that did not connect.

  \sa start(), originalTimeZoneEnd()
*/
QDateTime QCallListItem::end() const
{
    if ( d )
        return d->end;
    else
        return QDateTime();
}

/*!
  Returns the starting time of the call in the original timezone
  that the user was in when the call was made or received.

  This can be useful to display that a call started at 11:00 am
  local time, regardless of what the timezone is set to now.
  The start() function returns the start time in the current timezone.

  \sa originalTimeZoneEnd(), start()
*/
QDateTime QCallListItem::originalTimeZoneStart() const
{
    if ( d )
        return d->originalTimeZoneStart;
    else
        return QDateTime();
}

/*!
  Returns the ending time of the call in the original timezone
  that the user was in when the call was made or received.

  This can be useful to display that a call ended at 11:05 am
  local time, regardless of what the timezone is set to now.
  The end() function returns the end time in the current timezone.

  \sa originalTimeZoneStart(), end()
*/
QDateTime QCallListItem::originalTimeZoneEnd() const
{
    if ( d )
        return d->originalTimeZoneEnd;
    else
        return QDateTime();
}

/*!
  Returns the unique id of the contact associated with this call, if any.
*/
QUniqueId QCallListItem::contact() const
{
    if ( d )
        return d->contact;
    else
        return QUniqueId();
}

/*!
  Returns the service type associated with this call (Voice, Data, etc).
*/
QString QCallListItem::serviceType() const
{
    if ( d )
        return d->serviceType;
    else
        return QString();
}

/*!
  Returns the time zone id associated with this call (e.g. Australia/Brisbane).
*/
QString QCallListItem::timeZoneId() const
{
    if ( d )
        return d->timeZoneId;
    else
        return QString();
}

/*!
  Returns true if the call item is imported from SIM phone book.
*/
bool QCallListItem::isSIMRecord() const
{
    if ( d )
        return d->simRecord;
    else
        return false;
}


/*!
    \fn QCallListItem::serialize(Stream &stream) const

    Writes the contents of a QCallListItem to a \a stream.
*/
template <typename Stream> void QCallListItem::serialize(Stream &stream) const
{
    if (!d)
        d = new QCallListItemPrivate();
    stream << d->count;
    stream << (int)d->type;
    stream << d->number;
    stream << d->start;
    stream << d->end;
    stream << d->originalTimeZoneStart;
    stream << d->originalTimeZoneEnd;
    stream << d->contact;
    stream << d->serviceType;
    stream << d->timeZoneId;
    stream << (int)d->simRecord;
}

/*!
    \fn QCallListItem::deserialize(Stream &stream)

    Reads the contents of a QCallListItem from \a stream.
*/
template <typename Stream> void QCallListItem::deserialize(Stream &stream)
{
    if (!d) {
        d = new QCallListItemPrivate();
    } else if (d->count > 1) {
        d->deref();
        d = new QCallListItemPrivate();
    }
    int flag;
    stream >> d->count;
    stream >> flag;
    d->type = (QCallListItem::CallType)flag;
    stream >> d->number;
    stream >> d->start;
    stream >> d->end;
    stream >> d->originalTimeZoneStart;
    stream >> d->originalTimeZoneEnd;
    stream >> d->contact;
    stream >> d->serviceType;
    stream >> d->timeZoneId;
    stream >> flag;
    d->simRecord = ( flag != 0 );
}


/*!
    \class QCallList
    \inpublicgroup QtTelephonyModule

    \brief The QCallList class provides an interface for recording the details of dialed, received, and missed calls.

    \sa QCallListItem

    \ingroup telephony
*/

/*!
  Constructs a QCallList with an item limit of \a maxItemCount.
*/
QCallList::QCallList( int maxItemCount )
{
    d = new QCallListPrivate();
    setMaxItems( maxItemCount );

    QtopiaChannel *channel = new QtopiaChannel( "QPE/CallList", this );
    QObject::connect( channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(callListMessage(QString,QByteArray)) );

    d->load();
}

/*!
  Returns a QList containing this QCallList's calls.
*/
QList<QCallListItem> QCallList::allCalls() const
{
    QList<QCallListItem> rawList;
    if( d )
        rawList = d->callList;
    return rawList;
}


/*!
  Destructs the QCallList
*/
QCallList::~QCallList()
{
    delete d;
}

/*!
    \enum QCallList::ListType
    Defines the type of the call list.

    \value All List of all types of calls.
    \value Dialed List of outgoing calls.
    \value Received List of incoming calls.
    \value Missed List of missed calls.
*/

/*!
    \enum QCallList::DuplicateBehaviour
    Defines the duplicate handling behavior of QCallList::record().

    \value AllowDuplicates Allow duplicate numbers in the call list.
    \value OverwriteDuplicates Overwrite recent duplicate numbers in the call list.
*/

/*!
    \class QCallList::SearchOptions
    \inpublicgroup QtTelephonyModule

    \brief The SearchOptions class defines parameters to a search on the call history database.

    The \c number field indicates the phone number to search for, or an empty
    string if no number.  The \c contactId field indicates the QContact identifier
    to search for, or a null identifier if no contact.
*/

/*!
  Records the call \a item. If \a duplicates is \i AllowDuplicates
  (the default) duplicates of the number in \a item are allowed in
  the QCallList, otherwise any duplicate of the number is removed.
  The changes are saved to disk immediately.

  Duplicates occur when the same number is dialed, received, or missed
  as a previously recorded call.  The duplicate entry is stored with
  the new start and end times.  If \a duplicates is \i OverwriteDuplicates,
  and the number was recently seen, then the original start and end times
  will be replaced with the new start and end times.

  The \i OverwriteDuplicates flag is primarily used to collapse several
  recent entries for the same number into a single entry.  This is mostly
  useful for repeated dial attempts, or repeated missed calls, where it
  is only interesting when the last attempt was made, not a full log
  of all attempts.
*/
void QCallList::record( QCallListItem item, DuplicateBehaviour duplicates )
{
    d->record( item, duplicates == AllowDuplicates );
    d->checkLimit();

    // send message for other QCallList objects sync to the current status.
    QtopiaIpcEnvelope env( "QPE/CallList", "updated()" );
}

/*!
    \internal
    Listens to QPE/CallList channel to synchronize across a number of call list objects.
*/
void QCallList::callListMessage( const QString &message, const QByteArray & )
{
    if ( message == "updated()" )
        emit updated();
}

/*!
    \fn void QCallList::updated();

    This signal is emitted whenever call list is updated.

    \sa clear(), record(), removeAll(), removeAt()
*/

/*!
  Returns the number of items in the QCallList.

  \sa at()
*/
uint QCallList::count() const
{
    return d->callList.count();
}


/*!
  Returns the QCallListItem at \a posn.

  \sa count()
*/
QCallListItem QCallList::at( uint posn ) const
{
    if ( posn < count() ) {
        return d->callList[ posn ];
    } else {
        return QCallListItem();
    }
}


/*!
  Clears the QCallList and saves the changes to the database. updated() signal is emitted.
*/
void QCallList::clear()
{
    d->clear();

    // send message for other QCallList objects sync to the current status.
    QtopiaIpcEnvelope env( "QPE/CallList", "updated()" );
}


/*!
  Removes the item at \a posn from the list and saves
  the changes to the database. updated() signal is emitted.

  \sa removeAll()
*/
void QCallList::removeAt( uint posn )
{
    d->removeAt( posn );

    // send message for other QCallList objects sync to the current status.
    QtopiaIpcEnvelope env( "QPE/CallList", "updated()" );
}


/*!
  Removes all items with the same \a number from the list.
  The changes a saved to the database immediately and updated() signal is emitted.

  \sa removeAt()
*/
void QCallList::removeAll( const QString& number )
{
    d->removeByNumber( number );

    // send message for other QCallList objects sync to the current status.
    QtopiaIpcEnvelope env( "QPE/CallList", "updated()" );
}


/*!
  Removes all items with the same \a type from the list.
  The changes are recorded to the database immediately and updated() signal is emitted.

  \sa removeAt()
*/
void QCallList::removeAll( QCallListItem::CallType type )
{
    d->removeByType( type );

    // send message for other QCallList objects sync to the current status.
    QtopiaIpcEnvelope env( "QPE/CallList", "updated()" );
}

/*!
  Returns a QList containing QCallListItems that satisfy the search criteria \a options.
*/
QList<QCallListItem> QCallList::searchCalls( SearchOptions& options ) const
{
    return d->searchCalls( options );
}

/*!
  \internal
  Initialize ithe current time zone info and the call list by reading the database.
*/
void QCallListPrivate::load()
{
    QSqlQuery query( database() );
    QString statement;

    // select all call items.
    statement = "SELECT callhistory.recid, \
                 callhistory.servicetype, \
                 callhistory.calltype, \
                 callhistory.phonenumber, \
                 callhistory.contactid, \
                 callhistory.time, \
                 callhistory.endtime, \
                 callhistory.simrecord, \
                 callhistorytimezone.timezoneid \
                 FROM callhistory LEFT JOIN callhistorytimezone \
                 ON callhistory.timezoneid = callhistorytimezone.recid \
                 ORDER BY callhistory.recid DESC \
                 LIMIT :limit";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return;
    }

    query.bindValue( ":limit", limit );

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return;
    }

    QSqlRecord rec = query.record();
    int servicetypeCol = rec.indexOf( "servicetype" );
    int calltypeCol = rec.indexOf( "calltype" );
    int phonenumberCol = rec.indexOf( "phonenumber" );
    int contactidCol = rec.indexOf( "contactid" );
    int timeCol = rec.indexOf( "time" );
    int endtimeCol = rec.indexOf( "endtime" );
    int timezoneidCol = rec.indexOf( "timezoneid" );
    int simrecordCol = rec.indexOf( "simrecord" );

    while ( query.next() ) {
        QDateTime time, endTime;
        QVariant timeV(query.value(timeCol));
        QVariant endTimeV(query.value(endtimeCol));
        if (!timeV.isNull())    time    = timeV.toDateTime();
        if (!endTimeV.isNull()) endTime = endTimeV.toDateTime();
        QCallListItem newItem( static_cast<QCallListItem::CallType>(query.value(calltypeCol).toInt()),
                query.value(phonenumberCol).toString(),
                time,
                endTime,
                QUniqueId::fromUInt( query.value(contactidCol).toUInt() ),
                query.value(servicetypeCol).toString(),
                query.value(timezoneidCol).toString(),
                query.value(simrecordCol).toInt() );

        callList.append( newItem );
    }

}

/*!
  \internal
  Finds a time zone record id using \a id.
  If \a id does not exist in the database, it will create a new entry and return the record id.
*/
int QCallListPrivate::timeZoneRecId( const QString & id )
{
    QSqlQuery query( database() );
    QString statement;

    // see if the timezone exists
    statement = "SELECT recid FROM callhistorytimezone WHERE (timezoneid = :timezoneid)";
    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return -1;
    }

    query.bindValue( ":timezoneid", id );

    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return -1;
    }

    if ( query.next() ) // timezone exists
        return query.value( 0 ).toInt();

    // have to insert the current time zone to the table.
    statement = "INSERT INTO callhistorytimezone (timezoneid) VALUES (:timezoneid)";
    query.clear();
    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return -1;
    }

    query.bindValue( ":timezoneid", id );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return -1;
    }

    // now retreive the record id for the current time zone.
    statement = "SELECT recid FROM callhistorytimezone WHERE (timezoneid = :timezoneid)";
    query.clear();
    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return -1;
    }

    query.bindValue( ":timezoneid", id );

    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return -1;
    }

    if ( query.next() )
        return query.value( 0 ).toInt();
    return -1;
}


/*!
  Sets the item limit of the list to \a l.

  \sa maxItems()
*/
void QCallList::setMaxItems( const int &l )
{
    d->limit = l;
}

/*!
  Returns the item limit of the list.

  \sa setMaxItems()
*/
int QCallList::maxItems() const
{
    return d->limit;
}

/*!
  \internal
  Records the item, the logic is depending on premitDuplicates.
  If permitDuplicates is true item will be just prepended.
  If not, the consecutive occurrences with the same number in the beginning of the list
  will be removed and only the current one will be stored.
*/
void QCallListPrivate::record( QCallListItem item, bool permitDuplicates )
{
    if ( item.isNull() )
        return;

    QSqlQuery query( database() );
    QString statement;

    // for incoming calls & SIM records, need to find a contact id manually.
    QUniqueId contactid;
    if ( item.type() != QCallListItem::Dialed || item.isSIMRecord() ) {
        statement = "SELECT recid FROM contactphonenumbers \
                     WHERE phone_number = :number";
        if ( !query.prepare( statement ) ) {
            qLog(Sql) << "Failed to prepare query, " << statement;
        } else {
            query.bindValue( ":number", item.number() );
            QtopiaSql::instance()->logQuery( query );
            if ( !query.exec() ) {
                qLog(Sql) << "Failed to execute query," << statement;
            } else {
                // if there is more than one contacts that has the same number
                // it can not be decieded which one is calling.
                int i = 0;
                while ( query.next() ) {
                    contactid = QUniqueId::fromUInt( query.value( 0 ).toUInt() );
                    ++i;
                }
                if ( i != 1 )
                    contactid = QUniqueId();
            }
        }
    }

    // insert new record
    query.clear();

    if ( !item.isSIMRecord() ) { // normal call items
        statement = "INSERT INTO callhistory \
                    (servicetype, calltype, phonenumber, contactid, time, endtime, timezoneid, simrecord) \
                    VALUES (:servicetype, :calltype, :phonenumber, :contactid, :time, :endtime, :timezoneid, 0)";
        if ( !query.prepare( statement ) ) {
            qLog(Sql) << "Failed to prepare query," << statement;
            return;
        }

        query.bindValue( ":servicetype", item.serviceType() );
        query.bindValue( ":calltype", item.type() );
        query.bindValue( ":phonenumber", item.number() );
        if ( item.type() == QCallListItem::Dialed )
            query.bindValue( ":contactid", item.contact().toUInt() );
        else
            query.bindValue( ":contactid", contactid.toUInt() );
        query.bindValue( ":time", item.start() );
        query.bindValue( ":endtime", item.end() );
        query.bindValue( ":timezoneid", timeZoneRecId( item.timeZoneId() ) );
    } else { // SIM record
        statement = "INSERT INTO callhistory \
                     (calltype, phonenumber, simrecord) \
                     VALUES(:calltype, :phonenumber, 1)";
        if ( !query.prepare( statement ) ) {
            qLog(Sql) << "Failed to prepare query," << statement;
            return;
        }

        query.bindValue( ":calltype", item.type() );
        query.bindValue( ":phonenumber", item.number() );
    }

    QtopiaSql::instance()->logQuery( query );

    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return;
    }

    // contact id needs to be updated.
    if ( !item.isSIMRecord() && item.type() != QCallListItem::Dialed ) {
        QCallListItem newItem( item.type(), item.number(),
            item.start(), item.end(),
            contactid, item.serviceType(),
            item.timeZoneId(), false );
        callList.prepend( newItem );
    } else {
        callList.prepend( item );
    }

    if ( !permitDuplicates && callList.count() > 1 )
        removeRecentDuplicate( item );
}

/*!
  \internal
  Checks if the count of call item exceeds the limit.
  If so, remove the last items.
*/
void QCallListPrivate::checkLimit()
{
    if ( limit < 0 || ( limit > 0 && callList.count() < limit ) )
        return;

    // delete records from the table.
    QSqlQuery query( database() );
    QString statement = "DELETE FROM callhistory \
                         WHERE recid < (SELECT MIN(recid) FROM \
                         (SELECT recid FROM callhistory \
                          ORDER BY recid DESC LIMIT :limit))";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return;
    }

    query.bindValue( ":limit", limit );

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return;
    }

    // remove call items from the list
    while ( callList.count() > limit ) {
        callList.removeLast();
    }
}

/*!
  \internal
  Removes all the data from the table and call list.
*/
void QCallListPrivate::clear()
{
    // delete records from the table.
    QSqlQuery query( database() );
    QString statement = "DELETE FROM callhistory";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return;
    }

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return;
    }

    callList.clear();
}

void QCallListPrivate::removeAt( uint posn )
{
    // delete records from the table.
    QSqlQuery query( database() );
    QString statement = "DELETE FROM callhistory \
                         WHERE recid = (SELECT MIN(recid) FROM \
                         (SELECT recid FROM callhistory \
                          ORDER BY recid DESC LIMIT :limit))";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return;
    }

    query.bindValue( ":limit", posn + 1 );

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return;
    }

    callList.removeAt( posn );
}

bool QCallListPrivate::removeByNumber( const QString& number )
{
    // delete records from the table.
    QSqlQuery query( database() );
    QString statement = "DELETE FROM callhistory \
                         WHERE phonenumber = :number";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return false;
    }

    query.bindValue( ":number", number );

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return false;
    }

    // remove call item from the list.
    bool removed = false;
    QList<QCallListItem>::Iterator it = callList.begin();
    while( it != callList.end() )
    {
        if( (*it).number() == number )
        {
            it = callList.erase( it );
            removed = true;
        }
        else
        {
            ++it;
        }
    }

    return removed;
}

/*!
  \internal
  Removes recent duplicate if any from the list and the database.
*/
bool QCallListPrivate::removeRecentDuplicate( QCallListItem item )
{
    bool removed = false;

    QList<QCallListItem>::Iterator it = callList.begin();
    ++it; // comparing starts from the second item.

    while( it != callList.end() )
    {
        // different number found. no need to go further.
        if( (*it).number() != item.number() && (*it).contact() == item.contact() )
            break;

        // delete record from the table.
        QSqlQuery query( database() );
        QString statement = "DELETE FROM callhistory \
                             WHERE recid = (SELECT MIN(recid) FROM \
                             (SELECT recid FROM callhistory \
                              ORDER BY recid DESC LIMIT 2))";

        if ( !query.prepare( statement ) ) {
            qLog(Sql) << "Failed to prepare query," << statement;
            return removed;
        }

        QtopiaSql::instance()->logQuery( query );
        if ( !query.exec() ) {
            qLog(Sql) << "Failed to execute query," << statement;
            return removed;
        }

        // remove from the list.
        it = callList.erase( it );

        removed = true;
    }

    return removed;
}

/*!
  \internal
  Duplicates are not permitted. remove the recent occurrences of this number.
*/
void QCallListPrivate::removeByType( QCallListItem::CallType type )
{
    // delete records from the table.
    QSqlQuery query( database() );
    QString statement = "DELETE FROM callhistory \
                         WHERE calltype = :type";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return;
    }

    query.bindValue( ":type", type );

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return;
    }

    // remove call item from the list.
    QList<QCallListItem>::Iterator it = callList.begin();
    while( it != callList.end() )
    {
        if( (*it).type() == type)
        {
            it = callList.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

/*!
  \internal
  Returns a QList containing QCallListItems that meets search criteria as specified in \a options.
*/
QList<QCallListItem> QCallListPrivate::searchCalls( QCallList::SearchOptions& options )
{
    QList<QCallListItem> rawList;

    QSqlQuery query( database() );
    QString statement;

    statement = "SELECT callhistory.recid, \
                 callhistory.servicetype, \
                 callhistory.calltype, \
                 callhistory.phonenumber, \
                 callhistory.contactid, \
                 callhistory.time, \
                 callhistory.endtime, \
                 callhistory.simrecord, \
                 callhistorytimezone.timezoneid \
                 FROM callhistory LEFT JOIN callhistorytimezone \
                 ON callhistory.timezoneid = callhistorytimezone.recid ";

    if ( !options.number.isEmpty() )
        statement += "WHERE callhistory.phonenumber = :number ";

    if ( !options.contactId.isNull() ) {
        if ( options.number.isEmpty() )
            statement += "WHERE callhistory.contactid = :contactid ";
        else
            statement += "AND callhistory.contactid = :contactid ";
    }

    statement += "ORDER BY callhistory.recid DESC LIMIT :limit";

    if ( !query.prepare( statement ) ) {
        qLog(Sql) << "Failed to prepare query," << statement;
        return rawList;
    }

    if ( !options.number.isEmpty() )
        query.bindValue( ":number", options.number );

    if ( !options.contactId.isNull() )
        query.bindValue( ":contactid", options.contactId.toUInt() );

    query.bindValue( ":limit", limit );

    QtopiaSql::instance()->logQuery( query );
    if ( !query.exec() ) {
        qLog(Sql) << "Failed to execute query," << statement;
        return rawList;
    }

    QSqlRecord rec = query.record();
    int servicetypeCol = rec.indexOf( "servicetype" );
    int calltypeCol = rec.indexOf( "calltype" );
    int phonenumberCol = rec.indexOf( "phonenumber" );
    int contactidCol = rec.indexOf( "contactid" );
    int timeCol = rec.indexOf( "time" );
    int endtimeCol = rec.indexOf( "endtime" );
    int timezoneidCol = rec.indexOf( "timezoneid" );
    int simrecordCol = rec.indexOf( "simrecord" );

    while ( query.next() ) {
        QDateTime time, endTime;
        QVariant timeV(query.value(timeCol));
        QVariant endTimeV(query.value(endtimeCol));
        if (!timeV.isNull())    time    = timeV.toDateTime();
        if (!endTimeV.isNull()) endTime = endTimeV.toDateTime();
        QCallListItem newItem( static_cast<QCallListItem::CallType>(query.value(calltypeCol).toInt()),
                query.value(phonenumberCol).toString(),
                time,
                endTime,
                QUniqueId::fromUInt( query.value(contactidCol).toUInt() ),
                query.value(servicetypeCol).toString(),
                query.value(timezoneidCol).toString(),
                query.value(simrecordCol).toInt() );

        rawList.append( newItem );
    }

    return rawList;
}

Q_IMPLEMENT_USER_METATYPE(QCallListItem);
Q_IMPLEMENT_USER_METATYPE_ENUM(QCallList::ListType);

