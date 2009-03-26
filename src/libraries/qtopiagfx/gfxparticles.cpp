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

#include "gfxparticles.h"
#include <QList>
#include <QRect>
#include <QImage>
#include <QPoint>
#include "gfxpainter.h"
#include <gfximage.h>
#include <QDebug>
#include <QPainter>
#include <QLinearGradient>

struct Star
{
    qreal value;
    qreal rate;
    qreal max;
    qreal falling;
    qreal falling_x;
    int size;
    QPoint pos;
    QPoint destpos;
};

struct GfxParticlesPrivate
{
    GfxParticlesPrivate()
    : particles(0) {}

    QRect rect;
    unsigned int particles;
    QList<Star> stars;
    QList<QImage> imgs;
};

GfxParticles::GfxParticles()
: d(new GfxParticlesPrivate)
{
}

GfxParticles::~GfxParticles()
{
    delete d;
    d = 0;
}

static qreal valueToOpacity(qreal val)
{
    if(val <= 0.3f) {
        return val / 0.3f;
    } else if(val > 0.3f && val <= 0.5f) {
        val -= 0.3f;
        val /= 0.2f;
        if(val <= 0.5f)
            return 1.0f - 0.2f * val / 0.5f;
        else
            return 0.8f + 0.2f * (val - 0.5f) / 0.5f;
    } else if(val > 0.5f && val < 0.7f) {
        val -= 0.5f;
        val /= 0.2f;
        if(val <= 0.5f)
            return 1.0f - 0.2f * val / 0.5f;
        else
            return 0.8f + 0.2f * (val - 0.5f) / 0.5f;
    } else {
        val -= 0.7f;
        val /= 0.3f;
        return 1.0f - val;
    }
}

void GfxParticles::paint(GfxPainter &p)
{
    for(QList<Star>::ConstIterator iter = d->stars.begin(); 
        iter != d->stars.end(); ++iter) {

        const Star &star = *iter;
        const QImage &img = d->imgs.at(star.size);

        p.setOpacity(star.max * valueToOpacity(star.value));
        p.drawImage(star.pos + QPoint(-img.width() / 2, -img.height() / 2), img); 
    }
    p.setOpacity(1.0f);
}

void GfxParticles::addParticleImage(const QImage &img)
{
    d->imgs.append(img);
}

void GfxParticles::advance()
{
    for(QList<Star>::Iterator iter = d->stars.begin(); 
        iter != d->stars.end(); ) {

        Star &star = *iter;
        star.value += star.rate;
        if(star.value < 1.0f) {
            if(star.pos != star.destpos) {
                int x_diff = star.destpos.x() - star.pos.x();
                int y_diff = star.destpos.y() - star.pos.y();

                if(qAbs(x_diff) < star.falling_x) {
                    star.pos.setX(star.destpos.x());
                } else if(x_diff < 0) {
                    star.pos -= QPoint((int)star.falling_x, 0);
                } else if(x_diff > 0) {
                    star.pos += QPoint((int)star.falling_x, 0);
                }
                if(qAbs(y_diff) < star.falling) {
                    star.pos.setY(star.destpos.y());
                } else if(y_diff < 0) {
                    star.pos -= QPoint(0, (int)star.falling);
                } else if(y_diff > 0) {
                    star.pos += QPoint(0, (int)star.falling);
                }
            }

            ++iter;
        } else {
            iter = d->stars.erase(iter);
        }
    }

    if(!d->rect.isEmpty()) {
        while((unsigned)d->stars.count() < d->particles) {
            Star star;
            star.value = 0.0f;
            star.falling = 1.0f + (qreal)(qrand() % 1000) / 100.0f;;
            star.falling_x = 1.0f + (qreal)(qrand() % 1000) / 100.0f;;
            star.rate = 0.01f + (qreal)(qrand() % 1000) / 20000.0f;
            star.max = 0.2f + 0.8f * (qreal)(qrand() % 1000) / 1000.0f;
            star.pos.setX(d->rect.x() + qrand() % d->rect.width());
            star.pos.setY(d->rect.y() + qrand() % d->rect.height());
            star.destpos = star.pos;
            star.size = qrand() % d->imgs.count();
            d->stars.append(star);
        }
    }
}

unsigned int GfxParticles::particles() const
{
    return d->particles;
}

void GfxParticles::setParticles(unsigned int p)
{
    d->particles = p;
}

QRect GfxParticles::rect() const
{
    return d->rect;
}

void GfxParticles::setRect(const QRect &rect)
{
    d->rect = rect;
}

void GfxParticles::moveToRect(const QRect &rect)
{
    if(rect == d->rect)
        return;

    QRect oldrect = d->rect;
    d->rect = rect;

    int translated = rect.topLeft().y() - oldrect.topLeft().y();
    for(QList<Star>::Iterator iter = d->stars.begin(); 
        iter != d->stars.end(); ++iter) {

        iter->destpos += QPoint(0, translated);

    }
}

void GfxParticles::loadSimpleStars(int maxSize)
{
    for(int ii = 1; ii < maxSize; ++ii) {
        int size = ii;
        QImage pix(size * 2, size * 2, QImage::Format_ARGB32_Premultiplied);
        pix.fill(0);
        QPainter p(&pix);
        p.setPen(Qt::NoPen);

        QLinearGradient grad(0, 0, 0, size);
        grad.setColorAt(0.0f, QColor(0, 0, 0, 0));
        grad.setColorAt(1.0f, Qt::white);
        p.translate(size, 0);
        p.setBrush(grad);
        p.drawRect(0, 0, 1, size);

        grad = QLinearGradient(0, 0, size, 0);
        grad.setColorAt(0.0f, QColor(0, 0, 0, 0));
        grad.setColorAt(1.0f, Qt::white);
        p.translate(-size, size);
        p.setBrush(grad);
        p.drawRect(0, 0, size, 1);

        grad = QLinearGradient(0, 0, size, 0);
        grad.setColorAt(1.0f, QColor(0, 0, 0, 0));
        grad.setColorAt(0.0f, Qt::white);
        p.translate(size, 0);
        p.setBrush(grad);
        p.drawRect(0, 0, size, 1);

        grad = QLinearGradient(0, 0, 0, size);
        grad.setColorAt(1.0f, QColor(0, 0, 0, 0));
        grad.setColorAt(0.0f, Qt::white);
        p.translate(0, 0);
        p.setBrush(grad);
        p.drawRect(0, 0, 1, size);

        addParticleImage(pix);
    }
}

