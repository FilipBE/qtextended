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

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>
#include <qtopiacollective/private/dummypresenceservice_p.h>
#include <qtopiacollective/private/dummypresencecontrol_p.h>
#include <QCollectivePresenceInfo>
#include <QCollectivePresence>
#include <QValueSpace>
#include "qfuturesignal.h"

//TESTED_CLASS=DummyPresenceService
//TESTED_FILES=src/libraries/qtopiacollective/dummypresenceservice_p.h,src/libraries/qtopiacollective/dummypresenceservice.cpp

/*
    This class is a unit test for the DummyPresenceService class.
*/
class tst_DummyPresenceService : public QObject
{
    Q_OBJECT

signals:
    void localPresenceChanged();
    void peerPresencesChanged(const QStringList &presences);

    void pendingPublishRequestsChanged();

    void blockedPeersChanged();
    void subscribedPeersChanged();
    void publishedPeersChanged();
    void pendingPeerSubscriptionsChanged();

private slots:
    void initTestCase();

    void testInitialConditions();
    void testSubscribePeer();
    void testUnsubscribePeer();
    void testAuthorizePeer();
    void testDenyPeer();
    void testSetLocalPresence();
    void testPresenceUpdate();

private:
    QCollectivePresence *client;
    DummyPresenceControl *control;
    DummyPresenceService *service;
    QMap<QString, QCollectivePresenceInfo::PresenceType> map;
    QCollectivePresenceInfo localUser;
    QCollectivePresenceInfo remoteUser1;
    QCollectivePresenceInfo remoteUser2;
};

void tst_DummyPresenceService::initTestCase()
{
    QValueSpace::initValuespaceManager();

    map.insert("Offline", QCollectivePresenceInfo::Offline);
    map.insert("Online", QCollectivePresenceInfo::Online);
    map.insert("Idle", QCollectivePresenceInfo::Away);
    map.insert("Away", QCollectivePresenceInfo::Away);
    map.insert("Out to Lunch", QCollectivePresenceInfo::Away);
    map.insert("On the Phone", QCollectivePresenceInfo::Away);
    map.insert("Be Right Back", QCollectivePresenceInfo::Away);
    map.insert("Busy", QCollectivePresenceInfo::Busy);

    QStringList capabilities;
    capabilities.append("IM");

    localUser.setUri("user@local.host.net");
    localUser.setDisplayName("Local User");
    localUser.setPresence("Online", QCollectivePresenceInfo::Online);
    localUser.setCapabilities(capabilities);

    remoteUser1.setUri("remote.user.1@remote.host.net");
    remoteUser1.setDisplayName("Remote User #1");
    remoteUser1.setPresence("Away", QCollectivePresenceInfo::Away);
    remoteUser1.setCapabilities(capabilities);

    remoteUser2.setUri("remote.user.2@remote.host.net");
    remoteUser2.setDisplayName("Remote User #2");
    remoteUser2.setPresence("Online", QCollectivePresenceInfo::Online);
    remoteUser2.setCapabilities(capabilities);

    QList<QCollectivePresenceInfo> infos;
    infos.append(remoteUser1);
    infos.append(remoteUser2);

    service = new DummyPresenceService(map, localUser, infos, "test", this);

    QValueSpaceObject::sync();

    client = new QCollectivePresence("test", this);
    QVERIFY(client->available());

    control = new DummyPresenceControl("test", this);
    QVERIFY(control->available());

    connect(client, SIGNAL(localPresenceChanged()),
            this, SIGNAL(localPresenceChanged()));
    connect(client, SIGNAL(peerPresencesChanged(QStringList)),
            this, SIGNAL(peerPresencesChanged(QStringList)));
    connect(client, SIGNAL(pendingPublishRequestsChanged()),
            this, SIGNAL(pendingPublishRequestsChanged()));
    connect(client, SIGNAL(blockedPeersChanged()),
            this, SIGNAL(blockedPeersChanged()));
    connect(client, SIGNAL(subscribedPeersChanged()),
            this, SIGNAL(subscribedPeersChanged()));
    connect(client, SIGNAL(publishedPeersChanged()),
            this, SIGNAL(publishedPeersChanged()));
    connect(client, SIGNAL(pendingPeerSubscriptionsChanged()),
            this, SIGNAL(pendingPeerSubscriptionsChanged()));
}

void tst_DummyPresenceService::testInitialConditions()
{
    QVERIFY(client->statusTypes() == map);

    QStringList subscribedPeers = client->subscribedPeers();
    QVERIFY(subscribedPeers.contains("remote.user.1@remote.host.net"));
    QVERIFY(subscribedPeers.contains("remote.user.2@remote.host.net"));

    QStringList publishedPeers = client->publishedPeers();
    QVERIFY(publishedPeers.contains("remote.user.1@remote.host.net"));
    QVERIFY(publishedPeers.contains("remote.user.2@remote.host.net"));

    QVERIFY(client->pendingPublishRequests().count() == 0);
    QVERIFY(client->pendingPeerSubscriptions().count() == 0);
    QVERIFY(client->blockedPeers().count() == 0);

    QVERIFY(client->protocol() == "TEST");

    QCollectivePresenceInfo info = client->localInfo();
    QVERIFY(info.uri() == localUser.uri());
    QVERIFY(info.presence() == localUser.presence());
    QVERIFY(info.capabilities() == localUser.capabilities());

    info = client->peerInfo("remote.user.1@remote.host.net");
    QVERIFY(info.uri() == remoteUser1.uri());
    QVERIFY(info.presence() == remoteUser1.presence());
    QVERIFY(info.capabilities() == remoteUser1.capabilities());

    info = client->peerInfo("remote.user.2@remote.host.net");
    QVERIFY(info.uri() == remoteUser2.uri());
    QVERIFY(info.presence() == remoteUser2.presence());
    QVERIFY(info.capabilities() == remoteUser2.capabilities());
}

void tst_DummyPresenceService::testSubscribePeer()
{
    QFutureSignal pending(this, SIGNAL(pendingPeerSubscriptionsChanged()));
    QFutureSignal subscribed(this, SIGNAL(subscribedPeersChanged()));
    QFutureSignal published(this, SIGNAL(publishedPeersChanged()));

    client->subscribePeer("foo.bar@fub.ar.net");
    QVERIFY(pending.wait(100));
    QVERIFY(!client->subscribedPeers().contains("foo.bar@fub.ar.net"));
    QVERIFY(client->pendingPeerSubscriptions().contains("foo.bar@fub.ar.net"));

    control->simulateSubscribeAccept("foo.bar@fub.ar.net");
    QVERIFY(subscribed.wait(100));
    QVERIFY(published.wait(100));
    QVERIFY(!client->pendingPeerSubscriptions().contains("foo.bar@fub.ar.net"));
    QVERIFY(client->subscribedPeers().contains("foo.bar@fub.ar.net"));
    QVERIFY(client->publishedPeers().contains("foo.bar@fub.ar.net"));

    pending.clear();
    client->subscribePeer("bar.foo@bar.fu.net");
    QVERIFY(pending.wait(100));
    QVERIFY(!client->subscribedPeers().contains("bar.foo@bar.fu.net"));
    QVERIFY(client->pendingPeerSubscriptions().contains("bar.foo@bar.fu.net"));

    pending.clear();
    control->simulateSubscribeReject("bar.foo@bar.fu.net");
    QVERIFY(pending.wait(100));
    QVERIFY(!client->pendingPeerSubscriptions().contains("bar.foo@bar.fu.net"));
    QVERIFY(!client->subscribedPeers().contains("bar.foo@bar.fu.net"));
    QVERIFY(!client->publishedPeers().contains("bar.foo@bar.fu.net"));
}

void tst_DummyPresenceService::testUnsubscribePeer()
{
    QFutureSignal subscribed(this, SIGNAL(subscribedPeersChanged()));
    QFutureSignal published(this, SIGNAL(publishedPeersChanged()));
    QFutureSignal pending(this, SIGNAL(pendingPeerSubscriptionsChanged()));

    client->unsubscribePeer("foo.bar@fub.ar.net");
    QVERIFY(subscribed.wait(100));
    QVERIFY(published.wait(100));
    QVERIFY(!client->subscribedPeers().contains("foo.bar@fub.ar.net"));
    QVERIFY(!client->publishedPeers().contains("foo.bar@fub.ar.net"));

    subscribed.clear();
    published.clear();

    client->subscribePeer("foo.bar@fub.ar.net");
    QVERIFY(pending.wait(100));

    pending.clear();
    client->unsubscribePeer("foo.bar@fub.ar.net");
    QVERIFY(pending.wait(100));
}

void tst_DummyPresenceService::testAuthorizePeer()
{
    QFutureSignal subscribed(this, SIGNAL(subscribedPeersChanged()));
    QFutureSignal published(this, SIGNAL(publishedPeersChanged()));
    QFutureSignal pending(this, SIGNAL(pendingPublishRequestsChanged()));

    control->simulateIncomingPublishRequest("foobar@foo.com");
    QVERIFY(pending.wait(100));
    QVERIFY(client->pendingPublishRequests().contains("foobar@foo.com"));

    pending.clear();
    client->authorizePeer("foobar@foo.com");
    QVERIFY(pending.wait(100));
    QVERIFY(published.wait(100));
    QVERIFY(subscribed.wait(100));
    QVERIFY(client->subscribedPeers().contains("foobar@foo.com"));
    QVERIFY(client->publishedPeers().contains("foobar@foo.com"));
    QVERIFY(!client->pendingPublishRequests().contains("foobar@foo.com"));

    pending.clear();
    published.clear();
    subscribed.clear();

    client->unsubscribePeer("foobar@foo.com");
    QVERIFY(published.wait(100));
    QVERIFY(subscribed.wait(100));
    QVERIFY(!client->subscribedPeers().contains("foobar@foo.com"));
    QVERIFY(!client->publishedPeers().contains("foobar@foo.com"));
}

void tst_DummyPresenceService::testDenyPeer()
{
    QFutureSignal subscribed(this, SIGNAL(subscribedPeersChanged()));
    QFutureSignal published(this, SIGNAL(publishedPeersChanged()));
    QFutureSignal pending(this, SIGNAL(pendingPublishRequestsChanged()));

    control->simulateIncomingPublishRequest("foobar@foo.com");
    QVERIFY(pending.wait(100));
    QVERIFY(client->pendingPublishRequests().contains("foobar@foo.com"));

    pending.clear();
    client->denyPeer("foobar@foo.com");
    QVERIFY(pending.wait(100));
    QVERIFY(!client->subscribedPeers().contains("foobar@foo.com"));
    QVERIFY(!client->publishedPeers().contains("foobar@foo.com"));
    QVERIFY(!client->pendingPublishRequests().contains("foobar@foo.com"));
}

void tst_DummyPresenceService::testSetLocalPresence()
{
    QFutureSignal local(this, SIGNAL(localPresenceChanged()));

    QCollectivePresenceInfo info = client->localInfo();
    info.setPresence("Away", QCollectivePresenceInfo::Away);

    client->setLocalPresence(info);
    QVERIFY(local.wait(100));
    QVERIFY(client->localInfo().presence() == "Away");
}

void tst_DummyPresenceService::testPresenceUpdate()
{
    QFutureSignal presenceUpdate(this, SIGNAL(peerPresencesChanged(QStringList)));

    remoteUser1.setPresence("Out to Lunch", QCollectivePresenceInfo::ExtendedAway);
    remoteUser2.setPresence("Be Right Back", QCollectivePresenceInfo::Away);

    QList<QCollectivePresenceInfo> infos;
    infos.append(remoteUser1);
    infos.append(remoteUser2);
    control->updatePresences(infos);

    QVERIFY(presenceUpdate.wait(100));
    QCOMPARE(presenceUpdate.resultCount(), 1);

    QStringList urisChanged = presenceUpdate.results()[0][0].toStringList();
    QVERIFY(urisChanged.count() == 2);
    QVERIFY(urisChanged.contains(remoteUser1.uri()));
    QVERIFY(urisChanged.contains(remoteUser2.uri()));

    QCollectivePresenceInfo info = client->peerInfo(remoteUser1.uri());
    QVERIFY(info.presence() == "Out to Lunch");
    info = client->peerInfo(remoteUser2.uri());
    QVERIFY(info.presence() == "Be Right Back");
}

QTEST_APP_MAIN( tst_DummyPresenceService, QtopiaApplication )
#include "tst_dummypresenceservice.moc"
