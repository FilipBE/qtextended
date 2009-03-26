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
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QDir>
#include <QTextCodec>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QtopiaIpcEnvelope>
#include "qcontent_p.h"
#if !defined(QTOPIA_CONTENT_INSTALLER) && !defined(QTOPIA_TEST)
#include <QDSServiceInfo>
#include <QDSAction>
#include <qtopiasqlmigrateplugin_p.h>
#include <QPluginManager>
#endif
#include <unistd.h>
#include <QTextCodec>
#include <string.h>

// Set this to control number of backups files rotated on upgrading
// database.  If set to 0 no backups are made - thus import of old data is
// not possible.
#define QTOPIA_SQL_MAX_BACKUPS 3

#define Q_USE_SQLITE
// #define Q_USE_MIMER
// #define Q_USE_MYSQL

#define USE_LOCALE_AWARE

#ifndef QTOPIA_HOST
#if defined(Q_WS_QWS)
#include <qwsdisplay_qws.h>
#elif defined(Q_WS_X11)
#include <qx11info_x11.h>
#endif
#endif

/*!
  \class QtopiaSql
    \inpublicgroup QtBaseModule

  \brief The QtopiaSql class provides a collection of methods for setting up and managing database connections.

  Qt Extended has a database architecture which uses a number of different databases, including
  a seperate database for each removable storage media.  Additionally the database
  schema required for a particular version of Qt Extended may differ from that currently on the
  device due to Qt Extended being upgraded.

  Use these routines to manage connections to these databases.  The QtopiaDatabaseId type
  identifies a particular database of the collectin.

  The systemDatabase() method is used to fetch a QSqlDatabase object pointing to the Qt Extended main system database.  Call attachDB() to connect a database for a particular path, typically
  associated with a new storage media mount-point.  These will be operated on with the
  system database as a unit.

  Lists of the current databases can be obtained with databaseIds() and databases().  Given
  a database id obtain the path for the associated media mount point with databasePathForId()
  and obtain the QSqlDatabase itself with database().

  The ensureTableExists() methods call through to the Database migration utility to ensure
  the table schema referred to is loaded.

  The attachDB() methods use the underlying database implementation specific method for
  creating queries across seperate databases.  In SQLITE this method is the ATTACH statement,
  but an equivalent method is used for other implementations.

  This documentation should be read and understood in conjunction with the \l {Database Policy}
  and the \l {Database Specification}
*/

/*!
  Opens the main system database, if it is not already open.
*/
void QtopiaSql::openDatabase()
{
    if (!d()->defaultConn) {

        d()->preSetupDefaultConnection();

        d()->defaultConn = connectDatabase(QLatin1String(QSqlDatabase::defaultConnection));
        init(*d()->defaultConn);
        d()->masterAttachedConns[0] = *d()->defaultConn;
        d()->dbs.insert(0, *d()->defaultConn);

#ifndef QTOPIA_CONTENT_INSTALLER
        if (QApplication::type() == QApplication::GuiServer) {
            QPluginManager manager(QLatin1String("qtopiasqlmigrate"));

            if (QtopiaSqlMigratePlugin *plugin = qobject_cast<QtopiaSqlMigratePlugin *>(
                manager.instance(QLatin1String("dbmigrate")))) {
                plugin->migrate(d()->defaultConn, true);

                d()->disksChanged(plugin);
            }
        } else {
#else
        {
#endif
            d()->disksChanged();
        }
        d()->postSetupDefaultConnection();
    }
}

/*!
  Closes the main system database, if it is not already closed.
*/
void QtopiaSql::closeDatabase()
{
    delete d()->defaultConn;
    d()->defaultConn = 0;
}

/*!
  \fn QtopiaSql::stringCompare( const QString &l, const QString &r )

  Perform a locale aware string compare of the two argument QStrings \a l and \a r
  using the database implementations native collating functions.

  If the database does not provide one, or if one is provided but not integrated,
  then returns the same result as QString::localeAwareCompare()
*/

#ifdef Q_USE_SQLITE
#ifdef USE_LOCALE_AWARE
// QString is ushort/utf16 internally.
// so save 2*malloc+memcpy if use
// textcodec->fromUnicode direct, along with strcoll.
int sqliteLocaleAwareCompare(void *, int ll, const void *l, int rl, const void *r)
{
    QTextCodec *codec = QTextCodec::codecForLocale();

    if( codec )
    {
        return strcoll(
                codec->fromUnicode( static_cast< const QChar * >( l ), ll / 2 ).constData(),
                codec->fromUnicode( static_cast< const QChar * >( r ), rl / 2 ).constData() );
    }
    else
    {   // Fall back on a regular comparison if there is no local codec.
        QString left = QString::fromUtf16((const ushort *)l, ll/2);
        QString right = QString::fromUtf16((const ushort *)r, rl/2);

        return left.compare( right );
    }
}

int QtopiaSql::stringCompare(const QString &l, const QString &r)
{
    return QString::localeAwareCompare(l, r);
}
#else

/* copied straight from the sqlite3 source code */
const unsigned char sqlite3UpperToLower[] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
     18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
     36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
     54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99,100,101,102,103,
    104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,
    122, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
    126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
    162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,
    198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,
    234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
    252,253,254,255
};
#define UpperToLower sqlite3UpperToLower

/*
** Some systems have stricmp().  Others have strcasecmp().  Because
** there is no consistency, we will define our own.
*/
int sqlite3StrICmp(const char *zLeft, const char *zRight){
  register unsigned char *a, *b;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return UpperToLower[*a] - UpperToLower[*b];
}
int sqlite3StrNICmp(const char *zLeft, const char *zRight, int N){
  register unsigned char *a, *b;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/* sqlite default text comparision operation, memcmp
    http://www.sqlite.org/datatype3.html
 */
int QtopiaSql::stringCompare(const QString &l, const QString &r)
{
    return sqlite3StrICmp(l.toUtf8().constData(), r.toUtf8().constData());
}
#endif
#endif
#ifdef Q_USE_MIMER
int QtopiaSql::stringCompare(const QString &l, const QString &r)
{
    // TODO MIMER
    // fix to use Mimer collation library
    return l < r;
}
#endif

QSqlDatabase *QtopiaSql::connectDatabase(const QString &connName)
{
    qLog(Sql) << "Trying to connect database with connection name:" << connName;
    loadConfig();

    QSqlDatabase *db = new QSqlDatabase(QSqlDatabase::addDatabase(d()->type, connName));
    db->setDatabaseName(d()->name);
    db->setUserName(d()->user);
    db->setHostName(d()->hostname);
    db->setPassword(d()->password);
    if (!db->open())
    {
        qWarning() << "Could not connect" << d()->name << db->lastError().text();
        delete db;
        return NULL;
    }

#if defined(Q_USE_SQLITE)
    // Execute these if for no other reason than to flush the SQLite schema
    QSqlQuery xsql( *db );
    xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
    xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
#endif

    return db;
}

void QtopiaSql::init(QSqlDatabase &db, bool force)
{
    static bool doneInit = false;
    if (!doneInit || force) {
        d()->installSorting(db);
        doneInit = true;
    }
}

/*!
  Initializes the state of the QtopiaSql system with the \a type of database, eg "sqlite".

  The meaning of the \a name and \a user parameters are as defined in QSqlDatabase.
*/
void QtopiaSql::loadConfig(const QString &type, const QString &name, const QString &user)
{
    d()->type = type;
    d()->name = name;
    d()->user = user;
    closeDatabase();
    openDatabase();
}

void QtopiaSql::loadConfig()
{
#ifdef Q_USE_SQLITE
    if (d()->type.isEmpty())
    {
        QString dir = Qtopia::homePath() + QLatin1String( "/Applications" );
        if( !QFile::exists( dir ) && !QDir().mkdir( dir ) )
            qFatal( "Cannot write to %s", dir.toLatin1().constData() );

        dir += QLatin1String( "/Qtopia" );
        if( !QFile::exists( dir ) && !QDir().mkdir( dir ) )
            qFatal( "Cannot write to %s", dir.toLatin1().constData() );

        d()->type = QLatin1String("QSQLITE");
        d()->name = dir + QLatin1String( "/qtopia_db.sqlite" );
    }
#endif
#ifdef Q_USE_MIMER
    d()->type = QLatin1String("QODBC");
    d()->name = QLatin1String("qtopia");
    d()->user = QLatin1String("SYSADM");
    d()->password = QLatin1String("lfer64"); // ok, not so good
    d()->hostname = QLatin1String("10.1.1.22");
    //d()->port = 1360;
#endif
#ifdef Q_USE_MYSQL
    d()->type = QLatin1String("QMYSQL");
    d()->name = QLatin1String("qtopia");
    d()->user = QLatin1String("root");
#endif
}

void QtopiaSql::saveConfig()
{
}

/*!
  Execute the given QString \a query on the database \a db.  If \a inTransaction is true,
  then perform the query inside an SQL transaction, for atomicity, if the database
  backend supports this.
*/
QSqlError QtopiaSql::exec(const QString &query, QSqlDatabase& db, bool inTransaction)
{
    QSqlError result = QSqlError();
    QSqlQuery qry(db);
    qry.prepare(query);
    if (inTransaction && db.driver()->hasFeature(QSqlDriver::Transactions))
        if(!db.driver()->beginTransaction())
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing beginTransaction:" << db.lastError();
    QtopiaSql::logQuery( qry );
    if(qry.exec() == false)
    {
#ifdef Q_USE_SQLITE
        int loopcount = 0;
        while (qry.lastError().number() == 5 && loopcount < 5)
        {
            Qtopia::usleep(5000);
            qry.exec(query);
            loopcount++;
        }
#endif
        result = qry.lastError();
    }
    else
        result = QSqlError();
    if (inTransaction && db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        if (result.type() == QSqlError::NoError)
        {
            if(!db.driver()->commitTransaction())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing commitTransaction:" << result;
            }
        }
        else
        {
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing query (" << query << ") error:" << result << ", rolling back";
            if(!db.driver()->rollbackTransaction())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing rollbackTransaction:" << result;
            }
        }
    }
    return result;
}

/*!
  Execute the given QSqlQuery \a query on the database \a db.  If \a inTransaction is true,
  then perform the query inside an SQL transaction, for atomicity, if the database
  backend supports this.
*/
QSqlError QtopiaSql::exec(QSqlQuery &query, QSqlDatabase& db, bool inTransaction)
{
    QSqlError result = QSqlError();
    if (inTransaction && query.driver()->hasFeature(QSqlDriver::Transactions))
        if(!db.transaction())
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing beginTransaction:" << db.lastError();
    QtopiaSql::logQuery( query );
    if(query.exec() == false)
    {
#ifdef Q_USE_SQLITE
        int loopcount = 0;
        while (query.lastError().number() == 5 && loopcount < 5)
        {
            Qtopia::usleep(5000);
            query.exec();
            loopcount++;
        }
#endif
        result = query.lastError();
    }
    else
        result = QSqlError();
    if (inTransaction && query.driver()->hasFeature(QSqlDriver::Transactions))
    {
        if (result.type() == QSqlError::NoError)
        {
            if(!db.commit())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing commitTransaction:" << result;
            }
        }
        else
        {
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing query (" << query.lastQuery() << ") error:" << result << ", rolling back";
            if(!db.rollback())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing rollbackTransaction:" << result;
            }
        }
    }
    return result;
}


/*!
  Returns a reference to the qtopia database object (used for
  settings storage, doc manager, etc) for the given QtopiaDatabaseId \a id.  There is a separate set of database objects
  created per thread.
  */
QSqlDatabase &QtopiaSql::database(const QtopiaDatabaseId& id)
{
    if( id == quint32(-1) )
    {
        qWarning() << "Database requested for invalid handle";
        return d()->nullDatabase;
    }

    QMutexLocker guard(&d()->guardMutex);
    if (id == 0 && !d()->masterAttachedConns.contains(0))
        QtopiaSql::openDatabase();
    else if (!isValidDatabaseId(id) && d()->dbs.contains(id))
    {
        // remove the connection, it's stale.
        qWarning() << "Stale database handle, returning the system database instead";
        d()->dbs.remove(id);
        return d()->nullDatabase;
    }
    else if(!isValidDatabaseId(id))
    {
        qWarning() << "Database handle doesnt exist in the master list, returning an invalid database";
        if(id == 0)
            qFatal("Should not be requesting the system database from this location in the code");
        return d()->nullDatabase;
    }
    else if (!d()->dbs.contains(id))
    {
        QString connName = QString("QtopiaSqlDB%1").arg(d()->conId++);
        // copy the masterAttachedCons version, and register the connname in the list
        QSqlDatabase &from=d()->masterAttachedConns[id];
        QSqlDatabase db=QSqlDatabase::addDatabase(from.driverName(), connName);
        db.setDatabaseName(from.databaseName());
        db.setUserName(from.userName());
        db.setPassword(from.password());
        db.setHostName(from.hostName());
        db.setPort(from.port());
        db.setConnectOptions(from.connectOptions());
        if (db.open() )
        {
            d()->installSorting(db);
            QSqlQuery xsql( db );
#if defined(Q_USE_SQLITE)
            xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
            xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
#endif
            d()->dbs.insert(id, db);
            d()->connectionNames.insert(id, connName);

            qLog(Sql) << "Connected database" << db.databaseName() << "for database id" << id << "with connection name" << connName;
        }
        else
        {
            qWarning() << "Failed to open database" << db.databaseName() << "for database id" << id << "with connection name" << connName;
            qWarning() << db.lastError().text();

            return d()->nullDatabase;
        }
    }

    return d()->dbs[id];
}

/*!
    \deprecated

    Returns the given \a input with all single quote characters repalced with two consecutive quote characters.
*/
QString QtopiaSql::escapeString(const QString &input)
{
    return escape(input);
}

/*!
    Returns the given \a input with all single quote characters repalced with two consecutive quote characters.
*/
QString QtopiaSql::escape(const QString &input)
{
    // add more changes as needed.
    static QLatin1String singleQuote("'"), doubleQuote("''");
    QString result=input;
    result.replace(singleQuote, doubleQuote);
    return result;
}

/*!
  Attach the database for the media located at the mount-point \a path.
  The path given by \a dbPath is the database to use in the attach.

  \sa exec()
*/
void QtopiaSql::attachDB(const QString& path, const QString& dbPath)
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if (QApplication::type() == QApplication::GuiServer) {
        QPluginManager manager(QLatin1String("qtopiasqlmigrate"));

        if (QtopiaSqlMigratePlugin *plugin = qobject_cast<QtopiaSqlMigratePlugin *>(
            manager.instance(QLatin1String("dbmigrate")))) {
            attachDB(path, dbPath, plugin);
        }
    } else {
#else
    {
#endif
        attachDB(path, dbPath, 0);
    }
}

void QtopiaSql::attachDB(const QString& path, const QString& dbPath, QtopiaSqlMigratePlugin *plugin)
{
#ifdef QTOPIA_CONTENT_INSTALLER
    Q_UNUSED(plugin);
#endif

    qLog(Sql) << "Attaching database for path" << path << "with the db located at" << dbPath;
    QMutexLocker guard(&d()->guardMutex);
    // add database to default connections list.
    if(path.isEmpty() || dbPath.isEmpty() || d()->defaultConn == NULL || dbPath == d()->defaultConn->databaseName())
        return;
    QtopiaDatabaseId dbid = databaseIdForDatabasePath(dbPath);
    if(dbid != quint32(-1) && !isValidDatabaseId(dbid))
    {
        QString connName = QString("QtopiaSqlDB%1").arg(d()->conId++);
        // copy the masterAttachedCons version, and register the connname in the list
        QSqlDatabase db=QSqlDatabase::addDatabase(d()->type, connName);
        d()->dbs.insert(dbid, db);
        db.setDatabaseName(dbPath);
        db.setUserName(d()->user);
        db.setPassword(d()->password);
        db.setHostName(d()->hostname);
        if(!db.open())
        {
            qLog(Sql) << "!! Failed to open database:" << dbPath << db.lastError();
            return;
        }

#ifndef QTOPIA_CONTENT_INSTALLER
        if (plugin && !plugin->migrate(&db)) {
            db.close();

            return;
        }
#endif
        d()->installSorting(db);
        QSqlQuery xsql( db );
        xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
        xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
        // Once temp_store is "memory" the temp_store_directory has no effect
        // xsql.exec(QLatin1String("PRAGMA temp_store_directory = '/tmp';"));
        d()->connectionNames.insert(dbid, connName);
        d()->dbPaths.insert(dbid, path);
        d()->masterAttachedConns.insert(dbid, db);
        QtopiaSql::init(db, true);
        if ( qApp->type() == QApplication::GuiServer )
            QtopiaIpcEnvelope se("QPE/System", "resetContent()");
#if !defined(QTOPIA_CONTENT_INSTALLER) && !defined(QTOPIA_TEST)
        QContentUpdateManager::instance()->requestRefresh();
#endif
    }
    else
        qLog(Sql) << "tried to attach an already attached database:" << dbPath;
}

/*!
  Attach the database for the media located at the mount-point \a path.  The
  database to use is located on that media.

  After this call database entries for the attached database will be used in
  future queries executed.

  \sa exec()
*/
void QtopiaSql::attachDB(const QString& path)
{
    QtopiaSql::attachDB(path, d()->databaseFile(path));
}

/*!
  Undo the effect of the attachDB() method, for the database associated with
  mount-point \a path.

  After this call database queries executed will not attempt to reference this
  database.

  \sa attachDB(), exec()
*/
void QtopiaSql::detachDB(const QString& path)
{
    qLog(Sql) << "Detaching database at" << path;
    QMutexLocker guard(&d()->guardMutex);
    QtopiaDatabaseId dbid = d()->dbPaths.key( path, 0 );
    if(dbid != 0)
    {
        QString dbPath=d()->masterAttachedConns[dbid].databaseName();
        d()->masterAttachedConns.remove(dbid);
        d()->dbPaths.remove(dbid);

        if(d()->dbs.contains(dbid))
            d()->dbs.take(dbid).close();

        if(d()->connectionNames.contains(dbid))
            QSqlDatabase::removeDatabase(d()->connectionNames.take(dbid));
        // todo: if database itself is located in the temp directory, then delete it.
        if(dbPath.startsWith(Qtopia::tempDir())) {
            QFile::remove(dbPath);
            sync();
        }

        if ( qApp->type() == QApplication::GuiServer )
            QtopiaIpcEnvelope se("QPE/System", "resetContent()");
#if !defined(QTOPIA_CONTENT_INSTALLER) && !defined(QTOPIA_TEST)
        QContentUpdateManager::instance()->requestRefresh();
#endif
    }
    else
        qLog(Sql) << "tried to detach an invalid database path mapping:" << path;
}

/*!
  Returns a list QSqlDatabase objects comprising all attached databases.
*/
const QList<QSqlDatabase> QtopiaSql::databases()
{
    openDatabase();

    QList<QSqlDatabase> result;
    database(0);    // to force at least one entry to be loaded (0 being the system database of course).
    foreach(QtopiaDatabaseId id, QtopiaSql::databaseIds())
        result.append(database(id));
    return result;
}

/*!
  Return the QtopiaDatabaseId identifier for the database associated with the
  given \a path.

  Internally causes the database system to be opened, if not already open.
*/
QtopiaDatabaseId QtopiaSql::databaseIdForPath(const QString& path)
{
    openDatabase();

#ifndef QTOPIA_CONTENT_INSTALLER
    QFileSystem *bestMatch = 0;
    int bestLen = 0;

    foreach ( QFileSystem *fs, QStorageMetaInfo::instance()->fileSystems( NULL, false) ) {
        if ( fs->contentDatabase() ) {
            QString fsPath = !fs->path().isEmpty() ? fs->path() : fs->prevPath();
            if ( fsPath.length() > bestLen && path.startsWith( fsPath ) ) {
                int currLen = fsPath.length();
                if ( currLen == 1 )
                    currLen = 0; // Fix checking '/' root mount which is a special case
                if ( path.length() == currLen ||
                    path[currLen] == '/' ||
                    path[currLen] == '\\') {
                    bestMatch = fs;
                    bestLen = fsPath.length();
                }
            }
        }
    }

    if ( bestMatch ) {
        return d()->dbPaths.key( bestMatch->isConnected() ? bestMatch->path() : bestMatch->prevPath(), QtopiaDatabaseId(-1) );
    } else {
        return 0;
    }
#else
    Q_UNUSED(path)
    return 0;
#endif
}

/*!
  Return the QtopiaDatabaseId identifier for the database located at the
  given \a dbPath.

  Internally causes the database system to be opened, if not already open.
*/
QtopiaDatabaseId QtopiaSql::databaseIdForDatabasePath(const QString& dbPath)
{
    openDatabase();
    if(database(0).databaseName() == dbPath)
        return 0;
    else
        return qHash(dbPath);
}

/*!
  Return a list of the QtopiaDatabaseId entries for all the attached
  databases.

  Internally causes the database system to be opened, if not already open.
*/
const QList<QtopiaDatabaseId> QtopiaSql::databaseIds()
{
    openDatabase();

    return d()->masterAttachedConns.keys().toSet().toList();
}

/*!
  Return the QSqlDatabase instance of the main system database.
*/
QSqlDatabase &QtopiaSql::systemDatabase()
{
    return database(0);
}

/*!
  Return true if the file located at the given \a path is a Qt Extended database file,
  for example the datastore used by the SQLITE engine; return false otherwise.

  This may be used to test if the file should be shown to the end-user.
  \code
    if ( !QtopiaSql::isDatabase( fileFound ))
        listForDisplay.append( fileFound );
  \endcode
*/
bool QtopiaSql::isDatabase(const QString &path)
{
    openDatabase();

    qLog(Sql) << "QtopiaSql::isDatabase("<< path << ")";
    foreach(const QSqlDatabase &db, d()->masterAttachedConns)
    {
        qLog(Sql) << "db.databaseName:" << db.databaseName();
        if(path == db.databaseName() || path == db.databaseName() + QLatin1String("-journal"))
           return true;
    }
    return false;
}

/*!
  Return the full path to the mount-point related to the database
  for the given QtopiaDatabaseId \a id
*/
QString QtopiaSql::databasePathForId(const QtopiaDatabaseId& id)
{
    openDatabase();

    QString result;
    if(id != 0 && id != quint32(-1) && d()->dbPaths.contains(id))
        result = d()->dbPaths.value(id);
    return result;
}

/*!
  Send a string for the given QSqlQuery \a q to the qtopia logging
  system.
*/
void QtopiaSql::logQuery(const QSqlQuery &q)
{
    if (qLogEnabled(Sql))
    {
        qLog(Sql) << "executing:" << q.lastQuery();
        if (!q.boundValues().empty())
            qLog(Sql) << "   params:" << q.boundValues();
    }
}
/*!
  Return a QSqlDatabase object for the database specific to the application
  given by \a appname
  Note: You are responsible for calling QSqlDatabase::removeDatabase once you are finished
  with the connection.
*/
QSqlDatabase QtopiaSql::applicationSpecificDatabase(const QString &appname)
{
    QSqlDatabase db;
#ifdef Q_USE_SQLITE
    openDatabase();
    QString connName = QString("QtopiaSqlDB%1").arg(d()->conId++);
    // copy the masterAttachedCons version, and register the connname in the list
    db=QSqlDatabase::addDatabase(d()->type, connName);
    db.setDatabaseName(Qtopia::applicationFileName(appname, QLatin1String("qtopia_db.sqlite")));
    db.setUserName(d()->user);
    db.setPassword(d()->password);
    db.setHostName(d()->hostname);
    db.open();
    d()->installSorting(db);
    QSqlQuery xsql( db );
    xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
    xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
#endif

    return db;
}

/*!
  Ensure that the given table \a tableName exists in the database \a db

  Return true if the operation was successful; otherwise returns false
*/
bool QtopiaSql::ensureTableExists(const QString &tableName, QSqlDatabase &db )
{
#if !defined(QTOPIA_CONTENT_INSTALLER) && !defined(QTOPIA_TEST)
    if (db.tables().contains(tableName))
        return true;

    QFile data(QLatin1String(":/QtopiaSql/") + db.driverName() + QLatin1String("/") + tableName);

    if (!data.open(QIODevice::ReadOnly)) {
        qLog(Sql) << "QtopiaSql::ensureTableExists: resource" << tableName << "not found";
        return false;
    }

    QTextStream ts(&data);
    // read assuming utf8 encoding.
    ts.setCodec(QTextCodec::codecForName("utf8"));
    ts.setAutoDetectUnicode(true);

    QString qry;
    while (!ts.atEnd()) {
        /*
        Simplistic parsing.
        no comments in middle of line
        no starting another sql statment on same line one ends.

        For now, shouldn't be a problem.
        */

        QString line = ts.readLine();
        // comment, remove.
        if (line.contains(QLatin1String("--")))
            line.truncate(line.indexOf (QLatin1String("--")));
        if (line.trimmed().length () == 0)
            continue;
        qry += line;

        if (line.contains(QLatin1Char(';'))) {
            if (!QSqlQuery(db).exec(qry))
                return false;

            qry = QString();
        } else { // no ;, query spans to next line, keep going.
            qry += QLatin1Char(' ');
        }
    }

    return qry.isEmpty() || QSqlQuery(db).exec(qry);
#else
    Q_UNUSED(tableName);
    Q_UNUSED(db);
    return false;
#endif
}

/*!
  Ensure that all tables in the list of \a tableNames exists in the database \a db.

  Internally this invokes the "DBMigrationEngine" service to run any migrations
  or schema updates required.
*/
bool QtopiaSql::ensureTableExists(const QStringList &tableNames, QSqlDatabase&db )
{
    foreach (const QString table, tableNames)
        if (ensureTableExists(table, db))
            return false;

    return true;
}

/*!
  Returns true if the database \a id is a valid open database in the system.
*/
bool QtopiaSql::isValidDatabaseId(const QtopiaDatabaseId& id)
{
    return d()->masterAttachedConns.contains(id);
}
// Created as a function-local static to delete a QGlobalStatic<T>
template <typename T>
class QGlobalInstanceDeleter
{
public:
    T *globalStatic;
    QGlobalInstanceDeleter(T *_globalStatic)
        : globalStatic(_globalStatic)
    { }

    inline ~QGlobalInstanceDeleter()
    {
        delete globalStatic;
        globalStatic = 0;
    }
};

#define Q_INSTANCE_HELPER_WITH_ARGS(CLASSNAME, ARGS) \
    { \
        static QAtomicPointer<CLASSNAME> this_##CLASSNAME = NULL; \
        CLASSNAME *x = new CLASSNAME ARGS; \
        if(!this_##CLASSNAME.testAndSetOrdered(NULL, x)) \
            delete x; \
        else \
            static QGlobalInstanceDeleter<CLASSNAME> cleanup(this_##CLASSNAME); \
        return this_##CLASSNAME; \
    }

#define Q_INSTANCE_HELPER(CLASSNAME) \
    Q_INSTANCE_HELPER_WITH_ARGS(CLASSNAME, ())

/*!
  Returns a pointer to the singleton object of QtopiaSql. This is the preferred method of accessing the QtopiaSql class.
*/
QtopiaSql *QtopiaSql::instance()
{
    Q_INSTANCE_HELPER(QtopiaSql);
}

/*!
  Do not use this constructor to create a qtopiasql object. Please use \l instance() instead.
*/
QtopiaSql::QtopiaSql()
{
}

QtopiaSqlPrivate *QtopiaSql::d()
{
    return QtopiaSqlPrivate::instance();
}

void QtopiaSql::connectDiskChannel()
{
#ifndef QTOPIA_HOST
    d()->connectDiskChannel();
#endif
}
