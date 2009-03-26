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

#include <qmessagewaiting.h>

/*!
    \class QMessageWaiting
    \inpublicgroup QtTelephonyModule

    \brief The QMessageWaiting class provides information about waiting messages, usually voice mail messages.
    \ingroup telephony

    Many telephony systems provide a method to notify handsets when there are voice
    mail messages waiting for the user.  The QMessageWaiting class provides access
    to this information via the status() method.

    Because there may be multiple telephone numbers or call classes associated
    with a telephony service (e.g. voice and fax messages), this interface
    reports the message waiting status information as a list of
    QMessageWaitingStatus objects.

    If the telephony service provides a standard voice mail number for accessing
    the waiting messages, it should be made available to client applications
    via the QServiceNumbers interface.

    The changed() signal is emitted whenever the message waiting state changes.

    Telephony service implementations that inherit from this method should
    call updateStatus() whenever the status of a call class and/or telephone
    number changes, to properly update the values and emit the changed() signal.

    \sa QMessageWaitingStatus, QServiceNumbers, QCommInterface
*/

class QMessageWaitingPrivate
{
public:
    int generation;
    QList<QMessageWaitingStatus> status;
};

/*!
    Construct a new message waiting status object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports message waiting.  If there is more
    than one service that supports message waiting, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QMessageWaiting objects for each.

    \sa QCommServiceManager::supports()
*/
QMessageWaiting::QMessageWaiting
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QMessageWaiting", service, parent, mode )
{
    proxyAll( staticMetaObject );
    d = new QMessageWaitingPrivate();
    if ( mode == Client )
        d->generation = -1;
    else
        d->generation = 0;
}

/*!
    Destroy this message waiting status object.
*/
QMessageWaiting::~QMessageWaiting()
{
    delete d;
}

/*!
    Returns the message waiting status indication for this telephony service.
    The list may have multiple entries if there is more than one number or
    call class associated with the telephony service.

    \sa totalStatus()
*/
QList<QMessageWaitingStatus> QMessageWaiting::status() const
{
    if ( mode() == Client ) {
        if ( d->generation == -1 ||
             d->generation != value( "generation" ).toInt() ) {
            // Fetch a new copy of the message status.
            d->generation = value( "generation" ).toInt();
            QByteArray data = value( "status" ).toByteArray();
            if ( data.isEmpty() ) {
                d->status.clear();
            } else {
                QDataStream stream( data );
                stream >> d->status;
            }
        }
    }
    return d->status;
}

// Add two message waiting count values, taking -1 for "not set" into account.
static int mwadd( int value1, int value2 )
{
    if ( value1 < 0 )
        return value2;
    else if ( value2 < 0 )
        return value1;
    else
        return value1 + value2;
}

/*!
    Returns a total of all message waiting status indications from status().
    This is useful when the user interface does not want to distinguish
    between multiple call classes when presenting a total to the user.

    The QMessageWaitingStatus::callClass() and QMessageWaitingStatus::number()
    fields of the result will have their default values: they are not
    affected by the contents of status().

    \sa status()
*/
QMessageWaitingStatus QMessageWaiting::totalStatus() const
{
    QList<QMessageWaitingStatus> list = status();
    QMessageWaitingStatus result;
    foreach ( QMessageWaitingStatus s, list ) {
        if ( s.unreadMessagesWaiting() ) {
            result.setUnreadMessagesWaiting( true );
        }
        result.setUnreadCount( mwadd( result.unreadCount(), s.unreadCount() ) );
        result.setReadCount( mwadd( result.readCount(), s.readCount() ) );
        result.setUrgentUnreadCount( mwadd( result.urgentUnreadCount(), s.urgentUnreadCount() ) );
        result.setUrgentReadCount( mwadd( result.urgentReadCount(), s.urgentReadCount() ) );
    }
    return result;
}

/*!
    \fn void QMessageWaiting::changed()

    Signal that is emitted when the message waiting status on this service changes.

    \sa updateStatus()
*/

/*!
    Updates the message waiting status indication to include \a status.
    If there is an existing indication for the call class and number
    within \a status, it will be replaced.  Otherwise a new indication
    will be added to the list.  The changed() signal is emitted once
    the update is complete.

    \sa changed(), clearAllStatus()
*/
void QMessageWaiting::updateStatus( const QMessageWaitingStatus& status )
{
    int index;
    for ( index = 0; index < d->status.size(); ++index ) {
        if ( d->status[index].callClass() == status.callClass() &&
             d->status[index].number() == status.number() ) {
            d->status[index] = status;
            updateValueSpace();
            return;
        }
    }
    d->status += status;
    updateValueSpace();
}

/*!
    Clears all message waiting status indications.  This is typically
    called when the user loses network connectivity.  The changed()
    signal is emitted.

    \sa changed(), updateStatus()
*/
void QMessageWaiting::clearAllStatus()
{
    d->status.clear();
    updateValueSpace();
}

void QMessageWaiting::updateValueSpace()
{
    QByteArray array;
    {
        QDataStream stream
            ( &array, QIODevice::WriteOnly | QIODevice::Append );
        stream << d->status;
        // The stream should be flushed to the array at this point.
    }
    setValue( "generation", d->generation );
    setValue( "status", array );
    emit changed();
    ++(d->generation);
}

/*!
    \class QMessageWaitingStatus
    \inpublicgroup QtTelephonyModule

    \brief The QMessageWaitingStatus class provides status information about waiting messages for a particular call class and/or telephone number.
    \ingroup telephony

    The unreadMessagesWaiting() method provides a simple boolean indication
    of whether or not there are unread messages.  This is typically used to
    update a simple indicator light on the device.  For some telephony
    services, this is the only information they can provide about waiting
    messages: the other values should be -1.

    Further details on the message waiting status indication is provided by
    unreadCount(), readCount(), urgentUnreadCount(), and urgentReadCount().  The
    unreadMessagesWaiting() state should be set to true if unreadCount() or
    urgentUnreadCount() are greater than zero to support simple client applications
    that do not need the full message waiting status information.

    \sa QMessageWaiting
*/

class QMessageWaitingStatusPrivate
{
public:
    QMessageWaitingStatusPrivate()
    {
        callClass = QTelephony::CallClassVoice;
        unreadMessagesWaiting = false;
        unreadCount = -1;
        readCount = -1;
        urgentUnreadCount = -1;
        urgentReadCount = -1;
    }
    QMessageWaitingStatusPrivate( QMessageWaitingStatusPrivate *d )
    {
        callClass = d->callClass;
        number = d->number;
        unreadMessagesWaiting = d->unreadMessagesWaiting;
        unreadCount = d->unreadCount;
        readCount = d->readCount;
        urgentUnreadCount = d->urgentUnreadCount;
        urgentReadCount = d->urgentReadCount;
    }

    QTelephony::CallClass callClass;
    QString number;
    bool unreadMessagesWaiting;
    int unreadCount;
    int readCount;
    int urgentUnreadCount;
    int urgentReadCount;
};

/*!
    Constructs an empty message waiting status indication object.
*/
QMessageWaitingStatus::QMessageWaitingStatus()
{
    d = new QMessageWaitingStatusPrivate();
}

/*!
    Constructs a copy of \a other.
*/
QMessageWaitingStatus::QMessageWaitingStatus( const QMessageWaitingStatus& other )
{
    d = new QMessageWaitingStatusPrivate( other.d );
}

/*!
    Destroys this message waiting status indication object.
*/
QMessageWaitingStatus::~QMessageWaitingStatus()
{
    delete d;
}

/*!
    Copies \a other into this object.
*/
QMessageWaitingStatus& QMessageWaitingStatus::operator=( const QMessageWaitingStatus& other )
{
    if ( d != other.d ) {
        delete d;
        d = new QMessageWaitingStatusPrivate( other.d );
    }
    return *this;
}

/*!
    Returns the call class for this message waiting status indication.
    The default value is QTelephony::CallClassVoice.

    \sa setCallClass(), number()
*/
QTelephony::CallClass QMessageWaitingStatus::callClass() const
{
    return d->callClass;
}

/*!
    Sets the call class for this message waiting status indication
    to \a value.

    \sa callClass(), number()
*/
void QMessageWaitingStatus::setCallClass( QTelephony::CallClass value )
{
    d->callClass = value;
}

/*!
    Returns the telephone number that applies to this message waiting
    status indication.  The default value is an empty string.

    If the telephony service uses URI's to identify users, then this
    will return the URI.  Telephone numbers and URI's may be distinguished
    by the presence of a colon (:) character.

    If the number is empty, then it indicates the user's default number
    for calls of the callClass().  Thus, it is possible for a telephony
    service to indicate the number of waiting voice, data, and fax
    messages, even if the user's number cannot be determined.

    \sa setNumber(), callClass()
*/
QString QMessageWaitingStatus::number() const
{
    return d->number;
}

/*!
    Sets the telephone number that applies to this message waiting
    status indication to \a value.

    \sa number(), callClass()
*/
void QMessageWaitingStatus::setNumber( const QString& value )
{
    d->number = value;
}

/*!
    Returns true if there are unread messages waiting on this telephony
    service for the specified callClass() and number().  This value is suitable
    for controlling a simple message waiting light on the device.
    The default value is false.

    The unreadCount(), readCount(), urgentUnreadCount() and urgentReadCount()
    methods provide more detailed information for telephony services
    that can detect the actual number of messages waiting.

    \sa callClass(), number(), unreadCount()
    \sa readCount(), urgentUnreadCount(), urgentReadCount()
*/
bool QMessageWaitingStatus::unreadMessagesWaiting() const
{
    return d->unreadMessagesWaiting;
}

/*!
    Sets the unread messages flag to \a value.  This value is suitable
    for controlling a simple message waiting light on the device.

    \sa unreadMessagesWaiting()
*/
void QMessageWaitingStatus::setUnreadMessagesWaiting( bool value )
{
    d->unreadMessagesWaiting = value;
}

/*!
    Returns the number of messages that have not been read yet.
    The default value is -1, indicating that the value cannot be
    detected on this type of telephony service.  The value will
    be zero if the value can be detected but there are no messages
    at present.

    \sa setUnreadCount()
*/
int QMessageWaitingStatus::unreadCount() const
{
    return d->unreadCount;
}

/*!
    Sets the number of messages that have not been read yet to \a value.

    \sa unreadCount()
*/
void QMessageWaitingStatus::setUnreadCount( int value )
{
    d->unreadCount = value;
}

/*!
    Returns the number of messages that have been read but are not
    yet deleted.  The default value is -1, indicating that the value
    cannot be detected on this type of telephony service.  The value will
    be zero if the value can be detected but there are no messages
    at present.

    \sa setReadCount()
*/
int QMessageWaitingStatus::readCount() const
{
    return d->readCount;
}

/*!
    Sets the number of messages that have been read but are not
    yet deleted to \a value.

    \sa readCount()
*/
void QMessageWaitingStatus::setReadCount( int value )
{
    d->readCount = value;
}

/*!
    Returns the number of urgent messages that have not been read yet.
    The default value is zero.

    \sa setUrgentUnreadCount()
*/
int QMessageWaitingStatus::urgentUnreadCount() const
{
    return d->urgentUnreadCount;
}

/*!
    Sets the number of urgent messages that have not been read yet to \a value.

    \sa urgentUnreadCount()
*/
void QMessageWaitingStatus::setUrgentUnreadCount( int value )
{
    d->urgentUnreadCount = value;
}

/*!
    Returns the number of urgent messages that have been read but
    not yet deleted.  The default value is -1, indicating that the
    value cannot be detected on this type of telephony service.
    The value will be zero if the value can be detected but there
    are no messages at present.

    \sa setUrgentReadCount()
*/
int QMessageWaitingStatus::urgentReadCount() const
{
    return d->urgentReadCount;
}

/*!
    Sets the number of urgent messages that have been read but
    not yet deleted to \a value.

    \sa urgentReadCount()
*/
void QMessageWaitingStatus::setUrgentReadCount( int value )
{
    d->urgentReadCount = value;
}

/*!
    \internal
    \fn void QMessageWaitingStatus::serialize(Stream &stream) const
*/
template <typename Stream> void QMessageWaitingStatus::serialize(Stream &stream) const
{
    stream << (int)(d->callClass);
    stream << d->number;
    stream << (int)(d->unreadMessagesWaiting);
    stream << d->unreadCount;
    stream << d->readCount;
    stream << d->urgentUnreadCount;
    stream << d->urgentReadCount;
}

/*!
    \internal
    \fn void QMessageWaitingStatus::deserialize(Stream &stream)
*/
template <typename Stream> void QMessageWaitingStatus::deserialize(Stream &stream)
{
    int value;
    stream >> value;
    d->callClass = (QTelephony::CallClass)value;
    stream >> d->number;
    stream >> value;
    d->unreadMessagesWaiting = (value != 0);
    stream >> d->unreadCount;
    stream >> d->readCount;
    stream >> d->urgentUnreadCount;
    stream >> d->urgentReadCount;
}

Q_IMPLEMENT_USER_METATYPE(QMessageWaitingStatus)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QMessageWaitingStatus>)
