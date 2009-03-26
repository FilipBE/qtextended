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
#ifndef PHOTOGALLERY_H
#define PHOTOGALLERY_H

#include "albummodel.h"
#include "imageselector.h"

#include <QWidget>
#include <QtopiaIpcAdaptor>
#include <QValueSpaceObject>

class QContent;
class QStackedLayout;
class AlbumSelector;
class ImageView;
class SlideShowView;
class ThumbCache;

class PhotoGalleryService : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    PhotoGalleryService(const QString &service, QObject *parent = 0);

protected:
    QString memberToMessage(const QByteArray& member);
    QString receiveChannel(const QString& channel);

private:
    const QString m_service;
};

class PhotoGallery : public QWidget
{
    Q_OBJECT
public:
    static const QColor blue;
    static const QColor orange;
    static const QColor lightGrey;

    PhotoGallery(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~PhotoGallery();

    static ThumbCache *thumbCache() { return m_instance->m_thumbCache; }

    static int clickThreshold() { return m_instance->m_clickThreshold; }
    static int controlsTimeout() { return 10000; }

    static int rotation(const QContent &image);
    static void setRotation(QContent *image, int rotation);

private slots:
    void showAlbumSelector();
    void albumSelected(const QString &name, const QString &categoryId);
    void showImageSelector();
    void imageSelected(const QContent &image);
    void showSlideShow(const QContent &image);
    void exitSlideShow();

    void albumSortModeChanged(AlbumModel::SortMode sort);
    void imageSelectorViewChanged(ImageSelector::View view);

private:
    enum GalleryView
    {
        AlbumSelectorSortedByDate,
        AlbumSelectorSortedByName,
        ImageSelectorIconView,
        ImageSelectorFlowView,
        ImageViewer,
        SlideShowViewer
    };

    static PhotoGallery *m_instance;

    const QString m_applicationGroup;
    const QString m_rotationKey;

    PhotoGalleryService m_galleryService;
    QValueSpaceObject m_galleryVS;

    QStackedLayout *m_stack;
    AlbumSelector *m_albumSelector;
    ImageSelector *m_imageSelector;
    ImageView *m_imageView;
    SlideShowView *m_slideShowView;
    QWidget *m_slideShowLauncher;
    ThumbCache *m_thumbCache;
    int m_clickThreshold;
};

#endif
