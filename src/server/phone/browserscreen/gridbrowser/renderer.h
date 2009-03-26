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

#ifndef RENDERER_H
#define RENDERER_H

#include <QString>

class QPainter;
class QRectF;

class Renderer
{
public:
    virtual ~Renderer() {}
    virtual bool load(const QString &filename) = 0;
    virtual void render(QPainter *painter, const QRectF &bounds) = 0;

    static Renderer *rendererFor(const QString &filename);
};

#endif

