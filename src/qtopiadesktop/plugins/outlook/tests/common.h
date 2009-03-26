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

#ifndef COMMON_H
#define COMMON_H

#include <qd_common.h>
#include <qtopiadesktop>
#include "pluginmanager.h"
#include <QSignalSpy>
#include "outlooksync.h"
#include <desktopsettings.h>
#include "outlookthread.h"
#include <private/qdplugin_p.h>
#include <QProcess>

class DesktopWrapper : public CenterInterface
{
public:
    DesktopWrapper( QDPlugin *p, QObject *syncObject ) : mSyncObject( syncObject ) {}
    QDDevPlugin *currentDevice() { return 0; }
    const QDLinkPluginList linkPlugins() { return QDLinkPluginList(); }
    const QDDevPluginList devicePlugins() { return QDDevPluginList(); }
    QDPlugin *getPlugin( const QString &id ) { return 0; }
    QObject *syncObject() { return mSyncObject; }
    QObject *mSyncObject;
};

class QtopiaDesktopApplication
{
    friend class TestOutlookComms;
public:
    static void plugin_internal_init( QDPlugin *p ) { p->internal_init(); }
};

#define INIT_TEST_CASE_BODY(selected_plugin,enable_trace,wait_for_debugger)\
    QD_COMMON_INIT_TEST_CASE_BODY(enable_trace,wait_for_debugger)\
\
    syncObject = new QObject();\
\
    /* get the requested plugin */\
    pm = new PluginManager( this );\
    pm->setupPlugins( false );\
    foreach ( QDPlugin *plugin, pm->plugins() ) {\
        /*qDebug() << "QDPlugin::init" << "for plugin" << plugin->displayName();*/\
        pm->pluginData(plugin)->center = new DesktopWrapper( plugin, syncObject );\
        plugin->init();\
        QtopiaDesktopApplication::plugin_internal_init( plugin );\
    }\
    QDPlugin *p = 0;\
    foreach ( QDPlugin *plugin, pm->plugins() ) {\
        if ( plugin->id() == selected_plugin ) {\
            p = plugin;\
            break;\
        }\
    }\
    thePlugin = qobject_cast<OutlookSyncPlugin*>(p);\
    QVERIFY(thePlugin);\
\
    /* Force Qt Extended Sync Agent to have no memory of any Outlook events */\
    DesktopSettings settings(thePlugin->id());\
    settings.setValue( "rememberedIds", QStringList() );\
\
    /* Clean out the mapping database */\
    QFile file( DesktopSettings::homePath()+"qtopia.sqlite" );\
    if ( file.exists() )\
        file.remove();\
    QVERIFY(!file.exists());

#define PREPARE_FOR_SYNC_BODY()\
    QSignalSpy spy( thePlugin, SIGNAL(readyForSync(bool)) );\
    thePlugin->prepareForSync();\
    /* Async reply... just idle the event loop while we wait */\
    while ( spy.count() == 0 )\
        qApp->processEvents();\
    QList<QVariant> args = spy.takeFirst();\
    bool ok = args.at(0).toBool();\
    QVERIFY(ok);

#define CLEANUP_TEST_CASE_BODY()\
    thePlugin->finishSync();\
    delete syncObject;\
    delete pm;\
    pm = 0;

#define SANITIZE(array) QString(array).replace("\n","").replace(QRegExp("> +<"), "><").replace(QRegExp("<\\?xml .*\\?>"),"").trimmed().toLocal8Bit()

class testcase : public QObject
{
protected:
    QObject *syncObject;
    PluginManager *pm;
    OutlookSyncPlugin *thePlugin;
    QList<QByteArray> slowRecords;
    QList<QByteArray> todayRecords;
    QMap<QString,QString> idMap;

#define VERIFY(x) if ( !(x) ) { qWarning() << "VERIFY(" << #x << ") FAILED!" << __FILE__ << __LINE__; return false; }
    bool cleanOutTable()
    {
        QSignalSpy spyCreateServerRecord( thePlugin, SIGNAL(createServerRecord(QByteArray)) );
        QSignalSpy spyRemoveServerRecord( thePlugin, SIGNAL(removeServerRecord(QString)) );
        QSignalSpy spyReplaceServerRecord( thePlugin, SIGNAL(replaceServerRecord(QByteArray)) );
        QSignalSpy spyServerChangesCompleted( thePlugin, SIGNAL(serverChangesCompleted()) );

        thePlugin->fetchChangesSince( QDateTime() ); // force a slow sync
        // Async reply... just idle the event loop while we wait
        //qDebug() << "waiting for serverChangesCompleted" << __LINE__;
        while ( spyServerChangesCompleted.count() == 0 )
            qApp->processEvents();
        //qDebug() << "got serverChangesCompleted" << __LINE__;

        // There should not be any edit or remove events
        VERIFY(spyRemoveServerRecord.count() == 0);
        VERIFY(spyReplaceServerRecord.count() == 0);

        // There may have been new events...
        while ( spyCreateServerRecord.count() ) {
            QByteArray record = spyCreateServerRecord.takeFirst().at(0).toByteArray();
            QString id;
            bool mapped;
            bool ok = thePlugin->getIdentifier( record, id, mapped );
            VERIFY(ok);
            VERIFY(mapped);
            // We want to clear out the addressbook so just remove every identifier we see.
            //qDebug() << "Remove record" << id;
            thePlugin->removeClientRecord( id );
        }
        return true;
    }

    bool checkForEmptyTable()
    {
        QSignalSpy spyCreateServerRecord( thePlugin, SIGNAL(createServerRecord(QByteArray)) );
        QSignalSpy spyRemoveServerRecord( thePlugin, SIGNAL(removeServerRecord(QString)) );
        QSignalSpy spyReplaceServerRecord( thePlugin, SIGNAL(replaceServerRecord(QByteArray)) );
        QSignalSpy spyServerChangesCompleted( thePlugin, SIGNAL(serverChangesCompleted()) );

        thePlugin->fetchChangesSince( QDateTime() ); // force a slow sync
        // Async reply... just idle the event loop while we wait
        //qDebug() << "waiting for serverChangesCompleted" << __LINE__;
        while ( spyServerChangesCompleted.count() == 0 )
            qApp->processEvents();
        //qDebug() << "got serverChangesCompleted" << __LINE__;

        // There should not be any edit or remove events
        VERIFY(spyRemoveServerRecord.count() == 0);
        VERIFY(spyReplaceServerRecord.count() == 0);

        // Unless something went wrong there should not be any new events
        VERIFY(spyCreateServerRecord.count() == 0);

        return true;
    }

    bool addClientRecord( QByteArray newRecord )
    {
        QSignalSpy spyMappedId( thePlugin, SIGNAL(mappedId(QString,QString)) );
        thePlugin->createClientRecord( newRecord );
        // Async reply... just idle the event loop while we wait
        //qDebug() << "waiting for mappedId" << __LINE__;
        while ( spyMappedId.count() == 0 )
            qApp->processEvents();
        //qDebug() << "got mappedId" << __LINE__;
        VERIFY(spyMappedId.count() == 1);
        QList<QVariant> args = spyMappedId.takeFirst();
        QString serverId = args.at(0).toString();
        QString clientId = args.at(1).toString();
        idMap[serverId] = clientId;
        return true;
    }

    bool checkForAddedItem( QByteArray expected )
    {
        QSignalSpy spyCreateServerRecord( thePlugin, SIGNAL(createServerRecord(QByteArray)) );
        QSignalSpy spyRemoveServerRecord( thePlugin, SIGNAL(removeServerRecord(QString)) );
        QSignalSpy spyReplaceServerRecord( thePlugin, SIGNAL(replaceServerRecord(QByteArray)) );
        QSignalSpy spyServerChangesCompleted( thePlugin, SIGNAL(serverChangesCompleted()) );

        // We're going to assume it takes less than 1 minute to run any particular step of the test
        thePlugin->fetchChangesSince( QDateTime::currentDateTime().addSecs(-60) );
        // Async reply... just idle the event loop while we wait
        //qDebug() << "waiting for serverChangesCompleted" << __LINE__;
        while ( spyServerChangesCompleted.count() == 0 )
            qApp->processEvents();
        //qDebug() << "got serverChangesCompleted" << __LINE__;

        // There should not be any new or remove events
        bool die = false;
        while ( spyRemoveServerRecord.count() > 0 ) {
            die = true;
            QString id = spyRemoveServerRecord.takeFirst().at(0).toString();
            qWarning() << "Unexpected remove of id" << id;
        }
        while ( spyCreateServerRecord.count() > 0 ) {
            die = true;
            QByteArray data = spyCreateServerRecord.takeFirst().at(0).toByteArray();
            QString id;
            bool local;
            bool ok = thePlugin->getIdentifier( data, id, local );
            VERIFY(ok);
            VERIFY(local);
            data.replace(id.toLocal8Bit(), idMap[id].toLocal8Bit());
            data = SANITIZE(data);
            qDebug() << "Unexpected create" << endl << data;
        }
        VERIFY(!die);
        VERIFY(spyReplaceServerRecord.count() == 1);
        QByteArray data = spyReplaceServerRecord.takeFirst().at(0).toByteArray();
        QString id;
        bool local;
        bool ok = thePlugin->getIdentifier( data, id, local );
        VERIFY(ok);
        VERIFY(local);
        data.replace(id.toLocal8Bit(), idMap[id].toLocal8Bit());
        data = SANITIZE(data);
        expected = SANITIZE(expected);
        if ( data != expected ) {
            qDebug() << "expected" << endl << expected;
            qDebug() << "got" << endl << data;
        }
        VERIFY(data == expected);
        return true;
    }

    bool editClientRecord( const QByteArray &newRecord )
    {
        thePlugin->replaceClientRecord( newRecord );
        return true;
    }
};

#endif
