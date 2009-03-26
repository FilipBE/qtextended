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
#include "photogallery.h"
#include "thumbcache.h"
#include "albumselector.h"
#include "imageselector.h"
#include "imageview.h"
#include "slideshowview.h"

#include <QSettings>
#include <QtopiaService>
#include <QStackedLayout>

PhotoGalleryService::PhotoGalleryService(const QString &service, QObject *parent)
    : QtopiaIpcAdaptor(service, parent)
    , m_service(service)
{
}

QString PhotoGalleryService::memberToMessage(const QByteArray& member)
{
    return m_service + "::" + QtopiaIpcAdaptor::memberToMessage(member);
}

QString PhotoGalleryService::receiveChannel(const QString& channel)
{
    Q_UNUSED(channel);

    return QString();
}

const QColor PhotoGallery::blue(0, 100, 146);
const QColor PhotoGallery::orange(254, 152, 23);
const QColor PhotoGallery::lightGrey(230, 230, 230);

PhotoGallery *PhotoGallery::m_instance = 0;

PhotoGallery::PhotoGallery(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_applicationGroup(QLatin1String("PhotoGallery"))
    , m_rotationKey(QLatin1String("Rotation"))
    , m_galleryService(QLatin1String("PhotoGallery"))
    , m_galleryVS("/PhotoGallery")
    , m_albumSelector(0)
    , m_imageSelector(0)
    , m_imageView(0)
    , m_slideShowView(0)
    , m_slideShowLauncher(0)
    , m_thumbCache(new ThumbCache(this))
{
    m_instance = this;

    // 2mm threshold.
    m_clickThreshold = (physicalDpiX() + physicalDpiY()) / 25;

    setLayout(m_stack = new QStackedLayout);

    showAlbumSelector();
}

PhotoGallery::~PhotoGallery()
{
    if (m_slideShowView) {
        QSettings settings(QLatin1String("Trolltech"), QLatin1String("PhotoGallery"));
        settings.beginGroup(QLatin1String("SlideShow"));
        settings.setValue(QLatin1String("Timeout"), m_slideShowView->timeout());
    }

    m_instance = 0;
}

int PhotoGallery::rotation(const QContent &image)
{
    return image.property(m_instance->m_rotationKey, m_instance->m_applicationGroup).toInt();
}

void PhotoGallery::setRotation(QContent *image, int rotation)
{
    image->setProperty(
                m_instance->m_rotationKey,
                QString::number(rotation),
                m_instance->m_applicationGroup);
    image->commit();
}

void PhotoGallery::showAlbumSelector()
{
    if (!m_albumSelector) {
        m_albumSelector = new AlbumSelector;

        connect(m_albumSelector, SIGNAL(sortModeChanged(AlbumModel::SortMode)),
                this, SLOT(albumSortModeChanged(AlbumModel::SortMode)));
        connect(m_albumSelector, SIGNAL(albumSelected(QString,QString)),
                this, SLOT(albumSelected(QString,QString)));

        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(sortAlbumsByDate()),
                m_albumSelector, SLOT(sortByDate()));
        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(sortAlbumsByName()),
                m_albumSelector, SLOT(sortByName()));

        m_stack->addWidget(m_albumSelector);
    }

    albumSortModeChanged(m_albumSelector->sortMode());

    showMaximized();

    m_stack->setCurrentWidget(m_albumSelector);

    if (m_imageSelector)
        m_imageSelector->setAlbum(QString(), QString());
}

void PhotoGallery::albumSelected(const QString &name, const QString &categoryId)
{
    showImageSelector();

    m_imageSelector->setAlbum(name, categoryId);
}

void PhotoGallery::showImageSelector()
{
    if (!m_imageSelector) {
        m_imageSelector = new ImageSelector;

        connect(m_imageSelector, SIGNAL(viewChanged(ImageSelector::View)),
                this, SLOT(imageSelectorViewChanged(ImageSelector::View)));
        connect(m_imageSelector, SIGNAL(back()), this, SLOT(showAlbumSelector()));
        connect(m_imageSelector, SIGNAL(selected(QContent)), this, SLOT(imageSelected(QContent)));
        connect(m_imageSelector, SIGNAL(startSlideShow(QContent)),
                this, SLOT(showSlideShow(QContent)));

        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(imageSelectorRotateACW()),
                m_imageSelector, SLOT(rotateAnticlockwise()));
        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(imageSelectorRotateCW()),
                m_imageSelector, SLOT(rotateClockwise()));
        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(imageSelectorSlideShow()),
                m_imageSelector, SLOT(startSlideShow()));

        m_stack->addWidget(m_imageSelector);
    }

    imageSelectorViewChanged(m_imageSelector->view());

    showMaximized();
    m_stack->setCurrentWidget(m_imageSelector);

    if ( m_slideShowView )
        m_slideShowView->disableModelUpdates();

    if ( m_imageView )
        m_imageView->disableModelUpdates();
}

void PhotoGallery::imageSelected(const QContent &image)
{
    Q_UNUSED(image);

    if (!m_imageView) {
        m_imageView = new ImageView;

        m_imageView->setModel( m_imageSelector->model(), m_imageSelector->selectionModel() );

        connect(m_imageView, SIGNAL(back()), this, SLOT(showImageSelector()));

        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(imageViewRotateACW()),
                m_imageView, SLOT(rotateAnticlockwise()));
        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(imageViewRotateCW()),
                m_imageView, SLOT(rotateClockwise()));
        QtopiaIpcAdaptor::connect(
                &m_galleryService, MESSAGE(imageViewAddToFavorites()),
                m_imageView, SLOT(addToFavorites()));

        m_stack->addWidget(m_imageView);
    }

    if ( m_slideShowView )
        m_slideShowView->disableModelUpdates();

    m_galleryVS.setAttribute("View", int(ImageViewer));

    m_stack->setCurrentWidget(m_imageView);
    showFullScreen();
    m_imageView->enableModelUpdates();
}

void PhotoGallery::showSlideShow(const QContent &image)
{
    Q_UNUSED(image);
    if (!m_slideShowView) {
        QSettings settings(QLatin1String("Trolltech"), QLatin1String("PhotoGallery"));
        settings.beginGroup(QLatin1String("SlideShow"));

        m_slideShowView = new SlideShowView;
        m_slideShowView->setTimeout(settings.value(QLatin1String("Timeout"), 20).toInt());

        m_slideShowView->setModel( m_imageSelector->model(), m_imageSelector->selectionModel() );

        connect(m_slideShowView, SIGNAL(back()), this, SLOT(exitSlideShow()));

        m_stack->addWidget(m_slideShowView);
    }

    if ( m_imageView )
        m_imageView->disableModelUpdates();

    m_slideShowLauncher = m_stack->currentWidget();

    m_galleryVS.setAttribute("View", int(SlideShowViewer));

    showFullScreen();
    m_stack->setCurrentWidget(m_slideShowView);
    raise();
    m_slideShowView->enableModelUpdates();
    m_slideShowView->start();
}

void PhotoGallery::exitSlideShow()
{
    if (m_slideShowLauncher == m_imageView)
        imageSelected(QContent());
    else if (m_slideShowLauncher == m_imageSelector)
        showImageSelector();
    else if (m_slideShowLauncher == m_albumSelector)
        showAlbumSelector();
    else
        hide();
}

void PhotoGallery::albumSortModeChanged(AlbumModel::SortMode sort)
{
    switch (sort) {
    case AlbumModel::SortByDate:
        m_galleryVS.setAttribute("View", int(AlbumSelectorSortedByDate));
        break;
    case AlbumModel::SortByName:
        m_galleryVS.setAttribute("View", int(AlbumSelectorSortedByName));
        break;
    }
}

void PhotoGallery::imageSelectorViewChanged(ImageSelector::View view)
{
    switch (view) {
    case ImageSelector::IconView:
        m_galleryVS.setAttribute("View", int(ImageSelectorIconView));
        break;
#ifdef USE_PICTUREFLOW
    case ImageSelector::FlowView:
        m_galleryVS.setAttribute("View", int(ImageSelectorFlowView));
        break;
#endif
    }
}
