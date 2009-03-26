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
#ifndef QTOPIACUSTOM_H
#define QTOPIACUSTOM_H

#include <custom-qtopia.h>

// The documentation references these lines (see doc/src/syscust/custom.qdoc)
#ifndef V4L_VIDEO_DEVICE
#define V4L_VIDEO_DEVICE "/dev/video"
#endif

#include <qtopiaglobal.h>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps();
QTOPIABASE_EXPORT void qpe_setBrightness(int bright);

#endif
