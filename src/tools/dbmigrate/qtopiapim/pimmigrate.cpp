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

#include "pimmigrate.h"

#include <QStringList>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QTimeZone>
#include <QDebug>
#include <QTextCodec>
#include <QFile>
#include <QSettings>
#include <QSqlError>

#include "quniqueid.h"

#include "../migrateengine.h"

#include <sqlite3.h>
#include <QSqlDriver>
#include <netinet/in.h>

// migration tables
const QStringList &PimMigrate::tables() const
{
    static QStringList tables;

    if (tables.count() == 0) {
        // in order of when they should be processed.
        tables << "changelog";
        tables << "sqlsources";

        tables << "appointments";
        tables << "appointmentcategories";
        tables << "appointmentcustom";
        tables << "appointmentexceptions";

        tables << "contacts";
        tables << "contactaddresses";
        tables << "contactcategories";
        tables << "contactcustom";
        tables << "contactphonenumbers";
        tables << "contactpresence";
        tables << "emailaddresses";

        tables << "tasks";
        tables << "taskcategories";
        tables << "taskcustom";

        tables << "simcardidmap";
        tables << "currentsimcard";

        tables << "googleid";

        tables << "pimdependencies";
        tables << "simlabelidmap";

    }

    return tables;
}

static QMap<QString, int> expectedVersions()
{
    static QMap<QString, int> versions;
    if (versions.count() == 0) {
        versions.insert("changelog", 110);
        versions.insert("sqlsources", 110);

        versions.insert("appointments", 110);
        versions.insert("appointmentcategories", 110);
        versions.insert("appointmentcustom", 110);
        versions.insert("appointmentexceptions", 110);

        versions.insert("contacts", 111); // 111 adds the label field
        versions.insert("contactaddresses", 110);
        versions.insert("contactcategories", 110);
        versions.insert("contactcustom", 111); // 111 adds some indices
        versions.insert("contactphonenumbers", 111); // 111 adds some indices
        versions.insert("emailaddresses", 110);
        versions.insert("contactpresence", 112); // 111 is new, 112 adds avatar

        versions.insert("tasks", 110);
        versions.insert("taskcategories", 110);
        versions.insert("taskcustom", 110);

        versions.insert("simcardidmap", 110);
        versions.insert("currentsimcard", 110);

        versions.insert("googleid", 110);

        versions.insert("pimdependencies", 110);
        versions.insert("simlabelidmap", 110);

        versions.insert("syncservers", 111);

    }
    return versions;
}

void convertRecIdFunc(sqlite3_context *context, int, sqlite3_value**values)
{
    int size = sqlite3_value_bytes(values[0]);
    if (size != 8)
        return;

    struct PairedUint {
        quint32 left;
        quint32 right;
    };
    // network byte order
    PairedUint *id = (PairedUint *)sqlite3_value_blob(values[0]);

    quint32 result = (ntohl(id->left) << 24) | (ntohl(id->right) & 0x00ffffff);

    sqlite3_result_int(context, result);
}


PimMigrate::~PimMigrate()
{}

//PimMigrate::PimMigrate() : syncTime(QTimeZone::current().toUtc(QDateTime::currentDateTime()))
PimMigrate::PimMigrate(QDBMigrationEngine *engine) : syncTime(QDateTime::currentDateTime().toUTC()), mi(engine)
{
}

bool PimMigrate::migrate()
{
    // 4.2.0 brings in the changelog table
    // 4.2.2 brings in a change to rec id's.
    // 4.4 adds the label field to contacts (version 111)
    // and the 'contactpresence' table in a temporary db
    // first ensure changelog exists.
    CHECK(mi->ensureSchema("changelog"));
    CHECK(mi->setTableVersion("changelog", 110));

    const QSqlDatabase &db  = mi->database();

    // add function to migrate recid.  Since only db this affects is sqlite, use sqlite func
    QVariant v = db.driver()->handle();
    if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
        CHECK(handle);
        int result = sqlite3_create_function( handle, "convertRecId", 1, SQLITE_ANY, 0, convertRecIdFunc, 0, 0);
        CHECK(result == SQLITE_OK);
    }

    // These are the tables to backup
    QStringList oldTables;
    oldTables << "taskcustom";
    oldTables << "taskcategories";
    oldTables << "tasks";
    oldTables << "emailaddresses";
    oldTables << "contactphonenumbers";
    oldTables << "contactcustom";
    oldTables << "contactcategories";
    oldTables << "contactaddresses";
    oldTables << "contacts";
    oldTables << "appointmentexceptions";
    oldTables << "appointmentcustom";
    oldTables << "appointmentcategories";
    oldTables << "appointments";

    // first we need to drop the old schema's in the right order.
    QStringList existingTables = db.tables();
    foreach(QString table, oldTables) {
        int v = mi->tableVersion(table);
        int expectedVersion = expectedVersions().value(table);
        if (existingTables.contains(table) && v < expectedVersion) {
            // back it up and drop.
            CHECK(mi->copyTable(table, table+"_old"));
            QSqlQuery query(db);
            CHECK(query.exec("DROP TABLE "+table));
        }
    }

    foreach(QString table, tables()) {
        CHECK(migrate(db, table, mi->tableVersion(table)));
    }

    // only one version so far.. change this line if version of 'syncServers' changes
    if (!existingTables.contains("syncServers")) {
        // sync server identification
        QSqlQuery q(db);
        CHECK(q.exec("CREATE TABLE syncServers (serverIdentity VARCHAR(255), datasource VARCHAR(255), lastSyncAnchor TIMESTAMP, UNIQUE(serverIdentity, datasource))"));
        CHECK(mi->setTableVersion("syncServers", 111));
    }
    return true;
}


bool PimMigrate::migrate(const QSqlDatabase &db, const QString &table, int version)
{
    if (version >= expectedVersions().value(table))
        return true;
    else {
        CHECK(mi->ensureSchema(table));
        CHECK(mi->setTableVersion(table, expectedVersions().value(table)));

        QStringList existingTables = db.tables();

        // if it was backed up
        if (existingTables.contains(table+"_old")) {
            QSqlQuery query(db);
            CHECK(query.prepare(queryText("copy", table)));
            CHECK(query.exec());

            // If the version was prior to changelogs, insert a fake entry
            if (version < 110) {
                if (table == "tasks" || table == "contacts" || table == "appointments") {
                    QSqlQuery changelog(db);
                    CHECK(changelog.prepare("INSERT INTO changelog (recid, created, modified, removed) SELECT recid, :dt1, :dt2, NULL FROM " + table + ";"));
                    changelog.bindValue("dt1", syncTime);
                    changelog.bindValue("dt2", syncTime);
                    CHECK(changelog.exec());
                }
            }

            // For contacts version 111, we have to generate the label
            if (table == "contacts" && version < 111) {
                CHECK(generateContactLabels(db));
            }

            //mi->dropTable(table+"_old");
            CHECK(query.exec("DROP TABLE "+table+"_old;"));
        }

        if (table == "pimdependencies") {
            CHECK(createContactEvents(db));
            CHECK(createTodoEvents(db));
        }
    }
    return true;
}

QString PimMigrate::queryText(const QString& queryType, const QString &table)
{
    const QSqlDatabase &db  = mi->database();

    QFile data(QLatin1String(":/QtopiaSql/") + queryType + QLatin1String("/") + db.driverName() + QLatin1String("/") + table);
    data.open(QIODevice::ReadOnly);
    QTextStream ts(&data);
    // read assuming utf8 encoding.
    ts.setCodec(QTextCodec::codecForName("utf8"));
    ts.setAutoDetectUnicode(true);

    // assumption, no comments or mult-statements.  those belong in rc file if anywhere.
    return ts.readAll();
}

bool PimMigrate::createContactEvents(const QSqlDatabase &db)
{
    bool ret = true;

    uint birthdayContext = QUniqueIdGenerator::mappedContext(QUuid("822d32bc-d646-4b36-b1fd-090b2199b725"));
    uint anniversaryContext = QUniqueIdGenerator::mappedContext(QUuid("5a72a3fe-f2a8-4cba-94bb-0880dac41520"));

    QSqlQuery createBirthdays(db);
    QSqlQuery createBirthdaysDeps(db);
    QSqlQuery createAnniversaries(db);
    QSqlQuery createAnniversariesDeps(db);

    // Birthdays
    CHECK(createBirthdaysDeps.prepare(queryText("generate", "contact_birthdays_deps")));
    createBirthdaysDeps.bindValue(":birthdaycontext", birthdayContext);
    CHECK(createBirthdaysDeps.exec());

    CHECK(createBirthdays.prepare(queryText("generate", "contact_birthdays")));
    createBirthdays.bindValue(":birthdaycontext", birthdayContext);
    createBirthdays.bindValue(":birthdaycontext2", birthdayContext);
    CHECK(createBirthdays.exec());

    // Anniversaries
    CHECK(createAnniversariesDeps.prepare(queryText("generate", "contact_anniversaries_deps")));
    createAnniversariesDeps.bindValue(":anniversarycontext", anniversaryContext);
    CHECK(createAnniversariesDeps.exec());

    CHECK(createAnniversaries.prepare(queryText("generate", "contact_anniversaries")));
    createAnniversaries.bindValue(":anniversarycontext", anniversaryContext);
    createAnniversaries.bindValue(":anniversarycontext2", anniversaryContext);
    CHECK(createAnniversaries.exec());

    return ret;
}

bool PimMigrate::createTodoEvents(const QSqlDatabase &db)
{
    bool ret = true;

    uint taskContext = QUniqueIdGenerator::mappedContext(QUuid("a2c69584-a85a-49c6-967b-6e2895f5c777"));

    QSqlQuery createTaskEvents(db);
    QSqlQuery createTaskEventsDeps(db);

    CHECK(createTaskEvents.prepare(queryText("generate", "task_duedates")));
    createTaskEvents.bindValue(":taskcontext", taskContext);
    createTaskEvents.bindValue(":taskcontext2", taskContext);
    CHECK(createTaskEvents.exec());

    CHECK(createTaskEventsDeps.prepare(queryText("generate", "task_duedates_deps")));
    createTaskEventsDeps.bindValue(":taskcontext", taskContext);
    CHECK(createTaskEventsDeps.exec());

    return ret;
}

/* Parse helper structure */
typedef struct {
    QString field;
    QString text;
} labelComponent;

/*
    looong function for generating the SQL to create a label in
    the same way as ContactSqlIO (a combination of initFormat, setFormat, and sqlLabel)
*/
static QString sqlLabel()
{
    QSettings config( "Trolltech", "Contacts" );
    config.beginGroup( "formatting" );
    int curfmtn = config.value( "NameFormat" ).toInt();
    QString value = config.value( "NameFormatFormat"+QString::number(curfmtn) ).toString();
    config.endGroup();

    QList< QList<labelComponent> > newFormat;

    QStringList validTokens; // Also doubles as the SQL column name
    validTokens << "title" << "firstname" << "middlename" << "lastname" << "suffix" << "company" << "department" << "jobtitle" << "office";

    QStringList tokens = value.split(' ', QString::SkipEmptyParts);
    QList<labelComponent> last;
    bool lastvalid = false;

    while(tokens.count() > 0) {
        QString token = tokens.takeFirst();
        if (validTokens.contains(token)) {
            lastvalid = true;
            labelComponent c;
            c.field = token;
            last.append(c);
        } else if (token == "|") {
            if (lastvalid)
                newFormat.append(last);
            lastvalid = false;
            last.clear();
        } else {
            token.replace("_", " ");
            labelComponent c;
            c.text = token;
            last.append(c);
        }
    }
    if (lastvalid)
        newFormat.append(last);

    int fc = newFormat.count();
    QString expression = "(CASE ";
    for (int i = 0; i < fc; i++) {
        QList<labelComponent> f = newFormat[i];
        expression += "WHEN ";
        bool firstkey = true;
        QListIterator<labelComponent> it(f);

        /* First create the condition */
        while(it.hasNext()) {
            labelComponent v = it.next();
            if (v.field.isEmpty())
                continue;
            if (!firstkey)
                expression += "AND ";
            firstkey = false;
            expression += v.field + " IS NOT NULL ";
        }
        expression += "THEN ";

        /* Now the result */
        QListIterator<labelComponent> fit(f);
        while(fit.hasNext()) {
            labelComponent v = fit.next();
            if (!v.field.isEmpty()) {
                expression += v.field + " ";
            } else {
                expression += "\"" + v.text + "\" ";
            }
            if (fit.hasNext())
                expression += "|| ";
        }
    }
    expression += "ELSE NULL END)";

    return expression;
}


bool PimMigrate::generateContactLabels(const QSqlDatabase &db)
{
    /* Duplicate some code from libqtopiapim */
    QSettings config( "Trolltech", "Contacts" );
    config.beginGroup( "formatting" );

    QString currentSql = sqlLabel();
    QSqlQuery q(db);
    CHECK(q.prepare("DROP INDEX contactslabelindex"));
    CHECK(q.exec());
    CHECK(q.prepare("UPDATE contacts SET label=" + currentSql));
    CHECK(q.exec());
    CHECK(q.prepare("CREATE INDEX contactslabelindex ON contacts(label)"));
    CHECK(q.exec());

    config.setValue("NameFormatSql", currentSql);

    return true;
}

