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

#include "qmarqueelabel.h"

#include <QTimeLine>
#include <QImage>
#include <QPainter>
#include <QStylePainter>
#include <QPaintEvent>
#include <QDebug>
#include <QApplication>

struct QMarqueeLabelPrivate
{
    QString text;
    QImage *rotaterBitmap;
    QTimeLine *rotaterTimeLine;
    int steppingDuration;
    Qt::Alignment alignment;
    Qt::Alignment oldAlignment;
    int currentFrame;
};


QMarqueeLabel::QMarqueeLabel( QWidget* parent )
    : QWidget( parent )
{
    d = new QMarqueeLabelPrivate;
    d->steppingDuration = 300;
    setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    d->rotaterBitmap = new QImage;
    d->rotaterTimeLine = new QTimeLine;
    connect(d->rotaterTimeLine, SIGNAL(frameChanged(int)), this, SLOT(changeFrame(int)));
    connect(d->rotaterTimeLine, SIGNAL(finished()), this, SLOT(stopRotate()));
    connect(d->rotaterTimeLine, SIGNAL(finished()), this, SIGNAL(rotateFinished()));
}

QMarqueeLabel::~QMarqueeLabel( )
{
    delete d->rotaterBitmap;
    delete d->rotaterTimeLine;
    delete d;
}

void QMarqueeLabel::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );
    generateBitmap();
    if(d->rotaterTimeLine->state() != QTimeLine::NotRunning)
    {
        stopRotate();
        startRotate(d->steppingDuration);
    }
}

void QMarqueeLabel::generateBitmap()
{
    if(d->text.isEmpty())
        *(d->rotaterBitmap) = QImage();
    else
    {
        int textWidth=fontMetrics().boundingRect(d->text).width()+1;
        if(textWidth+3 > width())
        {
            QRect totalRect(0, 0, (textWidth+(5*fontMetrics().averageCharWidth()))*2, fontMetrics().height()+2);
            *(d->rotaterBitmap)=QImage(totalRect.size(), QImage::Format_ARGB32_Premultiplied);
            d->rotaterBitmap->fill(qRgba(0,0,0,25));
            QPainter painter(d->rotaterBitmap);
            drawOutline(&painter, QRect(0, 0, totalRect.width()/2, totalRect.height()), Qt::AlignLeft, d->text);
            drawOutline(&painter, QRect(totalRect.width()/2, 0, totalRect.width()/2, totalRect.height()), Qt::AlignLeft, d->text);
        }
        else
        {
            QRect totalRect(0, 0, textWidth, fontMetrics().height()+2);
            QImage img(totalRect.size(), QImage::Format_ARGB32_Premultiplied);
            *(d->rotaterBitmap)=QImage(totalRect.size(), QImage::Format_ARGB32_Premultiplied);
            d->rotaterBitmap->fill(qRgba(0,0,0,25));
            QPainter painter(d->rotaterBitmap);
            drawOutline(&painter, totalRect, alignment(), d->text);
        }
    }
    update();
}

void QMarqueeLabel::setText( const QString& text )
{
    if(d->text == text)
        return;
    d->text = text;
    stopRotate();
    if(d->rotaterTimeLine->state() == QTimeLine::NotRunning)
    {
        if(canRotate() && width() != 0)
        {
            //push rotate
            if(!d->oldAlignment)
                d->oldAlignment = alignment();
            setAlignment(Qt::AlignLeft);
        }
        else
        {
            if(!(!d->oldAlignment))
                setAlignment(d->oldAlignment);
        }
    }
    else
    {
        if(!(!d->oldAlignment))
            setAlignment(d->oldAlignment);
        startRotate(d->steppingDuration);
    }
    generateBitmap();
}

QString QMarqueeLabel::text () const
{
    return d->text;
}

bool QMarqueeLabel::isRotating()
{
    return d->rotaterTimeLine->state() != QTimeLine::NotRunning;
}

void QMarqueeLabel::startRotate(int msecPerChar)
{
    if(canRotate())
    {
        // clear out the QImage, and rework the bitmap over again.
        d->steppingDuration = msecPerChar;

        d->rotaterTimeLine->stop();
        d->rotaterTimeLine->setDuration((d->text.length()+5) * d->steppingDuration);
        d->rotaterTimeLine->setFrameRange( 0, (d->text.length() * 2) + 10 );
        d->rotaterTimeLine->setStartFrame(0);
        d->rotaterTimeLine->start();
        changeFrame(0);
    }
}

void QMarqueeLabel::startRotate()
{
    startRotate(300);
}

void QMarqueeLabel::stopRotate()
{
    if(d->rotaterTimeLine->state() != QTimeLine::NotRunning)
        d->rotaterTimeLine->stop();
    changeFrame(0);
    update();
}

void QMarqueeLabel::changeFrame(int frame)
{
    d->currentFrame = frame;
    update();
}

bool QMarqueeLabel::canRotate()
{
    return fontMetrics().boundingRect(d->text).width()+4 > width();
}

void QMarqueeLabel::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    if(isRotating())
    {
        int fromStart = (d->currentFrame * (d->rotaterBitmap->width()/2)) / d->rotaterTimeLine->endFrame();
        painter.drawImage(this->rect(), *(d->rotaterBitmap), QRect(QPoint(fromStart, 0), this->rect().size()));
    }
    else
    {
        QRect targetRect = QRect(0, 0, d->rotaterBitmap->width(), d->rotaterBitmap->height());
        if(alignment().testFlag(Qt::AlignHCenter) && !canRotate())
            targetRect = QRect((this->width()-d->rotaterBitmap->width())/2, 0, d->rotaterBitmap->width(), d->rotaterBitmap->height());
        else if(alignment().testFlag(Qt::AlignRight) && !canRotate())
            targetRect = QRect(this->width()-d->rotaterBitmap->width(), 0, d->rotaterBitmap->width(), d->rotaterBitmap->height());
        painter.drawImage(targetRect, *(d->rotaterBitmap), d->rotaterBitmap->rect());
    }
}

void QMarqueeLabel::drawOutline(QPainter *painter, const QRect &rect, int flags, const QString &text)
{
    if(d->text.isEmpty())
        return;
    // Cheaper to paint into pixmap and blit that four times than
    // to draw text four times.

    // First get correct image size for DPI of target.
    QImage img(QSize(1,1), QImage::Format_ARGB32_Premultiplied);
    QPainter ppm(&img);
    ppm.setFont(painter->font());
    QFontMetrics fm(ppm.font());
    QRect br = fm.boundingRect(rect, flags, text);
    ppm.end();

    // Now create proper image and paint.
    img = QImage(br.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(qRgba(0,0,0,0));
    ppm.begin(&img);
    ppm.setFont(painter->font());
    ppm.setPen(invertColor(palette().color(QPalette::Text)));
    ppm.drawText(QRect(QPoint(0,0), br.size()), flags, text);

    QPoint pos(br.topLeft());
    pos += QPoint(-1,0);
    painter->drawImage(pos, img);
    pos += QPoint(2,0);
    painter->drawImage(pos, img);
    pos += QPoint(-1,-1);
    painter->drawImage(pos, img);
    pos += QPoint(0,2);
    painter->drawImage(pos, img);

    img.fill(qRgba(0,0,0,0));
    ppm.setPen(palette().color(QPalette::Text));
    ppm.drawText(QRect(QPoint(0,0), br.size()), flags, text);

    pos += QPoint(0,-1);
    painter->drawImage(pos, img);
}

Qt::Alignment QMarqueeLabel::alignment () const
{
    return d->alignment;
}

void QMarqueeLabel::setAlignment ( Qt::Alignment value)
{
    if(alignment() != value) {
        d->alignment = value;
        generateBitmap();
    }
}

QSize QMarqueeLabel::sizeHint() const
{
    return fontMetrics().boundingRect(d->text).size();
}

QColor QMarqueeLabel::invertColor(const QColor& input)
{
    return QColor(255-input.red(), 255-input.green(), 255-input.blue(), input.alpha());
}

bool QMarqueeLabel::startRotating( QMarqueeLabel *firstpreference, QMarqueeLabel *secondpreference, QMarqueeLabel *thirdpreference, int msecPerChar)
{
    if(firstpreference && firstpreference->canRotate())
        firstpreference->startRotate(msecPerChar);
    else if(secondpreference && secondpreference->canRotate())
        secondpreference->startRotate(msecPerChar);
    else if(thirdpreference && thirdpreference->canRotate())
        thirdpreference->startRotate(msecPerChar);
    else
        return false;
    return true;
}
