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
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
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

#include "qlabel.h"
#include "qx11info_x11.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qbitmap.h"
#include "qevent.h"
#include "qapplication.h"
#include "qlist.h"
#include "qmenu.h"
#include "qtimer.h"
#include "qsystemtrayicon_p.h"

#ifndef QT_NO_SYSTEMTRAYICON
QT_BEGIN_NAMESPACE

Window QSystemTrayIconSys::sysTrayWindow = None;
QList<QSystemTrayIconSys *> QSystemTrayIconSys::trayIcons;
QCoreApplication::EventFilter QSystemTrayIconSys::oldEventFilter = 0;
Atom QSystemTrayIconSys::sysTraySelection = None;

// Locate the system tray
Window QSystemTrayIconSys::locateSystemTray()
{
    Display *display = QX11Info::display();
    if (sysTraySelection == None) {
        int screen = QX11Info::appScreen();
        QString net_sys_tray = QString::fromLatin1("_NET_SYSTEM_TRAY_S%1").arg(screen);
        sysTraySelection = XInternAtom(display, net_sys_tray.toLatin1(), False);
    }

    return XGetSelectionOwner(QX11Info::display(), sysTraySelection);
}

bool QSystemTrayIconSys::sysTrayTracker(void *message, long *result)
{
    bool retval = false;
    if (QSystemTrayIconSys::oldEventFilter)
        retval = QSystemTrayIconSys::oldEventFilter(message, result);

    if (trayIcons.isEmpty())
        return retval;

    Display *display = QX11Info::display();
    XAnyEvent *ev = (XAnyEvent *)message;
    if  (ev->type == DestroyNotify && ev->window == sysTrayWindow) {
	sysTrayWindow = locateSystemTray();
        for (int i = 0; i < trayIcons.count(); i++) {
            if (sysTrayWindow == None) {
	        QBalloonTip::hideBalloon();
                trayIcons[i]->hide(); // still no luck
                trayIcons[i]->destroy();
                trayIcons[i]->create();
	    } else
                trayIcons[i]->addToTray(); // add it to the new tray
        }
        retval = true;
    } else if (ev->type == ClientMessage && sysTrayWindow == None) {
        static Atom manager_atom = XInternAtom(display, "MANAGER", False);
        XClientMessageEvent *cm = (XClientMessageEvent *)message;
        if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == sysTraySelection)) {
	    sysTrayWindow = cm->data.l[2];
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask);
            for (int i = 0; i < trayIcons.count(); i++) {
                trayIcons[i]->addToTray();
            }
            retval = true;
        }
    }

    return retval;
}

QSystemTrayIconSys::QSystemTrayIconSys(QSystemTrayIcon *q)
    : QWidget(0, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint),
      q(q)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    static bool eventFilterAdded = false;
    Display *display = QX11Info::display();
    if (!eventFilterAdded) {
        oldEventFilter = qApp->setEventFilter(sysTrayTracker);
	eventFilterAdded = true;
	Window root = QX11Info::appRootWindow();
        XWindowAttributes attr;
        XGetWindowAttributes(display, root, &attr);
        if ((attr.your_event_mask & StructureNotifyMask) != StructureNotifyMask) {
            (void) QApplication::desktop(); // lame trick to ensure our event mask is not overridden
            XSelectInput(display, root, attr.your_event_mask | StructureNotifyMask); // for MANAGER selection
        }
    }
    if (trayIcons.isEmpty()) {
        sysTrayWindow = locateSystemTray();
	if (sysTrayWindow != None)
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask); // track tray events
    }
    trayIcons.append(this);
    setMouseTracking(true);
#ifndef QT_NO_TOOLTIP
    setToolTip(q->toolTip());
#endif
    if (sysTrayWindow != None)
        addToTray();
}

QSystemTrayIconSys::~QSystemTrayIconSys()
{
    trayIcons.removeAt(trayIcons.indexOf(this));
    if (trayIcons.isEmpty()) {
        Display *display = QX11Info::display();
        if (sysTrayWindow == None)
            return;
        if (display)
            XSelectInput(display, sysTrayWindow, 0); // stop tracking the tray
        sysTrayWindow = None;
    }
}

void QSystemTrayIconSys::addToTray()
{
    Q_ASSERT(sysTrayWindow != None);
    Display *display = QX11Info::display();
    Window wid = winId();

    XSetWindowBackgroundPixmap(display, wid, ParentRelative);

    // GNOME, NET WM Specification
    static Atom netwm_tray_atom = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
    long l[5] = { CurrentTime, SYSTEM_TRAY_REQUEST_DOCK, wid, 0, 0 };
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = sysTrayWindow;
    ev.xclient.message_type = netwm_tray_atom;
    ev.xclient.format = 32;
    memcpy((char *)&ev.xclient.data, (const char *) l, sizeof(l));
    XSendEvent(display, sysTrayWindow, False, 0, &ev);
    setMinimumSize(22, 22); // required at least on GNOME
}

void QSystemTrayIconSys::updateIcon()
{
    update();
}

void QSystemTrayIconSys::resizeEvent(QResizeEvent *re)
{
     QWidget::resizeEvent(re);
     updateIcon();
}

void QSystemTrayIconSys::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, background);
    q->icon().paint(&p, rect());
}

void QSystemTrayIconSys::mousePressEvent(QMouseEvent *ev)
{
    QPoint globalPos = ev->globalPos();
    if (ev->button() == Qt::RightButton && q->contextMenu())
        q->contextMenu()->popup(globalPos);

    if (ev->button() == Qt::LeftButton)
        emit q->activated(QSystemTrayIcon::Trigger);
    else if (ev->button() == Qt::RightButton)
        emit q->activated(QSystemTrayIcon::Context);
    else if (ev->button() == Qt::MidButton)
        emit q->activated(QSystemTrayIcon::MiddleClick);
}

void QSystemTrayIconSys::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit q->activated(QSystemTrayIcon::DoubleClick);
}

void QSystemTrayIconSys::wheelEvent(QWheelEvent *e)
{
    QApplication::sendEvent(q, e);
}

bool QSystemTrayIconSys::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        return QApplication::sendEvent(q, e);
    }
    return QWidget::event(e);
}

bool QSystemTrayIconSys::x11Event(XEvent *event)
{
    if (event->type == ReparentNotify)
        show();
    else if (event->type == ConfigureNotify || event->type == Expose) {
        XClearArea(QX11Info::display(), winId(), 0, 0, width(), height(), False);
        qApp->syncX();
        background = QPixmap::grabWindow(winId());
        update();
    }
    return QWidget::x11Event(event);
}

////////////////////////////////////////////////////////////////////////////
void QSystemTrayIconPrivate::install_sys()
{
    Q_Q(QSystemTrayIcon);
    if (!sys)
        sys = new QSystemTrayIconSys(q);
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
    if (!sys)
	return QRect();
    return QRect(sys->mapToGlobal(QPoint(0, 0)), sys->size());
}

void QSystemTrayIconPrivate::remove_sys()
{
    if (!sys)
        return;
    QBalloonTip::hideBalloon();
    sys->hide(); // this should do the trick, but...
    delete sys; // wm may resize system tray only for DestroyEvents
    sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
    if (!sys)
        return;
    sys->updateIcon();
}

void QSystemTrayIconPrivate::updateMenu_sys()
{

}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
    if (!sys)
        return;
#ifndef QT_NO_TOOLTIP
    sys->setToolTip(toolTip);
#endif
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return QSystemTrayIconSys::locateSystemTray() != None;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &message, const QString &title,
                                   QSystemTrayIcon::MessageIcon icon, int msecs)
{
    if (!sys)
        return;
    QPoint g = sys->mapToGlobal(QPoint(0, 0));
    QBalloonTip::showBalloon(icon, message, title, sys->q,
                             QPoint(g.x() + sys->width()/2, g.y() + sys->height()/2),
                             msecs);
}

QT_END_NAMESPACE
#endif //QT_NO_SYSTEMTRAYICON
