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

#include "telepathyconnectioninterfacealiasing.h"
#include "telepathyconnection.h"

#include <qtopialog.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusError>

#define CONNECTION_INTERFACE_ALIASING "org.freedesktop.Telepathy.Connection.Interface.Aliasing"

Q_DECLARE_METATYPE(QList<uint>)

class TelepathyConnectionInterfaceAliasingPrivate
{
public:
    TelepathyConnectionInterfaceAliasingPrivate();
    ~TelepathyConnectionInterfaceAliasingPrivate();

    QDBusInterface *m_iface;
};

class AliasingMessageHelper : public QObject
{
    Q_OBJECT

public:
    AliasingMessageHelper(const QList<uint> contacts,
                          TelepathyConnectionInterfaceAliasing *parent);

private slots:
    void requestAliasError(const QDBusError &error);
    void requestAliasSuccess(const QStringList &list);

private:
    QList<uint> m_contacts;
    TelepathyConnectionInterfaceAliasing *m_parent;
};

/*!
      Internal
*/
AliasingMessageHelper::AliasingMessageHelper(const QList<uint> contacts,
                                             TelepathyConnectionInterfaceAliasing *parent)
    : QObject(parent), m_contacts(contacts), m_parent(parent)
{
}

/*!
      Internal
*/
void AliasingMessageHelper::requestAliasError(const QDBusError &error)
{
    qLog(Telepathy) << "requestAlias resulted in an error" << error;
    delete this;
}

/*!
      Internal
*/
void AliasingMessageHelper::requestAliasSuccess(const QStringList &list)
{
    QMap<uint, QString> map;

    for (int i = 0; i < list.size(); ++i)
        map.insert(m_contacts[i], list[i]);

    emit m_parent->aliasesRetrieved(map);
    delete this;
}

/*!
      Internal
*/
TelepathyConnectionInterfaceAliasingPrivate::TelepathyConnectionInterfaceAliasingPrivate()
{
    m_iface = 0;
}

/*!
      Internal
*/
TelepathyConnectionInterfaceAliasingPrivate::~TelepathyConnectionInterfaceAliasingPrivate()
{
    delete m_iface;
}

/*!
    \class TelepathyConnectionInterfaceAliasing

    \brief Provides an interface to contacts nicknames.
*/

/*!
    \fn aliasesChanged(const QMap<uint, QString> &aliases)

      Signal that gets emitted when aliases \a aliases get changed.

*/

/*!
    \fn aliasesRetrieved(const QMap<uint, QString> &aliases)

    Signal gets emitted when the request for aliases is successful with \a aliases.
*/

/*!
    Constructs a TelepathyConnectionInterfaceAliasing on connection \a conn.
*/

TelepathyConnectionInterfaceAliasing::TelepathyConnectionInterfaceAliasing(TelepathyConnection *conn)
    : QObject(conn)
{
    m_data = new TelepathyConnectionInterfaceAliasingPrivate;

    QDBusConnection dbc = QDBusConnection::sessionBus();

    m_data->m_iface = new QDBusInterface(conn->service(),
                                         conn->path().path(),
                                         CONNECTION_INTERFACE_ALIASING,
                                         dbc);

    dbc.connect(conn->service(), conn->path().path(),
                CONNECTION_INTERFACE_ALIASING, "AliasesChanged",
                this, SLOT(aliasesChanged(QDBusMessage)));
}

/*!
    Destructs TelepathyConnectionInterfaceAliasing
*/
TelepathyConnectionInterfaceAliasing::~TelepathyConnectionInterfaceAliasing()
{
    delete m_data;
}

/*!
    Returns the flags that define the behavior on this connection.
*/
TelepathyConnectionInterfaceAliasing::AliasFlags
        TelepathyConnectionInterfaceAliasing::aliasFlags() const
{
    QDBusReply<uint> reply = m_data->m_iface->call("GetAliasFlags");

    if (!reply.isValid())
        return 0;

    return AliasFlags(reply.value());
}

/*!
    Returns true if successfully requesting the value of the list of contacts \a contacts, otherwise false.
*/
bool TelepathyConnectionInterfaceAliasing::requestAliases(const QList<uint> &contacts)
{
    QVariantList args;
    args.push_back(QVariant::fromValue(contacts));

    AliasingMessageHelper *helper = new AliasingMessageHelper(contacts, this);
    return m_data->m_iface->callWithCallback("RequestAliases", args,
                                             helper,
                                             SLOT(requestAliasSuccess(QStringList)),
                                             SLOT(requestAliasError(QDBusError)));
}

/*!
    Returns true if the request to change the given aliases are successful, otherwise false.
*/
bool TelepathyConnectionInterfaceAliasing::setAliases(const QMap<uint, QString> &aliases)
{
    QDBusArgument arg;
    QMap<uint, QString>::const_iterator it = aliases.begin();

    arg.beginMap(QVariant::UInt, QVariant::String);
    while (it != aliases.end()) {
        arg.beginMapEntry();
        arg << it.key() << it.value();
        arg.endMapEntry();
        ++it;
    }
    arg.endMap();

    m_data->m_iface->call(QDBus::NoBlock, "SetAliases", QVariant::fromValue(arg));

    return true;
}

/*!
    Internal
*/
void TelepathyConnectionInterfaceAliasing::aliasesChanged(const QDBusMessage &msg)
{
    if (msg.arguments().at(0).userType() != qMetaTypeId<QDBusArgument>()) {
        qWarning() << "Received unexpected message for aliasesChanged" << msg;
        return;
    }

    // Need to make a copy in order to de-marshal
    const QDBusArgument in =
        *reinterpret_cast<const QDBusArgument *>(msg.arguments().at(0).constData());

    QMap<uint, QString> aliases;

    in.beginArray();
    while (!in.atEnd()) {
        uint id;
        QString alias;
        in.beginStructure();
        in >> id >> alias;
        in.endStructure();
        aliases.insert(id, alias);
    }
    in.endArray();

    emit aliasesChanged(aliases);
}

#include "telepathyconnectioninterfacealiasing.moc"
