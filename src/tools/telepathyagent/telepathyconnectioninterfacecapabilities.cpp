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

#include "telepathyconnectioninterfacecapabilities.h"
#include "telepathyconnection.h"

#include <qtopialog.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusError>

#define CONNECTION_INTERFACE_CAPABILITIES "org.freedesktop.Telepathy.Connection.Interface.Capabilities"

Q_DECLARE_METATYPE(QList<uint>)

class TelepathyConnectionInterfaceCapabilitiesPrivate
{
public:
    TelepathyConnectionInterfaceCapabilitiesPrivate();
    ~TelepathyConnectionInterfaceCapabilitiesPrivate();

    QDBusInterface *m_iface;
};

class CapabilitiesRequestHelper : public QObject
{
    Q_OBJECT

public:
    CapabilitiesRequestHelper(const QList<uint> contacts,
                          TelepathyConnectionInterfaceCapabilities *parent);

private slots:
    void requestCapabilitiesError(const QDBusError &error);
    void requestCapabilitiesSuccess(const QDBusMessage &message);

private:
    QList<uint> m_contacts;
    TelepathyConnectionInterfaceCapabilities *m_parent;
};

/*!
      Internal
*/
CapabilitiesRequestHelper::CapabilitiesRequestHelper(const QList<uint> contacts,
        TelepathyConnectionInterfaceCapabilities *parent) :
        QObject(parent)
        , m_contacts(contacts), m_parent(parent)
{
}

/*!
      Internal
*/
void CapabilitiesRequestHelper::requestCapabilitiesError(const QDBusError &error)
{
    qLog(Telepathy) << "requestAlias resulted in an error" << error;
    delete this;
}

/*!
      Internal
*/
void CapabilitiesRequestHelper::requestCapabilitiesSuccess(const QDBusMessage &msg)
{
    if (msg.arguments().at(0).userType() != qMetaTypeId<QDBusArgument>()) {
        qWarning() << "Received unexpected message for aliasesChanged" << msg;
        return;
    }

    QMap<uint, TelepathyConnectionInterfaceCapabilities::ContactCapability> map;
    const QDBusArgument in =
        *reinterpret_cast<const QDBusArgument *>(msg.arguments().at(0).constData());

    in.beginArray();
    for (int i = 0; i < m_contacts.size(); ++i) {
        TelepathyConnectionInterfaceCapabilities::ContactCapability c;
        uint gf;
        in.beginStructure();
        in >> c.handle >> c.channel_type >> gf >> c.type_specific_flags;
        in.endStructure();

        c.generic_flags =
                static_cast<TelepathyConnectionInterfaceCapabilities::CapabilityFlags>(gf);
        map.insert(m_contacts[i], c);
    }
    in.endArray();

    emit m_parent->capabilitiesRetrieved(map);
    delete this;
}

/*!
      Internal
*/
TelepathyConnectionInterfaceCapabilitiesPrivate::TelepathyConnectionInterfaceCapabilitiesPrivate()
{
    m_iface = 0;
}

/*!
      Internal
*/
TelepathyConnectionInterfaceCapabilitiesPrivate::~TelepathyConnectionInterfaceCapabilitiesPrivate()
{
    delete m_iface;
}

/*!
    \class TelepathyConnectionInterfaceCapabilities

    \brief Provides an interface to the capabilities of the interface for this connection.


*/

/*!
    \fn capabilitiesRetrieved(const QMap<uint,
                               TelepathyConnectionInterfaceCapabilities::ContactCapability> &capabilities);

    Signal gets emitted when the request for capabilities of this connection are retrieved.
*/

/*!
    \fn capabilitiesChanged(const QList<TelepathyConnectionInterfaceCapabilities::CapabilityChange> &capabilities);

    Signal gets emitted when the request to change the capabilites is successful.
*/

/*!
    Constructs a TelepathyConnectionInterfaceCapabilities for the connection \a conn.
*/
TelepathyConnectionInterfaceCapabilities::TelepathyConnectionInterfaceCapabilities(TelepathyConnection *conn)
    : QObject(conn)
{
    m_data = new TelepathyConnectionInterfaceCapabilitiesPrivate;

    QDBusConnection dbc = QDBusConnection::sessionBus();

    m_data->m_iface = new QDBusInterface(conn->service(),
                                         conn->path().path(),
                                         CONNECTION_INTERFACE_CAPABILITIES,
                                         dbc);

    dbc.connect(conn->service(), conn->path().path(),
                CONNECTION_INTERFACE_CAPABILITIES, "CapabilitiesChanged",
                this, SLOT(capabilitiesChanged(QDBusMessage)));
}

/*!
    Destructs TelepathyConnectionInterfaceCapabilities
*/
TelepathyConnectionInterfaceCapabilities::~TelepathyConnectionInterfaceCapabilities()
{
    delete m_data;
}

/*!
    Returns true if the request to indicate the channel type capability was sucecssful, otherwise false.
*/
bool TelepathyConnectionInterfaceCapabilities::advertiseCapabilities(const QList<CapabilityPair> &add,
        const QStringList &remove)
{
    QDBusArgument arg;

    arg.beginArray();
    foreach (CapabilityPair c, add) {
        arg.beginStructure();
        arg << c.channel_type << c.type_specific_flags;
        arg.endStructure();
    }
    arg.endArray();

    m_data->m_iface->call(QDBus::NoBlock, "AdvertiseCapabilities", QVariant::fromValue(arg), remove);

    return true;
}

/*!
    Returns true if the request for capabilities for the list of handles \a handles of this connection are successful, otherwise false.
*/
bool TelepathyConnectionInterfaceCapabilities::retrieveCapabilities(const QList<uint> &handles)
{
    QVariantList args;
    args.push_back(QVariant::fromValue(handles));

    CapabilitiesRequestHelper *helper = new CapabilitiesRequestHelper(handles, this);
    return m_data->m_iface->callWithCallback("GetCapabilities", args,
                                             helper,
                                             SLOT(requestCapabilitiesSuccess(QDBusMessage)),
                                             SLOT(requestCapabilitiesError(QDBusError)));
}

/*!
    Internal
*/
void TelepathyConnectionInterfaceCapabilities::capabilitiesChanged(const QDBusMessage &msg)
{
    if (msg.arguments().at(0).userType() != qMetaTypeId<QDBusArgument>()) {
        qWarning() << "Received unexpected message for aliasesChanged" << msg;
        return;
    }

    const QDBusArgument in =
        *reinterpret_cast<const QDBusArgument *>(msg.arguments().at(0).constData());

    QList<CapabilityChange> ret;

    in.beginArray();
    while (!in.atEnd()) {
        uint handle;
        QString channel;
        uint ogf, ngf, otsf, ntsf;
        in.beginStructure();
        in >> handle >> channel >> ogf >> ngf >> otsf >> ntsf;
        in.endStructure();

        CapabilityChange c;
        c.handle = handle;
        c.channel_type = channel;
        c.old_generic_flags =
                static_cast<TelepathyConnectionInterfaceCapabilities::CapabilityFlags>(ogf);
        c.new_generic_flags =
                static_cast<TelepathyConnectionInterfaceCapabilities::CapabilityFlags>(ngf);
        c.old_type_specific_flags = otsf;
        c.new_type_specific_flags = ntsf;

        ret.append(c);
    }
    in.endArray();

    emit capabilitiesChanged(ret);
}

#include "telepathyconnectioninterfacecapabilities.moc"
