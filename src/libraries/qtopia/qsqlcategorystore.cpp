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
#include "qsqlcategorystore_p.h"
#include <QtopiaSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>

/*!
    \class QSqlCategoryStore
    \inpublicgroup QtBaseModule

    \brief The QSqlCategory store is an implementation of the QCategoryStore interface for managing categories in an SQL database.

    \internal
*/

/*!
    Constructs a new SQL category store.
*/
QSqlCategoryStore::QSqlCategoryStore()
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if (qApp)
        connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(reloadCategories()));
#endif
}

/*!
    Destroys an SQL category store.
*/
QSqlCategoryStore::~QSqlCategoryStore()
{
}

/*!
    \reimp
*/
bool QSqlCategoryStore::addCategory( const QString &categoryId, const QString &scope, const QString &label, const QString &icon, bool isSystem )
{
    static const QString insertquery = QLatin1String(
            "INSERT INTO categories (categoryid, categorytext, categoryscope, categoryicon, flags) "
            "VALUES (:id, :label, :categoryscope, :categoryicon, :flags)" );

    bool oneSucceeded = false;

    foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
    {
        QSqlQuery q(db);
        q.prepare(insertquery);
        q.bindValue(":id", categoryId);
        q.bindValue(":label", label);
        q.bindValue(":categoryscope", scope);
        q.bindValue(":categoryicon", icon);
        q.bindValue(":flags", isSystem ? 1:0);

        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::addCategory failed:" << q.lastError();
        } else {
            oneSucceeded = true;
        }
    }

    categoryAddedLocal( categoryId );

    return oneSucceeded;
}

/*!
    \reimp
*/
bool QSqlCategoryStore::categoryExists( const QString &categoryId )
{
    return !categoryFromId( categoryId ).isNull();
}

/*!
    \reimp
*/
QCategoryData QSqlCategoryStore::categoryFromId( const QString &categoryId )
{
    if( m_cache.contains( categoryId ) )
        return *m_cache[ categoryId ];

    static const QString selectString = QLatin1String(
            "SELECT categorytext, categoryscope, categoryicon, filename, flags "
            "FROM categories LEFT JOIN categoryringtone "
            "ON categories.categoryid = categoryringtone.categoryid "
            "WHERE categories.categoryid = :categoryId" );

    foreach( const QSqlDatabase &db, QtopiaSql::instance()->databases() )
    {
        QSqlQuery selectQuery( db );
        selectQuery.prepare( selectString );

        selectQuery.bindValue( QLatin1String( ":categoryId" ), categoryId );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            qWarning() << "QSqlCategoryStore::category(QString) : selectQuery";
            qWarning() << selectQuery.lastError();
        }
        else if( selectQuery.first() )
            return categoryFromRecord( categoryId, selectQuery.record() );
    }

    return QCategoryData();
}

/*!
    \reimp
*/
QMap< QString, QCategoryData > QSqlCategoryStore::scopeCategories( const QString &scope )
{
    QMap< QString, QCategoryData > categories;

    static const QString selectStringScoped = QLatin1String(
            "SELECT categories.categoryid, categorytext, categoryscope, categoryicon, filename, flags "
            "FROM categories LEFT JOIN categoryringtone "
            "ON categories.categoryid = categoryringtone.categoryid "
            "WHERE categoryscope = :scope OR categoryscope IS NULL" );

    static const QString selectStringGlobal = QLatin1String(
            "SELECT categories.categoryid, categorytext, categoryscope, categoryicon, filename, flags "
            "FROM categories LEFT JOIN categoryringtone "
            "ON categories.categoryid = categoryringtone.categoryid "
            "WHERE categoryscope IS NULL" );

    QString selectString = !scope.isEmpty()
            ? selectStringScoped
            : selectStringGlobal;

    foreach( const QSqlDatabase &db, QtopiaSql::instance()->databases() )
    {
        QSqlQuery selectQuery( db );
        selectQuery.prepare( selectString );

        if( !scope.isEmpty() )
            selectQuery.bindValue( QLatin1String( ":scope" ), scope );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            qWarning() << "QSqlCategoryStore::categories(QString) : selectQuery";
            qWarning() << selectQuery.lastError();
            qWarning() << "executing:" << selectString;
        }

        while( selectQuery.next() )
        {
            QString id = selectQuery.value( 0 ).toString();

            if( m_cache.contains( id ) )
                categories[ id ] = *m_cache[ id ];
            else
                categories[ id ] = categoryFromRecord( id, selectQuery.record() );
        }
    }

    return categories;
}

/*!
    \reimp
*/
bool QSqlCategoryStore::removeCategory( const QString &categoryId )
{
    static const QString remquery = QLatin1String("DELETE FROM categories WHERE categoryid = :id");
    static const QString remquery2 = QLatin1String("DELETE FROM categoryringtone WHERE categoryid = :id");

    bool oneSucceeded = false;

    foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
    {
        QSqlQuery q(db);
        q.prepare(remquery);
        q.bindValue(":id", categoryId);

        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::remove(QString)";
            qWarning() << q.lastError();
        } else {
            oneSucceeded = true;
        }
        q.clear();

        // remove ringtone records
        q.prepare(remquery2);
        q.bindValue(":id", categoryId);
        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::remove(QString)";
            qWarning() << q.lastError();
        } else {
            oneSucceeded = true;
        }
    }
    categoryRemovedLocal( categoryId );
    return oneSucceeded;
}

/*!
    \reimp
*/
bool QSqlCategoryStore::setCategoryScope( const QString &categoryId, const QString &scope )
{
    bool oneSucceeded = false;

    if( scope.isEmpty() ) {
        // straight update.
        static const QString makelocal = QLatin1String("UPDATE categories SET categoryscope = NULL WHERE categoryid = :id");
        foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
        {
            QSqlQuery q(db);
            q.prepare(makelocal);
            q.bindValue(":id", categoryId);

            QtopiaSql::instance()->logQuery( q );
            if (!q.exec()) {
                qWarning() << "QSqlCategoryStore::setScope(QString,QString) : makeGlobal";
                qWarning() << q.lastError();
            } else {
                oneSucceeded = true;
            }
        }
        categoryChangedLocal( categoryId );
        return oneSucceeded;
    } else {
        static const QString makelocal = QLatin1String("UPDATE categories SET categoryscope = :categoryscope WHERE categoryid = :id");
        foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
        {
            QSqlQuery q(db);
            q.prepare(makelocal);
            q.bindValue(":categoryscope", scope);
            q.bindValue(":id", categoryId);

            QtopiaSql::instance()->logQuery( q );
            if (!q.exec()) {
                qWarning() << "QSqlCategoryStore::setScope(QString,QString) : makeScoped";
                qWarning() << q.lastError();
            } else {
                oneSucceeded = true;
            }
        }
        categoryChangedLocal( categoryId );
        return oneSucceeded;
    }
}

/*!
    \reimp
*/
bool QSqlCategoryStore::setCategoryIcon( const QString &categoryId, const QString &icon )
{
    bool oneSucceeded = false;

    static const QString namequery = QLatin1String("UPDATE categories SET categoryicon = :icon WHERE categoryid = :id");
    foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
    {
        QSqlQuery q(db);
        q.prepare(namequery);
        q.bindValue(":icon", icon);
        q.bindValue(":id", categoryId);

        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::setIcon(QString,QString)";
            qWarning() << q.lastError();
        } else {
            oneSucceeded = true;
        }
    }

    categoryChangedLocal( categoryId );

    return oneSucceeded;
}

/*!
    \reimp
*/
bool QSqlCategoryStore::setCategoryRingTone( const QString &categoryId, const QString &fileName )
{
    bool oneSucceeded = false;

    // remove the record from the table.
    static const QString deletequery = QLatin1String("DELETE FROM categoryringtone WHERE categoryid = :id");
    foreach ( const QSqlDatabase &db, QtopiaSql::instance()->databases() )
    {
        QSqlQuery q(db);
        q.prepare(deletequery);
        q.bindValue(":id", categoryId);

        QtopiaSql::instance()->logQuery( q );
        if ( !q.exec() ) {
            qWarning() << "QSqlCategoryStore::setCategoryRingTone(QString,QString)";
            qWarning() << q.lastError();
        } else {
            oneSucceeded = true;
        }
    }

    categoryChangedLocal( categoryId );

    if ( fileName.isEmpty() )
        return oneSucceeded;

    // if there is new file name insert the new record.
    static const QString insertquery = QLatin1String("INSERT INTO categoryringtone (categoryid, filename) VALUES (:id, :fn)");
    foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
    {
        QSqlQuery q(db);
        q.prepare(insertquery);
        q.bindValue(":id", categoryId);
        q.bindValue(":fn", fileName);

        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::setCategoryRingTone(QString,QString)";
            qWarning() << q.lastError();
        } else {
            oneSucceeded = true;
        }
    }

    categoryChangedLocal( categoryId );

    return oneSucceeded;
}

/*!
    \reimp
*/
bool QSqlCategoryStore::setCategoryLabel( const QString &categoryId, const QString &label )
{
    bool oneSucceeded = false;

    static const QString namequery = QLatin1String("UPDATE categories SET categorytext = :label WHERE categoryid = :id");
    foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
    {
        QSqlQuery q(db);
        q.prepare(namequery);
        q.bindValue(":label", label);
        q.bindValue(":id", categoryId);

        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::setLabel failed:" << q.lastError();
        } else {
            oneSucceeded = true;
        }
    }

    categoryChangedLocal( categoryId );

    return oneSucceeded;
}

/*!
    \reimp
*/
bool QSqlCategoryStore::setSystemCategory( const QString &categoryId )
{
    static const QString systemquery = QLatin1String("update categories set flags=flags|1 where categoryid=:id");

    bool oneSucceeded = false;

    foreach(const QSqlDatabase &db, QtopiaSql::instance()->databases())
    {
        QSqlQuery q(db);
        q.prepare(systemquery);
        q.bindValue(":id", categoryId);

        QtopiaSql::instance()->logQuery( q );
        if (!q.exec()) {
            qWarning() << "QSqlCategoryStore::setSystem(QString)";
            qWarning() << q.lastError().text();
        } else {
            oneSucceeded = true;
        }
    }
    return oneSucceeded;
}

/*!
    Flushes cached data and emits the \l categoriesChanged() signal in response to receiving categoriesChanged on
    the the \c QPE/System channel.
*/
void QSqlCategoryStore::reloadCategories()
{
    m_cache.clear();

    emit categoriesChanged();
}

/*!
    Constructs a new QCategoryData instance from an SQL record.
*/
QCategoryData QSqlCategoryStore::categoryFromRecord( const QString &id, const QSqlRecord &record )
{
    QString label = record.value( QLatin1String( "categorytext" ) ).toString();
    QString scope = record.value( QLatin1String( "categoryscope" ) ).toString();
    QString iconFile = record.value( QLatin1String( "categoryicon" ) ).toString();
    QString ringtone = record.value( QLatin1String( "filename" ) ).toString();
    int systemFlag = record.value( "flags" ).toInt();

    QCategoryData::Flags flags;

    if( scope.isEmpty() )
        flags |= QCategoryData::Global;

    if ( systemFlag )
    {
        flags |= QCategoryData::System;

        label = Qtopia::translate( "Categories-*", "Categories", label );
    }

    QCategoryData *category = new QCategoryData( label, iconFile, ringtone, flags );

    m_cache.insert( id, category );

    return *category;
}

/*!
    Sends notifications indicating the category with the id \a id has been added.
*/
void QSqlCategoryStore::categoryAddedLocal( const QString &id )
{
    Q_UNUSED( id );

    m_cache.clear();

    QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" );
}

/*!
    Sends notifications indicating the category with the id \a id has been removed.
 */
void QSqlCategoryStore::categoryRemovedLocal( const QString &id )
{
    Q_UNUSED( id );

    m_cache.clear();

    QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" );
}

/*!
    Sends notifications indicating the category with the id \a id has been changed.
 */
void QSqlCategoryStore::categoryChangedLocal( const QString &id )
{
    Q_UNUSED( id );

    m_cache.clear();

    QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" );
}
