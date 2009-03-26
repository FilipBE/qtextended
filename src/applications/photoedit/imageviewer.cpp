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
#include "imageviewer.h"
#include "photoediteffect.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QPainter>
#include <QImageReader>
#include <QPaintEvent>
#include <QScrollBar>
#include <QtDebug>
#include <QDrmContent>
#include <QPluginManager>
#include <QCoreApplication>

Q_DECLARE_METATYPE(QList<QImage>);

class ImageScalerProcessor : public QObject
{
    Q_OBJECT
public:
    enum ScalerEvents
    {
        SetImage = QEvent::User,
        SetEffect
    };

    class SetImageEvent;
    class SetEffectEvent;

    ImageScalerProcessor(ImageScaler *scaler);

signals:
    void imageAvailable(const QContent &content, const QList<QImage> &images, const QSize &size, qreal prescaling);
    void effectApplied(const QList<QImage> &images);

protected:
    void customEvent(QEvent *event);

private:
    void setContent(const QContent &content);
    void applyEffect(
            const QString &plugin,
            const QString &effect,
            const QMap<QString, QVariant> &settings,
            const QImage &image);

    QList<QImage> scaleImage(const QImage &image) const;

    ImageScaler *m_scaler;
};


class ImageScalerProcessor::SetImageEvent : public QEvent
{
public:
    SetImageEvent(const QContent &image)
        : QEvent(QEvent::Type(SetImage))
        , m_image(image)
    {
    }

    QContent image() const{ return m_image; }

private:
    QContent m_image;
};

class ImageScalerProcessor::SetEffectEvent : public QEvent
{
public:
    SetEffectEvent(const QString &plugin, const QString &effect, const QMap<QString, QVariant> &settings, const QImage &image)
        : QEvent(QEvent::Type(SetEffect))
        , m_plugin(plugin)
        , m_effect(effect)
        , m_settings(settings)
        , m_image(image)
    {
    }

    QString plugin() const{ return m_plugin; }
    QString effect() const{ return m_effect; }
    QMap<QString, QVariant> settings() const{ return m_settings; }
    QImage image() const{ return m_image; }

private:
    QString m_plugin;
    QString m_effect;
    QMap<QString, QVariant> m_settings;
    QImage m_image;
};

ImageScalerProcessor::ImageScalerProcessor(ImageScaler *scaler)
    : m_scaler(scaler)
{
}

void ImageScalerProcessor::customEvent(QEvent *event)
{
    switch (event->type()) {
    case SetImage:
        setContent(static_cast<SetImageEvent *>(event)->image());

        event->accept();

        break;
    case SetEffect:
        {
            SetEffectEvent *e = static_cast<SetEffectEvent *>(event);

            applyEffect(e->plugin(), e->effect(), e->settings(), e->image());

            event->accept();
        }
        break;
    default:
        QObject::customEvent(event);
    }
}

void ImageScalerProcessor::setContent(const QContent &content)
{
    static const int maxArea = 2304000; // 1920*1200, 12,000 KB 32bpp Mip-mapped.

    QIODevice *device = 0;
    QImageReader reader;

    QSize size;
    qreal prescaling = 1.0;
    int area = 0;

    bool isValid = false;

    QDrmContent drmContent(QDrmRights::Display, QDrmContent::NoLicenseOptions);

    if (!content.isNull() && drmContent.requestLicense(content) && (device = content.open()) != 0) {
        reader.setDevice( device );

        size = reader.size();

        area = size.width() * size.height();

        isValid = reader.canRead()
            && area <= maxArea || reader.supportsOption(QImageIOHandler::ScaledSize);
    }

    while (isValid && area > maxArea) {
        prescaling /= 2.0;

        area = qRound(area * prescaling * prescaling);
    }

    QImage image;

    if (isValid) {
        if (prescaling < 1.0) {
            if ( prescaling < 0.5 )
                reader.setQuality( 49 ); // Otherwise Qt smooth scales
            reader.setScaledSize(size * prescaling);
        }

        reader.read(&image);
    }

    if (device)
        device->close();

    delete device;

    if (!image.isNull())
        emit imageAvailable(content, scaleImage(image), size, prescaling);
    else
        emit imageAvailable(content, QList<QImage>(), QSize(), 1.0);
}

void ImageScalerProcessor::applyEffect(
        const QString &plugin,
        const QString &effect,
        const QMap<QString, QVariant> &settings,
        const QImage &image)
{
    QImage editedImage = image;

    if (!plugin.isNull()) {
        m_scaler->m_syncMutex.lock();

        PhotoEditEffect *instance = m_scaler->m_effects.value(plugin);

        m_scaler->m_syncMutex.unlock();

        if (instance) {
            instance->applyEffect(effect, settings, &editedImage);

            QList<QImage> images = scaleImage(editedImage);

            if (!images.isEmpty())
                emit effectApplied(images);
        }
    }
}

QList<QImage> ImageScalerProcessor::scaleImage(const QImage &image) const
{
    QList<QImage> images;

    QImage scaledImage = image;

    images.append(scaledImage);

    while (qMax(scaledImage.width(), scaledImage.height()) > 120
        && qMin(scaledImage.width(), scaledImage.height()) > 10) {
        scaledImage = scaledImage.scaled(
                scaledImage.size() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if (!scaledImage.isNull()) {
            images.append(scaledImage);
        }
    }

    return images;
}

void ImageViewer::calculateScale()
{
    if (m_scaler->size().isValid()) {
        QSize bestFitSize = m_scaler->size();
        bestFitSize.scale(size(), Qt::KeepAspectRatio);

        qreal scale = qMin(qreal(1), qreal(bestFitSize.width()) / m_scaler->size().width());

        m_rotation = 0.0;

        if (m_scaleMode == ImageViewer::ScaleRotateToFit) {
            bestFitSize = m_scaler->size();
            bestFitSize.transpose();
            bestFitSize.scale(size(), Qt::KeepAspectRatio);

            qreal rotatedScale = qMin(qreal(1), qreal(bestFitSize.width()) / m_scaler->size().height());

            if (rotatedScale > scale) {
                scale = rotatedScale;

                m_rotation = -90.0;
            }
        }

        m_scaleX = scale;
        m_scaleY = scale;
    } else {
        m_scaleX   = 1.0;
        m_scaleY   = 1.0;
        m_rotation = 0.0;
    }

    calculateTransform();
}

void ImageViewer::calculateTransform()
{
    QRect imageRect(QPoint(0, 0), m_scaler->size());

    QTransform transform;

    transform.scale(m_scaleX, m_scaleY);

    m_scaledSize = transform.mapRect(imageRect).size();

    transform.rotate(m_rotation);

    QSize oldTransformedSize = m_transformedSize;

    m_transformedSize = transform.mapRect(imageRect).size();

    QScrollBar *hScroll = horizontalScrollBar();
    QScrollBar *vScroll = verticalScrollBar();

    if (m_scaler->size().isValid()) {
        int hValue = hScroll->value();
        int vValue = vScroll->value();

        QSize dSize = m_transformedSize - oldTransformedSize;

        hValue += dSize.width() / 2;
        vValue += dSize.height() / 2;

        dSize = m_transformedSize - size();

        hScroll->setRange(0, dSize.width());
        vScroll->setRange(0, dSize.height());
        hScroll->setValue(hValue );
        vScroll->setValue(vValue);
    } else {
        hScroll->setRange(0, 0);
        vScroll->setRange(0, 0);
    }
}

ImageScaler::ImageScaler(QObject *parent)
    : QThread(parent)
    , m_drmContent(QDrmRights::Display, QDrmContent::Reactivate)
    , m_prescaling(1.0)
    , m_effectManager("photoediteffects")
{
    static const int qImageListMetaId = qRegisterMetaType<QList<QImage> >();

    Q_UNUSED(qImageListMetaId);

    connect(&m_drmContent, SIGNAL(rightsExpired(QDrmContent)), this, SLOT(licenseExpired()));

    QMutexLocker locker(&m_syncMutex);

    start();

    m_syncCondition.wait(&m_syncMutex);
}

ImageScaler::~ImageScaler()
{
    quit();

    wait();
}

void ImageScaler::setContent(const QContent &content)
{
    m_images.clear();
    m_editedImages.clear();

    m_content = QContent();
    m_size = QSize();
    m_prescaling = 1.0;

    QContent image;

    QDrmContent drmContent(QDrmRights::Display, QDrmContent::Activate);

    if (!content.isNull() && drmContent.requestLicense(content)) {
        image = content;
    }

    QCoreApplication::postEvent(m_processor, new ImageScalerProcessor::SetImageEvent(image));
}

void ImageScaler::setEffect(const QString &plugin, const QString &effect, const QMap<QString, QVariant> &settings)
{
    m_editedImages.clear();

    if (m_images.isEmpty())
        return;

    if (PhotoEditEffect *instance = qobject_cast<PhotoEditEffect *>(m_effectManager.instance(plugin))) {
        QMutexLocker locker(&m_syncMutex);

        m_effects[plugin] = instance;

        QCoreApplication::postEvent(m_processor, new ImageScalerProcessor::SetEffectEvent(
                plugin, effect, settings, m_images.first()));
    }
}

QImage ImageScaler::image() const
{
    if (!m_editedImages.isEmpty())
        return m_editedImages.first();
    else if (!m_images.isEmpty())
        return m_images.first();
    else
        return QImage();
}

QImage ImageScaler::image(const QSize &size) const
{
    QList<QImage> images = !m_editedImages.isEmpty()
            ? m_editedImages
            : m_images;

    QImage image;

    for (int i = images.count() - 1;
        i >= 0 && (image.height() < size.height() || image.width() < size.width());
        --i) {
        image = images.at(i);
    }

    return image;
}

void ImageScaler::licenseExpired()
{
    m_images.clear();

    emit imageInvalidated();
}

void ImageScaler::imageAvailable(const QContent &content, const QList<QImage> &images, const QSize &size, qreal prescaling)
{
    m_content = content;
    m_images = images;
    m_size = size;
    m_prescaling = prescaling;

    if (m_drmContent.requestLicense(content)) {
        m_drmContent.renderStarted();

        emit imageChanged();
    } else {
        licenseExpired();
    }
}

void ImageScaler::effectApplied(const QList<QImage> &images)
{
    m_editedImages = images;

    emit imageChanged();
}

void ImageScaler::run()
{
    ImageScalerProcessor processor(this);

    m_processor = &processor;

    connect(m_processor, SIGNAL(imageAvailable(QContent,QList<QImage>,QSize,qreal)),
            this, SLOT(imageAvailable(QContent,QList<QImage>,QSize,qreal)));
    connect(m_processor, SIGNAL(effectApplied(QList<QImage>)),
            this, SLOT(effectApplied(QList<QImage>)));
    {
        QMutexLocker locker(&m_syncMutex);

        m_syncCondition.wakeAll();
    }

    exec();
}

ImageViewer::ImageViewer(ImageScaler *scaler, QWidget *parent)
    : QAbstractScrollArea(parent)
    , m_scaler(scaler)
    , m_scaleMode(ScaleToFit)
    , m_scaleX(1.0)
    , m_scaleY(1.0)
    , m_rotation(0.0)
    , m_tapTimerId(-1)
{
    setMinimumSize( 32, 32 );
    setFrameStyle( QFrame::NoFrame );

    horizontalScrollBar()->setSingleStep( 10 );
    verticalScrollBar()->setSingleStep( 10 );

    connect(scaler, SIGNAL(imageInvalidated()), this, SLOT(imageChanged()));
    connect(scaler, SIGNAL(imageChanged()), this, SLOT(imageChanged()));
}

ImageViewer::~ImageViewer()
{
}

qreal ImageViewer::scaleX() const
{
    return m_scaleX;
}

qreal ImageViewer::scaleY() const
{
    return m_scaleY;
}

QSize ImageViewer::scaledSize() const
{
    return m_scaledSize;
}

QSize ImageViewer::transformedSize() const
{
    return m_transformedSize;
}

void ImageViewer::setScale( qreal sx, qreal sy )
{
    m_scaleMode = FixedScale;

    if (sx != m_scaleX && m_scaleY != sy) {
        m_scaleX = sx;
        m_scaleY = sy;

        calculateTransform();

        viewport()->update();
    }
}

qreal ImageViewer::rotation() const
{
    return m_rotation;
}

void ImageViewer::setRotation(qreal rotation)
{
    m_scaleMode = FixedScale;

    if (rotation != m_rotation) {
        m_rotation = rotation;

        calculateTransform();

        viewport()->update();
    }
}

ImageViewer::ScaleMode ImageViewer::scaleMode() const
{
    return m_scaleMode;
}

void ImageViewer::setScaleMode(ScaleMode mode)
{
    m_scaleMode = mode;

    if (mode != FixedScale) {
        calculateScale();

        viewport()->update();
    }
}

void ImageViewer::paintEvent( QPaintEvent *event )
{
    QImage image = m_scaler->image(m_scaledSize);

    if(!image.isNull()) {
        QPainter painter(viewport());

        painter.setClipRegion(event->region());

        QTransform transform;

        transform.translate(
                -horizontalScrollBar()->value() + qMax(m_transformedSize.width(),  width())  / 2,
                -verticalScrollBar()->value()   + qMax(m_transformedSize.height(), height()) / 2 );
        transform.rotate(m_rotation);
        transform.translate(-m_scaledSize.width() / 2, -m_scaledSize.height() / 2);

        painter.setWorldTransform(transform, true);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(QRect(QPoint(0,0), m_scaledSize), image);

        event->accept();
    } else {
        QAbstractScrollArea::paintEvent(event);
    }
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QScrollBar *hScroll = horizontalScrollBar();
    QScrollBar *vScroll = verticalScrollBar();

    int hValue = hScroll->value();
    int vValue = vScroll->value();

    QSize dSize = m_transformedSize - event->size();

    hScroll->setRange(0, dSize.width());
    vScroll->setRange(0, dSize.height());

    dSize = event->size() - event->oldSize();

    hScroll->setValue(hValue + dSize.width()  / 2);
    vScroll->setValue(vValue + dSize.height() / 2);

    hScroll->setPageStep(width());
    vScroll->setPageStep(height());

    QAbstractScrollArea::resizeEvent(event);

    if(m_scaleMode != FixedScale) {
        calculateScale();

        viewport()->update();
    }
}

void ImageViewer::showEvent(QShowEvent *event)
{
    QAbstractScrollArea::showEvent(event);

    viewport()->update();
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{ 
    QAbstractScrollArea::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();

        m_tapTimerId = startTimer(100);
    }
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    QAbstractScrollArea::mouseMoveEvent( event );

    if (!m_lastMousePos.isNull()) {
        QPoint dPos = event->pos() - m_lastMousePos;

        QScrollBar *hScroll = horizontalScrollBar();
        QScrollBar *vScroll = verticalScrollBar();

        hScroll->setValue(hScroll->value() - dPos.x());
        vScroll->setValue(vScroll->value() - dPos.y());

        m_lastMousePos = event->pos();
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractScrollArea::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = QPoint();

        if (m_tapTimerId != -1) {
            killTimer(m_tapTimerId);

            m_tapTimerId = -1;

            emit tapped();
        }
    }
}

void ImageViewer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_tapTimerId) {
        killTimer(m_tapTimerId);

        m_tapTimerId = -1;

        event->accept();
    } else {
        QAbstractScrollArea::timerEvent(event);
    }
}

void ImageViewer::imageChanged()
{
    calculateScale();

    viewport()->update();
}

#include "imageviewer.moc"
