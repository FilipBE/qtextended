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

#include "qmailid.h"

class MailIdPrivate : public QSharedData
{
public:
    MailIdPrivate():QSharedData(){};

    quint64 id;
};

Q_IMPLEMENT_USER_METATYPE(MailId);

MailId::MailId()
{
    d = new MailIdPrivate();
    d->id = 0;
}

MailId::MailId(quint64 value)
{
    d = new MailIdPrivate();
    d->id = value;  
}

MailId::MailId(const MailId& other)
{
    d = other.d;
}

MailId::~MailId()
{
}

MailId& MailId::operator=(const MailId& other) 
{
    d = other.d;
    return *this;
}

bool MailId::isValid() const
{
    return d->id != 0;
}

quint64 MailId::toULongLong() const
{
    return d->id;
}

bool MailId::operator!= (const MailId & other) const
{
    return d->id != other.d->id;
}

bool MailId::operator== (const MailId& other) const
{
    return d->id == other.d->id;
}

bool MailId::operator< (const MailId& other) const
{
    return d->id < other.d->id;
}

template <typename Stream> void MailId::serialize(Stream &stream) const
{
    stream << d->id;
}

template <typename Stream> void MailId::deserialize(Stream &stream)
{
    stream >> d->id;
}


QDebug& operator<< (QDebug& debug, const MailId &id)
{
    return debug << id.toULongLong();
}


/*!
    \class QMailAccountId
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule
    \ingroup messaginglibrary

    \preliminary
    \brief The QMailAccountId class is used to identify accounts stored by QMailStore.

    QMailAccountId is a class used to represent accounts stored by the QMailStore, identified
    by their unique numeric internal indentifer.

    A QMailAccountId instance can be tested for validity, and compared to other instances
    for equality.  The numeric value of the identifier is not intrinsically meaningful 
    and cannot be modified.
    
    \sa QMailAccount, QMailStore::account()
*/

/*!
    \typedef QMailAccountIdList
    \relates QMailAccountId
*/

Q_IMPLEMENT_USER_METATYPE(QMailAccountId);

/*! 
    Construct an uninitialized QMailAccountId, for which isValid() returns false.
*/
QMailAccountId::QMailAccountId()
    : MailId()
{
}

/*! 
    Construct a QMailAccountId with the supplied numeric identifier \a value.
*/
QMailAccountId::QMailAccountId(quint64 value)
    : MailId(value)
{
}

/*! \internal */
QMailAccountId::QMailAccountId(const QMailAccountId& other)
    : MailId(other)
{
}

/*! \internal */
QMailAccountId::~QMailAccountId()
{
}

/*! \internal */
QMailAccountId& QMailAccountId::operator=(const QMailAccountId& other) 
{
    MailId::operator=(other);
    return *this;
}

/*!
    Returns true if this object has been initialized with the identifier of an
    existing message contained by QMailStore.
*/
bool QMailAccountId::isValid() const
{
    return MailId::isValid();
}

/*! \internal */
quint64 QMailAccountId::toULongLong() const
{
    return MailId::toULongLong();
}

/*!
    Returns a QVariant containing the value of this account identfier.
*/
QMailAccountId::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    Returns true if this object's identifier value differs from that of \a other.
*/
bool QMailAccountId::operator!= (const QMailAccountId& other) const
{
    return MailId::operator!=(other);
}

/*!
    Returns true if this object's identifier value is equal to that of \a other.
*/
bool QMailAccountId::operator== (const QMailAccountId& other) const
{
    return MailId::operator==(other);
}

/*!
    Returns true if this object's identifier value is less than that of \a other.
*/
bool QMailAccountId::operator< (const QMailAccountId& other) const
{
    return MailId::operator<(other);
}

/*! 
    \fn QMailAccountId::serialize(Stream&) const
    \internal 
*/
template <typename Stream> void QMailAccountId::serialize(Stream &stream) const
{
    MailId::serialize(stream);
}

/*! 
    \fn QMailAccountId::deserialize(Stream&)
    \internal 
*/
template <typename Stream> void QMailAccountId::deserialize(Stream &stream)
{
    MailId::deserialize(stream);
}

/*! \internal */
QDebug& operator<< (QDebug& debug, const QMailAccountId &id)
{
    return debug << static_cast<const MailId&>(id);
}

Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QMailAccountIdList, QMailAccountIdList)


/*!
    \class QMailFolderId
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule
    \ingroup messaginglibrary

    \preliminary
    \brief The QMailFolderId class is used to identify folders stored by QMailStore.

    QMailFolderId is a class used to represent folders stored by the QMailStore, identified
    by their unique numeric internal indentifer.

    A QMailFolderId instance can be tested for validity, and compared to other instances
    for equality.  The numeric value of the identifier is not intrinsically meaningful 
    and cannot be modified.
    
    \sa QMailFolder, QMailStore::folder()
*/

/*!
    \typedef QMailFolderIdList
    \relates QMailFolderId
*/

Q_IMPLEMENT_USER_METATYPE(QMailFolderId);

/*! 
    Construct an uninitialized QMailFolderId, for which isValid() returns false.
*/
QMailFolderId::QMailFolderId()
    : MailId()
{
}

/*! 
    Construct a QMailFolderId with the supplied numeric identifier \a value.
*/
QMailFolderId::QMailFolderId(quint64 value)
    : MailId(value)
{
}

/*! \internal */
QMailFolderId::QMailFolderId(const QMailFolderId& other)
    : MailId(other)
{
}

/*! \internal */
QMailFolderId::~QMailFolderId()
{
}

/*! \internal */
QMailFolderId& QMailFolderId::operator=(const QMailFolderId& other) 
{
    MailId::operator=(other);
    return *this;
}

/*!
    Returns true if this object has been initialized with the identifier of an
    existing message contained by QMailStore.
*/
bool QMailFolderId::isValid() const
{
    return MailId::isValid();
}

/*! \internal */
quint64 QMailFolderId::toULongLong() const
{
    return MailId::toULongLong();
}

/*!
    Returns a QVariant containing the value of this folder identfier.
*/
QMailFolderId::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    Returns true if this object's identifier value differs from that of \a other.
*/
bool QMailFolderId::operator!= (const QMailFolderId& other) const
{
    return MailId::operator!=(other);
}

/*!
    Returns true if this object's identifier value is equal to that of \a other.
*/
bool QMailFolderId::operator== (const QMailFolderId& other) const
{
    return MailId::operator==(other);
}

/*!
    Returns true if this object's identifier value is less than that of \a other.
*/
bool QMailFolderId::operator< (const QMailFolderId& other) const
{
    return MailId::operator<(other);
}

/*! 
    \fn QMailFolderId::serialize(Stream&) const
    \internal 
*/
template <typename Stream> void QMailFolderId::serialize(Stream &stream) const
{
    MailId::serialize(stream);
}

/*! 
    \fn QMailFolderId::deserialize(Stream&)
    \internal 
*/
template <typename Stream> void QMailFolderId::deserialize(Stream &stream)
{
    MailId::deserialize(stream);
}

/*! \internal */
QDebug& operator<< (QDebug& debug, const QMailFolderId &id)
{
    return debug << static_cast<const MailId&>(id);
}

Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QMailFolderIdList, QMailFolderIdList)


/*!
    \class QMailMessageId
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule
    \ingroup messaginglibrary

    \preliminary
    \brief The QMailMessageId class is used to identify messages stored by QMailStore.

    QMailMessageId is a class used to represent messages stored by the QMailStore, identified
    by their unique numeric internal indentifer.

    A QMailMessageId instance can be tested for validity, and compared to other instances
    for equality.  The numeric value of the identifier is not intrinsically meaningful 
    and cannot be modified.
    
    \sa QMailMessage, QMailStore::message()
*/

/*!
    \typedef QMailMessageIdList
    \relates QMailMessageId
*/

Q_IMPLEMENT_USER_METATYPE(QMailMessageId);

/*! 
    Construct an uninitialized QMailMessageId, for which isValid() returns false.
*/
QMailMessageId::QMailMessageId()
    : MailId()
{
}

/*! 
    Construct a QMailMessageId with the supplied numeric identifier \a value.
*/
QMailMessageId::QMailMessageId(quint64 value)
    : MailId(value)
{
}

/*! \internal */
QMailMessageId::QMailMessageId(const QMailMessageId& other)
    : MailId(other)
{
}

/*! \internal */
QMailMessageId::~QMailMessageId()
{
}

/*! \internal */
QMailMessageId& QMailMessageId::operator=(const QMailMessageId& other) 
{
    MailId::operator=(other);
    return *this;
}

/*!
    Returns true if this object has been initialized with the identifier of an
    existing message contained by QMailStore.
*/
bool QMailMessageId::isValid() const
{
    return MailId::isValid();
}

/*! \internal */
quint64 QMailMessageId::toULongLong() const
{
    return MailId::toULongLong();
}

/*!
    Returns a QVariant containing the value of this message identfier.
*/
QMailMessageId::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    Returns true if this object's identifier value differs from that of \a other.
*/
bool QMailMessageId::operator!= (const QMailMessageId& other) const
{
    return MailId::operator!=(other);
}

/*!
    Returns true if this object's identifier value is equal to that of \a other.
*/
bool QMailMessageId::operator== (const QMailMessageId& other) const
{
    return MailId::operator==(other);
}

/*!
    Returns true if this object's identifier value is less than that of \a other.
*/
bool QMailMessageId::operator< (const QMailMessageId& other) const
{
    return MailId::operator<(other);
}

/*! 
    \fn QMailMessageId::serialize(Stream&) const
    \internal 
*/
template <typename Stream> void QMailMessageId::serialize(Stream &stream) const
{
    MailId::serialize(stream);
}

/*! 
    \fn QMailMessageId::deserialize(Stream&)
    \internal 
*/
template <typename Stream> void QMailMessageId::deserialize(Stream &stream)
{
    MailId::deserialize(stream);
}

/*! \internal */
QDebug& operator<< (QDebug& debug, const QMailMessageId &id)
{
    return debug << static_cast<const MailId&>(id);
}

Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QMailMessageIdList, QMailMessageIdList)

/*! \internal */
uint qHash(const QMailAccountId &id)
{
    return qHash(id.toULongLong());
}

/*! \internal */
uint qHash(const QMailFolderId &id)
{
    return qHash(id.toULongLong());
}

/*! \internal */
uint qHash(const QMailMessageId &id)
{
    return qHash(id.toULongLong());
}


