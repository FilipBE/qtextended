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

#include <QVBoxLayout>
#include <QMediaContent>
#include <QMediaVideoControl>
#include <qtopiavideo.h>

#include "backend.h"

#include "videowidget.h"


namespace Phonon
{

namespace qtopiamedia
{

class VideoWidgetPrivate
{
public:
    Backend*            backend;
    QMediaContent*      content;
    QMediaVideoControl* control;
    Phonon::VideoWidget::ScaleMode  scaleMode;
    Phonon::VideoWidget::AspectRatio aspectRatio;
};

/*!
    \class Phonon::qtopiamedia::VideoWidget
    \internal
*/

VideoWidget::VideoWidget(Backend* backend, QWidget* parent):
    QWidget(parent),
    d(new VideoWidgetPrivate)
{
    d->backend = backend;
    d->content = 0;
    d->control = 0;
    d->scaleMode = Phonon::VideoWidget::FitInView;
    d->aspectRatio = Phonon::VideoWidget::AspectRatioAuto;

    setLayout(new QVBoxLayout);
}

VideoWidget::~VideoWidget()
{
    delete d;
}

Phonon::VideoWidget::AspectRatio VideoWidget::aspectRatio() const
{
    return d->aspectRatio;
}

void VideoWidget::setAspectRatio(Phonon::VideoWidget::AspectRatio)
{
}

qreal VideoWidget::brightness() const
{
    return 0;
}

void VideoWidget::setBrightness(qreal)
{
}

Phonon::VideoWidget::ScaleMode VideoWidget::scaleMode() const
{
    return d->scaleMode;
}

void VideoWidget::setScaleMode(Phonon::VideoWidget::ScaleMode scaleMode)
{
    d->scaleMode = scaleMode;

    if (d->control != 0) {
        switch (d->scaleMode) {
        case Phonon::VideoWidget::FitInView:
            d->control->setVideoScaleMode(QtopiaVideo::FitWindow);
            break;
        case Phonon::VideoWidget::ScaleAndCrop:
            d->control->setVideoScaleMode(QtopiaVideo::NoScale);
            break;
        }
    }
}

qreal VideoWidget::contrast() const
{
    return 0;
}

void VideoWidget::setContrast(qreal)
{
}

qreal VideoWidget::hue() const
{
    return 0;
}

void VideoWidget::setHue(qreal)
{
}

qreal VideoWidget::saturation() const
{
    return 0;
}

void VideoWidget::setSaturation(qreal)
{
}

QWidget* VideoWidget::widget()
{
    return this;
}

bool VideoWidget::connectNode(MediaNode* node)
{
    Q_UNUSED(node);
    return false;
}

bool VideoWidget::disconnectNode(MediaNode* node)
{
    Q_UNUSED(node);
    return false;
}

void VideoWidget::setContent(QMediaContent* content)
{
    if (d->content != content) {
        d->content = content;
        if (d->content != 0) {
            d->control = new QMediaVideoControl(d->content);
            connect(d->control, SIGNAL(valid()), SLOT(readyVideo()));
        }
    }
}

void VideoWidget::readyVideo()
{
    setScaleMode(d->scaleMode);

    QWidget* video = d->control->createVideoWidget(this);

    layout()->addWidget(video);
}

}

}

