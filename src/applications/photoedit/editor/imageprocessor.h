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

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "matrix.h"

#include <qobject.h>
#include <qpoint.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qimage.h>

class ImageIO;

class ImageProcessor : public QObject {
    Q_OBJECT
public:
    ImageProcessor( ImageIO*, QObject* parent = 0 );

    // Crop image to area within rect
    void crop( const QRect& );

    // Return point on transformed image given point on original
    QPoint map( const QPoint& ) const;

    // Return point on original image given point on transformed
    QPoint unmap( const QPoint& ) const;

    // Return a preview of the image with transformations applied at rect
    const QPixmap& preview( const QRect& rect ) const;

    // Return the image with transformations applied
    QImage image() const;

    // Return the image with transformation applied scaled to within size
    // The width to height ratio of the transformed image is preserved
    QImage image( const QSize& size ) const;

    // Return size of image with the current transformations applied
    QSize size() const;

    // Return true if transformed image is different from original
    bool isChanged() const;

signals:
    // Image has changed
    void changed();

public slots:
    // Set zoom factor for image
    // factor    0 < value
    void setZoom( double factor );

    // Set brightness factor for the image
    // factor    0 <= value <= 1
    void setBrightness( double factor );

    // Rotate image clockwise 90 degrees
    void rotate();

    void setCheckpoint();

private slots:
    // Reset transformations
    void reset();

private:
    // Return rect on transformed image given rect on original
    QRect map( const QRect& ) const;

    // Return rect on original image given rect on transformed
    QRect unmap( const QRect& ) const;

    QImage transform( const QImage&, const QRect& ) const;

    ImageIO *image_io;

    QRect viewport;

    double brightness_factor;
    double zoom_factor;
    Matrix transformation_matrix;

    struct
    {
        QRect viewport;
        double brightness_factor;
        Matrix transformation_matrix;
    } m_checkpoint;

    mutable QPixmap _preview;
};

#endif
