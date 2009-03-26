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
#include "qsqlcontentsetengine_p.h"
#include "qsqlcontentstore_p.h"
#include "qcontent_p.h"
#include <QTimer>
#include <QtopiaApplication>
#include <QtAlgorithms>
#include <QMetaObject>

class QSqlContentSetUpdateProxy : public QObject
{
    Q_OBJECT
public:
    QSqlContentSetUpdateProxy( QObject *parent = 0 ) : QObject( parent ){}

signals:
    void insert( int index, int count, int primaryIndex, int secondaryIndex );
    void remove( int index, int count, int primaryIndex, int secondaryIndex );
    void refresh( int index, int count );

    friend class QSqlContentSetEngine;
};

class QSqlContentSetUpdateThread : public QThread
{
    Q_OBJECT
public:
    QSqlContentSetUpdateThread( QSqlContentSetEngine *contentSet )
        : QThread( contentSet )
        , m_contentSet( contentSet )
    {
    }

protected:
    virtual void run()
    {
        while( m_contentSet->update() )
        {
            QThread::msleep( 500 );
        }
    }

private:
    QPointer< QSqlContentSetEngine > m_contentSet;
};

class QContentSetExplicitLessThan
{
public:
    QContentSetExplicitLessThan( const QContentSortCriteria &sort )
        : m_sort( sort )
    {
    }

    bool operator()( const QPair< quint64, QContent > &left, const QPair< quint64, QContent > &right )
    {
        return m_sort.lessThan( left.second, right.second );
    }

private:
    QContentSortCriteria m_sort;
};

/*!
    \class QSqlContentSetEngine
    \inpublicgroup QtBaseModule

    \brief The QSqlContentSetEngine is an implementation of the QContentSetEngine interface for content stored in QSqlContentStore.

    \internal
 */


/*!
    Constructs a new sql content set engine for content in the given content \a store with the filtering
    criteria \a filter, and sort order \a order.
 */
QSqlContentSetEngine::QSqlContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode, QSqlContentStore *store )
    : QContentSetEngine( filter, order, mode )
    , m_store( store )
    , m_filter(filter)
    , m_order(order)
    , m_updateThread( 0 )
    , m_count( 0 )
    , m_primaryOffset( 0 )
    , m_secondaryCutoff( 0 )
    , m_sortChanged(false)
    , m_contentChanged(filter.isValid())
    , m_deletePending( false )
    , m_explicitIdSource( 0 )
{
#ifndef QTOPIA_CONTENT_INSTALLER
    connect(qApp, SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
            this, SLOT(contentChangedEvent(QContentIdList,QContent::ChangeType)));
    connect(QContentUpdateManager::instance(), SIGNAL(refreshRequested()),
            this, SLOT(contentChangedEvent()));
#endif
}

QSqlContentSetEngine::~QSqlContentSetEngine()
{
    m_deletePending = true; // Tell the update thread to quit.

    if( m_updateThread )
    {
        do
        {
            m_syncCondition.wakeAll();
        }
        while( !m_updateThread->wait( 300 ) );

        delete m_updateThread;
    }
}

/*!
    \reimp
 */
int QSqlContentSetEngine::count() const
{
    return m_count;
}

/*!
    \reimp
 */
void QSqlContentSetEngine::filterChanged( const QContentFilter &filter )
{
    QMutexLocker locker( &m_databaseSetMutex );

    m_contentChanged = true;
    m_filter = filter;
}

/*!
    \reimp
 */
void QSqlContentSetEngine::sortCriteriaChanged( const QContentSortCriteria &sort )
{
    QMutexLocker locker( &m_databaseSetMutex );

    m_sortChanged = true;
    m_order = sort;
}

/*!
    \reimp
 */
void QSqlContentSetEngine::insertContent( const QContent &content )
{
    if( updateMode() == QContentSet::Synchronous )
    {
        const QContentSortCriteria sort = sortCriteria();

        int index;

        for( index = 0; index < m_explicit.count() && sort.greaterThan( content, m_explicit.at( index ).second ); index++ );

        QContentId id(QtopiaDatabaseId(-1), m_explicitIdSource++);

        m_explicit.insert( index, QPair< quint64, QContent >( id.second, content ) );

        index = expectedIndexOf( sort, content );

        m_primaryIds.insert( index, id );
        insertRange( index, 1 );
        m_count++;
    }
    else
    {
        QMutexLocker locker( &m_databaseSetMutex );

        const QContentSortCriteria sort = sortCriteria();

        int index;

        for( index = 0; index < m_explicit.count() && sort.greaterThan( content, m_explicit.at( index ).second ); index++ );

        m_explicit.insert( index, QPair< quint64, QContent >( m_explicitIdSource++, content ) );

        m_contentChanged = true;
    }
}

/*!
    \reimp
 */
void QSqlContentSetEngine::removeContent( const QContent &content )
{
    if( updateMode() == QContentSet::Synchronous )
    {
        int index = 0;

        for( ; index < m_explicit.count() && !(m_explicit.at( index ).second == content); index++ );

        if( index == m_explicit.count() )
            return;

        QContentId contentId( QtopiaDatabaseId(-1), m_explicit.at( index ).first );

        m_explicit.removeAt( index );

        index = m_primaryIds.indexOf( contentId );

        if( index != -1 )
        {
            m_primaryIds.removeAt( index );
            removeRange( index, 1 );
            m_count--;
        }
    }
    else
    {
        QMutexLocker locker( &m_databaseSetMutex );

        int index = 0;

        for( ; index < m_explicit.count(); index++ )
            if( m_explicit.at( index ).second == content )
                break;

        if( index == m_explicit.count() )
            return;

        m_explicit.removeAt( index );

        m_contentChanged = true;
    }
}

/*!
    \reimp
 */
void QSqlContentSetEngine::clear()
{
    {
        QMutexLocker locker( &m_databaseSetMutex );
        m_explicit.clear();

        m_contentChanged = true;
    }

    setSortCriteria( QContentSortCriteria() );
    setFilter( QContentFilter() );
}

/*!
    \reimp
*/
void QSqlContentSetEngine::commitChanges()
{
    if (updateMode() == QContentSet::Synchronous)
        performReset();
    else
        performUpdate();

}

/*!
    \reimp
 */
bool QSqlContentSetEngine::contains( const QContent &content ) const
{
    if( filter().test( content ) )
        return true;

    QList< QPair< quint64, QContent > > explicits;

    {
        QMutexLocker locker(&m_databaseSetMutex);
        explicits = m_explicit;
    }

    for( int i = 0; i < m_explicit.count(); i++ )
        if( explicits.at( i ).second == content )
            return true;

    return false;
}

void QSqlContentSetEngine::performReset()
{
    if (!m_sortChanged && !m_contentChanged)
        return;

    clearCache();
    m_primaryIds.clear();

    m_count = 0;

    QList< QContentIdList > contentIdLists;
    QList< QtopiaDatabaseId > databaseIds = QtopiaSql::instance()->databaseIds();

    int total = 0;

    const QContentFilter criteria = m_filter;
    const QContentSortCriteria sort = m_order;

    foreach( QtopiaDatabaseId databaseId, databaseIds )
    {
        QContentIdList contentIdList = m_store->matches( databaseId, criteria, sort );

        if( !contentIdList.isEmpty() )
        {
            contentIdLists.append( m_store->matches( databaseId, criteria, sort ) );

            total += contentIdLists.last().count();
        }
    }

    if (m_sortChanged)
        qSort( m_explicit.begin(), m_explicit.end(), QContentSetExplicitLessThan( sort ) );

    QContentIdList explicitIds;

    QPair< quint64, QContent > e;

    foreach( e, m_explicit )
        explicitIds.append( QContentId( QtopiaDatabaseId(-1), e.first ) );

    if( !explicitIds.isEmpty() )
    {
        contentIdLists.append( explicitIds );
        total += contentIdLists.last().count();
    }

    if( contentIdLists.count() == 1 )
    {
        m_primaryIds = contentIdLists.first();
    }
    else if( contentIdLists.count() > 1 )
    {
        m_primaryIds = sortIds( sort, contentIdLists, m_explicit );
    }

    m_count = m_primaryIds.count();
    m_sortChanged = false;
    m_contentChanged = false;
}

QContent QSqlContentSetEngine::explicitContent( quint64 id ) const
{
    for( int i = 0; i < m_explicit.count(); i++ )
        if( m_explicit.at( i ).first == id )
            return m_explicit.at( i ).second;

    return QContent();
}

QContent QSqlContentSetEngine::contentFromId( QContentId contentId ) const
{
    if( contentId.first == QtopiaDatabaseId(-1) )
    {
        for( int i = 0; i < m_explicit.count(); i++ )
            if( m_explicit.at( i ).first == contentId.second )
                return m_explicit.at( i ).second;

        return QContent();
    }
    else
        return QContent( contentId );
}

int QSqlContentSetEngine::expectedIndexOf( const QContentSortCriteria &sort, const QContent &content ) const
{
    int lowerBound = 0;
    int upperBound = m_primaryIds.count();
    int index = 0;

    while( upperBound - lowerBound > 1 )
    {
        index = lowerBound + (upperBound - lowerBound) / 2;

        if( sort.lessThan( content, contentFromId( m_primaryIds.at( index ) ) ) )
            upperBound = index;
        else
            lowerBound = index;
    }

    if( upperBound != lowerBound )
    {
        if( sort.greaterThan( content, contentFromId( m_primaryIds.at( lowerBound ) ) ) )
            return upperBound;
        else
            return lowerBound;
    }

    return index;
};


/*!
    Requeries the content of the set.
 */
void QSqlContentSetEngine::performUpdate()
{
    if( !m_updateThread )
        m_updateThread = new QSqlContentSetUpdateThread( this );

    if( !m_updateThread->isRunning() )
        m_updateThread->start();
}

void QSqlContentSetEngine::contentChangedEvent()
{
    QMutexLocker locker(&m_databaseSetMutex);

    m_contentChanged = true;
}

void QSqlContentSetEngine::contentChangedEvent(const QContentIdList &contentIds, QContent::ChangeType type)
{
    QMutexLocker locker(&m_databaseSetMutex);

    m_contentChanged = true;

    if (type == QContent::Updated)
        m_updatedIds += contentIds;
}

/*!
    Finds the index of the database set with the lowest value at the given \a indices when compared using \a sort and returns
    the set index and lowest value content record.
 */
int QSqlContentSetEngine::minimumSetIndex( const QContentSortCriteria &sort, const QVector< QContent > &content ) const
{
    int minimumSet = 0;

    for( int set = 1; set < content.count(); set++ )
    {
        if( content.at( minimumSet ).isNull() || !content.at( set ).isNull() && sort.lessThan( content.at( set ), content.at( minimumSet ) ) )
            minimumSet = set;
    }

    return minimumSet;
}

/*!
    Finds the index of the database set with the largest value at the given \a indices when compared using \a sort.  Sets where
    the upper (\a upperLimits) and lower (\a lowerLimits) limits have converged are excluded from the comparison.
*/
int QSqlContentSetEngine::maximumSetIndex( const QContentSortCriteria &sort, const QVector< QContent > &content ) const
{
    int maximumSet = 0;

    for( int set = 1; set < content.count(); set++ )
    {
        if( content.at( maximumSet ).isNull() || !content.at( set ).isNull() && sort.greaterThan( content.at( set ), content.at( maximumSet ) ) )
            maximumSet = set;
    }

    return maximumSet;
}

int QSqlContentSetEngine::valueCount() const
{
    return m_count;
}

QList< QContent > QSqlContentSetEngine::values( int index, int count )
{
    QContentIdList contentIds;

    {
        QMutexLocker locker( &m_databaseSetMutex );

        int primaryCount = count;
        int secondaryCount = qMin( count, m_secondaryCutoff - index );

        if( secondaryCount > 0 )
        {
            contentIds = m_secondaryIds.mid( index, secondaryCount );

            primaryCount -= secondaryCount;
            index += secondaryCount;
        }

        if( primaryCount > 0 )
            contentIds += m_primaryIds.mid( index - m_secondaryCutoff + m_primaryOffset, primaryCount );
    }

    QList< QContent > values = m_store->contentFromIds( contentIds );

    Q_ASSERT(values.count() == count);

    for( int i = 0; i < contentIds.count(); i++ )
        if( contentIds.at( i ).first == QtopiaDatabaseId(-1) )
            values[ i ] = explicitContent( contentIds.at( i ).second );

    return values;
}


bool QSqlContentSetEngine::update()
{
    QContentFilter criteria;
    QContentSortCriteria sort;
    QList< QPair< quint64, QContent > > explicits;
    QContentIdList updatedIds;
    bool sortChanged = false;
    bool contentChanged = false;

    {
        QMutexLocker locker( &m_databaseSetMutex );

        if (m_deletePending || (!m_sortChanged && !m_contentChanged))
            return false;

        criteria = m_filter;
        sort = m_order;

        explicits = m_explicit;

        updatedIds = m_updatedIds;
        m_updatedIds.clear();

        sortChanged = m_sortChanged;
        contentChanged = m_contentChanged;

        m_sortChanged = false;
        m_contentChanged = false;
    }

    QSqlContentSetUpdateProxy updateProxy;

    connect( &updateProxy, SIGNAL(insert(int,int,int,int)), this, SLOT(updateInsert(int,int,int,int)), Qt::QueuedConnection );
    connect( &updateProxy, SIGNAL(remove(int,int,int,int)), this, SLOT(updateRemove(int,int,int,int)), Qt::QueuedConnection );
    connect( &updateProxy, SIGNAL(refresh(int,int)), this, SLOT(updateRefresh(int,int)), Qt::QueuedConnection );

    if (sortChanged)
        qSort(explicits.begin(), explicits.end(), QContentSetExplicitLessThan(sort));

    if (sortChanged || contentChanged)
        synchronizeSets(criteria, sort, explicits, updatedIds, &updateProxy);

    return !m_deletePending;
}

/*!
    Updates the content set to reflect changes in the database.  If content has been inserted or removed
    the contentAboutToBeInserted()/contentInserted() and contentAboutToBeRemoved()/contentRemoved() signals
    are emitted for the appropriate indices.

    The update is performed by first querying updated data for each database set, and then iterating over both sets
    looking for differences.  When a difference is found the appropriate signals are emitted and a set of transisition
    indices are set indicating the point where content should stop being pulled from the pre-existing sets and instead
    be pulled from the updated sets.  After iterating through the set completely the pre-existing sets are replaced
    by the updated sets.
*/
void QSqlContentSetEngine::synchronizeSets(
        const QContentFilter &criteria,
        const QContentSortCriteria &sort,
        const QList< QPair<quint64, QContent> > &explicits,
        const QContentIdList &updatedIds,
        QSqlContentSetUpdateProxy *updateProxy)
{
    QMetaObject::invokeMethod( this, "startUpdate", Qt::QueuedConnection );

    QList< QContentIdList > contentIdLists;
    QList< QtopiaDatabaseId > databaseIds = QtopiaSql::instance()->databaseIds();

    foreach( QtopiaDatabaseId databaseId, databaseIds )
    {
        QContentIdList contentIdList = m_store->matches( databaseId, criteria, sort );

        if( !contentIdList.isEmpty() )
            contentIdLists.append( m_store->matches( databaseId, criteria, sort ) );
    }

    QContentIdList explicitIds;
    QPair< quint64, QContent > e;

    foreach( e, explicits )
        explicitIds.append( QContentId( QtopiaDatabaseId(-1), e.first ) );

    if( !explicitIds.isEmpty() )
        contentIdLists.append( explicitIds );

    if( contentIdLists.isEmpty() )
    {
        if( m_primaryIds.count() != 0 )
        {
            updateProxy->remove( 0, m_primaryIds.count(), m_primaryIds.count(), 0 );

            QMutexLocker locker( &m_databaseSetMutex );

            m_secondaryIds = QContentIdList();
        }
    }
    else if( contentIdLists.count() == 1 )
    {
        {
            QMutexLocker locker( &m_databaseSetMutex );

            m_secondaryIds = contentIdLists.first();
        }

        if( m_primaryIds.count() == 0 )
        {
            updateProxy->insert( 0, m_secondaryIds.count(), 0, m_secondaryIds.count() );
        }
        else
        {
            synchronizeSingleSet(updatedIds, updateProxy);
        }
    }
    else
    {
        synchronizeMultipleSets(sort, contentIdLists, explicits, updatedIds, updateProxy );
    }

    {
        QMutexLocker locker( &m_syncMutex );

        QMetaObject::invokeMethod( this, "updateFinish", Qt::QueuedConnection );

        m_syncCondition.wait( &m_syncMutex );
    }

    QMutexLocker locker( &m_databaseSetMutex );

    if( !m_deletePending )
    {
        Q_ASSERT( m_count == m_secondaryIds.count() );

        m_primaryIds = m_secondaryIds;
        m_secondaryIds.clear();
        m_primaryOffset = 0;
        m_secondaryCutoff = 0;
    }
};

void QSqlContentSetEngine::synchronizeSingleSet(
            const QContentIdList &updatedIds, QSqlContentSetUpdateProxy *updateProxy)
{
    const int primaryCount = m_primaryIds.count();
    const int secondaryCount = m_secondaryIds.count();

    int primaryIndex = 0;
    int secondaryIndex = 0;

    for( bool match = m_primaryIds.at( primaryIndex ) == m_secondaryIds.at( secondaryIndex ); primaryIndex < primaryCount && secondaryIndex < secondaryCount; )
    {
        int primaryIndex2 = primaryIndex + 1;
        int secondaryIndex2 = secondaryIndex + 1;

        for( ; match && primaryIndex2 < primaryCount && secondaryIndex2 < secondaryCount; primaryIndex2++, secondaryIndex2++ )
        {
            if (m_primaryIds.at(primaryIndex2) != m_secondaryIds.at(secondaryIndex2)) {
                primaryIndex = primaryIndex2;
                secondaryIndex = secondaryIndex2;

                match = false;
            } else if (updatedIds.contains(m_secondaryIds.at(secondaryIndex2))) {
                updateProxy->refresh(secondaryIndex2, secondaryIndex2);
            }
        }

        if( match )
        {
            primaryIndex = primaryIndex2;
            secondaryIndex = secondaryIndex2;
        }

        for( ; !match && primaryIndex2 < primaryCount && secondaryIndex2 < secondaryCount; primaryIndex2++, secondaryIndex2++ )
        {
            if( m_primaryIds.at( primaryIndex2 ) == m_secondaryIds.at( secondaryIndex ) )
            {
                updateProxy->remove( secondaryIndex, primaryIndex2 - primaryIndex, primaryIndex2, secondaryIndex );

                primaryIndex = primaryIndex2;

                match = true;
            }
            else if( m_primaryIds.at( primaryIndex ) == m_secondaryIds.at( secondaryIndex2 ) )
            {
                updateProxy->insert( secondaryIndex, secondaryIndex2 - secondaryIndex, primaryIndex, secondaryIndex2 );

                secondaryIndex = secondaryIndex2;

                match = true;
            }
        }

        for( ; !match && primaryIndex2 < primaryCount; primaryIndex2++ )
        {
            if( m_primaryIds.at( primaryIndex2 ) == m_secondaryIds.at( secondaryIndex ) )
            {
                updateProxy->remove( secondaryIndex, primaryIndex2 - primaryIndex, primaryIndex2, secondaryIndex );

                primaryIndex = primaryIndex2;

                match = true;
            }
        }

        for( ; !match && secondaryIndex2 < secondaryCount; secondaryIndex2++ )
        {
            if( m_primaryIds.at( primaryIndex ) == m_secondaryIds.at( secondaryIndex2 ) )
            {
                updateProxy->insert( secondaryIndex, secondaryIndex2 - secondaryIndex, primaryIndex, secondaryIndex2 );

                secondaryIndex = secondaryIndex2;

                match = true;
            }
        }

        if( !match )
        {
            updateProxy->remove( secondaryIndex, primaryIndex2 - primaryIndex, primaryIndex2, secondaryIndex );
            primaryIndex = primaryIndex2;
            updateProxy->insert( secondaryIndex, secondaryIndex2 - secondaryIndex, primaryIndex, secondaryIndex2 );
            secondaryIndex = secondaryIndex2;
        }
    }

    if( primaryIndex < primaryCount )
    {
        updateProxy->remove( secondaryIndex, primaryCount - primaryIndex, primaryCount, secondaryIndex );
    }
    else if( secondaryIndex < secondaryCount )
    {
        updateProxy->insert( secondaryIndex, secondaryCount - secondaryIndex, primaryIndex, secondaryCount );
    }
}

static int totalIdCount( const QList< QContentIdList > &contentIdLists )
{
    int count = 0;

    foreach( const QContentIdList &contentIdList, contentIdLists )
        count += contentIdList.count();

    return count;
}

void QSqlContentSetEngine::synchronizeMultipleSets(
        const QContentSortCriteria &sort,
        const QList<QContentIdList> &contentIdLists,
        const QList<QPair<quint64, QContent> > &explicits,
        const QContentIdList &updatedIds,
        QSqlContentSetUpdateProxy *updateProxy)
{
    QContentIdList contentIds = sortIds( sort, contentIdLists, explicits );

    {
        QMutexLocker locker( &m_databaseSetMutex );

        m_secondaryIds = contentIds;
    }

    if( m_primaryIds.count() == 0 )
    {
        updateProxy->insert( 0, m_secondaryIds.count(), 0, m_secondaryIds.count() );
    }
    else
    {
        synchronizeSingleSet(updatedIds, updateProxy);
    }
}

struct QSqlContentSetEngineColumn
{
    QtopiaDatabaseId databaseId;
    int index;
    int count;
    QContentList cache;
    int cacheIndex;
    int cacheCount;
};

QContentIdList QSqlContentSetEngine::sortIds( const QContentSortCriteria &sort, const QList< QContentIdList > &contentIdLists, const QList< QPair< quint64, QContent > > &explicits ) const
{
    const int columnCount = contentIdLists.count();
    const int idCount = totalIdCount( contentIdLists );

    QVector< QSqlContentSetEngineColumn > columns( columnCount );
    QVector< QContent > content( columnCount );

    for( int i = 0; i < columnCount; i++ )
    {
        QSqlContentSetEngineColumn &column = columns[ i ];

        column.databaseId = contentIdLists.at( i ).first().first;
        column.index = 0;
        column.count = contentIdLists.at( i ).count();
        if( column.databaseId == QtopiaDatabaseId(-1) )
        {
            for( int i = 0; i < column.count && i < 20; i++ )
                column.cache.append( explicits.at( i ).second );
        }
        else
            column.cache = m_store->contentFromIds( column.databaseId, contentIdLists.at( i ).mid( 0, 20 ) );
        column.cacheIndex = 0;
        column.cacheCount = column.cache.count();

        content[ i ] = column.cache.first();
    }

    QContentIdList contentIds;

    for( int i = 0; i < idCount; i++ )
    {
        int minimumColumn = 0;

        for( int column = 1; column < columnCount; column++ )
            if( content.at( minimumColumn ).isNull() || !content.at( column ).isNull() && sort.lessThan( content.at( column ), content.at( minimumColumn ) ) )
                minimumColumn = column;

        QSqlContentSetEngineColumn &column = columns[ minimumColumn ];

        contentIds.append( contentIdLists.at( minimumColumn ).at( column.index ) );

        column.index++;
        column.cacheIndex++;

        if( column.index < column.count )
        {
            if( column.cacheIndex == column.cacheCount )
            {
                if( column.databaseId == QtopiaDatabaseId(-1) )
                {
                    column.cache.clear();
                    for( int i = column.index; i < column.count && i < column.index + 20; i++ )
                        column.cache.append( explicits.at( i ).second );
                }
                else
                    column.cache = m_store->contentFromIds( column.databaseId, contentIdLists.at( minimumColumn ).mid( column.index, 20 ) );
                column.cacheIndex = 0;
                column.cacheCount = column.cache.count();
            }

            content[ minimumColumn ] = column.cache.at( column.cacheIndex );
        }
        else
            content[ minimumColumn ] = QContent();
    }

    return contentIds;
}

/*!
    Called by update() to insert content between the \a start and \a end indices into the visible set.  The content
    is inserted by updating the transition indices of each database set such that indices less than the \a secondaryIndices
    are pulled from the updated database set, and indices greater than or equal are pulled from the pre-existing set at
    an offset indicated by the \a primaryIndices.
 */
void QSqlContentSetEngine::updateInsert( int index, int count, int primaryIndex, int secondaryIndex )
{
    emit contentAboutToBeInserted( index, index + count - 1 );
    {
        QMutexLocker locker( &m_databaseSetMutex );

        // Update transition indices.
        m_primaryOffset = primaryIndex;
        m_secondaryCutoff = secondaryIndex;

        insertRange( index, count );

        m_count += count;
    }

    emit contentInserted();
}

/*!
    Called by update() to remove content between the \a start and \a end indices from the visible set.  The content
    is removed by updating the transition indices of each database set such that indices less than the \a secondaryIndices
    are pulled from the updated database set, and indices greater than or equal are pulled from the pre-existing set at
    an offset indicated by the \a primaryIndices.
*/
void QSqlContentSetEngine::updateRemove( int index, int count, int primaryIndex, int secondaryIndex )
{
    emit contentAboutToBeRemoved( index, index + count - 1 );
    {
        QMutexLocker locker( &m_databaseSetMutex );

        // Update transition indices.
        m_primaryOffset = primaryIndex;
        m_secondaryCutoff = secondaryIndex;

        removeRange( index, count );

        m_count -= count;
    }

    emit contentRemoved();
}

/*!
    Ensure the contents of the cache are up to date in the range starting at \a index with \a count items.
*/
void QSqlContentSetEngine::updateRefresh( int index, int count )
{
    refreshRange( index, count );

    emit contentChanged( index, index + count - 1 );
}

void QSqlContentSetEngine::updateFinish()
{
    finishUpdate();

    QMutexLocker locker( &m_syncMutex );

    m_syncCondition.wakeAll();
}

#include "qsqlcontentsetengine.moc"
