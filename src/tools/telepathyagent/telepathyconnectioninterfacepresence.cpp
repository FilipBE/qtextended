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

#include "telepathyconnectioninterfacepresence.h"
#include "telepathyconnection.h"

#include <qtopialog.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusError>

#define CONNECTION_INTERFACE_PRESENCE "org.freedesktop.Telepathy.Connection.Interface.Presence"

Q_DECLARE_METATYPE(QList<uint>)

class TelepathyConnectionInterfacePresencePrivate
{
public:
    TelepathyConnectionInterfacePresencePrivate();
    ~TelepathyConnectionInterfacePresencePrivate();

    QDBusInterface *m_iface;
};

/*!
      Internal
*/

TelepathyConnectionInterfacePresencePrivate::TelepathyConnectionInterfacePresencePrivate()
{
    m_iface = 0;
}

/*!
      Internal
*/
TelepathyConnectionInterfacePresencePrivate::~TelepathyConnectionInterfacePresencePrivate()
{
    delete m_iface;
}

/*!
    \class TelepathyConnectionInterfacePresence

    \brief Provides presence information for this connection.
*/

/*!
    \fn presenceUpdate(const TelepathyConnectionInterfacePresence::ContactPresences &presences)

    Signal get emitted when the contacts presence staus gets updated.
*/

/*!
    Constructs a TelepathyConnectionInterfacePresence for connection \a conn.
*/
TelepathyConnectionInterfacePresence::TelepathyConnectionInterfacePresence(TelepathyConnection *conn)
{
    m_data = new TelepathyConnectionInterfacePresencePrivate;

    QDBusConnection dbc = QDBusConnection::sessionBus();
    m_data->m_iface = new QDBusInterface(conn->service(),
                                         conn->path().path(),
                                         CONNECTION_INTERFACE_PRESENCE,
                                         dbc);

    dbc.connect(conn->service(), conn->path().path(),
                CONNECTION_INTERFACE_PRESENCE, "PresenceUpdate",
                this, SLOT(presenceUpdate(QDBusMessage)));
}

/*!
  Destructs TelepathyConnectionInterfacePresence
*/
TelepathyConnectionInterfacePresence::~TelepathyConnectionInterfacePresence()
{
    delete m_data;
}

/*!
    Returns true if the request to set the presence status \a status is successful, otherwise false.
*/
bool TelepathyConnectionInterfacePresence::setStatus(const QString &status,
        const QVariantMap &parameters)
{
    QDBusArgument arg;

    arg.beginMap(QVariant::String, qMetaTypeId<QVariantMap>());
    arg.beginMapEntry();
    arg << status << parameters;
    arg.endMapEntry();
    arg.endMap();

    QDBusReply<void> reply = m_data->m_iface->call("SetStatus", QVariant::fromValue(arg));

    if (!reply.isValid())
        return false;

    return true;
}

/*!
    Returns true if the request to clear the user's presence statuses is successful, otherwise false.
*/
bool TelepathyConnectionInterfacePresence::clearStatus()
{
    QDBusReply<void> reply = m_data->m_iface->call("ClearStatus");

    if (!reply.isValid())
        return false;

    return true;
}

/*!
    Requests the available statuses for this connection.  The connection
    must be in connected state in order to obtain the status.  If an error
    occurred or no statuses are available, an empty map is returned.
*/
QMap<QString, TelepathyConnectionInterfacePresence::StatusInfo>
         TelepathyConnectionInterfacePresence::availableStatuses() const
{
    QDBusMessage reply = m_data->m_iface->call("GetStatuses");

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "GetStatuses failed with error:" << reply;
        return QMap<QString, StatusInfo>();
    }

    const QDBusArgument &arg =
        *reinterpret_cast<const QDBusArgument *>(reply.arguments().at(0).constData());

    QMap<QString, StatusInfo> map;

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        StatusInfo info;
        uint type;

        arg.beginMapEntry();
        arg >> key;
        arg.beginStructure();
        arg >> type >> info.maySetOnSelf >> info.exclusive >> info.parameter_types;
        arg.endStructure();
        arg.endMapEntry();
        info.type = static_cast<PresenceType>(type);

        map.insert(key, info);
    }
    arg.endMap();

    return map;
}

/*!
    Returns true if the request to update the users last time of activity is updated, otherwise false.
*/
bool TelepathyConnectionInterfacePresence::setLastActivityTime(const QDateTime &time)
{
    uint t = time.toTime_t();
    QDBusReply<void> reply = m_data->m_iface->call("SetLastActivityTime", t);

    if (!reply.isValid())
        return false;

    return true;
}

/*!
    Returns the contacts presences.
*/
static TelepathyConnectionInterfacePresence::ContactPresences
        parsePresences(const QDBusArgument &arg)
{
    // Telepathy uses a bloody complicated map for passing
    // presence information, unfortunately only one
    // map entry is actually ever used/produced by telepathy-glib
    // based services, and multiple entries are deprecated in the
    // Telepathy-0.17 spec.  We handle this here

    TelepathyConnectionInterfacePresence::ContactPresences presences;
    arg.beginMap();
    while (!arg.atEnd()) {
        uint contact;
        uint t;
        QVariantMap params;
        QString status;

        arg.beginMapEntry();
        arg >> contact;
        arg.beginStructure();
        arg >> t;
        arg.beginMap();
        arg.beginMapEntry();
        arg >> status;
        arg >> params;
        arg.endMapEntry();
        arg.endMap();
        arg.endStructure();
        arg.endMapEntry();

        TelepathyConnectionInterfacePresence::LastActivityAndStatuses o;
        if (t != 0)
            o.lastActivity.setTime_t(t);
        o.status = status;
        o.parameters = params;

        presences.insert(contact, o);
    }
    arg.endMap();

    return presences;
}

/*!
    Returns the presence status for the given list of contacts \a contacts.
*/
TelepathyConnectionInterfacePresence::ContactPresences
         TelepathyConnectionInterfacePresence::readCachedPresence(const QList<uint> &contacts)
{
    QDBusMessage reply = m_data->m_iface->call("GetPresence", QVariant::fromValue(contacts));

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "GetStatuses failed with error:" << reply;
        return ContactPresences();
    }

    const QDBusArgument &arg =
        *reinterpret_cast<const QDBusArgument *>(reply.arguments().at(0).constData());

    return parsePresences(arg);
}

/*!
    Returns true if the request for presence status for the list of contacts \a contacts is successful, otherwise false.
*/
bool TelepathyConnectionInterfacePresence::requestPresence(const QList<uint> &contacts)
{
    m_data->m_iface->call(QDBus::NoBlock, "RequestPresence", QVariant::fromValue(contacts));

    return true;
}

/*!
    Internal
*/
void TelepathyConnectionInterfacePresence::presenceUpdate(const QDBusMessage &msg)
{
    // Need to make a copy in order to demarshall
    const QDBusArgument arg =
            *reinterpret_cast<const QDBusArgument *>(msg.arguments().at(0).constData());
    emit presenceUpdate(parsePresences(arg));
}
