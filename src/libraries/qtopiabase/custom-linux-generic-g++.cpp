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

#define QTOPIA_QVFB_BRIGHTNESS

#include <custom.h>
#include <qscreen_qws.h>
#include <qscreenvfb_qws.h>
#include <QDebug>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    return 10;
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
#ifdef Q_WS_QWS
#ifndef QT_NO_QWS_QVFB
    if ( b != 0 )
        b = b*20+55; // 55 = dimmest visible level
    QVFbScreen::setBrightness(b);
#else
    QScreen::instance()->blank(b==0);
#endif
#endif
}

