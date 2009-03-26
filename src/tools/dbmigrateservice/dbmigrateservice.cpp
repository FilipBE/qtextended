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

#include "dbmigrateservice.h"

#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QtopiaSql>
#include <qtopialog.h>
#include <QTextCodec>
#include <QMimeType>
#include <qtopianamespace.h>
#include <QStorageMetaInfo>
#include <QDSData>
#include <QtopiaApplication>
#include <QPluginManager>
#include <private/qtopiasqlmigrateplugin_p.h>

bool MigrationEngineService::ensureSchema(const QStringList &list, QSqlDatabase &db)
{
    QStringList tables = db.tables();
    foreach(QString table, list) {
        if (tables.contains(table, Qt::CaseInsensitive))
            continue;
        // load schema.
        QFile data(QLatin1String(":/QtopiaSql/") + db.driverName() + QLatin1String("/") + table);

        if (!data.open(QIODevice::ReadOnly))
            return false;

        QTextStream ts(&data);
        // read assuming utf8 encoding.
        ts.setCodec(QTextCodec::codecForName("utf8"));
        ts.setAutoDetectUnicode(true);

        if (!loadSchema(ts, db))
            return false;
    }
    return true;
}

bool MigrationEngineService::loadSchema(QTextStream &ts, QSqlDatabase &db)
{
    db.transaction();

    QString qry = "";
    while (!ts.atEnd()) {
        /*
        Simplistic parsing.
        no comments in middle of line
        no starting another sql statment on same line one ends.

        For now, shouldn't be a problem.
        */

        QString line = ts.readLine();
        // comment, remove.
        if (line.contains (QLatin1String("--")))
            line.truncate (line.indexOf (QLatin1String("--")));
        if (line.trimmed ().length () == 0)
            continue;
        qry += line;

        // no ;, query spans to next line, keep going.
        if ( line.contains( ';' ) == false) {
            qry += QLatin1String(" ");
            continue;
        }

        if (!QSqlQuery(db).exec(qry)) {
            db.rollback();

            return false;
        }

        qry = QLatin1String("");
    }

    if(!qry.isEmpty() && !QSqlQuery(db).exec(qry)) {
        db.rollback();

        return false;
    }


    return db.commit();

}

bool MigrationEngineService::doMigrate(const QStringList &args)
{
#ifndef QTOPIA_CONTENT_INSTALLER
    // Ensure QStorageMetaInfo and QtopiaSql are up to date.
    QStorageMetaInfo::instance()->update();
#endif

    // this should be passed a list of database paths
    QList<QtopiaDatabaseId> databaseIds;
    if(args.count() > 0)
    {
        foreach(const QString &arg, args) {
            const QtopiaDatabaseId &id( QtopiaSql::instance()->databaseIdForDatabasePath(arg) );
            if(!databaseIds.contains(id))
                databaseIds.append(id);
        }
    }
    else
        databaseIds = QtopiaSql::instance()->databaseIds();

    QPluginManager manager(QLatin1String("qtopiasqlmigrate"));

    if (QtopiaSqlMigratePlugin *plugin = qobject_cast<QtopiaSqlMigratePlugin *>(
        manager.instance(QLatin1String("dbmigrate")))) {

        foreach (QtopiaDatabaseId id, databaseIds) {
            QSqlDatabase db = QtopiaSql::instance()->database(id);

            if (!plugin->migrate(&db))
                return false;
        }

        return true;
    } else {
        return false;
    }
}

MigrationEngineService::MigrationEngineService( QObject *parent )
        : QtopiaAbstractService( "DBMigrationEngine", parent )
{
    publishAll();
    QtopiaApplication::instance()->registerRunningTask("dbmigrate");
    unregistrationTimer.setSingleShot(true);
    unregistrationTimer.setInterval(60000);
    connect(&unregistrationTimer, SIGNAL(timeout()), this, SLOT(unregister()));
}

void MigrationEngineService::doMigrate( const QDSActionRequest &request )
{
    if(unregistrationTimer.isActive())
        unregistrationTimer.stop();
    unregistrationTimer.start();
    QDSActionRequest requestCopy( request );
    QString data = requestCopy.requestData().data();
    if (doMigrate(data.split("\n")))
        requestCopy.respond(QDSData(QByteArray("Y"), QMimeType("text/x-dbm-qstring")));
    else
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
}

void MigrationEngineService::unregister()
{
    QtopiaApplication::instance()->unregisterRunningTask("dbmigrate");
}

void MigrationEngineService::ensureTableExists( const QDSActionRequest &request )
{
    if(unregistrationTimer.isActive())
        unregistrationTimer.stop();
    unregistrationTimer.start();
    QDSActionRequest requestCopy( request );
    QString table=requestCopy.requestData().data();
    QStringList script=QString(requestCopy.auxiliaryData().data()).split("\n");
    if(script.count() < 5)
    {
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
        return;
    }

    // set up the database connection...
    QSqlDatabase db=QSqlDatabase::addDatabase(script.first(), QLatin1String("dbmigrateconnection"));
    script.erase(script.begin());
    db.setDatabaseName(script.first());
    script.erase(script.begin());
    db.setUserName(script.first());
    script.erase(script.begin());
    db.setPassword(script.first());
    script.erase(script.begin());
    db.setHostName(script.first());
    script.erase(script.begin());
    if(!db.open())
    {
        qLog(Sql) << "failed to open database";
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
        return;
    }
    db.open();

    QStringList tables = db.tables();

    if (tables.contains(table, Qt::CaseInsensitive))
    {
        db=QSqlDatabase();
        QSqlDatabase::removeDatabase(QLatin1String("dbmigrateconnection"));
        requestCopy.respond(QDSData(QByteArray("Y"), QMimeType("text/x-dbm-qstring")));
        return;
    }

    QTextStream ts(script.join("\n").toUtf8());
    // read assuming utf8 encoding.
    ts.setCodec(QTextCodec::codecForName("utf8"));
    ts.setAutoDetectUnicode(true);

    bool result = loadSchema(ts, db);

    db.close();
    db=QSqlDatabase();
    QSqlDatabase::removeDatabase(QLatin1String("dbmigrateconnection"));
    if (result)
        requestCopy.respond(QDSData(QByteArray("Y"), QMimeType("text/x-dbm-qstring")));
    else
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
}
