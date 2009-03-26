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
#include "merge.h"
#include "mergeitem.h"

#include <desktopsettings.h>
#include <trace.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringList>
#include <QVariant>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

#define SANITIZE(array) QString(array).replace("\n","").replace(QRegExp("> +<"), "><").replace(QRegExp("<\\?xml .*\\?>"),"").trimmed()

Conflict::Conflict(const Change &c, const Change &s)
    : client(c), server(s)
{
    if (client.type == Change::Replace) {
        if (server.type == Change::Replace)
            type = ReplaceReplace;
        else if (server.type == Change::Remove)
            type = ReplaceRemove;
        else
            type = NoConflict;
    } else if (client.type == Change::Remove) {
        if (server.type == Change::Replace)
            type = RemoveReplace;
        else if (server.type == Change::Remove)
            type = RemoveRemove;
        else
            type = NoConflict;
    } else if (client.type == Change::Create && server.type == Change::Create) {
        type = CreateCreate;
    } else {
        type = NoConflict;
    }
}

Conflict::~Conflict()
{
}

class QSyncMergeData
{
public:
    QSyncMergeData() : reference(0) {}

    QSqlDatabase database;
    QSqlQuery query() const { return QSqlQuery(database); }
    QByteArray serverReference;
    QByteArray clientReference;
    MergeItem *reference;
    QString datasource;
};

QSyncMerge::QSyncMerge(QObject *parent)
    : QObject(parent)
{
    TRACE(QDSync) << "QSyncMerge::QSyncMerge";

    d = new QSyncMergeData;
    d->database = QSqlDatabase::addDatabase("QSQLITE", "merge");
    d->database.setDatabaseName(DesktopSettings::homePath() + "/qtopia.sqlite");
    if (!d->database.open())
        WARNING() << "Could not open database" << (DesktopSettings::homePath() + "/qtopia.sqlite");

    QSqlQuery q = d->query();

    // types, 0 Create, 1, Replace, 2 Remove.  To remember, think of record life-cycle.
    q.prepare("CREATE TEMPORARY TABLE serverchanges (clientid TEXT, changetype INT, mappedchangedata BLOB, changedata BLOB)");
    q.exec();
    q.prepare("CREATE TEMPORARY TABLE clientchanges (clientid TEXT, changetype INT, mappedchangedata BLOB, changedata BLOB)");
    q.exec();

    if (!d->database.tables().contains("identifiermap")) {
        q.prepare("CREATE TABLE identifiermap (serverid TEXT, clientid TEXT, UNIQUE(serverid), UNIQUE(clientid))");
        q.exec();
    }
    if (!d->database.tables().contains("identifiermap2")) {
        q.prepare("CREATE TABLE identifiermap2 (serverid TEXT, clientid TEXT, datasource TEXT, UNIQUE(serverid), UNIQUE(clientid))");
        q.exec();
    }
    if (!d->database.tables().contains("syncClients")) {
        q.prepare("CREATE TABLE syncClients (clientIdentity VARCHAR(255), datasource VARCHAR(255), lastSyncAnchor TIMESTAMP, lastSyncServer TIMESTAMP, UNIQUE(clientIdentity, datasource))");
        q.exec();
    } else if (!d->database.record("syncClients").contains("lastSyncServer")) {
        // lastSyncAnchor would be lastSyncClient, except can't rename or remove columns, only add.
        q.prepare("ALTER TABLE syncClients ADD COLUMN lastSyncServer TIMESTAMP");
        q.exec();
    }
}

QSyncMerge::~QSyncMerge()
{
    TRACE(QDSync) << "QSyncMerge::~QSyncMerge";
    d->database.close();
    delete d;
    QSqlDatabase::removeDatabase("merge");
}

void QSyncMerge::setServerReferenceSchema(const QByteArray &data)
{
    d->serverReference = data;
    if (d->reference) {
        delete d->reference;
        d->reference = 0;
    }
}

void QSyncMerge::setClientReferenceSchema(const QByteArray &data)
{
    d->clientReference = data;
    if (d->reference) {
        delete d->reference;
        d->reference = 0;
    }
}

MergeItem *QSyncMerge::referenceItem()
{
    bool ok = true;
    if (!d->reference) {
        if (!d->serverReference.isEmpty()) {
            d->reference = new MergeItem(this) ;
            ok = d->reference->read(d->serverReference, MergeItem::DataOnly);
            if ( !ok ) {
                qWarning() << "BUG! d->serverReference could not be parsed" << __FILE__ << __LINE__;
                d->serverReference = "";
            }
        }
        if (!d->clientReference.isEmpty()) {
            if (d->reference) {
                MergeItem item(this);
                ok = item.read(d->clientReference, MergeItem::DataOnly);
                if ( ok )
                    d->reference->restrictTo(item);
            } else {
                d->reference = new MergeItem(this) ;
                ok = d->reference->read(d->clientReference, MergeItem::DataOnly);
            }
            if ( !ok ) {
                qWarning() << "BUG! clientReference could not be parsed" << __FILE__ << __LINE__;
                d->clientReference = "";
            }
        }
    }
    if ( d->reference && !ok ) {
        delete d->reference;
        d->reference = 0;
    }
    return d->reference;
}

void QSyncMerge::mapIdentifier(const QString &serverId, const QString &clientId)
{
    TRACE(QDSync) << "QSyncMerge::mapIdentifier" << "serverId" << serverId << "clientId" << clientId;
    SyncLog() << "Mapping server identifier" << serverId << "to client identifier" << clientId << endl;
    QSqlQuery q = d->query();
    if (!q.prepare("INSERT INTO identifiermap2 (serverid, clientid, datasource) VALUES (:s, :c, :d)"))
        WARNING() << "failed to prepare" << q.lastError().text() << __LINE__;
    q.bindValue(":s", serverId);
    q.bindValue(":c", clientId);
    q.bindValue(":d", d->datasource);
    if (!q.exec())
        WARNING() << "failed to exec" << q.lastError().text() << __LINE__;
}

void QSyncMerge::clearIdentifierMap()
{
    TRACE(QDSync) << "QSyncMerge::clearIdentifierMap";
    {
        QSqlQuery q = d->query();
        if (!q.prepare("DELETE FROM identifiermap"))
            WARNING() << "failed to prepare" << q.lastError().text() << __LINE__;
        if (!q.exec())
            WARNING() << "failed to exec" << q.lastError().text() << __LINE__;
    }
    QSqlQuery q = d->query();
    if (!q.prepare("DELETE FROM identifiermap2 where datasource = :d"))
        WARNING() << "failed to prepare" << q.lastError().text() << __LINE__;
    q.bindValue(":d", d->datasource);
    if (!q.exec())
        WARNING() << "failed to exec" << q.lastError().text() << __LINE__;
}

QString QSyncMerge::map(const QString &ident, ChangeSource source)
{
    TRACE(QDSync) << "QSyncMerge::map" << "ident" << ident << "source" << (source==Server?"Server":"Client");
    {
        QSqlQuery q = d->query();
        if (source == Server) {
            if (!q.prepare("SELECT clientid FROM identifiermap2 WHERE serverid = :s"))
                WARNING() << "error in prepare" << __LINE__ << q.lastError().text();
            q.bindValue(":s", ident);
        } else {
            if (!q.prepare("SELECT serverid FROM identifiermap2 WHERE clientid = :c"))
                WARNING() << "error in prepare" << __LINE__ << q.lastError().text();
            q.bindValue(":c", ident);
        }
        if (q.exec() && q.next())
            return q.value(0).toString();
    }
    // Transparently migrate data from the old identifiermap table to the new identifiermap2 table.
    QSqlQuery q = d->query();
    if (source == Server) {
        if (!q.prepare("SELECT clientid FROM identifiermap WHERE serverid = :s"))
            WARNING() << "error in prepare" << __LINE__ << q.lastError().text();
        q.bindValue(":s", ident);
    } else {
        if (!q.prepare("SELECT serverid FROM identifiermap WHERE clientid = :c"))
            WARNING() << "error in prepare" << __LINE__ << q.lastError().text();
        q.bindValue(":c", ident);
    }
    if (q.exec() && q.next()) {
        QString ret = q.value(0).toString();
        QString serverid;
        QString clientid;
        if (source == Server) {
            serverid = ident;
            clientid = ret;
        } else {
            serverid = ret;
            clientid = ident;
        }
        // register the mapping in the new table
        mapIdentifier(serverid, clientid);
        // remove the mapping from the old table
        QSqlQuery del = d->query();
        if (!del.prepare("DELETE FROM identifiermap WHERE serverid = :s"))
            WARNING() << "error in prepare" << __LINE__ << q.lastError().text();
        del.bindValue(":s", serverid);
        del.exec();
        return ret;
    }
    return QString();
}

void QSyncMerge::lastSync(const QString &clientid, const QString &datasource, QDateTime &clientLastSync, QDateTime &serverLastSync)
{
    TRACE(QDSync) << "QSyncMerge::lastSync" << "clientid" << clientid << "datasource" << datasource;
    QSqlQuery q = d->query();
    if (!q.prepare("SELECT lastSyncAnchor, lastSyncServer FROM syncClients WHERE clientIdentity = :c AND datasource = :d"))
        WARNING() << "error in prepare" << __LINE__ << q.lastError().text();

    q.bindValue(":c", clientid);
    q.bindValue(":d", datasource);
    q.exec();

    if (q.next()) {
        clientLastSync = q.value(0).toDateTime();
        clientLastSync.setTimeSpec(Qt::UTC); // This is not preserved in the database
        serverLastSync = q.value(1).toDateTime();
        serverLastSync.setTimeSpec(Qt::UTC); // This is not preserved in the database
    }
}

void QSyncMerge::recordLastSync(const QString &clientid, const QString &datasource, const QDateTime &clientLastSync, const QDateTime &serverLastSync)
{
    TRACE(QDSync) << "QSyncMerge::recordLastSync" << "clientid" << clientid << "datasource" << datasource << "clientLastSync" << clientLastSync << "serverLastSync" << serverLastSync;
    QSqlQuery q = d->query();
    q.prepare("SELECT lastSyncAnchor FROM syncClients WHERE clientIdentity = :c AND datasource = :d");

    q.bindValue(":c", clientid);
    q.bindValue(":d", datasource);
    q.exec();

    if (q.next()) {
        q.prepare("UPDATE syncClients SET lastSyncAnchor = :cls, lastSyncServer = :sls WHERE clientIdentity = :id AND datasource = :ds");
    } else {
        q.prepare("INSERT INTO syncClients (clientIdentity, datasource, lastSyncAnchor, lastSyncServer) VALUES (:id, :ds, :cls, :sls)");
    }
    q.bindValue(":id", clientid);
    q.bindValue(":ds", datasource);
    q.bindValue(":cls", clientLastSync);
    q.bindValue(":sls", serverLastSync);

    q.exec();
}

void QSyncMerge::clearChanges()
{
    TRACE(QDSync) << "QSyncMerge::clearChanges";
    QSqlQuery q = d->query();
    q.prepare("DELETE FROM serverchanges");
    q.exec();
    q.prepare("DELETE FROM clientchanges");
    q.exec();
}

void QSyncMerge::createServerRecord(const QByteArray &array)
{
    TRACE(QDSync) << "QSyncMerge::createServerRecord";
    LOG() << "array" << array;
    SyncLog() << "Create server record" << SANITIZE(array) << endl;
    MergeItem item(this);
    bool ok = item.read(array, MergeItem::Server);
    if (!ok) {
        LOG() << "Could not parse record!";
        SyncLog() << "Could not parse record!";
        return;
    }
    if (referenceItem())
        item.restrictTo(*referenceItem());
    addServerChange(Change::Create, item);
}

void QSyncMerge::replaceServerRecord(const QByteArray &array)
{
    TRACE(QDSync) << "QSyncMerge::replaceServerRecord";
    LOG() << "array" << array;
    SyncLog() << "Replace server record" << SANITIZE(array) << endl;
    MergeItem item(this);
    bool ok = item.read(array, MergeItem::Server);
    if (!ok) {
        LOG() << "Could not parse record!";
        SyncLog() << "Could not parse record!";
        return;
    }
    if (referenceItem())
        item.restrictTo(*referenceItem());
    addServerChange(Change::Replace, item);
}

void QSyncMerge::removeServerRecord(const QString &id)
{
    TRACE(QDSync) << "QSyncMerge::removeServerRecord" << "id" << id;
    if (!canMap(id, Server)) {
        SyncLog() << "Ignoring remove server record (not on device?)" << id << endl;
        return;
    }
    SyncLog() << "Remove server record" << id << endl;

    QSqlQuery q = d->query();
    if (!q.prepare("INSERT INTO serverchanges (clientid, changetype) VALUES (:r, :t)"))
        WARNING() << "prepare failed" << q.lastError().text() << __LINE__;

    q.bindValue(":r", map(id, Server));
    q.bindValue(":t", Change::Remove);

    if (!q.exec())
        WARNING() << "exec failed" << q.lastError().text() << __LINE__;
}

void QSyncMerge::createClientRecord(const QByteArray &array)
{
    TRACE(QDSync) << "QSyncMerge::createClientRecord";
    LOG() << "array" << array;
    SyncLog() << "Create client record" << SANITIZE(array) << endl;
    MergeItem item(this);
    bool ok = item.read(array, MergeItem::Client);
    if (!ok) {
        LOG() << "Could not parse record!";
        SyncLog() << "Could not parse record!";
        return;
    }
    if (referenceItem())
        item.restrictTo(*referenceItem());
    addClientChange(Change::Create, item);
}

void QSyncMerge::replaceClientRecord(const QByteArray &array)
{
    TRACE(QDSync) << "QSyncMerge::replaceClientRecord";
    LOG() << "array" << array;
    SyncLog() << "Replace client record" << SANITIZE(array) << endl;
    MergeItem item(this);
    bool ok = item.read(array, MergeItem::Client);
    if (!ok) {
        LOG() << "Could not parse record!";
        SyncLog() << "Could not parse record!";
        return;
    }
    if (referenceItem())
        item.restrictTo(*referenceItem());
    addClientChange(Change::Replace, item);
}

void QSyncMerge::removeClientRecord(const QString &id)
{
    TRACE(QDSync) << "QSyncMerge::removeClientRecord" << "id" << id;
    SyncLog() << "Remove client record" << id << endl;
    if (!canMap(id, Client))
        return;

    QSqlQuery q = d->query();
    if (!q.prepare("INSERT INTO clientchanges (clientid, changetype) VALUES (:r, :t)"))
        WARNING() << "prepare failed" << q.lastError().text() << __LINE__;

    q.bindValue(":r", id);
    q.bindValue(":t", Change::Remove);

    if (!q.exec())
        WARNING() << "exec failed" << q.lastError().text() << __LINE__;
}

void QSyncMerge::addServerChange(Change::Type type, const MergeItem &item)
{
    TRACE(QDSync) << "QSyncMerge::addServerChange";
    QSqlQuery q = d->query();
    if (!q.prepare("INSERT INTO serverchanges (clientid, changetype, mappedchangedata, changedata) VALUES (:r, :t, :i, :d)"))
        WARNING() << "prepare failed" << q.lastError().text() << __LINE__;


    q.bindValue(":r", type == Change::Create ? item.serverIdentifier() : item.clientIdentifier());
    q.bindValue(":t", int(type));

    // in the middle of changing this
    q.bindValue(":i", item.write(MergeItem::IdentifierOnly));
    q.bindValue(":d", item.write(MergeItem::DataOnly));

    //qDebug() << "Add server change";
    //qDebug() << QString::fromUtf8(item.write(MergeItem::DataOnly));
    if (!q.exec())
        WARNING() << "exec failed" << q.lastError().text() << __LINE__;
}

void QSyncMerge::addClientChange(Change::Type type, const MergeItem &item)
{
    TRACE(QDSync) << "QSyncMerge::addClientChange";
    QSqlQuery q = d->query();
    if (!q.prepare("INSERT INTO clientchanges (clientid, changetype, mappedchangedata, changedata) VALUES (:r, :t, :i, :d)"))
        WARNING() << "prepare failed" << q.lastError().text() << __LINE__;

    q.bindValue(":r", item.clientIdentifier());
    q.bindValue(":t", int(type));

    // in the middle of changing this
    q.bindValue(":i", item.write(MergeItem::IdentifierOnly));
    q.bindValue(":d", item.write(MergeItem::DataOnly));

    //qDebug() << "Add client change";
    //qDebug() << QString::fromUtf8(item.write(MergeItem::DataOnly));
    if (!q.exec())
        WARNING() << "exec failed" << q.lastError().text() << __LINE__;
}

void QSyncMerge::readConflict(Change &client, Change &server, const QSqlQuery &q)
{
    TRACE(QDSync) << "QSyncMerge::readConflict";
    MergeItem clientItem(this), serverItem(this);
    clientItem.read(q.value(3).toByteArray(), MergeItem::DataOnly);
    clientItem.read(q.value(2).toByteArray(), MergeItem::IdentifierOnly);

    serverItem.read(q.value(7).toByteArray(), MergeItem::DataOnly);
    serverItem.read(q.value(6).toByteArray(), MergeItem::IdentifierOnly);

    client.id = q.value(0).toString();
    client.type = Change::Type(q.value(1).toInt());
    client.record = clientItem.write(MergeItem::Client);

    server.id = q.value(4).toString();
    server.type = Change::Type(q.value(5).toInt());
    server.record = serverItem.write(MergeItem::Server);
}

QList<Conflict> QSyncMerge::conflicts()
{
    TRACE(QDSync) << "QSyncMerge::conflicts";
    QList<Conflict> list;

    QSqlQuery q = d->query();
    // add/add conflicts. These are important for slow-sync recovery
    q.prepare("SELECT client.clientid, client.changetype, client.mappedchangedata, client.changedata, server.clientid, server.changetype, server.mappedchangedata, server.changedata FROM clientchanges AS client JOIN serverchanges AS server ON client.changedata = server.changedata WHERE server.changetype = 0 AND client.changetype = 0 ORDER BY client.clientid, server.clientid");
    q.exec();
    while(q.next()) {
        Change client;
        Change server;

        readConflict(client, server, q);

        list.append(Conflict(client, server));
    }
    // other types of conflicts (updateremove, removeremove, removeupdate);
    q.prepare("SELECT client.clientid, client.changetype, client.mappedchangedata, client.changedata, server.clientid, server.changetype, server.mappedchangedata, server.changedata FROM clientchanges AS client JOIN serverchanges AS server ON client.clientid = server.clientid WHERE server.changetype != 0 AND client.changetype != 0 ORDER BY client.changetype, server.changetype, client.clientid, server.clientid");
    q.exec();
    while(q.next()) {
        Change client;
        Change server;

        readConflict(client, server, q);

        list.append(Conflict(client, server));
    }
    return list;
}

/*!
  Resolve the conflict such that the client is modified to match the server change.
*/
bool QSyncMerge::resolveClient(const Conflict &conflict)
{
    TRACE(QDSync) << "QSyncMerge::resolveClient";
    return resolveBiased(conflict, true);
}

/*!
  Resolve the conflict such that the server is modified to match the client change.
*/
bool QSyncMerge::resolveServer(const Conflict &conflict)
{
    TRACE(QDSync) << "QSyncMerge::resolveServer";
    return resolveBiased(conflict, false);
}

bool QSyncMerge::resolveBiased(const Conflict &conflict, bool biasServer)
{
    TRACE(QDSync) << "QSyncMerge::resolveBiased" << "type" << conflict.type << "clientId" << conflict.client.id << "serverId" << conflict.server.id;
    QString favoured, disfavoured;
    QString favouredId, disfavouredId;
    if (biasServer) {
        favoured = "serverchanges";
        disfavoured = "clientchanges";
        favouredId = conflict.server.id;
        disfavouredId = conflict.client.id;
    } else {
        favoured = "clientchanges";
        disfavoured = "serverchanges";
        favouredId = conflict.client.id;
        disfavouredId = conflict.server.id;
    }

    QSqlQuery q = d->query();
    switch(conflict.type) {
        case Conflict::CreateCreate:
            //qDebug() << "resolve Create/Create conflict";
            // one becomes an edit, the other is removed.
            if (!q.prepare("DELETE FROM " + disfavoured + " WHERE clientid = :cid AND changetype = 0"))
                WARNING() << "failed prepare" << q.lastError().text() << __LINE__;
            q.bindValue(":cid", disfavouredId);
            if (!q.exec())
                WARNING() << "failed exec" << q.lastError().text() << __LINE__;

            if (!q.prepare("UPDATE " + favoured + " SET changetype = 1 WHERE changetype = 0 AND clientid = :cid"))
                WARNING() << "failed prepare" << q.lastError().text() << __LINE__;
            q.bindValue(":cid", favouredId);
            if (!q.exec())
                WARNING() << "failed exec" << q.lastError().text() << __LINE__;

            mapIdentifier(conflict.server.id, conflict.client.id);
            return true;
        case Conflict::RemoveRemove:
            // same change, easy resolve.
            q.prepare("DELETE FROM " + disfavoured + " WHERE clientid = :cid AND changetype = 2");
            q.bindValue(":cid", disfavouredId);
            q.exec();
            q.prepare("DELETE FROM " + favoured + " WHERE clientid = :sid AND changetype = 2");
            q.bindValue(":sid", favouredId);
            q.exec();
            return true;
        case Conflict::ReplaceReplace:
            // server change remains, client change removed
            q.prepare("DELETE FROM " + disfavoured + " WHERE clientid = :cid AND changetype = 1");
            q.bindValue(":cid", disfavouredId);
            q.exec();
            return true;
        case Conflict::ReplaceRemove: // same as RemoveReplace
        case Conflict::RemoveReplace:
            // delete wins
            q.prepare("DELETE FROM " + disfavoured + " WHERE clientid = :cid AND changetype = 1");
            q.bindValue(":cid", disfavouredId);
            q.exec();
            return true;
        case Conflict::NoConflict:
            return true;
    }
    return false;
}

bool QSyncMerge::resolveDuplicate(const Conflict &conflict)
{
    TRACE(QDSync) << "QSyncMerge::resolveDuplicate" << "type" << conflict.type << "clientId" << conflict.client.id << "serverId" << conflict.server.id;
    QSqlQuery q = d->query();
    switch (conflict.type) {
        case Conflict::NoConflict:
            break;
        case Conflict::CreateCreate:
            revertIdentifierMapping(conflict);
            return true;
        case Conflict::ReplaceReplace:
            // use a temp id and make them both creates
            // return false until the usage of temp id's is worked out.
            revertIdentifierMapping(conflict);

            q.prepare("UPDATE serverchanges SET changetype = 0 WHERE clientid = :cid");
            q.bindValue(":cid", conflict.server.id);
            q.exec();

            q.prepare("UPDATE clientchanges SET changetype = 0 WHERE clientid = :cid");
            q.bindValue(":cid", conflict.client.id);
            q.exec();

            return true;
        case Conflict::RemoveRemove:
            // same change, easy resolve.
            q.prepare("DELETE FROM serverchanges WHERE clientid = :cid AND changetype = 2");
            q.bindValue(":cid", conflict.server.id);
            q.exec();
            q.prepare("DELETE FROM clientchanges WHERE clientid = :sid AND changetype = 2");
            q.bindValue(":sid", conflict.client.id);
            q.exec();
            return true;
        case Conflict::ReplaceRemove:
            // turn the replace to a create.  Drop the existing id map.
            revertIdentifierMapping(conflict);

            q.prepare("UPDATE clientchanges SET changetype = 0 WHERE clientid = :cid");
            q.bindValue(":cid", conflict.client.id);
            q.exec();

            q.prepare("DELETE FROM serverchanges WHERE clientid= :cid");
            q.bindValue(":cid", conflict.server.id);
            q.exec();
            return true;
        case Conflict::RemoveReplace:
            // turn the replace to a create.  Drop the existing id map.
            revertIdentifierMapping(conflict);

            q.prepare("UPDATE serverchanges SET changetype = 0 WHERE clientid = :cid");
            q.bindValue(":cid", conflict.server.id);
            q.exec();
            q.prepare("DELETE FROM clientchanges WHERE clientid= :cid");
            q.bindValue(":cid", conflict.client.id);
            q.exec();
            return true;
    }
    return false;
}

#if 0
    // not currently advised, id mapping still an issue.
    // better to have something that is more specific about which
    // way the idents are mapped for the byte array.
bool QSyncMerge::resolveMerged(const Conflict &conflict, const QByteArray &replacementrecord)
{
    /* TODO when moving data between tables, need to be careful of
       the mapped id's in the xml structures.
    */
    QSqlQuery q = d->query();
    switch(conflict.type) {
        case Conflict::ReplaceReplace:
            q.prepare("UPDATE serverchanges SET changedata = :d WHERE clientid = :sid AND changetype = 1");
            q.bindValue(":d", replacementrecord);
            q.bindValue(":sid", conflict.server.id);
            q.exec();
            q.prepare("UPDATE clientchanges SET changedata = :d WHERE clientid = :cid AND changetype = 1");
            q.bindValue(":d", replacementrecord);
            q.bindValue(":cid", conflict.client.id);
            q.exec();
            return true;
        case Conflict::RemoveRemove:
        case Conflict::RemoveReplace:
        case Conflict::ReplaceRemove:
            return false;
        case Conflict::NoConflict:
            return true;
    }
    return false;
}
#endif

bool QSyncMerge::resolveAllClient()
{
    TRACE(QDSync) << "QSyncMerge::resolveAllClient";
    bool result = true;
    QList<Conflict> list = conflicts();
    //qDebug() << "attempt to resolve" << list.count() << "conflicts, biasing to the client";
    foreach(const Conflict &c, list) {
        if (!resolveClient(c))
            result = false; // continue processing.
    }
    return result;
}

bool QSyncMerge::resolveAllServer()
{
    TRACE(QDSync) << "QSyncMerge::resolveAllServer";
    bool result = true;
    QList<Conflict> list = conflicts();
    //qDebug() << "attempt to resolve" << list.count() << "conflicts, biasing to the server";
    foreach(const Conflict &c, list) {
        if (!resolveServer(c))
            result = false; // continue processing.
    }
    return result;
}

/*!
  Resolved changes for contacts modified on the server and to be applied to the client.
  Identifiers are client identifiers where mapping is available.
*/
QList<Change> QSyncMerge::serverDiff()
{
    TRACE(QDSync) << "QSyncMerge::serverDiff";
    QList<Change> list;
    QSqlQuery q = d->query();
    q.prepare("SELECT clientid, changetype, mappedchangedata, changedata FROM serverchanges");
    q.exec();
    while(q.next()) {
        Change client;
        MergeItem clientItem(this);

        client.id = q.value(0).toString();
        client.type = Change::Type(q.value(1).toInt());

        if ( client.type != Change::Remove ) {
            clientItem.read(q.value(3).toByteArray(), MergeItem::DataOnly);
            clientItem.read(q.value(2).toByteArray(), MergeItem::IdentifierOnly);
            client.record = clientItem.write(MergeItem::Client);
        }
        list.append(client);
    }
    return list;
}

/*!
  Resolved changes for contacts modified on the client and to be applied to the server.
  Identifiers are server identifiers where mapping is available.
*/
QList<Change> QSyncMerge::clientDiff()
{
    TRACE(QDSync) << "QSyncMerge::clientDiff";
    QList<Change> list;
    QSqlQuery q = d->query();
    q.prepare("SELECT clientid, changetype, mappedchangedata, changedata FROM clientchanges");
    q.exec();
    while(q.next()) {
        Change server;
        MergeItem serverItem(this);

        server.id = map(q.value(0).toString(), Client);
        server.type = Change::Type(q.value(1).toInt());

        serverItem.read(q.value(3).toByteArray(), MergeItem::DataOnly);
        serverItem.read(q.value(2).toByteArray(), MergeItem::IdentifierOnly);

        server.record = serverItem.write(MergeItem::Server);
        list.append(server);
    }
    return list;
}

/*!
  Parse the given \a array as an xml description of a record.   The first Identifier tag will be used as the record identifier.  For instance:

  \code
  <Task>
    <Identifier>{0-0}</Identifier>
    ...
  </Task>
  \endcode

  All Identifier tags found will be mapped to the opposing source type.  Hence server identifiers will be mapped to client identifiers, and vice-versa.  In the cases where no mapping is available, the original identifier is sent and the localIdentifier attribute is set to false.

  The specified \a source is used to identify which way to map the identifiers.
*/
QString QSyncMerge::parseIdentifiers(QByteArray &array, ChangeSource source, bool revert)
{
    TRACE(QDSync) << "QSyncMerge::parseIdentifiers";
    QByteArray result;
    QString clientId;
    QXmlStreamReader reader(array);
    QXmlStreamWriter writer(&result);

    bool isIdent = false;
    bool firstIdent = true;
    QXmlStreamAttributes identAttr;
    while (!reader.atEnd()) {
        switch(reader.readNext()) {
            case QXmlStreamReader::NoToken:
            case QXmlStreamReader::Invalid:
                // both of these are error cases.
            case QXmlStreamReader::EntityReference:
            case QXmlStreamReader::ProcessingInstruction:
                // these are not error cases, but are unexpected in
                // sync format XML.
                return QString();
                break;
            case QXmlStreamReader::StartDocument:
                writer.writeStartDocument();
                break;
            case QXmlStreamReader::EndDocument:
                writer.writeEndDocument();
                // done.
                break;
            case QXmlStreamReader::StartElement:
                if (reader.qualifiedName() == "Identifier") {
                    isIdent = true;
                    writer.writeStartElement(reader.namespaceUri().toString(), reader.name().toString());
                    identAttr = reader.attributes();
                    // will write attributes later when we know about
                    // the mapping.
                } else {
                    writer.writeStartElement(reader.namespaceUri().toString(), reader.name().toString());
                    QXmlStreamAttributes a = reader.attributes();
                    if (a.count())
                        writer.writeAttributes(a);
                }
                break;
            case QXmlStreamReader::EndElement:
                if (isIdent)
                    isIdent = false;
                writer.writeEndElement();
                break;
            case QXmlStreamReader::Characters:
                if (isIdent) {
                    QString identText = reader.text().toString();
                    if (revert) {
                        if (!contains(identAttr, "localIdentifier")
                                || booleanValue(identAttr, "localIdentifier"))
                        {
                            setBooleanValue(identAttr, "localIdentifier", false);
                            identText = map(identText,
                                    source == Client ? Server : Client);
                        }
                    } else {
                        // should only grab this if it is:
                        // The first identifier seen, not just first non-null
                        // The client mapping for this id.
                        if (firstIdent) {
                            if (source == Client) {
                                clientId = identText;
                            } else if (canMap(identText, Server)) {
                                clientId = map(identText, Server);
                            }
                            firstIdent = false;
                        }

                        if (canMap(identText, source)) {
                            identText = map(identText, source);
                        } else {
                            setBooleanValue(identAttr, "localIdentifier", false);
                        }
                    }
                    if (identAttr.count()) {
                        writer.writeAttributes(identAttr);
                    }
                    writer.writeCharacters(identText);
                } else {
                    if (reader.isWhitespace())
                        break;
                    if (reader.isCDATA())
                        writer.writeCDATA(reader.text().toString());
                    else
                        writer.writeCharacters(reader.text().toString());
                }
                break;
            case QXmlStreamReader::Comment:
                writer.writeComment(reader.text().toString());
                break;
            case QXmlStreamReader::DTD:
                writer.writeDTD(reader.text().toString());
                break;
        }
    }
    array = result;
    return clientId;
}

bool QSyncMerge::contains(const QXmlStreamAttributes &attr, const QString &qName)
{
    return !attr.value(qName).isNull();
}

bool QSyncMerge::booleanValue(const QXmlStreamAttributes &attr, const QString &qName)
{
    static const char *ttext = "true";
    static const char *tnum = "1";
    QStringRef v = attr.value(qName);
    return v == ttext || v == tnum;
}

void QSyncMerge::setBooleanValue(QXmlStreamAttributes &attr, const QString &qName, bool value)
{
    static const char *ttext = "true";
    static const char *ftext = "false";

    // this is the hard one.  attr is an array.  If we want to replace, we
    // have to iterate.
    int i;
    for (i = 0; i < attr.count(); ++i) {
        if (attr[i].qualifiedName() == qName) {
            attr[i] = QXmlStreamAttribute(qName, value ? ttext : ftext);
            return;
        }
    }
    attr.append(qName, value ? ttext : ftext);
}

void QSyncMerge::revertIdentifierMapping(const Conflict &conflict)
{
    TRACE(QDSync) << "QSyncMerge::revertIdentifierMapping";
    // for each byte array in the conflict, parseIdent(array, source, true)
    QByteArray crecord = conflict.client.record;
    QByteArray srecord = conflict.server.record;

    parseIdentifiers(crecord, Client, true);
    parseIdentifiers(srecord, Server, true);

    // update change tables
    QSqlQuery q = d->query();
    q.prepare("UPDATE clientchanges SET changedata = :b WHERE clientid = :r AND changetype = :t");
    q.bindValue(":b", crecord);
    q.bindValue(":r", conflict.client.id);
    q.bindValue(":t", int(conflict.client.type));
    q.exec();

    q.prepare("UPDATE serverchanges SET changedata = :b WHERE clientid = :r AND changetype = :t");
    q.bindValue(":b", srecord);
    q.bindValue(":r", conflict.server.id);
    q.bindValue(":t", int(conflict.server.type));
    q.exec();

    // then delete mapping line from ident tables.
    q.prepare("DELETE FROM identifiermap2 WHERE serverid = :s AND clientid = :c");
    q.bindValue(":s", conflict.server.id);
    q.bindValue(":c", conflict.client.id);
    q.exec();
}

void QSyncMerge::setDatasource( const QString &datasource )
{
    d->datasource = datasource;
}

// =====================================================================

QDebug _SyncLog()
{
    TRACE(TRACE) << "SyncLog";
    static QFile *file = 0;
    if ( !file ) {
        file = new QFile( DesktopSettings::homePath()+"/synclog.log" );
        LOG() << "Writing to" << DesktopSettings::homePath()+"/synclog.log";
        file->remove();
        bool ok = file->open( QIODevice::Append );
        Q_ASSERT(ok);
    }
    return QDebug( file );
}

