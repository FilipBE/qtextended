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
#include "qcamera.h"

/*!
    \namespace QtopiaCamera
    \inpublicgroup QtEssentialsModule
    \inpublicgroup QtMediaModule
    \brief The QtopiaCamera namespace provides common camera definitions
*/

/*!
    \enum QtopiaCamera::CameraError
    Error descriptor code
    \value    FatalError  A Fatal Error has occured
    \value    Warning     A  Non-Fatal error has occured
*/

/*!
    \enum QtopiaCamera::Resolution
    Image Resolutions
    \value NonStandard  Non Standard Resolution
    \value SQCIF 128x96
    \value QCIF 176x144
    \value QVGA 240x320
    \value CIF  352x288 or 0.10 Mega Pixels
    \value VGA  640x480 or 0.31 Mega Pixels
    \value XGA  1024x768 or 0.79 Mega Pixels
    \value SXGA 1280x1024 or 1.31 Mega Pixels
    \value UXGA 1600x1200 or 1.92 Mega Pixels
    \value WUXGA 1920x1200 or 2.30 Mega Pixels
    \value QXGA 2048x1536 or 3.14 Mega Pixels
    \value WQXGA 2560x1600 or 4.09 Mega Pixels
    \value FIVEMP 2580x1936 or 4.99 Mega Pixels
*/

/*!
    \enum QtopiaCamera::DataFormat
    Pixel and Image Data formats given in FOURCC code (see: http://v4l2spec.bytesex.org )
    \value  RGB32  24-bit rgb format: 0xffRRGGBB
    \value  RGB24  packed 24-bit rgb format:  0xRRGGBB
    \value  RGB565 16-bit rgb
    \value  RGB555 15-bit rgb
    \value  YUYV   YUV 4:2:2
    \value  UYVY   YUV 4:2:2
    \value  SBGGR8 10-bit Bayer format
    \value  MJPEG  Motion JPEG
    \value  JPEG   JPEG
    \value  MPEG   MPEG bitstream
    \value  H263   H.263 bitstream
*/

/*!
   \typedef QtopiaCamera::FormatResolutionMap
   \relates QMap

   Synonym for QMap<unsigned int, QList<QSize>>
   Holds a map of formats to a list of supported resolutions for that format
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaCamera::CameraError);

