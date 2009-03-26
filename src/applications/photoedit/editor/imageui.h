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

#ifndef IMAGEUI_H
#define IMAGEUI_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qregion.h>

class ImageProcessor;

class ImageUI : public QWidget {
    Q_OBJECT
public:
    ImageUI( ImageProcessor*, QWidget* parent = 0, Qt::WFlags f = 0 );

    // Enable preview if true
    void setEnabled( bool b ) { enabled = b; }

    // Return current size of image
    QSize space() const { return _space.size(); }

    // Return current viewport
    QRect viewport() const { return _viewport; }

    // Return viewport contained within rect
    QRect viewport( const QRect& ) const;

    // Return position and dimensions of viewport in widget
    QRegion region() const;

    // Reset viewport centering on space
    void reset();

signals:
    // Image dimensions has changed
    void changed();

    // Preview has been updated
    void updated();

public slots:
    // Move viewport and update preview
    void moveViewportBy( int dx, int dy );

private slots:
    // Update viewport and reload preview
    void updateViewport();

protected:
    // Update widget buffer with preview from image processor
    void paintEvent( QPaintEvent* );

    // Resize viewport and update preview position
    void resizeEvent( QResizeEvent* );

private:
    // Paint preview onto buffer
    void updateBuffer();

    // Contain viewport within space
    void containViewport();

    ImageProcessor *image_processor;

    bool enabled;

    QRect _space, _viewport;

    QPixmap preview;
    QPoint viewport_center;
};

#endif
