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

#include "greenphonekbdhandler.h"

#ifdef QT_QWS_GREENPHONE
#include <QScreen>
#include <QSocketNotifier>
#ifdef GREENPHONE_KEYPAD_REPEAT
#include <QTimer>
#endif

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

static int vtQws = 0;

GreenphoneKbdHandler::GreenphoneKbdHandler()
{
    qLog(Input) << "Loaded Greenphone keypad plugin";

    setObjectName( "Greenphone Keypad Handler" );

    kbdFD = ::open("/dev/tty0", O_RDONLY | O_NDELAY, 0);
    if (kbdFD >= 0)
    {
        qLog(Input) << "Opened tty0 as keypad input";
        m_notify = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    }
    else
    {
        qWarning("Cannot open tty0 for keypad (%s)", strerror(errno));
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

#ifdef GREENPHONE_KEYPAD_REPEAT
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(repeat()));
#endif
}

GreenphoneKbdHandler::~GreenphoneKbdHandler()
{
    if (kbdFD >= 0)
    {
        ioctl(kbdFD, KDSKBMODE, K_XLATE);
        tcsetattr(kbdFD, TCSANOW, &origTermData);
        ::close(kbdFD);
    }
}

void GreenphoneKbdHandler::handleTtySwitch(int sig)
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

void GreenphoneKbdHandler::readKbdData()
{
    unsigned char           buf[80];
    unsigned short          driverKeyCode;
    unsigned short          unicode;
    unsigned int            qtKeyCode;
    bool                    isPressed;
    Qt::KeyboardModifiers   modifiers = Qt::NoModifier;

    int n = ::read(kbdFD, buf, 80);

    for (int loop = 0; loop < n; loop++)
    {
        driverKeyCode   = (unsigned short)buf[loop];
        qtKeyCode       = 0;
        unicode         = 0xffff;
        isPressed       = (driverKeyCode & 0x80) == 0;

        qLog(Input) << "keypressed: code=" << driverKeyCode << " (" << (isPressed ? "Down" : "Up") << ")";

        switch (driverKeyCode & 0x7f)
        {
            // Keypad
            case 0x2e: qtKeyCode = Qt::Key_0; unicode  = 0x30; break;
            case 0x02: qtKeyCode = Qt::Key_1; unicode  = 0x31; break;
            case 0x03: qtKeyCode = Qt::Key_2; unicode  = 0x32; break;
            case 0x04: qtKeyCode = Qt::Key_3; unicode  = 0x33; break;
            case 0x05: qtKeyCode = Qt::Key_4; unicode  = 0x34; break;
            case 0x06: qtKeyCode = Qt::Key_5; unicode  = 0x35; break;
            case 0x08: qtKeyCode = Qt::Key_6; unicode  = 0x36; break;
            case 0x09: qtKeyCode = Qt::Key_7; unicode  = 0x37; break;
            case 0x0a: qtKeyCode = Qt::Key_8; unicode  = 0x38; break;
            case 0x0b: qtKeyCode = Qt::Key_9; unicode  = 0x39; break;

            case 0x1e: qtKeyCode = Qt::Key_Asterisk;
                       unicode  = 0x2A;
                       break;
            case 0x20: qtKeyCode = Qt::Key_NumberSign;
                       unicode  = 0x23;
                       break;

            // Navigation+
            case 0x32: qtKeyCode = Qt::Key_Call; break;
            case 0x16: qtKeyCode = Qt::Key_Hangup; break;
            case 0x19: qtKeyCode = Qt::Key_Context1; break;
            case 0x26: qtKeyCode = Qt::Key_Back; break;
            case 0x12: qtKeyCode = Qt::Key_Up; break;
            case 0x24: qtKeyCode = Qt::Key_Down; break;
            case 0x21: qtKeyCode = Qt::Key_Left; break;
            case 0x17: qtKeyCode = Qt::Key_Right; break;
            case 0x22: qtKeyCode = Qt::Key_Select; break;

            // Keys on left hand side of device
            case 0x07: qtKeyCode = Qt::Key_VolumeUp; break;
            case 0x14: qtKeyCode = Qt::Key_VolumeDown; break;

            // Keys on right hand side of device
            case 0x31: qtKeyCode = Qt::Key_F7; break;   // Key +
            case 0x30: qtKeyCode = Qt::Key_F8; break;   // Key -

            // Camera
            case 0x23: qtKeyCode = Qt::Key_F4; break;

            // Lock key on top of device
            case 0x36: qtKeyCode = Qt::Key_F29; break;

            // Key on headphones
            case 0x33: qtKeyCode = Qt::Key_F28; break;
        }

        qLog(Input) << "processKeyEvent(): key=" << qtKeyCode << ", unicode=" << unicode;

        processKeyEvent(unicode, qtKeyCode, modifiers, isPressed, false);

        if (isPressed)
            beginAutoRepeat(unicode, qtKeyCode, modifiers);
        else
            endAutoRepeat();

#ifdef GREENPHONE_KEYPAD_REPEAT
        if (isPressed)
        {
            m_repeatKeyCode = qtKeyCode;
            m_unicode = unicode;
            m_timer->start(400);
        }
        else
            m_timer->stop();
#endif
    }
}

#ifdef GREENPHONE_KEYPAD_REPEAT
void GreenphoneKbdHandler::repeat()
{
//    processKeyEvent(m_unicode, m_repeatKeyCode, Qt::NoModifier, false, true);
    processKeyEvent(m_unicode, m_repeatKeyCode, Qt::NoModifier, true, true);
    m_timer->start(80);
}
#endif

#endif // QT_QWS_GREENPHONE
