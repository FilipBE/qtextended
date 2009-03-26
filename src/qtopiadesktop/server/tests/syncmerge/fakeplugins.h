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

#ifndef FAKEPLUGINS_H
#define FAKEPLUGINS_H

#include <qdplugin.h>
#include <private/qdplugin_p.h>
#include "qsyncprotocol.h"
#include <QtXml>

class SyncData
{
public:
    QList<QByteArray> do_add;
    QList<QByteArray> expect_add;
    QList<QByteArray> did_add;

    QList<QByteArray> do_edit;
    QList<QByteArray> expect_edit;
    QList<QByteArray> did_edit;

    QList<QString> do_delete;
    QList<QString> did_delete;
    QList<QString> expect_delete;

    QList<QString> did_map;
    QList<QString> expect_map;

    QByteArray reference_schema;

    QDateTime lastSync;
    QDateTime nextSync;

    void clearSyncData()
    {
#define CLEAR(add)\
        do_##add.clear();\
        expect_##add.clear();\
        did_##add.clear()

        CLEAR(add);
        CLEAR(edit);
        CLEAR(delete);

#undef CLEAR

        did_map.clear();
        expect_map.clear();

        reference_schema = QByteArray();
    }

    QString unescape( const QString &string )
    {
        QString ret = string;
        ret.replace(QRegExp("&lt;"), "<");
        ret.replace(QRegExp("&gt;"), ">");
        ret.replace(QRegExp("&amp;"), "&");
        return ret;
    }

    bool getIdentifier( const QByteArray &record, QString &id, bool &local )
    {
        QXmlStreamReader reader(record);
        bool isIdent = false;
        bool leave = false;
        while (!reader.atEnd()) {
            switch(reader.readNext()) {
                case QXmlStreamReader::NoToken:
                case QXmlStreamReader::Invalid:
                    return false;
                case QXmlStreamReader::StartElement:
                    if (reader.qualifiedName() == "Identifier") {
                        isIdent = true;
                        QXmlStreamAttributes identAttr = reader.attributes();
                        QStringRef v = identAttr.value("localIdentifier");
                        if ( v.isNull() || v == "true" || v == "1" )
                            local = true;
                        else
                            local = false;
                    }
                    break;
                case QXmlStreamReader::EndElement:
                    if (isIdent)
                        leave = true;
                    break;
                case QXmlStreamReader::Characters:
                    if (isIdent)
                        id = unescape(reader.text().toString());
                    break;
                default:
                    break;
            }
            if ( leave )
                break;
        }
        return true;
    }
};

class FakeClient : public QDClientSyncPlugin, public SyncData
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(FakeClient,QDClientSyncPlugin)
public:
    void init() { d = new QDPluginData; internal_init(); }
    QString id() { return "client.id"; }
    QString displayName() { return "Client"; }

    QString dataset() { return "dataset"; }

    void serverSyncRequest(const QString &) {}
    void serverIdentity(const QString &) {}
    void serverVersion(int, int, int)
    {
        emit clientIdentity("client");
        emit clientVersion(4, 3, 0);
    }
    void serverSyncAnchors(const QDateTime &, const QDateTime &)
    {
        QDateTime _ls = lastSync;
        lastSync = nextSync;
        emit clientSyncAnchors(_ls, nextSync);
    }
    void createServerRecord(const QByteArray &record)
    {
        int idstart = record.indexOf("server.");
        Q_ASSERT(idstart != -1);
        int idend = record.indexOf('<', idstart);
        Q_ASSERT(idend != -1);
        QByteArray serverid = record.mid(idstart, idend-idstart);
        QByteArray clientid = serverid;
        clientid.replace("server", "client");
        did_add.append(record);
        did_map.append(QString("%1 %2").arg(QString(serverid)).arg(QString(clientid)));
        emit mappedId(serverid, clientid);
    }
    void replaceServerRecord(const QByteArray &record)
    {
        did_edit.append(record);
    }
    void removeServerRecord(const QString &id)
    {
        did_delete.append(id);
    }
    void requestTwoWaySync() { requestSlowSync(); }
    void requestSlowSync()
    {
        // fetch changes
        foreach (const QByteArray &record, do_add)
            emit createClientRecord(record);
        foreach (const QByteArray &record, do_edit)
            emit replaceClientRecord(record);
        foreach (const QString &id, do_delete)
            emit removeClientRecord(id);
        emit clientEnd();
    }
    void serverError()
    {
        emit clientError();
    }
    void serverEnd()
    {
        emit clientEnd();
    }
};

class FakeServer : public QDServerSyncPlugin, public SyncData
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(FakeServer,QDServerSyncPlugin)
public:
    void init() { d = new QDPluginData; internal_init(); }
    QString id() { return "server.id"; }
    QString displayName() { return "Server"; }

    QString dataset() { return "dataset"; }
    QByteArray referenceSchema() { return reference_schema; }

    void fetchChangesSince(const QDateTime &timestamp)
    {
        if ( timestamp != QDateTime() && timestamp != lastSync.addSecs(1) ) {
            qWarning() << "BUG! timestamp != lastSync.addSecs(1)!";
            qWarning() << timestamp;
            qWarning() << lastSync.addSecs(1);
            emit serverError();
            return;
        }
        lastSync = nextSync;
        // fetch changes
        foreach (const QByteArray &record, do_add)
            emit createServerRecord(record);
        foreach (const QByteArray &record, do_edit)
            emit replaceServerRecord(record);
        foreach (const QString &id, do_delete)
            emit removeServerRecord(id);
        emit serverChangesCompleted();
    }
    void createClientRecord(const QByteArray &record)
    {
        int idstart = record.indexOf("client.");
        Q_ASSERT(idstart != -1);
        int idend = record.indexOf('<', idstart);
        Q_ASSERT(idend != -1);
        QByteArray clientid = record.mid(idstart, idend-idstart);
        QByteArray serverid = clientid;
        serverid.replace("client", "server");
        did_add.append(record);
        did_map.append(QString("%1 %2").arg(QString(serverid)).arg(QString(clientid)));
        emit mappedId(serverid, clientid);
    }
    void replaceClientRecord(const QByteArray &record)
    {
        QString identifier;
        bool local;
        bool ok = getIdentifier( record, identifier, local );
        if ( !ok || !local )
            qDebug() << "getIdentifier" << "identifier" << identifier << "local" << local << "ok" << ok;
        Q_ASSERT(ok);
        Q_ASSERT(local);
        did_edit.append(record);
    }
    void removeClientRecord(const QString &id)
    {
        did_delete.append(id);
    }

    void beginTransaction(const QDateTime &) {}
    void abortTransaction() {}
    void commitTransaction() {}
};

#endif
