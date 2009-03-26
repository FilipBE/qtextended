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

#include "telepathyconnection.h"
#include "telepathychannel.h"

#include <qtopialog.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusArgument>

#include <QString>
#include <QList>
#include <QVariant>
#include <QStringList>

// QDBus library implements this, so no need to Q_IMPLEMENT
// Why is it not in a public header there?
Q_DECLARE_METATYPE(QList<uint>)

#define CONN_INTERFACE "org.freedesktop.Telepathy.Connection"

class TelepathyConnectionPrivate
{
public:
    QDBusInterface *m_connection;
    bool m_connectInProgress;
};

/*!
    \class TelepathyConnection
    \brief This class respresents a telepathy connection.

    Manages the channels opened on this connection.
*/

/*!
    \fn connectionStatusChanged(TelepathyConnection::ConnectionState state,
                                 TelepathyConnection::ConnectionStateReason)

     Signal is emitted when the connection state has changed with \a state and with the reason \a reason.
*/

/*!
    \fn newChannel(const TelepathyConnection::ChannelInfo &info, bool suppressHandler)

    Signal is emitted when the request for a new channel is successful with channel info \a info and \a suppressHandler.
*/

/*!
    Constructs a Telepathy connection with a name of \a busName and object path of \a path on org.freedesktop.Telepathy.Connection.busName

*/
TelepathyConnection::TelepathyConnection(const QString &busName, const QDBusObjectPath &path,
                               QObject *parent)
    : QObject(parent)
{
    QDBusConnection dbc = QDBusConnection::sessionBus();

    m_data = new TelepathyConnectionPrivate;
    m_data->m_connection = new QDBusInterface(busName, path.path(), CONN_INTERFACE, dbc);
    m_data->m_connectInProgress = false;

    dbc.connect(busName, path.path(), CONN_INTERFACE, "StatusChanged",
                this, SLOT(connectionStatusChanged(uint,uint)));
    dbc.connect(busName, path.path(), CONN_INTERFACE, "NewChannel",
                this, SLOT(newChannel(QDBusObjectPath,QString,uint,uint,bool)));
}

/*!
      Destructs TelepathyConnection
*/
TelepathyConnection::~TelepathyConnection()
{
    delete m_data->m_connection;
    delete m_data;
}

/*!
  Returns true if the connection is valid, otherwise false.
*/
bool TelepathyConnection::isValid() const
{
    return m_data->m_connection && m_data->m_connection->isValid();
}

/*!
  Returns the D-Bus connection interface path.
*/
QDBusObjectPath TelepathyConnection::path() const
{
    return QDBusObjectPath(m_data->m_connection->path());
}


/*!
  Returns the connection service.
*/
QString TelepathyConnection::service() const
{
    return m_data->m_connection->service();
}

/*!
  Returns the connections Telepathy connection status.
*/
TelepathyConnection::ConnectionState TelepathyConnection::state() const
{
    if (!m_data->m_connection->isValid())
        return TelepathyConnection::DisconnectedState;

    QDBusReply<uint> reply = m_data->m_connection->call("GetStatus");
    if (!reply.isValid())
        return TelepathyConnection::DisconnectedState;

    return static_cast<TelepathyConnection::ConnectionState>(reply.value());
}

/*!
    Connects to the D-Bus server.
*/
bool TelepathyConnection::connectToServer()
{
    if (!m_data->m_connection->isValid()) {
        qLog(Telepathy) << "Could not open Connection Interface";
        return false;
    }

    if (m_data->m_connectInProgress) {
        return true;
    }

    QList<QVariant> args;
    m_data->m_connection->callWithCallback("Connect", args, this,
                                    SLOT(connectSucceeded(QDBusMessage)),
                                    SLOT(connectFailed(QDBusError)));

    return true;
}

/*!
  Internal

*/
void TelepathyConnection::connectSucceeded(const QDBusMessage &)
{
    m_data->m_connectInProgress = false;
}

/*!
  Internal
*/
void TelepathyConnection::connectFailed(const QDBusError &error)
{
    qLog(Telepathy) << "Connect failed with error:" << error;
    m_data->m_connectInProgress = false;
}

/*
  Disconnect from the D_Bus server.
*/
void TelepathyConnection::disconnectFromServer()
{
    m_data->m_connection->call("Disconnect");
}

/*
  Internal
*/
void TelepathyConnection::connectionStatusChanged(uint status, uint reason)
{
    TelepathyConnection::ConnectionState connState =
            static_cast<TelepathyConnection::ConnectionState>(status);
    TelepathyConnection::ConnectionStateReason connReason =
            static_cast<TelepathyConnection::ConnectionStateReason>(reason);

    emit connectionStatusChanged(connState, connReason);
}

/*
  Returns a list of the protocols on this connection.
*/
QString TelepathyConnection::protocol() const
{
    QDBusReply<QString> reply = m_data->m_connection->call("GetProtocol");

    if (!reply.isValid())
        return QString();

    return reply.value();
}

/*
  Returns a list of the connection's interfaces
*/
QStringList TelepathyConnection::interfaces() const
{
    QDBusReply<QStringList> reply = m_data->m_connection->call("GetInterfaces");

    if (!reply.isValid()) {
        return QStringList();
    }

    return reply.value();
}

/*
  Returns a list of the channels associated with this connection.
*/
QList<TelepathyConnection::ChannelInfo> TelepathyConnection::channels() const
{
    QDBusReply<QDBusArgument> reply = m_data->m_connection->call("ListChannels");

    QList<TelepathyConnection::ChannelInfo> ret;

    if (!reply.isValid()) {
        qLog(Telepathy) << "TelepathyConnection::Error occurred while trying to obtain channel listing" << reply.error();
        return ret;
    }

    const QDBusArgument info = reply.value();
    info.beginArray();
    while (!info.atEnd()) {
        TelepathyConnection::ChannelInfo c;
        QString path;
        uint t;

        info.beginStructure();
        info >> path >> c.type >> t >> c.handle;
        info.endStructure();

        c.path = QDBusObjectPath(path);
        c.handleType = static_cast<Telepathy::HandleType>(t);

        ret.push_back(c);
    }
    info.endArray();

    return ret;
}

/*!
    Requests a channel with \a type, \a handleType and \a handle.  The
    \a suppressHandler parameter, if true, indicates that the caller
    will take the responsibility for handling this channel.  The
    \a receiver and \a slot specify the object and the slot to call
    when the channel is created successfully.  The slot should take
    a QDBusObjectPath parameter, which will hold the channel object
    path of the created channel.
*/
bool TelepathyConnection::requestChannel(const QString &type,
        Telepathy::HandleType handleType, uint handle, bool suppressHandler,
                                        QObject *receiver, const char *slot)
{
    QVariantList args;
    args.push_back(type);
    args.push_back(static_cast<uint>(handleType));
    args.push_back(handle);
    args.push_back(suppressHandler);

    return m_data->m_connection->callWithCallback("RequestChannel", args, receiver, slot);
}


/*!
    Returns the handle id for this connection.  The handle type
    is always Telepathy::NoneHandle.

    If an error occurs, a zero handle is returned.
*/
uint TelepathyConnection::selfHandle() const
{
    QDBusReply<uint> reply = m_data->m_connection->call("GetSelfHandle");
// depreciated, use GetAll
    if (!reply.isValid())
        return 0;

    return reply.value();
}

/*!
  Returns a list of the handles on this connection of type \a type for the handles \a handles.
*/
QList<uint> TelepathyConnection::requestHandles(Telepathy::HandleType type,
        const QStringList &handles)
{
    QDBusReply<QList<uint> > reply = m_data->m_connection->call("RequestHandles",
            static_cast<uint>(type), handles);

    if (!reply.isValid())
        return QList<uint>();

    return reply.value();
}

/*!
  Returns a list of QStrings of handles on this connection of type \a type for the handles \a handles.
*/
QStringList TelepathyConnection::inspectHandles(Telepathy::HandleType type,
                                               const QList<uint> &handles) const
{
    QDBusReply<QStringList> reply = m_data->m_connection->call("InspectHandles",
            static_cast<uint>(type), QVariant::fromValue(handles));

    if (!reply.isValid())
        return QStringList();

    return reply.value();
}

/*!
  Notify connection manager that this connection is holding references of the handle type \a handleType to the handles \a handles.
*/
bool TelepathyConnection::holdHandles(Telepathy::HandleType type,
                                      const QList<uint> &handles)
{
    QDBusReply<void> reply = m_data->m_connection->call("HoldHandles",
            static_cast<uint>(type), QVariant::fromValue(handles));

    if (!reply.isValid())
        return false;

    return true;
}

/*!
  Notify the connection manager that this connection is no longer holding references of the handle type \a handleType to the handles \a handle.
*/
bool TelepathyConnection::releaseHandles(Telepathy::HandleType type,
                                         const QList<uint> &handles)
{
    QDBusReply<void> reply = m_data->m_connection->call("ReleaseHandles",
            static_cast<uint>(type), QVariant::fromValue(handles));

    if (!reply.isValid())
        return false;

    return true;
}

/*!
  Internal
*/
void TelepathyConnection::newChannel(const QDBusObjectPath &path, const QString &type,
                    uint handleType, uint handle, bool suppressHandler)
{
    TelepathyConnection::ChannelInfo info;
    info.path = path;
    info.type = type;
    info.handleType = static_cast<Telepathy::HandleType>(handleType);
    info.handle = handle;

    emit newChannel(info, suppressHandler);
}
