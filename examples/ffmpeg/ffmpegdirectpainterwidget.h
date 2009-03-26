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

#ifndef __FFMPEG_DIRECTPAINTERSINK_H
#define __FFMPEG_DIRECTPAINTERSINK_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <QDirectPainter>

#include "ffmpegsinkwidget.h"

class QVideoFrame;
class QVideoSurface;

namespace ffmpeg
{

class DirectPainterWidget :
    public SinkWidget
{
public:
    DirectPainterWidget(QObject* parent = 0);
    ~DirectPainterWidget();

    // Sink widget
    QRect       m_viewPort;

    virtual QVideoSurface *videoSurface();

    void setVideoSize(int width, int height);
    void paint( const QVideoFrame& frame );

    virtual int windowId() const;

private:
    QVideoSurface *m_videoSurface;
};

}   // ns ffmpeg


#endif  // __FFMPEG_DIRECTPAINTERSINK_H
