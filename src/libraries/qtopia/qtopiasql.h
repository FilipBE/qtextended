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

#ifndef QTOPIASQL_H
#define QTOPIASQL_H

#include <qtopiaglobal.h>

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <qcontent.h>

class QStringList;
class QtopiaSqlPrivate;
class QtopiaSqlMigratePlugin;

// create table support required?
// named db support required (qtopia name of default db?)
class QTOPIA_EXPORT QtopiaSql {
public:
    static QtopiaSql *instance();

    void openDatabase();

    static int stringCompare(const QString &, const QString &);

    QSqlDatabase &systemDatabase();
    void loadConfig(const QString &type, const QString &name, const QString &user);

    QSqlError exec(const QString &query, QSqlDatabase& db, bool inTransaction=true );
    QSqlError exec(QSqlQuery &query, QSqlDatabase& db, bool inTransaction=true );
    void attachDB(const QString& path);
    void attachDB(const QString& path, const QString &dbPath);
    void detachDB(const QString& path);
    QtopiaDatabaseId databaseIdForPath(const QString& path);
    QtopiaDatabaseId databaseIdForDatabasePath(const QString& dbPath);
    QSqlDatabase &database(const QtopiaDatabaseId& id);
    const QList<QtopiaDatabaseId> databaseIds();
    const QList<QSqlDatabase> databases();
    bool isDatabase(const QString &path);
    QString databasePathForId(const QtopiaDatabaseId& id);
    bool isValidDatabaseId(const QtopiaDatabaseId& id);

    static QString escape(const QString &);
    QString escapeString(const QString &input);
    QSqlDatabase applicationSpecificDatabase(const QString &appname);
    bool ensureTableExists(const QString &table, QSqlDatabase &db );
    bool ensureTableExists(const QStringList &, QSqlDatabase& );

    void logQuery(const QSqlQuery &q);
private:
    void attachDB(const QString &path, const QString &dbPath, QtopiaSqlMigratePlugin *plugin);
    QtopiaSql();
    void closeDatabase();
    QSqlDatabase *connectDatabase(const QString &connName);
    void init(QSqlDatabase &db, bool force=false);
    void loadConfig();
    void saveConfig();
    void connectDiskChannel();

    QtopiaSqlPrivate *d();

    friend class QtopiaApplication;
    friend class QtopiaSqlPrivate;
};

#endif
