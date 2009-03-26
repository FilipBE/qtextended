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

#include <qbluetoothauthorizationagent.h>

#include <QBluetoothAddress>

#include <QList>
#include <qglobal.h>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusAbstractAdaptor>
#include <QDebug>

#include <stdio.h>
#include <string.h>

class AuthorizationAgentDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.AuthorizationAgent")

public:
    AuthorizationAgentDBusAdaptor(QObject *parent);
    ~AuthorizationAgentDBusAdaptor();
};

AuthorizationAgentDBusAdaptor::AuthorizationAgentDBusAdaptor(QObject *parent) :
         QDBusAbstractAdaptor(parent)
{

}

AuthorizationAgentDBusAdaptor::~AuthorizationAgentDBusAdaptor()
{

}

class QBluetoothAuthorizationAgentPrivate : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.AuthorizationAgent")

public:
    QBluetoothAuthorizationAgentPrivate(QBluetoothAuthorizationAgent *parent,
                                        const QString &name);
    QString m_name;
    QBluetoothAuthorizationAgent *m_parent;
    QBluetoothAuthorizationAgent::Error m_error;

    bool registerAgent(const QString &localAdapter);
    bool unregisterAgent(const QString &localAdapter);
    void handleError(const QDBusError &error);

public slots:
    void Authorize(const QString &path,
                   const QString &address,
                   const QString &service_path,
                   const QString &uuid,
                   const QDBusMessage &msg);
    void Cancel(const QString &path,
                const QString &address,
                const QString &service_path,
                const QString &uuid);
    void Release();
};

QBluetoothAuthorizationAgentPrivate::QBluetoothAuthorizationAgentPrivate(QBluetoothAuthorizationAgent *parent,
                                                                         const QString &name)
    : QObject(parent)
{
    m_parent = parent;
    m_name = name;
    m_error = QBluetoothAuthorizationAgent::NoError;

    QDBusConnection dbc = QDBusConnection::systemBus();
    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }


    QString path = m_name;
    path.prepend('/');

    new AuthorizationAgentDBusAdaptor(this);
    dbc.registerObject(path, this, QDBusConnection::ExportNonScriptableSlots);
}

struct bluez_error_mapping
{
    const char *name;
    QBluetoothAuthorizationAgent::Error error;
};

static bluez_error_mapping bluez_errors[] = {
    { "org.bluez.Error.AlreadyExists", QBluetoothAuthorizationAgent::AlreadyExists },
    { "org.bluez.Error.DoesNotExist", QBluetoothAuthorizationAgent::DoesNotExist },
    { NULL, QBluetoothAuthorizationAgent::NoError }
};

void QBluetoothAuthorizationAgentPrivate::handleError(const QDBusError &error)
{
    m_error = QBluetoothAuthorizationAgent::UnknownError;

    int i = 0;
    while (bluez_errors[i].name) {
        if (error.name() == bluez_errors[i].name) {
            m_error = bluez_errors[i].error;
            break;
        }
        i++;
    }
}

void QBluetoothAuthorizationAgentPrivate::Authorize(const QString &path,
                                                    const QString &address,
                                                    const QString &service_path,
                                                    const QString &uuid,
                                                    const QDBusMessage &msg)
{
    Q_UNUSED(uuid)

    QString devname = path.mid(11);
    QBluetoothAddress addr(address);

    msg.setDelayedReply(true);

    QDBusMessage reply;
    if (m_parent->authorize(devname, addr, service_path.mid(1), uuid)) {
        reply = msg.createReply();
    }
    else {
        reply = msg.createErrorReply("org.bluez.Error.Rejected", "Rejected");
    }

    QDBusConnection::systemBus().send(reply);
}

void QBluetoothAuthorizationAgentPrivate::Cancel(const QString &path,
                                                 const QString &address,
                                                 const QString &service_path,
                                                 const QString &uuid)
{
    Q_UNUSED(uuid)

    QBluetoothAddress addr(address);
    m_parent->cancel(path.mid(11), addr, service_path.mid(1), uuid);
}

void QBluetoothAuthorizationAgentPrivate::Release()
{
    m_parent->release();
}

bool QBluetoothAuthorizationAgentPrivate::registerAgent(const QString &localAdapter)
{
    QString bluezAdapter = "/org/bluez";

    if (!localAdapter.isNull()) {
        bluezAdapter.append("/");
        bluezAdapter.append(localAdapter);
    }

    QDBusInterface *iface = new QDBusInterface("org.bluez",
                                               bluezAdapter,
                                               "org.bluez.Security",
                                               QDBusConnection::systemBus());

    if (!iface->isValid())
        return false;

    QString bluezMethod;
    QVariantList args;

    QString path = m_name;
    path.prepend('/');
    args << path;

    QDBusReply<void> reply = iface->callWithArgumentList(QDBus::Block,
            "RegisterDefaultAuthorizationAgent", args);

    if (!reply.isValid()) {
        handleError(reply.error());
        return false;
    }

    return true;
}

bool QBluetoothAuthorizationAgentPrivate::unregisterAgent(const QString &localAdapter)
{
    QString bluezAdapter = "/org/bluez";

    if (!localAdapter.isNull()) {
        bluezAdapter.append("/");
        bluezAdapter.append(localAdapter);
    }

    QDBusInterface *iface = new QDBusInterface("org.bluez",
                                               bluezAdapter,
                                               "org.bluez.Security",
                                               QDBusConnection::systemBus());

    if (!iface->isValid())
        return false;

    QString bluezMethod;
    QVariantList args;

    QString path = m_name;
    path.prepend('/');
    args << path;

    QDBusReply<void> reply = iface->callWithArgumentList(QDBus::Block,
            "UnregisterDefaultAuthorizationAgent", args);

    if (!reply.isValid()) {
        handleError(reply.error());
        return false;
    }

    return true;
}

/*!
    \class QBluetoothAuthorizationAgent
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothAuthorizationAgent class represents an authorization proxy.

    The QBluetoothAuthorizationAgent class provides an abstract interface for
    querying and granting authorization requests whenever remote bluetooth
    devices wish to use local bluetooth services.  It is up to the clients
    of this class to actually establish whether the authorization request
    is granted or denied.  For instance, the client could ask the user by informing
    him or her of the incoming request.

    The authorization agent can be registered as a global for all local adapters
    or for a particular adapter.

    \ingroup qtopiabluetooth
    \sa QBluetoothAddress, QBluetoothLocalDevice
 */

/*!
    \enum QBluetoothAuthorizationAgent::Error
    \brief Possible errors that might occur.

    \value NoError No error.
    \value AlreadyExists Another passkey agent has already been registered.
    \value DoesNotExist The passkey agent has not been registered.
    \value UnknownError An unknown error has occurred.
*/

/*!
    Constructs an authorization agent.  The \a name parameter specifies a
    unique name of the authorization agent.  The name should be a valid identifier
    name, e.g. it cannot contain special characters or start with a number.
    For instance: DefaultAuthAgent.

    The \a parent parameter holds the QObject parent.
*/
QBluetoothAuthorizationAgent::QBluetoothAuthorizationAgent(const QString &name, QObject *parent)
    : QObject(parent)
{
    m_data = new QBluetoothAuthorizationAgentPrivate(this, name);
}

/*!
    Destroys the passkey agent.
*/
QBluetoothAuthorizationAgent::~QBluetoothAuthorizationAgent()
{
    delete m_data;
}

/*!
    Returns the name of the passkey agent as a string.
*/
QString QBluetoothAuthorizationAgent::name() const
{
    return m_data->m_name;
}

/*!
    Returns the last error that has occurred.
*/
QBluetoothAuthorizationAgent::Error QBluetoothAuthorizationAgent::error() const
{
    return m_data->m_error;
}

/*!
    This virtual function will be called whenever the bluetooth system
    has received an authorization request, and the agent is registered to
    handle the particular request.

    The \a localDevice parameter contains the local adapter this request
    came in on.  The \a remote parameter contains the Bluetooth address
    of the remote device requesting authorization, and \a service
    parameter holds the name of the service the authorization is being
    obtained for.  The \a uuid contains an optional uuid of the request.

    This function should return true if the request is to be authorized
    and false otherwise.

    \sa cancel()
*/
bool QBluetoothAuthorizationAgent::authorize(const QString &localDevice,
                                             const QBluetoothAddress &remote,
                                             const QString &service,
                                             const QString &uuid)
{
    Q_UNUSED(localDevice)
    Q_UNUSED(remote)
    Q_UNUSED(service)
    Q_UNUSED(uuid)

    return false;
}

/*!
    This function will be called whenever an authorization request
    has been canceled by the requestor (e.g. connection to the service
    has been dropped).

    The request being cancelled is on \a localDevice and \a remote is
    the address of the remote device.  The \a service is the service
    name of the service canceling the authorization request.  The
    \a uuid contains an optional uuid of the request.

    \sa authorize()
*/
void QBluetoothAuthorizationAgent::cancel(const QString &localDevice,
                                          const QBluetoothAddress &remote,
                                          const QString &service,
                                          const QString &uuid)
{
    Q_UNUSED(localDevice)
    Q_UNUSED(remote)
    Q_UNUSED(service)
    Q_UNUSED(uuid)

    return;
}

/*!
    This function will be called whenever a passkey agent has been
    unregistered.

    \sa unregisterAgent()
*/
void QBluetoothAuthorizationAgent::release()
{

}

/*!
    Register the authorization agent for all local adapters.

    Returns true if the agent cou7ld be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterAgent(), error()
*/
bool QBluetoothAuthorizationAgent::registerAgent()
{
    return m_data->registerAgent(QString());
}

/*!
    Unregister the authorization agent for all local adapters.

    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa registerAgent(), error()
*/
bool QBluetoothAuthorizationAgent::unregisterAgent()
{
    return m_data->unregisterAgent(QString());
}

/*!
    Register the authorization agent for \a localDevice adapter. The
    adapter name can be obtained from the QBluetoothLocalDevice object.

    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterAgent(), error(), QBluetoothLocalDevice
*/
bool QBluetoothAuthorizationAgent::registerAgent(const QString &localDevice)
{
    return m_data->registerAgent(localDevice);
}

/*!
    Unregister the authorization agent for \a localDevice adapter. The
    adapter name can be obtained from the QBluetoothLocalDevice object.

    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterAgent(), error(), QBluetoothLocalDevice
 */
bool QBluetoothAuthorizationAgent::unregisterAgent(const QString &localDevice)
{
    return m_data->unregisterAgent(localDevice);
}

#include "qbluetoothauthorizationagent.moc"
