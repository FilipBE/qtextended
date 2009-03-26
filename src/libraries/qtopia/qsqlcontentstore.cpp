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

#include "qsqlcontentstore_p.h"
#include "qfscontentengine_p.h"

#include <QtopiaSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFileSystem>
#include <qtopialog.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include "drmcontent_p.h"
#include "qdrmcontentengine_p.h"
#endif
#include "qsqlcontentsetengine_p.h"
#include "qmimetypedata_p.h"
#include "contentpluginmanager_p.h"
#include <qtopianamespace.h>

#ifndef QTOPIA_CONTENT_INSTALLER
#include "qcontent_p.h"
#endif

#include <string.h>

/*!
    \class QSqlContentStore
    \inpublicgroup QtBaseModule

    \brief The QSqlContentStore class is an implementation of QContentStore that provides access to an SQL database of content data.

    \internal
*/

/*!
    Constructs a new QSqlContentStore.
*/
QSqlContentStore::QSqlContentStore()
{
}

/*!
    Destroys a QSqlContentStore.
*/
QSqlContentStore::~QSqlContentStore()
{
}

/*!
    \reimp
*/
QContent QSqlContentStore::contentFromId( QContentId contentId )
{
    static const QString selectString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
                   "locationLookup.location as directory, icon, lastUpdated "
            "FROM content left join locationLookup on content.location = locationLookup.pKey "
            "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
            "WHERE cid=:cid" );

    if( contentId == QContent::InvalidId )
        return QContent();

    QContent content = QContentCache::instance()->lookup( contentId );

    if( !content.isNull() || !QtopiaSql::instance()->isValidDatabaseId( contentId.first ) )
        return content;

    QSqlQuery selectQuery( QtopiaSql::instance()->database( contentId.first ) );
    selectQuery.prepare( selectString );

    selectQuery.bindValue( QLatin1String( ":cid" ), contentId.second );

    QtopiaSql::instance()->logQuery( selectQuery );

    if ( !selectQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "selectQuery", contentId.first, selectQuery.lastError() );
    }
    else if ( !selectQuery.first() )
    {
        logError( __PRETTY_FUNCTION__,
                  QString( "No records returned for id %1:%2" ).arg( contentId.first ).arg( contentId.second ) );
    }
    else
    {
        content = contentFromQuery( selectQuery, contentId.first );

        QContentCache::instance()->cache( content );
    }

    return content;
}

/*!
    \reimp
*/
QContent QSqlContentStore::contentFromFileName( const QString &fileName, LookupFlags lookup )
{
    qLog(DocAPI) << __PRETTY_FUNCTION__ << fileName << lookup;

    if( fileName.isEmpty() )
    {
        return QContent();
    }

    if( lookup & Lookup )
    {
        static const QString selectString = QLatin1String(
                "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
                       "locationLookup.location as directory, icon, lastUpdated "
                "FROM content left join locationLookup on content.location = locationLookup.pKey "
                             "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
                "WHERE path = :path and locationLookup.location = :location" );

        if( fileName.endsWith( QLatin1String( ".desktop" ), Qt::CaseInsensitive ) )
        {
            QContent content = desktopContent( fileName );

            if( !content.isNull() )
                return content;
        }

        int slash = fileName.lastIndexOf( '/' );

        if( slash == -1 )
        {
            return executableContent( fileName );
        }

        QtopiaDatabaseId dbId = QtopiaSql::instance()->databaseIdForPath( fileName );

        if( dbId == quint32(-1) )
            return QContent();

        QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );
        selectQuery.prepare( selectString );

        selectQuery.bindValue( QLatin1String( ":path" ), fileName.mid( slash + 1 ) );
        selectQuery.bindValue( QLatin1String( ":location" ), fileName.left( slash ) );

        QtopiaSql::instance()->logQuery( selectQuery );

        if ( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );

            return QContent();
        }

        if( selectQuery.first() )
        {
            QContent content = contentFromQuery( selectQuery, dbId );

            QContentCache::instance()->cache( content );

            return content;
        }
    }

    QContent content;

    if( lookup & Construct )
    {
        QContentEngine *engine = new QFSContentEngine;

        engine->setFileName( fileName );

        QContent c( engine );

        if( !(lookup & Commit) )
        {
            if( (engine = installContent( &c ) ) != 0 )
                content = QContent( engine );
        }
        else if( commitContent( &c ) )
        {
            content = c;
        }
    }

    if( content.id() != QContent::InvalidId )
        QContentCache::instance()->cache( content );

    return content;
}

/*!
    \reimp
*/
QContent QSqlContentStore::contentFromEngineType( const QString &engineType )
{
    Q_UNUSED( engineType );

    return QContent( new QFSContentEngine );
}

/*!
    \reimp
*/
bool QSqlContentStore::commitContent( QContent *content )
{
    qLog(DocAPI) << __PRETTY_FUNCTION__ << *content;

#ifndef QTOPIA_CONTENT_INSTALLER
    if( !content->isValid( true ) )
    {
        qLog(DocAPI) << "Could not commit invalid QContent" << *content;
        return false;
    }
    else
#endif
    if( content->id() == QContent::InvalidId )
    {
        QContentEngine *engine = 0;

        if( content->lastUpdated().isNull() )
        {
            if( (engine = installContent( content )) == 0 )
                return false;
        }
        else if( !contentEngine( content )->isOutOfDate() || (engine = refreshContent( content )) == 0 )
        {
            engine = contentEngine( content );
        }

        if( engine && insertContent( engine, QtopiaSql::instance()->databaseIdForPath( engine->fileName() ) ) )
        {
            *content = QContent( engine );

            QContentCache::instance()->cache( *content );

            return true;
        }
        else
        {
            if( engine && !engine->ref )
                delete engine;

            return false;
        }
    }
    else
    {
        QContentEngine *engine = 0;

        if( ( content->name().isNull() || content->type().isNull() || contentEngine( content )->isOutOfDate() )
            && (engine = refreshContent( content )) != 0 )
        {
            *content = QContent( engine );
        }

        QtopiaDatabaseId dbId = QtopiaSql::instance()->databaseIdForPath( content->fileName() );

        if( dbId == content->id().first )
        {
            if( updateContent( contentEngine( content ) ) )
            {
                QContentCache::instance()->cache( *content );

                return true;
            }
            else
                return false;
        }
        else if( dbId == quint32(-1) )
        {
            uninstallContent( content->id() );

            return false;
        }
        else
        {
            QContentEngine *engine = contentEngine( content );

            ensurePropertiesLoaded( engine );
            ensureCategoriesLoaded( engine );

            uninstallContent( content->id() );

            if( insertContent( engine, dbId ) )
            {
                QContentCache::instance()->cache( *content );

                return true;
            }
            else
                return false;
        }
    }
}

/*!
    \reimp
 */
bool QSqlContentStore::moveContentTo( QContent *content, const QString &newFileName )
{
    if( contentEngine( content )->moveTo( newFileName ) )
    {
        if( commitContent( content ) )
            return true;
    }
    return false;
}

/*!
    \reimp
*/
bool QSqlContentStore::copyContentTo( QContent *content, const QString &newFileName )
{
    QContentEngine *newEngine = contentEngine( content )->copyTo( newFileName );

    if( newEngine )
    {
        QContent newContent( newEngine );

        return commitContent( &newContent );
    }
    else
        return false;
}

/*!
    \reimp
*/
bool QSqlContentStore::renameContent(QContent *content, const QString &name)
{
    return contentEngine(content)->rename(name) && commitContent(content);
}

/*!
    \reimp
*/
QIODevice *QSqlContentStore::openContent( QContent *content, QIODevice::OpenMode mode )
{
    QIODevice *dev = contentEngine( content )->open( mode );

    if( dev && (mode & (QIODevice::WriteOnly | QIODevice::Append | QIODevice::Truncate)) )
    {
        commitContent( content );

        setLastUpdated( QDateTime(), contentEngine( content ) );
    }

    return dev;
}

/*!
    reimp
*/
QContentEngine *QSqlContentStore::installContent( QContent *content )
{
    QContentEngine *engine = QContentEngine::createEngine( *content );

    if( engine )
        return engine;

    QString fileName = content->fileName();

    if( !fileName.isEmpty() )
    {
        QMimeType type = content->type().isEmpty()
                ? QMimeType::fromFileName( fileName )
                : QMimeType::fromId( content->type() );

        QString name = deriveName( fileName, type );

        QFileInfo fileInfo(fileName);

#ifndef QTOPIA_CONTENT_INSTALLER
        if( DrmContentPrivate::installContent( fileName, content ) )
        {
            engine = new QDrmContentEngine;

            engine->copy( *contentEngine( content ) );

            setDrmState( QContent::Protected, engine );
        }
        else if( QContentFactory::installContent( fileName, content ) || !fileInfo.isDir() )
        {
#else
            QContentFactory::installContent( fileName, content );
#endif
            engine = new QFSContentEngine;

            engine->copy( *contentEngine( content ) );
#ifndef QTOPIA_CONTENT_INSTALLER
        }
#endif
        if( engine )
        {
            if( engine->name().isEmpty() )
                engine->setName( name );

            if( engine->mimeType().isNull() )
                engine->setMimeType( type );

            if( engine->role() == QContent::UnknownUsage )
                engine->setRole( QContent::Document );

            if (engine->property(QLatin1String("none"), QLatin1String("CreationDate")).isEmpty()) {
                engine->setProperty(
                        QLatin1String("none"),
                        QLatin1String("CreationDate"),
                        fileInfo.created().toString(Qt::ISODate));
            }

            if (engine->lastUpdated().isNull())
                setLastUpdated(fileInfo.lastModified(), engine);
        }
    }

    return engine;
}

QContentEngine *QSqlContentStore::refreshContent( QContent *content )
{
    QContentEngine *engine = 0;

#ifndef QTOPIA_CONTENT_INSTALLER
    bool wasProtected = content->drmState() == QContent::Protected;

    if( DrmContentPrivate::updateContent( content ) )
    {
        if( !wasProtected )
        {
            engine = new QDrmContentEngine;

            engine->copy( *contentEngine( content ) );

            setId( content->id(), engine );
            setDrmState( QContent::Protected, engine );
        }
        else
            engine = contentEngine( content );
    }
    else if( !wasProtected && QContentFactory::updateContent( content ) || content->name().isEmpty() || content->type().isEmpty() )
#else
    if( QContentFactory::updateContent( content ) )
#endif
    {
        engine = contentEngine( content );
    }

    if( engine )
    {
        if( engine->mimeType().isNull() )
            engine->setMimeType( QMimeType::fromFileName( engine->fileName() ) );

        if( engine->name().isEmpty() )
            engine->setName( deriveName( engine->fileName(), engine->mimeType() ) );
    }

    return engine;
}

/*!
    \reimp
*/
bool QSqlContentStore::removeContent( QContent *content )
{
    if( contentEngine( content )->remove() )
    {
        bool uninstalled = uninstallContent( content->id() );

        *content = QContent();

        return uninstalled;
    }
    else
    {
        logError( __PRETTY_FUNCTION__, contentEngine( content )->errorString() );

        return false;
    }
}

/*!
    \reimp
*/
bool QSqlContentStore::uninstallContent( QContentId contentId )
{
    if( contentId == QContent::InvalidId )
    {
        logError( __PRETTY_FUNCTION__, "Couldn't remove content: Invalid id" );

        return false;
    }

    QSqlDatabase db = QtopiaSql::instance()->database( contentId.first );

    if( !db.isValid() )
        return false;

    QContent content(contentId);
#ifndef QTOPIA_CONTENT_INSTALLER
    QFile::remove(thumbnailPath(content.fileName()));
#endif
    bool succeeded = true;

    {
        static const QString deletePropertyString = QLatin1String(
                "delete from contentProps "
                "where cid = :cid" );

        QSqlQuery deletePropertyQuery( db );
        deletePropertyQuery.prepare( deletePropertyString );

        deletePropertyQuery.bindValue( QLatin1String( "cid" ), contentId.second );

        QtopiaSql::instance()->logQuery( deletePropertyQuery );

        if( !deletePropertyQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "deletePropertyQuery", contentId.first, deletePropertyQuery.lastError() );

            succeeded = false;
        }

        if(content.role() == QContent::Application)
        {
            foreach(QString current, content.mimeTypes())
            {
                removeAssociation(current, content.fileName());
            }
        }
    }

    succeeded &= removeCategories( contentId );

    {
        static const QString deleteContentString = QLatin1String(
                "delete from content "
                "where cid = :cid" );

        QSqlQuery deleteContentQuery( db );
        deleteContentQuery.prepare( deleteContentString );

        deleteContentQuery.bindValue( QLatin1String( "cid" ), contentId.second );

        QtopiaSql::instance()->logQuery( deleteContentQuery );

        if( !deleteContentQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "deleteContentQuery", contentId.first, deleteContentQuery.lastError() );

            succeeded = false;
        }
    }

#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->addUpdated( contentId, QContent::Removed );
#endif

    return succeeded;
}

void QSqlContentStore::batchCommitContent( const QContentList &content )
{
    qLog(DocAPI) << "Entering QSqlContentStore::batchCommitContent";
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->beginInstall();
#endif
    QMultiHash<QtopiaDatabaseId, QContent> list;

    foreach( const QContent &c, content )
        list.insert( QtopiaSql::instance()->databaseIdForPath( c.fileName() ), c );

    foreach( QtopiaDatabaseId dbId, list.uniqueKeys() )
    {
        if( !QtopiaSql::instance()->isValidDatabaseId( dbId ) )
        {
            qLog(Sql) << "QSqlContentStore::batchCommitContent: invalid database id";
            continue;
        }
        QSqlDatabase db = QtopiaSql::instance()->database( dbId );

        if (!db.transaction()) {
            qWarning() << "QSqlContentStore::batchCommitContent: couldn't start transaction:" << db.lastError();

            continue;
        }

        foreach( QContent content, list.values( dbId ) )
            commitContent( &content );

        if( !db.commit() ) {
            qWarning() << "QSqlContentStore::batchCommitContent: couldn't commit transaction" << db.lastError();

            db.rollback();
        }
    }
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->endInstall();
#endif
    qLog(DocAPI) << "Leaving QSqlContentStore::batchCommitContent";
}

void QSqlContentStore::batchUninstallContent( const QContentIdList &content )
{
    qLog(DocAPI) << "Entering QContent::uninstallBatch";
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->beginInstall();
#endif
    QMultiHash<QtopiaDatabaseId, QContentId> list;

    foreach (const QContentId& id, content)
        list.insert(id.first, id);

    foreach (QtopiaDatabaseId dbid, list.uniqueKeys()) {
        if (!QtopiaSql::instance()->isValidDatabaseId(dbid)) {
            qLog(Sql) << "QSqlContentStore::batchUninstallContent: invalid database id";

            continue;
        }

        QSqlDatabase db = QtopiaSql::instance()->database(dbid);

        if (!db.transaction()) {
            qWarning()
                    << "QSqlContentStore::batchUninstallContent: couldn't start transaction"
                    << db.lastError().text();

            continue;
        }

        foreach (QContentId contentId, list.values(dbid))
            uninstallContent(contentId);

        if (!db.commit()) {
            qWarning()
                    << "QSqlContentStore::batchUninstallContent: couldn't commit transaction"
                    << db.lastError().text();

            db.rollback();
        }
    }
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->endInstall();
#endif
    qLog(DocAPI) << "Leaving QContent::uninstallBatch";
}

/*!
    \reimp
*/
QStringList QSqlContentStore::contentCategories( QContentId contentId )
{
    QStringList categories;

    if( contentId == QContent::InvalidId )
        return categories;

    static const QString selectString = QLatin1String(
            "select categoryid "
            "from mapCategoryToContent "
            "where cid = :cid" );

    QSqlDatabase database = QtopiaSql::instance()->database( contentId.first );

    if( database.isValid() )
    {
        QSqlQuery selectQuery( database );
        selectQuery.prepare( selectString );

        selectQuery.bindValue( QLatin1String( ":cid" ), contentId.second );

        QtopiaSql::instance()->logQuery( selectQuery );

        if ( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", contentId.first, selectQuery.lastError() );

            return categories;
        }

        while ( selectQuery.next() )
            categories << selectQuery.value( 0 ).toString();
    }
    return categories;
}

/*!
    \reimp
*/
QContentEnginePropertyCache QSqlContentStore::contentProperties( QContentId contentId )
{
    QContentEnginePropertyCache properties;

    if( contentId == QContent::InvalidId )
        return properties;

    static const QString selectString = QLatin1String(
            "select grp, name, value "
            "from contentProps "
            "where cid = :cid" );

    QSqlDatabase database = QtopiaSql::instance()->database( contentId.first );

    if( database.isValid() )
    {
        QSqlQuery selectQuery( database );
        selectQuery.prepare( selectString );

        selectQuery.bindValue( QLatin1String( ":cid" ), contentId.second );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", contentId.first, selectQuery.lastError() );

            return properties;
        }

        while( selectQuery.next() )
            properties[ selectQuery.value( 0 ).toString() ][ selectQuery.value( 1 ).toString() ] = selectQuery.value( 2 ).toString();
    }
    return properties;
}

/*!
    \reimp
*/
QStringList QSqlContentStore::contentMimeTypes( QContentId contentId )
{

    QSet<QString> contentMimeTypes;
    QContent content(contentId);

    if( contentId == QContent::InvalidId || content.role() != QContent::Application)
        return contentMimeTypes.toList();

    foreach(QMimeEngineData data, associationsForApplication(content.fileName()))
    {
        contentMimeTypes.insert(data.type);
    }

    return contentMimeTypes.toList();
}

/*!
    Writes the contents of the content engine \a engine to the database with the database id \a dbId.
*/
bool QSqlContentStore::insertContent( QContentEngine *engine, QtopiaDatabaseId dbId )
{
    if( dbId == quint32(-1) )
        return false;

    setLastUpdated(QFileInfo(engine->fileName() ).lastModified(), engine);

    static const QString insertString = QLatin1String(
            "insert into content( uiName, uiNameSortOrder, mType, drmFlags, docStatus, path, location, icon, lastUpdated ) "
            "values( :uiName, :uiNameSortOrder, :mType, :drmFlags, :docStatus, :path, :location, :icon, :lastUpdated )" );

    QSqlQuery insertQuery( QtopiaSql::instance()->database( dbId ) );
    insertQuery.prepare( insertString );

    insertQuery.bindValue( QLatin1String( ":uiName" ), engine->name() );
    insertQuery.bindValue( QLatin1String( ":uiNameSortOrder" ), transformString( Qtopia::dehyphenate( engine->translatedName() ) ) );
    insertQuery.bindValue( QLatin1String( ":mType" ), mimeId( engine->mimeType().id(), dbId ) );
    insertQuery.bindValue( QLatin1String( ":drmFlags" ), convertDrmState( engine->drmState() ) );
    insertQuery.bindValue( QLatin1String( ":docStatus" ), convertRole( engine->role() ) );

    int slash = engine->fileName().lastIndexOf( '/' );

    if( slash == -1 )
    {
        insertQuery.bindValue( QLatin1String( ":path" ), engine->fileName() );
        insertQuery.bindValue( QLatin1String( ":location" ), 0 );
    }
    else
    {
        insertQuery.bindValue( QLatin1String( ":path" ), engine->fileName().mid( slash + 1 ) );
        insertQuery.bindValue( QLatin1String( ":location" ), locationId( engine->fileName().left( slash ), dbId ) );
    }

    insertQuery.bindValue( QLatin1String( ":icon" ), engine->iconName() );
    insertQuery.bindValue( QLatin1String( ":lastUpdated" ), engine->lastUpdated().toTime_t() );

    QtopiaSql::instance()->logQuery( insertQuery );

    if( !insertQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "insertQuery", *engine, dbId, insertQuery.lastError() );

        return false;
    }
    else
    {
        QContentId id = QContentId( dbId, insertQuery.lastInsertId().toULongLong() );

        insertCategories( id, engine->categories() );
        insertProperties( id, *engine );

        setId( id, engine );

        if(engine->role() == QContent::Application)
        {
            foreach(QMimeEngineData current, engine->executableMimeTypes())
            {
                addAssociation(current.type, engine->fileName(), current.icon, current.permission);
            }
        }

#ifndef QTOPIA_CONTENT_INSTALLER
        QContentUpdateManager::instance()->addUpdated( id, QContent::Added );
#endif

        return true;
    }
}

bool QSqlContentStore::updateContent( QContentEngine *engine )
{
    static const QString updateString = QLatin1String(
            "update content "
            "set uiName = :uiName, uiNameSortOrder = :uiNameSortOrder, mType = :mType, drmFlags = :drmFlags, docStatus = :docStatus, path = :path, "
            "location = :location, icon = :icon, lastUpdated = :lastUpdated "
            "where cid = :cid" );

    QSqlDatabase database = QtopiaSql::instance()->database( engine->id().first );

    if( !database.isValid() )
        return false;

    setLastUpdated(QFileInfo(engine->fileName()).lastModified(), engine);

    QSqlQuery updateQuery( database );
    updateQuery.prepare( updateString );

    updateQuery.bindValue( QLatin1String( ":uiName" ), engine->name() );
    updateQuery.bindValue( QLatin1String( ":uiNameSortOrder" ), transformString( Qtopia::dehyphenate( engine->translatedName() ) ) );
    updateQuery.bindValue( QLatin1String( ":mType" ), mimeId( engine->mimeType().id(), engine->id().first ) );
    updateQuery.bindValue( QLatin1String( ":drmFlags" ), convertDrmState( engine->drmState() ) );
    updateQuery.bindValue( QLatin1String( ":docStatus" ), convertRole( engine->role() ) );

    int slash = engine->fileName().lastIndexOf( '/' );

    if( slash == -1 )
    {
        updateQuery.bindValue( QLatin1String( ":path" ), engine->fileName() );
        updateQuery.bindValue( QLatin1String( ":location" ), 0 );
    }
    else
    {
        updateQuery.bindValue( QLatin1String( ":path" ), engine->fileName().mid( slash + 1 ) );
        updateQuery.bindValue( QLatin1String( ":location" ), locationId( engine->fileName().left( slash ), engine->id().first ) );
    }

    updateQuery.bindValue( QLatin1String( ":icon" ), engine->iconName() );
    updateQuery.bindValue( QLatin1String( ":lastUpdated" ), engine->lastUpdated().toTime_t() );
    updateQuery.bindValue( QLatin1String( ":cid" ), engine->id().second  );

    QtopiaSql::instance()->logQuery( updateQuery );

    if( !updateQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "updateQuery", *engine, engine->id().first, updateQuery.lastError() );

        return false;
    }
    else
    {
        bool success = true;

        if( dirtyAttributes( *engine ) & QContentEngine::Categories )
        {
            success &= removeCategories( engine->id() );
            success &= insertCategories( engine->id(), engine->categories() );
        }

        if( dirtyAttributes( *engine ) & QContentEngine::Properties )
        {
            success &= removeProperties( engine->id() );
            success &= insertProperties( engine->id(), *engine );
        }

        if( dirtyAttributes( *engine ) & QContentEngine::ExecutableMimeTypes && engine->role() == QContent::Application)
        {
            // foreach the associations.
            foreach(QMimeEngineData current, engine->executableMimeTypes())
            {
                removeAssociation(current.type, engine->fileName());
                addAssociation(current.type, engine->fileName(), current.icon, current.permission);
            }
        }

#ifndef QTOPIA_CONTENT_INSTALLER
        QContentUpdateManager::instance()->addUpdated( engine->id(), QContent::Updated );
#endif
        return success;
    }
}

/*!
    Transforms a \a string into a form that can be binary compared to get locale aware comparison.

    Returns the transformed string.
*/
QByteArray QSqlContentStore::transformString( const QString &string ) const
{
    char buffer[ 1024 ];

    QByteArray localString = string.toLocal8Bit();

    if( strxfrm( buffer, localString.constData(), 1024 ) > 0 )
        return QByteArray( buffer );
    else
        return localString;
}

/*!
    \reimp
*/
QMimeTypeData QSqlContentStore::mimeTypeFromId( const QString &mimeId )
{
    QMimeTypeData data( mimeId );

    if( mimeId.isEmpty() )
        return data;

    static const QString selectString = QLatin1String(
            "select application, icon, drmFlags "
            "from mimeTypeMapping "
            "where mimeType = :major and mimeSubType = :minor" );

    QString mimeMajor, mimeMinor;
    if(!mimeId.contains('/'))
    {
        mimeMajor = mimeId;
        mimeMinor = '*';
    }
    else
    {
        mimeMajor = mimeId.section('/', 0, 0);
        mimeMinor = mimeId.section('/', 1, 1);
    }

    foreach(QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds())
    {
        QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );
        selectQuery.prepare( selectString );

        selectQuery.bindValue( QLatin1String( ":major" ), mimeMajor );
        selectQuery.bindValue( QLatin1String( ":minor" ), mimeMinor );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );
        }

        while( selectQuery.next() )
        {
            data.addApplication(
                    QContent( selectQuery.value( 0 ).toString() ),
                    selectQuery.value( 1 ).toString(),
                    static_cast<QDrmRights::Permission>(selectQuery.value( 2 ).toInt()) );
        }
    }

    static const QString selectDefaultString = QLatin1String(
        "select application "
        "from defaultMimeApplication "
        "where mimeType = :major and mimeSubType = :minor" );

    QSqlQuery selectDefaultQuery( QtopiaSql::instance()->systemDatabase() );
    selectDefaultQuery.prepare( selectDefaultString );

    selectDefaultQuery.bindValue( QLatin1String( ":major" ), mimeMajor );
    selectDefaultQuery.bindValue( QLatin1String( ":minor" ), mimeMinor );

    QtopiaSql::instance()->logQuery( selectDefaultQuery );

    if( !selectDefaultQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "selectDefaultQuery", 0, selectDefaultQuery.lastError() );
    }
    else if( selectDefaultQuery.first() )
    {
        data.setDefaultApplication( QContent( selectDefaultQuery.value( 0 ).toString() ) );
    }

    return data;
}

/*!
    \reimp
*/
QContentSetEngine *QSqlContentStore::contentSet( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode )
{
    qLog(DocAPI) << __PRETTY_FUNCTION__ << filter;

    return new QSqlContentSetEngine( filter, order, mode, this );
}

/*!
    \reimp
*/
QContentFilterSetEngine *QSqlContentStore::contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType )
{
    Q_UNUSED( filter );
    Q_UNUSED( type );
    Q_UNUSED( subType );

    return 0;
}

/*!
    \reimp
*/
QStringList QSqlContentStore::filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType )
{
    switch( type )
    {
    case QContentFilter::MimeType:
        return mimeFilterMatches( filter, subType );
    case QContentFilter::Category:
        return categoryFilterMatches( filter, subType );
    case QContentFilter::Synthetic:
        return syntheticFilterMatches( filter, subType.section( '/', 0, 0 ), subType.section( '/', 1, 1 ) );
    case QContentFilter::Directory:
        return directoryFilterMatches( filter, subType );
    default:
        return QStringList();
    }
}

int QSqlContentStore::contentCount( const QContentFilter &filter )
{
    int count = 0;

    if( !filter.isValid() )
        return count;

    int insertAt = 0;
    QList< Parameter > parameters;
    QStringList joins;

    QString whereString = QLatin1String(" WHERE ")
            + buildWhereClause( filter, &parameters, &insertAt, &joins );

    QString selectString
            = QLatin1String( "SELECT count(DISTINCT content.cid) FROM " )
            + buildFrom( filter, joins )
            + whereString;

    foreach(QtopiaDatabaseId dbid, QtopiaSql::instance()->databaseIds())
    {
        QSqlQuery selectQuery( QtopiaSql::instance()->database(dbid) );
        selectQuery.prepare( selectString );

        bindParameters( &selectQuery, parameters );

        QtopiaSql::instance()->logQuery( selectQuery );

        if ( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", dbid, selectQuery.lastError() );

            continue;
        }

        if( selectQuery.first() )
            count += selectQuery.value( 0 ).toInt();
    }

    return count;
}

/*!
    Returns a list of content ids that match the content filter \a filter, sorted in the given \a order.
*/
QContentIdList QSqlContentStore::matches( const QContentFilter &filter, const QContentSortCriteria &order )
{
    QContentIdList matches;

    if( !filter.isValid() )
        return matches;

    int insertAt = 0;
    QList< Parameter > parameters;
    QStringList joins;

    QString whereString = QLatin1String(" WHERE ")
            + buildWhereClause( filter, &parameters, &insertAt, &joins );

    QString orderString = buildOrderBy( order, &parameters, &insertAt, &joins );
    QString selectString
            = QLatin1String( "SELECT DISTINCT content.cid FROM " )
            + buildFrom( filter, joins )
            + whereString
            + orderString;

    foreach(QtopiaDatabaseId dbid, QtopiaSql::instance()->databaseIds())
    {
        QSqlQuery selectQuery( QtopiaSql::instance()->database(dbid) );
        selectQuery.prepare( selectString );

        bindParameters( &selectQuery, parameters );

        QtopiaSql::instance()->logQuery( selectQuery );

        if ( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", dbid, selectQuery.lastError() );

            continue;
        }

        while ( selectQuery.next() )
            matches.append( QContentId( dbid, selectQuery.value( 0 ).toULongLong() ) );
    }

    return matches;
}

QContentIdList QSqlContentStore::matches( QtopiaDatabaseId databaseId, const QContentFilter &filter, const QContentSortCriteria &order )
{
    QContentIdList matches;

    if( !filter.isValid() || !QtopiaSql::instance()->isValidDatabaseId( databaseId ) )
        return matches;

    int insertAt = 0;
    QList< Parameter > parameters;
    QStringList joins;

    QString whereString = QLatin1String(" WHERE ")
            + buildWhereClause( filter, &parameters, &insertAt, &joins );
    QString orderString = buildOrderBy( order, &parameters, &insertAt, &joins );
    QString selectString
            = QLatin1String( "SELECT DISTINCT content.cid FROM " )
            + buildFrom( filter, joins )
            + whereString
            + orderString;

    QSqlQuery selectQuery( QtopiaSql::instance()->database( databaseId ) );
    selectQuery.prepare( selectString );

    bindParameters( &selectQuery, parameters );

    QtopiaSql::instance()->logQuery( selectQuery );

    if( !selectQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "selectQuery", databaseId, selectQuery.lastError() );
    }
    else
    {
        while ( selectQuery.next() )
            matches.append( QContentId( databaseId, selectQuery.value( 0 ).toULongLong() ) );
    }

    return matches;
}

QContentList QSqlContentStore::contentFromIds( QtopiaDatabaseId databaseId, const QContentIdList &contentIds )
{
    static const QString selectString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
            "locationLookup.location as directory, icon, lastUpdated "
            "FROM content left join locationLookup on content.location = locationLookup.pKey "
            "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
            "WHERE cid IN (%1)" );

    QList< quint64 > queryIds;

    QMap< quint64, QContent > contentMap;

    foreach( QContentId contentId, contentIds )
    {
        QContent content = QContentCache::instance()->lookup( contentId );

        if( content.isNull() )
            queryIds.append(contentId.second );
        else
            contentMap[ contentId.second ] = content;
    }

    if( !queryIds.isEmpty() )
    {
        QString paramString = QLatin1String( "?" );

        for( int i = 1; i < queryIds.count(); i++ )
            paramString += QLatin1String( ", ?" );

        QSqlQuery selectQuery( QtopiaSql::instance()->database( databaseId ) );
        selectQuery.prepare( selectString.arg( paramString ) );

        for( int i = 0; i < queryIds.count(); i++ )
            selectQuery.bindValue( i, queryIds.at( i ) );

        QtopiaSql::instance()->logQuery( selectQuery );

        if ( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", databaseId, selectQuery.lastError() );
        }
        else
        {
            while( selectQuery.next() )
            {
                QContent content = contentFromQuery( selectQuery, databaseId );

                QContentCache::instance()->cache( content );

                contentMap[ content.id().second ] = content;
            }
        }
    }

    QContentList content;

    foreach( QContentId contentId, contentIds )
        content.append( contentMap.value( contentId.second ) );

    return content;
}

QContentList QSqlContentStore::contentFromIds( const QContentIdList &contentIds )
{
    static const QString selectString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
            "locationLookup.location as directory, icon, lastUpdated "
            "FROM content left join locationLookup on content.location = locationLookup.pKey "
            "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
            "WHERE cid IN (%1)" );

    QMap< QContentId, QContent > contentMap;
    foreach( QtopiaDatabaseId databaseId, QtopiaSql::instance()->databaseIds() )
    {
        QList< quint64 > queryIds;

        foreach( QContentId contentId, contentIds )
        {
            if( contentId.first != databaseId )
                continue;

            QContent content = QContentCache::instance()->lookup( contentId );

            if( content.isNull() )
                queryIds.append( contentId.second );
            else
                contentMap[ contentId ] = content;
        }

        if( queryIds.isEmpty() )
            continue;

        QString paramString = QLatin1String( "?" );

        for( int i = 1; i < queryIds.count(); i++ )
            paramString += QLatin1String( ", ?" );

        QSqlQuery selectQuery( QtopiaSql::instance()->database( databaseId ) );
        selectQuery.prepare( selectString.arg( paramString ) );

        for( int i = 0; i < queryIds.count(); i++ )
            selectQuery.bindValue( i, queryIds.at( i ) );

        QtopiaSql::instance()->logQuery( selectQuery );

        if ( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", databaseId, selectQuery.lastError() );
        }
        else
        {
            while( selectQuery.next() )
            {
                QContent content = contentFromQuery( selectQuery, databaseId );

                QContentCache::instance()->cache( content );

                contentMap[ content.id() ] = content;
            }
        }
    }

    QContentList content;

    foreach( QContentId contentId, contentIds )
        content.append( contentMap.value( contentId ) );

    return content;
}

/*!
    Constructs a content engine from an sql \a query executed on the database with the id \a dbId.

    The column names must map to the indexes below.
    0: content.cid
    1: content.uiName
    2: mimeTypeLookup.mimeType
    3: content.drmFlags
    4: content.docStatus
    5: content.path
    6: locationLookup.location
    7: content.icon
    8: content.lastUpdated
*/
QContent QSqlContentStore::contentFromQuery( const QSqlQuery &query, QtopiaDatabaseId dbId ) const
{
    QContent::DrmState drmState = convertDrmState( query.value( 3 ).toInt() );

#ifndef QTOPIA_CONTENT_INSTALLER
    QContentEngine *engine = (drmState == QContent::Unprotected)
            ? new QFSContentEngine
            : new QDrmContentEngine;
#else
    QContentEngine *engine = new QFSContentEngine;
#endif

    setId( QContentId( dbId, query.value( 0 ).toULongLong() ), engine );

    engine->setName( query.value( 1 ).toString() );
    engine->setMimeType( QMimeType::fromId( query.value( 2 ).toString() ) );
    engine->setFileName( constructFileName( 
            query.value( 5 ).toString(),
            query.value( 6 ).toString() ) );
    engine->setRole( convertRole( query.value( 4 ).toString() ) );
    setDrmState( drmState, engine );
    engine->setIconName( query.value( 7 ).toString() );
    setLastUpdated( QDateTime::fromTime_t( query.value( 8 ).toInt() ), engine );

    return QContent( engine );
}

/*!
    Composes a full file path from the \a file and \a directory portions.
*/
QString QSqlContentStore::constructFileName( const QString &file, const QString &directory ) const
{
    if( directory.isEmpty() )
        return file;
    else
        return directory + '/' + file;
}

/*!
    Converts from the DRM \a flags data stored in the database to the QContent::DrmState enumeration.
*/
QContent::DrmState QSqlContentStore::convertDrmState( int flags ) const
{
    return (flags == 32768) ? QContent::Protected : QContent::Unprotected;
}

/*!
    Converts the QContent::DrmState enumeration \a state the flags data stored in the database.
*/
int QSqlContentStore::convertDrmState( QContent::DrmState state ) const
{
    return (state == QContent::Protected) ? 32768 : 65536;
}

/*!
    Coverts a \a role character from the database to the QContent::Role enumeration.
*/
QContent::Role QSqlContentStore::convertRole( const QString &role ) const
{
    if( role == "a" )
        return QContent::Application;
    else if( role == "d" )
        return QContent::Document;
    else if( role == "f" )
        return QContent::Folder;
    else
        return QContent::Data;
}

/*!
    Converts a QContent::Role enumeration \a role to a character for storing in the database.
*/
QString QSqlContentStore::convertRole( QContent::Role role ) const
{
    switch( role )
    {
    case QContent::Application:
        return QLatin1String( "a" );
    case QContent::Document:
        return QLatin1String( "d" );
    case QContent::Data:
        return QLatin1String( "b" );
    case QContent::Folder:
        return QLatin1String( "f" );
    default:
        return QLatin1String( "u" );
    }
}

/*!
    Returns the primary key for the mime type \a type in the database with the id \a dbId.
*/
int QSqlContentStore::mimeId( const QString &type, QtopiaDatabaseId dbId )
{
    int key = QContentCache::instance()->lookupMimeTypeKey( dbId, type );

    if( key != -1 )
        return key;

    key = queryMimeId( type, dbId );

    if( key != -1 )
        return key;

    static const QString insertString = QLatin1String(
            "insert into mimeTypeLookup( mimeType ) "
            "values( :type )" );

    QSqlQuery insertQuery( QtopiaSql::instance()->database( dbId ) );
    insertQuery.prepare( insertString );

    insertQuery.bindValue( QLatin1String( ":type" ), type );

    QtopiaSql::instance()->logQuery( insertQuery );

    if( !insertQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "insertQuery", dbId, insertQuery.lastError() );

        return 0;
    }

    key = insertQuery.lastInsertId().toInt();

    QContentCache::instance()->cacheMimeTypeKey( dbId, type, key );

    return key;
}

/*!
    Queries the database for the primary key for a mime \a type in the database with the id \a dbId.

    Returns the primary key if one exists and -1 otherwise.
*/
int QSqlContentStore::queryMimeId( const QString &type, QtopiaDatabaseId dbId )
{
    static const QString selectString = QLatin1String(
            "select pKey "
            "from mimeTypeLookup "
            "where mimeType = :type" );

    QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );
    selectQuery.prepare( selectString );

    selectQuery.bindValue( QLatin1String( ":type" ), type );

    QtopiaSql::instance()->logQuery( selectQuery );

    if( !selectQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );

        return -1;
    }

    if( selectQuery.first() )
    {
        int key = selectQuery.value( 0 ).toInt();

        QContentCache::instance()->cacheMimeTypeKey( dbId, type, key );

        return key;
    }

    return -1;
}

/*!
    Returns the primary key for the directory \a location in the database with the id \a dbId.
 */
int QSqlContentStore::locationId( const QString &location, QtopiaDatabaseId dbId )
{
    int key = QContentCache::instance()->lookupLocationKey( dbId, location );

    if( key != -1 )
        return key;

    key = queryLocationId( location, dbId );

    if( key != -1 )
        return key;

    static const QString insertString = QLatin1String(
            "insert into locationLookup( location ) "
            "values( :location )" );

    QSqlQuery insertQuery( QtopiaSql::instance()->database( dbId ) );
    insertQuery.prepare( insertString );

    insertQuery.bindValue( QLatin1String( ":location" ), location );

    QtopiaSql::instance()->logQuery( insertQuery );

    if( !insertQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "insertQuery", dbId, insertQuery.lastError() );

        return -1;
    }

    key = insertQuery.lastInsertId().toInt();

    QContentCache::instance()->cacheLocationKey( dbId, location, key );

    return key;
}

/*!
    Queries the database for the primary key for the directory \a location in the database with the id \a dbId.

    Returns the primary key if one exists or -1 otherwise.
*/
int QSqlContentStore::queryLocationId( const QString &location, QtopiaDatabaseId dbId )
{
    static const QString selectString = QLatin1String(
            "select pKey "
            "from locationLookup "
            "where location = :location" );

    QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );
    selectQuery.prepare( selectString );

    selectQuery.bindValue( QLatin1String( ":location" ), location );

    QtopiaSql::instance()->logQuery( selectQuery );

    if( !selectQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );

        return -1;
    }

    if( selectQuery.first() )
    {
        int key = selectQuery.value( 0 ).toInt();

        QContentCache::instance()->cacheLocationKey( dbId, location, key );

        return key;
    }

    return -1;
}

/*!
    Associates the content with the id \a id with the category ids \a categories in the database.
*/
bool QSqlContentStore::insertCategories( QContentId id, const QStringList &categories )
{
    static const QString insertString = QLatin1String(
            "insert into mapCategoryToContent( cid, categoryid ) "
            "values( :cid, :categoryid )" );

    QSqlQuery insertQuery( QtopiaSql::instance()->database( id.first ) );
    insertQuery.prepare( insertString );

    bool success = true;

    int i = -1;

    foreach( QString category, categories )
    {
        if( ( i == -1 || categories.lastIndexOf( category, i ) == -1 ) && syncCategory( id.first, category ) )
        {
            insertQuery.bindValue( QLatin1String( ":cid" ), id.second );
            insertQuery.bindValue( QLatin1String( ":categoryid" ), category );

            if( !insertQuery.exec() )
            {
                QtopiaSql::instance()->logQuery( insertQuery );

                logError( __PRETTY_FUNCTION__, "insertQuery", id.first,
                        insertQuery.lastError() );

                success = false;
            }
        }
        i++;
    }

    return success;
}

/*!
    Writes the properties from the content engine \a engine to the database associated with the content with
    the id \a id.
*/
bool QSqlContentStore::insertProperties( QContentId id, const QContentEngine &engine )
{
    static const QString insertString = QLatin1String(
            "insert into contentProps( cid, grp, name, value ) "
            "values( :cid, :grp, :name, :value )" );

    QSqlQuery insertQuery( QtopiaSql::instance()->database( id.first ) );
    insertQuery.prepare( insertString );

    bool success = true;

    foreach( QString group, engine.propertyGroups() )
    {
        foreach( QString key, engine.propertyKeys( group ) )
        {
            insertQuery.bindValue( QLatin1String( ":cid" ), id.second );
            insertQuery.bindValue( QLatin1String( ":grp" ), group );
            insertQuery.bindValue( QLatin1String( ":name" ), key );
            insertQuery.bindValue( QLatin1String( ":value" ), engine.property( group, key ) );

            if( !insertQuery.exec() )
            {
                QtopiaSql::instance()->logQuery( insertQuery );

                logError( __PRETTY_FUNCTION__, "insertQuery", id.first,
                          insertQuery.lastError() );

                success = false;
            }
        }
    }

    return success;
}

/*!
    Disassociates the content with the id \a id from all categories in the database.
*/
bool QSqlContentStore::removeCategories( QContentId id )
{
    static const QString deleteString = QLatin1String(
            "delete from mapCategoryToContent "
            "where cid = :cid" );

    QSqlQuery deleteQuery( QtopiaSql::instance()->database(  id.first ) );
    deleteQuery.prepare( deleteString );

    deleteQuery.bindValue( QLatin1String( "cid" ), id.second );

    QtopiaSql::instance()->logQuery( deleteQuery );

    if( !deleteQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "deleteQuery", id.first, deleteQuery.lastError() );

        return false;
    }
    else
        return true;
}

/*!
    Removes all properties associated with the content with the id \a id from the database.
*/
bool QSqlContentStore::removeProperties( QContentId id )
{
    static const QString deleteString = QLatin1String(
            "delete from contentProps "
            "where cid = :cid" );

    QSqlQuery deleteQuery( QtopiaSql::instance()->database( id.first ) );
    deleteQuery.prepare( deleteString );

    deleteQuery.bindValue( QLatin1String( ":cid" ), id.second );

    QtopiaSql::instance()->logQuery( deleteQuery );

    if( !deleteQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "deleteQuery", id.first, deleteQuery.lastError() );

        return false;
    }
    else
        return true;
}

/*!
    Queries the database for a content record with the link file \a fileName.
*/
QContent QSqlContentStore::desktopContent( const QString &fileName )
{
    static const QString selectString = QLatin1String(
            "SELECT content.cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
                   "locationLookup.location as directory, icon, lastUpdated "
            "FROM content left join locationLookup on content.location = locationLookup.pKey "
                         "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
                         "left join contentProps on content.cid = contentProps.cid "
                                                "and contentProps.grp = :group "
                                                "and contentProps.name = :key "
            "WHERE contentProps.value = :linkFile" );

    QtopiaDatabaseId dbId = QtopiaSql::instance()->databaseIdForPath( fileName );

    QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );
    selectQuery.prepare( selectString );

    selectQuery.bindValue( QLatin1String( ":group" ), QLatin1String( "dotdesktop" ) );
    selectQuery.bindValue( QLatin1String( ":key" ), QLatin1String( "linkFile" ) );
    selectQuery.bindValue( QLatin1String( ":linkFile" ), fileName );

    QtopiaSql::instance()->logQuery( selectQuery );

    if ( !selectQuery.exec() )
    {
        logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );

        return QContent();
    }

    if( selectQuery.first() )
    {
        QContent content = contentFromQuery( selectQuery, dbId );

        QContentCache::instance()->cache( content );

        return content;
    }
    else
        return QContent();

}

QContent QSqlContentStore::executableContent( const QString &fileName )
{
    static const QString selectString = QLatin1String(
            "SELECT cid, uiName, mimeTypeLookup.mimeType, drmFlags, docStatus, path, "
            "locationLookup.location as directory, icon, lastUpdated "
            "FROM content left join locationLookup on content.location = locationLookup.pKey "
            "left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
            "WHERE path = :path and locationLookup.location is null" );

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );
        selectQuery.prepare( selectString );

        selectQuery.bindValue( QLatin1String( ":path" ), fileName );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );
        }
        else if( selectQuery.first() )
        {
            QContent content = contentFromQuery( selectQuery, dbId );

            QContentCache::instance()->cache( content );

            return content;
        }
    }

    return QContent();
}

/*!
    Writes an sql \a error that occurred at the given \a location to debug output and sets the content store error string.
*/
void QSqlContentStore::logError( const char *signature, const char *query, const QContent &content, QtopiaDatabaseId databaseId, const QSqlError &error )
{
    qWarning() << signature << databaseId << query;
    qWarning() << error.text();
    qWarning() << content;

    setErrorString( error.text() );
}

/*!
    Writes an sql \a error that occurred at the given \a location to debug output and sets the content store error string.
*/
void QSqlContentStore::logError( const char *signature, const char *query, const QContentEngine &content, QtopiaDatabaseId databaseId, const QSqlError &error )
{
    qWarning() << signature << databaseId << query;
    qWarning() << error.text();
    qWarning() << content;

    setErrorString( error.text() );
}

/*!
    Writes an sql \a error that occurred at the given \a location to debug output and sets the content store error string.
*/
void QSqlContentStore::logError( const char *signature, const char *query, QtopiaDatabaseId databaseId, const QSqlError &error )
{
    qWarning() << signature << databaseId << query;
    qWarning() << error.text();

    setErrorString( error.text() );
}

/*!
    Writes an \a error that occurred at the given \a location to debug output and sets the content store error string.
 */
void QSqlContentStore::logError( const char *signature, const QString &error )
{
    qWarning() << signature;
    qWarning() << error;

    setErrorString( error );
}

/*!
    Ensures that the category with the id \a categoryId exists in the database with the id \a databaseId.

    Returns true if the category exists in the database on completion; false otherwise.
*/
bool QSqlContentStore::syncCategory( QtopiaDatabaseId databaseId, const QString &categoryId )
{
    static const QString selectString = QLatin1String(
            "select categorytext, categoryscope, categoryicon, flags "
            "from categories "
            "where categoryid = :categoryId" );

    {
        QSqlQuery selectQuery( QtopiaSql::instance()->database( databaseId ) );

        selectQuery.prepare( selectString );
        selectQuery.bindValue( QLatin1String( ":categoryId" ), categoryId );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", databaseId, selectQuery.lastError() );

            return false;
        }
        else if( selectQuery.first() )
            return true;
    }

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        if( dbId == databaseId )
            continue;

        QSqlQuery selectQuery( QtopiaSql::instance()->database( dbId ) );

        selectQuery.prepare( selectString );
        selectQuery.bindValue( QLatin1String(":categoryId"), categoryId );

        QtopiaSql::instance()->logQuery( selectQuery );

        if( !selectQuery.exec() )
        {
            logError( __PRETTY_FUNCTION__, "selectQuery", dbId, selectQuery.lastError() );
        }
        else if( selectQuery.first() )
        {
            QSqlRecord record = selectQuery.record();

            static const QString insertString = QLatin1String(
                    "insert into categories ( categoryid, categorytext, categoryscope, categoryicon, flags ) "
                    "values ( :categoryid, :categorytext, :categoryscope, :categoryicon, :flags )" );

            QSqlQuery insertQuery( QtopiaSql::instance()->database( databaseId ) );

            insertQuery.prepare( insertString );

            insertQuery.bindValue( QLatin1String( ":categoryid" ), categoryId );
            insertQuery.bindValue( QLatin1String( ":categorytext" ), record.value( 0 ).toString() );
            insertQuery.bindValue( QLatin1String( ":categoryscope" ), record.value( 1 ).toString() );
            insertQuery.bindValue( QLatin1String( ":categoryicon" ), record.value( 2 ).toString() );
            insertQuery.bindValue( QLatin1String( ":flags" ), record.value( 3 ).toString() );

            if( !insertQuery.exec() )
            {
                logError( __PRETTY_FUNCTION__, "insertQuery", dbId, insertQuery.lastError() );

                return false;
            }
            else
                return true;
        }
    }

#ifndef QTOPIA_CONTENT_INSTALLER
    qLog(DocAPI)
#else
    qWarning()
#endif
        << "Attempted to assign a category id that doesn't exist in any database" << categoryId;

    return false;
}

/*!
    Constructs a where clause for an SQL query from a content \a filter.

    A list of parameters to bind to the final query is appended to \a parameters.

    Any joins that must be made against the content table are appended to \a joins.  The position to insert parameters for joins
    is maintained in \a insertAt.
*/
QString QSqlContentStore::buildWhereClause( const QContentFilter &filter, QList< Parameter > *parameters, int *insertAt, QStringList *joins )
{
    // build up the WHERE clause
    QString operandConjunct = filter.operand() == QContentFilter::Or ? QLatin1String( " OR " ) : QLatin1String( " AND " );
    QString conjunct = QLatin1String(" ");
    QString qry;
    QString part;

    part = buildQtopiaTypes( filter.arguments( QContentFilter::Role ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildPaths( filter.arguments( QContentFilter::Location ), filter.arguments( QContentFilter::Directory ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildMimeTypes( filter.arguments( QContentFilter::MimeType ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildCategories( filter.arguments( QContentFilter::Category ), operandConjunct, parameters, insertAt, joins );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildSynthetic( filter.arguments( QContentFilter::Synthetic ), operandConjunct, parameters, insertAt, joins );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;
        conjunct = operandConjunct;
    }

    part = buildFileNames( filter.arguments( QContentFilter::FileName ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;

        conjunct = operandConjunct;
    }

    part = buildNames( filter.arguments( QContentFilter::Name ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;

        conjunct = operandConjunct;
    }

    part = buildDrm( filter.arguments( QContentFilter::DRM ), operandConjunct, parameters );
    if( !part.isEmpty() )
    {
        qry += conjunct + part;

        conjunct = operandConjunct;
    }

    foreach( QContentFilter subFilter, filter.subFilters() )
    {
        QString subQuery = buildWhereClause( subFilter, parameters, insertAt, joins );

        if( !subQuery.isEmpty() )
        {
            qry += conjunct + QLatin1String( "(" ) + subQuery + QLatin1String( ")" );

            conjunct = operandConjunct;
        }
    }

    if( filter.negated() )
        return QLatin1String( "NOT " ) + qry;
    else
        return qry;
}

/*!
    Constructs a where clause fragment which matches the QContent::Role attribute of content against \a types.

    Each comparison is joined using the conjuct string \a conjunct, and the parameters to bind are appended to \a parameters.
*/
QString QSqlContentStore::buildQtopiaTypes( const QStringList &types, const QString &conjunct, QList< Parameter > *parameters )
{
    if (types.isEmpty())
        return QString::null;

    QString expression;
    QString c;

    foreach( QString type, types )
    {
        QString value;

        if ( type == QLatin1String("Document") )
            value = QLatin1String( "d" );
        else if ( type == QLatin1String("Data") )
            value = QLatin1String( "b" );
        else if( type == QLatin1String( "Folder" ) )
            value = QLatin1String( "f" );
        else
            value = QLatin1String( "a" );

        expression += c + QString( "docStatus = %1" ).arg( addParameter( value, parameters ) );

        c = conjunct;
    }

    if( types.size() == 1 )
        return expression;
    else
        return QString( "(%1)" ).arg( expression );
}

/*!
    Constructs a where clause fragment which matches the path attribute of content against \a locations and \a directories.

    Locations are matched if the content is in the immediate directory or any of it's sub-directories, directories are
    matched only if content is in the immediate directory.

    Each comparison is joined using the conjuct string \a conjunct, and the parameters to bind are appended to \a parameters.
 */
QString QSqlContentStore::buildPaths( const QStringList &locations, const QStringList &directories, const QString &conjunct, QList< Parameter > *parameters )
{
    if (locations.isEmpty() && directories.isEmpty())
        return QString();

    QString expression;
    QString bracketedConjunct = QString( ")%1(" ).arg( conjunct );
    QString c;

    foreach( QString location, locations )
    {
        location = QDir::cleanPath( location );

        if( location.contains( '*' ) )
        {
            expression += c + QString( "locationLookup.location GLOB %1" )
                    .arg( addParameter( location, parameters ) );

            c = bracketedConjunct;
        }
        else
        {
            if( location.endsWith( '/' ) )
                location.chop( 1 );

            QString likeParam = addParameter( location + QLatin1String( "/*" ), parameters );
            QString equalParam = addParameter( location, parameters );

            expression += c + QString(
                    "locationLookup.location GLOB %1 OR locationLookup.location = %2" )
                    .arg( likeParam  )
                    .arg( equalParam );

            c = bracketedConjunct;
        }
    }

    foreach( QString directory, directories )
    {
        directory = QDir::cleanPath( directory );

        if( directory.contains( '*' ) )
        {
            expression += c + QString( "locationLookup.location GLOB %1" )
                    .arg( addParameter( directory, parameters ) );

            c = bracketedConjunct;
        }
        else
        {
            if( directory.endsWith( '/' ) )
                directory.chop( 1 );

            expression += c + QString( "locationLookup.location = %1" )
                    .arg( addParameter( directory, parameters ) );

            c = bracketedConjunct;
        }
    }

    return !expression.isEmpty() ? QString( "(%1)" ).arg( expression ) : QString();
}

/*!
    Constructs a where clause fragment which matches the mime type attribute of content against \a mimes.

    Each comparison is joined using the conjuct string \a conjunct, and the parameters to bind are appended to \a parameters.
 */
QString QSqlContentStore::buildMimeTypes( const QStringList &mimes, const QString &conjunct, QList< Parameter > *parameters )
{
    if( mimes.isEmpty() )
        return QString();

    QString expression;
    QString c;

    foreach( QString mime, mimes )
    {
        if( mime.contains( '*' ) )
        {
            mime.replace( '*', '%' );

            expression += c + QString( "mimeTypeLookup.mimeType LIKE %1" )
                    .arg( addParameter( mime, parameters ) );
        }
        else
        {
            expression += c + QString( "mimeTypeLookup.mimeType = %1" )
                    .arg( addParameter( mime, parameters ) );
        }

        c = conjunct;
    }

    return !expression.isEmpty() ? QString( "(%1)" ).arg( expression ) : QString();
}

/*!
    Constructs a where clause fragment which matches the content categories against \a categories.

    Any table joins required by the query are appended to \a joins.

    Each comparison is joined using the conjuct string \a conjunct, and the parameters to bind are appended to \a parameters.
 */
QString QSqlContentStore::buildCategories( const QStringList &categories, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins )
{
    QString expression;
    QString c;

    foreach( QString category, categories )
    {
        QString table = QString( "cat%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

        if( category == QLatin1String( "Unfiled" ) )
        {
            joins->append( QString( " left join mapCategoryToContent as %1 on %1.cid = content.cid" ).arg( table ) );

            expression += c + QString( "%1.categoryid is NULL" ).arg( table );
        }
        else if( category.contains( "*" ) )
        {
            category.replace(QLatin1Char('*'), QLatin1Char('%'));

            joins->append( QString( " left join mapCategoryToContent as %1 on %1.cid = content.cid and %1.categoryid like %2" )
                    .arg( table )
                    .arg( addParameter( category, parameters, insertAt ) ) );

            expression += c + QString( "%1.categoryid is not NULL" ).arg( table );
        }
        else
        {
            joins->append( QString( " left join mapCategoryToContent as %1 on %1.cid = content.cid and %1.categoryid = %2" )
                    .arg( table )
                    .arg( addParameter( category, parameters, insertAt ) ) );

            expression += c + QString( "%1.categoryid is not NULL" ).arg( table );
        }

        c = conjunct;
    }

    return expression;
}

/*!
    Constructs a where clause fragment which matches content properties against the properties in \a synthetic.

    Each entry in synthetic is a property group, key and value concatenated with forward slashes.

    Any table joins required by the query are appended to \a joins and the \a insertAt index updated.

    Each comparison is joined using the conjuct string \a conjunct, and the parameters to bind are appended to \a parameters.
 */
QString QSqlContentStore::buildSynthetic( const QStringList &synthetic, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins )
{
    QString expression;
    QString c;

    foreach( QString filter, synthetic )
    {
        QString group = filter.section( '/', 0, 0 );
        QString key   = filter.section( '/', 1, 1 );
        QString value = filter.section( '/', 2 );

        QString table = QString( "prop%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

        QString groupParam = addParameter( group, parameters, insertAt );
        QString keyParam   = addParameter( key  , parameters, insertAt );

        QString join = QString( " left join contentProps as %1 on content.cid = %1.cid and %1.grp = %2 and %1.name = %3" )
                .arg( table  )
                .arg( groupParam )
                .arg( keyParam );

        joins->append( join );

        if( value.isEmpty() )
        {
            expression += c + QString( "(%1.value is null)" )
                    .arg( table );
        }
        else if( value.contains( '*' ) )
        {
            value.replace( '*', '%' );

            expression += c + QString( "(%1.value like %2 and %1.value not null)" )
                    .arg( table )
                    .arg( addParameter( value, parameters ) );
        }
        else
        {
            expression += c + QString( "(%1.value = %2 and %1.value not null)" )
                    .arg( table )
                    .arg( addParameter( value, parameters ) );
        }

        c = conjunct;
    }

    return expression;
}

/*!
    Constructs a where clause fragment which matches the drm state attribute of content against the values of \a drm.

    Each comparison is joined using the conjuct string \a conjunct, and the parameters to bind are appended to \a parameters.
 */
QString QSqlContentStore::buildDrm( const QStringList &drm, const QString &conjunct, QList< Parameter > *parameters )
{
    QString expression;
    QString c;

    foreach( QString filter, drm )
    {
        if( filter == QLatin1String( "Protected" ) )
        {
            expression += c + QString( "drmFlags != %1" )
                    .arg( addParameter( QVariant( 65536 ), parameters ) );

            c = conjunct;
        }
        else if( filter == QLatin1String( "Unprotected" ) )
        {
            expression += c + QString( "drmFlags == %1" )
                    .arg( addParameter( QVariant( 65536 ), parameters ) );

            c = conjunct;
        }
    }

    return expression;
}

/*!
    Construncts a where clause fragment which matches the fileName (not including the path) of content against the 
    values of \a fileNames.

    Each comparison is joined using the conjunct string \a conjunct, and the parameters to bind are appended to \a parameters.
*/
QString QSqlContentStore::buildFileNames( const QStringList &fileNames, const QString &conjunct, QList< Parameter > *parameters )
{
    QString expression;
    QString c;

    foreach( QString filter, fileNames )
    {
        if( filter.contains( '*' ) )
        {
            filter.replace( '*', '%' );

            expression += c + QString( "path like %1" )
                    .arg( addParameter( filter, parameters ) );

            c = conjunct;
        }
        else
        {
            expression += c + QString( "path = %1" )
                    .arg( addParameter( filter, parameters ) );

            c = conjunct;
        }
    }

    return expression;
}

/*!
    Construncts a where clause fragment which matches the visible name of content against the 
    values of \a names.

    Each comparison is joined using the conjunct string \a conjunct, and the parameters to bind are appended to \a parameters.
 */
QString QSqlContentStore::buildNames( const QStringList &names, const QString &conjunct, QList< Parameter > *parameters )
{
    QString expression;
    QString c;

    foreach( QString filter, names )
    {
        if( filter.contains( '*' ) )
        {
            filter.replace( '*', '%' );

            expression += c + QString( "uiName like %1" )
                    .arg( addParameter( filter, parameters ) );

            c = conjunct;
        }
        else
        {
            expression += c + QString( "uiName = %1" )
                    .arg( addParameter( filter, parameters ) );

            c = conjunct;
        }
    }

    return expression;
}

/*!
    Constructs an SQL order by clause for sorting content in the order given in \a sortList.

    Any table joins required by the query are appended to \a joins and the \a insertAt index updated and  any
    the parameters that must be bound are appended to \a parameters.
*/
QString QSqlContentStore::buildOrderBy( const QContentSortCriteria &order, QList< Parameter > *parameters, int *insertAt, QStringList *joins  )
{
    QString result;
    QString conjunct;

    for( int i = 0; i < order.sortCount(); i++ )
    {
        switch( order.attribute( i ) )
        {
            case QContentSortCriteria::Name:
                result += conjunct + QLatin1String( "uiNameSortOrder" );
                break;
        case QContentSortCriteria::MimeType:
            {
                QString table = QString( "sort%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

                joins->append( QString( " left join mimeTypeLookup as %1 on content.mType = %1.pKey" ).arg( table ) );

                result += conjunct + QString( "%1.mimeType" ).arg( table );
            }
        case QContentSortCriteria::Property:
            {
                QString table = QString( "sort%1" ).arg( joins->count(), 3, 10, QLatin1Char( '0' ) );

                QString groupParam = addParameter( order.scope( i ).section( '/', 0, 0 ), parameters, insertAt );
                QString keyParam   = addParameter( order.scope( i ).section( '/', 1, 1 ), parameters, insertAt );

                QString join = QString(
                        " left join contentProps as %1 on content.cid = %1.cid"
                        " and %1.grp = %2 and %1.name = %3" )
                        .arg( table )
                        .arg( groupParam )
                        .arg( keyParam );

                joins->append( join );

                result += conjunct + QString( "%1.value" ).arg( table );
            }
            break;
        case QContentSortCriteria::FileName:
            result += conjunct + QLatin1String( "path COLLATE BINARY" );
            break;
        case QContentSortCriteria::LastUpdated:
            result += conjunct + QLatin1String( "lastUpdated" );
            break;
        default:
            continue;
        }

        if( order.order( i ) == Qt::DescendingOrder )
            result += QLatin1String( " DESC" );

        conjunct = QLatin1String( ", " );
    }

    if( result.isEmpty() )
        return QLatin1String( " ORDER BY uiNameSortOrder" );
    else
        return QLatin1String( " ORDER BY " ) + result;
}

/*!
    Constructs an SQL from clause for the filter \a filter and any additional joins \a joins.
*/
QString QSqlContentStore::buildFrom( const QContentFilter &filter, const QStringList &joins )
{
    QString from = QLatin1String( "content" );

    QSet< QContentFilter::FilterType > types = getAllFilterTypes( filter );

    if( types.contains( QContentFilter::Directory ) || types.contains( QContentFilter::Location ) )
        from += QLatin1String(
                " left join locationLookup on content.location = locationLookup.pKey" );

    if( types.contains( QContentFilter::MimeType ) )
        from += QLatin1String( " left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey" );

    foreach( QString join, joins )
        from += join;

    return from;
}

/*!
    Constructs a query based on the template \a queryTemplate the results of which are constrained by the filter \a filter.

    Any parameters that should be bound when executing the query are added to \a parameters.
*/
QString QSqlContentStore::buildQuery( const QString &queryTemplate, const QContentFilter &filter, QList< Parameter > *parameters )
{
    if( !filter.isValid() )
        return QString();

    int insertAt = 0;
    QStringList joins;

    QString where = buildWhereClause( filter, parameters, &insertAt, &joins );
    QString from = buildFrom( filter, joins );

    return queryTemplate.arg( from ).arg( where );
}

/*!
    Constructs a query based on the template \a queryTemplate the results of which are constrained by the filter \a filter
    and sorted according to \a sortOrder.

    Any parameters that should be bound when executing the query are added to \a parameters.
 */
QString QSqlContentStore::buildQuery( const QString &queryTemplate, const QContentFilter &filter, const QContentSortCriteria &sortOrder,  QList< Parameter > *parameters )
{
    if( !filter.isValid() )
        return QString();

    int insertAt = 0;
    QStringList joins;

    QString where = buildWhereClause( filter, parameters, &insertAt, &joins );
    QString order = buildOrderBy( sortOrder, parameters, &insertAt, &joins );
    QString from = buildFrom( filter, joins );

    return queryTemplate.arg( from ).arg( where );
}

/*!
    Binds the parameters in \a parameters to a \a query.
*/
void QSqlContentStore::bindParameters( QSqlQuery *query, const QList< Parameter > &parameters )
{
    foreach( const Parameter parameter, parameters )
        query->bindValue( parameter.first, parameter.second );
}

/*!
    Adds a \a parameter value to a list of parameters.
*/
QString QSqlContentStore::addParameter( const QVariant &parameter, QList< Parameter > *parameters )
{
    QString id = QString( ":param%1" ).arg( parameters->count(), 3, 10, QLatin1Char( '0' ) );

    parameters->append( Parameter( id, parameter ) );

    return id;
}

/*!
    Inserts a \a parameter value into a list of parameters at the index \a insertAt and increments the index.
 */
QString QSqlContentStore::addParameter( const QVariant &parameter, QList< Parameter > *parameters, int *insertAt )
{
    QString id = QString( ":param%1" ).arg( parameters->count(), 3, 10, QLatin1Char( '0' ) );

    parameters->insert( (*insertAt)++, Parameter( id, parameter ) );

    return id;
}

/*!
    Returns a set of all filter types in a \a filter.
*/
QSet< QContentFilter::FilterType > QSqlContentStore::getAllFilterTypes( const QContentFilter &filter )
{
    QSet< QContentFilter::FilterType > types = filter.types().toSet();

    foreach( QContentFilter f, filter.subFilters() )
        types.unite( getAllFilterTypes( f ) );

    return types;
}

/*!
    Returns a set of all synthetic keys in a \a filter.
*/
QSet< QString > QSqlContentStore::getAllSyntheticKeys( const QContentFilter &filter )
{
    QSet< QString > keys;

    QStringList syntheticFilters = filter.arguments( QContentFilter::Synthetic );

    foreach( QString syntheticFilter, syntheticFilters )
        keys.insert( syntheticFilter.section( '/', 0, 1 ) );

    foreach( QContentFilter f, filter.subFilters() )
        keys.unite( getAllSyntheticKeys( f ) );

    return keys;
}

/*!
    Returns a list of all mime types that match a \a filter and optionally match the mime major type \a subType.
*/
QStringList QSqlContentStore::mimeFilterMatches( const QContentFilter &filter, const QString &subType )
{
    QContentFilter subFilter = filter;

    if( !subType.isEmpty() )
        subFilter &= QContentFilter( QContentFilter::MimeType, subType + QLatin1String( "/*" ) );

    QString queryString;
    QList< Parameter > parameters;

    if( subFilter.isValid() )
    {
        QString mimeQuery;

        if( getAllFilterTypes( subFilter ).contains( QContentFilter::MimeType ) )
        {
            mimeQuery = QLatin1String(
                    "select distinct mimeTypeLookup.mimeType "
                    "from %1 "
                    "where %2 "
                    "order by mimeTypeLookup.mimeType" );
        }
        else
        {
            mimeQuery = QLatin1String(
                    "select distinct mimeTypeLookup.mimeType "
                    "from %1 left join mimeTypeLookup on content.mType = mimeTypeLookup.pKey "
                    "where %2 "
                    "order by mimeTypeLookup.mimeType" );
        }

        queryString = buildQuery( mimeQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct mimeType "
                "from mimeTypeLookup "
                "order by mimeType" );
    }

    QMap< QString, QString > filters;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        QSqlQuery query( QtopiaSql::instance()->database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::instance()->logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
            {
                QString f = qvariant_cast< QString >( query.value(0) );

                if( !f.isEmpty() )
                    filters.insert( f, f );
            }
        }
        else
        {
            qLog(DocAPI) << "mimeFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( filters.values() );
}

/*!
    Returns a list of all content property groups that match the filter \a filter.
*/
QStringList QSqlContentStore::syntheticFilterGroups( const QContentFilter &filter )
{
    QString queryString;
    QList< Parameter > parameters;

    if( filter.isValid() )
    {
        static const QString groupQuery = QLatin1String(
                "select distinct contentProps.grp "
                "from %1 left join contentProps on content.cid = contentProps.cid "
                "where %2 "
                "order by contentProps.grp" );

        queryString = buildQuery( groupQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct grp "
                "from contentProps "
                "order by grp" );
    }

    QMap< QString, QString > groups;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        QSqlQuery query( QtopiaSql::instance()->database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::instance()->logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
            {
                QString group = qvariant_cast< QString >( query.value(0) );

                if( !group.isEmpty() )
                    groups.insert( group, group );
            }
        }
        else
        {
            qLog(DocAPI) << "syntheticFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( groups.values() );
}

/*!
    Returns a list of all content property keys in the property group \a group that match the filter \a filter.
*/
QStringList QSqlContentStore::syntheticFilterKeys( const QContentFilter &filter, const QString &group )
{
    QString queryString;
    QList< Parameter > parameters;

    if( filter.isValid() )
    {
        QString keyQuery = QLatin1String(
                "select distinct contentProps.name "
                "from %2 left join contentProps on content.cid = contentProps.cid and contentProps.grp = :group "
                "where %3 "
                "order by contentProps.name" );

        parameters.append( Parameter( QLatin1String( "group" ), group ) );

        queryString = buildQuery( keyQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct name "
                "from contentProps "
                "where grp = :group" );
    }

    QMap< QString, QString > keys;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        QSqlQuery query( QtopiaSql::instance()->database( dbId ) );
        query.prepare( queryString );

        query.bindValue( QLatin1String( "group" ), group );

        bindParameters( &query, parameters );

        QtopiaSql::instance()->logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
            {
                QString key = qvariant_cast< QString >( query.value(0) );

                if( !key.isEmpty() )
                    keys.insert( key, key );
            }
        }
        else
        {
            qLog(DocAPI) << "syntheticFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( keys.values() );
}

/*!
    Returns a list all property values with the property group \a group and key \a key that match the content filter \a filter.
*/
QStringList QSqlContentStore::syntheticFilterMatches( const QContentFilter &filter, const QString &group, const QString &key )
{
    QString queryString;
    QList< Parameter > parameters;

    if( filter.isValid() )
    {
        static const QString valueQuery = QLatin1String(
                "select distinct contentProps.value "
                "from %1 left join contentProps on content.cid = contentProps.cid and contentProps.grp = :group and contentProps.name = :key "
                "where %2 "
                "order by contentProps.value");

        parameters.append( Parameter( QLatin1String( "group" ), group ) );
        parameters.append( Parameter( QLatin1String( "key" ), key ) );

        queryString = buildQuery( valueQuery, filter, &parameters );
    }
    else
    {
        queryString = QLatin1String(
                "select distinct value "
                "from contentProps "
                "where grp = :group and name = :key" );
    }

    QMap< QString, QString > filters;

    QString filterBase = group + '/' + key + '/';

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        QSqlQuery query( QtopiaSql::instance()->database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::instance()->logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
            {
                QString f = qvariant_cast< QString >( query.value(0) );

                filters.insert( f, filterBase + f );
            }
        }
        else
        {
            qLog(DocAPI) << "syntheticFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( filters.values() );
}

/*!
    Returns a list of all category ids in the scope \a scope that match the content filter \a filter.
*/
QStringList QSqlContentStore::categoryFilterMatches( const QContentFilter &filter, const QString &scope )
{
    QString queryString;
    QList< Parameter > parameters;
    QString scopeClause;

    if( !scope.isEmpty() )
    {
        scopeClause = QLatin1String( "categories.categoryscope = :scope" );

        parameters.append( Parameter( QLatin1String( "scope" ), scope ) );
    }
    else
        scopeClause = QLatin1String( "categories.categoryscope is null" );

    if( filter.isValid() )
    {
        queryString = QString(
                "select distinct category.categoryid "
                "from %2 inner join mapCategoryToContent as category on content.cid = category.cid "
                "inner join categories on category.categoryid = categories.categoryid and %1 "
                "where %3 "
                "order by categories.categorytext")
                .arg( scopeClause );

        queryString = buildQuery( queryString, filter, &parameters );
    }
    else
    {
        queryString = QString(
                "select distinct categoryid "
                "from categories "
                "where %1" )
                .arg( scopeClause );
    }

    QMap< QString, QString > filters;

    foreach( QtopiaDatabaseId dbId, QtopiaSql::instance()->databaseIds() )
    {
        QSqlQuery query( QtopiaSql::instance()->database( dbId ) );
        query.prepare( queryString );

        bindParameters( &query, parameters );

        QtopiaSql::instance()->logQuery( query );

        if( query.exec() )
        {
            while( query.next() )
            {
                QString f = qvariant_cast< QString >( query.value(0) );

                filters.insert( f, f );
            }
        }
        else
        {
            qLog(DocAPI) << "categoryFilterMatches query failed";
            qLog(DocAPI) << queryString;
        }
    }

    return QStringList( filters.values() );
}

QStringList QSqlContentStore::directoryFilterMatches( const QContentFilter &filter, const QString &directory )
{
    QString queryString;
    QList< Parameter > parameters;

    if( !filter.isValid() )
    {
        QString param1 = addParameter( directory + QLatin1String( "/%" ), &parameters );
        QString param2 = addParameter( directory + QLatin1String( "/%/%" ), &parameters );
        queryString = QString(
                "select location "
                "from locationLookup "
                "where location like %1 and not location like %2 "
                "order by location" )
                .arg( param1 )
                .arg( param2 );
    }
    else
    {
        QContentFilter f = filter;

        if (!directory.isEmpty()) {
                f &= QContentFilter( QContentFilter::Location, directory )
                    & ~QContentFilter( QContentFilter::Directory, directory + QLatin1String( "/*/*" ) );
        }

        QSet< QContentFilter::FilterType > types = getAllFilterTypes( f );

        QString directoryQuery;

        if( types.contains( QContentFilter::Directory ) || types.contains( QContentFilter::Location ) )
        {
            directoryQuery = QLatin1String(
                    "select distinct locationLookup.location "
                    "from %1 "
                    "where %2 "
                    "order by locationLookup.location" );
        }
        else
        {
            directoryQuery = QLatin1String(
                    "select distinct locationLookup.location "
                    "from %1 left join locationLookup on content.location = locationLookup.pKey "
                    "where %2 "
                    "order by locationLookup.location" );
        }

        queryString = buildQuery( directoryQuery, f, &parameters );
    }

    QStringList directories;

    QSqlQuery query( QtopiaSql::instance()->database( QtopiaSql::instance()->databaseIdForPath( directory ) ) );
    query.prepare( queryString );

    bindParameters( &query, parameters );

    QtopiaSql::instance()->logQuery( query );

    if( query.exec() )
    {
        while( query.next() )
            directories.append( query.value( 0 ).toString() );
    }
    else
    {
        qLog(DocAPI) << "directoryFilterMatches query failed";
        qLog(DocAPI) << queryString;
    }

    return directories;
}

void QSqlContentStore::addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission)
{
    if(mimeType.isEmpty() || application.isEmpty())
        return;
    QString mimeMajor, mimeMinor;
    if(!mimeType.contains('/'))
    {
        mimeMajor = mimeType;
        mimeMinor = '*';
    }
    else
    {
        mimeMajor = mimeType.section('/', 0, 0);
        mimeMinor = mimeType.section('/', 1, 1);
    }
    QSqlQuery qry(QtopiaSql::instance()->systemDatabase());
    qry.prepare("select count(*) from mimeTypeMapping where mimeType = :major and mimeSubType = :minor and application = :app");
    qry.bindValue("major", mimeMajor);
    qry.bindValue("minor", mimeMinor);
    qry.bindValue("app", application);
    QtopiaSql::instance()->logQuery(qry);
    if(qry.exec() && qry.first() && qry.value(0) == 0)
    {
        qry.prepare("insert into mimeTypeMapping(application, mimeType, mimeSubType, icon, drmFlags) VALUES (:application, :mimeType, :mimeSubType, :icon, :drmFlags)");
        qry.bindValue("application", application);
        qry.bindValue("mimeType", mimeMajor);
        qry.bindValue("mimeSubType", mimeMinor);
        qry.bindValue("icon", icon);
        qry.bindValue("drmFlags", permission);
        QtopiaSql::instance()->logQuery(qry);
        if(!qry.exec())
        {
            qLog(DocAPI) << "addAssociation query failed";
            qLog(DocAPI) << qry.lastError();
        }
    }
    // else ignore it, a mapping for this app and mime-type exists.
}

void QSqlContentStore::removeAssociation(const QString& mimeType, const QString& application)
{
    if(mimeType.isEmpty() || application.isEmpty())
        return;
    QString mimeMajor, mimeMinor;
    if(!mimeType.contains('/'))
    {
        mimeMajor = mimeType;
        mimeMinor = '*';
    }
    else
    {
        mimeMajor = mimeType.section('/', 0, 0);
        mimeMinor = mimeType.section('/', 1, 1);
    }
    QSqlQuery qry(QtopiaSql::instance()->systemDatabase());
    qry.prepare("delete from mimeTypeMapping where mimeType = :major and mimeSubType = :minor and application = :app");
    qry.bindValue("major", mimeMajor);
    qry.bindValue("minor", mimeMinor);
    qry.bindValue("app", application);
    QtopiaSql::instance()->logQuery(qry);
    if(!qry.exec())
    {
        qLog(DocAPI) << "removeAssociation query failed";
        qLog(DocAPI) << qry.lastError();
    }
}

void QSqlContentStore::setDefaultApplicationFor(const QString& mimeType, const QString& application)
{
    if(mimeType.isEmpty())
        return;
    QSqlQuery qry(QtopiaSql::instance()->systemDatabase());
    QString mimeMajor, mimeMinor;
    if(!mimeType.contains('/'))
    {
        mimeMajor = mimeType;
        mimeMinor = '*';
    }
    else
    {
        mimeMajor = mimeType.section('/', 0, 0);
        mimeMinor = mimeType.section('/', 1, 1);
    }
    qry.prepare("delete from defaultMimeApplication where mimeType = :major and mimeSubType = :minor");
    qry.bindValue("major", mimeMajor);
    qry.bindValue("minor", mimeMinor);
    QtopiaSql::instance()->logQuery(qry);
    if(!qry.exec())
    {
        qLog(DocAPI) << "setDefaultApplicationFor query failed";
        qLog(DocAPI) << qry.lastError();
    }
    else if(!application.isEmpty())
    {
        qry.prepare("insert into defaultMimeApplication(mimeType, mimeSubType, application) VALUES (:major, :minor, :app)");
        qry.bindValue("major", mimeMajor);
        qry.bindValue("minor", mimeMinor);
        qry.bindValue("app", application );
        QtopiaSql::instance()->logQuery(qry);
        if(!qry.exec())
        {
            qLog(DocAPI) << "setDefaultApplicationFor query failed";
            qLog(DocAPI) << qry.lastError();
        }
    }
}

QList<QMimeEngineData> QSqlContentStore::associationsForApplication(const QString& application )
{
    QList<QMimeEngineData> result;
    if(application.isEmpty())
        return result;
    QList<QtopiaDatabaseId> dbids=QtopiaSql::instance()->databaseIds();
    // set the system database as the last in the list
    dbids.removeAll(0);
    dbids.append(0);
    foreach(QtopiaDatabaseId dbid, dbids)
    {
        QSqlQuery qry(QtopiaSql::instance()->database(dbid));
        qry.prepare("select application, icon, mimeType, mimeSubType, drmFlags from mimeTypeMapping where application = :app");
        qry.bindValue("app", application);
        QtopiaSql::instance()->logQuery(qry);
        if(qry.exec()) {
            while (qry.next()) {
                QMimeEngineData data;
                data.application = qry.value(0).toString();
                data.icon = qry.value(1).toString();
                data.type = qry.value(2).toString() + '/' + qry.value(3).toString();
                data.permission = qvariant_cast<QDrmRights::Permission>(qry.value(4));
                result.append(data);
            }
        }
        else {
            qLog(DocAPI) << "associationsForAppliction query failed";
            qLog(DocAPI) << qry.lastError();
        }
    }
    return result;
}

QList<QMimeEngineData> QSqlContentStore::associationsForMimeType(const QString& mimeType )
{
    QList<QMimeEngineData> result;
    if(mimeType.isEmpty())
        return result;

    QString mimeMajor, mimeMinor;
    if(!mimeType.contains('/')) {
        mimeMajor = mimeType;
        mimeMinor = '*';
    }
    else {
        mimeMajor = mimeType.section('/', 0, 0);
        mimeMinor = mimeType.section('/', 1, 1);
    }

    QList<QtopiaDatabaseId> dbids=QtopiaSql::instance()->databaseIds();
    // set the system database as the last in the list
    dbids.removeAll(0);
    dbids.append(0);
    foreach(QtopiaDatabaseId dbid, dbids) {
        QSqlQuery qry(QtopiaSql::instance()->database(dbid));
        qry.prepare("select application, icon, mimeType, mimeSubType, drmFlags from mimeTypeMapping where mimeType = :major and mimeSubType = minor");
        qry.bindValue("major", mimeMajor);
        qry.bindValue("minor", mimeMinor);
        QtopiaSql::instance()->logQuery(qry);
        if(qry.exec()) {
            while (qry.next()) {
                QMimeEngineData data;
                data.application = qry.value(0).toString();
                data.icon = qry.value(1).toString();
                data.type = qry.value(2).toString() + '/' + qry.value(3).toString();
                data.permission = qvariant_cast<QDrmRights::Permission>(qry.value(4));
                result.append(data);
            }
        }
        else {
            qLog(DocAPI) << "associationsForAppliction query failed";
            qLog(DocAPI) << qry.lastError();
        }
    }
    return result;
}

QString QSqlContentStore::deriveName( const QString &fileName, const QMimeType &type ) const
{
    QString name;
    int slash = fileName.lastIndexOf( '/' );

    if( slash != -1 )
        name = fileName.mid( slash + 1 );
    else
        name = fileName;

    foreach( QString ext, type.extensions() )
    {
        ext.prepend( QLatin1Char( '.' ) );

        if( name.endsWith( ext, Qt::CaseInsensitive ) && name.length() != ext.length() )
        {
            name.chop( ext.length() );
            break;
        }
    }

    return name;
}
