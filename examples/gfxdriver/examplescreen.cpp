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

#include "examplescreen.h"
#include <QRect>
#include <QRegion>

#include <QDebug>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


ExampleScreen::ExampleScreen(int displayId)
: QLinuxFbScreen(displayId)
{
    fbd = ::open( "/dev/fb0", O_RDWR );
    if ( fbd < 0 ) {
        qWarning() << "ExampleScreen: Cannot open frame buffer device /dev/fb0";
        return;
    }
}

ExampleScreen::~ExampleScreen()
{
}

void ExampleScreen::exposeRegion(QRegion region, int changing)
{
    QLinuxFbScreen::exposeRegion( region, changing );
    dirtyrect = region.boundingRect();
    //add your own flush here to update the dirty rect!!
}

