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
#ifndef QSQLCONTENTSETENGINE_P_H
#define QSQLCONTENTSETENGINE_P_H

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

#include "qcontentsetengine_p.h"
#include <QContent>
#include <QCache>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>

class QSqlContentStore;
class QSqlDatabaseContentSet;
class QSqlContentSetIterator;
class QSqlContentSetUpdateThread;
class QSqlContentSetUpdateProxy;

class QSqlContentSetEngine : public QContentSetEngine
{
    Q_OBJECT
public:
    QSqlContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode, QSqlContentStore *store );
    virtual ~QSqlContentSetEngine();
    virtual int count() const;

    virtual void insertContent( const QContent &content );
    virtual void removeContent( const QContent &content );

    virtual void clear();

    virtual void commitChanges();

    virtual bool contains( const QContent &content ) const;

protected:
    virtual void filterChanged( const QContentFilter &filter );
    virtual void sortCriteriaChanged( const QContentSortCriteria &sort );

private slots:
    void performUpdate();
    void performReset();
    void contentChangedEvent();
    void contentChangedEvent(const QContentIdList &contentIds, QContent::ChangeType type);

    void updateInsert( int index, int count, int primaryIndex, int secondaryIndex );
    void updateRemove( int index, int count, int primaryIndex, int secondaryIndex );
    void updateRefresh( int index, int count );
    void updateFinish();

private:
    virtual int valueCount() const;
    virtual QList< QContent > values( int index, int count );
    QContent contentFromId( QContentId contentId ) const;
    int expectedIndexOf( const QContentSortCriteria &sort, const QContent &content ) const;
    QContent explicitContent( quint64 id ) const;

    bool update();

    int minimumSetIndex( const QContentSortCriteria &sort, const QVector< QContent > &content ) const;
    int maximumSetIndex( const QContentSortCriteria &sort, const QVector< QContent > &content ) const;

    void synchronizeSets(
            const QContentFilter &criteria,
            const QContentSortCriteria &sort,
            const QList<QPair<quint64, QContent> > &explicits,
            const QContentIdList &updatedIds,
            QSqlContentSetUpdateProxy *updateProxy);

    void synchronizeSingleSet(const QContentIdList &updatedIds, QSqlContentSetUpdateProxy *updateProxy);

    void synchronizeMultipleSets(
            const QContentSortCriteria &sort,
            const QList<QContentIdList> &contentIds,
            const QList<QPair<quint64, QContent> > &explicits,
            const QContentIdList &updatedIds,
            QSqlContentSetUpdateProxy *updateProxy);

    QContentIdList sortIds(
            const QContentSortCriteria &sort,
            const QList<QContentIdList> &contentIdLists,
            const QList<QPair<quint64, QContent> > &explicits) const;

    QSqlContentStore *m_store;

    QContentFilter m_filter;
    QContentSortCriteria m_order;

    QList< QContentId > m_primaryIds;
    QList< QContentId > m_secondaryIds;
    QList<QContentId> m_updatedIds;
    QList< QPair< quint64, QContent > > m_explicit;
    mutable QMutex m_databaseSetMutex;
    QMutex m_syncMutex;
    QWaitCondition m_syncCondition;
    QSqlContentSetUpdateThread *m_updateThread;

    int m_count;
    int m_primaryOffset;
    int m_secondaryCutoff;

    bool m_sortChanged;
    bool m_contentChanged;
    bool m_deletePending;

    quint64 m_explicitIdSource;

    friend class QSqlContentSetUpdateThread;
};

#endif
