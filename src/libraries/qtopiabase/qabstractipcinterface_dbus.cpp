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

#include <qabstractipcinterface.h>
#include <qvaluespace.h>
#include <QString>
#include <QMap>
#include <qtimer.h>
#include <qmetaobject.h>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

// The QAbstractIpcInterfaceDBusAdaptor class provides a proxy from
// the world of QAbstractIpcInterface to QDBusAbstractAdaptor.
// It creates a simulated QMetaObject that fakes out QDBusAbstractAdaptor
// sufficiently to make it marshal the signals and slots on the
// QAbstractIpcInterface correctly.  We need to do this because
// creating a compile-time D-BUS adaptor stub is not practical.

class QAbstractIpcInterfaceDBusAdaptor : public QDBusAbstractAdaptor
{
    // Do not put Q_OBJECT here.
public:
    QAbstractIpcInterfaceDBusAdaptor
        (QAbstractIpcInterface *parent, const QMetaObject *meta,
         const QString& interfaceName);
    ~QAbstractIpcInterfaceDBusAdaptor();

    const QMetaObject *metaObject() const;
    int qt_metacall(QMetaObject::Call, int, void **);

private:
    QAbstractIpcInterface *parent;
    QMetaObject simulatedMetaObject;
    QByteArray simulatedStringData;
    int *simulatedData;
    int methodOffset, methodCount;
};

// From qmetaobject.cpp in Qt.
struct QMetaObjectPrivate
{
    int revision;
    int className;
    int classInfoCount, classInfoData;
    int methodCount, methodData;
    int propertyCount, propertyData;
    int enumeratorCount, enumeratorData;
};

QAbstractIpcInterfaceDBusAdaptor::QAbstractIpcInterfaceDBusAdaptor
        (QAbstractIpcInterface *parent, const QMetaObject *meta,
         const QString& interfaceName)
    : QDBusAbstractAdaptor(parent)
{
    this->parent = parent;

    // Make it look like we are inheriting from QDBusAbstractAdaptor.
    simulatedMetaObject.d.superdata =
        QDBusAbstractAdaptor::staticMetaObject.d.superdata;

    // Copy the original string data and then add the classinfo strings.
    const char *str = meta->d.stringdata;
    int len = 0;
    if (str[len] != '\0') {
        // Skip usual location of the class name.
        int slen = strlen(str + len);
        len += slen + 1;
    }
    if (str[len] == '\0') {
        // Class name usually followed by a null string.
        ++len;
    }
    while (str[len] != '\0') {
        int slen = strlen(str + len);
        len += slen + 1;
    }
    simulatedStringData = QByteArray(str, len);
    int classInfoName = simulatedStringData.size();
    simulatedStringData += "D-Bus Interface";
    simulatedStringData += (char)'\0';
    int classInfoValue = simulatedStringData.size();
    simulatedStringData += interfaceName.toUtf8();
    simulatedStringData += (char)'\0';
    int adaptorClassName = simulatedStringData.size();
    simulatedStringData += meta->className();   // Original class name
    simulatedStringData += "DBusAdaptor";       // plus "DBusAdaptor".
    simulatedStringData += (char)'\0';
    simulatedStringData += (char)'\0';
    simulatedMetaObject.d.stringdata = simulatedStringData.constData();

    // Create a new data block from the original which only contains
    // the methods we are interested in, plus the new classinfo.
    const QMetaObjectPrivate *metaPrivate =
        reinterpret_cast<const QMetaObjectPrivate *>(meta->d.data);
    methodOffset = meta->methodOffset();
    methodCount = metaPrivate->methodCount;
    int numItems =
        10 +    // Number of items in the QMetaObjectPrivate header.
        methodCount * 5 +
        2;      // Number of items needed for the class info.
    simulatedData = new int [numItems];
    memset(simulatedData, 0, numItems * sizeof(int));
    QMetaObjectPrivate *header = reinterpret_cast<QMetaObjectPrivate *>(simulatedData);
    header->revision = 1;
    header->className = adaptorClassName;
    header->classInfoCount = 1;
    header->classInfoData = numItems - 2;
    header->methodCount = methodCount;
    header->methodData = 10;
    header->propertyCount = 0;
    header->propertyData = 0;
    header->enumeratorCount = 0;
    header->enumeratorData = 0;
    simulatedData[numItems - 2] = classInfoName;
    simulatedData[numItems - 1] = classInfoValue;
    memcpy(simulatedData + header->methodData,
           meta->d.data + metaPrivate->methodData,
           methodCount * 5 * sizeof(int));
    simulatedMetaObject.d.data = reinterpret_cast<const uint *>(simulatedData);

    // There is no extra data for the simulated class.
    simulatedMetaObject.d.extradata = 0;
}

QAbstractIpcInterfaceDBusAdaptor::~QAbstractIpcInterfaceDBusAdaptor()
{
    delete simulatedData;
}

const QMetaObject *QAbstractIpcInterfaceDBusAdaptor::metaObject() const
{
    return &simulatedMetaObject;
}

// Proxy the metacall across to the actual QAbstractIpcInterface object.
int QAbstractIpcInterfaceDBusAdaptor::qt_metacall
        (QMetaObject::Call call, int id, void **args)
{
    id = QDBusAbstractAdaptor::qt_metacall(call, id, args);
    if (id < 0)
        return id;
    parent->qt_metacall(call, id + methodOffset, args);
    return id - methodCount;
}

class QAbstractIpcInterfaceServiceNames
{
public:
    QMap<QString, int> names;
};

Q_GLOBAL_STATIC(QAbstractIpcInterfaceServiceNames, serviceNames);

class QAbstractIpcInterfacePrivate
{
public:
    QAbstractIpcInterfacePrivate(QAbstractIpcInterface::Mode mode)
        : dbusConnection(QDBusConnection::sessionBus())
    {
        this->mode = mode;
        this->dbusInterface = 0;
        this->settings = 0;
        this->inConnectNotify = false;
    }
    ~QAbstractIpcInterfacePrivate()
    {
        delete dbusInterface;
    }

    QString groupName;
    QString interfaceName;
    QString realInterfaceName;
    QString primaryInterfaceName;
    QString valueSpaceLocation;
    QString dbusService;
    QString dbusInterfaceName;
    QString dbusPath;
    QAbstractIpcInterface::Mode mode;
    QDBusConnection dbusConnection;
    QDBusInterface *dbusInterface;
    QValueSpaceObject *settings;
    QString path;
    bool inConnectNotify;
    QList<QByteArray> connectedSignals;
};

QAbstractIpcInterface::QAbstractIpcInterface
        ( const QString& valueSpaceLocation, const QString& interfaceName,
          const QString& groupName, QObject *parent,
          QAbstractIpcInterface::Mode mode )
    : QObject( parent)
{
    QString group = groupName;
    QString valuePath;

    // Create the private data holder.
    d = new QAbstractIpcInterfacePrivate( mode );
    d->interfaceName = interfaceName;
    d->realInterfaceName = interfaceName;
    d->valueSpaceLocation = valueSpaceLocation;
    while ( d->valueSpaceLocation.endsWith("/") )
        d->valueSpaceLocation.chop(1);
    if ( !d->valueSpaceLocation.startsWith( "/" ) )
        d->valueSpaceLocation.prepend( "/" );

    // Use the stripped version of the value space location as a D-BUS identifier.
    QString dbusName = d->valueSpaceLocation;
    dbusName.replace(QChar('/'), "");

    // Determine the path to use for settings and published values.
    d->path = d->valueSpaceLocation + QChar('/') + interfaceName + QChar('/') + group;

    // Determine the D-BUS interface name to use.
    d->dbusInterfaceName =
        QLatin1String("com.trolltech.qtopia.") + dbusName + QChar('.') + interfaceName;

    // Set up the D-BUS marshalling logic in server or client mode.
    if ( mode == Server ) {

        // Server side: register the object in the value space.
        // The actual D-BUS hook up is done in proxyAll().
        d->settings = new QValueSpaceObject( d->path, this );
        d->dbusService = QLatin1String("com.trolltech.qtopia.") +
                         dbusName + QChar('.') + group;
        d->dbusPath = QLatin1String("/com/trolltech/qtopia/") + dbusName + QChar('/') +
                      interfaceName + QChar('/') + group;
        QString channelPath = d->valueSpaceLocation + "/_channels/" +
                              interfaceName + "/" + group;
        QValueSpaceObject *channelObject = new QValueSpaceObject( channelPath, this );
        channelObject->setAttribute( QString(".dbusPath"), d->dbusPath );

    } else {

        // Client side: make sure that we could connect to D-BUS.
        if (!(d->dbusConnection.isConnected())) {
            qWarning() << "Unable to connect to D-BUS:" << d->dbusConnection.lastError();
            QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
            d->mode = Invalid;
            return;
        }

        // Locate the default group name if not specified.
        QValueSpaceItem item( d->valueSpaceLocation + "/_channels/" + interfaceName );
        if ( group.isEmpty() ) {
            // Search for the default group implementing this interface.
            QStringList values = item.subPaths();
            QStringList::ConstIterator iter;
            int bestPriority = -2147483647;
            for ( iter = values.begin(); iter != values.end(); ++iter ) {
                QVariant value = item.value( *iter + "/priority" );
                int priority = ( value.isNull() ? 0 : value.toInt() );
                if ( priority > bestPriority ) {
                    group = QString( *iter );
                    d->dbusPath = item.value( *iter + "/.dbusPath" ).toString();
                    valuePath =
                        item.value( *iter + "/.valuePath" ).toString();
                    bestPriority = priority;
                }
            }
            if ( group.isEmpty() ) {
                // Could not locate an appropriate default.
                QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
                d->mode = Invalid;
                return;
            }
        } else {
            // Use the specified group name to look up the channel.
            d->dbusPath = item.value( group + "/.dbusPath" ).toString();
            valuePath = item.value( group + "/.valuePath" ).toString();
            if ( d->dbusPath.isEmpty() ) {
                QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
                d->mode = Invalid;
                return;
            }
        }

        // Connect to the D-BUS server instance for this interface and group.
        d->dbusService = QLatin1String("com.trolltech.qtopia.") +
                         dbusName + QChar('.') + group;
        d->dbusInterface = new QDBusInterface
            (d->dbusService, d->dbusPath, d->dbusInterfaceName, d->dbusConnection);
        if (!(d->dbusInterface->isValid())) {
            delete d->dbusInterface;
            d->dbusInterface = 0;
            qWarning() << "Could not find" << d->dbusPath << "on D-BUS";
            QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
            d->mode = Invalid;
            return;
        }

        // Determine where the values for the interface will be stored.
        if ( valuePath.isEmpty() ) {
            d->path = d->valueSpaceLocation + QChar('/') + interfaceName +
                      QChar('/') + group;
        } else {
            d->path = valuePath;
        }
    }
    d->groupName = group;
}

QAbstractIpcInterface::~QAbstractIpcInterface()
{
    if (d->mode == Server) {
        d->dbusConnection.send
            (QDBusMessage::createSignal(d->dbusPath, d->dbusInterfaceName,
                                        QLatin1String("ifaceServerDisconnect")));
        QAbstractIpcInterfaceServiceNames *names = serviceNames();
        if (names->names.contains(d->dbusService)) {
            if (--(names->names[d->dbusService]) <= 0) {
                names->names.remove(d->dbusService);
                d->dbusConnection.unregisterService(d->dbusService);
            }
        }
    }
    delete d;
}

QString QAbstractIpcInterface::groupName() const
{
    return d->groupName;
}

QString QAbstractIpcInterface::interfaceName() const
{
    return d->realInterfaceName;
}

QAbstractIpcInterface::Mode QAbstractIpcInterface::mode() const
{
    return d->mode;
}

void QAbstractIpcInterface::setPriority(int value)
{
    if (d->settings) {
        d->settings->setAttribute(QString("priority"), value);
        QValueSpaceObject::sync();
    }
}

void QAbstractIpcInterface::proxy(const QByteArray& member)
{
    // Doesn't work for D-BUS yet.
    Q_UNUSED(member);
    Q_ASSERT(false);
}

void QAbstractIpcInterface::proxyAll(const QMetaObject& meta)
{
    // Only needed for the server at present.
    if ( d->mode != Server )
        return;

    // Register the D-BUS service name if this is the first interface
    // for this service in the system.
    QAbstractIpcInterfaceServiceNames *names = serviceNames();
    if (names->names.contains(d->dbusService)) {
        ++(names->names[d->dbusService]);
    } else {
        names->names[d->dbusService] = 1;
        if (!d->dbusConnection.registerService(d->dbusService)) {
            qWarning() << "Unable to register" << d->dbusService << "to D-BUS:"
                       << d->dbusConnection.lastError();
        }
    }

    // Create a D-BUS adapator and then register the object onto the bus.
    new QAbstractIpcInterfaceDBusAdaptor(this, &meta, d->dbusInterfaceName);
    if (!d->dbusConnection.registerObject(d->dbusPath, this)) {
        qWarning() << "Unable to publish" << d->dbusPath << "to D-BUS:"
                   << d->dbusConnection.lastError();
    }
}

void QAbstractIpcInterface::proxyAll
        (const QMetaObject& meta, const QString& subInterfaceName)
{
    // Doesn't work for D-BUS yet.
    Q_UNUSED(meta);
    Q_UNUSED(subInterfaceName);
    Q_ASSERT(false);
}

// Get the name of a Qt method in D-BUS form.
static QString dbusMethodName(const QByteArray& name)
{
    int index = name.indexOf((char)'(');
    if (index >= 1)
        return QString::fromUtf8(name.constData() + 1, index - 1);
    else
        return QString();
}

QtopiaIpcSendEnvelope QAbstractIpcInterface::invoke(const QByteArray& name)
{
    if (d->mode == Client)
        return QtopiaIpcSendEnvelope(d->dbusInterface, dbusMethodName(name));
    else
        return QtopiaIpcSendEnvelope();
}

void QAbstractIpcInterface::invoke(const QByteArray& name, QVariant arg1)
{
    if (d->mode == Client) {
        d->dbusInterface->call
            (QDBus::NoBlock, dbusMethodName(name), arg1);
    }
}

void QAbstractIpcInterface::invoke(const QByteArray& name, QVariant arg1, QVariant arg2)
{
    if (d->mode == Client) {
        d->dbusInterface->call
            (QDBus::NoBlock, dbusMethodName(name), arg1, arg2);
    }
}

void QAbstractIpcInterface::invoke(const QByteArray& name, QVariant arg1, QVariant arg2, QVariant arg3)
{
    if (d->mode == Client) {
        d->dbusInterface->call
            (QDBus::NoBlock, dbusMethodName(name), arg1, arg2, arg3);
    }
}

void QAbstractIpcInterface::invoke(const QByteArray& name, const QList<QVariant>& args)
{
    if (d->mode == Client) {
        d->dbusInterface->callWithArgumentList
            (QDBus::NoBlock, dbusMethodName(name), args);
    }
}

void QAbstractIpcInterface::setValue
        (const QString& name, const QVariant& value,
         QAbstractIpcInterface::SyncType sync)
{
    if (d->settings) {
        d->settings->setAttribute(name, value);
        if (sync == Immediate)
            QValueSpaceObject::sync();
    }
}

QVariant QAbstractIpcInterface::value
        (const QString& name, const QVariant& def) const
{
    QValueSpaceItem item( d->path );
    return item.value( name, def );
}

void QAbstractIpcInterface::removeValue
        (const QString& name, QAbstractIpcInterface::SyncType sync)
{
    if (d->settings) {
        d->settings->removeAttribute(name);
        if (sync == Immediate)
            QValueSpaceObject::sync();
    }
}

QList<QString> QAbstractIpcInterface::valueNames(const QString& path) const
{
    QList<QString> values;
    if (path.isEmpty())
        values = QValueSpaceItem(d->path).subPaths();
    else
        values = QValueSpaceItem(d->path + "/" + path).subPaths();
    for(QList<QString>::iterator iter = values.begin(); iter != values.end();) {
        if(iter->at(0) == QChar('.'))
            iter = values.erase(iter);
        else
            ++iter;
    }

    return values;
}

void QAbstractIpcInterface::groupInitialized(QAbstractIpcInterfaceGroup *)
{
    // Nothing to do here.
}

// Get the name of a Qt signal in D-BUS form.
static QString dbusSignalName(const char *signal)
{
    int posn = 0;
    while (signal[posn] != '\0' && signal[posn] != '(')
        ++posn;
    if (posn >= 1)
        return QString::fromUtf8(signal + 1, posn - 1);
    else
        return QString();
}

void QAbstractIpcInterface::connectNotify(const char *signal)
{
    // Protect against re-entry due to what QDBusConnection::connect() does.
    if (d->inConnectNotify) {
        QObject::connectNotify(signal);
        return;
    }
    d->inConnectNotify = true;

    // Deal with the connection details.
    if (d->mode == Client) {
        // If the message has not been connected to the signal yet,
        // then do it now.  We try to do this only on demand so that
        // client objects that don't need information from the server
        // via a signal won't incur the D-BUS signal setup overhead.
        QByteArray name = QByteArray(signal);
        if (!d->connectedSignals.contains(name)) {
            d->connectedSignals.append(name);
            if (name == SIGNAL(disconnected())) {
                d->dbusConnection.connect
                    (d->dbusService, d->dbusPath, d->dbusInterfaceName,
                     QLatin1String("ifaceServerDisconnect"),
                     this, SLOT(remoteDisconnected()));
            } else {
                d->dbusConnection.connect
                    (d->dbusService, d->dbusPath, d->dbusInterfaceName,
                     dbusSignalName(signal), this, signal);
            }
        }
    }

    // Safe to re-enter once more.
    d->inConnectNotify = false;

    // Pass control to the base implementation.
    QObject::connectNotify( signal );
}

void QAbstractIpcInterface::remoteDisconnected()
{
    if (d->mode != Invalid) {
        d->mode = Invalid;
        emit disconnected();
    }
}
