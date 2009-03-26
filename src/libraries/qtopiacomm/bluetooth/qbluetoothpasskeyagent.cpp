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

#include <qbluetoothpasskeyrequest.h>
#include <qbluetoothpasskeyagent.h>

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


class PasskeyAgentDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.PasskeyAgent")

public:
    PasskeyAgentDBusAdaptor(QObject *parent);
    ~PasskeyAgentDBusAdaptor();
};

PasskeyAgentDBusAdaptor::PasskeyAgentDBusAdaptor(QObject *parent) :
         QDBusAbstractAdaptor(parent)
{

}

PasskeyAgentDBusAdaptor::~PasskeyAgentDBusAdaptor()
{

}

class QBluetoothPasskeyAgent_Private : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.PasskeyAgent")

public:
    QBluetoothPasskeyAgent_Private(QBluetoothPasskeyAgent *parent,
                                   const QString &name);
    QString m_name;
    QBluetoothPasskeyAgent *m_parent;
    QBluetoothPasskeyAgent::Error m_error;

    bool registerAgent(const QString &localAdapter, const QString &addr);
    bool unregisterAgent(const QString &localAdapter, const QString &addr);
    void handleError(const QDBusError &error);

public slots:
    void Request(const QString &path,
                 const QString &address,
                 const QDBusMessage &msg);
    void Confirm(const QString &path,
                 const QString &address,
                 const QString &value,
                 const QDBusMessage &msg);
    void Cancel(const QString &path, const QString &address);
    void Release();
};

QBluetoothPasskeyAgent_Private::QBluetoothPasskeyAgent_Private(QBluetoothPasskeyAgent *parent, const QString &name) : QObject(parent)
{
    m_parent = parent;
    m_name = name;
    m_error = QBluetoothPasskeyAgent::NoError;

    QDBusConnection dbc = QDBusConnection::systemBus();
    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }


    QString path = m_name;
    path.prepend('/');

    new PasskeyAgentDBusAdaptor(this);
    dbc.registerObject(path, this, QDBusConnection::ExportNonScriptableSlots);
}

struct bluez_error_mapping
{
    const char *name;
    QBluetoothPasskeyAgent::Error error;
};

static bluez_error_mapping bluez_errors[] = {
    { "org.bluez.Error.AlreadyExists", QBluetoothPasskeyAgent::AlreadyExists },
    { "org.bluez.Error.DoesNotExist", QBluetoothPasskeyAgent::DoesNotExist },
    { "org.bluez.Error.UnknownAddress", QBluetoothPasskeyAgent::UnknownAddress },
    { NULL, QBluetoothPasskeyAgent::NoError }
};

void QBluetoothPasskeyAgent_Private::handleError(const QDBusError &error)
{
    m_error = QBluetoothPasskeyAgent::UnknownError;

    int i = 0;
    while (bluez_errors[i].name) {
        if (error.name() == bluez_errors[i].name) {
            m_error = bluez_errors[i].error;
            break;
        }
        i++;
    }
}

void QBluetoothPasskeyAgent_Private::Confirm(const QString &path,
                                             const QString &address,
                                             const QString &value,
                                             const QDBusMessage &msg)
{
    QString devname = path.mid(11);
    QBluetoothAddress addr(address);

    msg.setDelayedReply(true);

    QDBusMessage reply;
    if (m_parent->confirmPasskey(devname, addr, value)) {
        reply = msg.createReply();
    }
    else {
        reply = msg.createErrorReply("org.bluez.Error.Rejected", "Rejected");
    }

    QDBusConnection::systemBus().send(reply);
}

void QBluetoothPasskeyAgent_Private::Request(const QString &path,
                                             const QString &address,
                                             const QDBusMessage &msg)
{
    QString devname = path.mid(11);
    QBluetoothAddress addr(address);

    QBluetoothPasskeyRequest req(devname, addr);

    m_parent->requestPasskey(req);

    msg.setDelayedReply(true);

    QDBusMessage reply;
    if (req.isRejected()) {
        reply = msg.createErrorReply("org.bluez.Error.Rejected", "Rejected");
    }
    else {
        reply = msg.createReply(req.passkey());
    }

    QDBusConnection::systemBus().send(reply);
}

void QBluetoothPasskeyAgent_Private::Cancel(const QString &path, const QString &address)
{
    QBluetoothAddress addr( address);
    m_parent->cancelRequest(path.mid(11), addr);
}

void QBluetoothPasskeyAgent_Private::Release()
{
    m_parent->release();
}

bool QBluetoothPasskeyAgent_Private::registerAgent(const QString &localAdapter,
        const QString &addr)
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

    if (addr.isNull()) {
        bluezMethod = "RegisterDefaultPasskeyAgent";
    }
    else {
        bluezMethod = "RegisterPasskeyAgent";
        args << addr;
    }

    QDBusReply<void> reply = iface->callWithArgumentList(QDBus::Block,
            bluezMethod, args);

    if (!reply.isValid()) {
        handleError(reply.error());
        return false;
    }

    return true;
}

bool QBluetoothPasskeyAgent_Private::unregisterAgent(const QString &localAdapter,
        const QString &addr)
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

    if (addr.isNull()) {
        bluezMethod = "UnregisterDefaultPasskeyAgent";
    }
    else {
        bluezMethod = "UnregisterPasskeyAgent";
        args << addr;
    }

    QDBusReply<void> reply = iface->callWithArgumentList(QDBus::Block,
            bluezMethod, args);

    if (!reply.isValid()) {
        handleError(reply.error());
        return false;
    }

    return true;
}

/*!
    \class QBluetoothPasskeyAgent
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothPasskeyAgent class represents a passkey entry proxy.

    The QBluetoothPasskeyAgent class provides an abstract interface for
    requesting and providing passkeys to the bluetooth system whenever a new
    pairing procedure is initiated.  It is up to the clients to actually
    establish how the passkey is obtained.  For instance, the client
    could ask the user, or read passkeys from a file, or by presenting
    the user with a dialog box, or in some other fashion.

    The passkey agent can be registered as a global default, a default for a
    particular local bluetooth adapter, or just for a specific remote device.

    \ingroup qtopiabluetooth
    \sa QBluetoothAddress, QBluetoothLocalDevice, QBluetoothPasskeyRequest
 */

/*!
    \enum QBluetoothPasskeyAgent::Error
    \brief Possible errors that might occur.

    \value NoError No error.
    \value AlreadyExists Another passkey agent has already been registered.
    \value DoesNotExist The passkey agent has not been registered.
    \value UnknownAddress The address of the remote device is unknown.
    \value UnknownError An unknown error has occurred.
*/

/*!
    Constructs a passkey agent.  The \a name parameter specifies a unique name
    of the passkey agent.  The name should be a valid identifier name, e.g.
    it cannot contain special characters or start with a number.
    For instance: DefaultPasskeyAgent.

    The \a parent parameter holds the QObject parent.
*/
QBluetoothPasskeyAgent::QBluetoothPasskeyAgent(const QString &name, QObject *parent)
    : QObject(parent)
{
    m_data = new QBluetoothPasskeyAgent_Private(this, name);
}

/*!
    Destroys the passkey agent.
*/
QBluetoothPasskeyAgent::~QBluetoothPasskeyAgent()
{
    delete m_data;
}

/*!
    Returns the name of the passkey agent as a string.
*/
QString QBluetoothPasskeyAgent::name() const
{
    return m_data->m_name;
}

/*!
    Returns the last error that has occurred.
*/
QBluetoothPasskeyAgent::Error QBluetoothPasskeyAgent::error() const
{
    return m_data->m_error;
}

/*!
    This virtual function will be called whenever the bluetooth system
    has received a request for a passkey, and the agent is registered to
    handle the particular request.

    The \a request parameter contains the passkey request.  Please
    see the QBluetoothPasskeyRequest documentation for more details
    on how to handle the request.

    \sa confirmPasskey(), cancelRequest()
*/
void QBluetoothPasskeyAgent::requestPasskey(QBluetoothPasskeyRequest &request)
{
    Q_UNUSED(request)
}

/*!
    This function will be called whenever a passkey needs to be confirmed by
    the user.  The request came in on \a localDevice and the address of the
    paired device is \a remoteAddr.  The \a localDevice is of the form hciX,
    and can be used to construct a QBluetoothLocalDevice object. The \a passkey
    contains the passkey that should be confirmed.

    The method should return true if the passkeys match, and false otherwise.

    \sa requestPasskey(), cancelRequest()
*/
bool QBluetoothPasskeyAgent::confirmPasskey(const QString &localDevice,
                                            const QBluetoothAddress &remoteAddr,
                                            const QString &passkey)
{
    Q_UNUSED(localDevice)
    Q_UNUSED(remoteAddr)
    Q_UNUSED(passkey)

    return false;
}

/*!
    This function will be called whenever a passkey authentication request
    has failed.

    The request being cancelled is on \a localDevice and the address of the
    paired device is \a remoteAddr.  The \a localDevice is of the form hciX,
    and can be used to construct a QBluetoothLocalDevice object.

    \sa confirmPasskey(), requestPasskey()
*/
void QBluetoothPasskeyAgent::cancelRequest(const QString &localDevice, const QBluetoothAddress &remoteAddr)
{
    Q_UNUSED(localDevice)
    Q_UNUSED(remoteAddr)
}

/*!
    This function will be called whenever a passkey agent has been
    unregistered.

    \sa unregisterDefault(), unregisterForAddress()
*/
void QBluetoothPasskeyAgent::release()
{

}

/*!
    Register the passkey agent as the default agent for all local devices.

    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterDefault(), error()
*/
bool QBluetoothPasskeyAgent::registerDefault()
{
    return m_data->registerAgent(QString(), QString());
}

/*!
    Unregister the passkey agent as the default agent for all local devices.

    Returns true if the agent could be unregistered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa registerDefault(), error()
*/
bool QBluetoothPasskeyAgent::unregisterDefault()
{
    return m_data->unregisterAgent(QString(), QString());
}

/*!
    Register the passkey agent as the default agent for device given
    by \a localDevice.  The \a localDevice is of the form hciX,
    and can be used to construct a QBluetoothLocalDevice object.

    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterDefault(), error()
*/
bool QBluetoothPasskeyAgent::registerDefault(const QString &localDevice)
{
    return m_data->registerAgent(localDevice, QString());
}

/*!
    Unregister the passkey agent as the default agent for device given
    by \a localDevice. The \a localDevice is of the form hciX,
    and can be used to construct a QBluetoothLocalDevice object.

    Returns true if the agent could be unregistered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa registerDefault(), error()
 */
bool QBluetoothPasskeyAgent::unregisterDefault(const QString &localDevice)
{
    return m_data->unregisterAgent(localDevice, QString());
}

/*!
    Register the passkey agent for all local devices. It will only handle
    pairing requests associated with remote device which is given in
    \a addr.

    Note that once pairing is complete, or a timeout has been
    hit, the agent will be automatically unregistered.  The \c release()
    method will be called.  It is up to the application to register the
    agent again.
 
    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterForAddress(), error()
 */
bool QBluetoothPasskeyAgent::registerForAddress(QBluetoothAddress &addr)
{
    return m_data->registerAgent(QString(), addr.toString());
}

/*!
    Unregister the passkey agent for all local devices. This method
    attempts to unregister an agent that would have only handled
    pairing requests associated with remote device at address \a addr.

    Returns true if the agent could be unregistered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa registerForAddress(), error()
 */
bool QBluetoothPasskeyAgent::unregisterForAddress(QBluetoothAddress &addr)
{
    return m_data->unregisterAgent(QString(), addr.toString());
}

/*!
    Register the passkey agent for local device represented by
    \a localDevice. The \a localDevice is of the form hciX,
    and can be used to construct a QBluetoothLocalDevice object.
    It will only handle pairing requests associated with remote
    device which is given in \a addr.

    Note that once pairing is complete, or a timeout has been
    hit, the agent will be automatically unregistered.  The \c release()
    method will be called.  It is up to the application to register the
    agent again.
 
    Returns true if the agent could be registered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa unregisterForAddress(), error()
 */
bool QBluetoothPasskeyAgent::registerForAddress(const QString &localDevice,
         QBluetoothAddress &addr)
{
    return m_data->registerAgent(localDevice, addr.toString());
}

/*!
    Unregister the passkey agent for local device \a localDevice.
    The \a localDevice is of the form hciX, and can be used to construct a
    QBluetoothLocalDevice object. This method attempts to unregister an
    agent that would have only handled pairing requests associated with
    remote device at address \a addr.

    Returns true if the agent could be unregistered successfully, and false otherwise.
    If the call fails, the error can be obtained by calling error().

    \sa registerForAddress(), error()
 */
bool QBluetoothPasskeyAgent::unregisterForAddress(const QString &localDevice,
         QBluetoothAddress &addr)
{
    return m_data->unregisterAgent(localDevice, addr.toString());
}

#include "qbluetoothpasskeyagent.moc"
