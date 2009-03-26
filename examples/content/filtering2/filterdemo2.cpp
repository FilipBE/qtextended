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

#include "filterdemo2.h"
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

    QMediaPlaylist playlist(QContentFilter(QContent::Document));
    mediaList = new QMediaList(playlist);
    
    setModel( mediaList );

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
    case 0:
        qWarning("TEST: Artist : Billy, Album : Songs (randomized)");
        mediaList->clearFilter();
        mediaList->setDisplayRole(QMediaPlaylist::Title);
        mediaList->beginFilter();
        mediaList->setFilter(QMediaList::MimeType,"audio");
        mediaList->addFilter(QMediaList::Artist, "Billy*");
        mediaList->addFilter(QMediaList::Album, "Songs*");
        mediaList->randomize();
        mediaList->endFilter();
        return true;
    case 1:
        qWarning("TEST: all audio files (ascending order)");
        mediaList->clearFilter();
        mediaList->setSorting(QMediaList::Ascending);
        mediaList->setFilter(QMediaList::MimeType,"audio");
        return true;
    case 2:
        qWarning("TEST: Artist : Hunters");
        mediaList->setDisplayRole(QMediaPlaylist::Album);
        mediaList->addFilter(QMediaList::Artist, "Hunters*");
        return true;
    case 3:
        qWarning("TEST: Album : Greatest Hits (descending order)");
        mediaList->clearFilter();
        mediaList->beginFilter();
        mediaList->setDisplayRole(QMediaPlaylist::Title);
        mediaList->setFilter(QMediaList::MimeType,"audio");
        mediaList->addFilter(QMediaList::Album, "Greatest Hits*");
        mediaList->setSorting(QMediaList::Descending);
        mediaList->endFilter();

        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Back );

        return true;
    default:
        return false;
    }
}
