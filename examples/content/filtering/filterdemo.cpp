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

#include "filterdemo.h"
#include <QSoftMenuBar>
#include <QKeyEvent>

/*!
    Constructs a FilterDemo widget which is a child of \a parent and has the given window
    \a flags.
 */
FilterDemo::FilterDemo( QWidget *parent, Qt::WindowFlags flags )
    : QListView( parent )
    , index( 0 )
{
    setWindowFlags( flags );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    setModel( new QContentSetModel( &contentSet, this ) );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Next );

    nextFilter();
}

/*!
    Responds to a key press \a event.

    If the key pressed is the back key move to the next filter and accept the event. If it's
    another key, or the last filter has been displayed delegate the key handling to QListView.
*/
void FilterDemo::keyPressEvent( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Back && nextFilter() ) {
        event->accept();
    } else {
        QListView::keyPressEvent( event );
    }
}

/*!
    Apply the next set of filters.  If a filter has been applied returns true, otherwise if there
    are no filters remaining returns false.
*/
bool FilterDemo::nextFilter()
{
    switch( index++ )
    {
        // Filter for all applications.
    case 0:
        contentSet.setCriteria( QContentFilter( QContent::Application ) );
        return true;
        // Restrict the existing filter to only display content in the 'Games' category.
    case 1:
        contentSet.addCriteria( QContentFilter::Category, "Games", QContentFilter::And );
        return true;
        // Filter for documents with the 'image/jpeg' or 'image/png' MIME type.
    case 2:
        contentSet.setCriteria( QContentFilter::MimeType, "image/jpeg" );
        contentSet.addCriteria( QContentFilter::mimeType( "image/png" ), QContentFilter::Or );
        contentSet.addCriteria( QContentFilter( QContent::Document ), QContentFilter::And );
        return true;
        // Extend the existing filter to also include applications in the 'Games' category.
    case 3:
        contentSet.addCriteria( QContentFilter( QContent::Application )
                              & QContentFilter::category( "Games" )
                              , QContentFilter::Or );

        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Back );

        return true;
    default:
        return false;
    }
}
