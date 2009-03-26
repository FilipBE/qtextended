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

#include <qtopialog.h>

#include "telepathyconnectionmanager.h"
#include "telepathyconnection.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QVariant>
#include <QStringList>
#include <QSet>

#define CONN_MGR_BASE_SERVICE "org.freedesktop.Telepathy.ConnectionManager"
#define CONN_MGR_BASE_PATH "/org/freedesktop/Telepathy/ConnectionManager"
#define CONN_MGR_INTERFACE "org.freedesktop.Telepathy.ConnectionManager"

class TelepathyConnectionManagerPrivate
{
public:
    QDBusInterface *m_iface;
    QString m_name;
};

/*!
    \class TelepathyConnectionManager
    \brief TelepathyConnectionManager provides a Telepathy connection manager.


    Manages the connection to the Server by using a specified protocol i.e. jabber.
    and implements a D-Bus connection at org.freedesktop.Telepathy.ConnectionManager.cmname
*/

/*!
  Constructs a Telepathy connection manager with the name of \a cmname, with parent \parent.
*/
TelepathyConnectionManager::TelepathyConnectionManager(const QString &cmname, QObject *parent)
    : QObject(parent)
{
    m_data = new TelepathyConnectionManagerPrivate;
    m_data->m_name = cmname;

    QDBusConnection dbc = QDBusConnection::sessionBus();
    QString service(CONN_MGR_BASE_SERVICE);
    service.append(".");
    service.append(cmname);

    QString path(CONN_MGR_BASE_PATH);
    path.append("/");
    path.append(cmname);

    qLog(Telepathy) << "Requesting Manager interface with:" << service << path << CONN_MGR_INTERFACE;
    m_data->m_iface = new QDBusInterface(service, path, CONN_MGR_INTERFACE, dbc);

    dbc.connect(service, path, CONN_MGR_INTERFACE, "NewConnection", this,
                SIGNAL(newConnection(QString,QDBusObjectPath,QString)));
}

/*!
  Destructs TelepathyConnectionManager
*/
TelepathyConnectionManager::~TelepathyConnectionManager()
{
    delete m_data->m_iface;
    delete m_data;
}

/*!
    Returns true if the connection manager is valid, otherwise false.
*/
bool TelepathyConnectionManager::isValid() const
{
    return !m_data->m_name.isEmpty();
}

/*!
    Returns a QStringList of connection managers that are available.
*/
QStringList TelepathyConnectionManager::availableManagers()
{
    QDBusConnection dbc = QDBusConnection::sessionBus();

    QDBusConnectionInterface *ifc = dbc.interface();

    QStringList ret;

    if (!ifc || !ifc->isValid())
        return ret;

    QDBusReply<QStringList> running = ifc->call("ListNames");
    if (!running.isValid())
        return ret;

    QDBusReply<QStringList> activatable = ifc->call("ListActivatableNames");
    if (!activatable.isValid())
        return ret;

    QSet<QString> services;

    QString base(CONN_MGR_BASE_SERVICE);
    base.append(".");
    int midpos = base.length();

    foreach (QString service, running.value()) {
        if (service.startsWith(base))
            services.insert(service.mid(midpos));
    }

    foreach (QString service, activatable.value()) {
        if (service.startsWith(base))
            services.insert(service.mid(midpos));
    }

    return services.toList();
}

/*!
  Returns a StringList of capabilities for the connection manager.
*/
QStringList TelepathyConnectionManager::supportedProtocols() const
{
    if (!isValid())
        return QStringList();

    QDBusReply<QStringList> reply = m_data->m_iface->call("ListProtocols");
    if (!reply.isValid())
        return QStringList();

    return reply;
}

/*!
    Request a connection to the connection manager with the given \a protocol.

*/
TelepathyConnection *TelepathyConnectionManager::requestConnection(const QString &protocol,
        const QMap<QString, QVariant> &parameters)
{
    qLog(Telepathy) << "Requesting connection on protocol:" << protocol;
    if (!isValid())
        return NULL;

    QDBusMessage reply = m_data->m_iface->call("RequestConnection", protocol, parameters);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "RequestConnection failed with error:" << reply;
        return NULL;
    }

    QList<QVariant> replyArgs = reply.arguments();

    QString connService = replyArgs.at(0).toString();
    QDBusObjectPath connPath = qvariant_cast<QDBusObjectPath>(replyArgs.at(1));

    qLog(Telepathy) << "Connection manager returned:" << connService << connPath.path();

    return new TelepathyConnection(connService, connPath);
}

/*!
    Returns the name of the current connection manager.
*/
QString TelepathyConnectionManager::name() const
{
    return m_data->m_name;
}

/*!
    Returns the parameters of the current connection for the given \a proto.
*/
QList<TelepathyConnectionManager::ParamSpec>
        TelepathyConnectionManager::parametersForProtocol(const QString &proto) const
{
    QDBusReply<QDBusArgument> reply = m_data->m_iface->call("GetParameters", proto);

    QList<ParamSpec> ret;

    if (!reply.isValid()) {
        return ret;
    }

    const QDBusArgument params = reply.value();
    params.beginArray();
    while (!params.atEnd()) {
        ParamSpec c;
        uint flags;

        params.beginStructure();
        params >> c.name >> flags >> c.signature >> c.defaultValue;
        params.endStructure();

        c.flags = ParamFlags(flags);

        ret.push_back(c);
    }
    params.endArray();

    return ret;
}
