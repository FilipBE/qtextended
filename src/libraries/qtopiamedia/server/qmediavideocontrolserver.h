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

#ifndef QMEDIAVIDEOCONTROLSERVER_H
#define QMEDIAVIDEOCONTROLSERVER_H

#include <QWSEmbedWidget>

#include <qtopiaglobal.h>
#include <qtopiavideo.h>




class QMediaHandle;

class QMediaVideoControlServerPrivate;


class QTOPIAMEDIA_EXPORT QMediaVideoControlServer :
    public QObject
{
    Q_OBJECT

public:
    QMediaVideoControlServer(QMediaHandle const& handle,
                             QWidget *target = 0,
                             QObject *parent = 0);
    ~QMediaVideoControlServer();

    void setRenderTarget(QWidget* target);
    void setRenderTarget(WId wid);
    void unsetRenderTarget();

    QtopiaVideo::VideoRotation videoRotation() const;
    QtopiaVideo::VideoScaleMode videoScaleMode() const;

signals:
    void rotationChanged(QtopiaVideo::VideoRotation);
    void scaleModeChanged(QtopiaVideo::VideoScaleMode);

private slots:
    void updateVideoRotation();
    void updateVideoScaleMode();

private:
    QMediaVideoControlServerPrivate *d;
};

#endif

