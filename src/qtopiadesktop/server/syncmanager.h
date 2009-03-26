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
#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QProgressDialog>
#include <QMap>

class QDServerSyncPlugin;
class QDClientSyncPlugin;
class QSyncProtocol;

class SyncManager : public QProgressDialog
{
    Q_OBJECT
public:
    enum Status { Waiting, Ready, NotReady, Finished };

    SyncManager( QWidget *parent = 0 );
    ~SyncManager();

    void init();

    int errors();
    QObject *syncObject();

public slots:
    void abort();

private slots:
    void next();
    void syncComplete();
    void syncError(const QString &error);
    void progress();
    void clientReadyForSync( bool ready );
    void serverReadyForSync( bool ready );
    void _finished( int result );
    void clientFinishedSync();
    void serverFinishedSync();

private:
    void showEvent( QShowEvent *e );
    void closeEvent( QCloseEvent *e );
    void readyForSync();
    void finishedSync();

    QMap<QDServerSyncPlugin*,QDClientSyncPlugin*> pending;
    QSyncProtocol *protocol;
    int mError;
    int completed;
    Status clientStatus;
    Status serverStatus;
    QDClientSyncPlugin *client;
    QDServerSyncPlugin *server;
    bool complete;
    QObject *mSyncObject;
    bool canAbort;
    bool abortLater;
};

#endif
