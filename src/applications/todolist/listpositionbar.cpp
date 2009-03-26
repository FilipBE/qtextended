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

#include "listpositionbar.h"
#include <QPainter>
#include <QIcon>
#include <QPaintEvent>
#include <QStyle>

ListPositionBar::ListPositionBar(QWidget *parent)
 : QWidget(parent), mCurrent(0), mMax(0), mShowPrev(false), mShowNext(false)
{
    // We'd like around 6pt, but a minimum of 16px for legibility in CJK fonts etc
    // 16px @ 72dpi == 16pt
    // 16px @ 144 dpi == 8pt
    // 16px @ foo dpi == 16 / (dpi/72) == (16 * 72 / dpi)pt
    QFont f(font());
    qreal ptsize = 6;
    if (logicalDpiY() > 0 && (16.0 * 72)/logicalDpiY() > ptsize)
        ptsize = (16.0*72) / logicalDpiY();
    f.setPointSizeF(ptsize);
    setFont(f);

    mMetric = qMax(fontMetrics().height(), style()->pixelMetric(QStyle::PM_ToolBarIconSize));
    setMinimumHeight(mMetric + 2);

    mLeftPixmap = QIcon(":icon/left").pixmap(mMetric);
    mRightPixmap = QIcon(":icon/right").pixmap(mMetric);
    setFocusPolicy(Qt::NoFocus);

    setMessage(tr("%1 of %2", "page 1 of 2"));
    setPosition(0,0);
}

ListPositionBar::~ListPositionBar()
{
}

void ListPositionBar::setMessage(const QString& format)
{
    mFormat = format;
    setPosition(mCurrent, mMax);
}

void ListPositionBar::setPosition(int current, int max)
{
    mMax = max;
    mCurrent = current;

    if (max <= 1 || current < 1) {
        hide();
    } else {
        show();
        parentWidget()->updateGeometry();
        update();
    }

    if (current > 1)
        mShowPrev = true;
    else
        mShowPrev = false;

    if (current < max)
        mShowNext = true;
    else
        mShowNext = false;

    mPosition = mFormat.arg(current).arg(max);
}

void ListPositionBar::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == Qt::LeftButton) {
        bool next = false;
        if (me->x() < width()/ 2) {
            if (layoutDirection() == Qt::RightToLeft)
                next = true;
        } else {
            if (layoutDirection() == Qt::LeftToRight)
                next = true;
        }

        if (next)
            emit nextPosition();
        else
            emit previousPosition();
    }
}

void ListPositionBar::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);
    QPainter p(this);

    p.fillRect(pe->rect(), palette().alternateBase());

    QRect pixmaprect(0, 2, mMetric,mMetric);
    if (layoutDirection() == Qt::RightToLeft) {
        if (mShowPrev)
            p.drawPixmap(pixmaprect.adjusted(rect().right() - mMetric - 1, 0, 0, 0), mRightPixmap);
        if (mShowNext)
            p.drawPixmap(pixmaprect.adjusted(1,0,0,0), mLeftPixmap);
    } else {
        if (mShowNext)
            p.drawPixmap(pixmaprect.adjusted(rect().right() - mMetric - 1, 0, 0, 0), mRightPixmap);
        if (mShowPrev)
            p.drawPixmap(pixmaprect.adjusted(1,0,0,0), mLeftPixmap);
    }

    p.drawText(rect(), Qt::AlignCenter, mPosition);
}

//===========================================================================

