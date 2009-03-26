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

#include "windowmanagement.h"

#ifdef Q_WS_X11

#include <QX11Info>
#include <QtopiaApplication>
#include <QRect>
#include <QWidget>
#include <QDebug>
#include <QValueSpaceObject>
#include <QContent>
#include <QSet>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

class WindowManagementPrivate : public QObject, QtopiaApplication::X11EventFilter
{
    Q_OBJECT
public:
    WindowManagementPrivate(QObject *parent=0);
    ~WindowManagementPrivate();

    bool filterEvent(void *message, long *result);
    QString activeAppName() const { return prevActive; }

    bool x11EventFilter(XEvent *event);

signals:
    void windowActive(const QString &, const QRect &, WId);
    void windowCaption(const QString &);

private:
    Atom netActiveWindowAtom;
    Atom utf8StringAtom;
    Atom netWmNameAtom;
    Atom netWmVisibleNameAtom;
    Atom netWmWindowTypeAtom;
    Atom netWmWindowTypeNormalAtom;
    Atom netWmState;
    Atom netWmStateModal;
    Atom wmNameAtom;
    WId activeId;
    QString prevActive;
    QString prevCaption;
    QRect prevRect;
    QValueSpaceObject *vs;
    QSet<WId> monitoredWindows;

    void activeChanged(WId w);
};

Q_GLOBAL_STATIC(WindowManagementPrivate, windowManagement);

WindowManagementPrivate::WindowManagementPrivate(QObject *parent)
    : QObject(parent)
{
    netActiveWindowAtom = 0;
    utf8StringAtom = 0;
    netWmNameAtom = 0;
    netWmVisibleNameAtom = 0;
    netWmWindowTypeAtom = 0;
    netWmWindowTypeNormalAtom = 0;
    activeId = 0;
    prevCaption = QString("");
    prevActive = QString("");
    vs = new QValueSpaceObject("/UI/ActiveWindow", this);

    QtopiaApplication::instance()->installX11EventFilter(this);
}

WindowManagementPrivate::~WindowManagementPrivate()
{
    QtopiaApplication::instance()->removeX11EventFilter(this);
}

bool WindowManagementPrivate::x11EventFilter(XEvent *event)
{
    // Determine if the active window has changed.
    if (!netActiveWindowAtom) {
        netActiveWindowAtom = XInternAtom
            (QX11Info::display(), "_NET_ACTIVE_WINDOW", False );
        utf8StringAtom = XInternAtom
            (QX11Info::display(), "UTF8_STRING", False );
        netWmNameAtom = XInternAtom
            (QX11Info::display(), "_NET_WM_NAME", False );
        netWmVisibleNameAtom = XInternAtom
            (QX11Info::display(), "_NET_WM_VISIBLE_NAME", False );
        netWmWindowTypeAtom = XInternAtom
            (QX11Info::display(), "_NET_WM_WINDOW_TYPE", False );
        netWmWindowTypeNormalAtom = XInternAtom
            (QX11Info::display(), "_NET_WM_WINDOW_TYPE_NORMAL", False );
        netWmState = XInternAtom
            (QX11Info::display(), "_NET_WM_STATE", False );
        netWmStateModal = XInternAtom
            (QX11Info::display(), "_NET_WM_STATE_MODAL", False );
        wmNameAtom = XInternAtom
            (QX11Info::display(), "WM_NAME", False );
    }
    if (event->xany.type == PropertyNotify &&
        event->xproperty.window == QX11Info::appRootWindow() &&
        event->xproperty.atom == netActiveWindowAtom) {
        
        Atom actualType;
        int actualFormat;
        unsigned long size;
        unsigned long bytesAfterReturn;
        unsigned char *value;
        activeId = 0;
        if (XGetWindowProperty
                (QX11Info::display(), QX11Info::appRootWindow(),
                 netActiveWindowAtom, 0, 1, False, XA_WINDOW,
                 &actualType, &actualFormat, &size, &bytesAfterReturn,
                 &value ) == Success ) {
            activeId = (WId)(*((unsigned long *)value));
            XFree(value);
        }
        activeChanged(activeId);
    }

    // Is this one of the client windows we were monitoring?
    WId w = (WId)(event->xany.window);
    if (monitoredWindows.contains(w)) {
        if (event->xany.type == DestroyNotify) {
            // The window has been destroyed.
            monitoredWindows -= w;
            if (w == activeId) {
                activeId = 0;
                activeChanged(0);
            }
        } else if (event->xany.type == PropertyNotify) {
            if (event->xproperty.atom == netWmNameAtom ||
                event->xproperty.atom == netWmVisibleNameAtom ||
                event->xproperty.atom == wmNameAtom) {
                // The window's caption has changed.
                if (w == activeId)
                    activeChanged(w);
            }
        } else if (event->xany.type == UnmapNotify) {
            // The window has been withdrawn from the screen.
            if (w == activeId) {
                activeId = 0;
                activeChanged(0);
            }
        }
    }

    // Let the event propagate normally.
    return false;
}

static int nullErrorHandler(Display * /* display */, XErrorEvent* /* error_event */ )
{
    return 0;
}

static QString readUtf8Property(Display *dpy, WId w, Atom property, Atom type)
{
    Atom actualType;
    int actualFormat;
    unsigned long size;
    unsigned long bytesAfterReturn;
    unsigned char *value;
    if (XGetWindowProperty
            (dpy, (Window)w, property, 0, 1024, False, type,
             &actualType, &actualFormat, &size, &bytesAfterReturn,
             &value ) != Success || !value) {
        if (value)
            XFree(value);
        return QString();
    }
    QString result = QString::fromUtf8((char *)value, (int)size);
    XFree(value);
    return result;
}

static Atom readAtomProperty(Display *dpy, WId w, Atom property)
{
    Atom actualType;
    int actualFormat;
    unsigned long size;
    unsigned long bytesAfterReturn;
    unsigned char *value;
    if (XGetWindowProperty
            (dpy, (Window)w, property, 0, 1, False, XA_ATOM,
             &actualType, &actualFormat, &size, &bytesAfterReturn,
             &value ) != Success || !value) {
        if (value)
            XFree(value);
        return 0;
    }
    Atom result = *((Atom *)value);
    XFree(value);
    return result;
}

static bool checkForAtomProperty(Display *dpy, WId w, Atom property, Atom expected)
{
    Atom actualType;
    int actualFormat;
    unsigned long size;
    unsigned long bytesAfterReturn;
    unsigned char *value;
    if (XGetWindowProperty
            (dpy, (Window)w, property, 0, 32, False, XA_ATOM,
             &actualType, &actualFormat, &size, &bytesAfterReturn,
             &value ) != Success || !value) {
        if (value)
            XFree(value);
        return 0;
    }
    while (size > 0) {
        --size;
        if (((Atom *)value)[size] == expected) {
            XFree(value);
            return true;
        }
    }
    XFree(value);
    return false;
}

void WindowManagementPrivate::activeChanged(WId w)
{
    QString caption;
    QString appName;
    QRect rect;
    bool normal = true;

    // Fetch the window size and caption.
    if (w) {
        // Synchronize against the X display and set a null error handler.
        // This is to discard error messages if the window has gone away
        // between the time that we received the active changed message
        // and when we went looking for the window details.
        Display *dpy = QX11Info::display();
        XSync(dpy, False);
        XErrorHandler oldHandler = XSetErrorHandler(nullErrorHandler);

        // Get the window size information from XGetWindowAttributes,
        // and translate the client window's top-level position to root
        // co-ordinates using XTranslateCoordinates.
        XWindowAttributes attrs;
        int x, y;
        Window child;
        memset(&attrs, 0, sizeof(attrs));
        if (XGetWindowAttributes(dpy, (Window)w, &attrs) &&
            XTranslateCoordinates(dpy, (Window)w, attrs.root, 0, 0, &x, &y, &child)) {
            rect = QRect(x, y, attrs.width, attrs.height);
        } else {
            // Window has disappeared, so wait for next active window change.
            XSync(dpy, False);
            XSetErrorHandler(oldHandler);
            return;
        }

        // Read the window caption information.
        caption = readUtf8Property(dpy, w, netWmVisibleNameAtom, utf8StringAtom);
        if (caption.isEmpty())
            caption = readUtf8Property(dpy, w, netWmNameAtom, utf8StringAtom);
        if (caption.isEmpty()) {
            // Old-style X11 application.  Retrieve WM_NAME.
            XTextProperty textProp;
            if (XGetWMName(dpy, (Window)w, &textProp)) {
                char **list;
                int count;
                if (XTextPropertyToStringList(&textProp, &list, &count)) {
                    for (int index = 0; index < count; ++index)
                        caption += QString::fromLocal8Bit(list[index]);
                    XFreeStringList(list);
                }
            }
        }

        // Read the window type to determine if it is normal, dialog, or docked.
        Atom windowType = readAtomProperty(dpy, w, netWmWindowTypeAtom);
        Window transient = 0;
        XGetTransientForHint(dpy, (Window)w, &transient);
        if (windowType) {
            normal = (windowType == netWmWindowTypeNormalAtom);
        } else {
            // No window type, so if there is a transient-for hint,
            // it is probably a dialog, otherwise normal.
            normal = (transient != 0);
        }
        if (transient != 0 && !normal) {
            // If the window is transient, but not modal, then treat it as normal.
            if (!checkForAtomProperty(dpy, w, netWmState, netWmStateModal))
                normal = true;
        }

        // Get the class hint to determine the application's name.
        XClassHint classHint;
        if (XGetClassHint(dpy, (Window)w, &classHint)) {
            appName = QString::fromLatin1(classHint.res_name);
            XFree(classHint.res_name);
            XFree(classHint.res_class);
        }

        // Monitor the window for property changes and destroy notifications.
        if (!monitoredWindows.contains(w)) {
            monitoredWindows += w;
            if (!QWidget::find(w))  // Don't select on our own windows.
                XSelectInput(dpy, (Window)w, StructureNotifyMask | PropertyChangeMask);
        }

        // Sync again to flush errors and restore the original error handler.
        XSync(dpy, False);
        XSetErrorHandler(oldHandler);

        // If the caption is "_ignore_", we should ingore this window.
        // This is used by quicklauncher.
        if (caption == QLatin1String("_ignore_"))   // No tr
            return;
    }

    // Caption should only be emitted when a normal full-screen window
    // is made active.  Dialogs and docked apps shouldn't change it.
    if (normal) {
        if (appName != prevActive || caption != prevCaption || rect != prevRect) {
            prevActive = appName;
            prevCaption = caption;
            vs->setAttribute("Caption", caption);
            QString iconName;
            QContentId cid = QContent::execToContent(appName);
            if (cid != QContent::InvalidId) {
                QContent app(cid);
                iconName = QLatin1String(":icon/")+app.iconName();
            }
            vs->setAttribute("Icon", iconName);
            emit windowCaption(caption);
        }
    }

    // Advertise the change in active window.
    bool update = false;
    if (rect != prevRect) {
        prevRect = rect;
        vs->setAttribute("Rect", rect);
        update = true;
    }
    if (caption != prevCaption) {
        prevCaption = caption;
        vs->setAttribute("Title", caption);
        update = true;
    }
    if (update) {
        emit windowActive(caption, rect, w);
    }
}

WindowManagement::WindowManagement(QObject *parent)
    : QObject(parent)
{
    WindowManagementPrivate *priv = windowManagement();
    connect(priv, SIGNAL(windowActive(QString,QRect,WId)),
            this, SIGNAL(windowActive(QString,QRect,WId)));
    connect(priv, SIGNAL(windowCaption(QString)),
            this, SIGNAL(windowCaption(QString)));
}

WindowManagement::~WindowManagement()
{
}

void WindowManagement::protectWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());

    window->setWindowFlags(window->windowFlags() | Qt::WindowStaysOnTopHint);
}

void WindowManagement::dockWindow
    (QWidget *window, DockArea placement, int screen)
{
    dockWindow(window, placement, QSize(), screen);
}

void WindowManagement::dockWindow
    (QWidget *window, DockArea placement, const QSize &size, int /*screen*/)
{
    Q_ASSERT(window->isWindow());
    QSize sz(size);

    Atom winTypeAtom = XInternAtom
        (QX11Info::display(), "_NET_WM_WINDOW_TYPE", False);
    Atom dockAtom = XInternAtom
        (QX11Info::display(), "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atom strutAtom = XInternAtom
        (QX11Info::display(), "_NET_WM_STRUT", False);
    Atom partialStrutAtom = XInternAtom
        (QX11Info::display(), "_NET_WM_STRUT_PARTIAL", False);

    if (!sz.isValid())
        sz = window->sizeHint();

    // Compose the complete strut definition and determine the initial placement.
    unsigned long strut[12];
    memset(strut, 0, sizeof(strut));
    int displayWidth = DisplayWidth(QX11Info::display(), QX11Info::appScreen());
    int displayHeight = DisplayHeight(QX11Info::display(), QX11Info::appScreen());
    QRect geometry(0, 0, sz.width(), sz.height());
    switch (placement) {
        case Top:
        {
            strut[2] = sz.height();
            strut[9] = displayWidth;
            geometry.setWidth(displayWidth);
        }
        break;

        case Bottom:
        {
            strut[3] = sz.height();
            strut[11] = displayWidth;
            geometry.setWidth(displayWidth);
            geometry.setY(displayHeight - sz.height());
            geometry.setHeight(sz.height());
        }
        break;

        case Left:
        {
            strut[0] = sz.width();
            strut[5] = displayHeight;
            geometry.setHeight(displayHeight);
        }
        break;

        case Right:
        {
            strut[1] = sz.width();
            strut[7] = displayHeight;
            geometry.setHeight(displayHeight);
            geometry.setX(displayWidth - sz.width());
            geometry.setWidth(sz.width());
        }
        break;
    }
    window->setGeometry(geometry);

    // Set the window properties.  _NET_WM_STRUT is used by older window
    // managers and _NET_WM_STRUT_PARTIAL is used by newer window managers.
    XChangeProperty(QX11Info::display(), window->winId(),
                    winTypeAtom, XA_ATOM, 32, PropModeReplace,
                    (unsigned char *)&dockAtom, 1);
    XChangeProperty(QX11Info::display(), window->winId(),
                    strutAtom, XA_CARDINAL, 32, PropModeReplace,
                    (unsigned char *)strut, 4);
    XChangeProperty(QX11Info::display(), window->winId(),
                    partialStrutAtom, XA_CARDINAL, 32, PropModeReplace,
                    (unsigned char *)strut, 12);

    // Mark the window as not wanting keyboard input so that the user cannot
    // click on it and transfer focus away from where it should be.
    XWMHints *hints = XGetWMHints(QX11Info::display(), window->winId());
    if (hints) {
        hints->input = False;
        XSetWMHints(QX11Info::display(), window->winId(), hints);
        XFree(hints);
    }
}

void WindowManagement::setLowestWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());

    Atom typeAtom = XInternAtom
        (QX11Info::display(), "_NET_WM_WINDOW_TYPE", False);
    Atom desktopAtom = XInternAtom
        (QX11Info::display(), "_NET_WM_WINDOW_TYPE_DESKTOP", False);

    XChangeProperty(QX11Info::display(), window->winId(),
                    typeAtom, XA_ATOM, 32, PropModeReplace,
                    (unsigned char *)&desktopAtom, 1);
}

void WindowManagement::showDockedWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());
    window->show();
}

void WindowManagement::hideDockedWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());
    window->hide();
}

QString WindowManagement::activeAppName()
{
    return windowManagement()->activeAppName();
}

bool WindowManagement::supportsSoftMenus(WId winId)
{
    if (!winId)
        return false;

    // Synchronize against the X display and set a null error handler.
    // This is to discard error messages if the window has gone away.
    Display *dpy = QX11Info::display();
    XSync(dpy, False);
    XErrorHandler oldHandler = XSetErrorHandler(nullErrorHandler);

    // Retrieve the contents of the _QTOPIA_SOFT_MENUS property.
    Atom property = XInternAtom(dpy, "_QTOPIA_SOFT_MENUS", False);
    Atom actualType;
    int actualFormat;
    unsigned long size;
    unsigned long bytesAfterReturn;
    unsigned char *value;
    bool usesSoftMenus = false;
    if (XGetWindowProperty
            (dpy, (Window)winId, property, 0, 1, False, XA_CARDINAL,
             &actualType, &actualFormat, &size, &bytesAfterReturn,
             &value ) == Success && value) {
        usesSoftMenus = (*((unsigned long *)value) != 0);
    }
    if (value)
        XFree(value);

    // Sync again to flush errors and restore the original error handler.
    XSync(dpy, False);
    XSetErrorHandler(oldHandler);
    return usesSoftMenus;
}

void WindowManagement::closeWindow(WId winId)
{
    Display *dpy = QX11Info::display();
    if (dpy && winId) {
        XEvent event;
        memset(&event, 0, sizeof(event));
        event.xclient.type = ClientMessage;
        event.xclient.window = (Window)winId;
        event.xclient.message_type = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
        event.xclient.format = 32;
        event.xclient.data.l[0] = QX11Info::appUserTime();
        event.xclient.data.l[1] = 2;    // Closed by a "pager".
        XSendEvent(dpy, QX11Info::appRootWindow(), False,
                   (SubstructureNotifyMask | SubstructureRedirectMask),
                   &event);
    }
}

#include "windowmanagement_x11.moc"

#endif // Q_WS_X11
