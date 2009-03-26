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

#ifndef QGENERICVIDEOWIDGET_H
#define QGENERICVIDEOWIDGET_H

#include <QWidget>

#if defined(Q_WS_X11)
#include <QX11EmbedWidget>
#endif

#include "qtopiavideo.h"
#include "qvideoframe.h"

class QGenericVideoWidget :
#if defined(Q_WS_X11)
    public QX11EmbedWidget
#else
    public QWidget
#endif
{
    Q_OBJECT

public:
    QGenericVideoWidget( QWidget* parent = 0 );
    ~QGenericVideoWidget();

    void renderFrame( const QVideoFrame& );

    virtual QVideoFormatList preferredFormats() const;
    virtual QVideoFormatList supportedFormats() const;

    QtopiaVideo::VideoRotation rotation() const;
    QtopiaVideo::VideoScaleMode scaleMode() const;

public slots:
    void setRotation( QtopiaVideo::VideoRotation );
    void setScaleMode( QtopiaVideo::VideoScaleMode );

protected:
    void paintEvent(QPaintEvent* event);

private:
    QRect destRect();
    QImage m_buffer;
    QtopiaVideo::VideoRotation m_rotation;
    QtopiaVideo::VideoScaleMode m_scaleMode;
    double aspectRatio;
};

#endif
