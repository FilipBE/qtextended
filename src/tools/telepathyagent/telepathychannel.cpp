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

#include "telepathychannel.h"
#include "telepathyconnection.h"

#include <qtopialog.h>

#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

#include <QStringList>

#define CHANNEL_INTERFACE "org.freedesktop.Telepathy.Channel"

class TelepathyChannelPrivate
{
public:
    ~TelepathyChannelPrivate();

    QDBusInterface *m_iface;
    QString m_type;
    Telepathy::HandleType m_handleType;
    uint m_handle;
    TelepathyConnection *m_conn;
    QStringList m_interfaces;
    bool m_interfacesCached;
    QDBusObjectPath m_path;
};

TelepathyChannelPrivate::~TelepathyChannelPrivate()
{
    delete m_iface;
}

/*!
  Constructs a null Telepathy Channel

*/
TelepathyChannel::TelepathyChannel()
{
    m_data = 0;
}

/*!
    \class TelepathyChannel
    \brief This class constructs a Telepathy channel.

*/

/*
    Constructs a Telepathy Channel with interface type \a type on D-Bus object path \a path, using the connect \a conn

    \a type can be one of the Telepathy channel types at org.freedesktop.Telepathy.Channel.Type.

*/

TelepathyChannel::TelepathyChannel(const QString &type,
                                   const QDBusObjectPath &path,
                                   TelepathyConnection *conn)
    : QObject(conn)
{
    m_data = new TelepathyChannelPrivate;
    QDBusConnection dbc = QDBusConnection::sessionBus();
    m_data->m_iface = new QDBusInterface(conn->service(),
                                         path.path(), CHANNEL_INTERFACE, dbc);

    QDBusMessage handle = m_data->m_iface->call("GetHandle");

    if (handle.type() == QDBusMessage::ErrorMessage) {
        qLog(Telepathy) << "TelepathyChannel.GetHandle for channel:" <<
                path.path() << "failed with error:" << handle;
        delete m_data;
        m_data = 0;
        return;
    }

    const QList<QVariant> &replyArgs = handle.arguments();
    initialize(type, path,
               static_cast<Telepathy::HandleType>(replyArgs.at(0).toUInt()),
               replyArgs.at(1).toUInt(), conn);
}

/*!
    Constructs a Telepathy channel with interface type \a type on D-Bus object path \a path, with Telepathy handle type \a handleType, and Telepathy handle \a handle, using the connect \a conn
*/
TelepathyChannel::TelepathyChannel(const QString &type, const QDBusObjectPath &path,
                                   Telepathy::HandleType handleType, uint handle,
                                   TelepathyConnection *conn)
    : QObject(conn)
{
    m_data = new TelepathyChannelPrivate;
    QDBusConnection dbc = QDBusConnection::sessionBus();
    m_data->m_iface = new QDBusInterface(conn->service(), path.path(), CHANNEL_INTERFACE, dbc);

    initialize(type, path, handleType, handle, conn);
}

/*!
    Destruct TelepathyChannel
*/
TelepathyChannel::~TelepathyChannel()
{
    delete m_data;
}

/*!
    internal
*/
void TelepathyChannel::initialize(const QString &type, const QDBusObjectPath &path,
                                  Telepathy::HandleType handleType, uint handle,
                                  TelepathyConnection *conn)
{
    m_data->m_conn = conn;
    m_data->m_type = type;
    m_data->m_handleType = handleType;
    m_data->m_handle = handle;
    m_data->m_conn = conn;
    m_data->m_interfacesCached = false;
    m_data->m_path = path;

    QDBusConnection dbc = QDBusConnection::sessionBus();
    dbc.connect(conn->service(), path.path(), CHANNEL_INTERFACE, "Closed",
                this, SLOT(channelClosed()));
}

/*!
    Returns whether the connection is valid.
*/
bool TelepathyChannel::isValid() const
{
    return m_data != 0;
}

/*!
    Returns the handle type for this channel. Corresponds to Telepathy's Handle_Target_Type.
*/
Telepathy::HandleType TelepathyChannel::handleType() const
{
    if (!m_data)
        return Telepathy::None;

    return m_data->m_handleType;
}

/*!
    Returns the handle for this channel. Corresponds to Telepathy's Handle_Type.
*/
uint TelepathyChannel::handle() const
{
    if (!m_data)
        return 0;

    return m_data->m_handle;
}

/*!
    Returns the D-Bus object path for this channel.

*/
QDBusObjectPath TelepathyChannel::path() const
{
    if (!m_data)
        return QDBusObjectPath();

    return m_data->m_path;
}

/*!
    Returns the Telepathy Connection for this channel.
*/
TelepathyConnection *TelepathyChannel::connection()
{
    return m_data->m_conn;
}

/*!
    Returns a QStringList of the interfaces of this channel.
*/
QStringList TelepathyChannel::interfaces() const
{
    if (!m_data)
        return QStringList();

    if (m_data->m_interfacesCached)
        return m_data->m_interfaces;

    QDBusReply<QStringList> reply = m_data->m_iface->call("ListInterfaces");
    if (!reply.isValid())
        return QStringList();

    m_data->m_interfaces = reply.value();
    m_data->m_interfacesCached = true;
    return m_data->m_interfaces;
}

/*!
    Returns a QString of the name of the interface for this channel.
*/
QString TelepathyChannel::type() const
{
    if (!m_data)
        return QString();

    return m_data->m_type;
}

/*!
    \signal
*/
bool TelepathyChannel::close()
{
    if (!m_data)
        return false;

    QDBusReply<void> reply = m_data->m_iface->call("Close");
    if (!reply.isValid())
        return false;

    return true;
}

/*!
    Returns the last error on this channel
    Not implemented.
*/
Telepathy::Error TelepathyChannel::lastError() const
{
    //TODO
    return Telepathy::NoError;
}

/*!
    Returns the last error on this channel as a QString.
    Not implemented.
*/
QString TelepathyChannel::lastErrorString() const
{
    //TODO
    return QString();
}

/*!
    internal
*/
void TelepathyChannel::channelClosed()
{
    delete m_data;
    m_data = 0;
    emit closed();
}
