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

#include "nokiascreen.h"

#include <QRect>
#include <QRegion>
#include <QtGui/qscreen_qws.h>
#include <QWSServer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QTime>

#include <QFileSystemWatcher>

//#include <asm/arch-omap/omapfb.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>

#include <qtopialog.h>

#define DEVICE "/dev/fb0"

#define OMAP_IOW(num, dtype)   _IOW('O', num, dtype)
#define OMAP_IO(num)            _IO('O', num)

#define OMAPFB_UPDATE_WINDOW   OMAP_IOW(54, struct omapfb_update_window)
#define OMAPFB_SET_UPDATE_MODE OMAP_IOW(40, enum  omapfb_update_mode)
#define OMAPFB_SYNC_GFX        OMAP_IO(37)

#define OMAPFB_FORMAT_FLAG_TEARSYNC     0x0200

//manual screen update mode works faster, but it is not stable enough yet
//#define OMAP_MANUAL_SCREEN_UPDATE

//#define TEST_NOKIA_UPDATE_PERF

struct omapfb_update_window {
        __u32 x, y;
        __u32 width, height;
        __u32 format;
        __u32 out_x, out_y;
        __u32 out_width, out_height;
        __u32 reserved[8];
};

enum omapfb_update_mode {
    OMAPFB_UPDATE_DISABLED = 0,
    OMAPFB_AUTO_UPDATE,
    OMAPFB_MANUAL_UPDATE
};

NokiaScreen::NokiaScreen(int displayId)
: QLinuxFbScreen(displayId)
{
    qLog(Hardware) <<"NokiaScreen";

    infd = ::open( DEVICE, O_RDWR | O_NONBLOCK | O_NDELAY);
    if (infd < 0) {
        qLog(Hardware) << "can not open" << DEVICE;
    }
}

NokiaScreen::~NokiaScreen()
{
    ::close(infd);
}

bool NokiaScreen::initDevice()
{
#ifdef OMAP_MANUAL_SCREEN_UPDATE
    int updatemode = OMAPFB_MANUAL_UPDATE;
#else
    int updatemode = OMAPFB_AUTO_UPDATE;
#endif

    if (infd > 0) {
        if (ioctl( infd, OMAPFB_SET_UPDATE_MODE, &updatemode) < 0) {
            qLog(Hardware) << "failure to set manual update mode; ioctl( OMAPFB_SET_UPDATE_MODE)";
        }
    }

    return QLinuxFbScreen::initDevice();
}

void NokiaScreen::shutdownDevice()
{
    int updatemode = OMAPFB_AUTO_UPDATE;

    if (infd > 0) {
        if (ioctl( infd, OMAPFB_SET_UPDATE_MODE, &updatemode) < 0) {
            qLog(Hardware) << "failure to set auto update mode; ioctl( OMAPFB_SET_UPDATE_MODE)";
        }
    }

    QLinuxFbScreen::shutdownDevice();
}


void NokiaScreen::syncGfx()
{
#ifdef OMAP_MANUAL_SCREEN_UPDATE
    if (infd > 0)
        ioctl( infd, OMAPFB_SYNC_GFX );
#endif
}

void NokiaScreen::exposeRegion(QRegion r, int changing)
{
    QScreen::exposeRegion(r, changing);
}

void NokiaScreen::setDirty( const QRect& rect )
{
#ifdef OMAP_MANUAL_SCREEN_UPDATE
    struct omapfb_update_window update;

    dirtyRegion += QRegion(rect);

    QRect r = (dirtyRegion & region()).boundingRect();
    r = r.translated(-offset());

    update.x = r.x();
    update.y = r.y();
    update.width = r.width();
    update.height = r.height();
    update.out_x = update.x;
    update.out_y = update.y;
    update.out_width = update.width;
    update.out_height = update.height;
    update.format = 0;// | OMAPFB_FORMAT_FLAG_TEARSYNC ;// OMAPFB_COLOR_RGB565;

    if ( infd > 0 && okToUpdate() ) {
        syncGfx();
        //qLog(Hardware) <<"update" << r;
        if (ioctl( infd, OMAPFB_UPDATE_WINDOW, &update) < 0) {
            qLog(Hardware) <<"ioctl(OMAPFB_UPDATE_WINDOW)";
            perror("ioctl(OMAPFB_UPDATE_WINDOW)");
        }

        dirtyRegion = QRegion();

#ifdef TEST_NOKIA_UPDATE_PERF
        QTime t;
        t.start();
        ioctl( infd, OMAPFB_SYNC_GFX );
        qLog(Hardware) << "updated in" << t.elapsed() << "ms";
#endif
    }
#else
    Q_UNUSED(rect);
#endif
}

bool NokiaScreen::okToUpdate()
{
#ifdef OMAP_MANUAL_SCREEN_UPDATE
    QString path = "/sys/devices/platform/omapfb/panel/backlight_level";
    QString strvalue;

    QFile brightness;
    if (QFileInfo(path).exists() ) {
        brightness.setFileName(path);
    }

    if( !brightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"brightness File not opened";
    } else {
        QTextStream in(&brightness);
        in >> strvalue;
        brightness.close();
    }

    if ( strvalue.toInt() > 1 )
        return true;
    else
        return false;
#else
    return true;
#endif
}

