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

#include "imageloader.h"
#include <QEvent>
#include <QApplication>
#include <QImageReader>
#include <QTime>
#include <QDebug>
#include <QScreen>

class ImageLoadedEvent : public QEvent
{
public:
    ImageLoadedEvent(const QImage &image, bool downscaled )
        : QEvent(User)
        , image(image)
        , downscaled(downscaled)
    {
    }

    QImage image;
    bool downscaled;
};

ImageLoader::ImageLoader( QObject *parent )
:QThread(parent), m_exiting(false)
{
    start();
}

ImageLoader::~ImageLoader()
{
    QMutexLocker lock(&m_mutex);
    m_contentToLoad = QContent();
    m_exiting = true;
    m_waitCondition.wakeAll();

    wait();
}

void ImageLoader::loadImage( const QContent& content, const QSize& displaySize )
{
    QMutexLocker lock(&m_mutex);
    m_contentToLoad = content;
    m_displaySize = displaySize;
    m_waitCondition.wakeAll();
}

void ImageLoader::customEvent(QEvent *event)
{
    if (event->type() == QEvent::User) {
        ImageLoadedEvent *e = static_cast<ImageLoadedEvent *>(event);
        emit loaded( e->image, e->downscaled );
        event->accept();
    }
}

void ImageLoader::run()
{
    for (;;) {
        QContent content;
        QSize displaySize;
        {
            QMutexLocker lock(&m_mutex);

            if ( m_contentToLoad.isNull()) {
                m_waitCondition.wait(&m_mutex);

                if ( m_exiting )
                    return;
            }
            content = m_contentToLoad;
            displaySize = m_displaySize;
            m_contentToLoad = QContent();
        }

        bool downscaled = false;

        if ( !content.isNull() ) {
            QIODevice *device = content.open();
            QImage image;

            if ( device ) {
                QImageReader reader(device);

                if (reader.canRead()) {
                    reader.setQuality(25);
                    QSize originalSize = reader.size();

                    if ( !displaySize.isNull() ) {
                        QSize scaledSize = originalSize;
                        scaledSize.scale( displaySize, Qt::KeepAspectRatio );

                        if ( scaledSize.width() ) {
                            int scaleFactor = originalSize.width() / scaledSize.width();
                            if ( scaleFactor < 2) {
                                scaleFactor = 1;
                            } else if (scaleFactor < 4) {
                                scaleFactor = 2;
                            } else if (scaleFactor < 8) {
                                scaleFactor = 4;
                            } else {
                                scaleFactor = 8;
                            }

                            if ( scaleFactor > 1 ) {
                                int width = (originalSize.width()+scaleFactor-1)/scaleFactor;
                                int height = (originalSize.height()+scaleFactor-1)/scaleFactor;
                                scaledSize = QSize(width,height);
                                if ( scaleFactor > 2 )
                                    reader.setQuality( 49 ); // Otherwise Qt smooth scales
                                reader.setScaledSize( scaledSize );
                            }
                        }
                    }
                    image = reader.read();
                    downscaled = image.size().width() < originalSize.width();
                }
                delete device;
            }

            bool haveNextRequest = false;

            if ( image.format() != QImage::Format_ARGB32_Premultiplied &&
                 image.format() != QImage::Format_RGB32 &&
                 image.format() != QImage::Format_RGB16 )
            {
                m_mutex.lock();
                haveNextRequest = !m_contentToLoad.isNull();
                m_mutex.unlock();

                if ( haveNextRequest )
                    continue;

                if ( image.format() == QImage::Format_ARGB32 ) {
                    image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
                } else if ( QScreen::instance()->depth() == 32 ) {
                    image = image.convertToFormat( QImage::Format_RGB32 );
                } else
                    image = image.convertToFormat( QImage::Format_RGB16 );
            }

            m_mutex.lock();
            haveNextRequest = !m_contentToLoad.isNull();
            m_mutex.unlock();

            if ( haveNextRequest )
                continue;

            QCoreApplication::postEvent(this, new ImageLoadedEvent(image,downscaled));
        }
    }
}

