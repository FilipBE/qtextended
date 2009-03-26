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

#ifndef QPREPAREDQUERY_P_H
#define QPREPAREDQUERY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QVariant>
#include <QStringList>
#include <QSqlError>
#include <QSqlQuery>

#ifndef QT_44_API_QSQLQUERY_FINISH
#define SQLITE_DIRECT
#endif

#ifdef SQLITE_DIRECT
struct sqlite3_stmt;
struct sqlite3;
#endif

class QPreparedSqlQuery
{
public:
    QPreparedSqlQuery();
    QPreparedSqlQuery(QSqlDatabase);
    QPreparedSqlQuery(const QString &statement);
    ~QPreparedSqlQuery();

    bool isValid() const;

    bool prepare();
    bool prepare(const QString &);
    void reset();
    void clear();

    void bindValue( const QString & placeholder, const QVariant & val, QSql::ParamType paramType = QSql::In );
    void bindValue( int pos, const QVariant & val, QSql::ParamType paramType = QSql::In );

    QMap<QString, QVariant> boundValues() const;

    bool exec();
    bool isNull( int field ) const;

    QSqlError lastError() const;
    QString lastQuery() const;
    bool next();
    //bool seek( int index, bool relative = false );
    QVariant value( int index ) const;

    int errorCount() const;
    QStringList errors() const;
    void clearErrors();

private:
    void buildQuery();
    QString mText;
    QStringList mErrorList;
#ifdef SQLITE_DIRECT
    sqlite3_stmt *mHandle;
    sqlite3 *mDBHandle;
    QSqlError mError;
    bool skip_step;
    int step_res;
#else
    QSqlQuery *mQuery;
#endif
};

#endif
