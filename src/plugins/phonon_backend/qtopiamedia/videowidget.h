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

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>

#include <phonon/videowidget.h>
#include <phonon/videowidgetinterface.h>

#include "medianode.h"


namespace Phonon
{

namespace qtopiamedia
{

class Backend;

class VideoWidgetPrivate;

class VideoWidget :
    public QWidget,
    public VideoWidgetInterface,
    public MediaNode
{
    Q_OBJECT
    Q_INTERFACES(Phonon::VideoWidgetInterface Phonon::qtopiamedia::MediaNode)

public:
    VideoWidget(Backend* backend, QWidget* parent);
    ~VideoWidget();

    // VideoWidgetInterface
    Phonon::VideoWidget::AspectRatio aspectRatio() const;
    void setAspectRatio(Phonon::VideoWidget::AspectRatio);
    qreal brightness() const;
    void setBrightness(qreal);
    Phonon::VideoWidget::ScaleMode scaleMode() const;
    void setScaleMode(Phonon::VideoWidget::ScaleMode);
    qreal contrast() const;
    void setContrast(qreal);
    qreal hue() const;
    void setHue(qreal);
    qreal saturation() const;
    void setSaturation(qreal);
    QWidget *widget();

    // MediaNode
    bool connectNode(MediaNode* node);
    bool disconnectNode(MediaNode* node);
    void setContent(QMediaContent* content);

private slots:
    void readyVideo();

private:
    VideoWidgetPrivate* d;
};

}

}

#endif  // VIDEOWIDGET_H
