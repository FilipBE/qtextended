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

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <qcontent.h>
#include <qcontentset.h>
#include <QDrmContent>
#include <qobject.h>
#include <qtimer.h>

#define FACTOR 1000

class SlideShow : public QObject {
    Q_OBJECT
public:
    SlideShow( QObject* parent = 0 );

    ~SlideShow();

    // Set image collection
    void setCollection( const QContentSet& list ) { collection = list; }

    // Set first image to display
    void setFirstImage( const QContent& );

signals:
    // Current image has changed
    void changed( const QContent& );

    // Slide show has stopped
    void stopped();

public slots:
    // Start slide show
    void start();

    // Stop slide show
    void stop();

    // If true, loop through collection
    void setLoopThrough( bool b ) { loop_through = b; }

    // Set pause between images in seconds
    void setSlideLength( int sec ) { pause = sec; }

    void imageLoaded();
private slots:
    // Advance to next image or emit finished
    void advance();

private:
    bool loop_through;
    int pause;

    QContentSet collection;
    int collection_i;

    bool m_isPlaying;
};

#endif
