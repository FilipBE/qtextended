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

#include "qtopiavideo.h"

/*!
    \namespace QtopiaVideo
    \inpublicgroup QtEssentialsModule
    \inpublicgroup QtMediaModule
    \brief The QtopiaVideo namespace provides common video definitions.
*/

/*!
    \enum QtopiaVideo::VideoRotation

    This enum specifies the video frames orientation.
    \value NoRotation The image is not rotated.
    \value Rotate0 The same as NoRotation.
    \value Rotate90 The image is rotated 90 degree clockwise.
    \value Rotate180 The image is rotated 180 degree clockwise.
    \value Rotate270 The image is rotated 270 degree clockwise.
*/

/*!
    \enum QtopiaVideo::VideoScaleMode

    This enum specifies the video frames scaling.

    \value NoScale The image is not scaled but centered in the video window.
    \value FitWindow The image is scaled to fit video window.
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaVideo::VideoRotation);
Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaVideo::VideoScaleMode);


