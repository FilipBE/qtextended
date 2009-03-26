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
#include "syncmanager.h"
#include "pluginmanager.h"
#ifndef NO_SYNC
#include "qsyncprotocol.h"
#include "merge.h"
#endif
#include "qtopiadesktopapplication.h"

#include <qdplugin.h>
#include <trace.h>
#include <qcopenvelope_qd.h>

#include <QTimer>
#include <QApplication>
#include <QCloseEvent>

#ifndef SYNC_STEPS
#define SYNC_STEPS 6
#endif

SyncManager::SyncManager( QWidget *parent )
    : QProgressDialog( parent ), protocol( 0 ), mError( 0 ), mSyncObject( 0 ),
    canAbort( true ), abortLater( false )
{
}

void SyncManager::init()
{
    TRACE(QDSync) << "SyncManager::init";
    setWindowTitle( tr("Sync All") );
    setModal( true );
    //setAutoClose( false );
    setAutoReset( false );
    connect( this, SIGNAL(canceled()), this, SLOT(abort()) );
    connect( this, SIGNAL(finished(int)), this, SLOT(_finished(int)) );
    connect( qApp, SIGNAL(setConnectionState(int)), this, SLOT(abort()) );

    QList<QDServerSyncPlugin*> serverPlugins;
    QList<QDClientSyncPlugin*> clientPlugins;
    foreach ( QDPlugin *plugin, qdPluginManager()->plugins() ) {
        if ( QDServerSyncPlugin *server = qobject_cast<QDServerSyncPlugin*>(plugin) )
            serverPlugins << server;
        else if ( QDClientSyncPluginFactory *factory = qobject_cast<QDClientSyncPluginFactory*>(plugin) )
            foreach ( const QString &dataset, factory->datasets() ) {
                LOG() << "Creating dynamic QDClientSyncPlugin for dataset" << dataset;
                QDClientSyncPlugin *p = factory->pluginForDataset( dataset );
                qdPluginManager()->setupPlugin( p );
                ((QtopiaDesktopApplication*)qApp)->initializePlugin( p );
                clientPlugins << p;
            }
        else if ( QDClientSyncPlugin *client = qobject_cast<QDClientSyncPlugin*>(plugin) )
            clientPlugins << client;
    }

    foreach ( server, serverPlugins ) {
        LOG() << "server" << server->id() << server->dataset();
        foreach ( client, clientPlugins ) {
            if ( client->dataset() == server->dataset() ) {
                LOG() << "matched to" << client->id();
                Q_ASSERT(!pending.contains(server));
                pending[server] = client;
            }
        }
    }

    setMinimum( 0 );
    setMaximum( pending.count() * SYNC_STEPS );
    {
        QCopEnvelope e("QPE/QDSync", "syncSteps(int)");
        e << pending.count() * SYNC_STEPS;
    }
    setValue( 0 );
    {
        QCopEnvelope e("QPE/QDSync", "syncProgress(int)");
        e << value()+1;
    }
    completed = 0;
    client = 0;
    server = 0;

    SyncLog() << endl
              << "******************************************************************************" << endl
              << "Synchronization started at" << QDateTime::currentDateTime() << endl
              << "******************************************************************************" << endl
              << endl;
}

SyncManager::~SyncManager()
{
    TRACE(QDSync) << "SyncManager::~SyncManager";
#ifndef NO_SYNC
    if ( protocol )
        delete protocol;
#endif

    SyncLog() << "******************************************************************************" << endl
              << "Synchronization ended at" << QDateTime::currentDateTime() << endl
              << "******************************************************************************" << endl
              << endl;
}

void SyncManager::showEvent( QShowEvent * /*e*/ )
{
    TRACE(QDSync) << "SyncManager::showEvent";
    if ( pending.count() )
        QTimer::singleShot( 0, this, SLOT(next()) );
    else
        QTimer::singleShot( 0, this, SLOT(reject()) );
}

void SyncManager::next()
{
    TRACE(QDSync) << "SyncManager::next";
    Q_ASSERT( pending.count() );
    server = pending.begin().key();
    client = pending[server];
    pending.remove( server );
    setLabelText( tr("Syncing %1 with %2", "1=plugin, 2=plugin").arg(server->displayName()).arg(client->displayName()) );
    canAbort = false; // Don't process the abort until after the plugins are ready
    abortLater = false;
    qApp->processEvents();

    SyncLog() << "Synchronization between" << server->displayName() << "and" << client->displayName() << endl;

    clientStatus = Waiting;
    serverStatus = Waiting;
    connect( client, SIGNAL(readyForSync(bool)), this, SLOT(clientReadyForSync(bool)) );
    connect( server, SIGNAL(readyForSync(bool)), this, SLOT(serverReadyForSync(bool)) );
    client->prepareForSync();
    server->prepareForSync();
}

void SyncManager::clientReadyForSync( bool ready )
{
    TRACE(QDSync) << "SyncManager::clientReadyForSync" << ready;
    disconnect( client, SIGNAL(readyForSync(bool)), this, SLOT(clientReadyForSync(bool)) );
    Q_ASSERT( clientStatus == Waiting );
    clientStatus = ready?Ready:NotReady;
    if ( serverStatus != Waiting )
        readyForSync();
}

void SyncManager::serverReadyForSync( bool ready )
{
    TRACE(QDSync) << "SyncManager::serverReadyForSync" << ready;
    disconnect( server, SIGNAL(readyForSync(bool)), this, SLOT(serverReadyForSync(bool)) );
    Q_ASSERT( serverStatus == Waiting );
    serverStatus = ready?Ready:NotReady;
    if ( clientStatus != Waiting )
        readyForSync();
}

void SyncManager::readyForSync()
{
    TRACE(QDSync) << "SyncManager::readyForSync";
    canAbort = true;
    if ( abortLater ) {
        LOG() << "Processing delayed abort()";
        SyncLog() << "Synchronization aborted" << endl;
        abort();
        return;
    }
    bool clientOk = ( clientStatus == Ready );
    bool serverOk = ( serverStatus == Ready );
    LOG() << "client is" << (clientOk?"ok":"not ok") << "server is" << (serverOk?"ok":"not ok");
    SyncLog() << "client is" << (clientOk?"ok":"not ok") << "server is" << (serverOk?"ok":"not ok") << endl;
    complete = false;
    if ( clientOk && serverOk ) {
#ifndef NO_SYNC
        protocol = new QSyncProtocol;
        connect( protocol, SIGNAL(syncComplete()), this, SLOT(syncComplete()) );
        connect( protocol, SIGNAL(syncError(QString)), this, SLOT(syncError(QString)) );
        connect( protocol, SIGNAL(progress()), this, SLOT(progress()) );

        protocol->startSync( client, server );
#else
        QTimer::singleShot( 5000, this, SLOT(syncComplete()) );
#endif
    } else {
        mError++;
        QTimer::singleShot( 0, this, SLOT(syncComplete()) );
    }
}

void SyncManager::syncComplete()
{
    TRACE(QDSync) << "SyncManager::syncComplete";
    if ( complete ) {
        LOG() << "Can't call syncComplete again!";
        return;
    }
    complete = true;

    clientStatus = Waiting;
    serverStatus = Waiting;
    connect( client, SIGNAL(finishedSync()), this, SLOT(clientFinishedSync()) );
    connect( server, SIGNAL(finishedSync()), this, SLOT(serverFinishedSync()) );
    client->finishSync();
    server->finishSync();
}

void SyncManager::clientFinishedSync()
{
    TRACE(QDSync) << "SyncManager::clientFinishedSync";
    disconnect( client, SIGNAL(finishedSync()), this, SLOT(clientFinishedSync()) );
    Q_ASSERT( clientStatus == Waiting );
    clientStatus = Finished;
    if ( serverStatus != Waiting )
        finishedSync();
}

void SyncManager::serverFinishedSync()
{
    TRACE(QDSync) << "SyncManager::serverFinishedSync";
    disconnect( server, SIGNAL(finishedSync()), this, SLOT(serverFinishedSync()) );
    Q_ASSERT( serverStatus == Waiting );
    serverStatus = Finished;
    if ( clientStatus != Waiting )
        finishedSync();
}

void SyncManager::finishedSync()
{
    client = 0;
    server = 0;

    SyncLog() << "Synchronization finished" << endl << endl;
    if ( protocol )
        protocol->deleteLater();

    setValue( ++completed * SYNC_STEPS );

    if ( pending.count() ) {
        if ( protocol ) {
            connect( protocol, SIGNAL(destroyed()), this, SLOT(next()) );
            protocol = 0;
        } else {
            QTimer::singleShot( 0, this, SLOT(next()) );
        }
    } else {
        protocol = 0;
        reset();
        accept();
    }
}

void SyncManager::syncError( const QString &error )
{
    TRACE(QDSync) << "SyncManager::syncError" << "error" << error;
    mError++;
    if ( protocol )
        protocol->abortSync();
    syncComplete();
}

void SyncManager::abort()
{
    TRACE(QDSync) << "SyncManager::abort";
    if ( !canAbort ) {
        LOG() << "Can't abort now, registering delayed abort";
        abortLater = true;
        return;
    }
#ifndef NO_SYNC
    if ( protocol )
        protocol->abortSync();
    syncComplete();
#endif
    reject();
}

int SyncManager::errors()
{
    return mError;
}

void SyncManager::progress()
{
    TRACE(QDSync) << "SyncManager::progress";
    QCopEnvelope e("QPE/QDSync", "syncProgress(int)");
    setValue( value()+1 );
    e << value()+1;
}

void SyncManager::closeEvent( QCloseEvent *e )
{
    TRACE(QDSync) << "SyncManager::closeEvent";
    e->ignore();
    abort();
}

QObject *SyncManager::syncObject()
{
    if ( !mSyncObject )
        mSyncObject = new QObject( this );
    return mSyncObject;
}

void SyncManager::_finished( int )
{
    TRACE(QDSync) << "SyncManager::_finished";
    if ( mSyncObject ) {
        delete mSyncObject;
        mSyncObject = 0;
    }
}

