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

#ifndef MIGRATEENGINE_H
#define MIGRATEENGINE_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextStream>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <private/qtopiasqlmigrateplugin_p.h>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTimer>
#endif

#ifndef QTOPIA_CONTENT_INSTALLER
class QDBMigrationEngine : public QObject, public QtopiaSqlMigratePlugin
{
    Q_OBJECT
    Q_INTERFACES(QtopiaSqlMigratePlugin)
#else
class QDBMigrationEngine
{
#endif
public:
    QDBMigrationEngine();

    bool migrate(QSqlDatabase *database, bool system = false);

    int tableVersion(const QString &tableName);
    bool setTableVersion(const QString &tableName, int versionNum);
    bool copyTable(const QString &from, const QString &to);
    bool ensureSchema(const QString &table, bool transact=false) {return ensureSchema(QStringList() << table, transact);}
    bool ensureSchema(const QStringList &list, bool transact=false);
    void setDatabase(const QSqlDatabase& database) { db=database; }
    const QSqlDatabase& database() { return db; }
    bool loadSchema(QTextStream &ts, bool transact=false);
    bool check(bool result, int line, const char *file, const char *message);
    bool exec(QSqlQuery &query, int line, const char *file);
    bool exec(const QString &query, int line, const char *file);
    static QDBMigrationEngine *instance();
private:
    bool checkIntegrity(const QSqlDatabase &database);
    bool verifyLocale( const QSqlDatabase &database );
    QByteArray transformString( const QString &string ) const;

    QDBMigrationEngine *mi;

    QSqlDatabase db;
    bool failed;
};


#define CHECK(result) {if(mi->check((result), __LINE__, __FILE__, #result) == false) return false; }
#define EXEC(query) {if(mi->exec((query), __LINE__, __FILE__) == false) return false; }

#endif
