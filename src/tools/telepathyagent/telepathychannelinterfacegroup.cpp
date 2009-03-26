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

#include "telepathychannelinterfacegroup.h"
#include "telepathychannel.h"
#include "telepathyconnection.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusError>
#include <QDBusArgument>

#include <QDebug>

#define CHANNEL_INTERFACE_GROUP "org.freedesktop.Telepathy.Channel.Interface.Group"

Q_DECLARE_METATYPE(QList<uint>)

class TelepathyChannelInterfaceGroupPrivate
{
public:
    QDBusInterface *m_iface;

    TelepathyChannelInterfaceGroupPrivate(const QString &service, const QDBusObjectPath &path,
                                         TelepathyChannelInterfaceGroup *parent);
    ~TelepathyChannelInterfaceGroupPrivate();
};

TelepathyChannelInterfaceGroupPrivate::TelepathyChannelInterfaceGroupPrivate(const QString &service,
         const QDBusObjectPath &path, TelepathyChannelInterfaceGroup *parent)
{
    QDBusConnection dbc = QDBusConnection::sessionBus();

    m_iface = new QDBusInterface(service, path.path(),
                                 CHANNEL_INTERFACE_GROUP,
                                 dbc);

    dbc.connect(service, path.path(), CHANNEL_INTERFACE_GROUP, "GroupFlagsChanged",
                parent, SLOT(groupFlagsChanged(uint,uint)));
    dbc.connect(service, path.path(), CHANNEL_INTERFACE_GROUP, "MembersChanged",
                parent, SLOT(membersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,uint)));
}

TelepathyChannelInterfaceGroupPrivate::~TelepathyChannelInterfaceGroupPrivate()
{
    delete m_iface;
}

/*!
    \class TelepathyChannelInterfaceGroup

    \brief Represents a Telepathy interface for channels with multiple members. org.freedesktop.Telepathy.Channel.Interface.Group
*/

/*!
    Constructs a TelepathyChannelInterfaceGroup with the conneciton \a service, on D-Bus object path \a path and parent \a parent.
*/
TelepathyChannelInterfaceGroup::TelepathyChannelInterfaceGroup(const QString &service,
        const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    m_data = new TelepathyChannelInterfaceGroupPrivate(service, path, this);
}

/*!
    Constructs a TelepathyChannelInterfaceGroup on the channel \a channel.
*/
TelepathyChannelInterfaceGroup::TelepathyChannelInterfaceGroup(TelepathyChannel *channel)
    : QObject(channel)
{
    m_data = new TelepathyChannelInterfaceGroupPrivate(channel->connection()->service(),
            channel->path(), this);
}

/*!
    Destructs TelepathyChannelInterfaceGroup
*/
TelepathyChannelInterfaceGroup::~TelepathyChannelInterfaceGroup()
{
    delete m_data;
}

/*!
    Adds a list of members \a contacts to the channel, with message \a message.
*/
bool TelepathyChannelInterfaceGroup::addMembers(const QList<uint> &contacts, const QString &message)
{
    QDBusReply<void> reply = m_data->m_iface->call("AddMembers",
            QVariant::fromValue(contacts), message);

    if (!reply.isValid())
        return false;

    return true;
}

/*!
    Removes a list of memebers /a contacts from the channel with message \a message, with the given reason of \a reason.
*/
bool TelepathyChannelInterfaceGroup::removeMembers(const QList<uint> &contacts,
        const QString &message, GroupChangeReason reason)
{
    QVariantList list;
    list.push_back(QVariant::fromValue(contacts));
    list.push_back(message);

    QString member;
    if (reason == None) {
        member = "RemoveMembers";
    } else {
        member = "RemoveMembersWithReason";
        list.push_back(static_cast<uint>(reason));
    }

    QDBusReply<void> reply = m_data->m_iface->callWithArgumentList(QDBus::BlockWithGui, member, list);

    if (!reply.isValid()) {
        qDebug() << "Unable to remove member:" << reply.error();
        return false;
    }

    return true;
}

/*!
    Returns handles to all members of this group.  The current
    subscribed members are provided in the list with key "current",
    local pending members are provided under key "localpending",
    and remote pending members are provided under key "remotepending."

*/
QMap<QString, QList<uint> > TelepathyChannelInterfaceGroup::allMembers() const
{
    QDBusReply<QDBusArgument> reply = m_data->m_iface->call("GetAllMembers");

    QMap<QString, QList<uint> > ret;

    if (!reply.isValid()) {
        return ret;
    }

    const QDBusArgument all = reply.value();
    QList<uint> m;

    all >> m;
    ret["current"] = m;
    all >> m;
    ret["localpending"] = m;
    all >> m;
    ret["remotepending"] = m;

    return ret;
}

/*!
    Returns a list of members that have a pending membership request.
*/
QList<uint> TelepathyChannelInterfaceGroup::localPendingMembers() const
{
    QDBusReply<QList<uint> > reply = m_data->m_iface->call("GetLocalPendingMembers");

    if (!reply.isValid()) {
        return QList<uint>();
    }

    return reply.value();
}

/*!
    Returns a list of all current, local and remote pending members.
*/
QList<TelepathyChannelInterfaceGroup::LocalPendingInfo>
         TelepathyChannelInterfaceGroup::localPendingMembersWithInfo() const
{
    QDBusReply<QDBusArgument> reply = m_data->m_iface->call("GetAllMembers");

    QList<LocalPendingInfo> ret;

    if (!reply.isValid()) {
        return ret;
    }

    const QDBusArgument info = reply.value();
    info.beginArray();
    while (!info.atEnd()) {
        LocalPendingInfo c;
        uint reason;

        info.beginStructure();
        info >> c.toBeAdded >> c.actor >> reason >> c.message;
        info.endStructure();

        c.reason = static_cast<GroupChangeReason>(reason);

        ret.push_back(c);
    }
    info.endArray();

    return ret;
}

/*!
    Returns a list of pending members that have been invited to the channel.
*/
QList<uint> TelepathyChannelInterfaceGroup::remotePendingMembers() const
{
    QDBusReply<QList<uint> > reply = m_data->m_iface->call("GetRemotePendingMembers");

    if (!reply.isValid()) {
        return QList<uint>();
    }

    return reply.value();
}

/*!
    Returns a list of members of the channel.
*/
QList<uint> TelepathyChannelInterfaceGroup::members() const
{
    QDBusReply<QList<uint> > reply = m_data->m_iface->call("GetMembers");

    if (!reply.isValid()) {
        return QList<uint>();
    }

    return reply.value();
}

/*!
    Returns the flags representing the operations permitted on the channel.
*/
TelepathyChannelInterfaceGroup::GroupFlags TelepathyChannelInterfaceGroup::groupFlags() const
{
    QDBusReply<uint> reply = m_data->m_iface->call("GetGroupFlags");
//depreciated use GetAll
    if (!reply.isValid()) {
        return 0;
    }

    return static_cast<TelepathyChannelInterfaceGroup::GroupFlags>(reply.value());
}

/*!
    Returns a list all the owners of the handles on this channel.
*/
QList<uint> TelepathyChannelInterfaceGroup::handleOwners(const QList<uint> &handles) const
{
    QDBusReply<QList<uint> > reply = m_data->m_iface->call("GetHandleOwners",
            QVariant::fromValue(handles));
//depreciated, use HandleOwners or HandleOwnersChanged signal
    if (!reply.isValid()) {
        return QList<uint>();
    }

    return reply.value();
}

/*!
    Returns the handle of the user of this channel.
*/
uint TelepathyChannelInterfaceGroup::selfHandle() const
{
    QDBusReply<uint> reply = m_data->m_iface->call("GetSelfHandle");
// depreciated, use GetAll
    if (!reply.isValid()) {
        return 0;
    }

    return reply.value();
}

/*!
    Signal that is emittd when contacts join member, local and rmeote pending lists.
*/
void TelepathyChannelInterfaceGroup::membersChanged(const QString &msg,
                        const QList<uint> &added,
                        const QList<uint> &removed,
                        const QList<uint> &localPending,
                        const QList<uint> &remotePending,
                        uint actor, uint reason)
{
    emit membersChanged(msg, added, removed,
                        localPending, remotePending, actor,
                        static_cast<GroupChangeReason>(reason));
}

/*!
    Signal that is emitted when the flags of operation for the channel are changed.
*/
void TelepathyChannelInterfaceGroup::groupFlagsChanged(uint added, uint removed)
{
    emit groupFlagsChanged(GroupFlags(added), GroupFlags(removed));
}
