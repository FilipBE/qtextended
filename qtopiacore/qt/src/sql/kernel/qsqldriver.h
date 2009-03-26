/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtSql/qsql.h>
#ifdef QT3_SUPPORT
#include <QtSql/qsqlquery.h>
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sql)

class QSqlDatabase;
class QSqlDriverPrivate;
class QSqlError;
class QSqlField;
class QSqlIndex;
class QSqlRecord;
class QSqlResult;
class QVariant;

class Q_SQL_EXPORT QSqlDriver : public QObject
{
    friend class QSqlDatabase;
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlDriver)

public:
    enum DriverFeature { Transactions, QuerySize, BLOB, Unicode, PreparedQueries,
                         NamedPlaceholders, PositionalPlaceholders, LastInsertId,
                         BatchOperations, SimpleLocking, LowPrecisionNumbers,
                         EventNotifications, FinishQuery, MultipleResultSets };

    enum StatementType { WhereStatement, SelectStatement, UpdateStatement,
                         InsertStatement, DeleteStatement };

    enum IdentifierType { FieldName, TableName };

    explicit QSqlDriver(QObject *parent=0);
    ~QSqlDriver();
    virtual bool isOpen() const;
    bool isOpenError() const;

    virtual bool beginTransaction();
    virtual bool commitTransaction();
    virtual bool rollbackTransaction();
    virtual QStringList tables(QSql::TableType tableType) const;
    virtual QSqlIndex primaryIndex(const QString &tableName) const;
    virtual QSqlRecord record(const QString &tableName) const;
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QSqlRecord record(const QSqlQuery& query) const
    { return query.record(); }
    inline QT3_SUPPORT QSqlRecord recordInfo(const QString& tablename) const
    { return record(tablename); }
    inline QT3_SUPPORT QSqlRecord recordInfo(const QSqlQuery& query) const
    { return query.record(); }
    inline QT3_SUPPORT QString nullText() const { return QLatin1String("NULL"); }
    inline QT3_SUPPORT QString formatValue(const QSqlField *field, bool trimStrings = false) const
    { return field ? formatValue(*field, trimStrings) : QString(); }
#endif
    virtual QString formatValue(const QSqlField& field, bool trimStrings = false) const;

    virtual QString escapeIdentifier(const QString &identifier, IdentifierType type) const;
    virtual QString sqlStatement(StatementType type, const QString &tableName,
                                 const QSqlRecord &rec, bool preparedStatement) const;

    QSqlError lastError() const;

    virtual QVariant handle() const;
    virtual bool hasFeature(DriverFeature f) const = 0;
    virtual void close() = 0;
    virtual QSqlResult *createResult() const = 0;

    virtual bool open(const QString& db,
                      const QString& user = QString(),
                      const QString& password = QString(),
                      const QString& host = QString(),
                      int port = -1,
                      const QString& connOpts = QString()) = 0;
    bool subscribeToNotification(const QString &name);	    // ### Qt 5: make virtual
    bool unsubscribeFromNotification(const QString &name);  // ### Qt 5: make virtual
    QStringList subscribedToNotifications() const;          // ### Qt 5: make virtual

Q_SIGNALS:
    void notification(const QString &name);

protected:
    virtual void setOpen(bool o);
    virtual void setOpenError(bool e);
    virtual void setLastError(const QSqlError& e);

protected Q_SLOTS:
    bool subscribeToNotificationImplementation(const QString &name);        // ### Qt 5: eliminate, see subscribeToNotification()
    bool unsubscribeFromNotificationImplementation(const QString &name);    // ### Qt 5: eliminate, see unsubscribeFromNotification()
    QStringList subscribedToNotificationsImplementation() const;            // ### Qt 5: eliminate, see subscribedNotifications()

private:
    Q_DISABLE_COPY(QSqlDriver)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSQLDRIVER_H
