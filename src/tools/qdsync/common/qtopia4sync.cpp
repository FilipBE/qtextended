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
#include "qtopia4sync.h"
#include <trace.h>
QD_LOG_OPTION(Qtopia4Sync)
#include <qtopianamespace.h>
#include <QDateTime>
#include <QTimeZone>
#include <QSqlQuery>
#include <QtopiaSql>
#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>

#define SEND_CHANNEL "QD/Qtopia4Sync"
#define RECEIVE_CHANNEL "QPE/Qtopia4Sync"

/*!
  \class Qtopia4SyncPluginFactory
  \brief The Qtopia4SyncPluginFactory class is a handy way to embed multiple Qtopia4SyncPlugin instances in a single plugin.

  Using this class reduces the number of plugin files you need to support multiple datasets.

  \image dssyncplugin.png Synchronization plugin loading.

  \sa {Add a new device plugin}
*/

/*!
  Construct a Qtopia4SyncPluginFactory instance with the specified \a parent.
*/
Qtopia4SyncPluginFactory::Qtopia4SyncPluginFactory( QObject *parent )
    : QObject( parent )
{
}

/*!
  Destructor.
*/
Qtopia4SyncPluginFactory::~Qtopia4SyncPluginFactory()
{
}

/*!
  \fn Qtopia4SyncPluginFactory::keys()
  Returns a list of values suitable for passing to Qtopia4SyncPluginFactory::plugin().
*/

/*!
  \fn Qtopia4SyncPluginFactory::plugin( const QString &key )
  Returns a Qtopia4SyncPlugin instance for the indicated \a key.
  This function will be called once for every value returned by
  Qtopia4SyncPluginFactory::keys().
*/

// =====================================================================

/*!
  \class Qtopia4SyncPlugin
  \brief The Qtopia4SyncPlugin class handles synchronization of a single dataset.

  This class is called to fetch and apply changes. It is controlled by
  Qtopia4Sync, which remotes the API over QCop messages.

  Here is a message flow diagram showing how the class is used.

  \image qtopia4syncplugin.png Qtopia4SyncPlugin message flow.

  \sa Qtopia4SyncPluginFactory, {Add a new device plugin}
*/

/*!
  Construct a Qtopia4SyncPlugin with the specified \a parent.
*/
Qtopia4SyncPlugin::Qtopia4SyncPlugin( QObject *parent )
    : QObject( parent )
{
}

/*!
  Destructor.
*/
Qtopia4SyncPlugin::~Qtopia4SyncPlugin()
{
}

/*!
  \fn Qtopia4SyncPlugin::dataset()
  Returns the dataset that this plugin handles.
*/

/*!
  \fn void Qtopia4SyncPlugin::fetchChangesSince(const QDateTime &timestamp)
  Fetch changes since \a timestamp. Data and progress should be reported by emitting signals.
  This funtion can return before the fetching is completed.

  \bold Note that \a timestamp is in UTC time.
  \sa createClientRecord(), replaceClientRecord(), removeClientRecord(), clientChangesCompleted()
*/

/*!
  \fn void Qtopia4SyncPlugin::createServerRecord(const QByteArray &record)
  Create the item specified in \a record. Emit mappedId() so that the server record
  can be matched to the client record.
*/

/*!
  \fn void Qtopia4SyncPlugin::replaceServerRecord(const QByteArray &record)
  Update the item specified in \a record.
*/

/*!
  \fn void Qtopia4SyncPlugin::removeServerRecord(const QString &identifier)
  Remove the item indicated by \a identifier.
*/

/*!
  \fn void Qtopia4SyncPlugin::mappedId(const QString &serverId, const QString &clientId)
  Emit this signal in response to createServerRecord(). It maps server id \a serverId to
  client id \a clientId.
*/

/*!
  \fn void Qtopia4SyncPlugin::createClientRecord(const QByteArray &record)
  Emit this signal in response to fetchChangesSince() to send a new \a record.
  \sa replaceClientRecord(), removeClientRecord(), clientChangesCompleted()
*/

/*!
  \fn void Qtopia4SyncPlugin::replaceClientRecord(const QByteArray &record)
  Emit this signal in response to fetchChangesSince() to send a modified \a record.
  \sa createClientRecord(), removeClientRecord(), clientChangesCompleted()
*/

/*!
  \fn void Qtopia4SyncPlugin::removeClientRecord(const QString &identifier)
  Emit this signal in response to fetchChangesSince() to send a delete of \a identifier.
  \sa createClientRecord(), replaceClientRecord(), clientChangesCompleted()
*/

/*!
  \fn void Qtopia4SyncPlugin::clientChangesCompleted()
  Emit this signal in response to fetchChangesSince() when all changes have been reported.
*/

/*!
  \fn void Qtopia4SyncPlugin::clientError()
  Emit this signal any time if an error is encountered and the sync will be aborted.
*/

/*!
  \fn void Qtopia4SyncPlugin::beginTransaction( const QDateTime &timestamp )
  Begin a transaction. The \a timestamp may be ignored.
  \sa abortTransaction(), commitTransaction()
*/

/*!
  \fn void Qtopia4SyncPlugin::abortTransaction()
  Abort the transaction started with beginTransaction().
*/

/*!
  \fn void Qtopia4SyncPlugin::commitTransaction()
  Commit the transaction started with beginTransaction().
*/

// =====================================================================

enum SyncState {
    Idle,
    Header,
    SyncType,
    Diff
};

class Qtopia4SyncPrivate
{
public:
    QMap<QString,Qtopia4SyncPlugin*> plugins;

    // per-sync info
    Qtopia4SyncPlugin *currentPlugin;
    QString serverid;
    QDateTime lastSync;
    QDateTime nextSync;
    bool transaction;

    SyncState state;
};

// =====================================================================

Qtopia4Sync::Qtopia4Sync()
    : QObject()
{
    d = new Qtopia4SyncPrivate;
    initsyncVars();
    QCopChannel *chan = new QCopChannel( RECEIVE_CHANNEL, this );
    connect( chan, SIGNAL(received(QString,QByteArray)), this, SLOT(handleMessage(QString,QByteArray)) );
}

Qtopia4Sync::~Qtopia4Sync()
{
    delete d;
}

Qtopia4Sync *Qtopia4Sync::instance()
{
    static Qtopia4Sync *instance = 0;
    if ( !instance )
        instance = new Qtopia4Sync;
    return instance;
}

void Qtopia4Sync::registerPlugin( Qtopia4SyncPlugin *plugin )
{
    Q_ASSERT(plugin);
    d->plugins[plugin->dataset()] = plugin;
}

QStringList Qtopia4Sync::datasets()
{
    return d->plugins.keys();
}

void Qtopia4Sync::abort()
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::abort";
    if ( d->transaction )
        d->currentPlugin->abortTransaction();
    clientError();
    if ( d->currentPlugin )
        cleanupPlugin();
    initsyncVars();
}

void Qtopia4Sync::initsyncVars()
{
    d->currentPlugin = 0;
    d->serverid = QString();
    d->lastSync = QDateTime();
    d->nextSync = QDateTime();
    d->transaction = false;
    d->state = Idle;
}

#define CONNECT_SIGNALS(disconnect)\
    disconnect( d->currentPlugin, SIGNAL(createClientRecord(QByteArray)), this, SLOT(createClientRecord(QByteArray)));\
    disconnect( d->currentPlugin, SIGNAL(replaceClientRecord(QByteArray)), this, SLOT(replaceClientRecord(QByteArray)));\
    disconnect( d->currentPlugin, SIGNAL(removeClientRecord(QString)), this, SLOT(removeClientRecord(QString)));\
    disconnect( d->currentPlugin, SIGNAL(clientChangesCompleted()), this, SLOT(clientEnd()));\
    disconnect( d->currentPlugin, SIGNAL(mappedId(QString,QString)), this, SLOT(mappedId(QString,QString)));\
    disconnect( d->currentPlugin, SIGNAL(clientError()), this, SLOT(abort()))

void Qtopia4Sync::cleanupPlugin()
{
    CONNECT_SIGNALS(disconnect);
}

// =====================================================================

// Take incoming messages and dispatch them to our helper functions
void Qtopia4Sync::handleMessage( const QString &message, const QByteArray &data )
{
    QDataStream stream(data);
    QString s1, s2;
    int i1, i2, i3;
    QDateTime ts1, ts2;
    QByteArray r;
    if (message == "serverSyncRequest(QString)") {
        stream >> s1;
        serverSyncRequest(s1);
    } else if (message == "serverIdentity(QString)") {
        stream >> s1;
        serverIdentity(s1);
    } else if (message == "serverVersion(int,int,int)") {
        stream >> i1 >> i2 >> i3;
        serverVersion(i1, i2, i3);
    } else if (message == "serverSyncAnchors(QDateTime,QDateTime)") {
        stream >> ts1 >> ts2;
        serverSyncAnchors(ts1, ts2);
    } else if (message == "createServerRecord(QByteArray)") {
        stream >> r;
        createServerRecord(r);
    } else if (message == "replaceServerRecord(QByteArray)") {
        stream >> r;
        replaceServerRecord(r);
    } else if (message == "removeServerRecord(QString)") {
        stream >> s1;
        removeServerRecord(s1);
    } else if (message == "requestTwoWaySync()") {
        requestTwoWaySync();
    } else if (message == "requestSlowSync()") {
        requestSlowSync();
    } else if (message == "serverError()") {
        serverError();
    } else if (message == "serverEnd()") {
        serverEnd();
    }
}

void Qtopia4Sync::serverSyncRequest(const QString &source)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::serverSyncRequest" << "source" << source;
    Q_ASSERT(d->state == Idle);
    d->state = Header;

    Q_ASSERT(d->currentPlugin == 0);
    if ( d->plugins.contains( source ) ) {
        // Initialize the per-sync info
        d->currentPlugin = d->plugins[source];
        CONNECT_SIGNALS(connect);
    } else {
        abort();
    }
}

void Qtopia4Sync::serverIdentity(const QString &server)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::serverIdentity" << "server" << server;
    Q_ASSERT(d->state == Header);
    Q_ASSERT(d->serverid.isEmpty());
    d->serverid = server;
}

void Qtopia4Sync::serverVersion(int major, int minor, int patch)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::serverVersion" << "version" << major << minor << patch;
    Q_ASSERT(d->state == Header);
    // For now the version must be set to 4.3.0
    if ( major != 4 || minor != 3 || patch != 0 ) {
        abort();
        return;
    }

    clientIdentity(Qtopia::deviceId());
    clientVersion(4, 3, 0); // For now the version must be set to 4.3.0
}

void Qtopia4Sync::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::serverSyncAnchors" << "serverLastSync" << serverLastSync << "serverNextSync" << serverNextSync;
    Q_ASSERT(d->state == Header);
    // If the last sync tags don't match up will need a full sync.
    // let the server catch this as it is expected to chose the sync
    // type already.
    Q_UNUSED(serverLastSync)
    Q_UNUSED(serverNextSync)

    QSqlQuery q(QtopiaSql::instance()->systemDatabase());
    q.prepare("SELECT lastSyncAnchor FROM syncServers WHERE serverIdentity = :s AND datasource = :d");
    q.bindValue(":s", d->serverid);
    q.bindValue(":d", d->currentPlugin->dataset());
    q.exec();
    if (q.next())
        d->lastSync = q.value(0).toDateTime();
     else
        d->lastSync = QDateTime();

    d->nextSync = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    // send client header
    clientSyncAnchors(d->lastSync, d->nextSync);
    d->state = SyncType;
}

void Qtopia4Sync::checkTransaction()
{
    Q_ASSERT(d->state == Diff);
    if ( !d->transaction ) {
        d->transaction = true;
        d->currentPlugin->beginTransaction( d->nextSync );
    }
}

void Qtopia4Sync::createServerRecord(const QByteArray &record)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::createServerRecord"; //<< "record" << record;
    checkTransaction();
    d->currentPlugin->createServerRecord(record);
}

void Qtopia4Sync::replaceServerRecord(const QByteArray &record)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::replaceServerRecord"; //<< "record" << record;
    checkTransaction();
    d->currentPlugin->replaceServerRecord(record);
}

void Qtopia4Sync::removeServerRecord(const QString &localId)
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::removeServerRecord" << "localId" << localId;
    checkTransaction();
    d->currentPlugin->removeServerRecord(localId);
}

void Qtopia4Sync::requestTwoWaySync()
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::requestTwoWaySync";
    Q_ASSERT(d->state == SyncType);
    d->state = Diff;

    d->currentPlugin->fetchChangesSince(d->lastSync.addSecs(1)); // don't re-sync items matching last time-stamp
}

void Qtopia4Sync::requestSlowSync()
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::requestSlowSync";
    Q_ASSERT(d->state == SyncType);
    d->state = Diff;

    d->currentPlugin->fetchChangesSince(QDateTime());
}

void Qtopia4Sync::serverError()
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::serverError";
    abort();
}

void Qtopia4Sync::serverEnd()
{
    TRACE(Qtopia4Sync) << "Qtopia4Sync::serverEnd";
    Q_ASSERT(d->state == Diff);
    QSqlQuery q(QtopiaSql::instance()->systemDatabase());
    if (d->lastSync.isNull())
        q.prepare("INSERT INTO syncServers (serverIdentity, datasource, lastSyncAnchor) VALUES (:id, :ds, :ls)");
    else
        q.prepare("UPDATE syncServers SET lastSyncAnchor = :ls WHERE serverIdentity = :id AND datasource = :ds");
    q.bindValue(":id", d->serverid);
    q.bindValue(":ds", d->currentPlugin->dataset());
    q.bindValue(":ls", d->nextSync);

    if (!q.exec()) {
        abort();
        return;
    }

    if ( d->transaction ) {
        d->transaction = false;
        d->currentPlugin->commitTransaction();
    }

    cleanupPlugin();
    initsyncVars();
    clientEnd();
}

void Qtopia4Sync::clientIdentity(const QString &id)
{
    QCopEnvelope e( SEND_CHANNEL, "clientIdentity(QString)" );
    e << id;
}

void Qtopia4Sync::clientVersion(int major, int minor, int patch)
{
    QCopEnvelope e( SEND_CHANNEL, "clientVersion(int,int,int)" );
    e << major << minor << patch;
}

void Qtopia4Sync::clientSyncAnchors(const QDateTime &clientLastSync, const QDateTime &clientNextSync)
{
    QCopEnvelope e( SEND_CHANNEL, "clientSyncAnchors(QDateTime,QDateTime)" );
    e << clientLastSync << clientNextSync;
}

void Qtopia4Sync::createClientRecord(const QByteArray &record)
{
    QCopEnvelope e( SEND_CHANNEL, "createClientRecord(QByteArray)" );
    e << record;
}

void Qtopia4Sync::replaceClientRecord(const QByteArray &record)
{
    QCopEnvelope e( SEND_CHANNEL, "replaceClientRecord(QByteArray)" );
    e << record;
}

void Qtopia4Sync::removeClientRecord(const QString &clientId)
{
    QCopEnvelope e( SEND_CHANNEL, "removeClientRecord(QString)" );
    e << clientId;
}

void Qtopia4Sync::mappedId(const QString &serverId, const QString &clientId)
{
    QCopEnvelope e( SEND_CHANNEL, "mappedId(QString,QString)" );
    e << serverId << clientId;
}

void Qtopia4Sync::clientError()
{
    QCopEnvelope e( SEND_CHANNEL, "clientError()" );
}

void Qtopia4Sync::clientEnd()
{
    QCopEnvelope e( SEND_CHANNEL, "clientEnd()" );
}

