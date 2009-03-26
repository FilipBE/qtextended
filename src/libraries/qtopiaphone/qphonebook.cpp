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

#include <qphonebook.h>

/*!
    \class QPhoneBookEntry
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneBookEntry class specifies the contents of a single entry in a device's phone book.

    Phone book entries are retrieved by a call to QPhoneBook::getEntries().

    \ingroup telephony
    \sa QPhoneBook
*/

/*!
    \fn QPhoneBookEntry::QPhoneBookEntry()

    Constructs an empty phone book entry.
*/

/*!
    \fn QPhoneBookEntry::QPhoneBookEntry(const QPhoneBookEntry &other)

    Constructs a copy of \a other.
*/

/*!
    \fn QPhoneBookEntry::~QPhoneBookEntry()

    Destructs a phone book entry.
*/

/*!
    \fn QPhoneBookEntry & QPhoneBookEntry::operator=(const QPhoneBookEntry &other)

    Copies \a other into this phone book entry object.
*/

/*!
    \fn bool QPhoneBookEntry::operator==(const QPhoneBookEntry &other)

    Returns true if this phone book entry has the same index as \a other; otherwise returns false.
*/

/*!
    \fn uint QPhoneBookEntry::index() const

    Returns the index associated with this phone book entry.

    \sa setIndex()
*/

/*!
    \fn void QPhoneBookEntry::setIndex(uint value)

    Sets the index associated with this phone book entry to \a value.

    \sa index()
*/

/*!
    \fn QString QPhoneBookEntry::number() const

    Returns the phone number associated with this phone book entry.

    \sa setNumber()
*/

/*!
    \fn void QPhoneBookEntry::setNumber(const QString& value)

    Sets the phone number associated with this phone book entry to \a value.

    \sa number()
*/

/*!
    \fn QString QPhoneBookEntry::text() const

    Returns the textual name associated with this phone book entry.

    \sa setText()
*/

/*!
    \fn void QPhoneBookEntry::setText(const QString& value)

    Sets the textual name associated with this phone book entry to \a value.

    \sa text()
*/

/*!
    \class QPhoneBook
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneBook class provides an interface to the SIM phone books of a device.

    \ingroup telephony
    \sa QPhoneBookEntry, QPhoneBookLimits

    Phone devices contain a number of internal phone books.
    The principal one of these is \c SM, representing the phone
    book on the user's SIM card.

    The QPhoneBook class allows applications to access the contents of
    SIM phone books.  For access to full phone book information,
    including SIM phone books and on-disk phone books, use QContactModel.

    The following phone book stores are commonly provided on SIM's according
    to 3GPP TS 27.007, and can be accessed using QPhoneBook:

    \table
    \row \o \c SM \o Writable SIM phone book for user-defined names and numbers.
    \row \o \c SN \o Read-only SIM phone book for operator-defined service numbers.
    \row \o \c DC \o Read-only list of recently dialed calls.
    \row \o \c LD \o Read-only list containing the last dialed call.
    \row \o \c MC \o Read-only list of recently missed incoming calls.
    \row \o \c RC \o Read-only list of recently received calls.
    \row \o \c EN \o Read-only list of emergency numbers set by the network operator.
    \row \o \c FD \o Fixed dialing phone book, listing the numbers that can be dialed
                     when the fixed dialing mode is activated with setFixedDialingState().
    \endtable

    The \c ME, \c MT, and \c TA phone books mentioned in 3GPP TS 27.007 cannot be accessed
    using QPhoneBook; use QContactModel instead.

    The storages() method can be used to determine which storages are supported
    on the current SIM.

    \sa QPhoneBookEntry, QContactModel
*/

/*!
    \internal
    \fn void QPhoneBookEntry::deserialize(Stream &stream)
*/
template <typename Stream> void QPhoneBookEntry::deserialize(Stream &stream)
{
    uint index;
    QString number;
    QString text;
    stream >> index;
    stream >> number;
    stream >> text;
    setIndex( index );
    setNumber( number );
    setText( text );
}

/*!
    \internal
    \fn void QPhoneBookEntry::serialize(Stream &stream) const
*/
template <typename Stream> void QPhoneBookEntry::serialize(Stream &stream) const
{
    stream << index();
    stream << number();
    stream << text();
}

/*!
    Construct a new SIM phone book access object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SIM phone books.  If there is more
    than one service that supports SIM phone books, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QPhoneBook objects for each.

    \sa QCommServiceManager::supports()
*/
QPhoneBook::QPhoneBook( const QString& service, QObject *parent,
                        QCommInterface::Mode mode )
    : QCommInterface( "QPhoneBook", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this SIM phone book access object.
*/
QPhoneBook::~QPhoneBook()
{
    // Nothing to do here.
}

/*!
    Get all entries in the storage area \a store.  The most common
    storage area is called \c SM, indicating the phone book on
    the user's SIM card.  The entries are returned via the entries() signal.

    \sa entries()
*/
void QPhoneBook::getEntries( const QString& store )
{
    invoke( SLOT(getEntries(QString)), store );
}

/*!
    Add an \a entry consisting of number and text to the storage area
    \a store.  If \a flush is true, then flush the modifications immediately;
    otherwise wait for an explicit call to the flush() method.
    The \c index field of \a entry is ignored.

    \sa remove(), update(), flush()
*/
void QPhoneBook::add( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    invoke( SLOT(add(QPhoneBookEntry,QString,bool)),
            qVariantFromValue( entry ), store, flush );
}

/*!
    Remove the entry at \a index from the storage area \a store.  If \a flush
    is true, then flush the modifications immediately; otherwise wait for an
    explicit call to the flush() method.

    \sa add(), update(), flush()
*/
void QPhoneBook::remove( uint index, const QString& store, bool flush )
{
    invoke( SLOT(remove(uint,QString,bool)), index, store, flush );
}

/*!
    Update the \a entry in the storage area \a store with the.
    If \a flush is true, then flush the modifications
    immediately; otherwise wait for an explicit call to the
    flush() method.

    \sa add(), remove(), flush()
*/
void QPhoneBook::update( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    invoke( SLOT(update(QPhoneBookEntry,QString,bool)),
            qVariantFromValue( entry ), store, flush );
}

/*!
    Sets the password to use to access \a store to \a password.  This is
    typically the SIM PIN2 password for the \c FD (fixed dialing) store.
    This should be called before the first call on getEntries(), add(), etc.

    \sa clearPassword(), getEntries(), add()
*/
void QPhoneBook::setPassword( const QString& store, const QString& password )
{
    invoke( SLOT(setPassword(QString,QString)), store, password );
}

/*!
    Clear the password on \a store that was set using setPassword().
    This should be called after flush().

    \sa setPassword(), flush()
*/
void QPhoneBook::clearPassword( const QString& store )
{
    invoke( SLOT(clearPassword(QString)), store );
}

/*!
    Flush all pending operations on the storage area \a store.  This will cause
    the new list of entries for the storage area to be returned via the entries() signal.

    \sa add(), remove(), update(), entries()
*/
void QPhoneBook::flush( const QString& store )
{
    invoke( SLOT(flush(QString)), store );
}

/*!
    Request the field size and index limits for \a store.  The service
    responds with the limits() signal.

    \sa limits()
*/
void QPhoneBook::requestLimits( const QString& store )
{
    invoke( SLOT(requestLimits(QString)), store );
}

/*!
    Request the state of the fixed dialing lock on the SIM.  The
    service responds with the fixedDialingState() signal.

    \sa fixedDialingState(), setFixedDialingState()
*/
void QPhoneBook::requestFixedDialingState()
{
    invoke( SLOT(requestFixedDialingState()) );
}

/*!
    Sets the fixed dialing lock on the SIM to \a enabled.  The \a pin2
    argument specifies the PIN2 password to access the fixed dialing lock.
    The service will emit setFixedDialingStateResult() when the operation
    completes.

    \sa setFixedDialingStateResult(), requestFixedDialingState()
*/
void QPhoneBook::setFixedDialingState( bool enabled, const QString& pin2 )
{
    invoke( SLOT(setFixedDialingState(bool,QString)), enabled, pin2 );
}

/*!
    Request the supported storages.
*/
QStringList QPhoneBook::storages()
{
    return value( QLatin1String("Storages") ).toStringList();
}


/*!
    \fn void QPhoneBook::entries(const QString& store, const QList<QPhoneBookEntry>& list)
    Signal that is emitted to deliver the \a list of entries in the SIM
    storage area \a store.  Occurs whenever the list is explicitly
    requested with getEntries(), or the list is modified by
    add(), remove(), update(), or flush().

    \sa getEntries(), add(), remove(), update(), flush()
*/

/*!
    \fn void QPhoneBook::limits( const QString& store, const QPhoneBookLimits& value )

    Signal that is emitted in response to a requestLimits() call for \a store.
    The \a value structure contains the limits.  If the limits could not
    be obtained, \a value will be null.

    \sa requestLimits()
*/

/*!
    \fn void QPhoneBook::fixedDialingState( bool enabled )

    Signal that is emitted in response to requestFixedDialingState() to
    indicate whether the fixed dialing lock on the SIM is \a enabled
    or not.

    \sa requestFixedDialingState(), setFixedDialingState()
*/

/*!
    \fn void QPhoneBook::setFixedDialingStateResult( QTelephony::Result result )

    Signal that is emitted in response to setFixedDialingState() to report
    the \a result of the operation.

    \sa setFixedDialingState()
*/

/*!
    \fn void QPhoneBook::ready()

    Signal that is emitted when phone books are ready to be read.
    For example this signal can be emitted from \l {QModemPhoneBook}{phoneBooksReady()}.
*/

/*!
    \class QPhoneBookLimits
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneBookLimits class specifies the limits of a phone book's storage capacity.

    The limits of a phone book may be retrieved using QPhoneBook::requestLimits().

    \ingroup telephony
    \sa QPhoneBook
*/

class QPhoneBookLimitsPrivate
{
public:
    QPhoneBookLimitsPrivate()
    {
        numberLength = 0;
        textLength = 0;
        firstIndex = 0;
        lastIndex = 0;
        used = 0;
    }

    uint numberLength;
    uint textLength;
    uint firstIndex;
    uint lastIndex;
    uint used;

    void copy( const QPhoneBookLimitsPrivate *other )
    {
        numberLength = other->numberLength;
        textLength = other->textLength;
        firstIndex = other->firstIndex;
        lastIndex = other->lastIndex;
        used = other->used;
    }
};

/*!
    Construct a default limits structure and set all fields to zero.
*/
QPhoneBookLimits::QPhoneBookLimits()
{
    d = new QPhoneBookLimitsPrivate();
}

/*!
    Construct a copy of \a other.
*/
QPhoneBookLimits::QPhoneBookLimits( const QPhoneBookLimits& other )
{
    d = new QPhoneBookLimitsPrivate();
    d->copy( other.d );
}

/*!
    Destroy this limits structure.
*/
QPhoneBookLimits::~QPhoneBookLimits()
{
    delete d;
}

/*!
    Returns true if all fields within this limits structure are zero; otherwise returns false.
*/
bool QPhoneBookLimits::isNull() const
{
    return ( d->numberLength == 0 && d->textLength == 0 &&
             d->firstIndex == 0 && d->lastIndex == 0 &&
             d->used == 0 );
}

/*!
    Copy the contents of \a other into this object.
*/
QPhoneBookLimits& QPhoneBookLimits::operator=( const QPhoneBookLimits& other )
{
    if ( this != &other )
        d->copy( other.d );
    return *this;
}

/*!
    Returns the maximum length for phone numbers within the phone book.

    \sa setNumberLength()
*/
uint QPhoneBookLimits::numberLength() const
{
    return d->numberLength;
}

/*!
    Sets the maximum length for phone numbers within the phone book to \a value.

    \sa numberLength()
*/
void QPhoneBookLimits::setNumberLength( uint value )
{
    d->numberLength = value;
}

/*!
    Returns the maximum length for text names within the phone book.

    \sa setTextLength()
*/
uint QPhoneBookLimits::textLength() const
{
    return d->textLength;
}

/*!
    Sets the maximum length for text names within the phone book to \a value.

    \sa textLength()
*/
void QPhoneBookLimits::setTextLength( uint value )
{
    d->textLength = value;
}

/*!
    Returns the first index position that can store entries within the phone book.

    \sa setFirstIndex()
*/
uint QPhoneBookLimits::firstIndex() const
{
    return d->firstIndex;
}

/*!
    Sets the first index position that can store entries within
    the phone book to \a value.

    \sa firstIndex()
*/
void QPhoneBookLimits::setFirstIndex( uint value )
{
    d->firstIndex = value;
}

/*!
    Returns the last index position that can store entries within the phone book.

    \sa setLastIndex()
*/
uint QPhoneBookLimits::lastIndex() const
{
    return d->lastIndex;
}

/*!
    Sets the last index position that can store entries within
    the phone book to \a value.

    \sa lastIndex()
*/
void QPhoneBookLimits::setLastIndex( uint value )
{
    d->lastIndex = value;
}

/*!
    Returns the number of entries in the phone book that are currently in use.
    The total number of entries, including used and free, is equal to
    lastIndex() - firstIndex() + 1.

    \sa setUsed()
*/
uint QPhoneBookLimits::used() const
{
    return d->used;
}

/*!
    Sets the number of entries in the phone book that are currently in
    use to \a value.

    \sa used()
*/
void QPhoneBookLimits::setUsed( uint value )
{
    d->used = value;
}

/*!
    \internal
    \fn void QPhoneBookLimits::serialize(Stream &stream) const
*/
template <typename Stream> void QPhoneBookLimits::serialize(Stream &stream) const
{
    stream << d->numberLength;
    stream << d->textLength;
    stream << d->firstIndex;
    stream << d->lastIndex;
    stream << d->used;
}

/*!
    \internal
    \fn void QPhoneBookLimits::deserialize(Stream &stream)
*/
template <typename Stream> void QPhoneBookLimits::deserialize(Stream &stream)
{
    stream >> d->numberLength;
    stream >> d->textLength;
    stream >> d->firstIndex;
    stream >> d->lastIndex;
    stream >> d->used;
}

Q_IMPLEMENT_USER_METATYPE(QPhoneBookEntry)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QPhoneBookEntry>)
Q_IMPLEMENT_USER_METATYPE(QPhoneBookLimits)
