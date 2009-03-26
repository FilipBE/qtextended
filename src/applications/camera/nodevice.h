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

#ifndef NODEVICE_H
#define NODEVICE_H

namespace camera
{

class NoDevice : public VideoCaptureDevice
{
public:

    NoDevice() {}
    ~NoDevice() {}

    bool hasCamera() const { return false; }
    void getCameraImage(QImage&, bool = false ) {}

    QList<QSize> photoSizes() const { return QList<QSize>(); }
    QList<QSize> videoSizes() const { return QList<QSize>(); }

    QSize recommendedPhotoSize() const { return QSize(); }
    QSize recommendedVideoSize() const { return QSize(); }
    QSize recommendedPreviewSize() const { return QSize(); }

    QSize captureSize() const { return QSize(); }
    void setCaptureSize(QSize) {}

    uint refocusDelay() const { return 0; }
    int minimumFramePeriod() const { return 0; }

    int getFD() { return -1; }
};

}   // ns camera

#endif
