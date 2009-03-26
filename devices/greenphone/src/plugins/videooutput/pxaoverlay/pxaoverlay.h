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

#ifndef PXAOVERLAY_H
#define PXAOVERLAY_H

#include <QObject>
#include <QRect>
#include <QVideoFrame>
#include <qtopiavideo.h>

class PxaOverlayPrivate;

class PxaOverlay :public QObject
{
    Q_OBJECT

public:
    PxaOverlay(QObject *parent = 0);
    virtual ~PxaOverlay();

    bool open(const QRect& geometry);
    void close();

    bool isValid() const;

    QString errorMessage() const;
    QRect geometry() const;

public slots:
    void drawFrame(const QVideoFrame&); //draw rotated and prescaled image in YV12 or YUV420P formats
    void drawFrame(const QVideoFrame&, const QRect& frameRect, const QRect& overlayRect, QtopiaVideo::VideoRotation);

    void fill(int y = 16, int u = 128, int v = 128);

private:
    PxaOverlayPrivate *d;
};

#endif
