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
#include "qdocumentservercontentsetengine_p.h"
#include "qdocumentservercontentstore_p.h"
#include <QTimer>
#include <QtDebug>


/*!
    \class QDocumentServerContentSetEngine
    \inpublicgroup QtBaseModule

    \brief The QDocumentServerContentSetEngine is an implementation of the QContentSetEngine interface for content stored in QDocumentServerContentStore.

    \internal
 */

/*!
    Constructs a new sql content set engine for content in the given content \a store with the filtering
    criteria \a filter, and sort order \a order.
 */
QDocumentServerContentSetEngine::QDocumentServerContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode, int setId, QDocumentServerContentStore *store )
    : QContentSetEngine( filter, order, mode )
    , m_setId( setId )
    , m_store( store )
    , m_count( 0 )
{
}

QDocumentServerContentSetEngine::~QDocumentServerContentSetEngine()
{
    emit releaseContentSet( m_setId );
}

QContent QDocumentServerContentSetEngine::content( int index ) const
{
    return QContentSetEngine::content( index );
}

/*!
    \reimp
 */
int QDocumentServerContentSetEngine::count() const
{
    return m_count;
}

/*!
    \reimp
 */
bool QDocumentServerContentSetEngine::isEmpty() const
{
    return count() == 0;
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::filterChanged( const QContentFilter &filter )
{
    m_store->setContentSetFilter(m_setId, filter);
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::sortCriteriaChanged( const QContentSortCriteria &order )
{
    m_store->setContentSetSortOrder(m_setId, order);
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::insertContent( const QContent &content )
{
    m_store->insertContentIntoSet( m_setId, content );
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::removeContent( const QContent &content )
{
    m_store->removeContentFromSet( m_setId, content );
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::clear()
{
    setFilter( QContentFilter() );
    setSortCriteria( QContentSortCriteria() );
    m_store->clearContentSet(m_setId);
}

/*!
    \reimp
*/
void QDocumentServerContentSetEngine::commitChanges()
{
    m_store->commitContentSet(m_setId);

    if (updateMode() == QContentSet::Synchronous) {
        clearCache();

        m_count = m_store->contentSetCount(m_setId);
    }
}

/*!
    \reimp
 */
bool QDocumentServerContentSetEngine::contains( const QContent &content ) const
{
    return m_store->contentSetContains( m_setId, content );
}

void QDocumentServerContentSetEngine::insertContent( int start, int end )
{
    emit contentAboutToBeInserted( start, end );

    int count = end - start + 1;

    insertRange( start, count );

    m_count += count;

    emit contentInserted();
}

void QDocumentServerContentSetEngine::removeContent( int start, int end )
{
    emit contentAboutToBeRemoved( start, end );

    int count = end - start + 1;

    removeRange( start, count );

    m_count -= count;

    emit contentRemoved();
}

void QDocumentServerContentSetEngine::refreshContent( int start, int end )
{
    int count = end - start + 1;

    refreshRange( start, count );

    emit contentChanged( start, end );
}

QContentList QDocumentServerContentSetEngine::values( int index, int count )
{
    return m_store->contentSetFrame( m_setId, index, count );
}

int QDocumentServerContentSetEngine::valueCount() const
{
    return m_count;
}
