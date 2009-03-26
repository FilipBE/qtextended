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

#include "qcontentstore_p.h"
#include "qsqlcontentstore_p.h"

#include <QtopiaApplication>

#ifndef QTOPIA_CONTENT_INSTALLER
#include "qdocumentservercontentstore_p.h"
#include "contentpluginmanager_p.h"
#include <QApplication>
#include <QThreadStorage>
#include <QImageReader>
#include <QImageWriter>
#include <QCryptographicHash>
#include <QFileSystem>
Q_GLOBAL_STATIC( QThreadStorage< QContentStore * >, contentStores );
#endif

#include <qtopialog.h>
#include <QReadLocker>
#include <QWriteLocker>

/*!
    \class QContentStore
    \inpublicgroup QtBaseModule

    \brief The QContentStore class is an interface to a backing store of content information.

    \internal
*/

/*!
    \enum QContentStore::LookupFlag

    Lookup flags define options for performing a content lookup on a file name.

    \value Lookup The database should be checked for an existing record matching the file name before a new one is constructed.
    \value Commit If a new content record is constructed it should be commited immediately
    \value Construct If a lookup fails or no lookup is performed a new content record for the file name should be constructed.
*/

/*!
    \typedef QContentStore::LookupFlags

    Synonym for \c{QFlags<QcontentStore::LookupFlag>}
*/

/*!
    Constructs a new QContentStore with the parent object \a parent.
*/
QContentStore::QContentStore( QObject *parent )
    : QObject( parent )
    , m_folderThumbnails(QStringList()
            << QLatin1String(".album-art.jpg")
            << QLatin1String(".album-art.png")
            << QLatin1String("folder.jpg"))
    , m_audioPrefix(QLatin1String("audio/"))
{
}

/*!
    Destroys a QContentStore.
*/
QContentStore::~QContentStore()
{
}

/*!
    \fn QContentStore::contentFromId( QContentId contentId )

    Returns a content engine for the content in the store with the id \a contentId.  If no entry exists
    for the id a null pointer will be returned.
*/

/*!
    \fn QContentStore::contentFromFileName( const QString &fileName, LookupFlags lookup )

    Returns a content engine for the existing content with the file name \a fileName.

    Depending on the \a lookup flags a new content record may be created for the file if none already exists.

    If the file is invalid a null pointer will be returned.
*/

/*!
    \fn QContentStore::commitContent( QContent *content )

    Commits changes to \a content to the database.

    Returns true if the commit is successful; false otherwise.
*/

/*!
    \fn QContentStore::removeContent( QContent* content )

    Removes the \a content from the content store.
*/

/*!
    \fn QContentStore::contentCategories( QContentId contentId )

    Returns the category ids for the content with the id \a contentId.
*/

/*!
    \fn QContentStore::contentProperties( QContentId contentId )

    Returns all the properties associated with the content with the id \a contentId.
*/

/*!
    \fn QContentStore::contentMimeTypes( QContentId contentId )

    Returns a list of mime types the content with id \a contentId can execute.
*/

/*!
    \fn QContentStore::mimeTypeFromId( const QString &mimeId )

    Returns the mime type data for the mime type with the id \a mimeId.
*/

#ifndef QTOPIA_CONTENT_INSTALLER

/*!
    Loads a thumbnail representation of \a content.  The thumbnail will be scaled to \a size
    according to the given aspect ratio mode.
*/
QImage QContentStore::thumbnail(const QContent &content, const QSize &size, Qt::AspectRatioMode mode)
{
    QImage thumbnail;

    QString thumbPath = thumbnailPath(content.fileName());

    QFileInfo thumbInfo(thumbPath);

    if (thumbInfo.exists()) {
        if (thumbInfo.lastModified() > content.lastUpdated())
            thumbnail = readThumbnail(thumbPath, size, mode);
    } else {
        thumbnail = QContentFactory::thumbnail(content, size, mode);
    }

    if (thumbnail.isNull()) {
        if (QIODevice *device = content.open()) {
            QImageReader reader(device);

            if (reader.canRead()) {
                QSize scaledSize = reader.size();

                reader.setQuality(25);
                if (scaledSize.width() > 128 || scaledSize.height() > 128) {
                    scaledSize.scale(QSize(128, 128), Qt::KeepAspectRatio);

                    reader.setQuality( 49 ); // Otherwise Qt smooth scales
                    reader.setScaledSize(scaledSize);
                    reader.read(&thumbnail);

                    if (!thumbnail.isNull()) {
                        QImageWriter writer(thumbPath, QByteArray::fromRawData("PNG", 3));
                        writer.setQuality(25);
                        writer.write(thumbnail);

                        if (size.isValid())
                            thumbnail = thumbnail.scaled(size, mode);
                    }
                } else {
                    if (size.isValid()) {
                        scaledSize.scale(size, mode);
                        reader.setQuality( 49 ); // Otherwise Qt smooth scales
                        reader.setScaledSize(scaledSize);
                    }
                    reader.read(&thumbnail);
                }
            }

            delete device;
        }
    }

    if (thumbnail.isNull() && content.type().startsWith(m_audioPrefix)) {
        QDir dir = QFileInfo(content.fileName()).absoluteDir();

        foreach (const QString &fileName, m_folderThumbnails) {
            if (dir.exists(fileName)) {
                thumbnail = readThumbnail(dir.absoluteFilePath(fileName), size, mode);
                break;
            }
        }
    }

    return thumbnail;
}

/*!
    Loads a thumbnail from a thumbnail file with the given \a fileName.  The thumbnail will be
    scaled to \a size according to the given aspect ratio mode.
*/
QImage QContentStore::readThumbnail(const QString &fileName, const QSize &size, Qt::AspectRatioMode mode)
{
    QImage thumbnail;

    QImageReader reader(fileName);

    if (reader.canRead()) {
        if (size.isValid()) {
            QSize scaledSize = reader.size();
            scaledSize.scale(size, mode);
            reader.setQuality( 49 ); // Otherwise Qt smooth scales
            reader.setScaledSize(scaledSize);
        }

        reader.read(&thumbnail);
    }

    return thumbnail;
}

/*!
    Returns the directory to which thumbnails of \a fileName should be saved.
*/
QString QContentStore::thumbnailPath(const QString &fileName) const
{
    const QString thumbnails(QLatin1String("Thumbnails"));

    QFileSystem fs = QFileSystem::fromFileName(fileName);

    if (!fs.storesType(thumbnails))
        fs = QFileSystem::typeFileSystem(thumbnails);

    QDir dir(!fs.isNull() ? fs.typePath(thumbnails) : Qtopia::homePath() + QLatin1String("/Thumbnails"));

    if (!dir.exists())
        dir.mkpath(QLatin1String("."));

    QByteArray hash = QCryptographicHash::hash(
            fileName.toLocal8Bit(), QCryptographicHash::Md5).toHex();

    return dir.absoluteFilePath(QString::fromLatin1(hash.constData(), hash.length()))
            + QLatin1String(".png");
}

#endif

/*!
    \fn QContentStore::contentSet( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode )

    Returns an engine for a content set containing content filtered by the given \a filter and sorted in the given \a order.

    The value of \a mode will determine if the content of the set are update synchronously or asynchronously.
*/

/*!
    \fn QContentStore::contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType )

    Returns a engine for a filter set of filters of type \a type and partial argument \a subType that return content
    matches when and'ed with the filter \a filter.
*/

/*!
    \fn QContentStore::filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType )

    Returns a list of filter arguments for a content filter of type \a type and partial argument \a subType that will return
    content matches when and'ed with the filter \a filter.
*/

/*!
    \fn QContentStore::batchCommitContent( const QContentList &content )

    Performs a batch commit of \a content records.
*/

/*!
    \fn QContentStore::batchUninstallContent( const QContentIdList &content )

    Performs a batch removal of \a content records from the database.
*/

/*!
    \fn QContentStore::contentCount( const QContentFilter &filter )

    Returns the number of content records that match a \a filter.
*/

/*!
    \fn QContentStore::contentFromEngineType( const QString &engineType )

    Constructs an empty QContent with the engine type \a engineType.
*/

/*!
    \fn QContentStore::moveContentTo( QContent *content, const QString &newFileName )

    Moves the backing file of the given \a content to a new file location a \a newFileName.
*/

/*!
    \fn QContentStore::copyContentTo( QContent *content, const QString &newFileName )

    Copies the backing file of the given \a content to a new file location a \a newFileName and creates a copy
    of the content record for the new file.
 */

/*!
    \fn QContentStore::renameContent(QContent *content, const QString &name)

    Changes the \a name of \a content and renames the backing file appropriately.
*/

/*!
    \fn QContentStore::openContent( QContent *content, QIODevice::OpenMode mode )

    Opens the backing file of the given \a content in the given IO \a mode.
*/

/*!
    Sets an \a error string describing the last error that occured.
*/
void QContentStore::setErrorString( const QString &error )
{
    qLog(DocAPI) << "QContentStore::setErrorString()" << error;

    m_errorString = error;
}

/*!
    Returns a string describing the last error that occurred.
*/
QString QContentStore::errorString() const
{
    return m_errorString;
}

/*!
    Returns an instance of a QContentStore.
*/
QContentStore *QContentStore::instance()
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if( !contentStores()->hasLocalData() )
    {
        if( QContent::documentSystemConnection() == QContent::DocumentSystemDirect )
            contentStores()->setLocalData( new QSqlContentStore );
        else
            contentStores()->setLocalData(  new QDocumentServerContentStore );
    }

    return contentStores()->localData();
#else
    static QSqlContentStore *instance = 0;

    if( !instance )
        instance = new QSqlContentStore;

    return instance;
#endif
}

/*!
    Returns true if the content store has been initialized.

    This will only work from the main thread of the application, in any other thread this will always return false.
*/
bool QContentStore::initialized()
{
#ifndef QTOPIA_CONTENT_INSTALLER
    return qApp->thread() != QThread::currentThread() || contentStores()->hasLocalData();
#else
    return true;
#endif
}

/*!
    Returns the dirty attributes of a content \a engine.
*/
QContentEngine::Attributes QContentStore::dirtyAttributes( const QContentEngine &engine ) const
{
    return engine.dirtyAttributes();
}

/*!
    Sets the \a id of a content \a engine.
*/
void QContentStore::setId( QContentId id, QContentEngine *engine ) const
{
    engine->setId( id );
}

/*!
    Sets the DRM \a state of a content \a engine.
*/
void QContentStore::setDrmState( QContent::DrmState state, QContentEngine *engine ) const
{
    engine->setDrmState( state );
}

/*!
    Sets the last updated \a date of a content \a engine.
*/
void QContentStore::setLastUpdated( const QDateTime &date, QContentEngine *engine ) const
{
    engine->setLastUpdated( date );
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QContentStore::LookupFlags);

/*!
    Returns the internal content engine of \a content.
*/
QContentEngine *QContentStore::contentEngine( QContent *content ) const
{
    return content->d;
}

Q_GLOBAL_STATIC(QContentCache,g_contentCache);

QContentCache::QContentCache()
    : m_cache( 500 )
{
}

QContent QContentCache::lookup( QContentId contentId )
{
    QReadLocker locker( &m_lock );

    QContent *content = m_cache.object( contentId );

    if( content )
        return *content;
    else
        return QContent();
}

void QContentCache::cache( const QContent &content )
{
    QWriteLocker locker( &m_lock );

    if( content.id() != QContent::InvalidId )
        m_cache.insert( content.id(), new QContent( content ) );
}

void QContentCache::remove( QContentId contentId )
{
    QWriteLocker locker( &m_lock );

    m_cache.remove( contentId );
}

void QContentCache::cacheMimeTypeKey( QtopiaDatabaseId databaseId, const QString &mimeType, int key )
{
    QWriteLocker locker( &m_lock );

    m_mimeIdCache.insert( qMakePair( mimeType, databaseId ), new int( key ) );
}

void QContentCache::cacheLocationKey( QtopiaDatabaseId databaseId, const QString &location, int key )
{
    QWriteLocker locker( &m_lock );

    m_locationIdCache.insert( qMakePair( location, databaseId ), new int( key ) );
}

int QContentCache::lookupMimeTypeKey( QtopiaDatabaseId databaseId, const QString &mimeType )
{
    QReadLocker locker( &m_lock );

    int *key = m_mimeIdCache.object( qMakePair( mimeType, databaseId ) );

    return key ? *key : -1;
}

int QContentCache::lookupLocationKey( QtopiaDatabaseId databaseId, const QString &location )
{
    QReadLocker locker( &m_lock );

    int *key = m_locationIdCache.object( qMakePair( location, databaseId ) );

    return key ? *key : -1;
}

void QContentCache::clear()
{
    QWriteLocker locker( &m_lock );

    m_cache.clear();
    m_mimeIdCache.clear();
    m_locationIdCache.clear();
}

QContentCache *QContentCache::instance()
{
    return g_contentCache();
}
