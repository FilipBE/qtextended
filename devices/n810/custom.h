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

#ifndef QT_QWS_N810
#define QT_QWS_N810
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS

// Disable media player visualization
#ifndef NO_VISUALIZATION
#define NO_VISUALIZATION
#endif
#define QPE_NEED_CALIBRATION

#define V4L_RADIO_DEVICE "/dev/radio0"
#define V4L_VIDEO_DEVICE "/dev/video0"

//gstreamer volume range is 0 to 10 instead of 0 to 1, override
#define GST_MAX_VOLUME 10

// Adjust priority on n810 for mediaserver so you can control the player
#ifndef MEDIASERVER_PRIORITY
#define MEDIASERVER_PRIORITY -19
#endif

// Cruxus mediaengine overides to playback with reduced alsa ring buffer
#ifndef CRUXUS_FRAMESIZE
#define CRUXUS_FRAMESIZE  30
#endif

#ifndef CRUXUS_OUTPUT_FREQ
#define CRUXUS_OUTPUT_FREQ 44100
#endif

#ifndef ALSA_PERIOD_SIZE
#define ALSA_PERIOD_SIZE 1024
#endif

#ifndef FULLSCREEN_VIDEO_ROTATION
#define FULLSCREEN_VIDEO_ROTATION QtopiaVideo::Rotate270
#endif
