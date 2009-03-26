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

#ifndef PHONEMIGRATE_H
#define PHONEMIGRATE_H

#include <QDateTime>
#include <QString>

class QSqlDatabase;
class QDBMigrationEngine;
class QStringList;

class PhoneMigrate // : public DBMigrationEngine
{
public:
    PhoneMigrate(QDBMigrationEngine *engine);
    ~PhoneMigrate();

    bool migrate();
private:
    bool migrate(const QSqlDatabase &db, const QString &table, int version);
    const QStringList &tables() const;

    QString copyText(const QString &table);
    QDateTime syncTime;
    QDBMigrationEngine *mi;
};

#endif
