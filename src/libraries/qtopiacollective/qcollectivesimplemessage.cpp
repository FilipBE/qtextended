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

#include "qcollectivesimplemessage.h"

#include <QDateTime>

#include <qtopialog.h>

/*!
    \class QCollectiveSimpleMessage
    \inpublicgroup QtBaseModule

    \brief The QCollectiveSimpleMessage class holds a single message sent or received from a contact.

    This class holds a simple peer to peer message that was sent or received
    from a contact.  This information will be provided by the
    QCollectiveMessenger class.

    \sa QCollectiveMessenger

    \ingroup collective
*/

/*!
    \enum QCollectiveSimpleMessage::Type

    \value Normal The message is a normal message.
    \value AutoReply Message is an auto reply.
*/

class QCollectiveSimpleMessageData : public QSharedData
{
public:
    QCollectiveSimpleMessageData()
    {
    }

    QString m_from;
    QString m_to;
    QCollectiveSimpleMessage::Type m_type;
    QString m_text;
    QDateTime m_timestamp;
};

/*!
    Constructs a QCollectiveSimpleMessage object.

    \sa QCollectivePresence
*/
QCollectiveSimpleMessage::QCollectiveSimpleMessage()
{
    d = new QCollectiveSimpleMessageData;
}

/*!
    Deconstructs a QCollectiveSimpleMessage object.
*/
QCollectiveSimpleMessage::~QCollectiveSimpleMessage()
{
}

/*!
    Constructs a QCollectiveSimpleMessage object from the
    contents of \a other object.
*/
QCollectiveSimpleMessage::QCollectiveSimpleMessage(const QCollectiveSimpleMessage &other)
{
    d = other.d;
}

/*!
    Assigns the contents of \a other to this object.
*/
QCollectiveSimpleMessage &QCollectiveSimpleMessage::operator=(const QCollectiveSimpleMessage &other)
{
    if (this == &other)
        return *this;

    d = other.d;

    return *this;
}

/*!
    Returns the from URI for this message object.

    \sa setFrom()
*/
QString QCollectiveSimpleMessage::from() const
{
    return d->m_from;
}

/*!
    Sets the message's from URI to \a fromUri.

    \sa from()
*/
void QCollectiveSimpleMessage::setFrom(const QString &fromUri)
{
    d->m_from = fromUri;
}

/*!
    Returns the to URI for this message object.

    \sa setFrom()
*/
QString QCollectiveSimpleMessage::to() const
{
    return d->m_to;
}

/*!
    Sets the message's to URI to \a toUri.

    \sa from()
*/
void QCollectiveSimpleMessage::setTo(const QString &toUri)
{
    d->m_to = toUri;
}

/*!
    Returns the message type.

    \sa setType()
*/
QCollectiveSimpleMessage::Type QCollectiveSimpleMessage::type() const
{
    return d->m_type;
}

/*!
    Sets the message type to \a type.

    \sa type()
*/
void QCollectiveSimpleMessage::setType(QCollectiveSimpleMessage::Type type)
{
    d->m_type = type;
}

/*!
    Returns the text of this message.

    \sa setText()
*/
QString QCollectiveSimpleMessage::text() const
{
    return d->m_text;
}

/*!
    Sets the \a text of this message.

    \sa text()
*/
void QCollectiveSimpleMessage::setText(const QString &text)
{
    d->m_text = text;
}

/*!
    Returns the timestamp when this message was received or sent.

    \sa setTimestamp()
*/
QDateTime QCollectiveSimpleMessage::timestamp() const
{
    return d->m_timestamp;
}

/*!
    Sets the \a timestamp when this message was received or sent.
*/
void QCollectiveSimpleMessage::setTimestamp(const QDateTime &timestamp)
{
    d->m_timestamp = timestamp;
}

/*!
    Returns true if information in \a other is equal to information
    contained in this object; otherwise returns false.
*/
bool QCollectiveSimpleMessage::operator==( const QCollectiveSimpleMessage &other ) const
{
    return d->m_from == other.d->m_from &&
            d->m_to == other.d->m_to &&
            d->m_type == other.d->m_type &&
            d->m_text == other.d->m_text &&
            d->m_timestamp == other.d->m_timestamp;
}

/*!
    Returns true if information in \a other is not equal to information
    contained in this object; otherwise returns false.
*/
bool QCollectiveSimpleMessage::operator!=( const QCollectiveSimpleMessage &other ) const
{
    return d != other.d;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const QCollectiveSimpleMessage &info)
{
    debug << "QCollectiveSimpleMessage:(" << info.from() << "," << info.to()
            << "," << info.type() << "," << info.text()
            << "," << info.timestamp() << ")";
    return debug;
}
#endif

/*!
    \internal
    \fn void QCollectiveSimpleMessage::serialize(Stream &stream) const

    Serializes the presence account data into \a stream.
*/
template <typename Stream> void QCollectiveSimpleMessage::serialize(Stream &stream) const
{
    stream << d->m_from;
    stream << d->m_to;
    stream << static_cast<uint>(d->m_type);
    stream << d->m_text;
    stream << d->m_timestamp;
}

/*!
    \internal
    \fn void QCollectiveSimpleMessage::deserialize(Stream &stream)

    Deserialize the \a stream and updates this account.
*/
template <typename Stream> void QCollectiveSimpleMessage::deserialize(Stream &stream)
{
    stream >> d->m_from;
    stream >> d->m_to;
    uint type;
    stream >> type;
    d->m_type = static_cast<QCollectiveSimpleMessage::Type>(type);
    stream >> d->m_text;
    stream >> d->m_timestamp;
}

Q_IMPLEMENT_USER_METATYPE(QCollectiveSimpleMessage)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QCollectiveSimpleMessage>)
