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

#include "qcameradeviceplugin.h"

/*!
    \class QCameraPluginInterface
    \inpublicgroup QtMediaModule
    \brief The QCameraPluginInterface class is an interface class for the camera device plugin
*/

/*!
    \fn QStringList QCameraPluginInterface::keys() const
    Returns a string list of available devices names
*/

/*!
    \fn QCameraDevice* QCameraPluginInterface::create(QString const& name)
    Returns a newly created camera device with \a name as a description
    \sa QCameraDevice
*/

/*!
    dtor
    \internal
*/
QCameraPluginInterface::~QCameraPluginInterface()
{}

/*!
    \class QCameraDevicePlugin
    \inpublicgroup QtMediaModule
    \brief The QCameraDevicePlugin class provides a plugin mechanism for the camera device
*/

/*!
    ctor
    \internal
*/
QCameraDevicePlugin::QCameraDevicePlugin(QObject* parent)
:QObject(parent)
{}

/*!
    dtor
    \internal
*/
QCameraDevicePlugin::~QCameraDevicePlugin()
{}



