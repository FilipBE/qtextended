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

#ifndef GFXIMAGELOADER_H
#define GFXIMAGELOADER_H

#include <QList>
#include <QSize>
#include <QImage>
class QString;
class GfxImageLoader
{
public:
    GfxImageLoader();
    virtual ~GfxImageLoader();
    void loadImage(const QString &filename, const QList<QSize> &sizes);

    virtual void images(const QList<QImage> &imgs) = 0;
    virtual void filter(QList<QImage> &imgs);

    static unsigned int cacheSize();
    static void setCacheSize(unsigned int);
    static void start();
private:
    bool _active;
    friend class GfxImageLoaderEngine;
};

#endif
