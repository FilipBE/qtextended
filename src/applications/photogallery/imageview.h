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
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>
#include <QContent>

class QLabel;
class ZoomSlider;
class SmoothImageMover;
class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;
class TitleWindow;
class ImageLoader;

class ImageView : public QWidget
{
    Q_OBJECT
public:
    ImageView(QWidget *parent = 0);
    ~ImageView();

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel=0 );

    void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel* selectionModel();

public slots:
    void setImage( const QContent &image, int imageNumber=-1 );
    void setNextThumbnail( const QImage& );
    void setPreviousThumbnail( const QImage& );

    void rotateClockwise();
    void rotateAnticlockwise();
    void addToFavorites();

    void nextImage();
    void previousImage();

    void enableModelUpdates();
    void disableModelUpdates();

signals:
    void back();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void setImage(const QImage&, bool downscaled);
    void setZoom(int index);
    void zoomToFit();
    void setImageOffset(const QPoint&);
    void updateTransformation();
    void updateBoundaries();

    void titleVisibilityChanged(bool visible);

    void setAsBackgroundImage();
    void setAsAvatar();

    void setCurrentIndex(const QModelIndex&, const QModelIndex& );
    void setCurrentIndex(const QModelIndex&);
    void dataChanged( const QModelIndex&, const QModelIndex& );

private:
    bool copyImage(const QContent &image, const QString &target);

    QImage image() const;
    QImage nextThumbnail() const;
    QImage previousThumbnail() const;

    int nextImageNumber() const;
    int previousImageNumber() const;

    QAbstractItemModel *m_model;
    QItemSelectionModel *m_selectionModel;
    bool m_enableModelUpdates;

    QLabel *m_title;
    ZoomSlider *m_zoomSlider;
    TitleWindow *m_controls;
    int m_smoothUpdateTimerId;

    QContent m_content;
    QImage m_image;

    int m_currentImage;
    QMap<int, QImage> m_thumbnailsCache;
    QMap<int, QImage> m_screenSizeImageCache;

    int m_rotation;
    int m_zoom;
    QList<int> m_zoomSteps;

    QRect m_imageRect;
    QTransform m_transform;
    QTransform m_prevImageTransform;
    QTransform m_nextImagetransform;


    QPoint m_imageOffset;
    QPoint m_prevPainterOffset;
    QPoint m_pressedPos;

    SmoothImageMover *m_imageMover;

    ImageLoader *m_imageLoader;
    bool m_isLoading;
};

#endif

