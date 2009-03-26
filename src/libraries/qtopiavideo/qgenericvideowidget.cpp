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

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

#include "qgenericvideowidget.h"


QGenericVideoWidget::QGenericVideoWidget(QWidget* parent):
#if defined(Q_WS_X11)
    QX11EmbedWidget(parent),
#else
    QWidget(parent),
#endif
    m_rotation( QtopiaVideo::NoRotation ),
    m_scaleMode( QtopiaVideo::FitWindow ),
    aspectRatio(0)
{
    // Optimize paint event
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
}

QGenericVideoWidget::~QGenericVideoWidget()
{
}

QVideoFormatList QGenericVideoWidget::preferredFormats() const
{
    QVideoFormatList res;
    res << QVideoFrame::Format_RGB32 << QVideoFrame::Format_RGB24 << QVideoFrame::Format_RGB565;
    return res;
}

QVideoFormatList QGenericVideoWidget::supportedFormats() const
{
    return preferredFormats();
}

void QGenericVideoWidget::renderFrame( const QVideoFrame& frame )
{
    QImage::Format imageFormat = QImage::Format_Invalid;

    switch ( frame.format() ) {
    case QVideoFrame::Format_RGB565:
        imageFormat = QImage::Format_RGB16;
        break;
    case QVideoFrame::Format_RGB32:
        imageFormat = QImage::Format_RGB32;
        break;
    case QVideoFrame::Format_RGB24:
        imageFormat = QImage::Format_RGB888;
        break;
    default:
        break;
    }

    const uchar* imageData = frame.planeData(0);
    int bytesPerLine = frame.bytesPerLine(0);

    if ( imageFormat != QImage::Format_Invalid ) {
        m_buffer = QImage( imageData,
                           frame.size().width(),
                           frame.size().height(),
                           bytesPerLine,
                           imageFormat );
        m_buffer.detach();
    } else
        m_buffer = QImage();

    aspectRatio = frame.hasCustomAspectRatio() ? frame.aspectRatio() : 0;

    if (!isHidden())
        QWidget::update( destRect() );
}

QRect QGenericVideoWidget::destRect()
{
    QRect res;
    if ( !m_buffer.isNull() ) {
        QSize videoSize = m_buffer.size();
#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
        if ( aspectRatio )
            videoSize.setWidth( int(videoSize.height()*aspectRatio) );
        videoSize.scale(size(), Qt::KeepAspectRatio);
#endif
        res = QRect(QPoint(0, 0), videoSize);
        res.moveCenter(rect().center());
    }

    return res;
}

void QGenericVideoWidget::paintEvent(QPaintEvent* event)
{
    QPainter    p(this);

    const QVector<QRect> rects = (QRegion(event->rect()) ^ QRegion(destRect())).rects();
    for (QVector<QRect>::const_iterator it = rects.begin(); it != rects.end(); ++it)
        p.fillRect(*it, Qt::black);

    if (!m_buffer.isNull()) {
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawImage(destRect(), m_buffer);
    }
}

QtopiaVideo::VideoRotation QGenericVideoWidget::rotation() const
{
    return m_rotation;
}

QtopiaVideo::VideoScaleMode QGenericVideoWidget::scaleMode() const
{
    return m_scaleMode;
}

void QGenericVideoWidget::setRotation( QtopiaVideo::VideoRotation rotation )
{
    m_rotation = rotation;
    update();
}

void QGenericVideoWidget::setScaleMode( QtopiaVideo::VideoScaleMode scaleMode )
{
    m_scaleMode = scaleMode;
    update();
}

