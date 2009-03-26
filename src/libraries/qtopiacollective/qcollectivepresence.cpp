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

#include "qcollectivepresence.h"

#include <QValueSpaceObject>
#include <QValueSpaceItem>
#include <QDateTime>
#include <QDebug>

/*!
    \class QCollectivePresence
    \inpublicgroup QtBaseModule

    \brief The QCollectivePresence class defines a generic interface to presence information.

    The QCollectivePresence class inherits QAbstractIpcInterface which provides
    client/server communications framework.

    The QCollectivePresence in Server mode parses the presence data
    received from the network and informs the rest of the system
    upon changes to the peers' presence for a certain type of service.

    For example, to implement the server mode provider that provides
    presence interface in "voip" service, create a class that inherits
    QCollectivePresence class and override public slots.

    \code
    class PresenceProvider : public QCollectivePresence
    {
        Q_OBJECT
    public:
        PresenceProvider( QObject *parent=0 )
            : QCollectivePresence( "voip", parent, Server )
        {
            ...
        }
    public slots:
        virtual void subscribePeer( const QString &uri )
        {
            ...
        }
        ...
    };
    \endcode

    The QCollectivePresence in Client mode is used by client applications to
    access the presence data for both local and remote
    peers for the type of service that the server mode provides.

    To start monitoring a list of peers from a client application, for example:

    \code
    QCollectivePresence *provider = new QCollectivePresence( "voip" );
    foreach ( QString id, voipIds ) {
        provider->subscribePeer(id);
    }
    \endcode

    \ingroup collective
*/

class QCollectivePresencePrivate
{
};

/*!
    Constructs a QCollectivePresence object for \a service and attaches it to \a parent.
    The object will be created in client mode if \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports presence.  If there is more than one
    service that supports presence, the caller should enumerate
    them with QCommServiceManager::supports() and create separate
    QCollectivePresence objects for each.

    \sa QCommServiceManager::supports()
*/
QCollectivePresence::QCollectivePresence(const QString &service, QObject *parent,
                                         QAbstractIpcInterface::Mode mode)
                : QAbstractIpcInterface( "/Communications", "QCollectivePresence",
                                 service, parent, mode )
{
    proxyAll(staticMetaObject);
}

/*!
    Destroys the QCollectivePresence object.
*/
QCollectivePresence::~QCollectivePresence()
{
}

/*!
    \property QCollectivePresence::protocol
    \brief the name of the protocol supported by this presence service.

    The server implentation needs to make sure to set
    the protocol name in the constructor using QAbstractIpcInterface::setValue() function
    with the key "protocol".

    \code
    PresenceProviderServer::PresenceProviderServer
        ( const QString &service, QObject *parent, QAbstractIpcInterface::Mode mode )
        : QCollectivePresence( service, parent, Server )
    {
        ...
        setValue( "Protocol", "SIP" );
        ...
    }
    \endcode
*/
QString QCollectivePresence::protocol() const
{
    return value("Protocol", QString()).toString();
}

/*!
    Returns the presence information currently set for this service
    for the local user.

    \sa localPresenceChanged()
*/
QCollectivePresenceInfo QCollectivePresence::localInfo() const
{
    QString uri = value("LocalUri", QString()).toString();
    return readPeerInfo(uri);
}

/*!
    Returns the presence information for a remote peer given by \a uri.

    \sa peerPresencesChanged()
*/
QCollectivePresenceInfo QCollectivePresence::peerInfo(const QString &uri) const
{
    return readPeerInfo(uri);
}

/*!
    Returns a dictionary of supported presence statuses mapped to
    a presence type.  This can be used by client applications to
    display the appropriate icon for the specific status.

    Please note that the "Offline" state is not settable on the 
    local account.

    \sa setLocalPresence()
*/
QMap<QString, QCollectivePresenceInfo::PresenceType> QCollectivePresence::statusTypes() const
{
    QString statusPath("StatusTypes");
    QStringList statusStrings = valueNames(statusPath);

    QMap<QString, QCollectivePresenceInfo::PresenceType> ret;

    foreach (QString status, statusStrings) {
        QString vsPath = statusPath + "/" + status;
        QCollectivePresenceInfo::PresenceType type =
                static_cast<QCollectivePresenceInfo::PresenceType>(value(vsPath).toUInt());
        ret.insert(status, type);
    }

    return ret;
}

/*!
    Returns a list of peer uris which have sent a subscribe request
    for local presence information, but which haven't been rescinded,
    denied using denyPeer() or accepted using authorizePeer().  Once
    the request is accepted using authorizePeer() the local presence
    information will be published to the remote peer.

    Only protocols which support publish lists will implement
    this method.

    \sa pendingPublishRequestsChanged()
*/
QStringList QCollectivePresence::pendingPublishRequests() const
{
    return value("PendingPublishRequests", QStringList()).toStringList();
}

/*!
    Returns a list of peer uris which have been sent a subscription
    request by the local client, but where the remote peer has not
    authorized to publish its presence to the local peer.

    \sa pendingPeerSubscriptionsChanged()
*/
QStringList QCollectivePresence::pendingPeerSubscriptions() const
{
    return value("PendingPeerSubscriptions", QStringList()).toStringList();
}

/*!
    Returns a list of peers which are temporarily blocked from
    receiving local presence information.

    \sa blockPeer(), unblockPeer(), blockedPeersChanged()
*/
QStringList QCollectivePresence::blockedPeers() const
{
    return value("BlockedPeers", QStringList()).toStringList();
}

/*!
    Returns a list of peer uris for which presence
    information is available.

    \sa subscribePeer(), unsubscribePeer(), subscribedPeersChanged()
*/
QStringList QCollectivePresence::subscribedPeers() const
{
    return value("SubscribedPeers", QStringList()).toStringList();
}

/*!
    Returns a list of peer uris who receive local
    presence information.

    \sa authorizePeer(), denyPeer(), publishedPeersChanged()
*/
QStringList QCollectivePresence::publishedPeers() const
{
    return value("PublishedPeers", QStringList()).toStringList();
}

/*!
    Attempts to set the local presence information published
    by this service to \a info.  Note that the avatar field
    will be ignored when setting the presence information.
    To set the avatar, use the regular PIM mechanisms.

    The localPresenceChanged() signal will be emitted once the
    local presence information has been updated.
*/
void QCollectivePresence::setLocalPresence(const QCollectivePresenceInfo &info)
{
    invoke(SLOT(setLocalPresence(QCollectivePresenceInfo)), QVariant::fromValue(info));
}

/*!
    Attempts to subscribe to presence information of
    peer \a uri.  The subscribedPeersChanged() signal
    will be emitted whenever the list changes.  Client
    applications should monitor this list to find out
    when the desired peer uri has been added to the
    subscribed list.  From then on the presence information
    will be available using peerInfo().

    \sa unsubscribePeer()
*/
void QCollectivePresence::subscribePeer(const QString &uri)
{
    invoke(SLOT(subscribePeer(QString)), uri );
}

/*!
    Attempts to unsubscribe from presence information
    published by peer \a uri.

    \sa subscribePeer()
*/
void QCollectivePresence::unsubscribePeer(const QString &uri)
{
    invoke(SLOT(unsubscribePeer(QString)), uri );
}

/*!
    Authorizes the pending presence subscription request
    from remote peer \a uri.  If there is no pending
    request from \a uri, then this method has no effect.

    The publishedPeersChanged() signal will be emitted once
    the service adds the \a uri to the publish list.

    \sa denyPeer()
*/
void QCollectivePresence::authorizePeer(const QString &uri)
{
    invoke(SLOT(authorizePeer(QString)), uri );
}

/*!
    Denies the pending presence subscription request
    from remote peer \a uri.  If there is no pending
    request from \a uri, this method has no effect.

    \sa authorizePeer()
*/
void QCollectivePresence::denyPeer(const QString &uri)
{
    invoke(SLOT(denyPeer(QString)), uri);
}

/*!
    Attempts to temporarily block a peer \a uri from receiving
    local presence information.  Some protocol implementations
    might not support this functionality.

    The blockedPeersChanged() signal will be emitted once
    the peer has been blocked.

    \sa blockedPeers(), blockedPeersChanged(), unblockPeer()
*/
void QCollectivePresence::blockPeer(const QString &uri)
{
    invoke(SLOT(blockPeer(QString)), uri );
}

/*!
    Attempts to remove a peer \a uri from the blocked peers list.
    Attempts to unblock peers which are not blocked will be
    ignored.

    The blockedPeersChanged() signal will be emitted once
    the peer has been unblocked.

    \sa blockedPeers(), blockPeer(), blockedPeersChanged()
*/
void QCollectivePresence::unblockPeer(const QString &uri)
{
    invoke(SLOT(unblockPeer(QString)), uri );
}

/*!
    \fn void QCollectivePresence::localPresenceChanged()

    This signal is emitted whenever the local user's presence has been
    changed.

    \sa localInfo()
*/

/*!
    \fn void QCollectivePresence::peerPresencesChanged(const QStringList &presences)

    This signal is emitted whenever the presence service receives
    notification of a change in presence status.  The uris of the
    members with changed presence status is given in \a presences.

    \sa peerInfo()
*/

/*!
    \fn void QCollectivePresence::pendingPublishRequestsChanged()

    This signal is emitted whenever a subscription request has
    been received. The user can accept this subscription by using the
    authorizePeer() method, in which case the user's presence information
    will be published to the remote peer.  The publishedPeersChanged()
    signal will be emitted once the peer has been added to the publish
    list successfully.

    The user can deny the subscription request by using the
    denyPeer() method.  This will refuse the subscription request.

    For protocols that do not use user-confirmed subscriptions, this
    signal is not used.

    \sa denyPeer(), authorizePeer()
*/

/*!
    \fn void QCollectivePresence::blockedPeersChanged()

    This signal is emitted whenever a peer has been added or removed
    from the blocked peers list.  Blocked peers will temporarily
    not receive any presence information.
*/

/*!
    \fn void QCollectivePresence::subscribedPeersChanged()

    This signal is emitted whenever a new peer has been added to the
    subscribe list (e.g. a remote user has accepted a subscribe request).

    \sa subscribePeer(), unsubscribePeer()
*/

/*!
    \fn void QCollectivePresence::publishedPeersChanged()

    This signal is emitted whenever the list of peers who receive local
    presence information has changed. This usually happens when
    authorizePeer() method has added the remote peer to
    the publish list successfully.

    \sa authorizePeer(), denyPeer(), pendingPublishRequestsChanged()
*/

/*!
    \fn void QCollectivePresence::pendingPeerSubscriptionsChanged()

    This signal is emitted whenever a peer has been sent a subscribe
    request, but the request has not been accepted, denied or rescinded.

    \sa subscribePeer()
*/

QCollectivePresenceInfo QCollectivePresence::readPeerInfo(const QString &uri) const
{
    QString vsPath = "/Communications/QCollectivePresence/" + groupName() + "/PresenceInfo/" + uri;
    QValueSpaceItem vsItem(vsPath);

    QStringList subPaths = vsItem.subPaths();
    if (!subPaths.count()) {
        return QCollectivePresenceInfo();
    }

    QCollectivePresenceInfo ret(uri);
    QVariantMap properties;

    foreach ( QString str, vsItem.subPaths() ) {
        if (str == "Presence") {
            QString status = vsItem.value("Presence", QString()).toString();
            QCollectivePresenceInfo::PresenceType type =
                static_cast<QCollectivePresenceInfo::PresenceType>(value("StatusTypes/" + status, QCollectivePresenceInfo::None).toUInt());
            ret.setPresence(status, type);
        } else if (str == "DisplayName")
            ret.setDisplayName(vsItem.value("DisplayName", QString()).toString());
        else if (str == "Capabilities")
            ret.setCapabilities(vsItem.value("Capabilities", QStringList()).toStringList());
        else if (str == "Message")
            ret.setMessage(vsItem.value("Message", QString()).toString());
        else if (str == "Avatar")
            ret.setAvatar(vsItem.value("Avatar", QString()).toString());
        else if (str == "LastUpdateTime") {
            QDateTime dt;
            QString strTime = vsItem.value("LastUpdateTime", QString()).toString();
            if (!strTime.isEmpty()) {
                QDateTime::fromString(strTime, Qt::ISODate);
                if (dt.isValid())
                    dt.setTimeSpec(Qt::UTC);
            }
            ret.setLastUpdateTime(dt);
        } else
            properties.insert(str, vsItem.value(str));
    }

    ret.setProperties(properties);

    return ret;
}

/*!
    This is a convenience method designed to be used by presence
    service implementations in order to publish peer presence \a info
    so that it can be accessed by Client mode QCollectivePresence objects.
*/
void QCollectivePresence::publishPeerInfo(const QCollectivePresenceInfo &info)
{
    QString vsPath = "PresenceInfo/" + info.uri();

    setValue(vsPath + "/Presence", info.presence(), Delayed);
    setValue(vsPath + "/DisplayName", info.displayName(), Delayed);
    setValue(vsPath + "/Capabilities", info.capabilities(), Delayed);
    setValue(vsPath + "/Message", info.message(), Delayed);
    setValue(vsPath + "/Avatar", info.avatar(), Delayed);
    setValue(vsPath + "/LastUpdateTime", info.lastUpdateTime().toUTC().toString(Qt::ISODate), Delayed);

    QVariantMap properties = info.properties();
    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        setValue(vsPath + "/" + it.key(), it.value(), Delayed);
        ++it;
    }

    QValueSpaceObject::sync();
}

/*!
    Removes the peer info with \a uri from storage.  The current
    implementation removes the storage value from ValueSpace by
    using removeValue() function.
*/
void QCollectivePresence::removePeerInfo(const QString &uri)
{
    removeValue("PresenceInfo/" + uri);
}
