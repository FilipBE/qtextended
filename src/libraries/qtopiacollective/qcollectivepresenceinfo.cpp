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

#include "qcollectivepresenceinfo.h"

#include <qtopialog.h>

#include <QDateTime>
#include <QStringList>

/*!
    \class QCollectivePresenceInfo
    \inpublicgroup QtBaseModule

    \brief The QCollectivePresenceInfo class holds presence information for a contact.

    This class holds the presence information about a particular contact.  This
    information is reported by the QCollectivePresence provider implementation.
    For protocols that support multiple presence states or multiple presence resources,
    it is up to the presence provider implementation to report the collated
    presence information.  Presence provider implementations can choose to
    support custom fields in order to support more complex presence models.
    These fields can be set and accessed using the setProperties() and
    properties() methods.  The use of such fields is implementation dependent.

    \ingroup collective

    \sa QCollectivePresence
*/

class QCollectivePresenceInfoData : public QSharedData
{
public:
    QCollectivePresenceInfoData(const QString &uri) : m_uri(uri)
    {
    }

    QString m_uri;
    QString m_presence;
    QCollectivePresenceInfo::PresenceType m_type;
    QString m_displayName;
    QStringList m_capabilities;
    QString m_message;
    QString m_avatar;
    QDateTime m_lastUpdateTime;
    QVariantMap m_properties;
};

/*!
    Constructs a QCollectivePresenceInfo object for a
    presence account with \a uri.

    \sa QCollectivePresence
*/
QCollectivePresenceInfo::QCollectivePresenceInfo(const QString &uri)
{
    d = new QCollectivePresenceInfoData( uri );
}

/*!
    Creates a new QCollectivePresenceInfo object that is a copy of \a other.
*/
QCollectivePresenceInfo::QCollectivePresenceInfo(const QCollectivePresenceInfo &other)
{
    d = other.d;
}

/*!
    Returns true if uri() returns an empty string; otherwise returns false.
*/
bool QCollectivePresenceInfo::isNull() const
{
    return d->m_uri.isEmpty();
}

/*!
    Assigns \a other to this account and returns a reference to this presence account.
*/
QCollectivePresenceInfo &QCollectivePresenceInfo::operator=(const QCollectivePresenceInfo &other)
{
    if (this == &other)
        return *this;

    d = other.d;

    return *this;
}

/*!
    Destroys the presence acount object.
*/
QCollectivePresenceInfo::~QCollectivePresenceInfo()
{
}

/*!
    Returns the URI for this presence account object.

    An account with no URI info is considered to be null.

    \sa isNull()
*/
QString QCollectivePresenceInfo::uri() const
{
    return d->m_uri;
}

/*!
    Sets the URI for this presence account to \a uri.

    \sa uri()
*/
void QCollectivePresenceInfo::setUri(const QString &uri)
{
    d->m_uri = uri;
}

/*!
    Returns the presence state string associated with the uri.

    Different providers may have different standard strings
    associated with QCollectivePresenceInfo::PresenceType values.

    \sa setPresence()
*/
QString QCollectivePresenceInfo::presence() const
{
    return d->m_presence;
}

/*!
    Returns the presence state type associated with the uri.

    The presence state type is the standardized representation
    of the presence state, which may differ between different
    presence providers.

    \sa setPresence()
*/
QCollectivePresenceInfo::PresenceType QCollectivePresenceInfo::presenceType() const
{
    return d->m_type;
}

/*!
    Sets the \a presence state and corresponding \a type associated with the uri.

    Presence providers should perform the mapping from the presence provider
    specific presence state string to the standardized enumeration
    (\l QCollectivePresenceInfo::PresenceType).

    \sa presence()
*/
void QCollectivePresenceInfo::setPresence(const QString &presence, PresenceType type)
{
    d->m_presence = presence;
    d->m_type = type;
}

/*!
    Returns the display name for this presence account object.

    \sa setDisplayName()
*/
QString QCollectivePresenceInfo::displayName() const
{
    return d->m_displayName;
}

/*!
    Sets the display name of entity to \a displayName.

    \sa displayName()
*/
void QCollectivePresenceInfo::setDisplayName( const QString & displayName )
{
    d->m_displayName = displayName;
}

/*!
    Returns the capabilities of the entity.

    \sa setCapabilities()
*/
QStringList QCollectivePresenceInfo::capabilities() const
{
    return d->m_capabilities;
}

/*!
    Sets the \a capabilities of the entity.

    \sa capabilities()
*/
void QCollectivePresenceInfo::setCapabilities(const QStringList &capabilities)
{
    d->m_capabilities = capabilities;
}

/*!
    Returns the message from this entity.

    \sa setMessage()
*/
QString QCollectivePresenceInfo::message() const
{
    return d->m_message;
}

/*!
    Sets the \a message for this entity.

    \sa message()
*/

void QCollectivePresenceInfo::setMessage(const QString &message)
{
    d->m_message = message;
}

/*!
    Returns the avatar (buddy icon) filename from this entity.

    \sa setAvatar()
*/
QString QCollectivePresenceInfo::avatar() const
{
    return d->m_avatar;
}

/*!
    Sets the \a avatar (buddy icon) filename for this entity.

    \sa message()
*/

void QCollectivePresenceInfo::setAvatar(const QString &avatar)
{
    d->m_avatar = avatar;
}

/*!
    Returns the last updated time of this entity.

    \sa setLastUpdateTime()
*/
QDateTime QCollectivePresenceInfo::lastUpdateTime() const
{
    return d->m_lastUpdateTime;
}

/*!
    Sets the last updated time for this entity to \a lastUpdate.
*/
void QCollectivePresenceInfo::setLastUpdateTime(const QDateTime &lastUpdate)
{
    d->m_lastUpdateTime = lastUpdate;
}

/*!
    Returns the custom properties of this entity.  These
    properties might be reported by some implementations
    of QCollectivePresence and can be used by client
    applications that have a deeper knowledge of the
    specific provider.

    \sa setProperties()
*/
QVariantMap QCollectivePresenceInfo::properties() const
{
    return d->m_properties;
}

/*!
    Sets the custom \a properties of this entity.
*/
void QCollectivePresenceInfo::setProperties(const QVariantMap &properties)
{
    d->m_properties = properties;
}

/*!
    Returns true if information in \a other is equal to information
    contained in this object; otherwise returns false.

    Note that this does not compare the properties.
*/
bool QCollectivePresenceInfo::operator==( const QCollectivePresenceInfo &other ) const
{
    return d->m_uri == other.d->m_uri &&
            d->m_presence == other.d->m_presence &&
            d->m_displayName == other.d->m_displayName &&
            d->m_capabilities == other.d->m_capabilities &&
            d->m_message == other.d->m_message &&
            d->m_avatar == other.d->m_avatar &&
            d->m_lastUpdateTime == other.d->m_lastUpdateTime;
}

/*!
    Returns true if information in \a other is not equal to information
    contained in this object; otherwise returns false.

    Note that this does not compare the properties.
*/
bool QCollectivePresenceInfo::operator!=( const QCollectivePresenceInfo &other ) const
{
    return !operator==(other);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const QCollectivePresenceInfo &info)
{
    debug << "PresenceInfo:(" << info.uri() << "," << info.presence()
            << "," << info.displayName() << "," << info.capabilities()
            << "," << info.message() << "," << info.avatar()
            << "," << info.lastUpdateTime() << info.properties() << ")";
    return debug;
}
#endif

/*!
    \internal
    \fn void QCollectivePresenceInfo::serialize(Stream &stream) const

    Serializes the presence account data into \a stream.
*/
template <typename Stream> void QCollectivePresenceInfo::serialize(Stream &stream) const
{
    stream << d->m_uri;
    stream << d->m_presence;
    stream << d->m_displayName;
    stream << d->m_capabilities;
    stream << d->m_message;
    stream << d->m_avatar;
    stream << d->m_lastUpdateTime;
    stream << d->m_properties;
}

/*!
    \internal
    \fn void QCollectivePresenceInfo::deserialize(Stream &stream)

    Deserialize the \a stream and updates this account.
*/
template <typename Stream> void QCollectivePresenceInfo::deserialize(Stream &stream)
{
    stream >> d->m_uri;
    stream >> d->m_presence;
    stream >> d->m_displayName;
    stream >> d->m_capabilities;
    stream >> d->m_message;
    stream >> d->m_avatar;
    stream >> d->m_lastUpdateTime;
    stream >> d->m_properties;
}

Q_IMPLEMENT_USER_METATYPE(QCollectivePresenceInfo)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QCollectivePresenceInfo>)
Q_IMPLEMENT_USER_METATYPE_ENUM(QCollectivePresenceInfo::PresenceType)

/*!
    \enum QCollectivePresenceInfo::PresenceType

    This enum specifies the available presence state.

    \value None The presence state is not specified
    \value Offline The presence entity is offline
    \value Away The presence entity is away for a short time from the resource and cannot have instant communications
    \value Online The presence entity is online and available for communications
    \value ExtendedAway This presence state signifies that the party is away for an extended period of time
    \value Hidden The presence entity is hidden
    \value Busy The presence entity is busy and should not be contacted
*/

