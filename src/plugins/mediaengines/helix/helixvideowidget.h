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

#ifndef HELIXVIDEOWIDGET_H
#define HELIXVIDEOWIDGET_H

#include <QWidget>

#include "helixvideosurface.h"

class QPluginManager;
class QVideoSurface;

class VideoWidget :
    public PaintObserver
{
public:
    VideoWidget(GenericVideoSurface* surface,
                             QWidget* parent = 0);
    ~VideoWidget();

    // Observer
    void paint( const QVideoFrame& frame );
    virtual QVideoFormatList preferredFormats() const;
    virtual QVideoFormatList supportedFormats() const;

    virtual QVideoSurface *videoSurface();

    int winId() const;

    static int isSupported();

    void repaintLastFrame() { m_surface->repaintLastFrame(); }

private:
    GenericVideoSurface *m_surface;
    QVideoSurface *m_videoSurface;
};

#endif
