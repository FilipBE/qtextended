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

#include <QSize>
#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QtGui/qscreen_qws.h>
#include <QDebug>

#include <custom.h>

#include "ffmpegdirectpainterwidget.h"
#include "ffmpegsinkwidget.h"

#include "qvideosurface.h"


namespace ffmpeg
{


/*!
    \class ffmpeg::DirectPainterWidget
    \internal
*/

DirectPainterWidget::DirectPainterWidget(QObject* parent)
{
    m_videoSurface = new QVideoSurface(0);
}

DirectPainterWidget::~DirectPainterWidget()
{
    delete m_videoSurface;
}

int DirectPainterWidget::windowId() const
{
    return m_videoSurface->winId();
}

void DirectPainterWidget::setVideoSize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

QVideoSurface *DirectPainterWidget::videoSurface()
{
    return m_videoSurface;
}

void DirectPainterWidget::paint(const QVideoFrame& frame)
{
    m_videoSurface->renderFrame( frame );
}

}   // ns ffmpeg
