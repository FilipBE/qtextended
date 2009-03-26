/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qkbdtty_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qscreen_qws.h"

#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qsocketnotifier.h"
#include "qnamespace.h"
#include "qtimer.h"
#include <private/qwssignalhandler_p.h>
#include <private/qwindowsurface_qws_p.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>

#include <qeventloop.h>

#ifdef Q_OS_LINUX
#include <sys/kd.h>
#include <sys/vt.h>
#endif

QT_BEGIN_NAMESPACE

#define VTACQSIG SIGUSR1
#define VTRELSIG SIGUSR2

static int  vtQws = 0;
static int  kbdFD = -1;

//===========================================================================

//
// Tty keyboard
//

#ifndef QT_NO_QWS_KBD_TTY

class QWSTtyKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSTtyKbPrivate(QWSPC101KeyboardHandler *, const QString &device);
    ~QWSTtyKbPrivate();

private slots:
    void readKeyboardData();
    void handleTtySwitch(int);

private:
    QWSPC101KeyboardHandler *handler;
    struct termios origTermData;
};

QWSTtyKeyboardHandler::QWSTtyKeyboardHandler(const QString &device)
    : QWSPC101KeyboardHandler(device)
{
    d = new QWSTtyKbPrivate(this, device);
}

QWSTtyKeyboardHandler::~QWSTtyKeyboardHandler()
{
    delete d;
}

void QWSTtyKeyboardHandler::processKeyEvent(int unicode, int keycode,
                                            Qt::KeyboardModifiers modifiers, bool isPress,
                                            bool autoRepeat)
{
#if defined(Q_OS_LINUX)
    // Virtual console switching
    int term = 0;
    bool ctrl = modifiers & Qt::ControlModifier;
    bool alt = modifiers & Qt::AltModifier;
    if (ctrl && alt && keycode >= Qt::Key_F1 && keycode <= Qt::Key_F10)
        term = keycode - Qt::Key_F1 + 1;
    else if (ctrl && alt && keycode == Qt::Key_Left)
        term = qMax(vtQws - 1, 1);
    else if (ctrl && alt && keycode == Qt::Key_Right)
        term = qMin(vtQws + 1, 10);
    if (term && isPress) {
        ioctl(kbdFD, VT_ACTIVATE, term);
        return;
    }
#endif

    QWSPC101KeyboardHandler::processKeyEvent(unicode, keycode, modifiers,
        isPress, autoRepeat);
}


QWSTtyKbPrivate::QWSTtyKbPrivate(QWSPC101KeyboardHandler *h, const QString &device) : handler(h)
{
    kbdFD = ::open(device.isEmpty()?"/dev/tty0":device.toLatin1().constData(), O_RDWR|O_NDELAY, 0);
#ifndef QT_NO_QWS_SIGNALHANDLER
    QWSSignalHandler::instance()->addObject(this);
#endif

    if (kbdFD >= 0) {
        QSocketNotifier *notifier;
        notifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)),this,
                 SLOT(readKeyboardData()));

        // save for restore.
        tcgetattr(kbdFD, &origTermData);

        struct termios termdata;
        tcgetattr(kbdFD, &termdata);

#if defined(Q_OS_LINUX)
# ifdef QT_QWS_USE_KEYCODES
        ioctl(kbdFD, KDSKBMODE, K_MEDIUMRAW);
# else
        ioctl(kbdFD, KDSKBMODE, K_RAW);
# endif
#endif

        termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
        termdata.c_oflag = 0;
        termdata.c_cflag = CREAD | CS8;
        termdata.c_lflag = 0;
        termdata.c_cc[VTIME]=0;
        termdata.c_cc[VMIN]=1;
        cfsetispeed(&termdata, 9600);
        cfsetospeed(&termdata, 9600);
        tcsetattr(kbdFD, TCSANOW, &termdata);

#if defined(Q_OS_LINUX)

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
#endif
    } else {
        qCritical("Cannot open keyboard: %s", strerror(errno));
    }

}

QWSTtyKbPrivate::~QWSTtyKbPrivate()
{
    if (kbdFD >= 0) {
#if defined(Q_OS_LINUX)
        ioctl(kbdFD, KDSKBMODE, K_XLATE);
#endif
        tcsetattr(kbdFD, TCSANOW, &origTermData);
        ::close(kbdFD);
        kbdFD = -1;
    }
}

void QWSTtyKbPrivate::handleTtySwitch(int sig)
{
#if defined(Q_OS_LINUX)
    if (sig == VTACQSIG) {
        if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
            qwsServer->enablePainting(true);
            qt_screen->restore();
            qwsServer->resumeMouse();
            qwsServer->refresh();
        }
    } else if (sig == VTRELSIG) {
        qwsServer->enablePainting(false);

        // Check for reserved surfaces which might still do painting
        bool allWindowsHidden = true;
        const QList<QWSWindow*> windows = QWSServer::instance()->clientWindows();
        for (int i = 0; i < windows.size(); ++i) {
            const QWSWindow *w = windows.at(i);
            QWSWindowSurface *s = w->windowSurface();
            if (s && s->isRegionReserved() && !w->allocatedRegion().isEmpty()) {
                allWindowsHidden = false;
                break;
            }
        }

        if (!allWindowsHidden) {
            ioctl(kbdFD, VT_RELDISP, 0); // abort console switch
            qwsServer->enablePainting(true);
        } else if (ioctl(kbdFD, VT_RELDISP, 1) == 0) {
            qt_screen->save();
            qwsServer->suspendMouse();
        } else {
            qwsServer->enablePainting(true);
        }
    }
#endif
}

void QWSTtyKbPrivate::readKeyboardData()
{
    unsigned char buf[81];
    int n = read(kbdFD, buf, 80);
    for (int loop = 0; loop < n; loop++)
        handler->doKey(buf[loop]);
}

#endif // QT_NO_QWS_KBD_TTY

QT_END_NAMESPACE

#include "qkbdtty_qws.moc"

#endif // QT_NO_QWS_KEYBOARD
