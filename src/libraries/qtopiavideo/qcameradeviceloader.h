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

#ifndef QCAMERADEVICELOADER_H
#define QCAMERADEVICELOADER_H

#include <QObject>
#include <QList>

#include "qcamera.h"
#include "qcameradevice.h"

class CameraDeviceLoaderPrivate;


class QTOPIAVIDEO_EXPORT QCameraDeviceLoader : public QObject
{
    Q_OBJECT

public:
    ~QCameraDeviceLoader();

    bool cameraDevicesAvailable() const;

    QList<QCameraDevice*> allAvailableCameras() const;

    QCameraDevice* deviceWithOrientation(QCameraDevice::Orientation orientation) const;

    static QCameraDeviceLoader* instance();

private:
    QCameraDeviceLoader();

    CameraDeviceLoaderPrivate* d;
};

#endif
