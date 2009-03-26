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

#include <qsmsreader.h>

/*!
    \class QSMSReader
    \inpublicgroup QtTelephonyModule

    \brief The QSMSReader class provides access to the incoming SMS message store on the modem device.

    \ingroup telephony

    The modem scans for new messages at start up and whenever a new message
    indication is received from the modem.  It will then update unreadCount(),
    unreadList(), usedMessages(), and totalMessages() to indicate the current
    state of the incoming SMS message store.

    The messageCount() signal is emitted when these values change, especially for
    new messages.  The unreadCountChanged() signal will be emitted whenever the
    number of unread messages changes, which could be when a new message arrives,
    or when the count is reset with setUnreadCount().

    SMS reading applications, such as \c qtmail, access the message store by
    calling check().  The QSMSReader class will respond with a messageCount()
    signal once the store is ready to be read.  The SMS reading application then
    calls firstMessage() to fetch the first message in the store.  The QSMSReader
    class will respond with a fetched() signal containing the message.  If the
    \c id parameter of fetched() is empty, then the list has terminated.
    Otherwise, the SMS reading application can call nextMessage() to read the
    next message; and so on.

    The deleteMessage() function can be used to delete a message from the
    incoming SMS message store.

    The setUnreadCount() function is used by the \c qpe server to reset
    unreadCount() after power up to the state it had before shutdown.
    It is also used by SMS reading applications like \c qtmail to reset
    the count to zero once all new messages have been processed.

    Some special SMS messages are not delivered via QSMSReader.  These are
    WAP push messages and SMS datagrams that are dispatched as soon as they
    arrive.  See QSMSMessage::destinationPort() for more information on how
    these types of messages are dispatched.

    \sa QSMSMessage
*/

/*!
    Construct a new SMS request object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SMS requests.  If there is more
    than one service that supports network registration, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QSMSReader objects for each.

    \sa QCommServiceManager::supports()
*/
QSMSReader::QSMSReader( const QString& service, QObject *parent,
                          QCommInterface::Mode mode )
    : QCommInterface( "QSMSReader", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this SMS request object.
*/
QSMSReader::~QSMSReader()
{
}

/*!
    Returns the number of messages that have not yet been read by the user.
    This value will gradually increase as new messages arrive until
    reset by a call to setUnreadCount().

    \sa unreadList(), setUnreadCount(), unreadCountChanged()
*/
int QSMSReader::unreadCount() const
{
    return value( "unreadCount", 0 ).toInt();
}

/*!
    Returns a list of message identifiers for messages that are new and unread
    since the last time firstMessage() was called.  This list will
    reset to empty when firstMessage() is called.  The list will be
    ordered roughly in the order in which the messages arrived.

    \sa unreadCount(), setUnreadCount(), unreadCountChanged()
*/
QStringList QSMSReader::unreadList() const
{
    return value( "unreadList" ).toStringList();
}

/*!
    Returns the number of messages that are currently stored on the SIM.
    Note that this may be greater than the value returned by the
    messageCount() signal.  The messageCount() signal reports the
    value after multi-part messages have been combined.  The unreadMessages()
    function returns the value before multi-part messages have been
    combined.  Returns -1 if the value is not currently known because
    the incoming message store has not yet been scanned.

    \sa totalMessages(), messageCount()
*/
int QSMSReader::usedMessages() const
{
    return value( "usedMessages", -1 ).toInt();
}

/*!
    Returns the total number of messages that could be stored on the SIM
    if all slots were in use.  If usedMessages() and totalMessages()
    are the same, then the SIM is full.  Returns -1 if the value
    is not currently known because the incoming message store has
    not yet been scanned.

    \sa usedMessages(), messageCount()
*/
int QSMSReader::totalMessages() const
{
    return value( "totalMessages", -1 ).toInt();
}

/*!
    Returns true if the SMS reader is ready to access the SIM; otherwise returns false.

    \sa setReady(), readyChanged()
*/
bool QSMSReader::ready() const
{
    return value( "ready" ).toBool();
}

/*!
    Fetch the total number of messages in the incoming SMS buffer.
    The number is returned via the messageCount() signal.

    \sa messageCount(), firstMessage()
*/
void QSMSReader::check()
{
    invoke( SLOT(check()) );
}

/*!
    Fetch the first message in the incoming SMS buffer.  Note: only one
    application (usually the mail application) should ever call this
    method and nextMessage().

    \sa nextMessage()
*/
void QSMSReader::firstMessage()
{
    invoke( SLOT(firstMessage()) );
}

/*!
    Fetch the next message in the incoming SMS buffer.

    \sa firstMessage()
*/
void QSMSReader::nextMessage()
{
    invoke( SLOT(nextMessage()) );
}

/*!
    Delete the message with the identifier \a id from the incoming SMS buffer.
*/
void QSMSReader::deleteMessage( const QString& id )
{
    invoke( SLOT(deleteMessage(QString)), id );
}

/*!
    Sets the number of unread messages to \a value.

    \sa unreadCount(), unreadList(), unreadCountChanged()
*/
void QSMSReader::setUnreadCount( int value )
{
    invoke( SLOT(setUnreadCount(int)), value );
}

/*!
    Sets the ready() state to \a value and emit the readyChanged() signal.

    \sa ready(), readyChanged()
*/
void QSMSReader::setReady( bool value )
{
    if ( value != ready() ) {
        setValue( "ready", value );
        emit readyChanged();
    }
}

/*!
    \fn void QSMSReader::unreadCountChanged()

    Signal that is emitted when unreadCount() changes.

    \sa unreadCount(), unreadList(), setUnreadCount()
*/

/*!
    \fn void QSMSReader::messageCount(int total)

    Signal that is emitted when either a new SMS message arrives,
    the \a total count is explicitly fetched using check(),
    or a message is deleted using deleteMessage().

    \sa check(), deleteMessage()
*/

/*!
    \fn void QSMSReader::fetched(const QString& id, const QSMSMessage& m)

    Signal that is emitted when a message in the incoming SMS buffer
    of is fetched.  \a id will be zero-length if there are no
    remaining messages in the buffer to be fetched.  Otherwise, the
    message content is in \a m.

    \sa firstMessage(), nextMessage()
*/

/*!
    \fn void QSMSReader::readyChanged()

    Signal that is emitted whenever ready() changes state.

    \sa ready(), setReady()
*/
