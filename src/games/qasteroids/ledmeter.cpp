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

#include <qpainter.h>
#include "ledmeter.h"
#include <QtDebug>

KALedMeter::KALedMeter(QWidget* parent) : QFrame(parent)
{
    maxRawValue_ = 1000;
    currentMeterLevel_ = 0;
    rawValue_ = 1000;
    meterLevels_ = 20;
    setMinimumWidth(meterLevels_ * 2 + frameWidth());
}

void KALedMeter::setMaxRawValue(int max)
{
    maxRawValue_ = max;
    if (maxRawValue_ < 1)
        maxRawValue_ = 1;
    setValue(rawValue_);
    update();
}

void KALedMeter::setMeterLevels(int count)
{
    meterLevels_ = count;
    if (meterLevels_ < 1)
        meterLevels_ = 1;
    setMinimumWidth(meterLevels_ * 2 + frameWidth());
    setValue(rawValue_);
    update();
}

void KALedMeter::setValue(int v)
{
    if (v > maxRawValue())
        v = maxRawValue();
    else if (v < 0)
        v = 0;
    rawValue_ = v;
    int c = (v + maxRawValue()/meterLevels() - 1) * meterLevels()/maxRawValue();
    if (c != currentMeterLevel_) {
        currentMeterLevel_ = c;
        update();
    }
}

void KALedMeter::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);
    int w = (width() - frameWidth() - 2) / meterLevels() * meterLevels();
    w += frameWidth() + 2;
    setFrameRect(QRect(0,0,w,height()));
}

void KALedMeter::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    drawContents();
}

void KALedMeter::drawContents()
{
    QPainter p(this);
    QRect b = contentsRect();

    int lw = b.width() / meterLevels();
    int lx = b.left() + 1;
    p.setBrush(Qt::black);
    p.setPen(Qt::green);
    int i = 0;
    if (currentMeterLevel_ > 1) {
        while (i < currentMeterLevel_) {
            p.drawRect(lx,b.top()+1,lw-1,b.height()-2);
            ++i;
            lx += lw;
        }
        p.setPen(Qt::darkGray);
    }
    else
        p.setPen(Qt::red);
    while (i < meterLevels()) {
        p.drawRect(lx,b.top()+1,lw-1,b.height()-2);
        ++i;
        lx += lw;
    }
}

