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
#include "durationslider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>

DurationSlider::DurationSlider(QWidget *parent)
    : QAbstractSlider(parent)
    , m_transparency(100)
{
    connect(&m_fadeOut, SIGNAL(frameChanged(int)), this, SLOT(setTransparency(int)));
    connect(&m_fadeOut, SIGNAL(finished()), this, SLOT(fadeOutFinished()));

    m_fadeOut.setFrameRange(0, 100);
}

void DurationSlider::paintEvent(QPaintEvent *event)
{
    if (isSliderDown()) {
        const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        const int margin = style()->pixelMetric(QStyle::PM_ButtonIconSize);
        const int height = style()->pixelMetric(QStyle::PM_ButtonIconSize) + 2 * frameWidth;

        QPainter painter(this);
        painter.setOpacity(qreal(100 - m_transparency) / 100.0);

        QRect r = rect();
        r.adjust(margin, r.height() - margin - height, -margin, -margin);

        const int radius = r.height() / 5;

        painter.setPen(QPen(palette().windowText(), frameWidth));
        painter.drawRoundedRect(r, radius, radius);

        QRect valueRect = r;

        valueRect.adjust(frameWidth * 2, frameWidth * 2, -frameWidth * 2, -frameWidth * 2);
        valueRect.setWidth(QStyle::sliderPositionFromValue(0, maximum(), sliderPosition(), valueRect.width()));

        painter.setBrush(palette().highlight());
        painter.drawRoundedRect(valueRect, radius, radius);

        QFont f = font();
        f.setBold(true);

        painter.setFont(f);
        painter.setPen(QPen(palette().color(QPalette::HighlightedText)));
        painter.drawText(r, Qt::AlignCenter, tr("%1:%2", "minutes:seconds")
                .arg(sliderPosition() / 60, 2, 10, QLatin1Char('0'))
                .arg(sliderPosition() % 60, 2, 10, QLatin1Char('0')));
    }
    event->accept();
}

void DurationSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressedPosition = event->pos();

        event->ignore();
    } else {
        QAbstractSlider::mousePressEvent(event);
    }
}

void DurationSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_transparency == 100 && (event->pos() - m_pressedPosition).manhattanLength() > 12) {
            setSliderDown(true);

            if (m_fadeOut.state() == QTimeLine::Running)
                m_fadeOut.stop();

            setTransparency(0);
        }

        if (isSliderDown()) {
            const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            const int margin = style()->pixelMetric(QStyle::PM_ButtonIconSize) - frameWidth * 2;
            const int height = style()->pixelMetric(QStyle::PM_ButtonIconSize) + 2 * frameWidth;

            QRect r = rect();
            r.adjust(margin, r.height() - margin - height, -margin, -margin);

            setSliderPosition(QStyle::sliderValueFromPosition(
                    0, maximum(), event->pos().x() - r.left(), r.width()));
        }

        event->ignore();
    } else {
        QAbstractSlider::mouseMoveEvent(event);
    }
}

void DurationSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_fadeOut.state() != QTimeLine::Running)
            m_fadeOut.start();

        event->ignore();
    } else {
        QAbstractSlider::mouseReleaseEvent(event);
    }
}

void DurationSlider::setTransparency(int transparency)
{
    m_transparency = transparency;

    update();
}

void DurationSlider::fadeOutFinished()
{
    setSliderDown(false);
}
