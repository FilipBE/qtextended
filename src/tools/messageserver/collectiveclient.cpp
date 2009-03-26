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

#include "collectiveclient.h"
#include "qtopialog.h"

#include <private/accountconfiguration_p.h>

#include <QCollectiveSimpleMessage>
#include <QCollectiveMessenger>
#include <QCommServiceManager>
#include <QDateTime>
#include <QMailAccount>
#include <QMailMessage>
#include <QMailStore>
#include <QNetworkRegistration>
#include <QSettings>
#include <QString>
#include <QtopiaIpcAdaptor>


static const int MaxSubjectLength = 256;
static const int RegistrationPeriod = 5 * 1000;


static QString g_accountName("Collective");

// We currently support only the jabber service; multiple services can be supported
// using additional QCollectiveMessenger instances to transmit, and IncomingMessages
// adaptors to receive from
static QString g_serviceName("jabber");
static QString g_channelName("QPE/JabberIncomingMessages");


class CollectiveIncomingMessages : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    CollectiveIncomingMessages(QObject *parent, const QString &);

signals:
    void incomingMessages(const QList<QCollectiveSimpleMessage> &messages);
};

CollectiveIncomingMessages::CollectiveIncomingMessages(QObject *parent, const QString &channel)
    : QtopiaIpcAdaptor(channel, parent)
{
    QtopiaIpcAdaptor::connect(this, MESSAGE(incomingMessages(QList<QCollectiveSimpleMessage>)),
                              this, SIGNAL(incomingMessages(QList<QCollectiveSimpleMessage>)),
                              QtopiaIpcAdaptor::SenderIsChannel);
}


CollectiveClient::CollectiveClient(QObject* parent)
    : TransmittingClient(parent),
      m_manager(new QCommServiceManager(this)),
      m_registration(0),
      m_messenger(0),
      m_registered(false)
{
    connect(m_manager, SIGNAL(servicesChanged()), this, SLOT(servicesChanged()));

    CollectiveIncomingMessages *incoming = new CollectiveIncomingMessages(this, g_channelName);

    connect(incoming, SIGNAL(incomingMessages(QList<QCollectiveSimpleMessage>)),
            this, SLOT(incomingMessages(QList<QCollectiveSimpleMessage>)));

    registrationTimer.setSingleShot(true);
    connect(&registrationTimer, SIGNAL(timeout()), this, SLOT(registrationTimeout()));

    registerRegistration();
    registerMessenger();
}

CollectiveClient::~CollectiveClient()
{
}

void CollectiveClient::registerRegistration()
{
    if (!m_registration) {
        m_registration = new QNetworkRegistration(g_serviceName, this);
        if (m_registration->available()) {
            registrationStateChanged();

            connect(m_registration, SIGNAL(registrationStateChanged()), this, SLOT(registrationStateChanged()));
        } else {
            delete m_registration;
            m_registration = 0;
        }
    }
}

void CollectiveClient::registerMessenger()
{
    if (!m_messenger) {
        m_messenger = new QCollectiveMessenger(g_serviceName, this);
        if (m_messenger->available()) {
            m_messenger->registerIncomingHandler(QString("Channel:%1").arg(g_channelName));

            connect(m_messenger, SIGNAL(disconnected()), this, SLOT(messengerDisconnected()));

            // Transmit any waiting messages
            if (!m_messages.isEmpty())
                transmitMessages();
        } else {
            delete m_messenger;
            m_messenger = 0;
        }
    }
}

void CollectiveClient::servicesChanged()
{
    QStringList interfaces(m_manager->interfaces(g_serviceName));

    if (!m_registration && interfaces.contains("QNetworkRegistration")) {
        registerRegistration();
    }

    if (!m_messenger && interfaces.contains("QCollectiveMessenger")) {
        registerMessenger();
    }
}

void CollectiveClient::registrationStateChanged()
{
    if (m_registration) {
        QTelephony::RegistrationState state = m_registration->registrationState();

        if (state == QTelephony::RegistrationHome || 
            state == QTelephony::RegistrationRoaming || 
            state == QTelephony::RegistrationUnknown) {
            // We have registered with a network
            m_registered = true;
            updateAccountSettings();
            
            // Transmit any waiting messages
            if (!m_messages.isEmpty())
                transmitMessages();
        } else {
            m_registered = false;
        }
    }
}

void CollectiveClient::messengerDisconnected()
{
    delete m_messenger;
    m_messenger = 0;
    m_registered = false;
}

void CollectiveClient::checkForNewMessages()
{
    // There are no new messages that we haven't retrieved yet
    emit allMessagesReceived();
}

bool CollectiveClient::hasDeleteImmediately() const
{
    return true;
}

void CollectiveClient::setAccount(const QMailAccountId &id)
{
    m_accountId = id;

    QString accountName = QMailAccount(m_accountId).accountName();
    if (accountName == g_accountName) {
        updateAccountSettings();
    } else {
        qLog(Messaging) << "Unknown collective account:" << accountName;
    }
}

QMailAccountId CollectiveClient::account() const
{
    return m_accountId;
}

void CollectiveClient::newConnection()
{
    if (m_registered && m_messenger) {
        transmitMessages();
    } else {
        // Wait a short while for the connection to come up
        registrationTimer.start(RegistrationPeriod);
    }
}

int CollectiveClient::addMail(const QMailMessage& message)
{
    if (!m_registration) {
        qLog(Messaging) << "Cannot addMail - collective client not available";
        return QMailMessageServer::ErrConnectionNotReady;
    }

    QStringList identifiers;
    foreach (const QMailAddress &addr, message.to()) {
        QString service, identifier;
        QCollective::decodeUri(addr.address(), service, identifier);
        
        if (service != g_serviceName) {
            if (service.isEmpty()) {
                qLog(Messaging) << "Cannot addMail - cannot transmit via unknown service:" << service;
            } else {
                qLog(Messaging) << "Cannot addMail - cannot transmit to address without service specification";
            }

            return QMailMessageServer::ErrInvalidAddress;
        }
        
        identifiers.append(identifier);
    }

    QCollectiveSimpleMessage msg;

    msg.setFrom(m_fromAddress);
    msg.setText(message.body().data());
    msg.setTimestamp(message.date().toLocalTime());

    m_messages.append(qMakePair(msg, qMakePair(identifiers, message.id())));

    return QMailMessageServer::ErrNoError;
}

void CollectiveClient::errorHandling(int errorCode, QString msg)
{
    if (msg.isEmpty())
        msg = tr("Error occurred");

    m_messages.clear();

    emit updateStatus(QMailMessageServer::None, msg);
    emit errorOccurred(errorCode, msg);
}

static QMailAddress addressFromIdentifier(const QString& identifier, const QString& service)
{
    return QMailAddress(QString(), QCollective::encodeUri(service, identifier));
}

void CollectiveClient::incomingMessages(const QList<QCollectiveSimpleMessage> &messages)
{
    foreach (const QCollectiveSimpleMessage &msg, messages) {
        // Create a message from the data
        QMailMessage message;
        message.setMessageType(QMailMessage::Instant);
        message.setStatus(QMailMessage::Downloaded, true);
        message.setDate(QMailTimeStamp(msg.timestamp()));
        message.setTo(addressFromIdentifier(msg.to(), g_serviceName));
        message.setFrom(addressFromIdentifier(msg.from(), g_serviceName));
        message.setFromAccount(g_accountName);
        message.setContent(QMailMessage::PlainTextContent);

        // Ensure there are no hard line breaks in the message text
        QString text = msg.text();
        text.replace(QMailMessage::CRLF, "\n");

        QMailMessageContentType type("text/plain; charset=UTF-8");
        message.setBody(QMailMessageBody::fromData(text, type, QMailMessageBody::Base64));

        // Give the message a truncated subject if necessary
        if (text.length() > MaxSubjectLength) {
            // Append elipsis character
            message.setSubject(text.left(MaxSubjectLength) + QChar(0x2026));
            message.appendHeaderField("X-qtopia-internal-truncated-subject", "true");
        } else {
            message.setSubject(text);
        }

        // Hand the message off for processing
        emit newMessage(message, false);
    }

    emit allMessagesReceived();
}

void CollectiveClient::cancelTransfer()
{
    errorHandling(QMailMessageServer::ErrCancel, tr("Cancelled by user"));
}

void CollectiveClient::updateAccountSettings()
{
    if (m_accountId.isValid()) {
        // See if the settings have changed since we last loaded them
        QSettings config("Trolltech", "GTalk");

        config.beginGroup("Parameters");
        QString from = config.value("account", QString()).toString();
        config.endGroup();

        AccountConfiguration accountConfig(m_accountId);
        if (accountConfig.emailAddress() != from) {
            // The outgoing address is the only parameter that affects us
            accountConfig.setEmailAddress(from);

            QMailAccount account(m_accountId);
            QMailStore::instance()->updateAccount(&account, &accountConfig);
        }

        m_fromAddress = accountConfig.emailAddress();
    }
}

void CollectiveClient::registrationTimeout()
{
    if (!m_registered) {
        qLog(Messaging) << "Cannot transmit - collective client not registered";
        errorHandling(QMailMessageServer::ErrConnectionNotReady, tr("Network registration unavailable"));
    }
}

void CollectiveClient::transmitMessages()
{
    if (m_registered && m_messenger) {
        // We're not waiting any longer
        registrationTimer.stop();

        foreach (const MessageData &datum, m_messages) {
            QCollectiveSimpleMessage msg(datum.first);

            foreach (const QString &identifier, datum.second.first) {
                msg.setTo(identifier);
                m_messenger->sendMessage(msg);
            }

            emit messageTransmitted(datum.second.second);
            emit messageProcessed(datum.second.second);
        }

        m_messages.clear();
    }
}

#include "collectiveclient.moc"

