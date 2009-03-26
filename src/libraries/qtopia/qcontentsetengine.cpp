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
#include "qcontentsetengine_p.h"
#include <QTimerEvent>

/*!
    \class QContentSetEngine
    \inpublicgroup QtBaseModule

    \brief The QContentSetEngine class provides an interface to a filtered set of content.

    \internal
*/

/*!
    Destroys a QContentSetEngine.
*/
QContentSetEngine::~QContentSetEngine()
{
}

/*!
    \fn QContentSetEngine::content( int index ) const

    Returns the content object at the given \a index in the set.
*/
QContent QContentSetEngine::content( int index ) const
{
    return index >= 0 && index < valueCount()
        ? value( index )
        : QContent();
}

/*!
    Returns the id of the content object at the given \a index in the set.
*/
QContentId QContentSetEngine::contentId( int index ) const
{
    return content( index ).id();
}

int QContentSetEngine::count() const
{
    return valueCount();
}

/*!
    Returns true if the set is empty.
 */
bool QContentSetEngine::isEmpty() const
{
    return count() == 0;
}

/*!
    Sorts the set in the given \a order.
*/
void QContentSetEngine::setSortOrder( const QStringList &order )
{
    if( order != m_sortOrder )
    {
        m_sortOrder = order;

        setSortCriteria( convertSortOrder( order ) );
    }
}


/*!
    Sorts the set according to the \a sort criteria.
*/
void QContentSetEngine::setSortCriteria( const QContentSortCriteria &sort )
{
    if( sort != m_sortCriteria )
    {
        m_sortCriteria = sort;

        sortCriteriaChanged( m_sortCriteria );
    }
}

/*!
    Sets the filtering criteria of the set to \a filter.
 */
void QContentSetEngine::setFilter( const QContentFilter &filter )
{
    if( filter != m_filter )
    {
        m_filter = filter;

        filterChanged( filter );
    }
}

QContentSortCriteria QContentSetEngine::convertSortOrder( const QStringList &sortOrder )
{
    QContentSortCriteria criteria;

    foreach( QString sort, sortOrder )
    {
        QContentSortCriteria::Attribute attribute;
        QString argument;
        Qt::SortOrder order = Qt::AscendingOrder;

        if( sort.startsWith( QLatin1String( "name" ) ) )
            attribute = QContentSortCriteria::Name;
        else if( sort.startsWith( QLatin1String( "type" ) ) )
            attribute = QContentSortCriteria::MimeType;
        else if( sort.startsWith( QLatin1String( "synthetic" ) ) )
        {
            attribute = QContentSortCriteria::Property;

            argument = sort.section( QLatin1Char( '/' ), 1, 2 );

            if( argument.endsWith( QLatin1String( " asc" ) ) )
                argument.chop( 4 );
            else if( argument.endsWith( QLatin1String( " desc" ) ) )
                argument.chop( 5 );
        }
        else if( sort.startsWith( QLatin1String( "time" ) ) )
            attribute = QContentSortCriteria::LastUpdated;
        else
            continue;

        if( sort.endsWith( QLatin1String( " desc" ) ) )
            order = Qt::DescendingOrder;

        criteria.addSort( attribute, argument, order );
    }

    return criteria;
}

/*!
    \fn QContentSetEngine::insertContent( const QContent &content )

    Inserts a \a content object into the set.  Inserted content is always visible irregardless of the filtering
    criteria of the set.
*/

/*!
    \fn QContentSetEngine::removeContent( const QContent &content )

    Removes a \a content object from the set.
*/

/*!
    \fn QContentSetEngine::clear()

    Removes all content from the set and clears the filtering criteria.
*/

/*!
    \fn QContentSetEngine::commitChanges()

    Commits any changes made to a content set.
*/

/*!
    Flushes a pending update.
*/
void QContentSetEngine::flush()
{
    if (m_flushTimerId != -1) {
        killTimer(m_flushTimerId);

        m_flushTimerId = -1;

        commitChanges();

        if (m_updateMode == QContentSet::Synchronous)
            emit contentChanged();
    }
}

/*!
    Schedules a flush of the content set.

    The flush will occur either on returning to the event loop or on the next call to count() if
    the content set is synchronous, whichever happens first.
*/
void QContentSetEngine::update()
{
    if (m_flushTimerId == -1) {
        m_flushTimerId = startTimer(0);

        if (m_updateMode == QContentSet::Synchronous)
            emit reset();
    }
}

/*!
    \fn QContentSetEngine::contains( const QContent &content ) const

    Returns true if the given \a content is in the set.
*/

/*!
    \fn QContentSetEngine::sortOrder() const

    Returns the sort order of the set.
*/

/*!
    \fn QContentSetEngine::filter() const

    Returns the filtering criteria of the set.
 */

/*!
    \fn QContentSetEngine::contentAboutToBeRemoved( int start, int end )

    Signals that content between the \a start and \a end indexes are about to be removed from the set.
*/

/*!
    \fn QContentSetEngine::contentAboutToBeInserted( int start, int end )

    Signals that content is about to inserted into the set between the \a start and \a end indexes.
*/

/*!
    \fn QContentSetEngine::contentInserted()

    Signals that the content insertion indicated by the \l contentAboutToBeInserted() signal has completed.
 */

/*!
    \fn QContentSetEngine::contentRemoved()

    Signals that the content removal indicated by the \l contentAboutToBeRemoved() signal has completed.
*/

/*!
    \fn QContentSetEngine::contentChanged()

    Signals that the contents of the set have changed in some manner.
*/

/*!
    \fn QContentSetEngine::reset()

    Signals that the contents of the set have been reset.
*/

/*!
    \fn QContentSetEngine::sortCriteriaChanged(const QContentSortCriteria &sort)

    Indicates the content set's \a sort criteria has changed.

    Implementing classes should re-sort the set response to this event.

    \sa sortCriteria(), setSortCriteria()
*/

/*!
    \fn QContentSetEngine::sortCriteria() const

    Returns the criteria used to sort the content set.
*/

/*!
    \fn QContentSetEngine::filterChanged( const QContentFilter &filter )

    Indicates the content set's \a filter has changed.

    Implementing classes should re-filter the set in response to this event.

    \sa filter(), setFilter()
*/

/*!
    \fn QContentSetEngine::updateStarted()

    Signals that an update of the contents of the set has begun.

    \sa startUpdate(), updateInProgress(), updateFinished()
*/

/*!
    \fn QContentSetEngine::updateFinished()

    Signals that an update of the content of the set has finished.

    \sa finishUpdate(), updateInProgress(), updateStarted()
*/

/*!
    \fn QContentSetEngine::updateInProgress() const

    Returns true if the contents of the set are currently being updated.

    \sa updateStarted(), updateFinished()
*/

/*!
    Puts the content set in update mode.

    \sa updateStarted(), updateInProgress(), finishUpdate()
*/
void QContentSetEngine::startUpdate()
{
    m_updateInProgress = true;

    emit updateStarted();
}

/*!
    Takes the content set out of update mode.

    \sa updateFinished(), updateInProgress(), startUpdate()
*/
void QContentSetEngine::finishUpdate()
{
    m_updateInProgress = false;

    emit updateFinished();
}

void QContentSetEngine::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_flushTimerId) {
        event->accept();

        killTimer(m_flushTimerId);

        m_flushTimerId = -1;

        commitChanges();
    } else {
        QObject::timerEvent(event);
    }
}
