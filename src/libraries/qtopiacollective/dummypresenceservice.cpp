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

#include "dummypresenceservice_p.h"
#include "dummypresencecontrol_p.h"

#include <qtopialog.h>

class DummyPresenceProvider : public QCollectivePresence
{
    Q_OBJECT

public:
    DummyPresenceProvider(const QMap<QString, QCollectivePresenceInfo::PresenceType> &presenceTypes,
                          const QCollectivePresenceInfo &local,
                          const QList<QCollectivePresenceInfo> &initialSubscribers,
                          DummyPresenceService *service);
    ~DummyPresenceProvider();

public slots:
    virtual void setLocalPresence(const QCollectivePresenceInfo &info);
    virtual void subscribePeer(const QString &uri);
    virtual void unsubscribePeer(const QString &uri);

    virtual void authorizePeer(const QString &uri);
    virtual void denyPeer(const QString &uri);

    virtual void blockPeer(const QString &uri);
    virtual void unblockPeer(const QString &uri);

    void subscribeRejected(const QString &uri);
    void subscribeAccepted(const QString &uri);

    void publishRequestArrived(const QString &uri);
    void presencesChanged(const QList<QCollectivePresenceInfo> &presences);

private:
    DummyPresenceService *m_service;
    friend class DummyPresenceService;
};

class DummyPresenceServicePrivate
{
public:
    DummyPresenceProvider *m_provider;
    DummyPresenceControlProvider *m_control;
};

DummyPresenceProvider::DummyPresenceProvider(
        const QMap<QString, QCollectivePresenceInfo::PresenceType> &presenceTypes,
        const QCollectivePresenceInfo &localUser,
        const QList<QCollectivePresenceInfo> &initialSubscribers,
        DummyPresenceService *service)
    : QCollectivePresence(service->groupName(), service, QAbstractIpcInterface::Server)

{
    setValue("Protocol", "TEST", Delayed);

    QMap<QString, QCollectivePresenceInfo::PresenceType>::const_iterator it =
            presenceTypes.constBegin();
    while (it != presenceTypes.constEnd()) {
        setValue(QString("StatusTypes/")+it.key(), static_cast<uint>(it.value()), Delayed);
        ++it;
    }

    setValue("LocalUri", localUser.uri(), Delayed);
    publishPeerInfo(localUser);
    emit localPresenceChanged();

    QStringList subscribedPeers;
    for (int i = 0; i < initialSubscribers.size(); ++i) {
        subscribedPeers.append(initialSubscribers[i].uri());
        publishPeerInfo(initialSubscribers[i]);
    }

    if (initialSubscribers.count() > 0) {
        setValue("SubscribedPeers", subscribedPeers);
        setValue("PublishedPeers", subscribedPeers);
        emit subscribedPeersChanged();
        emit publishedPeersChanged();
    }
}

DummyPresenceProvider::~DummyPresenceProvider()
{
    
}

void DummyPresenceProvider::setLocalPresence(const QCollectivePresenceInfo &info)
{
    publishPeerInfo(info);
    emit localPresenceChanged();
}

void DummyPresenceProvider::subscribePeer(const QString &uri)
{
    QStringList memberList = value("SubscribedPeers", QStringList()).toStringList();

    if (memberList.contains(uri))
        return;

    QStringList pendingList = value("PendingPeerSubscriptions", QStringList()).toStringList();
    if (pendingList.contains(uri))
        return;

    pendingList.append(uri);
    setValue("PendingPeerSubscriptions", pendingList);

    emit pendingPeerSubscriptionsChanged();
}

void DummyPresenceProvider::subscribeRejected(const QString &uri)
{
    QStringList pendingList = value("PendingPeerSubscriptions", QStringList()).toStringList();

    int pendingChanged = pendingList.removeAll(uri);

    if (pendingChanged) {
        setValue("PendingPeerSubscriptions", pendingList);
        emit pendingPeerSubscriptionsChanged();
    }
}

void DummyPresenceProvider::subscribeAccepted(const QString &uri)
{
    QStringList pendingList = value("PendingPeerSubscriptions", QStringList()).toStringList();
    int pendingChanged = pendingList.removeAll(uri);

    if (pendingChanged) {
        setValue("PendingPeerSubscriptions", pendingList);
        emit pendingPeerSubscriptionsChanged();

        QStringList memberList = value("SubscribedPeers", QStringList()).toStringList();
        memberList.append(uri);
        setValue("SubscribedPeers", memberList);
        emit subscribedPeersChanged();

        QStringList publishList = value("PublishedPeers", QStringList()).toStringList();
        publishList.append(uri);
        setValue("PublishedPeers", publishList);
        emit publishedPeersChanged();
    }
}

void DummyPresenceProvider::unsubscribePeer(const QString &uri)
{
    QStringList memberList = value("SubscribedPeers", QStringList()).toStringList();
    QStringList pendingList = value("PendingPeerSubscriptions", QStringList()).toStringList();
    QStringList publishList = value("PublishedPeers", QStringList()).toStringList();

    int subscribedChanged = memberList.removeAll(uri);
    int pendingChanged = pendingList.removeAll(uri);
    int publishedChanged = publishList.removeAll(uri);

    if (!subscribedChanged && !pendingChanged)
        return;

    if (subscribedChanged) {
        setValue("SubscribedPeers", memberList);
        emit subscribedPeersChanged();
    }

    if (publishedChanged) {
        setValue("PublishedPeers", publishList);
        emit publishedPeersChanged();
    }

    if (pendingChanged) {
        setValue("PendingPeerSubscriptions", pendingList);
        emit pendingPeerSubscriptionsChanged();
    }

    removePeerInfo(uri);
}

void DummyPresenceProvider::publishRequestArrived(const QString &uri)
{
    QStringList pendingList = value("PendingPublishRequests", QStringList()).toStringList();
    if (pendingList.contains(uri))
        return;

    pendingList.append(uri);
    setValue("PendingPublishRequests", pendingList);

    emit pendingPublishRequestsChanged();
}

void DummyPresenceProvider::authorizePeer(const QString &uri)
{
    QStringList pendingList = value("PendingPublishRequests", QStringList()).toStringList();
    int pendingChanged = pendingList.removeAll(uri);

    if (pendingChanged) {
        setValue("PendingPublishRequests", pendingList);
        emit pendingPublishRequestsChanged();

        QStringList memberList = value("SubscribedPeers", QStringList()).toStringList();
        memberList.append(uri);
        setValue("SubscribedPeers", memberList);
        emit subscribedPeersChanged();

        QStringList publishList = value("PublishedPeers", QStringList()).toStringList();
        publishList.append(uri);
        setValue("PublishedPeers", publishList);
        emit publishedPeersChanged();
    }
}

void DummyPresenceProvider::denyPeer(const QString &uri)
{
    QStringList pendingList = value("PendingPublishRequests", QStringList()).toStringList();
    int pendingChanged = pendingList.removeAll(uri);

    if (pendingChanged) {
        setValue("PendingPublishRequests", pendingList);
        emit pendingPublishRequestsChanged();
    }
}

void DummyPresenceProvider::blockPeer(const QString &uri)
{
    Q_UNUSED(uri)
}

void DummyPresenceProvider::unblockPeer(const QString &uri)
{
    Q_UNUSED(uri)
}

void DummyPresenceProvider::presencesChanged(const QList<QCollectivePresenceInfo> &presences)
{
    QStringList uris;

    for (int i = 0; i < presences.size(); i++) {
        publishPeerInfo(presences[i]);
        uris.append(presences[i].uri());
    }

    emit peerPresencesChanged(uris);
}

// Presence Control Provider

class DummyPresenceControlProvider : public DummyPresenceControl
{
    Q_OBJECT

public:
    DummyPresenceControlProvider(DummyPresenceService *service);
    ~DummyPresenceControlProvider();

public slots:
    virtual void updatePresences(const QList<QCollectivePresenceInfo> &presences);
    virtual void simulateIncomingPublishRequest(const QString &uri);
    virtual void simulateSubscribeAccept(const QString &uri);
    virtual void simulateSubscribeReject(const QString &uri);

private:
    DummyPresenceService *m_service;
    friend class DummyPresenceService;
};

DummyPresenceControlProvider::DummyPresenceControlProvider(
        DummyPresenceService *service)
        : DummyPresenceControl(service->groupName(), service, QAbstractIpcInterface::Server)
        , m_service(service)
{
}

DummyPresenceControlProvider::~DummyPresenceControlProvider()
{
}

void DummyPresenceControlProvider::updatePresences(const QList<QCollectivePresenceInfo> &presences)
{
    m_service->m_data->m_provider->presencesChanged(presences);
}

void DummyPresenceControlProvider::simulateIncomingPublishRequest(const QString &uri)
{
    m_service->m_data->m_provider->publishRequestArrived(uri);
}

void DummyPresenceControlProvider::simulateSubscribeAccept(const QString &uri)
{
    m_service->m_data->m_provider->subscribeAccepted(uri);
}

void DummyPresenceControlProvider::simulateSubscribeReject(const QString &uri)
{
    m_service->m_data->m_provider->subscribeRejected(uri);
}

// Dummy Presence Service

DummyPresenceService::DummyPresenceService(
        const QMap<QString, QCollectivePresenceInfo::PresenceType> &presenceTypes,
        const QCollectivePresenceInfo &localUser,
        const QList<QCollectivePresenceInfo> &initialSubscribers,
        const QString &name,
        QObject *parent)
    : QAbstractIpcInterfaceGroup(name, parent)
{
    m_data = new DummyPresenceServicePrivate;
    m_data->m_provider = new DummyPresenceProvider(presenceTypes,
            localUser, initialSubscribers, this);

    m_data->m_control = new DummyPresenceControlProvider(this);
}

DummyPresenceService::~DummyPresenceService()
{
    delete m_data->m_provider;
    delete m_data->m_control;
    delete m_data;
}

void DummyPresenceService::initialize()
{
    if (!supports<QCollectivePresence>())
        addInterface(m_data->m_provider);

    if (!supports<DummyPresenceControl>() )
        addInterface(m_data->m_control);

    QAbstractIpcInterfaceGroup::initialize();
}

#include "dummypresenceservice.moc"

