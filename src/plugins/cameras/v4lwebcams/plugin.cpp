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

#include "plugin.h"
#include "webcams.h"
#include <QCameraDevice>

Plugin::Plugin(QObject* parent)
:QCameraDevicePlugin(parent)
{}

Plugin::~Plugin()
{ }

QStringList Plugin::keys() const
{
    QStringList cameras;
#ifdef QTOPIA_HAVE_V4L2
    cameras <<  QString("v4l2webcam");
#endif
    cameras << QString("v4l1webcam");
    return cameras;
}

QCameraDevice* Plugin::create(QString const& key)
{

#ifdef QTOPIA_HAVE_V4L2
    static int run_once = 1;
    static int hasV4L2Camera = false;
    static int hasV4L1Camera = false;

    if (run_once) {
        int fd;
        if ((fd=::open(V4L_VIDEO_DEVICE, O_RDWR)) != -1) {
            v4l2_capability caps;
            if (ioctl(fd,VIDIOC_QUERYCAP, &caps) == -1) {
                if(errno == EINVAL) {
                    hasV4L2Camera = false;  // likely a V4L1 webcam
                    hasV4L1Camera = true;
                } else
                    hasV4L1Camera = true;
            }
             else
                hasV4L2Camera = true;
            ::close(fd);
        } else
            hasV4L2Camera = false;
        run_once = 0;
    }
    if( key == "v4l2webcam" && hasV4L2Camera)
        return new V4L2Webcam;
    if (key == "v4l1webcam" && hasV4L1Camera)
        return new V4L1Webcam;
#else
    if( key == "v4l1webcam" )
        return new V4L1Webcam;
#endif
    return 0;
}

QTOPIA_EXPORT_PLUGIN(Plugin);
