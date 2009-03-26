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

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaSql>
#include <QtopiaApplication>
#include <QSqlQuery>
#include <QSqlError>


//TESTED_CLASS=QtopiaSql
//TESTED_FILES=src/libraries/qtopia/qtopiasql.h

/*
    The tst_QtopiaSql class provides unit tests for the QtopiaSql class.
    Note that QtopiaSql doesn't appear to be intended for public use.
*/
class tst_QtopiaSql : public QObject
{
    Q_OBJECT
/*
    Test needs fixing w.r.t. dbmigrate.
private slots:
*/

private:
    void sql();
    void sql_data();

    void attach_detach();
    void attach_detach_data();

    void database_empty();

    void escapeString();
    void escapeString_data();

    void stringCompare();
    void stringCompare_data();

    void initTestCase();
    void cleanupTestCase();

private:
    QString testDataPath;
    QString testTable;
};





/* ================================== QTest additions ==================================== */
/* These macros and methods allow QCOMPARE to work for QSqlDatabase and QSqlError objects. */
namespace QTest
{

#define COMPARE_METHODS2(cast, method) \
  qCompare((cast)t1.method(), (cast)t2.method(), \
           QString("%1.%2()").arg(actual).arg(#method).toLatin1(), \
           QString("%1.%2()").arg(expected).arg(#method).toLatin1(), file, line)

#define COMPARE_METHODS(method) \
  qCompare(t1.method(), t2.method(), \
           QString("%1.%2()").arg(actual).arg(#method).toLatin1(), \
           QString("%1.%2()").arg(expected).arg(#method).toLatin1(), file, line)

template<>
inline bool qCompare(QSqlDatabase const &t1, QSqlDatabase const &t2,
                     const char *actual, const char *expected, const char *file, int line)
{
    return 
        COMPARE_METHODS(databaseName) &&
        COMPARE_METHODS(userName) &&
        COMPARE_METHODS(password) &&
        COMPARE_METHODS(hostName) &&
        COMPARE_METHODS(driverName) &&
        COMPARE_METHODS(port) &&
        COMPARE_METHODS(connectOptions);
}

template<>
inline bool qCompare(QSqlError const &t1, QSqlError const &t2,
                     const char *actual, const char *expected, const char *file, int line)
{
    return 
        COMPARE_METHODS(text) &&
        COMPARE_METHODS(driverText) &&
        COMPARE_METHODS(databaseText) &&
        COMPARE_METHODS2(int, type) &&
        COMPARE_METHODS(number) &&
        COMPARE_METHODS(isValid);
}

#undef COMPARE_METHODS
#undef COMPARE_METHODS2
}
/* ============================================ END Qtest additions ======================== */





QTEST_APP_MAIN(tst_QtopiaSql, QtopiaApplication)
#include "tst_qtopiasql.moc"

/* Must be declared as metatypes to use as test data. */
Q_DECLARE_METATYPE(QSqlError);
Q_DECLARE_METATYPE(QList< QList< QVariant > >);

/*?
    Initialisation before all test cases.
    Sets up directories for storing data.
    Also retrieves the system database, which forces QtopiaSql
    to do some private initialisation.
*/
void tst_QtopiaSql::initTestCase()
{
    testDataPath = QtopiaUnitTest::baseDataPath() + "/tst_qtopiasql";
    testTable = "great_table";

    QDir dir(testDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
        dir = QDir(testDataPath);
        QVERIFY(dir.exists());
    }

    QtopiaSql::instance()->systemDatabase();
}

/*?
    Cleanup after all test functions.
    Detaches all databases, deletes folder containing test
    data, and deletes test database tables.
*/
void tst_QtopiaSql::cleanupTestCase()
{
    /* Detach every database. */
    QList<QtopiaDatabaseId> ids = QtopiaSql::instance()->databaseIds();
    foreach (QtopiaDatabaseId id, ids) {
        /* Don't try to detach system database. */
        if (0 == id) continue;
        QtopiaSql::instance()->detachDB(QtopiaSql::instance()->databasePathForId(id));
    }

    /* Remove testDataPath. */
    QDir dir(testDataPath);
    if (dir.exists()) {
        foreach(QString file, dir.entryList()) {
            if (file != "." && file != "..")
                QVERIFY(dir.remove(file));
        }
        QVERIFY(dir.rmpath("."));
    }
    dir = QDir(testDataPath);
    QVERIFY(!dir.exists());


    /* Drop all created database tables in the system database. */
    QSqlDatabase base = QtopiaSql::instance()->systemDatabase();
    QtopiaSql::instance()->exec(QString("DROP TABLE IF EXISTS %1;").arg(testTable), base, false);
    for (int i = 0; i < 10; ++i) {
        QtopiaSql::instance()->exec(QString("DROP TABLE IF EXISTS %1%2;").arg(testTable).arg(i), base, false);
    }

}

/*?
    Test for QtopiaSql::stringCompare().

    This test calls QtopiaSql::stringCompare() on two strings
    and compares the sign of the result with an expected value.
*/
void tst_QtopiaSql::stringCompare()
{
    QFETCH(QString, string1);
    QFETCH(QString, string2);
    QFETCH(int, result);

    int ret = QtopiaSql::instance()->stringCompare(string1, string2);
    if (!result) QCOMPARE(ret, 0);
    else         QVERIFY(ret*result > 0); // same-sign test
}

/*?
    Data for stringCompare().
*/
void tst_QtopiaSql::stringCompare_data()
{
    QTest::addColumn<QString>("string1");
    QTest::addColumn<QString>("string2");
    QTest::addColumn<int>("result");

    QTest::newRow("first smaller 1")
        << "abcd"
        << "bcde"
        << -1;

    QTest::newRow("first smaller 2")
        << "foo bar"
        << "z"
        << -1;

    QTest::newRow("same 1")
        << "Baa Baa Black Sheep"
        << "Baa Baa Black Sheep"
        << 0;

    QTest::newRow("same 2")
        << "Foo, Bar & Baz Inc."
        << "Foo, Bar & Baz Inc."
        << 0;

    QTest::newRow("second smaller 1")
        << "zebra"
        << "yoyo"
        << 1;

    QTest::newRow("second smaller 2")
        << "yoyo"
        << "donkey"
        << 1;

    QTest::newRow("first empty")
        << ""
        << "hello world"
        << -1;

    QTest::newRow("second empty")
        << "hello world"
        << ""
        << 1;

    QTest::newRow("both empty")
        << ""
        << ""
        << 0;

    QTest::newRow("numbers")
        << "12345"
        << "23456"
        << -1;

    QTest::newRow("mixed 1")
        << "12345"
        << "p1zza"
        << -1;

    QTest::newRow("mixed 2")
        << "p1zza"
        << "401935822347"
        << 1;
}

/*?
    Test for QtopiaSql::escapeString().

    This test simply calls QtopiaSql::escapeString() on a string and
    compares the result with an expected value.
*/
void tst_QtopiaSql::escapeString()
{
    QFETCH(QString, raw);

    QTEST( QtopiaSql::instance()->escapeString(raw), "escaped" );
}

/*?
    Data for escapeString().

    Note that, since QtopiaSql::escapeString() is "sparsely" documented,
    exactly what should be escaped is not well defined.
*/
void tst_QtopiaSql::escapeString_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<QString>("escaped");

    QTest::newRow("unchanged")
        << "a simple string"
        << "a simple string";

    QTest::newRow("simple 1")
        << "you can't say that on TV!"
        << "you can''t say that on TV!";

    QTest::newRow("simple 2")
        << "tell me something I don't know"
        << "tell me something I don''t know";

    QTest::newRow("double 1")
        << "automated testing is ''the'' cool new thing"
        << "automated testing is ''''the'''' cool new thing";

    QTest::newRow("double 2")
        << "foo ''bar'' baz"
        << "foo ''''bar'''' baz";

    QTest::newRow("mixed 1")
        << "''few men would've believed the Martians were watching''"
        << "''''few men would''ve believed the Martians were watching''''";

    QTest::newRow("mixed 2")
        << "Listen to your mother when she tells you ''eat your greens'', they're good for you."
        << "Listen to your mother when she tells you ''''eat your greens'''', they''re good for you.";

    QTest::newRow("empty")
        << ""
        << "";
}

/*?
    Test for QtopiaSql::database when no databases have been attached.
    This test:
        * Checks that the system database is initially available with ID 0
        * Checks that all databases of ID 1..9999 are not attached, and
          requesting these databases results in the system database.
*/
void tst_QtopiaSql::database_empty()
{
    QSqlDatabase base0 = QtopiaSql::instance()->database(0);
    QSqlDatabase baseSystem = QtopiaSql::instance()->systemDatabase();
    QCOMPARE( base0, baseSystem );

    QSqlDatabase base;
    QSqlDatabase base_again;
    for (uint j = 1; j < 10000; ++j) {
        // We have not attached a database with ID j, so we should get two error messages.
        // Note additional space at end of message; this is put there by QDebug's << operator.
        for (int i = 0; i < 2; ++i)
            QTest::ignoreMessage(QtWarningMsg,
                                "Database handle doesnt exist in the "
                                "master list, returning the system database ");    

        base = QtopiaSql::instance()->database(j);
        base_again = QtopiaSql::instance()->database(j);
        QCOMPARE(base, base_again);
        // Since no databases have been attached, should be equal to
        // system database.
        QCOMPARE(base, baseSystem);
    }
}


/*?
    Test attaching and detaching databases.

    This test:
        * Attachs a database with a given path.
        * Retrieves the ID for the path and ensures it is not 0,
          as 0 represents the pre-existing system database.
        * Retrieves the database and ensures it is not equal
          to the system database.  Note that QtopiaSql returns
          the system database when failures occur.
        * Retrieves the path of the directory in which the database
          is stored, and ensures this directory is equal to the
          given path.
        * Runs a simple query on the database and ensures the
          result is as expected.
        * Re-attaches the same database and repeats the above
          checks.
        * Detaches the database and ensures QtopiaSql now
          returns the system database on subsequent attempts
          to retrieve the now-detached database.
        * Repeats the above steps many times to ensure behaviour
          remains consistent after attaching and detaching many
          times.

    It should be noted that this function does not test the cases
    where attaching and detaching are expected to fail.  At the
    moment, the behaviour of QtopiaSql appears to be undefined in
    these cases.
*/
void tst_QtopiaSql::attach_detach()
{
    QFETCH(QString, path);
    QSqlDatabase baseSystem = QtopiaSql::instance()->systemDatabase();

    uint id;
    QSqlDatabase base;
    QString dbPath;

    for (int i = 0; i < 100; ++i) {
        /* Attach database at the given path and retrieve the
         * created QSqlDatabase and related information. */
        QtopiaSql::instance()->attachDB(path);
        id = QtopiaSql::instance()->databaseIdForPath(path);
        base = QtopiaSql::instance()->database(id);
        dbPath = QtopiaSql::instance()->databasePathForId(id);

        /* Make sure we didn't get the system database and
         * the alleged database file exists in the right place. */
        QVERIFY(id != 0);
        QVERIFY(QFile::exists(dbPath));
        QVERIFY(dbPath.startsWith(path));
        QVERIFY(QtopiaSql::instance()->databaseIds().contains(id));

        QSqlError err = QtopiaSql::instance()->exec("SELECT * FROM not_existing_table;", base, false);
        QCOMPARE(err, QSqlError("Unable to fetch row", "no such table: not_existing_table",
                                QSqlError::ConnectionError, 21));
            
        // Attach a second time - should not harm anything
        QtopiaSql::instance()->attachDB(path);
        QCOMPARE( QtopiaSql::instance()->databaseIdForPath(path), id);
        QCOMPARE( QtopiaSql::instance()->database(id), base );
        QCOMPARE( QtopiaSql::instance()->databasePathForId(id), dbPath );
        QVERIFY( QFile::exists(dbPath) );
        
        // Forget about the database so it can be detached OK.
        base = QSqlDatabase();

        // Detach.
        QtopiaSql::instance()->detachDB(path);
        QVERIFY(!QtopiaSql::instance()->databaseIds().contains(id));
        id = QtopiaSql::instance()->databaseIdForPath(path);
        QCOMPARE(id, (uint)0);
        /* We should now get the system database when requesting
         * the detached database. */
        base = QtopiaSql::instance()->database(id);
        QCOMPARE(base, baseSystem);
    }
}

/*?
    Data for attach_detach test.
*/
void tst_QtopiaSql::attach_detach_data()
{
    QTest::addColumn<QString>("path");

    QTest::newRow("simple in temp") << Qtopia::tempDir();
    
    QTest::newRow("simple in testDataPath") << testDataPath;
}

/*?
    Perform complex tests with SQL queries to ensure database
    is working correctly.

    This test:
        * Retrieves a database with a specific ID.
        * Runs a given query on that database.
        * Compares the error (if any) caused by the query against
          an expected value.
        * Compares the results (if any) of the query against
          expected values.

    This test is intended to be run many times with consecutive
    SQL queries which establish and test database state.
*/
void tst_QtopiaSql::sql()
{
    QFETCH(QString, query);
    QFETCH(int, dbId);
    QFETCH(bool, inTransaction);
    QFETCH(bool, method);
    QFETCH(QList< QList< QVariant> >, values);
    QFETCH(QSqlError, error);

    if (method) {
        /* Get the database, execute query with QString, check error. 
         * Using this type of execution, we can't check query results. */
        QSqlDatabase base = QtopiaSql::instance()->database(dbId);
        QSqlError retError = QtopiaSql::instance()->exec(query, base, inTransaction);
        QCOMPARE(retError, error);        
    } else {
        /* Get the database and prepare query. */
        QSqlDatabase base = QtopiaSql::instance()->database(dbId);
        QSqlQuery x(base);
        x.prepare(query);

        /* Execute QSqlQuery, check error. */
        QSqlError retError = QtopiaSql::instance()->exec(x, base, inTransaction);
        QCOMPARE(retError, error);

        /* If this was a SELECT query... */
        if (x.isSelect()) {
            int i = 0;
            /* Compare all results against expected results */
            while (x.next()) {
                if (i+1 > values.count())
                    QFAIL(QString("SELECT returned %1 or more rows, expected %2\n")
                        .arg(i+1).arg(values.count()).toLatin1());
                for (int j = 0; j < values[i].size(); ++j) {
                    QCOMPARE(x.value(j), values[i][j]);
                }
                ++i;
            } // while (x.next())
            QCOMPARE(i, values.count());
        } // if (x.isSelect())
    } // else
}

/*?
    Data for sql test function.

    The queries here are run in order and each test depends on
    the state created in previous tests.

    It is important to note that this test needs only to test the correct
    behaviour of QtopiaSql, not the underlying database or database driver.
    Hence all major types of SQL queries should be used at least once,
    but it is not necessary to do exhaustive tests on the integrity of
    the database.
*/
void tst_QtopiaSql::sql_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("dbId");
    QTest::addColumn<bool>("inTransaction");
    QTest::addColumn<bool>("method");
    QTest::addColumn< QList< QList<QVariant> > >("values");
    QTest::addColumn<QSqlError>("error");

    QtopiaSql::instance()->openDatabase();
    QtopiaSql::instance()->attachDB(testDataPath);
    int id = QtopiaSql::instance()->databaseIdForPath(testDataPath);
    QtopiaSql::instance()->database(id);

    // Do tests for database 0 (system) and created database
    for (int dbId = 0; dbId < id+1; dbId += id) {

    /* Do tests using both versions of QtopiaSql::exec().
     * method = true means to use the QString version,
     * method = false means to use the QSqlQuery version.
     */
    for (int meth_i = 0; meth_i < 2; ++meth_i) {
    bool meth = (meth_i == 0);

    /* Due to the way QtopiaSql and probably the sqlite driver are written, there are restrictions
     * on when inTransaction can safely be true.  It's safe to use it for:
     *    - valid SELECT queries which return no data
     *    - valid CREATE TABLE queries
     *    - valid INSERT queries
     *    - valid DELETE queries
     * It's unsafe to use it for:
     *    - valid SELECT queries which return data
     *    - any invalid query
     */

    QList< QList<QVariant> > list;

    QTest::newRow(QString("completely invalid SQL, d%1 m%2").arg(dbId).arg(meth).toLatin1()) <<
        "Why, my dear fellow, I do believe that this is not a valid SQL statement."
        << dbId
        << false
        << meth
        << list
        << QSqlError("Unable to fetch row", "near \"Why\": syntax error",
                     QSqlError::ConnectionError, 21);

    for (int i = 0; i < 7; ++i) {
        QString table = QString("%1%2").arg(testTable).arg(i);
        QString dmi = QString(" d%1 m%2 i%3").arg(dbId).arg(meth).arg(i);
        /* TABLE STATE: not created */

        QTest::newRow(("select on non-existent table" + dmi).toLatin1()) <<
            QString(
            "SELECT * FROM %1;"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError("Unable to fetch row", QString("no such table: %1").arg(table),
                        QSqlError::ConnectionError, 21);

        /* TABLE STATE: not created */

        QTest::newRow(("insert on non-existent table" + dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, name, fav_colour) "
            "VALUES(0, 'billy', 'red');"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError("Unable to fetch row", QString("no such table: %1").arg(table),
                        QSqlError::ConnectionError, 21);
    
        /* TABLE STATE: not created */

        QTest::newRow(("create new table"+dmi).toLatin1()) <<
            QString(
            "CREATE TABLE %1 ("
            "id           INTEGER     PRIMARY KEY, "
            "name         VARCHAR     NOT NULL, "
            "fav_colour   CHAR(30)    NULL"
            ");"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |
         */

        QTest::newRow(("select empty 1"+dmi).toLatin1()) <<
            QString(
            "SELECT * FROM %1;"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |
         */

        QTest::newRow(("insert with non-existent column"+dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, name, doesnt_exist) "
            "VALUES(0, 'billy', 'red');"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError("Unable to fetch row", QString("table %1 has no column named doesnt_exist").arg(table),
                        QSqlError::ConnectionError, 21);

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |
         */

        QTest::newRow(("insert with missing value"+dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, name, fav_colour) "
            "VALUES(0, 'billy');"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError("Unable to fetch row", "2 values for 3 columns",
                        QSqlError::ConnectionError, 21);

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |
         */

        QTest::newRow(("insert with prohibited NULL value"+dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, fav_colour) "
            "VALUES(0, 'blue');"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError("Unable to fetch row", "SQL logic error or missing database",
                        QSqlError::ConnectionError, 1);

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |
         */

        QTest::newRow(("simple insert 1"+dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, name, fav_colour) "
            "VALUES(0, 'billy', 'red');"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *                   |                   |
         */

        QTest::newRow(("simple insert 2"+dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, name, fav_colour) "
            "VALUES(1, 'frank', 'green');"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *         1         |     frank         |          green
         *                   |                   |               
         */

        list << (QList<QVariant>() << 0 << "billy" << "red");
        list << (QList<QVariant>() << 1 << "frank" << "green");
        QTest::newRow(("select non-empty 1"+dmi).toLatin1()) <<
            QString(
            "SELECT * FROM %1;"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *         1         |     frank         |          green
         *                   |                   |               
         */

        QTest::newRow(("simple insert 3"+dmi).toLatin1()) <<
            QString(
            "INSERT INTO %1 (id, name, fav_colour) "
            "VALUES(2, 'bob', 'green');"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *         1         |     frank         |          green
         *         2         |       bob         |          green
         *                   |                   |               
         */

        list << (QList<QVariant>() << 2 << "bob" << "green");
        QTest::newRow(("select non-empty 2"+dmi).toLatin1()) <<
            QString(
            "SELECT * FROM %1;"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *         1         |     frank         |          green
         *         2         |       bob         |          green
         *                   |                   |               
         */

        QTest::newRow(("delete 1"+dmi).toLatin1()) <<
            QString(
            "DELETE FROM %1 WHERE fav_colour='green';"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        list.removeAt(2);
        list.removeAt(1);

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *                   |                   |               
         */

        QTest::newRow(("select non-empty 3"+dmi).toLatin1()) <<
            QString(
            "SELECT * FROM %1;"
            ).arg(table)
            << dbId
            << false
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *         0         |     billy         |            red
         *                   |                   |               
         */

        QTest::newRow(("delete all"+dmi).toLatin1()) <<
            QString(
            "DELETE FROM %1;"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        list.clear();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |               
         */

        QTest::newRow(("select empty 2"+dmi).toLatin1()) <<
            QString(
            "SELECT * FROM %1;"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE:
         *        id         |      name         |     fav_colour
         *                   |                   |               
         */

        QTest::newRow(("drop table"+dmi).toLatin1()) <<
            QString(
            "DROP TABLE %1;"
            ).arg(table)
            << dbId
            << true
            << meth
            << list
            << QSqlError();

        /* TABLE STATE: destroyed */
    } // for (int i...)
    } // for (int dbId...)
    } // for (int meth_i...)
}
