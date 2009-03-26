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
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QAbstractScrollArea>
#include <QThread>
#include <QDrmContent>
#include <QMutex>
#include <QWaitCondition>
#include <QPixmap>
#include <QPluginManager>

class QContent;
class ImageScalerProcessor;
class PhotoEditEffect;

class ImageScaler : public QThread
{
    Q_OBJECT
public:
    ImageScaler(QObject *parent = 0);
    ~ImageScaler();

    QContent content() const { return m_content; }
    QByteArray format() const { return m_format; }
    QSize size() const { return m_size; }
    qreal prescaling() const{ return m_prescaling; }

    QImage image() const;
    QImage image(const QSize &size) const;

public slots:
    void setContent(const QContent &content);
    void setEffect(const QString &plugin, const QString &effect, const QMap<QString, QVariant> &settings);

signals:
    void imageInvalidated();
    void imageChanged();

protected:
    void run();

private slots:
    void licenseExpired();
    void imageAvailable(
            const QContent &content,
            const QList<QImage> &images,
            const QSize &size,
            qreal prescaling);

    void effectApplied(const QList<QImage> &images);

private:
    ImageScalerProcessor *m_processor;
    QContent m_content;
    QDrmContent m_drmContent;
    QByteArray m_format;
    QSize m_size;
    qreal m_prescaling;
    bool m_isValid;
    QList<QImage> m_images;
    QList<QImage> m_editedImages;
    QMutex m_syncMutex;
    QWaitCondition m_syncCondition;
    QPluginManager m_effectManager;
    QMap<QString, PhotoEditEffect *> m_effects;

    friend class ImageScalerProcessor;
};

class ImageViewer : public QAbstractScrollArea
{
    Q_OBJECT
public:

    enum ScaleMode
    {
        FixedScale,
        ScaleToFit,
        ScaleRotateToFit
    };

    ImageViewer(ImageScaler *scaler, QWidget *parent = 0);
    virtual ~ImageViewer();

    ImageScaler *scaler() const;

    ScaleMode scaleMode() const;

    qreal rotation() const;
    qreal scaleX() const;
    qreal scaleY() const;
    QSize scaledSize() const;
    QSize transformedSize() const;

signals:
    void tapped();

public slots:
    void setScaleMode(ImageViewer::ScaleMode mode);
    void setScale(qreal sx, qreal sy);
    void setRotation( qreal rotation );

private slots:
    void imageChanged();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    void calculateScale();
    void calculateTransform();

    ImageScaler *m_scaler;
    ScaleMode m_scaleMode;
    qreal m_scaleX;
    qreal m_scaleY;
    qreal m_rotation;
    QSize m_scaledSize;
    QSize m_transformedSize;
    QRect m_screenRect;
    QRect m_destRect;
    QRect m_sourceRect;
    QPixmap m_pixmap;
    QRect m_pixmapRect;
    int m_updateTimerId;
    QPoint m_lastMousePos;
    bool m_tapTimerId;
};

#endif
