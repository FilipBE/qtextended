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

#ifndef PIMMIGRATE_H
#define PIMMIGRATE_H

#include <QDateTime>
#include <QString>

class QSqlDatabase;
class QDBMigrationEngine;
class QStringList;

class PimMigrate // : public DBMigrationEngine
{
public:
    PimMigrate(QDBMigrationEngine *engine);
    ~PimMigrate();

    bool migrate();
private:
    bool migrate(const QSqlDatabase &db, const QString &table, int version);
    const QStringList &tables() const;
    bool createContactEvents(const QSqlDatabase &db);
    bool createTodoEvents(const QSqlDatabase &db);
    bool generateContactLabels(const QSqlDatabase &db);

    QString queryText(const QString& type, const QString &table);
    QDateTime syncTime;
    QDBMigrationEngine *mi;
};

#endif
