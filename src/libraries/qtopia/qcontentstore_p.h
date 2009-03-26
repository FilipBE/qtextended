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

#ifndef QCONTENTSTORE_P_H
#define QCONTENTSTORE_P_H

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

#include <QContentFilter>
#include <QThread>
#include <QCache>
#include <QReadWriteLock>

#include "qcontentengine_p.h"

class QContentSetEngine;
class QContentFilterSetEngine;
class QMimeTypeData;

class QContentStore : public QObject
{
    Q_OBJECT
public:

    enum LookupFlag
    {
        Lookup    = 0x01,
        Commit    = 0x02,
        Construct = 0x04
    };

    typedef QFlags< LookupFlag > LookupFlags;

    QContentStore( QObject *parent = 0 );

    virtual ~QContentStore();

    virtual QContent contentFromId( QContentId contentId ) = 0;
    virtual QContent contentFromFileName( const QString &fileName, LookupFlags lookup ) = 0;
    virtual QContent contentFromEngineType( const QString &engineType ) = 0;

    virtual bool commitContent( QContent *content ) = 0;
    virtual bool removeContent( QContent *content ) = 0;
    virtual bool uninstallContent( QContentId contentId ) = 0;

    virtual void batchCommitContent( const QContentList &content ) = 0;
    virtual void batchUninstallContent( const QContentIdList &content ) = 0;

    virtual bool moveContentTo( QContent *content, const QString &newFileName ) = 0;
    virtual bool copyContentTo( QContent *content, const QString &newFileName ) = 0;
    virtual bool renameContent(QContent *content, const QString &name) = 0;

    virtual QIODevice *openContent( QContent *content, QIODevice::OpenMode mode ) = 0;

    virtual QStringList contentCategories( QContentId contentId ) = 0;

    virtual QContentEnginePropertyCache contentProperties( QContentId id ) = 0;

    virtual QStringList contentMimeTypes( QContentId contentId ) = 0;

#ifndef QTOPIA_CONTENT_INSTALLER
    virtual QImage thumbnail(const QContent &content, const QSize &size, Qt::AspectRatioMode);
#endif

    virtual QMimeTypeData mimeTypeFromId( const QString &mimeId ) = 0;

    virtual QContentSetEngine *contentSet( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode ) = 0;

    virtual QContentFilterSetEngine *contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType ) = 0;

    virtual QStringList filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType ) = 0;

    virtual int contentCount( const QContentFilter &filter ) = 0;

    virtual void addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission) = 0;
    virtual void removeAssociation(const QString& mimeType, const QString& application) = 0;
    virtual void setDefaultApplicationFor(const QString& mimeType, const QString& application) = 0;
    virtual QList<QMimeEngineData> associationsForApplication(const QString& application ) = 0;
    virtual QList<QMimeEngineData> associationsForMimeType(const QString& mimeType ) = 0;

    static QContentStore *instance();

    static bool initialized();

    QString errorString() const;

protected:
    void setErrorString( const QString &error );

#ifndef QTOPIA_CONTENT_INSTALLER
    QImage readThumbnail(const QString &fileName, const QSize &size, Qt::AspectRatioMode mode);
    void saveThumbnail(const QString &fileName, const QImage &thumbnail);

    QString thumbnailPath(const QString &fileName) const;
#endif

    QContentEngine::Attributes dirtyAttributes( const QContentEngine &engine ) const;

    void setId( QContentId id, QContentEngine *engine ) const;

    void setDrmState( QContent::DrmState state, QContentEngine *engine ) const;

    void setLastUpdated( const QDateTime &date, QContentEngine *engine ) const;

    QContentEngine *contentEngine( QContent *content ) const;

    void ensurePropertiesLoaded( QContentEngine *engine ) const{ if( !(engine->d_func()->loadedAttributes & QContentEngine::Properties) ) engine->loadProperties(); }
    void ensureCategoriesLoaded( QContentEngine *engine ) const{ if( !(engine->d_func()->loadedAttributes & QContentEngine::Categories) ) engine->loadCategories(); }

private:
    const QStringList m_folderThumbnails;
    const QString m_audioPrefix;
    QString m_errorString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QContentStore::LookupFlags);
Q_DECLARE_USER_METATYPE_ENUM(QContentStore::LookupFlags);

class QContentCache
{
public:
    QContentCache();

    QContent lookup( QContentId contentId );
    void cache( const QContent &content );
    void remove( QContentId contentId );
    void clear();

    void cacheMimeTypeKey( QtopiaDatabaseId databaseId, const QString &mimeType, int key );
    void cacheLocationKey( QtopiaDatabaseId databaseId, const QString &location, int key );

    int lookupMimeTypeKey( QtopiaDatabaseId databaseId, const QString &mimeType );
    int lookupLocationKey( QtopiaDatabaseId databaseId, const QString &location );

    static QContentCache *instance();

private:
    QCache< QContentId, QContent > m_cache;
    QCache< QPair<QString, QtopiaDatabaseId>, int> m_mimeIdCache;
    QCache< QPair<QString, QtopiaDatabaseId>, int> m_locationIdCache;
    QReadWriteLock m_lock;
};

#endif
