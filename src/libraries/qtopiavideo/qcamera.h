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


#ifndef QTOPIA_CAMERA_H
#define QTOPIA_CAMERA_H

#include <qtopiaglobal.h>

#include <qtopiaipcmarshal.h>
#include <QSize>

namespace QtopiaCamera
{
    enum CameraError
    {
        FatalError = 0,
        Warning
    };

    enum Resolution
    {
        NonStandard = 0,
        SQCIF ,     //128x96
        QCIF ,           //176x144
        QVGA ,           //240x320
        CIF ,            //352x288   - 0.10 MP
        VGA,            //640x480   - 0.31 MP
        XGA,            //1024x768  - 0.79 MP
        SXGA,           //1280x1024 - 1.31 MP
        UXGA,           //1600x1200 - 1.92 MP
        WUXGA,          //1920x1200 - 2.30 MP
        QXGA,           //2048x1536 - 3.14 MP
        WQXGA,          //2560x1600 - 4.09 MP
        FIVEMP          //2580x1936 - 4.99 MP
    };

    enum DataFormat
    {
        RGB32 = 876758866,
        RGB24 = 859981650,
        RGB565 = 1346520914,
        RGB555 = 809650002,
        YUYV = 1448695129,
        UYVY = 1498831189,
        SBGGR8 = 825770306,
        MJPEG = 1196444237,
        JPEG = 1195724874,
        MPEG = 1195724877,
        H263 = 859189832
    };

    typedef QMap<unsigned int, QList<QSize> > FormatResolutionMap;
}


Q_DECLARE_USER_METATYPE_ENUM(QtopiaCamera::CameraError);

#endif /*QTOPIACAMERA_H*/

