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
#ifndef QSYNCPROTOCOL_H
#define QSYNCPROTOCOL_H

#include <QObject>
#include <QDateTime>

class QSyncMerge;
class QDServerSyncPlugin;
class QDClientSyncPlugin;

class QSyncProtocol : public QObject
{
    Q_OBJECT
public:
    QSyncProtocol(QObject *parent = 0);
    ~QSyncProtocol();

    void startSync( QDClientSyncPlugin *client, QDServerSyncPlugin *server );
    void abortSync();

signals:
    // started, progress, error and complete are currently not hooked up.
    //void syncStarted(const QString &);
    //void syncProgress(int done, int total);
    void progress();

    void syncError(const QString &);
    void syncComplete();

private slots:
    void markServerChangesComplete();

    // send
    void serverIdentity(const QString &);
    void serverVersion(int, int, int);
    void serverSyncRequest(const QString &datasource); // e.g. contacts/notes/calendar
    void serverSyncAnchors(const QDateTime &, const QDateTime &);
    void createServerRecord(const QByteArray &);
    void replaceServerRecord(const QByteArray &);
    void removeServerRecord(const QString &);
    void requestTwoWaySync();
    void requestSlowSync();
    void serverError();
    void serverEnd();

    // receive
    void clientIdentity(const QString &);
    void clientVersion(int, int, int);
    void clientSyncAnchors(const QDateTime &, const QDateTime &);
    void createClientRecord(const QByteArray &);
    void replaceClientRecord(const QByteArray &);
    void removeClientRecord(const QString &);
    void clientMappedId(const QString &, const QString &);
    void clientError();
    void clientEnd();

private:
    void abort(const QString &);

    void mergeAndApply();

    // doubles as 'data'
    enum Expecting {
        None,
        Header,
        SyncType,
        ClientDiff,
        IdMapping,
        Aborted
    };

    Expecting state;

    QString clientid;
    QString datasource;

    QDateTime m_clientLastSync;
    QDateTime m_clientNextSync;
    QDateTime m_serverLastSync;
    QDateTime m_serverNextSync;

    bool pendingServerChanges;
    bool pendingClientChanges;

    QSyncMerge *merge;
    QDClientSyncPlugin *client;
    QDServerSyncPlugin *server;
};

#endif
