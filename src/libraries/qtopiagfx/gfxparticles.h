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

#ifndef GFXPARTICLES_H
#define GFXPARTICLES_H

class QRect;
class QImage;
class GfxPainter;
class GfxParticlesPrivate;
class GfxParticles
{
public:
    GfxParticles();
    virtual ~GfxParticles();

    void advance();

    unsigned int particles() const;
    void setParticles(unsigned int);

    void addParticleImage(const QImage &);

    QRect rect() const;
    void setRect(const QRect &rect);
    void moveToRect(const QRect &rect);

    void paint(GfxPainter &);

    // Common particle images
    void loadSimpleStars(int maxSize);

private:
    GfxParticlesPrivate *d;
};

#endif
