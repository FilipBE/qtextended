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

#include <QPluginManager>
#include <QMap>

#include "qcameradevice.h"
#include "qcameradeviceplugin.h"
#include "qcameradeviceloader.h"


#define CUSTOM_PLUGIN_DIR QLatin1String("cameras")


class CameraDeviceLoaderPrivate
{
public:
    CameraDeviceLoaderPrivate();
    ~CameraDeviceLoaderPrivate();

    QPluginManager* manager;
    QMap<QString, QCameraDevice*> deviceMap;
    QObject *instance;
    bool pluginsLoaded;
    bool hasPlugins;

    void load();
    void unload();
};


CameraDeviceLoaderPrivate::CameraDeviceLoaderPrivate():
    pluginsLoaded(false),
    hasPlugins(false)
{
    manager = new QPluginManager(CUSTOM_PLUGIN_DIR);

}

CameraDeviceLoaderPrivate::~CameraDeviceLoaderPrivate()
{
    delete manager;
}

/*!
    Loads available plugins
    \internal
*/

void CameraDeviceLoaderPrivate::load()
{
    if(pluginsLoaded)
        return;

    if(manager->list().isEmpty()){
        hasPlugins = false;
        pluginsLoaded = true;
        return;
    }

    QCameraDevice *device = 0;
    foreach(QString const& pluginName, manager->list()) {
        instance = manager->instance(pluginName);
        QCameraDevicePlugin *plugin;
        if( (plugin = qobject_cast<QCameraDevicePlugin*>(instance)) != 0) {
            foreach(QString devId, plugin->keys()) {
                device = plugin->create(devId);
                if(device) {
                    deviceMap.insert(devId, device);
                    hasPlugins = true;
                }
            }
        }
    }

    pluginsLoaded = true;
}


/*!
    free devices
    \internal
*/
void CameraDeviceLoaderPrivate::unload()
{
    if(hasPlugins) {
        foreach(QCameraDevice* device, deviceMap.values())
            delete device;
        delete instance;
        hasPlugins = pluginsLoaded = false;
    }

}


////////////////////////////QCameraDeviceLoader///////////////////////////

/*!
    \class  QCameraDeviceLoader
    \inpublicgroup QtMediaModule
    \brief The QCameraDeviceLoader class provide acccess to camera devices through a plugin mechanism

*/

/*!
    destructor
    unloads the plugins
*/
QCameraDeviceLoader::~QCameraDeviceLoader()
{
    d->unload();
    delete d;
}

/*!
    \fn bool QCameraDeviceLoader::cameraDevicesAvailable() const
    Returns true there are cameras devices available, false otherwise
*/
bool QCameraDeviceLoader::cameraDevicesAvailable() const
{
    return d->hasPlugins;
}

/*!
    \fn QList<QCameraDevice*> QCameraDeviceLoader::allAvailableCameras() const
    Returns a list of all avaiable camera devices
*/
QList<QCameraDevice*> QCameraDeviceLoader::allAvailableCameras() const
{
    if (!cameraDevicesAvailable())
        return QList<QCameraDevice*>();

    return d->deviceMap.values();
}

/*!

    \fn QCameraDevice* QCameraDeviceLoader::deviceWithOrientation(QCameraDevice::Orientation orientation) const
    Returns the first device found with Orientation \a orientation
*/

QCameraDevice* QCameraDeviceLoader::deviceWithOrientation(QCameraDevice::Orientation orientation) const
{
    if (!cameraDevicesAvailable())
        return 0;
    foreach (QString devId, d->deviceMap.keys()) {
        if (d->deviceMap[devId]->orientation() == orientation)
            return  d->deviceMap[devId];
    }
    return 0;
}

/*!
    \fn QCameraDeviceLoader* QCameraDeviceLoader::instance()
    Returns an instance of the camera device loader
*/

QCameraDeviceLoader* QCameraDeviceLoader::instance()
{
    static QCameraDeviceLoader self;

    return &self;
}


QCameraDeviceLoader::QCameraDeviceLoader():
    d(new CameraDeviceLoaderPrivate)
{
    d->load();
}



