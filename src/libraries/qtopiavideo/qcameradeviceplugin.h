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

#ifndef QCAMERADEVICEPLUGIN_H
#define QCAMERADEVICEPLUGIN_H

#include <qtopiaglobal.h>
#include <QStringList>

#include "qcamera.h"

class QCameraDevice;

class QTOPIAVIDEO_EXPORT QCameraPluginInterface
{
public:
    virtual ~QCameraPluginInterface();
    virtual QStringList keys() const = 0;
    virtual QCameraDevice* create(QString const& name) = 0;
};

Q_DECLARE_INTERFACE(QCameraPluginInterface, "com.trolltech.qtopia.CameraPluginInterface/1.0");


class QTOPIAVIDEO_EXPORT QCameraDevicePlugin : public QObject, public QCameraPluginInterface
{
    Q_OBJECT
public:

    explicit QCameraDevicePlugin(QObject* parent);
    virtual ~QCameraDevicePlugin();
};

#endif

