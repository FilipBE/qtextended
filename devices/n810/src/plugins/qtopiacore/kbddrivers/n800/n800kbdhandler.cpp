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

#include "n800kbdhandler.h"

#ifdef QT_QWS_N800
#include <QScreen>
#include <QSocketNotifier>

#include "qscreen_qws.h"
#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qnamespace.h"

#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/vt.h>
#include <sys/kd.h>

#define VTACQSIG SIGUSR1
#define VTRELSIG SIGUSR2

#define DEVICE "/dev/tty0"

struct N800Input {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

static int vtQws = 0;

N800KbdHandler::N800KbdHandler()
{
    numKeyPress = 0;
    qLog(Input) << "Loaded N800 keypad plugin";
    setObjectName( "N800 Keypad Handler" );
    kbdFD = ::open( DEVICE, O_RDONLY|O_NDELAY, 0);
    if (kbdFD >= 0) {
        qLog(Input) << "Opened event2 as keypad input";
        m_notify = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
        connect( m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    } else {
        qWarning("Cannot open event2 for keypad (%s)", strerror(errno));
        return;
    }

    tcgetattr(kbdFD, &origTermData);
    struct termios termdata;
    tcgetattr(kbdFD, &termdata);

    ioctl(kbdFD, KDSKBMODE, K_RAW);

    termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termdata.c_oflag = 0;
    termdata.c_cflag = CREAD | CS8;
    termdata.c_lflag = 0;
    termdata.c_cc[VTIME]=0;
    termdata.c_cc[VMIN]=1;
    cfsetispeed(&termdata, 9600);
    cfsetospeed(&termdata, 9600);
    tcsetattr(kbdFD, TCSANOW, &termdata);

    connect(QApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(handleTtySwitch(int)));
    QApplication::instance()->watchUnixSignal(VTACQSIG, true);
    QApplication::instance()->watchUnixSignal(VTRELSIG, true);

    struct vt_mode vtMode;
    ioctl(kbdFD, VT_GETMODE, &vtMode);

    // let us control VT switching
    vtMode.mode = VT_PROCESS;
    vtMode.relsig = VTRELSIG;
    vtMode.acqsig = VTACQSIG;
    ioctl(kbdFD, VT_SETMODE, &vtMode);

    struct vt_stat vtStat;
    ioctl(kbdFD, VT_GETSTATE, &vtStat);
    vtQws = vtStat.v_active;
}

N800KbdHandler::~N800KbdHandler()
{
    if (kbdFD >= 0) {
        ioctl(kbdFD, KDSKBMODE, K_XLATE);
        tcsetattr(kbdFD, TCSANOW, &origTermData);
        ::close(kbdFD);
        kbdFD = -1;
    }
}

void N800KbdHandler::handleTtySwitch(int sig)
{
    if (sig == VTACQSIG) {
        if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
            qwsServer->enablePainting(true);
            qt_screen->restore();
            qwsServer->resumeMouse();
            qwsServer->refresh();
        }
    } else if (sig == VTRELSIG) {
        qwsServer->enablePainting(false);
        qt_screen->save();
        if(ioctl(kbdFD, VT_RELDISP, 1) == 0) {
            qwsServer->suspendMouse();
        } else {
            qwsServer->enablePainting(true);
        }
    }
}

void N800KbdHandler::readKbdData()
{
    unsigned char  buf[81];
    unsigned short key_code;
    unsigned short Phkey;
    unsigned short unicode;
    unsigned int   key_code2;
    int            modifiers;

    int n = ::read(kbdFD, buf, 80);
    for ( int loop = 0; loop < n; loop++ ) {
//        qLog(Input) << "read " << n <<" bytes";
        key_code = (unsigned short)buf[loop];
        key_code2 = 0;
        unicode = 0xffff;
        modifiers = 0;
        if(((key_code & 0x80)==0))
            numKeyPress++;

        qLog(Input) << "keypressed: code=" << key_code << " (" << (((key_code & 0x80)==0) ? "Down":"Up") << ")";
        Phkey = key_code;// & 0x7F;

        switch(Phkey)
        {
        case 0x74: //n770
        case 0x5e: //n800
            key_code2 = Qt::Key_Hangup;
            unicode  = 0xffff;
            break;
        case 0x3f:
            key_code2 = Qt::Key_Home;
            unicode  = 0xffff;
            break;
        case 0x3e:
            key_code2 = Qt::Key_Context1;
            unicode  = 0xffff;
            break;
        case 0x1:
            key_code2 = Qt::Key_Back;
            unicode  = 0xffff;
            break;

        case 0x1C:
            key_code2 = Qt::Key_Select;
            unicode  = 0xffff;
            break;
        case 0x67: //n770
        case 0x48: //n800
            key_code2 = Qt::Key_Up;
            unicode  = 0xffff;
            break;
        case 0x6C: //n770
        case 0x50: //n800
            key_code2 = Qt::Key_Down;
            unicode  = 0xffff;
            break;
        case 0x69: //n770
        case 0x4B: //n800
            key_code2 = Qt::Key_Left;
            unicode  = 0xffff;
            break;
        case 0x6A: //n770
        case 0x4D: //n800
            key_code2 = Qt::Key_Right;
            unicode  = 0xffff;
            break;
            // Volume Down
        case 0x42:
            key_code2 = Qt::Key_VolumeUp;
            unicode = 0xffff;
            break;
//             // Volume Up
        case 0x41:
            key_code2 = Qt::Key_VolumeDown;
            unicode = 0xffff;
            break;

        }

        numKeyPress++;
        qLog(Input) << "processKeyEvent(): key=" << key_code2 << ", unicode=" << unicode << "numKeyPress "<<numKeyPress;
        if(numKeyPress < 3)
            processKeyEvent(unicode, key_code2, (Qt::KeyboardModifiers)modifiers, !(key_code & 0x80), false);


        if((key_code & 0x80) == 0) {
            // Down
            if(numKeyPress > 2)
                beginAutoRepeat(unicode, key_code2, (Qt::KeyboardModifiers)modifiers);
        } else {
            // Released
            numKeyPress = 0;
            endAutoRepeat();
        }
    }

}

#endif // QT_QWS_N800
