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
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTimer>

class MigrationEngineService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    MigrationEngineService( QObject *parent );

    static bool doMigrate(const QStringList &args);
public slots:
    void doMigrate( const QDSActionRequest &request );
    void ensureTableExists( const QDSActionRequest &request );
private:
    bool ensureSchema(const QStringList &list, QSqlDatabase &db);
    bool loadSchema(QTextStream &ts, QSqlDatabase &db);

    QTimer unregistrationTimer;
private slots:
    void unregister();
};
