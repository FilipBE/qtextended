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

#include "changelistener.h"
#include <QSoftMenuBar>
#include <QContentSet>
#include <QTimer>
#include <QDir>
#include <QCategoryManager>
#include <QtDebug>

/*!
    \class ChangeListener
    \brief The ChangeListener application demonstrates listening for content changed events.

    It periodically installs images into the document system and listens for the content changed
    notifications generated in response.  When a notification is received for one of the images it
    installed it will display that image.
*/

/*!
    Constructs a ChangeListener widget which is a child of \a parent and has the given window
    \a flags.
 */
ChangeListener::ChangeListener( QWidget *parent, Qt::WindowFlags flags )
    : QLabel( parent, flags )
    , nextIndex( 0 )
    , lastContentId( QContent::InvalidId )
{
    setScaledContents( true );

    // Construct context menu, available to the user via Qtopia's soft menu bar.
     QSoftMenuBar::menuFor( this );

     // Populate the list of images to display.
    imageFiles.append( QFileInfo( ":image/Bubble.png" ) );
    imageFiles.append( QFileInfo( ":image/Clouds.png" ) );
    imageFiles.append( QFileInfo( ":image/Splatters.png" ) );
    imageFiles.append( QFileInfo( ":image/Water.png" ) );

    // Ensure the 'Change Listener' category exists in the the 'Examples' scope.
    QCategoryManager categoryManager( "Examples" );

    categoryId = categoryManager.idForLabel( "Change Listener" );

    if( categoryId.isEmpty() )
        categoryId = categoryManager.add( "Change Listener" );

    // Create a content set and listen to it's changed() signal.  Unistall any content in the
    // Change Listener category that may have been left over from an earlier run that was abnormally
    // terminated.
    QContentSet *contentSet = new QContentSet( QContentFilter::category( categoryId ), this );

    connect( contentSet, SIGNAL(changed(QContentIdList,QContent::ChangeType)),
            this, SLOT(changed(QContentIdList,QContent::ChangeType)) );

    for ( int i = 0; i < contentSet->count(); i++ ) {
        QContent::uninstall( contentSet->contentId( i ) );
    }

    // Construct a timer to time out at 3 second intervals.
    QTimer *timer = new QTimer( this );

    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    timer->start( 3000 );
}

/*!
    Destroys a ChangeListener widget.
 */
ChangeListener::~ChangeListener()
{
    if( lastContentId != QContent::InvalidId )
        QContent::uninstall( lastContentId );
}

/*!
    Installs an image to the document system and uninstalls the one previously installed in response
    to a timer time out.
*/
void ChangeListener::timeout()
{
    QFileInfo fileInfo = imageFiles.at( nextIndex );

    // At the least, we want to set the QContent's name and mime type. We'll also
    // give the object a category, so that all picture objects created by this application
    // can be grouped together and the QContent::Data role so it doesn't appear in document menus.
    QContent image;
    image.setName( fileInfo.baseName() );
    image.setFile( fileInfo.absoluteFilePath() );
    image.setType( "image/png" );
    image.setRole( QContent::Data );
    image.setCategories( QStringList( categoryId ) );

    if ( image.commit() ) {
        if ( lastContentId != QContent::InvalidId ) {
            QContent::uninstall( lastContentId );
        }
        lastContentId = image.id();
    } else {
        qWarning("Could not commit the new content object!! Document generator exits.");
    }

    nextIndex = (nextIndex + 1) % imageFiles.count();
}

/*!
    Responds to a content changed notification.  If the notification refers to an image installed by the
    timeout(), the image installed will be displayed.
*/
void ChangeListener::changed(const QContentIdList &idList,QContent::ChangeType changeType)
{
    if ( changeType == QContent::Added ) {
        foreach ( QContentId id, idList ) {
            QContent content( id );
            // Check that we've got a valid content object and that its category is the same
            // as that which was set in the generate() method.
            if ( content.isValid() && content.categories().contains( categoryId ) ) {
                setWindowTitle( content.name() );
                // Open the content in read-only mode.
                QIODevice *ioDevice = content.open( QIODevice::ReadOnly );
                if ( ioDevice ) {
                    // Read the image and display it.
                    QImage image;
                    image.load( ioDevice,"PNG" );
                    setPixmap( QPixmap::fromImage( image ) );

                    // Close and delete the I/O device.
                    ioDevice->close();
                    delete ioDevice;
                }

                // For the purposes of this example, we're only interested in one (and there
                // should only be one).
                break;
            }
        }
    }
}

