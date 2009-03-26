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

#ifndef TELEPATHYCHANNELINTERFACEGROUP_H
#define TELEPATHYCHANNELINTERFACEGROUP_H

#include <QObject>
#include <QString>
#include <QMap>

class TelepathyChannelInterfaceGroupPrivate;
class TelepathyChannel;
class QDBusObjectPath;

class TelepathyChannelInterfaceGroup : public QObject
{
    Q_OBJECT

public:
    enum GroupFlag {
        CanAddFlag = 1,
        CanRemoveFlag = 2,
        CanRescindFlag = 4,
        MessageAddFlag = 8,
        MessageRemoveFlag = 16,
        MessageAcceptFlag = 32,
        MessageRejectFlag = 64,
        MessageRescindFlag = 128,
        ChannelSpecificHandlesFlag = 256,
        OnlyOneGroupFlag = 512,
        HandleOwnersNotAvailableFlag = 1024
    };

    Q_DECLARE_FLAGS(GroupFlags, GroupFlag)

    enum GroupChangeReason {
        None = 0,
        Offline = 1,
        Kicked = 2,
        Busy = 3,
        Invited = 4,
        Banned = 5,
        Error = 6,
        InvalidContact = 7,
        NoAnswer = 8,
        Renamed = 9,
        PermissionDenied = 10,
        Separated = 11,
    };

    struct LocalPendingInfo {
        uint toBeAdded;
        uint actor;
        GroupChangeReason reason;
        QString message;
    };

    TelepathyChannelInterfaceGroup(TelepathyChannel *channel);
    TelepathyChannelInterfaceGroup(const QString &service, const QDBusObjectPath &path,
                                   QObject *parent = 0);
    ~TelepathyChannelInterfaceGroup();

    bool addMembers(const QList<uint> &contacts, const QString &message);
    bool removeMembers(const QList<uint> &contacts, const QString &message,
                       GroupChangeReason reason = None);

    QMap<QString, QList<uint> > allMembers() const;
    QList<uint> localPendingMembers() const;
    QList<LocalPendingInfo> localPendingMembersWithInfo() const;
    QList<uint> remotePendingMembers() const;
    QList<uint> members() const;

    TelepathyChannelInterfaceGroup::GroupFlags groupFlags() const;
    QList<uint> handleOwners(const QList<uint> &handles) const;

    uint selfHandle() const;

signals:
    void groupFlagsChanged(GroupFlags added, GroupFlags removed);
    void membersChanged(const QString &message,
                        const QList<uint> &added,
                        const QList<uint> &removed,
                        const QList<uint> &localPending,
                        const QList<uint> &remotePending,
                        uint actor, TelepathyChannelInterfaceGroup::GroupChangeReason reason);
private slots:
    void membersChanged(const QString &msg,
                        const QList<uint> &added,
                        const QList<uint> &removed,
                        const QList<uint> &localPending,
                        const QList<uint> &remotePending,
                        uint actor, uint reason);

    void groupFlagsChanged(uint added, uint removed);

private:
    Q_DISABLE_COPY(TelepathyChannelInterfaceGroup)
    TelepathyChannelInterfaceGroupPrivate *m_data;
};

#endif
