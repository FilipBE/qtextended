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
#include "qcameratools.h"

#include <QHash>



QSize qResolutionToQSize(int res)
{
    using namespace QtopiaCamera;
    switch(res) {
        case NonStandard:  return QSize(-1, -1);
        case SQCIF:        return QSize(128, 96);
        case QCIF:         return QSize(176, 144);
        case QVGA:         return QSize(320, 240);
        case CIF:          return QSize(352, 288);
        case VGA:          return QSize(640, 480);
        case XGA:          return QSize(1024, 768);
        case SXGA:         return QSize(1280, 1024);
        case UXGA:         return QSize(1600, 1200);
        case WUXGA:        return QSize(1920, 1200);
        case QXGA:         return QSize(2048, 1536);
        case WQXGA:        return QSize(2560, 1600);
        case FIVEMP:       return QSize(2580, 1936);
    }
    return QSize(-1,-1);
}

QVideoFrame::PixelFormat qFourccToVideoFormat(unsigned int fourcc)
{
#define fourcc(a,b,c,d)\
    (((unsigned int)(a)<<0)|((unsigned int)(b)<<8)|((unsigned int)(c)<<16)|((unsigned int)(d)<<24))

    // Note: This map must stay in sync with QVideoFrame::PixelFormat
    static unsigned int formatMap[] =
    {
        0,                       // Format_Invalid
        0,                       // Format_ARGB32
        fourcc('R','G','B','4'), // Format_RGB32
        fourcc('R','G','B','3'), // Format_RGB24
        fourcc('R','G','B','P'), // Format_RGB565

        0,                       // Format_BGRA32
        fourcc('B','G','R','4'), // Format_BGR32
        fourcc('B','G','R','3'), // Format_BGR24
        0,                       // Format_GBR565

        0,                       // Format YUV444
        0,                       // Format_YUV420P
        0,                       // Format_YV12
        fourcc('U','Y','V','Y'), // Format_UYVY
        fourcc('Y','U','Y','V'), // Format_YUYV
        0                        // Format_Y8
    };
#undef fourcc

    // Note: This map must stay in sync with QVideoFrame::PixelFormat
    static QVideoFrame::PixelFormat pixelFormats[] =
    {
        QVideoFrame::Format_Invalid,
        QVideoFrame::Format_ARGB32,
        QVideoFrame::Format_RGB32,
        QVideoFrame::Format_RGB24,
        QVideoFrame::Format_RGB565,

        QVideoFrame::Format_BGRA32,
        QVideoFrame::Format_BGR32,
        QVideoFrame::Format_BGR24,
        QVideoFrame::Format_BGR565,

        QVideoFrame::Format_YUV444,
        QVideoFrame::Format_YUV420P,
        QVideoFrame::Format_YV12,
        QVideoFrame::Format_UYVY,
        QVideoFrame::Format_YUYV,
        QVideoFrame::Format_Y8
    };


    static QHash<unsigned int, QVideoFrame::PixelFormat> _map;
    static int once = 1;

    if (once) {
        for(unsigned int i = 0; i <  sizeof(formatMap)/sizeof(unsigned int); ++i) {
            if (formatMap[i] != 0)
                _map.insert(formatMap[i], pixelFormats[i]);
        }
        once = 0;
    }

    return _map.value(fourcc, QVideoFrame::Format_Invalid);
}

