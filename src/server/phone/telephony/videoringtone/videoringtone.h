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

#ifndef VIDEORINGTONE_H
#define VIDEORINGTONE_H

#include <QObject>
#include "qtopiaserverapplication.h"

class QWidget;
class VideoRingtonePrivate;

class VideoRingtone : public QObject
{
    Q_OBJECT

public:
    VideoRingtone( QObject *parent = 0);
    ~VideoRingtone();

    QWidget* videoWidget();

public slots:
    void playVideo(const QString& fileName);
    void stopVideo();

signals:
    void videoRingtoneFailed();
    void videoWidgetReady();
    void videoRingtoneStopped();

private:
    VideoRingtonePrivate *d;
};

QTOPIA_TASK_INTERFACE(VideoRingtone);

#endif
