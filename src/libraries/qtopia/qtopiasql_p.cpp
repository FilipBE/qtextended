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

#include <QtopiaSql>
#include "qtopiasql_p.h"
#ifndef QTOPIA_CONTENT_INSTALLER
#include <QApplication>
#include <qtopiasqlmigrateplugin_p.h>
#include <QPluginManager>
#endif
#include <QtopiaChannel>
#include <QtopiaIpcEnvelope>
#include <qtopialog.h>
#include <QTemporaryFile>
#include <qtopianamespace.h>
#include <QContentSet>

#define Q_USE_SQLITE
// #define Q_USE_MIMER
// #define Q_USE_MYSQL

#ifndef QTOPIA_HOST
#define USE_LOCALE_AWARE
#endif

#ifdef USE_LOCALE_AWARE
#ifdef Q_USE_SQLITE
#include <QSqlDriver>
#include <sqlite3.h>
#endif //Q_USE_SQLITE
#endif

#ifndef QTOPIA_HOST
#if defined(Q_WS_QWS)
#include <qwsdisplay_qws.h>
#elif defined(Q_WS_X11)
#include <qx11info_x11.h>
#endif
#endif

QtopiaSqlPrivate::QtopiaSqlPrivate ()
    : defaultConn(0)
    , guardMutex(QMutex::Recursive)
    , conId(0)
{
}

#ifndef QTOPIA_HOST
void QtopiaSqlPrivate::connectDiskChannel()
{
    if ( qApp->type() == QApplication::GuiServer ) {
        // The server handles mtab updates differently
    QtopiaChannel *channel = new QtopiaChannel( "QPE/Card", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(cardMessage(QString,QByteArray)) );
    }
    connect(QStorageMetaInfo::instance(), SIGNAL(disksChanged()), this, SLOT(disksChanged()));
    if ( qApp->type() != QApplication::Tty) {
        QtopiaChannel *channel2 = new QtopiaChannel( "QPE/System", this );
        connect( channel2, SIGNAL(received(QString,QByteArray)),
                    this, SLOT(systemMessage(QString,QByteArray)) );
    }
}
#endif

void QtopiaSqlPrivate::cardMessage(const QString &message, const QByteArray &data)
{
    Q_UNUSED(data);
    // The mtab has changed. Manually update our filesystems and then handle this before
    // passing the message on to other processes.
    if ( message == "mtabChanged()" ) {
        QStorageMetaInfo::instance()->update();
        disksChanged();
        QtopiaIpcEnvelope("QPE/QStorage", "updateStorage()");
    }
}

void QtopiaSqlPrivate::systemMessage(const QString &message, const QByteArray &data)
{
    Q_UNUSED(data);
    if ( message == "resetContent()" )
    {
        // okay we need to flush all (old) the database handles here...

        QHash<QtopiaDatabaseId, QSqlDatabase>::iterator hash;
        for( hash = dbs.begin(); hash != dbs.end(); hash++ )
        {
            if(!masterAttachedConns.contains(hash.key()))
            {
                dbs.take(hash.key()).close();
                if(connectionNames.keys().contains(hash.key()))
                {
                    QSqlDatabase::removeDatabase(connectionNames.take(hash.key()));
                }
            }
        }

    }
}

void QtopiaSqlPrivate::disksChanged(QtopiaSqlMigratePlugin *migratePlugin)
{
#ifdef QTOPIA_CONTENT_INSTALLER
    Q_UNUSED(migratePlugin);
#endif

#ifndef QTOPIA_HOST
    // update the "databases" array after rescanning QStorageMetaInfo
    qLog(Sql) << "disks changed... update database info";

#if defined(Q_WS_QWS)
    bool isServer = !qt_fbdpy || qApp->type() == QApplication::GuiServer;
#elif defined(Q_WS_X11)
    bool isServer = !QX11Info::display() || qApp->type() == QApplication::GuiServer;
#endif

    QFileSystemFilter f;
    f.content = QFileSystemFilter::Set;

    QStorageMetaInfo *storage = 0;
    QList<QFileSystem*> fileSystems;
#if defined(Q_WS_QWS)
    if( !qt_fbdpy )
#elif defined(Q_WS_X11)
    if( !QX11Info::display() )
#else
    if( false )
#endif
    {
        storage = new QStorageMetaInfo;

        fileSystems = storage->fileSystems(&f, false);
    }
    else
    {
        fileSystems = QStorageMetaInfo::instance()->fileSystems(&f, false);
    }

    QPluginManager *pluginManager = 0;

    foreach ( const QFileSystem *fs, fileSystems ) {
        // Have I seen this database already?
        if( !fs->isConnected() && fs->contentDatabase() ) {
            if ( !fs->prevPath().isEmpty() && dbPaths.key( fs->prevPath(), quint32(-1) ) != quint32(-1) ) {
                bool detach = true;

                // Don't detach if another storage device is connected on the same mount point.
                foreach (const QFileSystem *f, fileSystems) {
                    if (f != fs
                        && f->isConnected()
                        && f->contentDatabase()
                        && fs->prevPath() == f->path()) {
                        detach = false;
                        break;
                    }
                }

                if (detach) {
                    // Better late than never... detach this database now
                    qLog(Sql) << "Detaching database for path" << fs->prevPath();
                    QtopiaSql::instance()->detachDB(fs->prevPath());
                }
            }
        } else if ( fs->contentDatabase() ) {
            QtopiaDatabaseId databaseId = QtopiaSql::instance()->databaseIdForPath(fs->path());

            if ( databaseId != quint32(-1) && databaseId != 0 ) {
                continue;
            }

            if (!pluginManager && !migratePlugin && QApplication::type() == QApplication::GuiServer) {
                pluginManager = new QPluginManager(QLatin1String("qtopiasqlmigrate"));
                migratePlugin = qobject_cast<QtopiaSqlMigratePlugin *>(
                        pluginManager->instance(QLatin1String("dbmigrate")));
            }

            // qLog(Sql) << "fs->path() =" << fs->path() << "fs->isRemovable() =" << fs->isRemovable() << "fs->isWritable() =" << fs->isWritable();
            if ( !fs->isRemovable() && fs->isWritable() ) {
                // Simple case, just attach normally
                QtopiaSql::instance()->attachDB(fs->path(), databaseFile(fs->path()), migratePlugin);
            } else {
                QString dbPath = databaseFile(fs->path());
                bool dbExists = QFile::exists(dbPath);
                // If we're not writeable use a temporary file (sqlite can't open read-only files)
                bool useTemp = !fs->isWritable();
                // If we're removable, test that the file can be opened.
                // Read-only SD cards still show as writeable.
                if ( fs->isRemovable() ) {
                    // try and write the database.
                    QString tmpfilename=fs->path()+QLatin1String("/.qtopia_write_test");
                    if(QFile::exists(tmpfilename))
                        QFile::remove(tmpfilename);
                    QTemporaryFile tmpfile(tmpfilename);
                    if( !tmpfile.open() ) {
                        qLog(Sql) << "test file open failed";
                        useTemp = true;
                    }
                    else if( !tmpfile.write("test", 4) == 4 || !tmpfile.flush() ) {
                        qLog(Sql) << "test file write failed";
                        useTemp = true;
                    }
                }
                // qLog(Sql) << "dbExists =" << dbExists << "useTemp =" << useTemp;
                if ( useTemp ) {
                    QString origPath = dbPath;

                    // Use a file in /tmp (we know it's writeable)
                    // This is "unique" by path and lets apps share read-only databases
                    QString path = QDir::cleanPath(Qtopia::tempDir()+fs->path());
                    QDir().mkpath(path);    // ensure the path exists
                    dbPath = databaseFile(path);

                    // Copy the database (if it exists at the source and not at the dest)
                    // The server does this before any other apps get a chance to notice this file
                    if ( dbExists && isServer ) {
                        // If it already exists, unlink it (just in case anyone has it open)
                        if ( QFileInfo(dbPath).exists() ) {
                            QFile::remove(dbPath);
                        }
                        if( !QFile::copy(origPath, dbPath) ) {
                            QFile::remove(dbPath);
                            continue;
                        }
                    }
                }

                // Finally, attach to the database
                qLog(Sql) << "Attaching database" << dbPath << "for path" << fs->path();
                QtopiaSql::instance()->attachDB(fs->path(), dbPath, migratePlugin);
            }

            // Media may have been removed while dbmigrate was occuring, in this case we want to detach the database
            // and not scan the media.
            if( !fs->isConnected() ) {
                if ( !fs->prevPath().isEmpty() ) {
                    qLog(Sql) << "Detaching database for path" << fs->prevPath();
                    QtopiaSql::instance()->detachDB(fs->prevPath());
                }
            } else if ( qApp->type() == QApplication::GuiServer && fs->documents() ) {
                // Tell ContentServer to scan for documents (if this location contains them)
                QContentSet::scan(fs->path());
            }
        }
    }

    delete pluginManager;
    delete storage;
#endif
}

QString QtopiaSqlPrivate::databaseFile(const QString &path)
{
    QString file = QDir::cleanPath(path+QLatin1String("/")+QFileInfo(name).fileName());
    return file;
}

void QtopiaSqlPrivate::installSorting( QSqlDatabase &db)
{
#if defined(Q_USE_SQLITE) && defined(USE_LOCALE_AWARE)
        int sqliteLocaleAwareCompare(void *, int ll, const void *l, int rl, const void *r);
        QVariant v = db.driver()->handle();
        if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0) {
            // v.data() returns a pointer to the handle
            sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
            if (handle != 0) { // check that it is not NULL
                int result = sqlite3_create_collation(
                        handle,
                        "localeAwareCompare",
                        SQLITE_UTF16, // ANY would be nice, but we only encode in 16 anyway.
                        0,
                        sqliteLocaleAwareCompare);
                if (result != SQLITE_OK)
                    qLog(Sql) << "Could not add string collation function: " << result;
            } else {
                qLog(Sql) << "Could not get sqlite handle";
            }
        } else {
            qLog(Sql) << "handle variant returned typename " << v.typeName();
        }
#else
    Q_UNUSED(db);
#endif

}

Q_GLOBAL_STATIC(QtopiaSqlPrivate, internalinstance);

QtopiaSqlPrivate *QtopiaSqlPrivate::instance()
{
    return internalinstance();
}

void QtopiaSqlPrivate::preSetupDefaultConnection()
{
#ifdef Q_USE_SQLITE
 #if SQLITE_VERSION_NUMBER > 3003000
    sqlite3_enable_shared_cache(true);
 #endif
#endif
}

void QtopiaSqlPrivate::postSetupDefaultConnection()
{
}

