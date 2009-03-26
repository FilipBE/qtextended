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
#ifndef QSQLCONTENTSTORE_P_H
#define QSQLCONTENTSTORE_P_H

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
#include <QtopiaSql>
#include <QCache>

class QSqlRecord;
class QSqlError;

class QSqlContentStore : public QContentStore
{
public:
    QSqlContentStore();
    virtual ~QSqlContentStore();

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

    virtual QContentFilterSetEngine *contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType = QString() );

    virtual QStringList filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType );

    virtual int contentCount( const QContentFilter &filter );

    QContentIdList matches( const QContentFilter &filter, const QContentSortCriteria &order );

    QContentIdList matches( QtopiaDatabaseId databaseId, const QContentFilter &filter, const QContentSortCriteria &order );

    QContentList contentFromIds( QtopiaDatabaseId databaseId, const QContentIdList &contentIds );
    QContentList contentFromIds( const QContentIdList &contentIds );

    virtual void addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission);
    virtual void removeAssociation(const QString& mimeType, const QString& application);
    virtual void setDefaultApplicationFor(const QString& mimeType, const QString& application);

    virtual QList<QMimeEngineData> associationsForApplication(const QString& application );
    virtual QList<QMimeEngineData> associationsForMimeType(const QString& mimeType );

private:
    typedef QPair< QString, QVariant > Parameter;

    QContentEngine *installContent( QContent *content );
    QContentEngine *refreshContent( QContent *content );

    bool insertContent( QContentEngine *engine, QtopiaDatabaseId dbId );
    bool updateContent( QContentEngine *engine );

    QByteArray transformString( const QString &name ) const;

    QContent contentFromQuery( const QSqlQuery &query, QtopiaDatabaseId dbId ) const;

    QString constructFileName( const QString &file, const QString &directory ) const;

    QContent::DrmState convertDrmState( int state ) const;
    int convertDrmState( QContent::DrmState state ) const;

    QContent::Role convertRole( const QString &role ) const;
    QString convertRole( QContent::Role role ) const;

    int mimeId( const QString &type, QtopiaDatabaseId dbId );
    int queryMimeId( const QString &type, QtopiaDatabaseId dbId );
    int locationId( const QString &location, QtopiaDatabaseId dbId );
    int queryLocationId( const QString &location, QtopiaDatabaseId dbId );

    QContent desktopContent( const QString &fileName );
    QContent executableContent( const QString &fileName );

    void logError( const char *signature, const char *query, const QContent &content, QtopiaDatabaseId databaseId, const QSqlError &error );
    void logError( const char *signature, const char *query, const QContentEngine &content, QtopiaDatabaseId databaseId, const QSqlError &error );
    void logError( const char *signature, const char *query, QtopiaDatabaseId databaseId, const QSqlError &error );
    void logError( const char *signature, const QString &error );

    bool insertCategories( QContentId id, const QStringList &categories );
    bool removeCategories( QContentId id );

    bool insertProperties( QContentId id, const QContentEngine &engine );
    bool removeProperties( QContentId id );

    bool syncCategory( QtopiaDatabaseId, const QString &categoryId );

    QString buildFrom( const QContentFilter &, const QStringList &joins );
    QString buildWhereClause( const QContentFilter &filter, QList< Parameter > *parameters, int *insertAt, QStringList *joins );
    QString buildQtopiaTypes( const QStringList &types, const QString &conjunct, QList< Parameter > *parameters );
    QString buildMimeTypes( const QStringList &mimes, const QString &conjunct, QList< Parameter > *parameters );
    QString buildPaths( const QStringList &locations, const QStringList &directories, const QString &conjunct, QList< Parameter > *parameters );
    QString buildCategories( const QStringList &categories, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins );
    QString buildDrm( const QStringList &drm, const QString &conjunct, QList< Parameter > *parameters );
    QString buildSynthetic( const QStringList &synthetic, const QString &conjunct, QList< Parameter > *parameters, int *insertAt, QStringList *joins );
    QString buildFileNames( const QStringList &fileNames, const QString &conjunct, QList< Parameter > *parameters );
    QString buildNames( const QStringList &names, const QString &conjunct, QList< Parameter > *parameters );
    QString buildOrderBy( const QContentSortCriteria &, QList< Parameter > *, int *, QStringList * );

    QString buildQuery( const QString &queryTemplate, const QContentFilter &filter, QList< Parameter > *parameters );
    QString buildQuery( const QString &queryTemplate, const QContentFilter &filter, const QContentSortCriteria &sortOrder, QList< Parameter > *parameters );
    void bindParameters( QSqlQuery *query, const QList< Parameter > &parameters );
    QString addParameter( const QVariant &parameter, QList< Parameter > *parameters );
    QString addParameter( const QVariant &parameter, QList< Parameter > *parameters, int *insertAt );

    QSet< QContentFilter::FilterType > getAllFilterTypes( const QContentFilter &filter );
    QSet< QString > getAllSyntheticKeys( const QContentFilter &filter );

    QStringList mimeFilterMatches( const QContentFilter &filter, const QString &subType = QString() );
    QStringList syntheticFilterGroups( const QContentFilter &filter );
    QStringList syntheticFilterKeys( const QContentFilter &filter, const QString &group );
    QStringList syntheticFilterMatches( const QContentFilter &filter, const QString &group, const QString &key );
    QStringList categoryFilterMatches( const QContentFilter &filter, const QString &key );
    QStringList directoryFilterMatches( const QContentFilter &filter, const QString &directory );

    QString deriveName( const QString &fileName, const QMimeType &type ) const;
};

#endif
