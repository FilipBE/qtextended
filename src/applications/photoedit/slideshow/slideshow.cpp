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

#include "slideshow.h"

#include <qtopiaapplication.h>


SlideShow::SlideShow( QObject* parent )
    : QObject( parent )
    , collection_i( -1 )
    , m_isPlaying(false)
{
}

SlideShow::~SlideShow()
{
    // Ensure backlight dimmer is re-enabled.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
}

void SlideShow::setFirstImage( const QContent& image )
{
    // Find image in collection and update iterator with this image

    for( collection_i = 0; collection_i < collection.count(); collection_i++ )
    {
        if( collection.content( collection_i ).id() == image.id() )
            return;
    }
}

void SlideShow::start()
{
    if ( collection.count() == 0 ) {
        // No slides to play.
        return;
    }

    // Disable power save while playing so that the device
    // doesn't suspend before the slides finish.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);

    m_isPlaying = true;

    QContent image = collection.content(
            collection_i < collection.count() ? collection_i : 0);

    emit changed(image);
}

void SlideShow::stop()
{
    m_isPlaying = false;

    // Re-enable power save (that was disabled in start()).
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    emit stopped();
}

void SlideShow::advance()
{
    // If at end of collection and not looping through, stop slideshow
    // Otherwise, advance to next image in collection
    if (!m_isPlaying)
        return;

    collection_i++;
    if (collection_i == collection.count() && !loop_through)
    {
        stop();
    }
    else
    {
        if (collection.count() > 1)
        {
            //if(collection_i == collection.count())
            //    collection_i = 0;
            if(collection_i == collection.count()) {
                // Wrap around - and put the power saving back on, just in case it wraps
                // around forever.
                collection_i = 0;
                QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
            }

            QContent image = collection.content( collection_i );

            emit changed(image);

        }
    }
}

void SlideShow::imageLoaded()
{
    if (m_isPlaying)
        QTimer::singleShot(pause*FACTOR, this, SLOT(advance()));
}
