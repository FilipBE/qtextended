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

#include "service.h"

#include <qtopiaapplication.h>
#include <QCollectivePresence>
#include <QCollectivePresenceInfo>
#include <QCollectiveMessenger>
#include <QCollectiveSimpleMessage>
#include <QNetworkRegistrationServer>
#include <QTelephonyConfiguration>
#include <QServiceChecker>
#include <QTimer>
#include <QVarLengthArray>
#include <QQueue>
#include <QLinkedList>
#include <QtopiaServiceRequest>
#include <qtopianamespace.h>
#include <QSet>
#include <QContactModel>
#include <QContact>
#include <QFileInfo>
#include <QImage>

#include "telepathyconnection.h"
#include "telepathyconnectionmanager.h"
#include "telepathyconnectioninterfacepresence.h"
#include "telepathyconnectioninterfacealiasing.h"
#include "telepathyconnectioninterfaceavatars.h"
#include "telepathychannelinterfacegroup.h"
#include "telepathychannel.h"
#include "telepathyconnectioninterfacecapabilities.h"
#include "telepathychanneltypetext.h"

#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

class TelepathyLayer : public QObject
{
    Q_OBJECT

public:
    TelepathyLayer(TelepathyTelephonyService *service, TelepathyConnection *conn);
    ~TelepathyLayer();

    QString lookupUri(uint id);
    void clearAvatar();
    void storeToken(uint contact, const QString &token,
                    const TelepathyConnectionInterfaceAvatars::AvatarData &data);
    void removeToken(uint contact);
    QString avatarFileName(uint contact) const;

    QString m_selfUri;
    uint m_selfHandle;
    QMap<QString, uint> m_uriToIdMap;
    QMap<uint, QString> m_idToUriMap;
    TelepathyConnection *m_conn;
    TelepathyChannelInterfaceGroup *m_subscribeChannel;
    TelepathyChannelInterfaceGroup *m_publishChannel;
    TelepathyChannelInterfaceGroup *m_denyChannel;
    QContactModel *m_contactModel;
    QSettings *m_avatarMap;
    TelepathyTelephonyService *m_service;

private slots:
    void subscribeChannelReady(const QDBusObjectPath &);
    void publishChannelReady(const QDBusObjectPath &);
    void denyChannelReady(const QDBusObjectPath &);

signals:
    void subscribeChannelReady();
    void publishChannelReady();
    void denyChannelReady();
};

class MessageSender;
class TelepathyMessengerProvider : public QCollectiveMessenger
{
    Q_OBJECT

public:
    TelepathyMessengerProvider(TelepathyTelephonyService *service);
    ~TelepathyMessengerProvider();

public slots:
    void sendMessage(const QCollectiveSimpleMessage &message);
    void registerIncomingHandler(const QString &service);
    void unregisterIncomingHandler(const QString &service);

private slots:
    void channelCreated(const TelepathyConnection::ChannelInfo &info, bool suppressHandler);
    void destroyed();

private:
    void incomingMessages(const QList<QCollectiveSimpleMessage> &messages);
    void sendPendingMessages();

private:
    friend class MessageSender;
    TelepathyTelephonyService *m_service;
    QMap<QString, MessageSender *> m_senders;
    QLinkedList<QCollectiveSimpleMessage> m_pending;
    QStringList m_handlers;

    static const int MAX_MESSAGES;
};

class TelepathyPresenceProvider : public QCollectivePresence
{
    Q_OBJECT

public:
    TelepathyPresenceProvider(TelepathyTelephonyService *service);
    ~TelepathyPresenceProvider();

public slots:
    void subscribePeer(const QString &uri);
    void unsubscribePeer(const QString &uri);
    void authorizePeer(const QString &uri);
    void denyPeer(const QString &uri);
    void blockPeer(const QString &uri);
    void unblockPeer(const QString &uri);
    void setLocalPresence(const QCollectivePresenceInfo &info);

private slots:
    void subscribeChannelReady();
    void publishChannelReady();
    void denyChannelReady();

    void presenceUpdate(const TelepathyConnectionInterfacePresence::ContactPresences &presences);
    void aliasesChanged(const QMap<uint, QString> &aliases);
    void aliasesRetrieved(const QMap<uint, QString> &aliases);

    void subscribeMembersChanged(const QString &message,
                                 const QList<uint> &added,
                                 const QList<uint> &removed,
                                 const QList<uint> &localPending,
                                 const QList<uint> &remotePending,
                                 uint actor, TelepathyChannelInterfaceGroup::GroupChangeReason reason);
    void publishMembersChanged(const QString &message,
                               const QList<uint> &added,
                               const QList<uint> &removed,
                               const QList<uint> &localPending,
                               const QList<uint> &remotePending,
                               uint actor, TelepathyChannelInterfaceGroup::GroupChangeReason reason);
    void denyMembersChanged(const QString &message,
                            const QList<uint> &added,
                            const QList<uint> &removed,
                            const QList<uint> &localPending,
                            const QList<uint> &remotePending,
                            uint actor, TelepathyChannelInterfaceGroup::GroupChangeReason reason);

    void capabilitiesRetrieved(const QMap<uint,
                               TelepathyConnectionInterfaceCapabilities::ContactCapability> &capabilities);
    void capabilitiesChanged(const QList<TelepathyConnectionInterfaceCapabilities::CapabilityChange> &capabilities);

    void avatarUpdated(uint contact, const QString &token);
    void avatarRetrieved(uint contact, const QString &token,
                         const TelepathyConnectionInterfaceAvatars::AvatarData &data);
    void modelChanged();

private:
    enum ChangeSignal {
        None = 0,
        BlockedPeers,
        SubscribedPeers,
        PublishedPeers,
        PendingPublishRequests,
        PendingPeerSubscriptions
    };

    void resolveContacts(const QList<uint> &members,
                         const QList<uint> &remotePending,
                         const QList<uint> &localPending,
                        const QString &currentVS, ChangeSignal currentSig,
                        const QString &remoteVS, ChangeSignal remoteSig,
                        const QString &localVS, ChangeSignal localSig);
    void resolveSubscriptionChanges(const QList<uint> &added,
                                    const QList<uint> &removed,
                                    const QList<uint> localPending,
                                    const QList<uint> remotePending,
                                    const QString &memberVS, ChangeSignal currentSig,
                                    const QString &remoteVS, ChangeSignal remoteSig,
                                    const QString &localVS, ChangeSignal localSig);
    void emitSignal(ChangeSignal sig);
    void publishInitialPresences(const QList<uint> &contacts);
    void resolveAliases(const QMap<uint, QString> &aliases);
    void initializeOwnAvatar();
    static const char *qtImageformatForMimetype(const QString &imageMimeType);
    void setAvatarWithConstraints(const QString &file, const QDateTime &lastUpdate);
    void updateAvatar(uint contact);

private:
    TelepathyTelephonyService *m_service;
    TelepathyConnectionInterfacePresence *m_presencei;
    TelepathyConnectionInterfaceCapabilities *m_capabilities;
    QMap<QString, TelepathyConnectionInterfacePresence::StatusInfo> m_statuses;

    TelepathyConnectionInterfaceAliasing *m_aliasing;
    TelepathyConnectionInterfaceAvatars *m_avatars;

    QList<uint> m_subscribed;
    TelepathyConnectionInterfaceAvatars::AvatarRequirements m_reqs;
};

TelepathyLayer::TelepathyLayer(TelepathyTelephonyService *service, TelepathyConnection *conn)
    : m_conn(conn), m_subscribeChannel(0), m_publishChannel(0), m_denyChannel(0), m_service(service)
{
    m_selfHandle = conn->selfHandle();
    QList<uint> handles;
    handles.append(m_selfHandle);
    QStringList inspected = conn->inspectHandles(Telepathy::Contact, handles);

    if (inspected.size() == 0) {
        qWarning() << "Unable to inspect local handle, telepathy agent quitting";
        return;
    }

    m_idToUriMap[m_selfHandle] = inspected[0];
    m_uriToIdMap[inspected[0]] = m_selfHandle;

    m_selfUri = inspected[0];

    qLog(Telepathy) << "Local URI is:" << m_selfUri;

    // Request subscribe, publish, deny ContactList channels
    // to enable add/remove deny/authorize and block/unblock functionality
    {
        QStringList list;
        list.append("subscribe");
        QList<uint> handles = conn->requestHandles(Telepathy::List, list);
        if (handles.size() > 0) 
            m_conn->requestChannel(QString("org.freedesktop.Telepathy.Channel.Type.ContactList"),
                                   Telepathy::List, handles[0], true,
                                   this, SLOT(subscribeChannelReady(QDBusObjectPath)));
    }

    {
        QStringList list;
        list.append("publish");
        QList<uint> handles = conn->requestHandles(Telepathy::List, list);
        if (handles.size() > 0) 
            m_conn->requestChannel("org.freedesktop.Telepathy.Channel.Type.ContactList",
                                   Telepathy::List, handles[0], true,
                                   this, SLOT(publishChannelReady(QDBusObjectPath)));
    }

    {
        QStringList list;
        list.append("deny");
        QList<uint> handles = conn->requestHandles(Telepathy::List, list);
        if (handles.size() > 0)
            m_conn->requestChannel("org.freedesktop.Telepathy.Channel.Type.ContactList",
                                   Telepathy::List, handles[0], true,
                                   this, SLOT(denyChannelReady(QDBusObjectPath)));
    }

    m_contactModel = new QContactModel(this);

    QDir dataDir;
    QString mapName(Qtopia::applicationFileName("telepathyagent", m_service->service()));

    if (!dataDir.exists(mapName))
        dataDir.mkdir(mapName);

    mapName.append('/');
    mapName.append("avatars.map");
    m_avatarMap = new QSettings(mapName, QSettings::IniFormat);
}

TelepathyLayer::~TelepathyLayer()
{
    delete m_publishChannel;
    delete m_subscribeChannel;
    delete m_denyChannel;
    delete m_contactModel;
    delete m_avatarMap;
}

QString TelepathyLayer::avatarFileName(uint contact) const
{
    QString token = m_service->layer()->m_avatarMap->value(QString::number(contact)
        + "/Token").toString();
    if (token.isEmpty())
        return QString();

    QString baseName(m_service->service());
    baseName.append('/');
    baseName.append(token);

    QString imageFileName = Qtopia::applicationFileName("telepathyagent", baseName);
    return imageFileName;
}

QString TelepathyLayer::lookupUri(uint id)
{
    if (m_idToUriMap.contains(id))
        return m_idToUriMap[id];

    QList<uint> handles;
    handles.append(id);

    QStringList uris =
            m_conn->inspectHandles(Telepathy::Contact, handles);

    if (uris.size() != 1)
        return QString();

    return uris[0];
}

void TelepathyLayer::subscribeChannelReady(const QDBusObjectPath &path)
{
    m_subscribeChannel = new TelepathyChannelInterfaceGroup(m_conn->service(),
            path);

    emit subscribeChannelReady();
}

void TelepathyLayer::publishChannelReady(const QDBusObjectPath &path)
{
    m_publishChannel = new TelepathyChannelInterfaceGroup(m_conn->service(),
            path);

    emit publishChannelReady();
}

void TelepathyLayer::denyChannelReady(const QDBusObjectPath &path)
{
    m_denyChannel = new TelepathyChannelInterfaceGroup(m_conn->service(),
            path);

    emit denyChannelReady();
}

void TelepathyLayer::storeToken(uint contact, const QString &token,
                                         const TelepathyConnectionInterfaceAvatars::AvatarData &data)
{
    QString baseName(m_service->service());
    baseName.append('/');

    QString imageName(baseName);
    imageName.append(token);
    QString imageFileName = Qtopia::applicationFileName("telepathyagent", imageName);

    qLog(Telepathy) << "Writing token image data to:" << imageFileName;

    QFile imageFile(imageFileName);
    if (!imageFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Unable to open Avatar Token Store:" << imageFileName;
        return;
    }

    imageFile.write(data.imageData);
    imageFile.close();

    if (m_avatarMap->childGroups().contains(QString::number(contact))) {
        QString prevFile = Qtopia::applicationFileName("telepathyagent",
                baseName + m_avatarMap->value(QString::number(contact) + "/Token").toString());
        qLog(Telepathy) << "Removing previous token:" << prevFile;
        QFile::remove(prevFile);
    }

    m_avatarMap->setValue(QString::number(contact) + "/MimeType", data.mimeType);
    m_avatarMap->setValue(QString::number(contact) + "/Token", token);
    m_avatarMap->sync();
}

void TelepathyLayer::removeToken(uint contact)
{
    if (!m_avatarMap->childGroups().contains(QString::number(contact)))
        return;

    QString imageName(m_service->service());
    imageName.append('/');
    imageName.append(m_avatarMap->value(QString::number(contact) + "/Token").toString());
    imageName = Qtopia::applicationFileName("telepathyagent", imageName);

    qLog(Telepathy) << "Removing token for uid:" << contact << imageName;
    QFile::remove(imageName);

    m_avatarMap->remove(QString::number(contact));
    m_avatarMap->sync();
}

void TelepathyLayer::clearAvatar()
{
    m_avatarMap->remove("AvatarFile");
    m_avatarMap->remove("AvatarTimestamp");
    m_avatarMap->sync();
}

const int TelepathyMessengerProvider::MAX_MESSAGES = 100;

// 60 seconds
#define MESSAGE_SENDER_TIMEOUT 60000

class MessageSender : public QObject
{
    Q_OBJECT

public:
    MessageSender(TelepathyMessengerProvider *provider);
    MessageSender(TelepathyChannel *channel, TelepathyMessengerProvider *provider);
    ~MessageSender();

    void send(const QCollectiveSimpleMessage &message);
    QString remoteUri() const;

signals:
    void aboutToClose();

private slots:
    void received(const TelepathyChannelTypeText::PendingTextMessage &message);
    void channelCreated(const QDBusObjectPath &path);
    void channelClosed();
    void timedOut();
    void sendNextMessage();
    void messageSent();
    void messageError(const QDBusError &error);

private:
    QString m_remoteUri;
    TelepathyChannel *m_channel;
    TelepathyChannelTypeText *m_iface;
    TelepathyMessengerProvider *m_provider;
    QQueue<QCollectiveSimpleMessage> m_msgQ;
    bool m_sendingMessage;
    QTimer m_timer;
};

MessageSender::MessageSender(TelepathyMessengerProvider *provider)
    : QObject(provider)
      , m_channel(0)
      , m_iface(0)
      , m_provider(provider)
      , m_sendingMessage(false)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timedOut()));
}

MessageSender::MessageSender(TelepathyChannel *channel, TelepathyMessengerProvider *provider)
    : QObject(provider)
      , m_channel(channel)
      , m_provider(provider)
      , m_sendingMessage(false)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timedOut()));
    connect(m_channel, SIGNAL(closed()), this, SLOT(channelClosed()));

    m_iface = new TelepathyChannelTypeText(m_channel);
    connect(m_iface, SIGNAL(received(TelepathyChannelTypeText::PendingTextMessage)),
            this, SLOT(received(TelepathyChannelTypeText::PendingTextMessage)));

    m_remoteUri = m_provider->m_service->layer()->lookupUri(m_channel->handle());

    if (m_remoteUri.isEmpty())
        return;

    QList<TelepathyChannelTypeText::PendingTextMessage> list =
            m_iface->listPendingMessages(true);

    QList<QCollectiveSimpleMessage> messages;
    QCollectiveSimpleMessage sm;

    foreach (TelepathyChannelTypeText::PendingTextMessage msg, list) {
        sm.setText(msg.text);
        sm.setTo(m_provider->m_service->layer()->m_selfUri);
        sm.setFrom(m_remoteUri);
        sm.setTimestamp(msg.timestamp);
        if (msg.message_type == TelepathyChannelTypeText::Auto_Reply)
            sm.setType(QCollectiveSimpleMessage::AutoReply);
        else
            sm.setType(QCollectiveSimpleMessage::Normal);

        messages.append(sm);
    }

    m_provider->incomingMessages(messages);

    m_timer.start(MESSAGE_SENDER_TIMEOUT);
}

MessageSender::~MessageSender()
{
    emit aboutToClose();
    // We can get here in two ways
    // 1: Requesting a channel, but it failed, timeout will occur
    // 2: No messages have been sent or received for
    //    MESSAGE_SENDER_TIMEOUT ms, free resources

    foreach (QCollectiveSimpleMessage msg, m_msgQ) {
        emit m_provider->messageFailed(msg);
    }

    delete m_iface;
    m_channel->close();
}

void MessageSender::sendNextMessage()
{
    if (m_msgQ.isEmpty()) {
        m_timer.start(MESSAGE_SENDER_TIMEOUT);
        return;
    }

    m_sendingMessage = true;
    m_timer.stop();
    m_iface->send(TelepathyChannelTypeText::Normal, m_msgQ.head().text(),
                  this, SLOT(messageSent()), SLOT(messageError(QDBusError)));
}

void MessageSender::messageSent()
{
    QCollectiveSimpleMessage message = m_msgQ.dequeue();
    message.setTimestamp(QDateTime::currentDateTime());
    emit m_provider->messageSent(message);

    m_sendingMessage = false;

    sendNextMessage();
}

void MessageSender::messageError(const QDBusError &error)
{
    qLog(Telepathy) << "Message error" << error;

    QCollectiveSimpleMessage message = m_msgQ.dequeue();
    message.setTimestamp(QDateTime::currentDateTime().toUTC());
    emit m_provider->messageFailed(message);

    m_sendingMessage = false;

    sendNextMessage();
}

void MessageSender::send(const QCollectiveSimpleMessage &message)
{
    if (m_iface) {
        if (m_remoteUri != message.to()) {
            qWarning() << "Sending a message to" << message.to() << "in handler of:" << m_remoteUri;
            return;
        }

        m_msgQ.enqueue(message);
        if (!m_sendingMessage) {
            sendNextMessage();
        }
        return;
    }

    // Used for the sending path (outgoing messages)
    // Construct a channel, handle timeouts
    TelepathyConnection *conn = m_provider->m_service->connection();
    QList<QString> list;
    list.append(message.to());
    QList<uint> handles = conn->requestHandles(Telepathy::Contact, list);

    if (handles.size() != 1) {
        emit m_provider->messageFailed(message);
        return;
    }

    if (!conn->requestChannel(QString("org.freedesktop.Telepathy.Channel.Type.Text"),
                        Telepathy::Contact, handles[0], true,
                        this, SLOT(channelCreated(QDBusObjectPath)))) {
        emit m_provider->messageFailed(message);
        return;
    }

    m_msgQ.enqueue(message);
    m_timer.start(MESSAGE_SENDER_TIMEOUT);
}

void MessageSender::channelCreated(const QDBusObjectPath &path)
{
    m_channel = new TelepathyChannel("org.freedesktop.Telepathy.Channel.Type.Text",
                                     path, m_provider->m_service->connection());
    connect(m_channel, SIGNAL(closed()), this, SLOT(channelClosed()));

    m_iface = new TelepathyChannelTypeText(m_channel);
    connect(m_iface, SIGNAL(received(TelepathyChannelTypeText::PendingTextMessage)),
            this, SLOT(received(TelepathyChannelTypeText::PendingTextMessage)));

    m_remoteUri = m_msgQ.head().to();
    sendNextMessage();
}

void MessageSender::received(const TelepathyChannelTypeText::PendingTextMessage &message)
{
    QCollectiveSimpleMessage sm;
    sm.setText(message.text);
    sm.setTo(m_provider->m_service->layer()->m_selfUri);
    sm.setFrom(m_remoteUri);
    sm.setTimestamp(message.timestamp);
    if (message.message_type == TelepathyChannelTypeText::Auto_Reply)
        sm.setType(QCollectiveSimpleMessage::AutoReply);
    else
       sm.setType(QCollectiveSimpleMessage::Normal);

    QList<QCollectiveSimpleMessage> messages;
    messages.append(sm);
    m_provider->incomingMessages(messages);

    QList<uint> message_ids;
    message_ids.append(message.message_id);
    m_iface->acknowledgePendingMessages(message_ids);

    m_timer.start(MESSAGE_SENDER_TIMEOUT);
}

void MessageSender::channelClosed()
{
    deleteLater();
}

void MessageSender::timedOut()
{
    qLog(Telepathy) << "MessageSender for" << m_remoteUri << "(" << this << ")" <<
            "timed out";
    deleteLater();
}

QString MessageSender::remoteUri() const
{
    return m_remoteUri;
}

TelepathyMessengerProvider::TelepathyMessengerProvider(TelepathyTelephonyService *service)
    : QCollectiveMessenger(service->service(), service, Server), m_service(service)
{
    QObject::connect(service->connection(), SIGNAL(newChannel(TelepathyConnection::ChannelInfo,bool)),
                     this, SLOT(channelCreated(TelepathyConnection::ChannelInfo,bool)));
}

TelepathyMessengerProvider::~TelepathyMessengerProvider()
{
    foreach (MessageSender *sender, m_senders) {
        disconnect(sender, SIGNAL(aboutToClose()), this, SLOT(destroyed()));
        delete sender;
    }

    m_senders.clear();
}

void TelepathyMessengerProvider::sendMessage(const QCollectiveSimpleMessage &message)
{
    MessageSender *sender;
    if (m_senders.contains(message.to())) {
        sender = m_senders[message.to()];
    }
    else {
        sender = new MessageSender(this);
        connect(sender, SIGNAL(aboutToClose()), this, SLOT(destroyed()));
        m_senders.insert(message.to(), sender);
    }

    sender->send(message);
}

void TelepathyMessengerProvider::registerIncomingHandler(const QString &service)
{
    if (m_handlers.contains(service))
        return;

    m_handlers.append(service);

    sendPendingMessages();
}

void TelepathyMessengerProvider::unregisterIncomingHandler(const QString &service)
{
    m_handlers.removeAll(service);
}

void TelepathyMessengerProvider::incomingMessages(const QList<QCollectiveSimpleMessage> &messages)
{
    foreach (QCollectiveSimpleMessage message, messages) {
        if (m_pending.size() == MAX_MESSAGES)
            m_pending.pop_front();

        m_pending.push_back(message);
    }

    sendPendingMessages();
}

void TelepathyMessengerProvider::sendPendingMessages()
{
    if (m_pending.size() == 0)
        return;

    if (m_handlers.size() > 0) {
        QList<QCollectiveSimpleMessage> messages;
        while (!m_pending.empty()) {
            messages.append(m_pending.takeFirst());
        }

        if (messages.size() == 0)
            return;

        foreach (QString handler, m_handlers) {
            QtopiaServiceRequest req(handler, "incomingMessages(QList<QCollectiveSimpleMessage>)");
            req << messages;
            req.send();
        }
    }
}

void TelepathyMessengerProvider::destroyed()
{
    MessageSender *s = qobject_cast<MessageSender *>(sender());
    if (!s)
        return;

    m_senders.remove(s->remoteUri());
}

void TelepathyMessengerProvider::channelCreated(const TelepathyConnection::ChannelInfo &info,
                                               bool suppressHandler)
{
    if (info.type == "org.freedesktop.Telepathy.Channel.Type.Text"
        && !suppressHandler && info.handleType == Telepathy::Contact) {
        TelepathyChannel *channel = new TelepathyChannel(info.type, info.path,
                info.handleType, info.handle, m_service->connection());
        MessageSender *sender = new MessageSender(channel, this);

        if (sender->remoteUri().isEmpty()) {
            qWarning() << "TelepathyMessengerProvider::channelCreated - Unable to construct the message sender correctly";
            return;
        }

        connect(sender, SIGNAL(aboutToClose()), this, SLOT(destroyed()));
        m_senders.insert(sender->remoteUri(), sender);
    }
}

TelepathyPresenceProvider::TelepathyPresenceProvider(TelepathyTelephonyService *service)
    : QCollectivePresence(service->service(), service, Server), m_service(service)
{
    TelepathyConnection *conn = m_service->connection();
    Q_ASSERT(conn);

    m_service = service;

    m_aliasing = 0;
    m_capabilities = 0;
    m_avatars = 0;

    setValue("Protocol", conn->protocol(), Delayed);

    QStringList interfaces = m_service->connection()->interfaces();
    if (interfaces.contains("org.freedesktop.Telepathy.Connection.Interface.Aliasing")) {
        m_aliasing = new TelepathyConnectionInterfaceAliasing(m_service->connection());
        connect(m_aliasing, SIGNAL(aliasesChanged(QMap<uint,QString>)),
                this, SLOT(aliasesChanged(QMap<uint,QString>)));
        connect(m_aliasing, SIGNAL(aliasesRetrieved(QMap<uint,QString>)),
                this, SLOT(aliasesRetrieved(QMap<uint,QString>)));
    }

    if (interfaces.contains("org.freedesktop.Telepathy.Connection.Interface.Capabilities")) {
        m_capabilities = new TelepathyConnectionInterfaceCapabilities(m_service->connection());
        connect(m_capabilities,
                SIGNAL(capabilitiesRetrieved(QMap<uint,TelepathyConnectionInterfaceCapabilities::ContactCapability>)),
                this, SLOT(capabilitiesRetrieved(QMap<uint,TelepathyConnectionInterfaceCapabilities::ContactCapability>)));
        connect(m_capabilities, SIGNAL(capabilitiesChanged(QList<TelepathyConnectionInterfaceCapabilities::CapabilityChange>)),
                this, SLOT(capabilitiesChanged(QList<TelepathyConnectionInterfaceCapabilities::CapabilityChange>)));
    }

    m_presencei = new TelepathyConnectionInterfacePresence(conn);
    connect(m_presencei, SIGNAL(presenceUpdate(TelepathyConnectionInterfacePresence::ContactPresences)),
            this, SLOT(presenceUpdate(TelepathyConnectionInterfacePresence::ContactPresences)));
    m_statuses = m_presencei->availableStatuses();

    // Request the Status Types from the server and
    // build the away types info for QPresenceProvider
    QMap<QString, TelepathyConnectionInterfacePresence::StatusInfo>::const_iterator it =
            m_statuses.constBegin();
    QStringList awayTypes;
    while (it != m_statuses.constEnd()) {
        qLog(Telepathy) << "Processing Possible Status:" << it.key() << it.value().type <<
                it.value().maySetOnSelf << it.value().exclusive << it.value().parameter_types;

        QCollectivePresenceInfo::PresenceType presenceType = QCollectivePresenceInfo::None;
        switch (it.value().type) {
            case TelepathyConnectionInterfacePresence::Unset:
                presenceType = QCollectivePresenceInfo::None;
                break;
            case TelepathyConnectionInterfacePresence::Offline:
                presenceType  = QCollectivePresenceInfo::Offline;
                break;
            case TelepathyConnectionInterfacePresence::Available:
                presenceType = QCollectivePresenceInfo::Online;
                break;
            case TelepathyConnectionInterfacePresence::Away:
                presenceType = QCollectivePresenceInfo::Away;
                break;
            case TelepathyConnectionInterfacePresence::ExtendedAway:
                presenceType = QCollectivePresenceInfo::ExtendedAway;
                break;
            case TelepathyConnectionInterfacePresence::Hidden:
                presenceType = QCollectivePresenceInfo::Hidden;
                break;
            case TelepathyConnectionInterfacePresence::Busy:
                presenceType = QCollectivePresenceInfo::Busy;
                break;
        }

        setValue(QString("StatusTypes/") + it.key(), static_cast<uint>(presenceType), Delayed);
        ++it;
    }

    if (m_service->layer()->m_publishChannel) {
        publishChannelReady();
    } else {
        connect(m_service->layer(), SIGNAL(publishChannelReady()),
                this, SLOT(publishChannelReady()));
    }

    if (m_service->layer()->m_subscribeChannel) {
        subscribeChannelReady();
    } else {
        connect(m_service->layer(), SIGNAL(subscribeChannelReady()),
                this, SLOT(subscribeChannelReady()));
    }

    if (m_service->layer()->m_denyChannel) {
        denyChannelReady();
    } else {
        connect(m_service->layer(), SIGNAL(denyChannelReady()),
                this, SLOT(denyChannelReady()));
    }

    {
        QList<uint> handles;
        handles.append(m_service->layer()->m_selfHandle);
        publishInitialPresences(handles);
        if (m_aliasing)
            m_aliasing->requestAliases(handles);
        // Wait until local alias is resolved to emit localPresenceChanged()
        setValue("LocalUri", m_service->layer()->lookupUri(m_service->layer()->m_selfHandle));
    }

    if (interfaces.contains("org.freedesktop.Telepathy.Connection.Interface.Avatars")) {
        qLog(Telepathy) << "Adding Avatar provider";
        m_avatars = new TelepathyConnectionInterfaceAvatars(m_service->connection());
        connect(m_avatars, SIGNAL(avatarRetrieved(uint,QString,TelepathyConnectionInterfaceAvatars::AvatarData)),
            this, SLOT(avatarRetrieved(uint,QString,TelepathyConnectionInterfaceAvatars::AvatarData)));

        m_reqs = m_avatars->avatarRequirements();

        qLog(Telepathy) << "Avatar Requirements:" << m_reqs.mimeTypes << m_reqs.minimumWidth <<
                m_reqs.minimumHeight << m_reqs.maximumWidth << m_reqs.maximumHeight <<
                m_reqs.maximumSize;

        connect(m_service->layer()->m_contactModel, SIGNAL(modelReset()),
                this, SLOT(modelChanged()));
        connect(m_service->layer()->m_contactModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(modelChanged()));

        initializeOwnAvatar();
    }
}

TelepathyPresenceProvider::~TelepathyPresenceProvider()
{
    delete m_presencei;
    delete m_aliasing;
}

void TelepathyPresenceProvider::subscribePeer(const QString &uri)
{
    if (!m_service->layer()->m_subscribeChannel)
        return;

    QList<uint> ids;
    if (m_service->layer()->m_uriToIdMap.contains(uri)) {
        ids.push_back(m_service->layer()->m_uriToIdMap[uri]);
    } else {
        // Need to create a new contact
        QStringList uris;
        uris.append(uri);
        ids = m_service->connection()->requestHandles(Telepathy::Contact, uris);
    }

    if (ids.size() != 1)
        return;

    m_service->layer()->m_subscribeChannel->addMembers(ids, QString(""));
}

void TelepathyPresenceProvider::unsubscribePeer(const QString &uri)
{
    if (!m_service->layer()->m_subscribeChannel)
        return;

    if (!m_service->layer()->m_uriToIdMap.contains(uri))
        return;

    QList<uint> ids;
    ids.push_back(m_service->layer()->m_uriToIdMap[uri]);

    m_service->layer()->m_subscribeChannel->removeMembers(ids, QString());
}

void TelepathyPresenceProvider::authorizePeer(const QString &uri)
{
    if (!m_service->layer()->m_publishChannel)
        return;

    if (!m_service->layer()->m_uriToIdMap.contains(uri))
        return;

    QList<uint> ids;
    ids.push_back(m_service->layer()->m_uriToIdMap[uri]);
    m_service->layer()->m_publishChannel->addMembers(ids, QString());
}

void TelepathyPresenceProvider::denyPeer(const QString &uri)
{
    if (!m_service->layer()->m_publishChannel)
        return;

    if (!m_service->layer()->m_uriToIdMap.contains(uri))
        return;

    QList<uint> ids;
    ids.push_back(m_service->layer()->m_uriToIdMap[uri]);
    m_service->layer()->m_publishChannel->removeMembers(ids, QString());
}

void TelepathyPresenceProvider::blockPeer(const QString &uri)
{
    if (!m_service->layer()->m_denyChannel)
        return;

    if (!m_service->layer()->m_uriToIdMap.contains(uri))
        return;

    QList<uint> ids;
    ids.push_back(m_service->layer()->m_uriToIdMap[uri]);
    m_service->layer()->m_denyChannel->addMembers(ids, QString());
}

void TelepathyPresenceProvider::unblockPeer(const QString &uri)
{
    if (!m_service->layer()->m_denyChannel)
        return;

    if (!m_service->layer()->m_uriToIdMap.contains(uri))
        return;

    QList<uint> ids;
    ids.push_back(m_service->layer()->m_uriToIdMap[uri]);
    m_service->layer()->m_denyChannel->removeMembers(ids, QString());
}

void TelepathyPresenceProvider::setLocalPresence(const QCollectivePresenceInfo &info)
{
    QCollectivePresenceInfo oldInfo = localInfo();

    if (info.uri() != m_service->layer()->m_selfUri)
        return;

    if (m_aliasing && (oldInfo.displayName() != info.displayName())) {
        QMap<uint, QString> alias;
        alias.insert(m_service->layer()->m_selfHandle, info.displayName());
        if (!m_aliasing->setAliases(alias)) {
            qWarning() << "setLocalPresence: Unable to set local alias";
        }
    }

    if (!statusTypes().keys().contains(info.presence()))
        return;

    if ((statusTypes()[info.presence()] == QCollectivePresenceInfo::Offline) ||
        (statusTypes()[info.presence()] == QCollectivePresenceInfo::None)) {
        qWarning() << "Setting an Offline or None presence on local account";
        return;
    }

    m_presencei->setStatus(info.presence(), info.properties());
    if (info.lastUpdateTime().isValid())
        m_presencei->setLastActivityTime(info.lastUpdateTime());

    publishPeerInfo(info);
    emit localPresenceChanged();
}

void TelepathyPresenceProvider::emitSignal(ChangeSignal sig)
{
    switch (sig) {
        case BlockedPeers:
            emit blockedPeersChanged();
            return;
        case SubscribedPeers:
            emit subscribedPeersChanged();
            return;
        case PublishedPeers:
            emit publishedPeersChanged();
            return;
        case PendingPublishRequests:
            emit pendingPublishRequestsChanged();
            return;
        case PendingPeerSubscriptions:
            emit pendingPeerSubscriptionsChanged();
            return;
        default:
            return;
    };
}

void TelepathyPresenceProvider::resolveContacts(const QList<uint> &members,
        const QList<uint> &remotePending,
        const QList<uint> &localPending,
        const QString &currentVS, ChangeSignal currentSig,
        const QString &remotePendingVS, ChangeSignal remoteSig,
        const QString &localPendingVS, ChangeSignal localSig)
{
    QList<uint> unified = members;
    for (int i = 0; i < remotePending.size(); ++i)
        unified.append(remotePending[i]);
    for (int i = 0; i < localPending.size(); ++i)
        unified.append(localPending[i]);

    QStringList unifiedUris = m_service->connection()->inspectHandles(Telepathy::Contact, unified);
    for (int i = 0; i < unifiedUris.size(); ++i) {
        m_service->layer()->m_uriToIdMap[unifiedUris[i]] = unified[i];
        m_service->layer()->m_idToUriMap[unified[i]] = unifiedUris[i];
    }

    int ui = 0;
    if (!currentVS.isEmpty()) {
        QStringList uris;
        for (int i = 0; i < members.size(); ++ui, ++i) {
            uris.push_back(unifiedUris[ui]);
        }

        setValue(currentVS, uris);
        emitSignal(currentSig);
    } else {
        ui += members.size();
    }

    if (!remotePendingVS.isEmpty()) {
        QStringList uris;
        for (int i = 0; i < remotePending.size(); ++ui, ++i) {
            uris.push_back(unifiedUris[ui]);
        }

        setValue(remotePendingVS, uris);
        emitSignal(remoteSig);
    } else {
        ui += remotePending.size();
    }

    if (!localPendingVS.isEmpty()) {
        QStringList uris;
        for (int i = 0; i < localPending.size(); ++ui, ++i) {
            uris.push_back(unifiedUris[ui]);
        }

        setValue(localPendingVS, uris);
        emitSignal(localSig);
    }
}

void TelepathyPresenceProvider::publishInitialPresences(const QList<uint> &contacts)
{
    TelepathyConnectionInterfacePresence::ContactPresences presences =
            m_presencei->readCachedPresence(contacts);

    if (presences.size() == 0)
        return;

    QStringList capabilities;
    capabilities.append("IM");

    TelepathyConnectionInterfacePresence::ContactPresences::const_iterator it =
            presences.constBegin();
    while (it != presences.constEnd()) {
        QCollectivePresenceInfo info(m_service->layer()->m_idToUriMap[it.key()]);
        info.setLastUpdateTime(it.value().lastActivity);
        QString status = it.value().status;
        QCollectivePresenceInfo::PresenceType type = static_cast<QCollectivePresenceInfo::PresenceType>(value("StatusTypes/" + status, QCollectivePresenceInfo::None).toUInt());
        info.setPresence(status, type);
        info.setCapabilities(capabilities);
        info.setMessage(it.value().parameters.value("message").toString());

        qLog(Telepathy) << "PublishInitialPresences: Setting avatar to:"
                << m_service->layer()->avatarFileName(it.key());
        info.setAvatar(m_service->layer()->avatarFileName(it.key()));
        publishPeerInfo(info);
        ++it;
    }
}

void TelepathyPresenceProvider::subscribeChannelReady()
{
    connect(m_service->layer()->m_subscribeChannel, SIGNAL(membersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,TelepathyChannelInterfaceGroup::GroupChangeReason)),
            this, SLOT(subscribeMembersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,TelepathyChannelInterfaceGroup::GroupChangeReason)));

    QMap<QString, QList<uint> > allMembers = m_service->layer()->m_subscribeChannel->allMembers();
    m_subscribed = allMembers["current"];
    QList<uint> remotePendingSubs = allMembers["remotepending"];
    QList<uint> localPendingSubs = allMembers["localpending"];

    if (m_subscribed.size() == 0 && remotePendingSubs.size() == 0 && localPendingSubs.size() == 0)
        return;

    resolveContacts(m_subscribed, remotePendingSubs, localPendingSubs,
                    "SubscribedPeers", SubscribedPeers,
                    "PendingPeerSubscriptions", PendingPeerSubscriptions,
                    QString(), None);

    if (m_avatars) {
        connect(m_avatars, SIGNAL(avatarUpdated(uint,QString)),
            this, SLOT(avatarUpdated(uint,QString)));
        qLog(Telepathy) << "TelepathyAvatars::subscribeChannelReady:" << m_subscribed;

        // Remove any contacts not on the subscribed list from the filesystem
        QSet<uint> subscribedContacts = QSet<uint>::fromList(m_subscribed);
        QSet<uint> existingContacts;
        foreach (QString contact, m_service->layer()->m_avatarMap->childGroups()) {
            uint id = contact.toUInt();
            if (id != 0)
                existingContacts.insert(id);
        }

        qLog(Telepathy) << "subscribedSet:" << subscribedContacts;
        qLog(Telepathy) << "existingSet:" << existingContacts;
        QSet<uint> toRemove = existingContacts - subscribedContacts;
        qLog(Telepathy) << "toRemove set:" << toRemove;

        foreach (uint contact, toRemove) {
            m_service->layer()->removeToken(contact);
        }

        QMap<uint, QString> tokens = m_avatars->knownAvatarTokens(m_subscribed);

        // Here check the subscribed list against the known tokens
        // If the subscribed contact is not in the known map, add them
        // to a list of contacts to request avatars from
        // If avatars are known, check against cached content, and add
        // them to the list if the tokens are different
        QList<uint> avatarsToRequest;

        for (int i = 0; i < m_subscribed.size(); i++) {
            QMap<uint, QString>::const_iterator it = tokens.find(m_subscribed[i]);
            if (it == tokens.constEnd()) {
                qLog(Telepathy) << "Adding contact to request list:" << m_subscribed[i];
                avatarsToRequest.append(m_subscribed[i]);
                ++it;
                continue;
            }

            QString prevToken = m_service->layer()->m_avatarMap->value(QString::number(m_subscribed[i])
                    + "/Token").toString();

            if (prevToken != it.value() && !it.value().isEmpty()) {
                avatarsToRequest.append(m_subscribed[i]);
            }

            ++it;
        }

        if (avatarsToRequest.size() > 0) {
            qLog(Telepathy) << "Requesting avatars:" << avatarsToRequest;
            m_avatars->requestAvatars(avatarsToRequest);
        }
    }

    if (m_subscribed.size() > 0) {
        publishInitialPresences(m_subscribed);

        if (m_aliasing)
            m_aliasing->requestAliases(m_subscribed);

        if (m_capabilities)
            m_capabilities->retrieveCapabilities(m_subscribed);
    }
}

void TelepathyPresenceProvider::publishChannelReady()
{
    connect(m_service->layer()->m_publishChannel, SIGNAL(membersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,TelepathyChannelInterfaceGroup::GroupChangeReason)),
            this, SLOT(publishMembersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,TelepathyChannelInterfaceGroup::GroupChangeReason)));

    QMap<QString, QList<uint> > allMembers = m_service->layer()->m_publishChannel->allMembers();
    QList<uint> published = allMembers["current"];
    QList<uint> remotePending = allMembers["remotepending"];
    QList<uint> localPending = allMembers["localpending"];

    if (published.size() == 0 && remotePending.size() == 0 && localPending.size() == 0)
        return;

    resolveContacts(published, remotePending, localPending,
                    "PublishedPeers", PublishedPeers,
                    QString(), None,
                    "PendingPublishRequests", PendingPublishRequests);
}

void TelepathyPresenceProvider::denyChannelReady()
{
    connect(m_service->layer()->m_denyChannel, SIGNAL(membersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,TelepathyChannelInterfaceGroup::GroupChangeReason)),
            this, SLOT(denyMembersChanged(QString,QList<uint>,QList<uint>,QList<uint>,QList<uint>,uint,TelepathyChannelInterfaceGroup::GroupChangeReason)));

    QMap<QString, QList<uint> > allMembers = m_service->layer()->m_denyChannel->allMembers();
    QList<uint> blocked = allMembers["current"];
    QList<uint> remotePending = allMembers["remotepending"];
    QList<uint> localPending = allMembers["localpending"];

    if (blocked.size() == 0 && remotePending.size() == 0 && localPending.size() == 0)
        return;

    resolveContacts(blocked, remotePending, localPending,
                    "BlockedPeers", BlockedPeers,
                    QString(), None,
                    QString(), None);
}

void TelepathyPresenceProvider::presenceUpdate(
        const TelepathyConnectionInterfacePresence::ContactPresences &presences)
{
    QStringList changedUris;
    TelepathyConnectionInterfacePresence::ContactPresences::const_iterator it =
            presences.constBegin();

    while (it != presences.end()) {
        if (m_service->layer()->m_idToUriMap.contains(it.key())) {
            QString uri = m_service->layer()->m_idToUriMap[it.key()];
            QCollectivePresenceInfo info = peerInfo(uri);
            if (!info.isNull()) {
                QString status = it.value().status;
                QCollectivePresenceInfo::PresenceType type = static_cast<QCollectivePresenceInfo::PresenceType>(value("StatusTypes/" + status, QCollectivePresenceInfo::None).toUInt());
                info.setPresence(status, type);
                info.setLastUpdateTime(it.value().lastActivity);
                info.setMessage(it.value().parameters.value("message").toString());

                if (it.key() != m_service->layer()->m_selfHandle)
                    changedUris.append(uri);
                publishPeerInfo(info);
            }
        }
        ++it;
    }

    if (presences.contains(m_service->layer()->m_selfHandle)) {
        emit localPresenceChanged();
        if (presences.size() == 1)
            return;
    }

    if (changedUris.size() > 0)
        emit peerPresencesChanged(changedUris);
}

void TelepathyPresenceProvider::resolveAliases(const QMap<uint, QString> &aliases)
{
    QStringList changedUris;
    QMap<uint, QString>::const_iterator it = aliases.constBegin();

    while (it != aliases.constEnd()) {
        if (m_service->layer()->m_idToUriMap.contains(it.key())) {
            QString uri = m_service->layer()->m_idToUriMap[it.key()];
            QCollectivePresenceInfo info = peerInfo(uri);
            if (!info.isNull() && (info.displayName() != it.value())) {
                info.setDisplayName(it.value());
                publishPeerInfo(info);
                changedUris.append(uri);
            }
        }
        ++it;
    }

    if (changedUris.size() == 0)
        return;

    if (changedUris.contains(m_service->layer()->m_selfUri)) {
        emit localPresenceChanged();
        if (changedUris.size() == 1)
            return;
    }

    emit peerPresencesChanged(changedUris);
}

void TelepathyPresenceProvider::aliasesChanged(const QMap<uint, QString> &aliases)
{
    resolveAliases(aliases);
}

void TelepathyPresenceProvider::aliasesRetrieved(const QMap<uint, QString> &aliases)
{
    resolveAliases(aliases);
}

void TelepathyPresenceProvider::resolveSubscriptionChanges(const QList<uint> &added,
        const QList<uint> &removed,
        const QList<uint> localPending, const QList<uint> remotePending,
        const QString &memberVS, ChangeSignal memberSig,
        const QString &remoteVS, ChangeSignal remoteSig,
        const QString &localVS, ChangeSignal localSig)
{
    QList<uint> newIdsToMap;

    bool processMembers = !memberVS.isEmpty();
    int addedOffset = 0;
    int addedLength = 0;
    QStringList memberList;
    if (processMembers) {
        memberList = value(memberVS, QStringList()).toStringList();
        foreach (uint id, added) {
            if (m_service->layer()->m_idToUriMap.contains(id))
                memberList.append(m_service->layer()->m_idToUriMap[id]);
            else {
                newIdsToMap.append(id);
                ++addedLength;
            }
        }
    }

    bool processLocal = !localVS.isEmpty();
    int localOffset = addedOffset + addedLength;
    int localLength = 0;
    QStringList localPendingList;
    if (processLocal) {
        localPendingList = value(localVS, QStringList()).toStringList();
        foreach (uint id, localPending) {
            if (m_service->layer()->m_idToUriMap.contains(id))
                localPendingList.append(m_service->layer()->m_idToUriMap[id]);
            else {
                newIdsToMap.append(id);
                ++localLength;
            }
        }
    }

    bool processRemote = !remoteVS.isEmpty();
    int remoteOffset = localOffset + localLength;
    int remoteLength = 0;
    QStringList remotePendingList;
    if (processRemote) {
        remotePendingList = value(remoteVS, QStringList()).toStringList();
        foreach (uint id, remotePending) {
            if (m_service->layer()->m_idToUriMap.contains(id))
                remotePendingList.append(m_service->layer()->m_idToUriMap[id]);
            else {
                newIdsToMap.append(id);
                ++remoteLength;
            }
        }
    }

    if (newIdsToMap.count() > 0) {
        QStringList mappedUris = m_service->connection()->inspectHandles(Telepathy::Contact, newIdsToMap);

        for (int i = 0; i < mappedUris.size(); i++) {
            m_service->layer()->m_uriToIdMap[mappedUris[i]] = newIdsToMap[i];
            m_service->layer()->m_idToUriMap[newIdsToMap[i]] = mappedUris[i];
        }

        if (processMembers)
            for (int i = addedOffset; i < (addedOffset + addedLength); ++i)
                memberList.append(mappedUris[i]);

        if (processLocal)
            for (int i = localOffset; i < (localOffset + localLength); ++i)
                localPendingList.append(mappedUris[i]);

        if (processRemote)
            for (int i = remoteOffset; i < (remoteOffset + localOffset); ++i)
                remotePendingList.append(mappedUris[i]);
    }

    // Now resolve removals
    bool removedMembers = false;
    bool removedLocal = false;
    bool removedRemote = false;

    for (int i = 0; i < removed.size(); i++) {
        if (processMembers && memberList.contains(m_service->layer()->m_idToUriMap[removed[i]])) {
            QString uri = m_service->layer()->m_idToUriMap[removed[i]];
            memberList.removeAll(uri);
            // Remove presence info from value space
            if (memberSig == SubscribedPeers)
                removeValue("PresenceInfo/" + uri);
            removedMembers = true;
        } else if (processLocal &&
                   localPendingList.contains(m_service->layer()->m_idToUriMap[removed[i]])) {
            localPendingList.removeAll(m_service->layer()->m_idToUriMap[removed[i]]);
            removedLocal = true;
        }
        else if (processRemote &&
                 remotePendingList.contains(m_service->layer()->m_idToUriMap[removed[i]])) {
            remotePendingList.removeAll(m_service->layer()->m_idToUriMap[removed[i]]);
            removedRemote = true;
        }
    }

    if (processMembers && (removedMembers || added.size() > 0)) {
        setValue(memberVS, memberList);
        emitSignal(memberSig);
    }

    if (processLocal && (removedLocal || localPending.size() > 0)) {
        setValue(localVS, localPendingList);
        emitSignal(localSig);
    }

    if (processRemote && (removedRemote || remotePending.size() > 0)) {
        setValue(remoteVS, remotePendingList);
        emitSignal(remoteSig);
    }
}

void TelepathyPresenceProvider::subscribeMembersChanged(const QString &message,
                                                        const QList<uint> &added,
                                                        const QList<uint> &removed,
                                                        const QList<uint> &localPending,
                                                        const QList<uint> &remotePending,
                                                        uint actor,
                                                        TelepathyChannelInterfaceGroup::GroupChangeReason reason)
{
    Q_UNUSED(message)
    Q_UNUSED(actor)
    Q_UNUSED(reason)

    QStringList oldList = subscribedPeers();

    resolveSubscriptionChanges(added, removed, localPending, remotePending,
                               "SubscribedPeers", SubscribedPeers,
                               "PendingPeerSubscriptions", PendingPeerSubscriptions,
                               QString(), None);

    if (added.size() > 0) {
        publishInitialPresences(added);
        if (m_aliasing) m_aliasing->requestAliases(added);

        QStringList addedPeers;
        for (int i = 0; i < added.size(); i++)
            addedPeers.append(m_service->layer()->m_idToUriMap[added[i]]);
        emit peerPresencesChanged(addedPeers);

        if (m_avatars) {
            //TODO: Is this necessary, or will the CM automatically request
            // one for us?
            m_avatars->requestAvatars(added);
        }
    }

    if (removed.size() > 0) {
        if (m_avatars) {
            foreach (uint contact, removed) {
                if (m_subscribed.removeAll(contact)) {
                    qLog(Telepathy) << "Contact removed from subscribe list:" << contact;
                    m_service->layer()->removeToken(contact);
                }
            }
        }
    }

}

void TelepathyPresenceProvider::publishMembersChanged(const QString &message,
                                                      const QList<uint> &added,
                                                      const QList<uint> &removed,
                                                      const QList<uint> &localPending,
                                                      const QList<uint> &remotePending,
                                                      uint actor,
                                                      TelepathyChannelInterfaceGroup::GroupChangeReason reason)
{
    Q_UNUSED(actor)
    Q_UNUSED(reason)
    Q_UNUSED(message)

    resolveSubscriptionChanges(added, removed, localPending, remotePending,
                               "PublishedPeers", PublishedPeers,
                               QString(), None,
                               "PendingPublishRequests", PendingPublishRequests);
}

void TelepathyPresenceProvider::denyMembersChanged(const QString &message,
                                                   const QList<uint> &added,
                                                   const QList<uint> &removed,
                                                   const QList<uint> &localPending,
                                                   const QList<uint> &remotePending,
                                                   uint actor,
                                                   TelepathyChannelInterfaceGroup::GroupChangeReason reason)
{
    Q_UNUSED(actor)
    Q_UNUSED(reason)
    Q_UNUSED(message)

    resolveSubscriptionChanges(added, removed, localPending, remotePending,
                               "BlockedPeers", BlockedPeers,
                               QString(), None, QString(), None);
}

void TelepathyPresenceProvider::capabilitiesRetrieved(
        const QMap<uint, TelepathyConnectionInterfaceCapabilities::ContactCapability> &capabilities)
{
    QMap<uint, TelepathyConnectionInterfaceCapabilities::ContactCapability>::const_iterator it =
            capabilities.constBegin();

#ifdef CAPABILITIES_DEBUG
    while (it != capabilities.constEnd()) {
        qDebug() << it.key() << it.value().handle <<
                it.value().channel_type << it.value().generic_flags << it.value().type_specific_flags;
        ++it;
    }
#endif
}

void TelepathyPresenceProvider::capabilitiesChanged(
        const QList<TelepathyConnectionInterfaceCapabilities::CapabilityChange> &capabilities)
{
#ifdef CAPABILITIES_DEBUG
    foreach (const TelepathyConnectionInterfaceCapabilities::CapabilityChange &c, capabilities) {
        qDebug() << c.handle << c.channel_type << c.old_generic_flags << c.new_generic_flags
                << c.old_type_specific_flags << c.new_type_specific_flags;
    }
#else
    Q_UNUSED(capabilities);
#endif
}

void TelepathyPresenceProvider::setAvatarWithConstraints(const QString &file,
        const QDateTime &lastModified)
{
    QImage img(file);
    if (img.isNull()) {
        qWarning() << "TelepathyAvatarProvider: Unable to load image:" << file;
        return;
    }

    QImage scaled;
    if (img.height() > m_reqs.maximumHeight || img.width() > m_reqs.maximumWidth) {
        qLog(Telepathy) << "Current avatar picture is too large (" <<
                img.width() << "," << img.height() << ")";
        scaled = img.height() > img.width()
            ? img.scaledToHeight(m_reqs.maximumHeight) :
                img.scaledToWidth(m_reqs.maximumWidth);
    } else {
        scaled = img;
    }

    int targetHeight = m_reqs.maximumHeight;
    int targetWidth = m_reqs.maximumWidth;
    bool acceptableSize = false;
    QByteArray avatar;

    while ( (targetHeight > m_reqs.minimumHeight) &&
                (targetWidth > m_reqs.minimumWidth) && !acceptableSize) {
        avatar.clear();
        QBuffer buffer(&avatar);
        buffer.open(QIODevice::WriteOnly);
        scaled.save(&buffer, qtImageformatForMimetype(m_reqs.mimeTypes[0]));
        buffer.close();

        qLog(Telepathy) << "Saved image as:" << m_reqs.mimeTypes[0]
                << "new size is:" << avatar.size() << ", required max:" << m_reqs.maximumSize;

        targetHeight -= 16;
        targetWidth -= 16;

        if (static_cast<uint>(avatar.size()) <= m_reqs.maximumSize)
            acceptableSize = true;
    }

    m_service->layer()->m_avatarMap->setValue("AvatarFile", file);
    m_service->layer()->m_avatarMap->setValue("AvatarTimestamp", lastModified);
    m_service->layer()->m_avatarMap->sync();

    TelepathyConnectionInterfaceAvatars::AvatarData data;
    data.imageData = avatar;
    data.mimeType = m_reqs.mimeTypes[0];
    m_avatars->setAvatar(data);
}

void TelepathyPresenceProvider::initializeOwnAvatar()
{
    QString avatarFile = m_service->layer()->m_avatarMap->value("AvatarFile").toString();
    QDateTime avatarStamp = m_service->layer()->m_avatarMap->value("AvatarTimestamp").toDateTime();

    // Four cases
    // Case 1: We have cached an avatar, and user has reset own contact to null
    // Case 2: We have cached an avatar, and it matches what is set now
    // Case 3: We have not cached an avatar, or it doesn't match what is
    //         set now.
    // Case 4: No cached avatar, no own contact set

    QContactModel *model = m_service->layer()->m_contactModel; 

    qLog(Telepathy) << "Personal AvatarFile:" << avatarFile << "AvatarStamp:"
            << avatarStamp << "HasPersonalDetails:"
            << model->hasPersonalDetails();
    // Case 1
    if (!avatarFile.isEmpty() && avatarStamp.isValid() && !model->hasPersonalDetails()) {
        qLog(Telepathy) << "Clearing own avatar";
        m_avatars->clearAvatar();
        m_service->layer()->clearAvatar();
        return;
    }

    // Case 4
    if (!model->hasPersonalDetails()) {
        return;
    }

    QContact contact = model->personalDetails();
    QString portraitFile = contact.portraitFile();
    if (portraitFile.isEmpty()) {
        qLog(Telepathy) << "Clearing own avatar";
        m_avatars->clearAvatar();
        m_service->layer()->clearAvatar();
        return;
    }

    portraitFile = Qtopia::applicationFileName("addressbook",
            QString("contactimages/") + portraitFile);
    QFileInfo fileInfo(portraitFile);

    if (!fileInfo.exists()) {
        qLog(Telepathy) << "Clearing own avatar";
        m_avatars->clearAvatar();
        m_service->layer()->clearAvatar();
        return;
    }

    // Case 2: Nothing to do here
    if (portraitFile == avatarFile && avatarStamp == fileInfo.lastModified())
        return;

    setAvatarWithConstraints(portraitFile, fileInfo.lastModified());
}

void TelepathyPresenceProvider::modelChanged()
{
    qLog(Telepathy) << "Contact Model has changed:";
    initializeOwnAvatar();
}

void TelepathyPresenceProvider::updateAvatar(uint contact)
{
    QString uri = m_service->layer()->m_idToUriMap[contact];
    QCollectivePresenceInfo info = peerInfo(uri);
    if (info.isNull())
        return;

    info.setAvatar(m_service->layer()->avatarFileName(contact));
    publishPeerInfo(info);
    QStringList peers;
    peers.append(uri);
    emit peerPresencesChanged(peers);
}

void TelepathyPresenceProvider::avatarUpdated(uint contact, const QString &token)
{
    qLog(Telepathy) << "TelepathyAvatarProvider::avatarUpdated:" << contact << token;

    if (!m_subscribed.contains(contact))
        return;

    if (token.isEmpty()) {
        m_service->layer()->removeToken(contact);
        updateAvatar(contact);
        return;
    }

    QString prevToken = m_service->layer()->m_avatarMap->value(QString::number(contact)
            + "/Token").toString();
    if (prevToken == token) {
        qLog(Telepathy) << "Token for:" << contact << "same as before, not requesting update.";
        return;
    }

    QList<uint> contacts;
    contacts.append(contact);
    m_avatars->requestAvatars(contacts);
}

void TelepathyPresenceProvider::avatarRetrieved(uint contact, const QString &token,
                                              const TelepathyConnectionInterfaceAvatars::AvatarData &data)
{
    qLog(Telepathy) << "Retrieved avatar for:" << contact << token << data.mimeType;

    if (token.isEmpty()) {
        m_service->layer()->removeToken(contact);
        updateAvatar(contact);
        return;
    }

    QString prevToken = m_service->layer()->m_avatarMap->value(QString::number(contact)
            + "/Token").toString();
    if (prevToken == token) {
        qLog(Telepathy) << "Token for:" << contact << "same as before, not storing.";
        return;
    }

    m_service->layer()->storeToken(contact, token, data);
    updateAvatar(contact);
}

const char *TelepathyPresenceProvider::qtImageformatForMimetype(const QString &imageMimeType)
{
    if (imageMimeType == "image/png")
        return "PNG";
    if (imageMimeType == "image/jpeg")
        return "JPG";
    if (imageMimeType == "image/gif")
        return "GIF";

    return 0;
}

class TelepathyNetworkRegistration : public QNetworkRegistrationServer
{
    Q_OBJECT

public:
    TelepathyNetworkRegistration(TelepathyTelephonyService *service);
    ~TelepathyNetworkRegistration();

    void registerToServer();
    void deregisterFromServer();
    void updateRegistrationConfig();

public slots:
    virtual void setCurrentOperator(QTelephony::OperatorMode mode,
                                    const QString &id, const QString &technology);
    virtual void requestAvailableOperators();

private slots:
    void autoRegisterToServer();
    void registrationStateChanged(TelepathyConnection::ConnectionState state,
                                  TelepathyConnection::ConnectionStateReason reason);

private:
    bool m_pendingSetCurrentOperator;
    TelepathyTelephonyService *m_service;
    TelepathyConnection *m_conn;

    bool m_registering;
};

TelepathyNetworkRegistration::TelepathyNetworkRegistration(TelepathyTelephonyService *service)
    : QNetworkRegistrationServer(service->service(), service), m_service(service)
{
    m_pendingSetCurrentOperator = false;
    m_conn = 0;

    m_registering = false;

    QTimer::singleShot(1000, this, SLOT(autoRegisterToServer()));
}

TelepathyNetworkRegistration::~TelepathyNetworkRegistration()
{
    // deletion of m_conn is taken care of here
    deregisterFromServer();
}

void TelepathyNetworkRegistration::autoRegisterToServer()
{
    QSettings config( "Trolltech", m_service->configuration() );
    config.beginGroup( "Registration" );
    if ( config.value( "AutoRegister", false ).toBool() )
        registerToServer();
}

void TelepathyNetworkRegistration::registerToServer()
{
    qLog(Telepathy) << "TelepathyNetworkRegistration::registerToServer" << m_registering << registrationState() << QTelephony::RegistrationHome;

    if (m_registering)
        return;

    if (registrationState() == QTelephony::RegistrationHome) {
        if ( m_pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::OK );
            m_pendingSetCurrentOperator = false;
        }

        return;
    }

    // If no server, then assume that we cannot register at all.
    if (!m_service->parametersAreValid()) {
        qLog(Telepathy) << "Telepathy Registration parameters are not even valid, giving up";
        updateRegistrationState( QTelephony::RegistrationNone );
        if ( m_pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::Error );
            m_pendingSetCurrentOperator = false;
        }
        return;
    }

    updateRegistrationState( QTelephony::RegistrationSearching );

    const QMap<QString, QVariant> &parameters = m_service->parameters();

    QString proto = m_service->protocol();
    TelepathyConnectionManager *mgr = m_service->manager();
    m_conn = mgr->requestConnection(proto, parameters);

    if (!m_conn || !m_conn->isValid()) {
        updateRegistrationState( QTelephony::RegistrationNone );
        if ( m_pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::Error );
            m_pendingSetCurrentOperator = false;
        }
        return;
    }

    m_registering = true;
    QObject::connect(m_conn,
                     SIGNAL(connectionStatusChanged(TelepathyConnection::ConnectionState,TelepathyConnection::ConnectionStateReason)),
                     this,
                     SLOT(registrationStateChanged(TelepathyConnection::ConnectionState,TelepathyConnection::ConnectionStateReason)));

    m_conn->connectToServer();
}

void TelepathyNetworkRegistration::deregisterFromServer()
{
    qLog(Telepathy) << "TelepathyNetworkRegistration::deregisterFromServer";

    TelepathyConnection *conn = m_conn;

    if (conn != NULL) {
        m_service->setConnection(NULL);
        conn->disconnectFromServer();
        delete m_conn;
        m_conn = 0;
    }

    qLog(Telepathy) << "Registration set to None";

    updateRegistrationState(QTelephony::RegistrationNone);
    if (m_pendingSetCurrentOperator) {
        emit setCurrentOperatorResult(QTelephony::OK);
        m_pendingSetCurrentOperator = false;
    }
}

void TelepathyNetworkRegistration::updateRegistrationConfig()
{
    qLog(Telepathy) << "TelepathyNetworkRegistration::updateRegistrationConfig" << m_service->connection();

    if (m_service->connection() != NULL) {
        deregisterFromServer();
        registerToServer();
    }
}

void TelepathyNetworkRegistration::registrationStateChanged(
        TelepathyConnection::ConnectionState state,
        TelepathyConnection::ConnectionStateReason reason)
{
    qLog(Telepathy) << "TelepathyNetworkRegistration::registrationStateChanged" << state << reason;

    // The following case has already been handled in deregisterFromServer()
    if (state == TelepathyConnection::DisconnectedState && reason == TelepathyConnection::Requested)
        return;

    if (state == TelepathyConnection::ConnectingState)
        return;

    if (state == TelepathyConnection::ConnectedState) {
        // Registration is now valid.
        qLog(Telepathy) << "Telepathy registration set to Home";
        updateRegistrationState( QTelephony::RegistrationHome );
        if ( m_pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::OK );
            m_pendingSetCurrentOperator = false;
        }

        m_service->setConnection(m_conn);
        m_registering = false;

        return;
    }

    // Registration Failed

    switch (reason) {
        case TelepathyConnection::None:
        case TelepathyConnection::NetworkError:
        case TelepathyConnection::UnknownError:
        case TelepathyConnection::Requested:
        {
            // Registration has failed for reason other than authentication
            // (e.g. user deregistered) or network error
            qLog(Telepathy) << "Telepathy registration set to None, event type" << reason;
            updateRegistrationState( QTelephony::RegistrationNone );
            if ( m_pendingSetCurrentOperator ) {
                emit setCurrentOperatorResult( QTelephony::Error );
                m_pendingSetCurrentOperator = false;
            }
            m_registering = false;
        }
        break;
        default:
        {
            // Registration authentication failed.
            qLog(Telepathy) << "Telepathy registration set to Denied";
            updateRegistrationState( QTelephony::RegistrationDenied );
            if ( m_pendingSetCurrentOperator ) {
                emit setCurrentOperatorResult( QTelephony::NetworkNotAllowed );
                m_pendingSetCurrentOperator = false;
            }
            m_registering = false;
        }
        break;
    }

    delete m_conn;
    m_conn = 0;
}

void TelepathyNetworkRegistration::setCurrentOperator(QTelephony::OperatorMode mode,
        const QString &id, const QString &technology)
{
    qLog(Telepathy) << "TelepathyNetworkRegistration::setCurrentOperator Got setCurrentOperator:" << mode << id << technology;
    if ( mode == QTelephony::OperatorModeDeregister ) {
        qLog(Telepathy) << "Deregistering...";
        m_pendingSetCurrentOperator = true;
        deregisterFromServer();
    } else {
        qLog(Telepathy) << "Registering...";
        m_pendingSetCurrentOperator = true;
        registerToServer();
    }
}

void TelepathyNetworkRegistration::requestAvailableOperators()
{
    QList<QNetworkRegistration::AvailableOperator> list;
    emit availableOperators(list);
}

class TelepathyConfiguration : public QTelephonyConfiguration
{
    Q_OBJECT

public:
    TelepathyConfiguration(TelepathyTelephonyService *service);
    ~TelepathyConfiguration();

public slots:
    void update(const QString& name, const QString& value);
    void request(const QString& name);

private:
    TelepathyTelephonyService *m_service;
};

TelepathyConfiguration::TelepathyConfiguration(TelepathyTelephonyService *service)
    : QTelephonyConfiguration(service->service(), service, Server)
{
    m_service = service;
}

TelepathyConfiguration::~TelepathyConfiguration()
{
}

void TelepathyConfiguration::update( const QString& name, const QString& )
{
    if ( name == "registration" )
        m_service->updateRegistrationConfig();
}

void TelepathyConfiguration::request( const QString& name )
{
    // Not supported - just return an empty value.
    emit notification( name, QString() );
}

class TelepathyServiceChecker : public QServiceChecker
{
    Q_OBJECT
public:
    explicit TelepathyServiceChecker( TelepathyTelephonyService *service );
    ~TelepathyServiceChecker();

public slots:
    void updateRegistrationConfig();

private:
    TelepathyTelephonyService *m_service;
};

TelepathyServiceChecker::TelepathyServiceChecker( TelepathyTelephonyService *service )
    : QServiceChecker( service->service(), service, Server ), m_service(service)
{
    updateRegistrationConfig();
}

TelepathyServiceChecker::~TelepathyServiceChecker()
{
}

void TelepathyServiceChecker::updateRegistrationConfig()
{
    bool valid = m_service->parametersAreValid();
    setValid(valid);
}

TelepathyTelephonyService::TelepathyTelephonyService(const QString &configuration,
        const QString &manager, const QString &protocol, QObject *parent)
    : QTelephonyService(protocol, parent)
{
    m_configuration = configuration;
    m_protocol = protocol;
    m_connection = NULL;
    m_paramsValid = false;

    m_manager = new TelepathyConnectionManager(manager);

    // Close all existing connections on this manager
    QDBusConnection dbc = QDBusConnection::sessionBus();
    QDBusConnectionInterface *ifc = dbc.interface();

    if (!ifc || !ifc->isValid())
        return;

    QDBusReply<QStringList> running = ifc->call("ListNames");
    if (!running.isValid())
        return;

    QString base("org.freedesktop.Telepathy.Connection.");
    QString basePath("/org/freedesktop/Telepathy/Connection/");

    base.append(manager);
    basePath.append(manager);

    base.append('.');
    basePath.append('/');

    QString proto(protocol);
    proto.replace('-', '_');
    base.append(protocol);
    base.append('.');
    basePath.append(protocol);
    basePath.append('/');

    int midpos = base.length();

    foreach (QString service, running.value()) {
        if (!service.startsWith(base))
            continue;

        QString account = service.mid(midpos);

        QString busName(base);
        busName.append(account);

        QString path(basePath);
        path.append(account);

        TelepathyConnection conn(busName, QDBusObjectPath(path));
        conn.disconnectFromServer();
    }

    updateRegistrationConfig();
}

TelepathyTelephonyService::~TelepathyTelephonyService()
{
    delete m_connection;
    delete m_manager;
}

void TelepathyTelephonyService::initialize()
{
    if ( !supports<QNetworkRegistration>() ) {
        addInterface(new TelepathyNetworkRegistration(this));
    }

    if ( !supports<QServiceChecker>() ) {
        addInterface(new TelepathyServiceChecker(this));
    }

    if ( !supports<QTelephonyConfiguration>() ) {
        addInterface(new TelepathyConfiguration(this));
    }

    QTelephonyService::initialize();
}

void TelepathyTelephonyService::setConnection(TelepathyConnection *conn)
{
    m_connection = conn;

    if (!conn) {
        QCollectivePresence *p = interface<QCollectivePresence>();
        if (p) delete p;
        QCollectiveMessenger *m = interface<QCollectiveMessenger>();
        if (m) delete m;
        delete m_layer;
        m_layer = 0;
        return;
    }

    m_layer = new TelepathyLayer(this, conn);
    QStringList interfaces = conn->interfaces();

    qLog(Telepathy) << "TelepathyTelephonyService: Supported Interfaces:" << interfaces;

    if (interfaces.contains("org.freedesktop.Telepathy.Connection.Interface.Presence") &&
       !supports<QCollectivePresence>() ) {
        qLog(Telepathy) << "Adding presence provider";
        addInterface(new TelepathyPresenceProvider(this));
    }

    if (!supports<QCollectiveMessenger>()) {
        qLog(Telepathy) << "Adding messenger provider";
        addInterface(new TelepathyMessengerProvider(this));
    }
}

TelepathyConnection *TelepathyTelephonyService::connection()
{
    return m_connection;
}

TelepathyLayer *TelepathyTelephonyService::layer()
{
    return m_layer;
}

TelepathyConnectionManager *TelepathyTelephonyService::manager()
{
    return m_manager;
}

QString TelepathyTelephonyService::protocol() const
{
    return m_protocol;
}

QString TelepathyTelephonyService::configuration() const
{
    return m_configuration;
}

static QVariant obtainFromSettings(const QVariant &in, const QString &signature)
{
    switch (signature[0].toLatin1()) {
    case 'b':
        return qVariantFromValue(in.toBool());
    case 'y':
        return qVariantFromValue(in.value<uchar>());
    case 'q':
        return qVariantFromValue(in.value<unsigned short>());
    case 'u':
        return qVariantFromValue(in.toInt());
    case 't':
        return qVariantFromValue(in.toULongLong());
    case 'n':
        return qVariantFromValue(in.value<short>());
    case 'i':
        return qVariantFromValue(in.toInt());
    case 'x':
        return qVariantFromValue(in.toLongLong());
    case 'd':
        return qVariantFromValue(in.toDouble());
    case 's':
        return qVariantFromValue(in.toString());
    default:
        qWarning() << "obtainFromSettings: Unable to process unknown type signature:" << signature;
    };

    return QVariant();
}

void TelepathyTelephonyService::updateRegistrationConfig()
{
    QList<TelepathyConnectionManager::ParamSpec> specs =
            m_manager->parametersForProtocol(m_protocol);

    if (specs.size() == 0) {
        qLog(Telepathy) << "Unable to obtain parameters for protocol:" << m_protocol;
        return;
    }

    m_paramsValid = false;

    QSettings config("Trolltech", m_configuration);
    config.beginGroup("Parameters");

    for (int i = 0; i < specs.size(); i++) {
        qLog(Telepathy) << "Processing parameter:" << specs[i].name <<
                specs[i].flags << specs[i].signature << specs[i].defaultValue;

        if (!config.contains(specs[i].name)) {
            if (specs[i].flags & TelepathyConnectionManager::Required) {
                qLog(Telepathy) << "Parameter required for logon, yet not present in config";
                m_parameters.clear();
                goto failed;
            }

            qLog(Telepathy) << "parameter not present, skipping...";
            continue;
        }

        QVariant from = config.value(specs[i].name);
	if (from.toString().startsWith(":")) {
            // Password is in base64 format
	    from = QVariant(QString::fromUtf8(QByteArray::fromBase64(from.toString().mid(1).toLatin1())));
	}
        QVariant to = obtainFromSettings(from, specs[i].signature);

        if (to.isValid())
            m_parameters[specs[i].name] = to;
    }
    m_paramsValid = true;

failed:
    config.endGroup();

    TelepathyNetworkRegistration *netReg =
            qobject_cast<TelepathyNetworkRegistration *>(interface<QNetworkRegistration>());
    if (netReg)
        netReg->updateRegistrationConfig();

    TelepathyServiceChecker *checker =
            qobject_cast<TelepathyServiceChecker *>(interface<QServiceChecker>());
    if (checker)
        checker->updateRegistrationConfig();
}

bool TelepathyTelephonyService::parametersAreValid() const
{
    return m_paramsValid;
}

const QMap<QString, QVariant> &TelepathyTelephonyService::parameters() const
{
    return m_parameters;
}

class GabbleTelephonyService : public TelepathyTelephonyService
{
public:
    explicit GabbleTelephonyService(QObject *parent = 0);
};

GabbleTelephonyService::GabbleTelephonyService(QObject *parent) :
        TelepathyTelephonyService("GTalk", "gabble", "jabber", parent)
{
}

TelepathyTelephonyServiceQCop::TelepathyTelephonyServiceQCop( QObject *parent )
    : QtopiaAbstractService( "Telephony", parent )
{
    publishAll();
    service = 0;
        service = new GabbleTelephonyService(this);
        service->initialize();
        // Start up the sip-based VoIP telephony service.
        ((QtopiaApplication *)qApp)->registerRunningTask("TelepathyService", this);
}

TelepathyTelephonyServiceQCop::~TelepathyTelephonyServiceQCop()
{
}

void TelepathyTelephonyServiceQCop::start()
{
    qLog(Telepathy) << "TelepathyTelephonyServiceQCop::start()";
    if ( !service ) {
    }
}

void TelepathyTelephonyServiceQCop::stop()
{
    qLog(Telepathy) << "TelepathyTelephonyServiceQCop::stop()";
    if ( service ) {
        delete service;
        service = 0;
        ((QtopiaApplication *)qApp)->unregisterRunningTask("TelepathyService");
    }
}

#include "service.moc"
