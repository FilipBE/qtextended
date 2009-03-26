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
#ifndef QDOCUMENTSERVERCONTENTSTORE_P_H
#define QDOCUMENTSERVERCONTENTSTORE_P_H

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

#include "qcontentstore_p.h"

class QDocumentServerContentStorePrivate;

class QDocumentServerContentStore : public QContentStore
{
    Q_OBJECT
public:
    QDocumentServerContentStore( QObject *parent = 0 );
    virtual ~QDocumentServerContentStore();

    virtual QContent contentFromId( QContentId contentId );
    virtual QContent contentFromFileName( const QString &fileName, LookupFlags lookup );
    virtual QContent contentFromEngineType( const QString &engineType );

    virtual bool commitContent( QContent *content );
    virtual bool removeContent( QContent *content );
    virtual bool uninstallContent( QContentId contentId );

    virtual void batchCommitContent( const QContentList &content );
    virtual void batchUninstallContent( const QContentIdList &content );

    virtual bool moveContentTo( QContent *content, const QString &newFileName );
    virtual bool copyContentTo( QContent *content, const QString &newFileName );
    virtual bool renameContent(QContent *content, const QString &name);

    virtual QIODevice *openContent( QContent *content, QIODevice::OpenMode mode );

    virtual QStringList contentCategories( QContentId contentId );

    virtual QContentEnginePropertyCache contentProperties( QContentId contentId );

    virtual QStringList contentMimeTypes( QContentId contentId );

    virtual QMimeTypeData mimeTypeFromId( const QString &mimeId );

    virtual QContentSetEngine *contentSet( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode );

    virtual QContentFilterSetEngine *contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType );

    virtual QStringList filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType );

    virtual int contentCount( const QContentFilter &filter );

    int contentSetCount( int setId );

    QContentList contentSetFrame( int setId, int start, int count );

    void setContentSetFilter( int setId, const QContentFilter &filter );

    void setContentSetSortOrder( int setId, const QContentSortCriteria &sortOrder );

    void setContentSetCriteria( int setId, const QContentFilter &filter, const QContentSortCriteria &sortOrder );

    void insertContentIntoSet( int setId, const QContent &content );

    void removeContentFromSet( int setId, const QContent &content );

    void clearContentSet(int setId);

    void commitContentSet(int setId);

    bool contentSetContains( int setId, const QContent &content );

    bool contentValid( QContentId contentId );

    QDrmRights contentRights( QContentId contentId, QDrmRights::Permission );

    bool executeContent( QContentId contentId, const QStringList &arguments );

    QDrmRights::Permissions contentPermissions( QContentId contentId );

    qint64 contentSize( QContentId contentId );

    virtual void addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission);
    virtual void removeAssociation(const QString& mimeType, const QString& application);
    virtual void setDefaultApplicationFor(const QString& mimeType, const QString& application);
    virtual QList<QMimeEngineData> associationsForApplication(const QString& application );
    virtual QList<QMimeEngineData> associationsForMimeType(const QString& mimeType );

private slots:
    void releaseContentSet( int setId );

private:
    QDocumentServerContentStorePrivate *d;
};

#endif
