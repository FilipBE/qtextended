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

#include "qpreparedquery_p.h"
#include <sqlite3.h>
#include <qpimsqlio_p.h> // for db handle
#include <QSqlDriver>
#include <QDebug>

#include <qtopialog.h>


int QPreparedSqlQuery::errorCount() const
{
    return mErrorList.count();
}

QStringList QPreparedSqlQuery::errors() const
{
    return mErrorList;
}

void QPreparedSqlQuery::clearErrors()
{
    mErrorList.clear();
}

#ifdef SQLITE_DIRECT

/*!
    \class QPreparedSqlQuery
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \brief The QPreparedSqlQuery class provides additional features
    to that found for QSqlQuery.
    \internal

    QPreparedSqlQuery provides a near feature equivalent API to
    QSqlQuery with a few additions.

    Primarily it maps directly to SQLite, avoiding some of the layers
    of abstraction included with QSqlQuery, improving performance.  It
    also uses qLog easing coding so that code using QPreparedSqlQuery
    does not need to check for errors unless it would take a different
    code path.  The error values are still kept and returned properly.

    QPreparedSqlQuery also can be constructed before a database is opened,
    so long as the database is opened before the query is prepared.  This
    allows the class to be member variables of PIM classes which in
    some cases open the database after they have been constructed.
    If no database is provided in the constructor to QPreparedSqlQuery
    the database as provided by QPimSQLIO::database() will be used.

    \sa QSqlQuery
*/

/*!
    Constructs a new QPreparedSqlQuery
*/
QPreparedSqlQuery::QPreparedSqlQuery()
    : mHandle(0), mDBHandle(0), skip_step(false)
{
}

/*!
    Constructs a new QPrepraredSqlQuery that is connected to the
    given \a database.  It is required that the database uses the
    SQLite driver.
*/
QPreparedSqlQuery::QPreparedSqlQuery(QSqlDatabase database)
    : mHandle(0), mDBHandle(*static_cast<sqlite3**>(database.driver()->handle().data())), skip_step(false)
{
}


/*!
    Constructs a QPreparedSqlQuery object using the SQL \a query.
    Unlike the constructor for QSqlQuery, it will not prepare nor
    execute the query until explicitly made to do so using the
    prepare() and exec() functions.
*/
QPreparedSqlQuery::QPreparedSqlQuery(const QString &query)
    : mText(query), mHandle(0), mDBHandle(0), skip_step(false)
{
}

/*!
    Destorys the object and frees any allocated resources.
*/
QPreparedSqlQuery::~QPreparedSqlQuery()
{
    if (mHandle)
        clear();
}

/*!
    Returns true if the query is currently positioned on a valid
    record; otherwise returns false.
*/
bool QPreparedSqlQuery::isValid() const
{
    return (mHandle != 0);
}

/*!
    Prepares the previously specified query string.  Returns true if
    the query was successfully prepared.
*/
bool QPreparedSqlQuery::prepare()
{
    qLog(Sql) << "QPreparedSqlQuery::prepare()";
    if (mText.isEmpty()) {
        return false;
    }
    if (mHandle) {
        if (!sqlite3_expired(mHandle)) {
            reset();
            return true; // e.g. once prepared, always prepared until cleared.
        } else {
            // Hmm, our query expired
            qLog(Sql) << "QPreparedSqlQuery::prepare() - prepared query expired";
            sqlite3_finalize(mHandle);
            mHandle = 0;
        }
    }
    qLog(Sql) << "QPreparedSqlQuery::prepare() - parse statement:" << mText;

    if (!mDBHandle)
        mDBHandle = *static_cast<sqlite3**>(QPimSqlIO::database().driver()->handle().data());

    // could use this to 'parse for multiple statements'.
    // use v2 when it becomes available.
    switch(sqlite3_prepare(mDBHandle, mText.toUtf8().data(), -1, &mHandle, 0)) {
        case SQLITE_OK:
            qLog(Sql) << "prepare query succeeded";
            return true;
        default:
            mError.setDriverText(QString::fromUtf8(sqlite3_errmsg(mDBHandle)));
            qLog(Sql) << "prepare query error:" << mError.driverText();
            mErrorList.append(mError.driverText());
            return false;
    }
}

/*!
    Prepares the given SQL \a query for execution.  Returns true
    if the query is prepared successfully.

    The query may
    contain placeholders for binding values. Both Oracle style
    colon-name (e.g., \c{:surname}), and ODBC style (\c{?})
    placeholders are supported; but they cannot be mixed in the same
    query. See the \l{QSqlQuery examples}{Detailed Description} for examples.
*/
bool QPreparedSqlQuery::prepare(const QString &query)
{
    if (mHandle && query != mText)
        clear();
    mText = query;
    return prepare();
}
/*!
    Instruct the database driver that no more data will be fetched
    from this query until it is re-executed.  Calling this function
    will free resources such as locks or cursors, but doesn't clear
    bound values or the prepared statement.  For frequently called
    queries this can lead to very significant performance improvements.

    \sa QSqlQuery::finish()
*/
void QPreparedSqlQuery::reset()
{
    qLog(Sql) << "QPreparedSqlQuery::reset() -" << mText;
    if (mHandle) {
        int res = sqlite3_reset(mHandle);
        if (res != SQLITE_OK)
            qLog(Sql) << "Failed to reset query:" << mText << "-" << sqlite3_errmsg(mDBHandle);
    }
    skip_step = false;
}

/*!
    Clears the result set and releases any resources held by the
    query.  In addition to resources released with reset() will also
    release resources for bound values and the prepared statement.

    This function is called automatically when the object is destroyed.
*/
void QPreparedSqlQuery::clear()
{
    qLog(Sql) << "QPreparedSqlQuery::clear() - " << mText;
    int res;
    skip_step = false;
    if (mHandle) {
        res = sqlite3_finalize(mHandle);
        if (res != SQLITE_OK)
            qLog(Sql) << "Failed to finalize query:" << mText << "-" << sqlite3_errmsg(mDBHandle);
    }
    mHandle = 0;
}

/*!
    Set the placeholder \a placeholder to be bound to value \a val in
    the prepared statement. Note that the placeholder mark (e.g \c{:})
    must be included when specifying the placeholder name. If \a paramType
    is QSql::Out or QSql::InOut, the placeholder will be
    overwritten with data from the database after the exec() call.

    To bind a NULL value, use a null QVariant; for example, use
    \c {QVariant(QVariant::String)} if you are binding a string.

    \sa addBindValue(), prepare(), exec(), boundValue() boundValues()
*/
void QPreparedSqlQuery::bindValue( const QString & placeholder, const QVariant & value, QSql::ParamType )
{
    if (mHandle) {
        int pos = sqlite3_bind_parameter_index(mHandle, placeholder.toUtf8().constData());
        if (pos)
            bindValue(pos-1, value);
    }

}

/*!
    \overload

    Set the placeholder in position \a pos to be bound to value \a val
    in the prepared statement. Field numbering starts at 0. If \a paramType
    is QSql::Out or QSql::InOut, the placeholder will be
    overwritten with data from the database after the exec() call.
*/
void QPreparedSqlQuery::bindValue( int pos, const QVariant & value, QSql::ParamType )
{
    qLog(Sql) << "QPreparedSqlQuery::bindValue(" << pos << value << ") -" << mText;
    if (!mHandle)
        return;
    pos++;
    int res;
    const QByteArray *ba;
    const QString *str;
    int attempt = 2;
    while (attempt--) {
        if (value.isNull()) {
            res = sqlite3_bind_null(mHandle, pos);
        } else {
            switch (value.type()) {
                case QVariant::ByteArray:
                    ba = static_cast<const QByteArray*>(value.constData());
                    res = sqlite3_bind_blob(mHandle, pos, ba->constData(),
                            ba->size(), SQLITE_TRANSIENT);
                    break;
                case QVariant::Int:
                    res = sqlite3_bind_int(mHandle, pos, value.toInt());
                    break;
                case QVariant::Double:
                    res = sqlite3_bind_double(mHandle, pos, value.toDouble());
                    break;
                case QVariant::UInt:
                case QVariant::LongLong:
                    res = sqlite3_bind_int64(mHandle, pos, value.toLongLong());
                    break;
                case QVariant::String:
                    str = static_cast<const QString*>(value.constData());
                    res = sqlite3_bind_text16(mHandle, pos, str->utf16(),
                            (str->size()) * sizeof(QChar), SQLITE_TRANSIENT);
                default:
                    {
                        QString str = value.toString();
                        res = sqlite3_bind_text16(mHandle, pos, str.utf16(),
                                (str.size()) * sizeof(QChar), SQLITE_TRANSIENT);
                    }
                    break;
            }
        }
        /* assume lack of step before rebind... */
        if (res == SQLITE_MISUSE) {
            sqlite3_reset(mHandle);
        }
    }

    if (res != SQLITE_OK) {
        mError.setDriverText(
                QString("Failed to bind parameter at %1: %2").arg(sqlite3_errmsg(mDBHandle)).arg(pos));
        qLog(Sql) << "bind value error:" << mError.driverText();
    }
}

/*!
    Returns a map of the bound values.

    With named binding, the bound values can be examined in the
    following ways:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 14

    With positional binding, the code becomes:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 15

    \sa boundValue() bindValue() addBindValue()
*/
QMap<QString, QVariant> QPreparedSqlQuery::boundValues() const
{
    qLog(Sql) << "boundValues not supported in QPreparedSqlQuery";
    return QMap<QString, QVariant>(); // sqlite doesn't let you query bound values
}

/*!
    Executes a previously prepared SQL query. Returns true if the
    query executed successfully.
*/
bool QPreparedSqlQuery::exec()
{
    qLog(Sql) << "QPreparedSqlQuery::exec()";
    // SQLITE doesn't need an explicit exec, rather it will just exec
    // on the next call to step.  Qt uses exec as a chance to bind
    // parameters and check errors while doing so.  This isn't
    // done here for performance and simplicity reasons.
    
    // doesn't unbind values.
    int res = sqlite3_step(mHandle);
    switch(res) {
        case SQLITE_ROW:
        case SQLITE_DONE:
            skip_step = true;
            step_res = res;
            qLog(Sql) << "exec succeeded";
            break;
        default:
            mError.setDriverText( sqlite3_errmsg(mDBHandle) );
            qLog(Sql) << "exec error:" << res << mError.driverText();
            mErrorList.append(mError.driverText());
            return false;
    }
    return true;
}

/*!
    Returns true if the query is active and positioned on a valid
    record and the \a field is NULL; otherwise returns false. Note
    that for some drivers, isNull() will not return accurate
    information until after an attempt is made to retrieve data.

    \sa isActive(), isValid(), value()
*/
bool QPreparedSqlQuery::isNull( int field ) const
{
    return sqlite3_column_type(mHandle, field) == SQLITE_NULL;
}

/*!
    Returns error information about the last error (if any) that
    occurred with this query.

    \sa QSqlError, QSqlDatabase::lastError()
*/
QSqlError QPreparedSqlQuery::lastError() const
{
    return mError;
}

/*!
    Returns the text of the current query being used, or an empty
    string if there is no current query text.

    \sa executedQuery()
*/
QString QPreparedSqlQuery::lastQuery() const
{
    return mText;
}

/*!
    Retrieves the next record in the result, if available, and
    positions the query on the retrieved record.

    Returns true if it was able to move to the next record.
*/
bool QPreparedSqlQuery::next()
{
    if (skip_step) {
        skip_step = false;
        return step_res == SQLITE_ROW;
    } else {
        switch(sqlite3_step(mHandle))
        {
            case SQLITE_ROW:
                return true;
            case SQLITE_DONE:
                return false;
            case SQLITE_BUSY:
                mError.setDriverText( sqlite3_errmsg(mDBHandle) );
                qLog(Sql) << mError.driverText();
                return false;
            case SQLITE_ERROR:
                mError.setDriverText( sqlite3_errmsg(mDBHandle) );
                qLog(Sql) << mError.driverText();
                return false;
            case SQLITE_MISUSE:
                mError.setDriverText( sqlite3_errmsg(mDBHandle) );
                qLog(Sql) << mError.driverText();
                return false;
        }
    }
    return false;
}

//bool QPreparedSqlQuery::seek( int index, bool relative = false )
//{
//}

/*!
    Returns the value of field \a index in the current record.

    \sa QSqlQuery::value()
*/
QVariant QPreparedSqlQuery::value( int index ) const
{
    switch(sqlite3_column_type(mHandle, index)) {
        case SQLITE_INTEGER:
            return QVariant(sqlite3_column_int64(mHandle, index));
        case SQLITE_FLOAT:
            return QByteArray(static_cast<const char *>(
                        sqlite3_column_blob(mHandle, index)),
                    sqlite3_column_bytes(mHandle, index));
        case SQLITE_BLOB:
            return QVariant(sqlite3_column_double(mHandle, index));
        case SQLITE_NULL:
            return QVariant();
        default:
        case SQLITE_TEXT:
            return QString::fromUtf16(static_cast<const ushort *>(sqlite3_column_text16(mHandle, index)),
                        sqlite3_column_bytes16(mHandle, index) / sizeof(ushort));
    }
}

#else // SQLITE_DIRECT

QPreparedSqlQuery::QPreparedSqlQuery()
    : mQuery(0)
{
}

QPreparedSqlQuery::QPreparedSqlQuery(QSqlDatabase db)
    : mQuery(new QSqlQuery(db))
{
    mQuery->setForwardOnly(true);
}


// +1, hash being different from statement means not prepared.
QPreparedSqlQuery::QPreparedSqlQuery(const QString &statement)
    : mText(statement), mQuery(0)
{
}

QPreparedSqlQuery::~QPreparedSqlQuery()
{
    clear();
}

bool QPreparedSqlQuery::isValid() const
{
    return mQuery && mQuery->isValid();
}

bool QPreparedSqlQuery::prepare()
{
    qLog(Sql) << "QPreparedSqlQuery::prepare()";
    bool res;
    if (!mText.isEmpty() && !mQuery)
    {
        mQuery = new QSqlQuery(QPimSqlIO::database());
        mQuery->setForwardOnly(true);
        qLog(Sql) << "QPreparedSqlQuery::prepare() - parse statement:" << mText;
        res =  mQuery->prepare(mText);
    } else if (mQuery) {
        res = true;
    } else
        res = false;
    if (res) {
        qLog(Sql) << "prepare query succeeded";
    } else {
        qLog(Sql) << "prepare query error:" << mQuery->lastError().text();
        mErrorList.append(mQuery->lastError.text());
    }
    return res;
}

bool QPreparedSqlQuery::prepare(const QString &text)
{
    if (text != mText && !text.isEmpty())
    {
        clear();
        delete mQuery;
        mQuery = 0;
        mText = text;
        return prepare();
    } else if (text.isEmpty()) {
        qLog(Sql) << "prepare query error: no text";
        return false;
    }
    qLog(Sql) << "prepare query succeeded - already prepared as text";
    return true;
}

void QPreparedSqlQuery::reset()
{
    qLog(Sql) << "QPreparedSqlQuery::reset() -" << mText;
    if (mQuery)
        mQuery->finish();
}

void QPreparedSqlQuery::clear()
{
    qLog(Sql) << "QPreparedSqlQuery::clear() - " << mText;
    if (mQuery)
        mQuery->clear();
}

void QPreparedSqlQuery::bindValue( const QString & placeholder, const QVariant & value, QSql::ParamType type )
{
    qLog(Sql) << "QPreparedSqlQuery::bindValue(" << placeholder << value << ") -" << mText;
    if (mQuery)
        mQuery->bindValue(placeholder, value, type);
}

void QPreparedSqlQuery::bindValue( int pos, const QVariant & value, QSql::ParamType type)
{
    qLog(Sql) << "QPreparedSqlQuery::bindValue(" << pos << value << ") -" << mText;
    if (mQuery)
        mQuery->bindValue(pos, value, type);
}

QMap<QString, QVariant> QPreparedSqlQuery::boundValues() const
{
    if (mQuery)
        return mQuery->boundValues();
    return QMap<QString, QVariant>();
}

bool QPreparedSqlQuery::exec()
{
    qLog(Sql) << "QPreparedSqlQuery::exec()";
    if (mQuery) {
        bool res;
        res =  mQuery->exec();
        if (!res)
            qLog(Sql) << "QPreparedSqlQuery::exec() -" << mQuery->lastError();
        return res;
    }
    qLog(Sql) << "QPreparedSqlQuery::exec() - query not prepared";
    mErrorList.append(mQuery->lastError());
    return false;
}

bool QPreparedSqlQuery::isNull( int field ) const
{
    if (mQuery)
        return mQuery->isNull(field);
    return false;
}

QSqlError QPreparedSqlQuery::lastError() const
{
    if (mQuery)
        return mQuery->lastError();
    return QSqlError();
}

QString QPreparedSqlQuery::lastQuery() const
{
    if (mQuery)
        return mQuery->lastQuery();
    return QString();
}

bool QPreparedSqlQuery::next()
{
    if (mQuery)
        return mQuery->next();
    return false;
}

QVariant QPreparedSqlQuery::value( int index ) const
{
    if (mQuery)
        return mQuery->value(index);
    return QVariant();
}
#endif // SQLITE_DIRECT
