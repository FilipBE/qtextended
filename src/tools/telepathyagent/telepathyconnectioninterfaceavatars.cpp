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

#include "telepathyconnectioninterfaceavatars.h"
#include "telepathyconnection.h"

#include <qtopialog.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusError>

#define CONNECTION_INTERFACE_AVATARS "org.freedesktop.Telepathy.Connection.Interface.Avatars"

Q_DECLARE_METATYPE(QList<uint>)

class TelepathyConnectionInterfaceAvatarsPrivate
{
public:
    TelepathyConnectionInterfaceAvatarsPrivate();
    ~TelepathyConnectionInterfaceAvatarsPrivate();

    QDBusInterface *m_iface;
};

/*!
      Internal
*/
TelepathyConnectionInterfaceAvatarsPrivate::TelepathyConnectionInterfaceAvatarsPrivate()
{
    m_iface = 0;
}

/*!
      Internal
*/
TelepathyConnectionInterfaceAvatarsPrivate::~TelepathyConnectionInterfaceAvatarsPrivate()
{
    delete m_iface;
}

/*!
    \class TelepathyConnectionInterfaceAvatars

    \brief An interface for requesting Avatars or contacts images.

*/

/*!
    \fn avatarUpdated(uint contact, const QString &newAvatarToken)

    Signal gets emitted when the avatar for the contact \a contact gets updated with token \a newAvatarToken.
*/

/*!
    \fn avatarRetrieved(uint contact, const QString &token,
                         const TelepathyConnectionInterfaceAvatars::AvatarData &avatar)

    Signal gets emitted when the request for the avatar of the contact \a contact, with token \a token, with avatar image \a avatar.
*/

/*!
    Constructs a TelepathyConnectionInterfaceAvatars on connection \a conn.
*/
TelepathyConnectionInterfaceAvatars::TelepathyConnectionInterfaceAvatars(TelepathyConnection *conn)
    : QObject(conn)
{
    m_data = new TelepathyConnectionInterfaceAvatarsPrivate;

    QDBusConnection dbc = QDBusConnection::sessionBus();

    m_data->m_iface = new QDBusInterface(conn->service(),
                                         conn->path().path(),
                                         CONNECTION_INTERFACE_AVATARS,
                                         dbc);

    dbc.connect(conn->service(), conn->path().path(),
                CONNECTION_INTERFACE_AVATARS, "AvatarUpdated",
                this, SIGNAL(avatarUpdated(uint,QString)));
    dbc.connect(conn->service(), conn->path().path(),
                CONNECTION_INTERFACE_AVATARS, "AvatarRetrieved",
                this, SLOT(avatarRetrieved(QDBusMessage)));
}

/*!
    Descructs TelepathyConnectionInterfaceAvatars
*/
TelepathyConnectionInterfaceAvatars::~TelepathyConnectionInterfaceAvatars()
{
    delete m_data;
}

/*!
    Returns the required format of the avatars on this connection.
*/
TelepathyConnectionInterfaceAvatars::AvatarRequirements
         TelepathyConnectionInterfaceAvatars::avatarRequirements() const
{
    QDBusMessage reply = m_data->m_iface->call("GetAvatarRequirements");
    AvatarRequirements reqs;
    reqs.minimumWidth = 0;
    reqs.minimumHeight = 0;
    reqs.maximumWidth = 0;
    reqs.maximumHeight = 0;
    reqs.maximumSize = 0;

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "GetAvatarRequirements failed with error:" << reply;
        return reqs;
    }

    const QList<QVariant> &replyArgs = reply.arguments();
    reqs.mimeTypes = replyArgs.at(0).toStringList();
    reqs.minimumWidth = replyArgs.at(1).value<ushort>();
    reqs.minimumHeight = replyArgs.at(2).value<ushort>();
    reqs.maximumWidth = replyArgs.at(3).value<ushort>();
    reqs.maximumHeight = replyArgs.at(4).value<ushort>();
    reqs.maximumSize = replyArgs.at(5).toUInt();

    return reqs;
}

/*!
    Returns the tokens of the avatars for all contatcs \a contacts.
 */
QMap<uint, QString> TelepathyConnectionInterfaceAvatars::knownAvatarTokens(const QList<uint> &contacts) const
{
    QMap<uint, QString> ret;
    QDBusMessage reply = m_data->m_iface->call("GetKnownAvatarTokens", QVariant::fromValue(contacts));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "GetAvatarRequirements failed with error:" << reply;
        return ret;
    }

    const QDBusArgument &arg =
        *reinterpret_cast<const QDBusArgument *>(reply.arguments().at(0).constData());

    arg.beginMap();
    while (!arg.atEnd()) {
        QString value;
        uint key;

        arg.beginMapEntry();
        arg >> key;
        arg >> value;
        arg.endMapEntry();

        ret.insert(key, value);
    }

    return ret;
}

/*!
    Returns true if the request to remove the avatar image for this connection is successful, otherwise false.
*/
bool TelepathyConnectionInterfaceAvatars::clearAvatar()
{
    QDBusReply<void> reply = m_data->m_iface->call("ClearAvatar");
    if (reply.isValid())
        return true;

    return false;
}

/*!
    Returns true if the request to set the avatar image \a avatar for this connection is successful, otherwise false.
*/
bool TelepathyConnectionInterfaceAvatars::setAvatar(const AvatarData &avatar)
{
    QDBusReply<void> reply = m_data->m_iface->call("SetAvatar", avatar.imageData, avatar.mimeType);
    if (reply.isValid())
        return true;

    return false;
}

/*!
    Returns the avatar image for the contact \a contact.
*/
TelepathyConnectionInterfaceAvatars::
        AvatarData TelepathyConnectionInterfaceAvatars::requestAvatar(uint contact)
{
//depreciated, use RequestAvatars
    AvatarData resp;

    QDBusMessage reply = m_data->m_iface->call("RequestAvatar", contact);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "RequestAvatar failed:" << QDBusError(reply);
        return AvatarData();
    }

    const QList<QVariant> &replyArgs = reply.arguments();
    resp.imageData = replyArgs.at(0).toByteArray();
    resp.mimeType = replyArgs.at(1).toString();

    return resp;
}

/*!
    Request the avatars for the list of contacts \a contacts.
*/
void TelepathyConnectionInterfaceAvatars::requestAvatars(const QList<uint> &contacts)
{
    m_data->m_iface->call(QDBus::NoBlock, "RequestAvatars", QVariant::fromValue(contacts));
}

/*!
    Internal
*/
void TelepathyConnectionInterfaceAvatars::avatarRetrieved(const QDBusMessage &msg)
{
    AvatarData avatar;
    uint contact = msg.arguments().at(0).toUInt();
    QString token = msg.arguments().at(1).toString();
    avatar.imageData = msg.arguments().at(2).toByteArray();
    avatar.mimeType = msg.arguments().at(3).toString();
    emit avatarRetrieved(contact, token, avatar);
}

#include "telepathyconnectioninterfaceavatars.moc"
