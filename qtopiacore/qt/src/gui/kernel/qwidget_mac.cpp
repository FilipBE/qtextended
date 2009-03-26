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
//#define QT_RASTER_PAINTENGINE

#include <private/qt_mac_p.h>

#include "qapplication.h"
#include "qapplication_p.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qimage.h"
#include "qlayout.h"
#include "qmenubar.h"
#ifdef QT_RASTER_PAINTENGINE
# include <private/qpaintengine_raster_p.h>
#endif
#include <private/qpaintengine_mac_p.h>
#include "qpainter.h"
#include "qstyle.h"
#include "qtimer.h"
#include "qfocusframe.h"
#include "qdebug.h"
#include <private/qmainwindowlayout_p.h>

#include <private/qabstractscrollarea_p.h>
#include <qabstractscrollarea.h>
#include <ApplicationServices/ApplicationServices.h>
#include <limits.h>

#include "qwidget_p.h"
#include "qdnd_p.h"

QT_BEGIN_NAMESPACE

#define XCOORD_MAX 16383
#define WRECT_MAX 8191


/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS
//#define DEBUG_WINDOW_CREATE
//#define DEBUG_WINDOW_STATE
//#define DEBUG_WIDGET_PAINT

/*****************************************************************************
  QWidget globals
 *****************************************************************************/
typedef QHash<Qt::WindowFlags, WindowGroupRef> WindowGroupHash;
Q_GLOBAL_STATIC(WindowGroupHash, qt_mac_window_groups)
static bool qt_mac_raise_process = true;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
const UInt32 kWidgetCreatorQt = 'cute';
enum {
    kWidgetPropertyQWidget = 'QWId' //QWidget *
};

#ifdef QT_NAMESPACE

// produce the string "com.trolltech.qt-namespace.widget", where "namespace" is the contents of QT_NAMESPACE.
#define SS(x) #x
#define S0(x) SS(x)
#define S "com.trolltech.qt-" S0(QT_NAMESPACE) ".widget"

static CFStringRef kObjectQWidget = CFSTR(S);

#undef SS
#undef S0
#undef S

#else
static CFStringRef kObjectQWidget = CFSTR("com.trolltech.qt.widget");
#endif

Q_GUI_EXPORT QPoint qt_mac_posInWindow(const QWidget *w);

/*****************************************************************************
  Externals
 *****************************************************************************/
extern QWidget *qt_mac_modal_blocked(QWidget *); //qapplication_mac.cpp
extern void qt_event_request_activate(QWidget *); //qapplication_mac.cpp
extern bool qt_event_remove_activate(); //qapplication_mac.cpp
extern void qt_mac_event_release(QWidget *w); //qapplication_mac.cpp
extern void qt_event_request_showsheet(QWidget *); //qapplication_mac.cpp
extern void qt_event_request_window_change(QWidget *); //qapplication_mac.cpp
extern IconRef qt_mac_create_iconref(const QPixmap &); //qpixmap_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const QPoint &); //qcursor_mac.cpp
extern void qt_mac_update_cursor(); //qcursor_mac.cpp
extern bool qt_nograb();
extern CGImageRef qt_mac_create_cgimage(const QPixmap &, bool); //qpixmap_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle rgn); //qregion_mac.cpp

/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/
void Q_GUI_EXPORT qt_mac_set_raise_process(bool b) { qt_mac_raise_process = b; }
static QSize qt_mac_desktopSize()
{
    int w = 0, h = 0;
    CGDisplayCount cg_count;
    CGGetActiveDisplayList(0, 0, &cg_count);
    QVector<CGDirectDisplayID> displays(cg_count);
    CGGetActiveDisplayList(cg_count, displays.data(), &cg_count);
    Q_ASSERT(cg_count == (CGDisplayCount)displays.size());
    for(int i = 0; i < (int)cg_count; ++i) {
        CGRect r = CGDisplayBounds(displays.at(i));
        w = qMax<int>(w, qRound(r.origin.x + r.size.width));
        h = qMax<int>(h, qRound(r.origin.y + r.size.height));
    }
    return QSize(w, h);
}

bool qt_mac_can_clickThrough(const QWidget *w)
{
    static int qt_mac_carbon_clickthrough = -1;
    if (qt_mac_carbon_clickthrough < 0)
        qt_mac_carbon_clickthrough = !qgetenv("QT_MAC_NO_COCOA_CLICKTHROUGH").isEmpty();
    bool ret = !qt_mac_carbon_clickthrough;
    for ( ; w; w = w->parentWidget()) {
        if (w->testAttribute(Qt::WA_MacNoClickThrough)) {
            ret = false;
            break;
        }
    }
    return ret;
}

bool qt_mac_is_macsheet(const QWidget *w)
{
    return (w && w->windowType() == Qt::Sheet
       && w->parentWidget() && w->parentWidget()->window()->windowType() != Qt::Desktop
       && w->parentWidget()->window()->isVisible());
}

bool qt_mac_is_macdrawer(const QWidget *w)
{
    return (w && w->parentWidget() && w->windowType() == Qt::Drawer);
}

bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::DockWidgetArea where) //users of Qt for Mac OS X can use this..
{
    if(!qt_mac_is_macdrawer(w))
        return false;
    OptionBits edge;
    if(where & Qt::LeftDockWidgetArea)
        edge = kWindowEdgeLeft;
    else if(where & Qt::RightDockWidgetArea)
        edge = kWindowEdgeRight;
    else if(where & Qt::TopDockWidgetArea)
        edge = kWindowEdgeTop;
    else if(where & Qt::BottomDockWidgetArea)
        edge = kWindowEdgeBottom;
    else
        return false;
    WindowPtr window = qt_mac_window_for(w);
    if(edge == GetDrawerPreferredEdge(window)) //no-op
        return false;
    //do it
    SetDrawerPreferredEdge(window, edge);
    if(w->isVisible()) {
        CloseDrawer(window, false);
        OpenDrawer(window, edge, true);
    }
    return true;
}

QPoint qt_mac_posInWindow(const QWidget *w)
{
    QPoint ret = w->data->wrect.topLeft();
    while(w && !w->isWindow()) {
        ret += w->pos();
        w =  w->parentWidget();
    }
    return ret;
}

//find a QWidget from a WindowPtr
QWidget *qt_mac_find_window(WindowPtr window)
{
    if(!window)
        return 0;

    QWidget *ret;
    if(GetWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(ret), 0, &ret) == noErr)
        return ret;
    return 0;
}

inline static void qt_mac_set_fullscreen_mode(bool b)
{
    extern bool qt_mac_app_fullscreen; //qapplication_mac.cpp
    if(qt_mac_app_fullscreen == b)
        return;
    qt_mac_app_fullscreen = b;
#if 0
    if(b)
        SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
    else
        SetSystemUIMode(kUIModeNormal, 0);
#else
    if(b)
        HideMenuBar();
    else
        ShowMenuBar();
#endif
}

Q_GUI_EXPORT HIViewRef qt_mac_hiview_for(const QWidget *w)
{
    return (HIViewRef)w->data->winid;
}

Q_GUI_EXPORT HIViewRef qt_mac_hiview_for(WindowPtr w)
{
    HIViewRef ret = 0;
    OSStatus err = GetRootControl(w, &ret);  // Returns the window's content view (Apple QA1214)
    if (err == errUnknownControl) {
        ret = HIViewGetRoot(w);
    } else if (err != noErr) {
        qWarning("Qt:Could not get content or root view of window! %s:%d [%ld]",
                 __FILE__, __LINE__, err);
    }
    return ret;
}

Q_GUI_EXPORT WindowPtr qt_mac_window_for(HIViewRef hiview)
{
    return HIViewGetWindow(hiview);
}
Q_GUI_EXPORT WindowPtr qt_mac_window_for(const QWidget *w)
{
    HIViewRef hiview = qt_mac_hiview_for(w);
    if (hiview){
        WindowPtr window = qt_mac_window_for(hiview);
        if(!window && HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget)) {
            w->window()->d_func()->createWindow_sys();
            hiview = qt_mac_hiview_for(w);
            window = qt_mac_window_for(hiview);
        }
        return window;
    }
    return (WindowPtr) 0;
}

/*  Checks if the current group is a 'stay on top' group. If so, the
    group gets removed from the hash table */
static void qt_mac_release_stays_on_top_group(WindowGroupRef group)
{
    for (WindowGroupHash::iterator it = qt_mac_window_groups()->begin(); it != qt_mac_window_groups()->end(); ++it) {
        if (it.value() == group) {
            qt_mac_window_groups()->remove(it.key());
            return;
        }
    }
}

static bool qt_isGenuineQWidget(HIViewRef ref)
{
    return HIObjectIsOfClass(HIObjectRef(ref), kObjectQWidget);
}

bool qt_isGenuineQWidget(const QWidget *window)
{
    return window && qt_isGenuineQWidget(HIViewRef(window->winId()));
}

/* Use this function instead of ReleaseWindowGroup, this will be sure to release the
   stays on top window group (created with qt_mac_get_stays_on_top_group below) */
static void qt_mac_release_window_group(WindowGroupRef group)
{
    ReleaseWindowGroup(group);
    if (GetWindowGroupRetainCount(group) == 0)
        qt_mac_release_stays_on_top_group(group);
}
#define ReleaseWindowGroup(x) Are you sure you wanted to do that? (you wanted qt_mac_release_window_group)

SInt32 qt_mac_get_group_level(WindowClass wclass)
{
    SInt32 group_level;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        CGWindowLevel tmpLevel;
        GetWindowGroupLevelOfType(GetWindowGroupOfClass(wclass), kWindowGroupLevelActive, &tmpLevel);
        group_level = tmpLevel;
    } else
#endif
    {
        GetWindowGroupLevel(GetWindowGroupOfClass(wclass), &group_level);
    }
    return group_level;
}

/**
    Checks if the user has told us explicitly that he wants buttons on the
    title bar. Buttons are set by default on e.g. QDialog, but ignored by
    QWidgetPrivate::determineWindowClass if the dialog is modal...unless
    the user told us explicitly otherwise.
*/
static inline bool qt_mac_menu_buttons_explicitly_set(const Qt::WindowFlags &flags)
{
    // if CustomizeWindowHint is sat, together
    // with any of the buttons, return true:
    return (flags & Qt::CustomizeWindowHint
        && ((flags & Qt::WindowSystemMenuHint)
        || (flags & Qt::WindowMinimizeButtonHint)
        || (flags & Qt::WindowMaximizeButtonHint)));
}

static void qt_mac_set_window_group(WindowRef window, Qt::WindowFlags flags, int level)
{
    WindowGroupRef group = 0;
    if (qt_mac_window_groups()->contains(flags)) {
        group = qt_mac_window_groups()->value(flags);
        RetainWindowGroup(group);
    } else {
        CreateWindowGroup(kWindowActivationScopeNone, &group);
        SetWindowGroupLevel(group, level);
        SetWindowGroupParent(group, GetWindowGroupOfClass(kAllWindowClasses));
        qt_mac_window_groups()->insert(flags, group);
    }
    SetWindowGroup(window, group);
}

inline static void qt_mac_set_window_group_to_stays_on_top(WindowRef window, Qt::WindowType type)
{
    // We create one static stays on top window group so that
    // all stays on top (aka popups) will fall into the same
    // group and be able to be raise()'d with releation to one another (from
    // within the same window group).
    qt_mac_set_window_group(window, type|Qt::WindowStaysOnTopHint, qt_mac_get_group_level(kOverlayWindowClass));
}

inline static void qt_mac_set_window_group_to_tooltip(WindowRef window)
{
    // Since new groups are created for 'stays on top' windows, the
    // same must be done for tooltips. Otherwise, tooltips would be drawn
    // below 'stays on top' widgets even tough they are on the same level.
    // Also, add 'two' to the group level to make sure they also get on top of popups.
    qt_mac_set_window_group(window, Qt::ToolTip, qt_mac_get_group_level(kHelpWindowClass)+2);
}

inline static void qt_mac_set_window_group_to_popup(WindowRef window)
{
    // In Qt, a popup is seen as a 'stay on top' window.
    // Since new groups are created for 'stays on top' windows, the
    // same must be done for popups. Otherwise, popups would be drawn
    // below 'stays on top' windows. Add 1 to get above pure stay-on-top windows.
    qt_mac_set_window_group(window, Qt::Popup, qt_mac_get_group_level(kOverlayWindowClass)+1);
}

void QWidgetPrivate::macUpdateIsOpaque()
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created))
        return;
    HIViewFeatures bits;
    HIViewRef hiview = qt_mac_hiview_for(q);
    HIViewGetFeatures(hiview, &bits);
    const bool opaque = isOpaque();
    if ((bits & kHIViewIsOpaque) == opaque)
        return;
    if(opaque) {
        HIViewChangeFeatures(hiview, kHIViewIsOpaque, 0);
    } else {
        HIViewChangeFeatures(hiview, 0, kHIViewIsOpaque);
    }
    if (q->isVisible())
        HIViewReshapeStructure(qt_mac_hiview_for(q));
}

static OSStatus qt_mac_create_window(WindowClass wclass, WindowAttributes wattr,
                                     Rect *geo, WindowPtr *w)
{
    OSStatus ret;
    if(geo->right <= geo->left)
        geo->right = geo->left + 1;
    if(geo->bottom <= geo->top)
        geo->bottom = geo->top + 1;
    Rect null_rect; SetRect(&null_rect, 0, 0, 1, 1);
    ret = CreateNewWindow(wclass, wattr, &null_rect, w);
    if(ret == noErr) {
        ret = SetWindowBounds(*w, kWindowContentRgn, geo);
        if(ret != noErr)
            qWarning("QWidget: Internal error (%s:%d)", __FILE__, __LINE__);
    }
    return ret;
}

// window events
static EventTypeSpec window_events[] = {
    { kEventClassWindow, kEventWindowClose },
    { kEventClassWindow, kEventWindowExpanded },
    { kEventClassWindow, kEventWindowZoomed },
    { kEventClassWindow, kEventWindowCollapsed },
    { kEventClassWindow, kEventWindowToolbarSwitchMode },
    { kEventClassWindow, kEventWindowProxyBeginDrag },
    { kEventClassWindow, kEventWindowProxyEndDrag },
    { kEventClassWindow, kEventWindowResizeStarted },
    { kEventClassWindow, kEventWindowResizeCompleted },
    { kEventClassWindow, kEventWindowDragStarted },
    { kEventClassWindow, kEventWindowDragCompleted },
    { kEventClassWindow, kEventWindowBoundsChanging },
    { kEventClassWindow, kEventWindowBoundsChanged },
    { kEventClassWindow, kEventWindowGetRegion },
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    { kEventClassWindow, kEventWindowGetClickModality },
#endif
    { kEventClassWindow, kEventWindowTransitionCompleted },
    { kEventClassMouse, kEventMouseDown }
};
static EventHandlerUPP mac_win_eventUPP = 0;
static void cleanup_win_eventUPP()
{
    DisposeEventHandlerUPP(mac_win_eventUPP);
    mac_win_eventUPP = 0;
}
static const EventHandlerUPP make_win_eventUPP()
{
    if(mac_win_eventUPP)
        return mac_win_eventUPP;
    qAddPostRoutine(cleanup_win_eventUPP);
    return mac_win_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_window_event);
}
OSStatus QWidgetPrivate::qt_window_event(EventHandlerCallRef er, EventRef event, void *)
{
    QScopedLoopLevelCounter loopLevelCounter(qApp->d_func()->threadData);
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassWindow: {
        WindowRef wid = 0;
        GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                          sizeof(WindowRef), 0, &wid);
        QWidget *widget = qt_mac_find_window(wid);
        if(!widget) {
            handled_event = false;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
        } else if(ekind == kEventWindowGetClickModality) {
            handled_event = false;
            if(QWidget *blocker = qt_mac_modal_blocked(widget)) {
                if(!qt_mac_is_macsheet(blocker) || blocker->parentWidget() != widget) {
                    handled_event = true;
                    WindowPtr blockerWindowRef = qt_mac_window_for(blocker);
                    SetEventParameter(event, kEventParamModalWindow, typeWindowRef, sizeof(blockerWindowRef), &blockerWindowRef);
                    HIModalClickResult clickResult = kHIModalClickIsModal;
                    SetEventParameter(event, kEventParamModalClickResult, typeModalClickResult, sizeof(clickResult), &clickResult);
                }
            }
#endif
        } else if(ekind == kEventWindowClose) {
            widget->d_func()->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
            QMenuBar::macUpdateMenuBar();
        } else if (ekind == kEventWindowTransitionCompleted) {
            WindowTransitionAction transitionAction;
            GetEventParameter(event, kEventParamWindowTransitionAction, typeWindowTransitionAction,
                              0, sizeof(transitionAction), 0, &transitionAction);
            if (transitionAction == kWindowHideTransitionAction)
                widget->hide();
        } else if(ekind == kEventWindowExpanded) {
            Qt::WindowStates currState = Qt::WindowStates(widget->data->window_state);
            Qt::WindowStates newState = currState;
            if (currState & Qt::WindowMinimized)
                newState &= ~Qt::WindowMinimized;
            if (!(currState & Qt::WindowActive))
                newState |= Qt::WindowActive;
            if (newState != currState) {
                widget->data->window_state = newState;
                QWindowStateChangeEvent e(currState);
                QApplication::sendSpontaneousEvent(widget, &e);
            }

            if (!widget->isVisible()){
                // Don't send a show event if the window is already showing, as an event
                // has been sendt to preserve the X11 order: first 'show', then 'activated'.
                QShowEvent qse;
                QApplication::sendSpontaneousEvent(widget, &qse);
            }
        } else if(ekind == kEventWindowZoomed) {
            WindowPartCode windowPart;
            GetEventParameter(event, kEventParamWindowPartCode,
                              typeWindowPartCode, 0, sizeof(windowPart), 0, &windowPart);
            if(windowPart == inZoomIn && widget->isMaximized()) {

                widget->data->window_state = widget->data->window_state & ~Qt::WindowMaximized;
                QWindowStateChangeEvent e(Qt::WindowStates(widget->data->window_state | Qt::WindowMaximized));
                QApplication::sendSpontaneousEvent(widget, &e);
            } else if(windowPart == inZoomOut && !widget->isMaximized()) {
                widget->data->window_state = widget->data->window_state | Qt::WindowMaximized;
                QWindowStateChangeEvent e(Qt::WindowStates(widget->data->window_state
                                                           & ~Qt::WindowMaximized));
                QApplication::sendSpontaneousEvent(widget, &e);
            }
            extern QPointer<QWidget> qt_button_down; //qapplication_mac.cpp
            qt_button_down = 0;
        } else if(ekind == kEventWindowCollapsed) {
            if (!widget->isMinimized()) {
                widget->data->window_state = widget->data->window_state | Qt::WindowMinimized;
                QWindowStateChangeEvent e(Qt::WindowStates(widget->data->window_state & ~Qt::WindowMinimized));
                QApplication::sendSpontaneousEvent(widget, &e);
            }

            // Deactivate this window:
            if (widget->isActiveWindow() && !(widget->windowType() == Qt::Popup)) {
                QWidget *w = 0;
                if (widget->parentWidget())
                    w = widget->parentWidget()->window();
                if (!w || (!w->isVisible() && !w->isMinimized())) {
                    for (WindowPtr wp = GetFrontWindowOfClass(kDocumentWindowClass, true);
                        wp; wp = GetNextWindowOfClass(wp, kDocumentWindowClass, true)) {
                        if ((w = qt_mac_find_window(wp)))
                            break;
                    }
                }
                if(!(w && w->isVisible() && !w->isMinimized()))
                    qApp->setActiveWindow(0);
            }

            //we send a hide to be like X11/Windows
            QEvent e(QEvent::Hide);
            QApplication::sendSpontaneousEvent(widget, &e);
            extern QPointer<QWidget> qt_button_down; //qapplication_mac.cpp
            qt_button_down = 0;
        } else if(ekind == kEventWindowToolbarSwitchMode) {
            QToolBarChangeEvent ev(!(GetCurrentKeyModifiers() & cmdKey));
            QApplication::sendSpontaneousEvent(widget, &ev);
            HIToolbarRef toolbar;
            if (GetWindowToolbar(wid, &toolbar) == noErr) {
                if (toolbar) {
                    // Let HIToolbar do its thang, but things like the OpenGL context
                    // needs to know about it.
                    CallNextEventHandler(er, event);
                    qt_event_request_window_change(widget);
                    widget->data->fstrut_dirty = true;
                }
            }
        } else if(ekind == kEventWindowGetRegion) {
            WindowRef window;
            GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                              sizeof(window), 0, &window);
            WindowRegionCode wcode;
            GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, 0,
                              sizeof(wcode), 0, &wcode);
            if (wcode != kWindowOpaqueRgn){
                // If the region is kWindowOpaqueRgn, don't call next
                // event handler cause this will make the shadow of
                // masked windows become offset. Unfortunately, we're not sure why.
                CallNextEventHandler(er, event);
            }
			RgnHandle rgn;
            GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, 0,
                              sizeof(rgn), 0, &rgn);

            if(QWidgetPrivate::qt_widget_rgn(qt_mac_find_window(window), wcode, rgn, false))
                SetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, sizeof(rgn), &rgn);
        } else if(ekind == kEventWindowProxyBeginDrag) {
            QIconDragEvent e;
            QApplication::sendSpontaneousEvent(widget, &e);
        } else if(ekind == kEventWindowResizeStarted || ekind == kEventWindowDragStarted) {
            QMacBlockingFunction::addRef();
        } else if(ekind == kEventWindowResizeCompleted) {
            // Create a mouse up event, since such an event is not send by carbon to the
            // application event handler (while a mouse down <b>is</b> on kEventWindowResizeStarted)
            EventRef mouseUpEvent;
            CreateEvent(0, kEventClassMouse, kEventMouseUp, 0, kEventAttributeUserEvent, &mouseUpEvent);
            UInt16 mbutton = kEventMouseButtonPrimary;
            SetEventParameter(mouseUpEvent, kEventParamMouseButton, typeMouseButton, sizeof(mbutton), &mbutton);
            WindowRef window;
            GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0, sizeof(window), 0, &window);
            Rect dragRect;
            GetWindowBounds(window, kWindowGrowRgn, &dragRect);
            Point pos = {dragRect.bottom, dragRect.right};
            SetEventParameter(mouseUpEvent, kEventParamMouseLocation, typeQDPoint, sizeof(pos), &pos);
            SendEventToApplication(mouseUpEvent);
            ReleaseEvent(mouseUpEvent);
            QMacBlockingFunction::subRef();
        } else if(ekind == kEventWindowDragCompleted) {
            QMacBlockingFunction::subRef();
        } else if(ekind == kEventWindowBoundsChanging || ekind == kEventWindowBoundsChanged) {
            // Panther doesn't send Changing for sheets, only changed, so only
            // bother handling Changed event if we are on 10.3 and we are a
            // sheet.
            if (ekind == kEventWindowBoundsChanged
                    && (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4
                        || !(widget->windowFlags() & Qt::Sheet))) {
                handled_event = false;
            } else {
                UInt32 flags = 0;
                GetEventParameter(event, kEventParamAttributes, typeUInt32, 0,
                                      sizeof(flags), 0, &flags);
                Rect nr;
                GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, 0,
                                      sizeof(nr), 0, &nr);

                QRect newRect(nr.left, nr.top, nr.right - nr.left, nr.bottom - nr.top);

                QTLWExtra * const tlwExtra = widget->d_func()->maybeTopData();
                if (tlwExtra && tlwExtra->isSetGeometry == 1) {
                    widget->d_func()->setGeometry_sys_helper(newRect.left(), newRect.top(), newRect.width(), newRect.height(), tlwExtra->isMove);
                } else {
                    //implicitly removes the maximized bit
                    if((widget->data->window_state & Qt::WindowMaximized) &&
                       IsWindowInStandardState(wid, 0, 0)) {
                        widget->data->window_state &= ~Qt::WindowMaximized;
                        QWindowStateChangeEvent e(Qt::WindowStates(widget->data->window_state
                                                    | Qt::WindowMaximized));
                        QApplication::sendSpontaneousEvent(widget, &e);

                    }

                    handled_event = false;
                    const QRect oldRect = widget->data->crect;
                    if((flags & kWindowBoundsChangeOriginChanged)) {
                        if(nr.left != oldRect.x() || nr.top != oldRect.y()) {
                            widget->data->crect.moveTo(nr.left, nr.top);
                            QMoveEvent qme(widget->data->crect.topLeft(), oldRect.topLeft());
                            QApplication::sendSpontaneousEvent(widget, &qme);
                        }
                    }
                    if((flags & kWindowBoundsChangeSizeChanged)) {
                        if (widget->isWindow()) {
                            QSize newSize = QLayout::closestAcceptableSize(widget, newRect.size());
                            int dh = newSize.height() - newRect.height();
                            int dw = newSize.width() - newRect.width();
                            if (dw != 0 || dh != 0) {
                                handled_event = true;  // We want to change the bounds, so we handle the event

                                // set the rect, so we can also do the resize down below (yes, we need to resize).
                                newRect.setBottom(newRect.bottom() + dh);
                                newRect.setRight(newRect.right() + dw);

                                nr.left = newRect.x();
                                nr.top = newRect.y();
                                nr.right = nr.left + newRect.width();
                                nr.bottom = nr.top + newRect.height();
                                SetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, sizeof(Rect), &nr);
                            }
                        }

                        if (oldRect.width() != newRect.width() || oldRect.height() != newRect.height()) {
                            widget->data->crect.setSize(newRect.size());
                            HIRect bounds = CGRectMake(0, 0, newRect.width(), newRect.height());

                            // If the WA_StaticContents attribute is set we can optimize the resize
                            // by only repainting the newly exposed area. We do this by disabling
                            // painting when setting the size of the view. The OS will invalidate
                            // the newly exposed area for us.
                            const bool staticContents = widget->testAttribute(Qt::WA_StaticContents);
                            const HIViewRef view = qt_mac_hiview_for(widget);
                            if (staticContents)
                                HIViewSetDrawingEnabled(view, false);
                            HIViewSetFrame(view, &bounds);
                            if (staticContents)
                                HIViewSetDrawingEnabled(view, true);

                            QResizeEvent qre(newRect.size(), oldRect.size());
                            QApplication::sendSpontaneousEvent(widget, &qre);
                            qt_event_request_window_change(widget);
                        }
                    }
                }
            }
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassMouse: {
#if 0
        return SendEventToApplication(event);
#endif

        bool send_to_app = false;
        {
            WindowPartCode wpc;
            if (GetEventParameter(event, kEventParamWindowPartCode, typeWindowPartCode, 0,
                                  sizeof(wpc), 0, &wpc) == noErr && wpc != inContent)
                send_to_app = true;
        }
        if(!send_to_app) {
            WindowRef window;
            if(GetEventParameter(event, kEventParamWindowRef, typeWindowRef, 0,
                                 sizeof(window), 0, &window) == noErr) {
                HIViewRef hiview;
                if(HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &hiview) == noErr) {
                    if(QWidget *w = QWidget::find((WId)hiview)) {
#if 0
                        send_to_app = !w->isActiveWindow();
#else
                        Q_UNUSED(w);
                        send_to_app = true;
#endif
                    }
                }
            }
        }
        if(send_to_app)
            return SendEventToApplication(event);
        handled_event = false;
        break; }
    default:
        handled_event = false;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}

// widget events
static HIObjectClassRef widget_class = 0;
static EventTypeSpec widget_events[] = {
    { kEventClassHIObject, kEventHIObjectConstruct },
    { kEventClassHIObject, kEventHIObjectDestruct },

    { kEventClassControl, kEventControlDraw },
    { kEventClassControl, kEventControlInitialize },
    { kEventClassControl, kEventControlGetPartRegion },
    { kEventClassControl, kEventControlGetClickActivation },
    { kEventClassControl, kEventControlSetFocusPart },
    { kEventClassControl, kEventControlDragEnter },
    { kEventClassControl, kEventControlDragWithin },
    { kEventClassControl, kEventControlDragLeave },
    { kEventClassControl, kEventControlDragReceive },
    { kEventClassControl, kEventControlOwningWindowChanged },
    { kEventClassControl, kEventControlBoundsChanged },
    { kEventClassControl, kEventControlGetSizeConstraints },
    { kEventClassControl, kEventControlVisibilityChanged },

    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseMoved },
    { kEventClassMouse, kEventMouseDragged }
};
static EventHandlerUPP mac_widget_eventUPP = 0;
static void cleanup_widget_eventUPP()
{
    DisposeEventHandlerUPP(mac_widget_eventUPP);
    mac_widget_eventUPP = 0;
}
static const EventHandlerUPP make_widget_eventUPP()
{
    if(mac_widget_eventUPP)
        return mac_widget_eventUPP;
    qAddPostRoutine(cleanup_widget_eventUPP);
    return mac_widget_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_widget_event);
}
OSStatus QWidgetPrivate::qt_widget_event(EventHandlerCallRef er, EventRef event, void *)
{
    QScopedLoopLevelCounter loopLevelCounter(QApplicationPrivate::instance()->threadData);

    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassHIObject: {
        HIViewRef view = 0;
        GetEventParameter(event, kEventParamHIObjectInstance, typeHIObjectRef,
                          0, sizeof(view), 0, &view);
        if(ekind == kEventHIObjectConstruct) {
            if(view) {
                HIViewChangeFeatures(view, kHIViewAllowsSubviews, 0);
                SetEventParameter(event, kEventParamHIObjectInstance,
                                  typeVoidPtr, sizeof(view), &view);
            }
        } else if(ekind == kEventHIObjectDestruct) {
            //nothing to really do.. or is there?
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassControl: {
        QWidget *widget = 0;
        HIViewRef hiview = 0;
        if(GetEventParameter(event, kEventParamDirectObject, typeControlRef,
                             0, sizeof(hiview), 0, &hiview) == noErr)
            widget = QWidget::find((WId)hiview);
        if (widget && widget->macEvent(er, event))
            return noErr;
        if(ekind == kEventControlDraw) {
            if(widget && qt_isGenuineQWidget(hiview)) {

                // if there is a window change event pending for any gl child wigets,
                // send it immediately. (required for flicker-free resizing)
                extern void qt_mac_send_posted_gl_updates(QWidget *widget);
                qt_mac_send_posted_gl_updates(widget);

                //requested rgn
                widget->d_func()->clp_serial++;
                RgnHandle rgn;
                GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, 0, sizeof(rgn), 0, &rgn);
                QRegion qrgn(qt_mac_convert_mac_region(rgn));

                //get widget region
                RgnHandle widgetRgn = qt_mac_get_rgn();
                GetControlRegion(hiview, kControlStructureMetaPart, widgetRgn);
                widget->d_func()->clp = qt_mac_convert_mac_region(widgetRgn);
                qt_mac_dispose_rgn(widgetRgn);
                if(!widget->isWindow()) {
                    QPoint pt(qt_mac_posInWindow(widget));
                    widget->d_func()->clp.translate(pt.x(), pt.y());
                }

                //update handles
                GrafPtr qd = 0;
                CGContextRef cg = 0;
#ifndef QT_MAC_NO_QUICKDRAW
                {
                    if(GetEventParameter(event, kEventParamGrafPort, typeGrafPtr, 0, sizeof(qd), 0, &qd) != noErr) {
                        GDHandle dev = 0;
                        GetGWorld(&qd, &dev); //just use the global port..
                    }
                }
                bool end_cg_context = false;
                if(GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, 0, sizeof(cg), 0, &cg) != noErr && qd) {
                    end_cg_context = true;
                    QDBeginCGContext(qd, &cg);
                }
#else
                if(GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, 0, sizeof(cg), 0, &cg) != noErr) {
                    Q_ASSERT(false);
                }
#endif
                widget->d_func()->hd = cg;
                widget->d_func()->qd_hd = qd;
                CGContextSaveGState(cg);

#ifdef DEBUG_WIDGET_PAINT
                const bool doDebug = true;
                if(doDebug)  {
                    qDebug("asked to draw %p[%p] [%s::%s] %p[%p] [%d] [%dx%d]", widget, hiview, widget->metaObject()->className(),
                           widget->objectName().local8Bit().data(), widget->parentWidget(),
                           (HIViewRef)(widget->parentWidget() ? qt_mac_hiview_for(widget->parentWidget()) : (HIViewRef)0),
                           HIViewIsCompositingEnabled(hiview), qt_mac_posInWindow(widget).x(), qt_mac_posInWindow(widget).y());
#if 0
                    QVector<QRect> region_rects = qrgn.rects();
                    qDebug("Region! %d", region_rects.count());
                    for(int i = 0; i < region_rects.count(); i++)
                        qDebug("%d %d %d %d", region_rects[i].x(), region_rects[i].y(),
                               region_rects[i].width(), region_rects[i].height());
                    region_rects = widget->d_func()->clp.rects();
                    qDebug("Widget Region! %d", region_rects.count());
                    for(int i = 0; i < region_rects.count(); i++)
                        qDebug("%d %d %d %d", region_rects[i].x(), region_rects[i].y(),
                               region_rects[i].width(), region_rects[i].height());
#endif
                }
#endif
                if (widget->isVisible() && widget->updatesEnabled()) { //process the actual paint event.
                    if(widget->testAttribute(Qt::WA_WState_InPaintEvent))
                        qWarning("QWidget::repaint: Recursive repaint detected");

                    QPoint redirectionOffset(0, 0);
                    QWidget *tl = widget->window();
                    if (tl && (tl->windowFlags() & Qt::FramelessWindowHint)) {
                        if(tl->d_func()->extra && !tl->d_func()->extra->mask.isEmpty())
                            redirectionOffset += tl->d_func()->extra->mask.boundingRect().topLeft();
                    }

                    //setup the context
                    widget->setAttribute(Qt::WA_WState_InPaintEvent);
                    QPaintEngine *engine = widget->paintEngine();
                    if (engine)
                        engine->setSystemClip(qrgn);

                    //handle the erase
                    if (engine && (!widget->testAttribute(Qt::WA_NoSystemBackground)
                        && (widget->isWindow() || widget->autoFillBackground())
                        || widget->testAttribute(Qt::WA_TintedBackground)
                        || widget->testAttribute(Qt::WA_StyledBackground))) {
                        QRect rr = qrgn.boundingRect();
#ifdef DEBUG_WIDGET_PAINT
                        if(doDebug)
                            qDebug(" Handling erase for [%s::%s]", widget->metaObject()->className(),
                                   widget->objectName().local8Bit().data());
#endif
                        if (!redirectionOffset.isNull()) {
                            widget->d_func()->setRedirected(widget, redirectionOffset);
                            rr.setWidth(rr.width()+redirectionOffset.x());
                            rr.setHeight(rr.height()+redirectionOffset.y());
                        }

                        bool was_unclipped = widget->testAttribute(Qt::WA_PaintUnclipped);
                        widget->setAttribute(Qt::WA_PaintUnclipped, false);
                        QPainter p(widget);
                        if(was_unclipped)
                            widget->setAttribute(Qt::WA_PaintUnclipped);
                        p.setClipRegion(qrgn.translated(redirectionOffset));

                        QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(widget->parent());
                        if (scrollArea && scrollArea->viewport() == widget) {
                            QAbstractScrollAreaPrivate *priv = static_cast<QAbstractScrollAreaPrivate *>(static_cast<QWidget *>(scrollArea)->d_ptr);
                            const QPoint offset = priv->contentsOffset();
                            p.translate(-offset);
                            rr.translate(offset);
                        }

                        widget->d_func()->paintBackground(&p, rr, widget->isWindow() ? DrawAsRoot : 0);
                        if (widget->testAttribute(Qt::WA_TintedBackground)) {
                            QColor tint = widget->palette().window().color();
                            tint.setAlphaF(.6);
                            p.fillRect(rr, tint);
                        }
                        p.end();
                        if (!redirectionOffset.isNull())
                            widget->d_func()->restoreRedirected();
                    } else if(0) {
                        QRect qrgnRect = qrgn.boundingRect();
                        CGContextClearRect(cg, CGRectMake(qrgnRect.x(), qrgnRect.y(), qrgnRect.width(), qrgnRect.height()));
                    }

                    if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget))
                        CallNextEventHandler(er, event);

                    //send the paint
                    redirectionOffset += widget->data->wrect.topLeft(); // Map from system to qt coordinates
                    if (!redirectionOffset.isNull())
                        widget->d_func()->setRedirected(widget, redirectionOffset);
                    qrgn.translate(redirectionOffset);
                    QPaintEvent e(qrgn);
                    widget->d_func()->dirtyOnWidget = QRegion();
#ifdef QT3_SUPPORT
                    e.setErased(true);
#endif
                    QApplication::sendSpontaneousEvent(widget, &e);
                    if (!redirectionOffset.isNull())
                        widget->d_func()->restoreRedirected();
#ifdef QT_RASTER_PAINTENGINE
                    if(engine->type() == QPaintEngine::Raster)
                        static_cast<QRasterPaintEngine*>(engine)->flush(widget,
                                                                        qrgn.boundingRect().topLeft());
#endif

                    //cleanup
                    if (engine)
                        engine->setSystemClip(QRegion());

                    widget->setAttribute(Qt::WA_WState_InPaintEvent, false);
                    if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
                        qWarning("QWidget: It is dangerous to leave painters active on a widget outside of the PaintEvent");
                }

                widget->d_func()->clp_serial++;
                widget->d_func()->clp = QRegion();
                widget->d_func()->hd = 0;
                widget->d_func()->qd_hd = 0;
                CGContextRestoreGState(cg);
#ifndef QT_MAC_NO_QUICKDRAW
                if(end_cg_context)
                    QDEndCGContext(qd, &cg);
#endif
            } else if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget)) {
                CallNextEventHandler(er, event);
            }
        } else if(ekind == kEventControlInitialize) {
            if(HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget)) {
                UInt32 features = kControlSupportsDragAndDrop | kControlSupportsClickActivation | kControlSupportsFocus;
                SetEventParameter(event, kEventParamControlFeatures, typeUInt32, sizeof(features), &features);
            } else {
                handled_event = false;
            }
        } else if(ekind == kEventControlSetFocusPart) {
            if(widget) {
                ControlPartCode part;
                GetEventParameter(event, kEventParamControlPart, typeControlPartCode, 0,
                                  sizeof(part), 0, &part);
                if(part == kControlFocusNoPart)
                    widget->clearFocus();
                else
                    widget->setFocus();
            }
            if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget))
                CallNextEventHandler(er, event);
        } else if(ekind == kEventControlGetClickActivation) {
            ClickActivationResult clickT = kActivateAndIgnoreClick;
            SetEventParameter(event, kEventParamClickActivation, typeClickActivationResult,
                              sizeof(clickT), &clickT);
        } else if(ekind == kEventControlGetPartRegion) {
            handled_event = false;
            if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget) && CallNextEventHandler(er, event) == noErr) {
                handled_event = true;
                break;
            }
            if(widget && !widget->isWindow()) {
                ControlPartCode part;
                GetEventParameter(event, kEventParamControlPart, typeControlPartCode, 0,
                                  sizeof(part), 0, &part);
                if(part == kControlClickableMetaPart && widget->testAttribute(Qt::WA_TransparentForMouseEvents)) {
                    RgnHandle rgn;
                    GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, 0,
                                      sizeof(rgn), 0, &rgn);
                    SetEmptyRgn(rgn);
                    handled_event = true;
                } else if(part == kControlStructureMetaPart || part == kControlClickableMetaPart) {
                    RgnHandle rgn;
                    GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, 0,
                                      sizeof(rgn), 0, &rgn);
                    SetRectRgn(rgn, 0, 0, widget->width(), widget->height());
                    if(QWidgetPrivate::qt_widget_rgn(widget, kWindowStructureRgn, rgn, false))
                        handled_event = true;
                } else if(part == kControlOpaqueMetaPart) {
                    if(widget->d_func()->isOpaque()) {
                        RgnHandle rgn;
                        GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, 0,
                                          sizeof(RgnHandle), 0, &rgn);
                        SetRectRgn(rgn, 0, 0, widget->width(), widget->height());
                        QWidgetPrivate::qt_widget_rgn(widget, kWindowStructureRgn, rgn, false);
                        SetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle,
                                sizeof(RgnHandle), &rgn);
                        handled_event = true;
                    }
                }
            }
        } else if(ekind == kEventControlOwningWindowChanged) {
            if(!HIObjectIsOfClass((HIObjectRef)hiview, kObjectQWidget))
                CallNextEventHandler(er, event);
            if(widget && qt_mac_window_for(hiview)) {
                WindowRef foo = 0;
                GetEventParameter(event, kEventParamControlCurrentOwningWindow, typeWindowRef, 0,
                                  sizeof(foo), 0, &foo);
                widget->d_func()->initWindowPtr();
            }
            if (widget)
                qt_event_request_window_change(widget);
        } else if(ekind == kEventControlDragEnter || ekind == kEventControlDragWithin ||
                  ekind == kEventControlDragLeave || ekind == kEventControlDragReceive) {
            // dnd are really handled in qdnd_mac.cpp,
            // just modularize the code a little...
            DragRef drag;
            GetEventParameter(event, kEventParamDragRef, typeDragRef, 0, sizeof(drag), 0, &drag);
            handled_event = false;
            bool drag_allowed = false;

            QWidget *dropWidget = widget;
            if (qobject_cast<QFocusFrame *>(widget)){
                // We might shadow widgets underneath the focus
                // frame, so stay interrested, and let the dnd through
                drag_allowed = true;
                handled_event = true;
                Point where;
                GetDragMouse(drag, &where, 0);
                dropWidget = QApplication::widgetAt(QPoint(where.h, where.v));

                if (dropWidget != QDragManager::self()->currentTarget()) {
                    // We have to 'fake' enter and leave events for the shaddowed widgets:
                    if (ekind == kEventControlDragEnter) {
                        if (QDragManager::self()->currentTarget())
                            QDragManager::self()->currentTarget()->d_func()->qt_mac_dnd_event(kEventControlDragLeave, drag);
                        if (dropWidget) {
                            dropWidget->d_func()->qt_mac_dnd_event(kEventControlDragEnter, drag);
                        }
                        // Set dropWidget to zero, so qt_mac_dnd_event
                        // doesn't get called a second time below:
                        dropWidget = 0;
                    }
                }
            }

            // Send the dnd event to the widget:
            if (dropWidget && dropWidget->d_func()->qt_mac_dnd_event(ekind, drag)) {
                drag_allowed = true;
                handled_event = true;
            }

            if (ekind == kEventControlDragEnter) {
                // If we don't accept the enter event, we will
                // receive no more drag events for this widget
                const Boolean wouldAccept = drag_allowed ? true : false;
                SetEventParameter(event, kEventParamControlWouldAcceptDrop, typeBoolean,
                        sizeof(wouldAccept), &wouldAccept);
            }
        } else if (ekind == kEventControlBoundsChanged) {
            if (!widget || widget->isWindow() || widget->testAttribute(Qt::WA_Moved) || widget->testAttribute(Qt::WA_Resized)) {
                handled_event = false;
            } else {
                // Sync our view in case some other (non-Qt) view is controlling us.
                handled_event = true;
                Rect newBounds;
                GetEventParameter(event, kEventParamCurrentBounds,
                                  typeQDRectangle, 0, sizeof(Rect), 0, &newBounds);
                QRect rect(newBounds.left, newBounds.top,
                            newBounds.right - newBounds.left, newBounds.bottom - newBounds.top);

                bool moved = widget->testAttribute(Qt::WA_Moved);
                bool resized = widget->testAttribute(Qt::WA_Resized);
                widget->setGeometry(rect);
                widget->setAttribute(Qt::WA_Moved, moved);
                widget->setAttribute(Qt::WA_Resized, resized);
                qt_event_request_window_change(widget);
            }
        } else if (ekind == kEventControlGetSizeConstraints) {
            if (!widget || !qt_isGenuineQWidget(widget)) {
                handled_event = false;
            } else {
                handled_event = true;
                QWidgetItem item(widget);
                QSize size = item.minimumSize();
                HISize hisize = { size.width(), size.height() };
                SetEventParameter(event, kEventParamMinimumSize, typeHISize, sizeof(HISize), &hisize);
                size = item.maximumSize();
                hisize.width = size.width() + 2; // ### shouldn't have to add 2 (but it works).
                hisize.height = size.height();
                SetEventParameter(event, kEventParamMaximumSize, typeHISize, sizeof(HISize), &hisize);
            }
        } else if (ekind == kEventControlVisibilityChanged) {
            handled_event = false;
            if (widget) {
                qt_event_request_window_change(widget);
                if (!HIViewIsVisible(HIViewRef(widget->winId()))) {
                    extern QPointer<QWidget> qt_button_down; //qapplication_mac.cpp
                    if (widget == qt_button_down)
                        qt_button_down = 0;
                }
            }
        }
        break; }
    case kEventClassMouse: {
        bool send_to_app = false;
        extern QPointer<QWidget> qt_button_down; //qapplication_mac.cpp
        if(qt_button_down)
            send_to_app = true;
        if(send_to_app) {
            OSStatus err = SendEventToApplication(event);
            if(err != noErr)
                handled_event = false;
        } else {
            CallNextEventHandler(er, event);
        }
        break; }
    default:
        handled_event = false;
        break;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}
static HIViewRef qt_mac_create_widget(HIViewRef parent)
{
    if(!widget_class) {
        OSStatus err = HIObjectRegisterSubclass(kObjectQWidget, kHIViewClassID, 0, make_widget_eventUPP(),
                                                GetEventTypeCount(widget_events), widget_events,
                                                0, &widget_class);
        if (err && err != hiObjectClassExistsErr)
            qWarning("QWidget: Internal error (%d)", __LINE__);
    }
    HIViewRef ret = 0;
    if(HIObjectCreate(kObjectQWidget, 0, (HIObjectRef*)&ret) != noErr)
        qWarning("QWidget: Internal error (%d)", __LINE__);
    if(ret && parent)
        HIViewAddSubview(parent, ret);
    //HIViewSetVisible(ret, false);
    return ret;
}

void qt_mac_unregister_widget()
{
    HIObjectUnregisterClass(widget_class);
    widget_class = 0;
}

void QWidgetPrivate::toggleDrawers(bool visible)
{
    for (int i = 0; i < children.size(); ++i) {
        register QObject *object = children.at(i);
        if (!object->isWidgetType())
            continue;
        QWidget *widget = static_cast<QWidget*>(object);
        if(qt_mac_is_macdrawer(widget)) {
            if(visible) {
                if (!widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
                    widget->show();
            } else {
                widget->hide();
                widget->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
            }
        }
    }
}

/*****************************************************************************
  QWidgetPrivate member functions
 *****************************************************************************/
bool QWidgetPrivate::qt_mac_update_sizer(QWidget *w, int up=0)
{
    if(!w || !w->isWindow())
        return false;

    QTLWExtra *topData = w->d_func()->topData();
    QWExtra *extraData = w->d_func()->extraData();
    topData->resizer += up;
    {
        WindowClass wclass;
        GetWindowClass(qt_mac_window_for(w), &wclass);
        if(!(GetAvailableWindowAttributes(wclass) & kWindowResizableAttribute))
            return true;
    }
    bool remove_grip = (topData->resizer ||
                        (extraData->maxw && extraData->maxh &&
                         extraData->maxw == extraData->minw && extraData->maxh == extraData->minh));

    WindowAttributes attr;
    GetWindowAttributes(qt_mac_window_for(w), &attr);
    if(remove_grip) {
        if(attr & kWindowResizableAttribute) {
            ChangeWindowAttributes(qt_mac_window_for(w), kWindowNoAttributes,
                                   kWindowResizableAttribute);
            ReshapeCustomWindow(qt_mac_window_for(w));
        }
    } else if(!(attr & kWindowResizableAttribute)) {
        ChangeWindowAttributes(qt_mac_window_for(w), kWindowResizableAttribute,
                               kWindowNoAttributes);
        ReshapeCustomWindow(qt_mac_window_for(w));
    }
    return true;
}

static WindowPtr qt_root_win = 0;
void QWidgetPrivate::qt_clean_root_win()
{
    if(!qt_root_win)
        return;
    ReleaseWindow(qt_root_win);
    qt_root_win = 0;
}

bool QWidgetPrivate::qt_create_root_win() {
    if(qt_root_win)
        return false;
    Rect r;
    const QSize desktopSize = qt_mac_desktopSize();
    SetRect(&r, 0, 0, desktopSize.width(), desktopSize.height());
    WindowAttributes wattr = (kWindowCompositingAttribute | kWindowStandardHandlerAttribute);
    qt_mac_create_window(kOverlayWindowClass, wattr, &r, &qt_root_win);
    if(!qt_root_win)
        return false;
    qAddPostRoutine(qt_clean_root_win);
    return true;
}

bool QWidgetPrivate::qt_recreate_root_win() {
    if(!qt_root_win) //sanity check
        return false;
    //store old
    WindowPtr old_root_win = qt_root_win;
    HIViewRef old_root_hiview = HIViewGetRoot(qt_root_win);
    //recreate
    qt_root_win = 0;
    qt_create_root_win();
    //cleanup old window
    old_root_hiview = 0;
    ReleaseWindow(old_root_win);
    return true;
}

bool QWidgetPrivate::qt_widget_rgn(QWidget *widget, short wcode, RgnHandle rgn, bool force = false)
{
    bool ret = false;
    switch(wcode) {
    case kWindowStructureRgn: {
        if(widget) {
            if(widget->d_func()->extra && !widget->d_func()->extra->mask.isEmpty()) {
                QRegion rin = qt_mac_convert_mac_region(rgn);
                if(!rin.isEmpty()) {
                    QPoint rin_tl = rin.boundingRect().topLeft(); //in offset
                    rin.translate(-rin_tl.x(), -rin_tl.y()); //bring into same space as below
                    QRegion mask = widget->d_func()->extra->mask;
                    if(widget->isWindow() && !(widget->windowFlags() & Qt::FramelessWindowHint)) {
                        QRegion title;
                        {
                            RgnHandle rgn = qt_mac_get_rgn();
                            GetWindowRegion(qt_mac_window_for(widget), kWindowTitleBarRgn, rgn);
                            title = qt_mac_convert_mac_region(rgn);
                            qt_mac_dispose_rgn(rgn);
                        }
                        QRect br = title.boundingRect();
                        mask.translate(0, br.height()); //put the mask 'under' the title bar..
                        title.translate(-br.x(), -br.y());
                        mask += title;
                    }

                    QRegion cr = rin & mask;
                    cr.translate(rin_tl.x(), rin_tl.y()); //translate back to incoming space
                    CopyRgn(cr.handle(true), rgn);
                }
                ret = true;
            } else if(force) {
                QRegion cr(widget->geometry());
                CopyRgn(cr.handle(true), rgn);
                ret = true;
            }
        }
        break; }
    default: break;
    }
    //qDebug() << widget << ret << wcode << qt_mac_convert_mac_region(rgn);
    return ret;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidgetPrivate::determineWindowClass()
{
    Q_Q(QWidget);

    const Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    const bool popup = (type == Qt::Popup);
    if (type == Qt::ToolTip || type == Qt::SplashScreen || popup)
        flags |= Qt::FramelessWindowHint;

    WindowClass wclass = kSheetWindowClass;
    if(qt_mac_is_macdrawer(q))
        wclass = kDrawerWindowClass;
    else if (q->testAttribute(Qt::WA_ShowModal) && qt_mac_menu_buttons_explicitly_set(flags))
        wclass = kDocumentWindowClass;
    else if(popup || (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5 && type == Qt::SplashScreen))
        wclass = kModalWindowClass;
    else if(q->testAttribute(Qt::WA_ShowModal))
        wclass = kMovableModalWindowClass;
    else if(type == Qt::ToolTip)
        wclass = kHelpWindowClass;
    else if(type == Qt::Tool || (QSysInfo::MacintoshVersion < QSysInfo::MV_10_5
                                 && type == Qt::SplashScreen))
        wclass = kFloatingWindowClass;
    else
        wclass = kDocumentWindowClass;

    WindowGroupRef grp = 0;
    WindowAttributes wattr = (kWindowCompositingAttribute | kWindowStandardHandlerAttribute);
    if(qt_mac_is_macsheet(q)) {
        //grp = GetWindowGroupOfClass(kMovableModalWindowClass);
        wclass = kSheetWindowClass;
    } else {
        grp = GetWindowGroupOfClass(wclass);
        // Shift things around a bit to get the correct window class based on the presence
        // (or lack) of the border.
        if(flags & Qt::FramelessWindowHint) {
            if(wclass == kDocumentWindowClass) {
                wclass = kSimpleWindowClass;
            } else if(wclass == kFloatingWindowClass) {
                wclass = kToolbarWindowClass;
            } else if (wclass  == kMovableModalWindowClass) {
                wclass  = kModalWindowClass;
            }
        } else {
            if(wclass != kModalWindowClass)
                wattr |= kWindowResizableAttribute;
        }
        // Only add extra decorations (well, buttons) for widgets that can have them
        // and have an actual border we can put them on.
        if(wclass != kModalWindowClass && wclass != kMovableModalWindowClass
                && wclass != kSheetWindowClass && wclass != kPlainWindowClass
                && !(flags & Qt::FramelessWindowHint) && wclass != kDrawerWindowClass
                && wclass != kHelpWindowClass) {
            if (flags & Qt::WindowMaximizeButtonHint)
                wattr |= kWindowFullZoomAttribute;
            if (flags & Qt::WindowMinimizeButtonHint)
                wattr |= kWindowCollapseBoxAttribute;
            if (flags & Qt::WindowSystemMenuHint)
                wattr |= kWindowCloseBoxAttribute;
        } else {
            // Clear these hints so that we aren't call them on invalid windows
            flags &= ~(Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint
                       | Qt::WindowSystemMenuHint);
        }
    }
    if((popup || type == Qt::Tool) && !q->isModal())
        wattr |= kWindowHideOnSuspendAttribute;
    wattr |= kWindowLiveResizeAttribute;

#ifdef DEBUG_WINDOW_CREATE
#define ADD_DEBUG_WINDOW_NAME(x) { x, #x }
    struct {
        UInt32 tag;
        const char *name;
    } known_attribs[] = {
        ADD_DEBUG_WINDOW_NAME(kWindowCompositingAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowMetalAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowCollapseBoxAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowHorizontalZoomAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowVerticalZoomAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowResizableAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowNoActivatesAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowNoUpdatesAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowOpaqueForEventsAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowLiveResizeAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowCloseBoxAttribute),
        ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
        { 0, 0 }
    }, known_classes[] = {
        ADD_DEBUG_WINDOW_NAME(kHelpWindowClass),
        ADD_DEBUG_WINDOW_NAME(kPlainWindowClass),
        ADD_DEBUG_WINDOW_NAME(kDrawerWindowClass),
        ADD_DEBUG_WINDOW_NAME(kUtilityWindowClass),
        ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
        ADD_DEBUG_WINDOW_NAME(kSheetWindowClass),
        ADD_DEBUG_WINDOW_NAME(kFloatingWindowClass),
        ADD_DEBUG_WINDOW_NAME(kUtilityWindowClass),
        ADD_DEBUG_WINDOW_NAME(kDocumentWindowClass),
        ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
        ADD_DEBUG_WINDOW_NAME(kMovableModalWindowClass),
        ADD_DEBUG_WINDOW_NAME(kModalWindowClass),
        { 0, 0 }
    };
    qDebug("Qt: internal: ************* Creating new window %p (%s::%s)", q, q->metaObject()->className(),
            q->objectName().toLocal8Bit().constData());
    bool found_class = false;
    for(int i = 0; known_classes[i].name; i++) {
        if(wclass == known_classes[i].tag) {
            found_class = true;
            qDebug("Qt: internal: ** Class: %s", known_classes[i].name);
            break;
        }
    }
    if(!found_class)
        qDebug("Qt: internal: !! Class: Unknown! (%d)", (int)wclass);
    if(wattr) {
        WindowAttributes tmp_wattr = wattr;
        qDebug("Qt: internal: ** Attributes:");
        for(int i = 0; tmp_wattr && known_attribs[i].name; i++) {
            if((tmp_wattr & known_attribs[i].tag) == known_attribs[i].tag) {
                tmp_wattr ^= known_attribs[i].tag;
                qDebug("Qt: internal: * %s %s", known_attribs[i].name,
                        (GetAvailableWindowAttributes(wclass) & known_attribs[i].tag) ? "" : "(*)");
            }
        }
        if(tmp_wattr)
            qDebug("Qt: internal: !! Attributes: Unknown (%d)", (int)tmp_wattr);
    }
#endif

    /* Just to be extra careful we will change to the kUtilityWindowClass if the
       requested attributes cannot be used */
    if((GetAvailableWindowAttributes(wclass) & wattr) != wattr) {
        WindowClass tmp_class = wclass;
        if(wclass == kToolbarWindowClass || wclass == kUtilityWindowClass)
            wclass = kFloatingWindowClass;
        if(tmp_class != wclass) {
            if(!grp)
                grp = GetWindowGroupOfClass(wclass);
            wclass = tmp_class;
        }
    }
    topData()->wclass = wclass;
    topData()->wattr = wattr;
}

void QWidgetPrivate::initWindowPtr()
{
    Q_Q(QWidget);
    WindowPtr windowRef = qt_mac_window_for(qt_mac_hiview_for(q)); //do not create!
    if(!windowRef)
        return;
    QWidget *window = q->window(), *oldWindow = 0;
    if(GetWindowProperty(windowRef, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(oldWindow), 0, &oldWindow) == noErr) {
        Q_ASSERT(window == oldWindow);
        return;
    }

    if(SetWindowProperty(windowRef, kWidgetCreatorQt, kWidgetPropertyQWidget, sizeof(window), &window) != noErr)
        qWarning("Qt:Internal error (%s:%d)", __FILE__, __LINE__); //no real way to recover
    if(!q->windowType() != Qt::Desktop) { //setup an event callback handler on the window
        InstallWindowEventHandler(windowRef, make_win_eventUPP(), GetEventTypeCount(window_events),
                window_events, static_cast<void *>(qApp), &window_event);
    }
}

void QWidgetPrivate::createWindow_sys()
{
    Q_Q(QWidget);

    const Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    const bool desktop = (type == Qt::Desktop);
    const bool dialog = (type == Qt::Dialog
                         || type == Qt::Sheet
                         || type == Qt::Drawer
                         || (flags & Qt::MSWindowsFixedSizeDialogHint));
    QTLWExtra *topExtra = topData();
    quint32 wattr = topExtra->wattr;

    if(parentWidget && (parentWidget->window()->windowFlags() & Qt::WindowStaysOnTopHint)) // If our parent has Qt::WStyle_StaysOnTop, so must we
        flags |= Qt::WindowStaysOnTopHint;
    if (0 && q->testAttribute(Qt::WA_ShowModal)  // ### Look at this, again!
            && !(flags & Qt::CustomizeWindowHint)
        && !(desktop || type == Qt::Popup)) {
        flags &= ~(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    }

    Rect r;
    SetRect(&r, data.crect.left(), data.crect.top(), data.crect.right() + 1, data.crect.bottom() + 1);
    data.fstrut_dirty = true;
    WindowRef windowRef = 0;
    if (OSStatus ret = qt_mac_create_window(topExtra->wclass, wattr, &r, &windowRef))
        qWarning("QWidget: Internal error: %s:%d: If you reach this error please contact Trolltech and include the\n"
                "      WidgetFlags used in creating the widget (%ld)", __FILE__, __LINE__, long(ret));
    if (!desktop)
        SetAutomaticControlDragTrackingEnabledForWindow(windowRef, true);
    HIWindowChangeFeatures(windowRef, kWindowCanCollapse, 0);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        if (wattr & kWindowHideOnSuspendAttribute)
            HIWindowChangeAvailability(windowRef, kHIWindowExposeHidden, 0);
        else
            HIWindowChangeAvailability(windowRef, 0, kHIWindowExposeHidden);
    }
#endif
    WindowGroupRef grp = 0;
    if ((flags & Qt::WindowStaysOnTopHint))
        ChangeWindowAttributes(windowRef, kWindowNoAttributes, kWindowHideOnSuspendAttribute);
    if (qt_mac_is_macdrawer(q) && parentWidget)
        SetDrawerParent(windowRef, qt_mac_window_for (parentWidget));
    if (dialog && !parentWidget && !q->testAttribute(Qt::WA_ShowModal))
        grp = GetWindowGroupOfClass(kDocumentWindowClass);
    if (topExtra->group) {
        qt_mac_release_window_group(topExtra->group);
        topExtra->group = 0;
    }
    if (type == Qt::ToolTip)
        qt_mac_set_window_group_to_tooltip(windowRef);
    else if (type == Qt::Popup && (flags & Qt::WindowStaysOnTopHint))
        qt_mac_set_window_group_to_popup(windowRef);
    else if (flags & Qt::WindowStaysOnTopHint)
        qt_mac_set_window_group_to_stays_on_top(windowRef, type);
    else if (grp)
        SetWindowGroup(windowRef, grp);

#ifdef DEBUG_WINDOW_CREATE
    if (WindowGroupRef grpf = GetWindowGroup(windowRef)) {
        QCFString cfname;
        CopyWindowGroupName(grpf, &cfname);
        SInt32 lvl;
        GetWindowGroupLevel(grpf, &lvl);
        const char *from = "Default";
        if (topExtra && grpf == topData()->group)
            from = "Created";
        else if (grpf == grp)
            from = "Copied";
        qDebug("Qt: internal: With window group '%s' [%p] @ %d: %s",
                static_cast<QString>(cfname).toLatin1().constData(), grpf, (int)lvl, from);
    } else {
        qDebug("Qt: internal: No window group!!!");
    }
    HIWindowAvailability hi_avail = 0;
    if (HIWindowGetAvailability(windowRef, &hi_avail) == noErr) {
        struct {
            UInt32 tag;
            const char *name;
        } known_avail[] = {
            ADD_DEBUG_WINDOW_NAME(kHIWindowExposeHidden),
            { 0, 0 }
        };
        qDebug("Qt: internal: ** HIWindowAvailibility:");
        for (int i = 0; hi_avail && known_avail[i].name; i++) {
            if ((hi_avail & known_avail[i].tag) == known_avail[i].tag) {
                hi_avail ^= known_avail[i].tag;
                qDebug("Qt: internal: * %s", known_avail[i].name);
            }
        }
        if (hi_avail)
            qDebug("Qt: internal: !! Attributes: Unknown (%d)", (int)hi_avail);
    }
#undef ADD_DEBUG_WINDOW_NAME
#endif
    if (extra && !extra->mask.isEmpty())
        ReshapeCustomWindow(windowRef);
    SetWindowModality(windowRef, kWindowModalityNone, 0);
    if (qt_mac_is_macdrawer(q))
        SetDrawerOffsets(windowRef, 0.0, 25.0);
    data.fstrut_dirty = true; // when we create a toplevel widget, the frame strut should be dirty
    HIViewRef hiview = (HIViewRef)data.winid;
    HIViewRef window_hiview = qt_mac_hiview_for(windowRef);
    if(!hiview) {
        hiview = qt_mac_create_widget(window_hiview);
        setWinId((WId)hiview);
    } else {
        HIViewAddSubview(window_hiview, hiview);
    }
    if (hiview) {
        Rect win_rect;
        GetWindowBounds(qt_mac_window_for (window_hiview), kWindowContentRgn, &win_rect);
        HIRect bounds = CGRectMake(0, 0, win_rect.right-win_rect.left, win_rect.bottom-win_rect.top);
        HIViewSetFrame(hiview, &bounds);
        HIViewSetVisible(hiview, true);
        if (q->testAttribute(Qt::WA_DropSiteRegistered))
            registerDropSite(true);
        transferChildren();
    }
    initWindowPtr();

    if (topExtra->posFromMove) {
        updateFrameStrut();
        const QRect &fStrut = frameStrut();
        SetRect(&r, r.left + fStrut.left(), r.top + fStrut.top(),
                    (r.left + fStrut.left() + data.crect.width()) - fStrut.right(),
                    (r.top + fStrut.top() + data.crect.height()) - fStrut.bottom());
        SetWindowBounds(windowRef, kWindowContentRgn, &r);
        topExtra->posFromMove = false;
    }

    if (qt_mac_is_macsheet(q)){
        SetThemeWindowBackground(qt_mac_window_for(q), kThemeBrushSheetBackgroundTransparent, true);
        CGFloat alpha = 0;
        GetWindowAlpha(qt_mac_window_for(q), &alpha);
        if (alpha == 1){
            // For some reason the 'SetThemeWindowBackground' does not seem
            // to work. So we do this little hack until it hopefully starts to
            // work in newer versions of mac OS.
            q->setWindowOpacity(0.95f);
        }
    }
    else if (topExtra->opacity != 255)
        q->setWindowOpacity(topExtra->opacity / 255.0f);

    // Since we only now have a window, sync our state.
    macUpdateHideOnSuspend();
    macUpdateOpaqueSizeGrip();
    macUpdateMetalAttribute();
    macUpdateIgnoreMouseEvents();
    setWindowTitle_helper(extra->topextra->caption);
    setWindowIconText_helper(extra->topextra->iconText);
    setWindowFilePath_helper(extra->topextra->filePath);
    setWindowModified_sys(q->isWindowModified());
    updateFrameStrut();
}

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_Q(QWidget);
    window_event = 0;
    HIViewRef destroyid = 0;

    Qt::WindowType type = q->windowType();
    Qt::WindowFlags flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    bool topLevel = (flags & Qt::Window);
    bool popup = (type == Qt::Popup);
    bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || type == Qt::Drawer
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
    bool desktop = (type == Qt::Desktop);

    if (desktop) {
        QSize desktopSize = qt_mac_desktopSize();
        q->setAttribute(Qt::WA_WState_Visible);
        data.crect.setRect(0, 0, desktopSize.width(), desktopSize.height());
        dialog = popup = false;                  // force these flags off
    } else {
        q->setAttribute(Qt::WA_WState_Visible, false);

        if (topLevel && (type != Qt::Drawer)) {
            if(QDesktopWidget *dsk = QApplication::desktop()) { // calc pos/size from screen
                const bool wasResized = q->testAttribute(Qt::WA_Resized);
                const bool wasMoved = q->testAttribute(Qt::WA_Moved);

                int deskn = dsk->primaryScreen();
                if(parentWidget && parentWidget->windowType() != Qt::Desktop)
                    deskn = dsk->screenNumber(parentWidget);
                QRect dskr = dsk->screenGeometry(deskn);
                if (!wasResized)
                    data.crect.setSize(QSize(dskr.width()/2, 4*dskr.height()/10));
                if (!wasMoved)
                    data.crect.moveTopLeft(QPoint(dskr.width()/4, 3*dskr.height()/10));
            }
        }
    }


    if(!window)                              // always initialize
        initializeWindow=true;

    hd = 0;
    if(window) {                                // override the old window (with a new HIViewRef)
        HIViewRef hiview = (HIViewRef)window, parent = 0;
        CFRetain(hiview);
        if(destroyOldWindow)
            destroyid = qt_mac_hiview_for(q);
        bool transfer = false;
        setWinId((WId)hiview);
#ifndef HIViewInstallEventHandler
        // Macro taken from the CarbonEvents Header on Tiger
#define HIViewInstallEventHandler( target, handler, numTypes, list, userData, outHandlerRef ) \
               InstallEventHandler( HIObjectGetEventTarget( (HIObjectRef) (target) ), (handler), (numTypes), (list), (userData), (outHandlerRef) )
#endif
        HIViewInstallEventHandler(hiview, make_widget_eventUPP(), GetEventTypeCount(widget_events), widget_events, 0, 0);
        if(topLevel) {
            determineWindowClass();
            for(int i = 0; i < 2; ++i) {
                if(i == 1) {
                    if(!initializeWindow)
                        break;
                    createWindow_sys();
                }
                if(WindowRef windowref = qt_mac_window_for(hiview)) {
                    RetainWindow(windowref);
                    if (initializeWindow)
                        parent = qt_mac_hiview_for(windowref);
                    else
                        parent = HIViewGetSuperview(hiview);
                    break;
                }
            }
            if(!parent)
                transfer = true;
        } else if (parentWidget) {
            // I need to be added to my parent, therefore my parent needs an HIViewRef
            parentWidget->createWinId();
            parent = qt_mac_hiview_for(parentWidget);
        }
        if(parent != hiview)
            HIViewAddSubview(parent, hiview);
        if(transfer)
            transferChildren();
        data.fstrut_dirty = true; // we'll re calculate this later
        q->setAttribute(Qt::WA_WState_Visible, HIViewIsVisible(hiview));
        if(initializeWindow) {
            HIRect bounds = CGRectMake(data.crect.x(), data.crect.y(), data.crect.width(), data.crect.height());
            HIViewSetFrame(hiview, &bounds);
            q->setAttribute(Qt::WA_WState_Visible, HIViewIsVisible(hiview));
        }
        initWindowPtr();
    } else if(desktop) {                        // desktop widget
        if(!qt_root_win)
            QWidgetPrivate::qt_create_root_win();
        Q_ASSERT(qt_root_win);
        CFRetain(qt_root_win);
        if(HIViewRef hiview = HIViewGetRoot(qt_root_win)) {
            CFRetain(hiview);
            setWinId((WId)hiview);
        }
    } else if(topLevel) {
        determineWindowClass();
        if(HIViewRef hiview = qt_mac_create_widget(0)) {
            HIRect bounds = CGRectMake(data.crect.x(), data.crect.y(),
                                       data.crect.width(), data.crect.height());
            HIViewSetFrame(hiview, &bounds);
            setWinId((WId)hiview);
        }
    } else {
        data.fstrut_dirty = false; // non-toplevel widgets don't have a frame, so no need to update the strut
        if(HIViewRef hiview = qt_mac_create_widget(qt_mac_hiview_for(parentWidget))) {
            HIRect bounds = CGRectMake(data.crect.x(), data.crect.y(), data.crect.width(), data.crect.height());
            HIViewSetFrame(hiview, &bounds);
            setWinId((WId)hiview);
            if (q->testAttribute(Qt::WA_DropSiteRegistered))
                registerDropSite(true);
        }
    }

    updateIsOpaque();
    if (!topLevel && initializeWindow)
        setWSGeometry();

    if(destroyid) {
        HIViewRemoveFromSuperview(destroyid);
        CFRelease(destroyid);
    }
}

/*!
    Returns the QuickDraw handle of the widget. Use of this function is not
    portable. This function will return 0 if QuickDraw is not supported, or
    if the handle could not be created.

    \warning This function is only available on Mac OS X.
*/

Qt::HANDLE
QWidget::macQDHandle() const
{
    Q_D(const QWidget);
    return d->qd_hd;
}

/*!
  Returns the CoreGraphics handle of the widget. Use of this function is
  not portable. This function will return 0 if no painter context can be
  established, or if the handle could not be created.

  \warning This function is only available on Mac OS X.
*/
Qt::HANDLE
QWidget::macCGHandle() const
{
    return handle();
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);
    d->deactivateWidgetCleanup();
    qt_mac_event_release(this);
    if(testAttribute(Qt::WA_WState_Created)) {
        setAttribute(Qt::WA_WState_Created, false);
        QObjectList chldrn = children();
        for(int i = 0; i < chldrn.size(); i++) {  // destroy all widget children
            QObject *obj = chldrn.at(i);
            if(obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows, destroySubWindows);
        }
        if(mac_mouse_grabber == this)
            releaseMouse();
        if(mac_keyboard_grabber == this)
            releaseKeyboard();
        if(acceptDrops())
            setAcceptDrops(false);

        if(testAttribute(Qt::WA_ShowModal))          // just be sure we leave modal
            QApplicationPrivate::leaveModal(this);
        else if((windowType() == Qt::Popup))
            qApp->d_func()->closePopup(this);
        if(destroyWindow) {
            if(d->window_event)
                RemoveEventHandler(d->window_event);
            if(HIViewRef hiview = qt_mac_hiview_for(this)) {
                WindowPtr window = isWindow() ? qt_mac_window_for(hiview) : 0;
                if(window) {
                    RemoveWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget);
                    ReleaseWindow(window);
                } else {
                    HIViewRemoveFromSuperview(hiview);
                    CFRelease(hiview);
                }
            }
        }
        d->setWinId(0);
    }
}

void QWidgetPrivate::transferChildren()
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created))
        return;  // Can't add any views anyway

    QObjectList chlist = q->children();
    for (int i = 0; i < chlist.size(); ++i) {
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if (!w->isWindow()) {
                // This seems weird, no need to call it in a loop right?
                if (!topData()->caption.isEmpty())
                    setWindowTitle_helper(extra->topextra->caption);
                if (w->testAttribute(Qt::WA_WState_Created))
                    HIViewAddSubview(qt_mac_hiview_for(q), qt_mac_hiview_for(w));
            }
        }
    }
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WindowFlags f)
{
    Q_Q(QWidget);
    QTLWExtra *topData = maybeTopData();
    bool wasCreated = q->testAttribute(Qt::WA_WState_Created);


    // Maintain the glWidgets list on parent change: remove "our" gl widgets
    // from the list on the old parent and grandparents.
    if (glWidgets.isEmpty() == false) {
        QWidget *current = q->parentWidget();
        while (current) {
            for (QList<QWidgetPrivate::GlWidgetInfo>::const_iterator it = glWidgets.constBegin();
                 it != glWidgets.constEnd(); ++it)
                current->d_func()->glWidgets.removeAll(*it);

            if (current->isWindow())
                break;
            current = current->parentWidget();
        }
    }

    EventHandlerRef old_window_event = 0;
    HIViewRef old_id = 0;
    if (wasCreated && !(q->windowType() == Qt::Desktop)) {
        old_id = qt_mac_hiview_for(q);
        old_window_event = window_event;
    }
    QWidget* oldtlw = q->window();

    if (q->testAttribute(Qt::WA_DropSiteRegistered))
        q->setAttribute(Qt::WA_DropSiteRegistered, false);

    //recreate and setup flags
    QObjectPrivate::setParent_helper(parent);
    QPoint pt = q->pos();
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);
    if (wasCreated && !qt_isGenuineQWidget(q))
        return;

    if ((data.window_flags & Qt::Sheet) && topData && topData->opacity == 242)
        q->setWindowOpacity(1.0f);

    setWinId(0); //do after the above because they may want the id

    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    adjustFlags(data.window_flags, q);
    // keep compatibility with previous versions, we need to preserve the created state
    // (but we recreate the winId for the widget being reparented, again for compatibility)
    if (wasCreated || (!q->isWindow() && parent->testAttribute(Qt::WA_WState_Created)))
        createWinId();
    if (q->isWindow() || (!parent || parent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    if (wasCreated) {
        transferChildren();
        if (topData &&
                (!topData->caption.isEmpty() || !topData->filePath.isEmpty()))
            setWindowTitle_helper(q->windowTitle());
    }

    if (q->testAttribute(Qt::WA_AcceptDrops)
        || (!q->isWindow() && q->parentWidget()
            && q->parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)))
        q->setAttribute(Qt::WA_DropSiteRegistered, true);

    //cleanup
    if(old_window_event)
        RemoveEventHandler(old_window_event);
    if(old_id) { //don't need old window anymore
        WindowPtr window = (oldtlw == q) ? qt_mac_window_for(old_id) : 0;
        if(window) {
            RemoveWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget);
            ReleaseWindow(window);
        } else {
            HIViewRemoveFromSuperview(old_id);
            CFRelease(old_id);
        }
    }

    // Maintain the glWidgets list on parent change: add "our" gl widgets
    // to the list on the new parent and grandparents.
    if (glWidgets.isEmpty() == false) {
        QWidget *current = q->parentWidget();
        while (current) {
            current->d_func()->glWidgets += glWidgets;
            if (current->isWindow())
                break;
            current = current->parentWidget();
        }
    }

    qt_event_request_window_change(q);
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!testAttribute(Qt::WA_WState_Created)) {
        QPoint p = pos + data->crect.topLeft();
        return isWindow() ?  p : parentWidget()->mapToGlobal(p);
    }
    QPoint tmp = d->mapToWS(pos);
    HIPoint hi_pos = CGPointMake(tmp.x(), tmp.y());
    HIViewConvertPoint(&hi_pos, qt_mac_hiview_for(this), 0);
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for(this), kWindowStructureRgn, &win_rect);
    return QPoint((int)hi_pos.x+win_rect.left, (int)hi_pos.y+win_rect.top);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!testAttribute(Qt::WA_WState_Created)) {
        QPoint p = isWindow() ?  pos : parentWidget()->mapFromGlobal(pos);
        return p - data->crect.topLeft();
    }
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for(this), kWindowStructureRgn, &win_rect);
    HIPoint hi_pos = CGPointMake(pos.x()-win_rect.left, pos.y()-win_rect.top);
    HIViewConvertPoint(&hi_pos, 0, qt_mac_hiview_for(this));
    return d->mapFromWS(QPoint((int)hi_pos.x, (int)hi_pos.y));
}

void QWidgetPrivate::updateSystemBackground()
{
}

void QWidgetPrivate::setCursor_sys(const QCursor &)
{
    qt_mac_update_cursor();
}

void QWidgetPrivate::unsetCursor_sys()
{
    qt_mac_update_cursor();
}

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    if(q->isWindow())
        SetWindowTitleWithCFString(qt_mac_window_for(q), QCFString(caption));
}

void QWidgetPrivate::setWindowModified_sys(bool mod)
{
    Q_Q(QWidget);
    if (q->isWindow() && q->testAttribute(Qt::WA_WState_Created))
        SetWindowModified(qt_mac_window_for(q), mod);
}

void QWidgetPrivate::setWindowFilePath_sys(const QString &filePath)
{
    Q_Q(QWidget);
    bool validRef = false;
    FSRef ref;
    bzero(&ref, sizeof(ref));
    OSStatus status;

    if (!filePath.isEmpty()) {
        status = FSPathMakeRef(reinterpret_cast<const UInt8 *>(filePath.toUtf8().constData()), &ref, 0);
        validRef = (status == noErr);
    }
    // Set the proxy regardless, since this is our way of clearing it as well, but ignore the
    // return value as well.
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        status = HIWindowSetProxyFSRef(qt_mac_window_for(q), &ref);
        if (validRef && (status != noErr))
            qWarning("QWidget::setWindowFilePath: Error setting proxyicon for path (%s):%ld",
                     qPrintable(filePath), status);
    } else
#endif
    {
#ifndef Q_WS_MAC64
        // Convert to an FSSpec and set it. It's deprecated but it works for where we don't have the other call.
        FSSpec fsspec;
        FSGetCatalogInfo(&ref, kFSCatInfoNone, 0, 0, &fsspec, 0);
        status = SetWindowProxyFSSpec(qt_mac_window_for(q), &fsspec);
        if (validRef && (status != noErr)) {
            qWarning("QWidget::setWindowFilePath: Error setting FSSpec proxyicon for path (%s):%ld",
                     qPrintable(filePath), status);
        }
#endif
    }
}

void QWidgetPrivate::setWindowIcon_sys(bool forceReset)
{
    Q_Q(QWidget);

    if (!q->testAttribute(Qt::WA_WState_Created))
        return;

    QTLWExtra *topData = this->topData();
    if (topData->iconPixmap && !forceReset) // already set
        return;

    QIcon icon = q->windowIcon();
    QPixmap *pm = 0;
    if (!icon.isNull()) {
        // now create the extra
        if (!topData->iconPixmap) {
            pm = new QPixmap(icon.pixmap(QSize(22, 22)));
            topData->iconPixmap = pm;
        } else {
            pm = topData->iconPixmap;
        }
    }
    if (q->isWindow()) {
        IconRef previousIcon = 0;
        if (icon.isNull()) {
            RemoveWindowProxy(qt_mac_window_for(q));
            previousIcon = topData->windowIcon;
            topData->windowIcon = 0;
        } else {
            WindowClass wclass;
            GetWindowClass(qt_mac_window_for(q), &wclass);

            if (wclass == kDocumentWindowClass) {
                IconRef newIcon = qt_mac_create_iconref(*pm);
                previousIcon = topData->windowIcon;
                topData->windowIcon = newIcon;
                SetWindowProxyIcon(qt_mac_window_for(q), newIcon);
            }
        }

        // Release the previous icon if it was set by this function.
        if (previousIcon != 0)
            ReleaseIconRef(previousIcon);
    }
}

void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{
    Q_Q(QWidget);
    if(q->isWindow() && !iconText.isEmpty())
        SetWindowAlternateTitle(qt_mac_window_for(q), QCFString(iconText));
}

void QWidgetPrivate::dirtyWidget_sys(const QRegion &rgn, bool updateImmediately)
{
    if (rgn.isEmpty())
        return;

    dirtyOnWidget += rgn;

    QRegion dirty = rgn;
    dirty &= clipRect();
    if (extra && !extra->mask.isEmpty())
        dirty &= extra->mask;

    if (dirty.isEmpty())
        return;

    Q_Q(QWidget);
    QWidget *widget = q;
    if (!q->isWindow()) {
        widget = q->window();
        dirty.translate(q->mapTo(widget, QPoint()));
    }

    widget->d_func()->dirtyOnScreen += dirty;
    if (updateImmediately) {
        QEvent event(QEvent::UpdateRequest);
        QApplication::sendEvent(widget, &event);
    } else {
        QApplication::postEvent(widget, new QEvent(QEvent::UpdateRequest), Qt::LowEventPriority);
    }
}

void QWidgetPrivate::cleanWidget_sys(const QRegion &rgn)
{
    if (rgn.isEmpty())
        return;

    Q_Q(QWidget);
    QRegion clean = rgn;

    QWidget *widget = q;
    if (!q->isWindow()) {
        widget = q->window();
        clean.translate(q->mapTo(widget, QPoint()));
    }

    widget->d_func()->dirtyOnScreen -= clean;
}

void QWidget::grabMouse()
{
    if(isVisible() && !qt_nograb()) {
        if(mac_mouse_grabber)
            mac_mouse_grabber->releaseMouse();
        mac_mouse_grabber=this;
    }
}

void QWidget::grabMouse(const QCursor &)
{
    if(isVisible() && !qt_nograb()) {
        if(mac_mouse_grabber)
            mac_mouse_grabber->releaseMouse();
        mac_mouse_grabber=this;
    }
}

void QWidget::releaseMouse()
{
    if(!qt_nograb() && mac_mouse_grabber == this)
        mac_mouse_grabber = 0;
}

void QWidget::grabKeyboard()
{
    if(!qt_nograb()) {
        if(mac_keyboard_grabber)
            mac_keyboard_grabber->releaseKeyboard();
        mac_keyboard_grabber = this;
    }
}

void QWidget::releaseKeyboard()
{
    if(!qt_nograb() && mac_keyboard_grabber == this)
        mac_keyboard_grabber = 0;
}

QWidget *QWidget::mouseGrabber()
{
    return mac_mouse_grabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return mac_keyboard_grabber;
}

void QWidget::activateWindow()
{
    QWidget *tlw = window();
    if(!tlw->isVisible() || !tlw->isWindow() || (tlw->windowType() == Qt::Desktop))
        return;
    qt_event_remove_activate();

    QWidget *fullScreenWidget = tlw;
    QWidget *parentW = tlw;
    // Find the oldest parent or the parent with fullscreen, whichever comes first.
    while (parentW) {
        fullScreenWidget = parentW->window();
        if (fullScreenWidget->windowState() & Qt::WindowFullScreen)
            break;
        parentW = fullScreenWidget->parentWidget();
    }

    if (fullScreenWidget->windowType() != Qt::ToolTip) {
        qt_mac_set_fullscreen_mode((fullScreenWidget->windowState() & Qt::WindowFullScreen) &&
                                               qApp->desktop()->screenNumber(this) == 0);
    }

    WindowPtr window = qt_mac_window_for(tlw);
    if((tlw->windowType() == Qt::Popup) || (tlw->windowType() == Qt::Tool) ||
       qt_mac_is_macdrawer(tlw) || IsWindowActive(window)) {
        ActivateWindow(window, true);
        qApp->setActiveWindow(tlw);
    } else if(!isMinimized()){
        SelectWindow(window);
    }
    SetUserFocusWindow(window);
}

void QWidget::update()
{
    if (updatesEnabled() && isVisible()) {
        if (testAttribute(Qt::WA_WState_InPaintEvent)) {
            QApplication::postEvent(this, new QUpdateLaterEvent(rect()));
        } else {
            HIViewSetNeedsDisplay(qt_mac_hiview_for(this), true);
            d_func()->dirtyWidget_sys(rect());
        }
    }
}

void QWidget::update(const QRect &r)
{
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
    if(w < 0)
        w = data->crect.width()  - x;
    if(h < 0)
        h = data->crect.height() - y;
    if (w && h && updatesEnabled() && isVisible()) {
        if (testAttribute(Qt::WA_WState_InPaintEvent)) {
            QApplication::postEvent(this, new QUpdateLaterEvent(QRect(x, y, w, h)));
        } else {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
                HIRect r = CGRectMake(x, y, w, h);
                HIViewSetNeedsDisplayInRect(qt_mac_hiview_for(this), &r, true);
                d_func()->dirtyWidget_sys(QRegion(x, y, w, h));
            } else
#endif
            {
                update(QRegion(x, y, w, h));
            }
        }
    }
}

void QWidget::update(const QRegion &rgn)
{
    if (updatesEnabled() && isVisible() && !rgn.isEmpty()) {
        if (testAttribute(Qt::WA_WState_InPaintEvent))
            QApplication::postEvent(this, new QUpdateLaterEvent(rgn));
        else {
            HIViewSetNeedsDisplayInRegion(qt_mac_hiview_for(this), rgn.handle(true), true);
            d_func()->dirtyWidget_sys(rgn);
        }
    }
}

void QWidget::repaint(const QRegion &rgn)
{
    if(rgn.isEmpty())
        return;

    HIViewSetNeedsDisplayInRegion(qt_mac_hiview_for(this), rgn.handle(true), true);
    d_func()->dirtyWidget_sys(rgn, true);
#if 0 && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    OSStatus (*HIViewRender_ptr)(HIViewRef) = HIViewRender; // workaround for gcc warning
    if(HIViewRender_ptr)
        (*HIViewRender_ptr)(qt_mac_hiview_for(window())); //yes the top level!!
#endif
}

void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop)) //desktop is always visible
        return;

    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;
    q->setAttribute(Qt::WA_Mapped);
    if (q->testAttribute(Qt::WA_DontShowOnScreen))
        return;

    if(q->isWindow() && !q->testAttribute(Qt::WA_Moved)) {
        q->createWinId();
        if (QWidget *p = q->parentWidget()) {
            p->createWinId();
            RepositionWindow(qt_mac_window_for(q), qt_mac_window_for(p), kWindowCenterOnParentWindow);
        } else {
            RepositionWindow(qt_mac_window_for(q), 0, kWindowCenterOnMainScreen);
        }
    }
    data.fstrut_dirty = true;
    if(q->isWindow()) {
        setModal_sys();
        WindowPtr window = qt_mac_window_for(q);
        SizeWindow(window, q->width(), q->height(), true);
        if(qt_mac_is_macsheet(q)) {
            qt_event_request_showsheet(q);
        } else if(qt_mac_is_macdrawer(q)) {
            OpenDrawer(window, kWindowEdgeDefault, false);
        } else {
            if (data.window_modality == Qt::WindowModal) {
                if (q->parentWidget())
                    SetWindowModality(window, kWindowModalityWindowModal,
                                      qt_mac_window_for(q->parentWidget()->window()));
            }
            ShowHide(window, true);
            toggleDrawers(true);
        }
        if (q->windowState() & Qt::WindowMinimized) //show in collapsed state
            CollapseWindow(window, true);
        else if (!q->testAttribute(Qt::WA_ShowWithoutActivating))
            qt_event_request_activate(q);
    } else if(!q->parentWidget() || q->parentWidget()->isVisible()) {
        HIViewSetVisible(qt_mac_hiview_for(q), true);
    }
    qt_event_request_window_change(q);
}

void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop)) //you can't hide the desktop!
        return;

    if(q->isWindow()) {
        WindowPtr window = qt_mac_window_for(q);
        if(qt_mac_is_macsheet(q)) {
            WindowPtr parent = 0;
            if(GetSheetWindowParent(window, &parent) != noErr || !parent)
                ShowHide(window, false);
            else
                HideSheetWindow(window);
        } else if(qt_mac_is_macdrawer(q)) {
            CloseDrawer(window, false);
        } else {
            ShowHide(window, false);
            toggleDrawers(false);
            if (data.window_modality == Qt::WindowModal) {
                if (q->parentWidget())
                    SetWindowModality(window, kWindowModalityNone,
                                      qt_mac_window_for(q->parentWidget()->window()));
            }
        }
        if(q->isActiveWindow() && !(q->windowType() == Qt::Popup)) {
            QWidget *w = 0;
            if(q->parentWidget())
                w = q->parentWidget()->window();
            if(!w || (!w->isVisible() && !w->isMinimized())) {
                for(WindowPtr wp = GetFrontWindowOfClass(kDocumentWindowClass, true);
                    wp; wp = GetNextWindowOfClass(wp, kDocumentWindowClass, true)) {
                    if((w = qt_mac_find_window(wp)))
                        break;
                }
                if (!w){
                    for(WindowPtr wp = GetFrontWindowOfClass(kSimpleWindowClass, true);
                        wp; wp = GetNextWindowOfClass(wp, kSimpleWindowClass, true)) {
                        if((w = qt_mac_find_window(wp)))
                            break;
                    }
                }
            }
            if(w && w->isVisible() && !w->isMinimized())
                qt_event_request_activate(w);
        }
    } else {
        HIViewSetVisible(qt_mac_hiview_for(q), false);
    }
    qt_event_request_window_change(q);
    deactivateWidgetCleanup();
    qt_mac_event_release(q);
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    bool needShow = false;
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;

    bool needSendStateChange = true;
    if(isWindow()) {
        if((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if(newstate & Qt::WindowFullScreen) {
                if(QTLWExtra *tlextra = d->topData()) {
                    if(tlextra->normalGeometry.width() < 0) {
                        if(!testAttribute(Qt::WA_Resized))
                            adjustSize();
                        tlextra->normalGeometry = geometry();
                    }
                    tlextra->savedFlags = windowFlags();
                }
                needShow = isVisible();
                const QRect fullscreen(qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this)));
                setParent(parentWidget(), Qt::Window | Qt::FramelessWindowHint | (windowFlags() & 0xffff0000)); //save
                setGeometry(fullscreen);
                if(!qApp->desktop()->screenNumber(this))
                    qt_mac_set_fullscreen_mode(true);
            } else {
                needShow = isVisible();
                setParent(parentWidget(), d->topData()->savedFlags);
                setGeometry(d->topData()->normalGeometry);
                if(!qApp->desktop()->screenNumber(this))
                    qt_mac_set_fullscreen_mode(false);
                d->topData()->normalGeometry.setRect(0, 0, -1, -1);
            }
        }

        d->createWinId();

        WindowRef window = qt_mac_window_for(this);
        if((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized)) {
            if (oldstate & Qt::WindowMinimized){
                // The window is about to be expanded. Send a show
                // event early so it happends before 'activated':
                QShowEvent qse;
                QApplication::sendSpontaneousEvent(this, &qse);
                CollapseWindow(window, false);
            } else
                CollapseWindow(window, true);
            needSendStateChange = oldstate == windowState(); // Collapse didn't change our flags.
        }

        if((newstate & Qt::WindowMaximized) && !((newstate & Qt::WindowFullScreen))) {
            if(QTLWExtra *tlextra = d->topData()) {
                if(tlextra->normalGeometry.width() < 0) {
                    if(!testAttribute(Qt::WA_Resized))
                        adjustSize();
                    tlextra->normalGeometry = geometry();
                }
            }
        } else if(!(newstate & Qt::WindowFullScreen)) {
//            d->topData()->normalGeometry = QRect(0, 0, -1, -1);
        }

#ifdef DEBUG_WINDOW_STATE
#define WSTATE(x) qDebug("%s -- %s -- %s", #x, (newstate & x) ? "true" : "false", (oldstate & x) ? "true" : "false")
        WSTATE(Qt::WindowMinimized);
        WSTATE(Qt::WindowMaximized);
        WSTATE(Qt::WindowFullScreen);
#undef WSTATE
#endif
        if(!(newstate & (Qt::WindowMinimized|Qt::WindowFullScreen)) &&
           ((oldstate & Qt::WindowFullScreen) || (oldstate & Qt::WindowMinimized) ||
            (oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized))) {
            if(newstate & Qt::WindowMaximized) {
                Rect bounds;
                HIToolbarRef toolbarRef;
                data->fstrut_dirty = true;
                if (GetWindowToolbar(window, &toolbarRef) == noErr && toolbarRef
                        && !isVisible() && !IsWindowToolbarVisible(window)) {
                    // HIToolbar, needs to be shown so that it's in the structure window
                    // Typically this is part of a main window and will get shown
                    // during the show, but it's will make the maximize all wrong.
                    ShowHideWindowToolbar(window, true, false);
                    d->updateFrameStrut();  // In theory the dirty would work, but it's optimized out if the window is not visible :(
                }
                QDesktopWidget *dsk = QApplication::desktop();
                QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
                SetRect(&bounds, avail.x(), avail.y(), avail.x() + avail.width(), avail.y() + avail.height());
                if(QWExtra *extra = d->extraData()) {
                    if(bounds.right - bounds.left > extra->maxw)
                        bounds.right = bounds.left + extra->maxw;
                    if(bounds.bottom - bounds.top > extra->maxh)
                        bounds.bottom = bounds.top + extra->maxh;
                }
                if(d->topData()) {
                    QRect fs = d->frameStrut();
                    bounds.left += fs.left();
                    if(bounds.right < avail.x()+avail.width())
                        bounds.right = qMin<short>((uint)avail.x()+avail.width(), bounds.right+fs.left());
                    if(bounds.bottom < avail.y()+avail.height())
                        bounds.bottom = qMin<short>((uint)avail.y()+avail.height(), bounds.bottom+fs.top());
                    bounds.top += fs.top();
                    bounds.right -= fs.right();
                    bounds.bottom -= fs.bottom();
                }
                QRect orect(geometry().x(), geometry().y(), width(), height()),
                      nrect(bounds.left, bounds.top, bounds.right - bounds.left,
                            bounds.bottom - bounds.top);
                if(orect != nrect) { // the new rect differ from the old
                    Rect oldr;
                    QTLWExtra *tlextra = d->topData();
                    SetRect(&oldr, tlextra->normalGeometry.left(), tlextra->normalGeometry.top(),
                        tlextra->normalGeometry.right() + 1, tlextra->normalGeometry.bottom() + 1);
                    SetWindowUserState(window, &oldr);

                    SetWindowStandardState(window, &bounds);
                    ZoomWindow(window, inZoomOut, false);
                    bool moved = testAttribute(Qt::WA_Moved);
                    bool resized = testAttribute(Qt::WA_Resized);
                    setGeometry(nrect);
                    setAttribute(Qt::WA_Moved, moved);
                    setAttribute(Qt::WA_Resized, resized);
                    needSendStateChange = oldstate == windowState(); // Zoom didn't change flags.
                }
            } else if(oldstate & Qt::WindowMaximized) {
                ZoomWindow(window, inZoomIn, false);
                if(QTLWExtra *tlextra = d->topData()) {
                    setGeometry(tlextra->normalGeometry);
                    tlextra->normalGeometry.setRect(0, 0, -1, -1);
                }
            }
        }
    }

    data->window_state = newstate;

    if(needShow)
        show();

    if(newstate & Qt::WindowActive)
        activateWindow();

    qt_event_request_window_change(this);
    if (needSendStateChange) {
        QWindowStateChangeEvent e(oldstate);
        QApplication::sendEvent(this, &e);
    }
}

void QWidgetPrivate::setFocus_sys()
{
    Q_Q(QWidget);
    if (q->testAttribute(Qt::WA_WState_Created))
        SetKeyboardFocus(qt_mac_window_for(q), qt_mac_hiview_for(q), 1);
}

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop))
        return;
    if(q->isWindow()) {
        //raise this window
        BringToFront(qt_mac_window_for(q));
        if(qt_mac_raise_process) { //we get to be the active process now
            ProcessSerialNumber psn;
            GetCurrentProcess(&psn);
            SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
        }
    } else if(q->parentWidget()) {
        HIViewSetZOrder(qt_mac_hiview_for(q), kHIViewZOrderAbove, 0);
        qt_event_request_window_change(q);
    }
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    if((q->windowType() == Qt::Desktop))
        return;
    if(q->isWindow()) {
        SendBehind(qt_mac_window_for(q), 0);
    } else if(q->parentWidget()) {
        HIViewSetZOrder(qt_mac_hiview_for(q), kHIViewZOrderBelow, 0);
        qt_event_request_window_change(q);
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget *w)
{
    Q_Q(QWidget);
    if(!w || q->isWindow() || (q->windowType() == Qt::Desktop))
        return;

    QWidget *p = q->parentWidget();
    if(!p || p != w->parentWidget())
        return;
    HIViewSetZOrder(qt_mac_hiview_for(q), kHIViewZOrderBelow, qt_mac_hiview_for(w));
    qt_event_request_window_change(q);
}

/*
  Helper function for non-toplevel widgets. Helps to map Qt's 32bit
  coordinate system to OS X's 16bit coordinate system.

  Sets the geometry of the widget to data.crect, but clipped to sizes
  that OS X can handle. Unmaps widgets that are completely outside the
  valid range.

  Maintains data.wrect, which is the geometry of the OS X widget,
  measured in this widget's coordinate system.

  if the parent is not clipped, parentWRect is empty, otherwise
  parentWRect is the geometry of the parent's OS X rect, measured in
  parent's coord sys
*/
void QWidgetPrivate::setWSGeometry(bool dontShow, const QRect &oldRect)
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

    /*
      There are up to four different coordinate systems here:
      Qt coordinate system for this widget.
      X coordinate system for this widget (relative to wrect).
      Qt coordinate system for parent
      X coordinate system for parent (relative to parent's wrect).
    */
    QRect validRange(-XCOORD_MAX,-XCOORD_MAX, 2*XCOORD_MAX, 2*XCOORD_MAX);
    QRect wrectRange(-WRECT_MAX,-WRECT_MAX, 2*WRECT_MAX, 2*WRECT_MAX);
    QRect wrect;
    //xrect is the X geometry of my X widget. (starts out in  parent's Qt coord sys, and ends up in parent's X coord sys)
    QRect xrect = data.crect;

    QRect parentWRect = q->parentWidget()->data->wrect;

    if (parentWRect.isValid()) {
        // parent is clipped, and we have to clip to the same limit as parent
        if (!parentWRect.contains(xrect)) {
            xrect &= parentWRect;
            wrect = xrect;
            //translate from parent's to my Qt coord sys
            wrect.translate(-data.crect.topLeft());
        }
        //translate from parent's Qt coords to parent's X coords
        xrect.translate(-parentWRect.topLeft());

    } else {
        // parent is not clipped, we may or may not have to clip

        if (data.wrect.isValid() && QRect(QPoint(),data.crect.size()).contains(data.wrect)) {
            // This is where the main optimization is: we are already
            // clipped, and if our clip is still valid, we can just
            // move our window, and do not need to move or clip
            // children

            QRect vrect = xrect & q->parentWidget()->rect();
            vrect.translate(-data.crect.topLeft()); //the part of me that's visible through parent, in my Qt coords
            if (data.wrect.contains(vrect)) {
                xrect = data.wrect;
                xrect.translate(data.crect.topLeft());
                HIRect bounds = CGRectMake(xrect.x(), xrect.y(),
                                           xrect.width(), xrect.height());
                HIViewSetFrame(qt_mac_hiview_for(q), &bounds);
                if (q->testAttribute(Qt::WA_OutsideWSRange)) {
                    q->setAttribute(Qt::WA_OutsideWSRange, false);
                    if (!dontShow) {
                        q->setAttribute(Qt::WA_Mapped);
                        HIViewSetVisible(qt_mac_hiview_for(q), true);
                    }
                }
                return;
            }
        }

        if (!validRange.contains(xrect)) {
            // we are too big, and must clip
            xrect &=wrectRange;
            wrect = xrect;
            wrect.translate(-data.crect.topLeft());
            //parent's X coord system is equal to parent's Qt coord
            //sys, so we don't need to map xrect.
        }

    }

    // unmap if we are outside the valid window system coord system
    bool outsideRange = !xrect.isValid();
    bool mapWindow = false;
    if (q->testAttribute(Qt::WA_OutsideWSRange) != outsideRange) {
        q->setAttribute(Qt::WA_OutsideWSRange, outsideRange);
        if (outsideRange) {
            HIViewSetVisible(qt_mac_hiview_for(q), false);
            q->setAttribute(Qt::WA_Mapped, false);
        } else if (!q->isHidden()) {
            mapWindow = true;
        }
    }

    if (outsideRange)
        return;

    bool jump = (data.wrect != wrect);
    data.wrect = wrect;


    // and now recursively for all children...
    // ### can be optimized
    for (int i = 0; i < children.size(); ++i) {
        QObject *object = children.at(i);
        if (object->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(object);
            if (!w->isWindow() && w->testAttribute(Qt::WA_WState_Created))
                w->d_func()->setWSGeometry();
        }
    }

    // move ourselves to the new position and map (if necessary) after
    // the movement. Rationale: moving unmapped windows is much faster
    // than moving mapped windows
    HIRect bounds = CGRectMake(xrect.x(), xrect.y(),
                               xrect.width(), xrect.height());

    // StaticContents optimization for non-toplevel widgets: when this flag is set
    // only invalidate the newly exposed areas on pure resizes.
    const QRect newRect(q->mapToParent(QPoint(0, 0)), q->size());
    const bool isMove = (oldRect.topLeft() != newRect.topLeft());
    const bool isResize = (oldRect.size() != newRect.size());
    const HIViewRef view = qt_mac_hiview_for(q);
    if (q->layoutDirection() == Qt::LeftToRight && q->testAttribute(Qt::WA_StaticContents)
        && isResize && !isMove) {
        // Update view geometry without repainting.
        HIViewSetDrawingEnabled(view, false);
        HIViewSetFrame(view, &bounds);
        HIViewSetDrawingEnabled(view, true);

        // Invalidate the exposed slices.
        const int startx = oldRect.width();
        const int stopx = newRect.width();
        const int starty = oldRect.height();
        const int stopy = newRect.height();

        const HIRect verticalSlice = CGRectMake(startx, 0, stopx , stopy);
        HIViewSetNeedsDisplayInRect(view, &verticalSlice, true);
        const HIRect horizontalSlice = CGRectMake(0, starty, startx, stopy);
        HIViewSetNeedsDisplayInRect(view, &horizontalSlice, true);
    } else {
        HIViewSetFrame(qt_mac_hiview_for(q), &bounds);
    }

    if  (jump) {
        updateSystemBackground();
        q->update();
    }
    if (mapWindow && !dontShow) {
        q->setAttribute(Qt::WA_Mapped);
        HIViewSetVisible(qt_mac_hiview_for(q), true);
    }
}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

    if(q->windowType() == Qt::Desktop)
        return;

    // Special case for resizing a window: call SetWindowBounds which will
    // send us a kWindowBoundsChangeSizeChanged event, whose handler
    // calls setGeometry_sys_helper().
    // The reason for doing it this way is that SetWindowBounds
    // repaints the window immediately and the only way we can send our resize
    // events at the proper time (after the window has been resized but before
    // the paint) is to handle the BoundsChange event.
    if (q->isWindow() && !(w == 0 && h == 0)) {
        topData()->isSetGeometry = 1;
        topData()->isMove = isMove;
        Rect r; SetRect(&r, x, y, x + w, y + h);
        SetWindowBounds(qt_mac_window_for(q), kWindowContentRgn, &r);
        topData()->isSetGeometry = 0;
    } else {
        setGeometry_sys_helper(x, y, w, h, isMove);
    }
}

void QWidgetPrivate::setGeometry_sys_helper(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    if(QWExtra *extra = extraData()) {        // any size restrictions?
        if(q->isWindow()) {
            WindowPtr window = qt_mac_window_for(q);
            qt_mac_update_sizer(q);
            if(q->windowFlags() & Qt::WindowMaximizeButtonHint) {
                if(extra->maxw && extra->maxh && extra->maxw == extra->minw
                        && extra->maxh == extra->minh) {
                    ChangeWindowAttributes(window, kWindowNoAttributes, kWindowFullZoomAttribute);
                } else {
                    ChangeWindowAttributes(window, kWindowFullZoomAttribute, kWindowNoAttributes);
                }
            }
        }

        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);

        // Deal with size increment
        if(QTLWExtra *top = topData()) {
            if(top->incw) {
                w = w/top->incw;
                w *= top->incw;
            }
            if(top->inch) {
                h = h/top->inch;
                h *= top->inch;
            }
        }
    }

    if (q->isWindow()) {
        w = qMax(0, w);
        h = qMax(0, h);
    }

    QPoint oldp = q->pos();
    QSize  olds = q->size();
    const bool isResize = (olds != QSize(w, h));
    if(!q->isWindow() && !isResize && QPoint(x, y) == oldp)
        return;
    if(isResize && q->isMaximized())
        data.window_state = data.window_state & ~Qt::WindowMaximized;
    const bool visible = q->isVisible();
    data.crect = QRect(x, y, w, h);

    if(q->isWindow()) {
        if(QWExtra *extra = extraData()) { //set constraints
            const float max_f(20000);
#define SF(x) ((x > max_f) ? max_f : x)
            HISize max = CGSizeMake(SF(extra->maxw), SF(extra->maxh));
            HISize min = CGSizeMake(SF(extra->minw), SF(extra->minh));
#undef SF
            SetWindowResizeLimits(qt_mac_window_for(q), &min, &max);
        }
        HIRect bounds = CGRectMake(0, 0, w, h);
        HIViewSetFrame(qt_mac_hiview_for(q), &bounds);
    } else {
        const QRect oldRect(oldp, olds);
        setWSGeometry(false, oldRect);
    }

    if(isMove || isResize) {
        if(!visible) {
            if(isMove && q->pos() != oldp)
                q->setAttribute(Qt::WA_PendingMoveEvent, true);
            if(isResize)
                q->setAttribute(Qt::WA_PendingResizeEvent, true);
        } else {
            if(isResize) { //send the resize event..
                QResizeEvent e(q->size(), olds);
                QApplication::sendEvent(q, &e);
            }
            if(isMove && q->pos() != oldp) { //send the move event..
                QMoveEvent e(q->pos(), oldp);
                QApplication::sendEvent(q, &e);
            }
        }
    }
    qt_event_request_window_change(q);
}

void QWidgetPrivate::setConstraints_sys()
{
}

void QWidgetPrivate::scroll_sys(int dx, int dy)
{
    scroll_sys(dx, dy, QRect());
}

void QWidgetPrivate::scroll_sys(int dx, int dy, const QRect &r)
{
    Q_Q(QWidget);

    const bool valid_rect = r.isValid();
    if (!q->updatesEnabled() &&  (valid_rect || q->children().isEmpty()))
        return;

    qt_event_request_window_change(q);

    if(!valid_rect) {        // scroll children
        QPoint pd(dx, dy);
        QWidgetList moved;
        QObjectList chldrn = q->children();
        for(int i = 0; i < chldrn.size(); i++) {  //first move all children
            QObject *obj = chldrn.at(i);
            if(obj->isWidgetType()) {
                QWidget *w = (QWidget*)obj;
                if(!w->isWindow()) {
                    w->data->crect = QRect(w->pos() + pd, w->size());
                    if (w->testAttribute(Qt::WA_WState_Created)) {
                        HIRect bounds = CGRectMake(w->data->crect.x(), w->data->crect.y(),
                                                   w->data->crect.width(), w->data->crect.height());
                        HIViewSetFrame(qt_mac_hiview_for(w), &bounds);
                    }
                    moved.append(w);
                }
            }
        }
        //now send move events (do not do this in the above loop, breaks QAquaFocusWidget)
        for(int i = 0; i < moved.size(); i++) {
            QWidget *w = moved.at(i);
            QMoveEvent e(w->pos(), w->pos() - pd);
            QApplication::sendEvent(w, &e);
        }
    }

    if (!q->testAttribute(Qt::WA_WState_Created))
        return;

    if (!q->isVisible())
        return;

    HIViewRef hiview = qt_mac_hiview_for(q);


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
    // HIViewScrollRect doesn't scroll the invalidated rects previously set
    // with the HIViewSetNeedsDisplay functions. Scroll and invalidate those
    // rects here.
    QRegion displayRegion = r.isNull() ? dirtyOnWidget : (dirtyOnWidget & r);
    const QVector<QRect> rects = dirtyOnWidget.rects();
    const QVector<QRect>::const_iterator end = rects.end();
    QVector<QRect>::const_iterator it = rects.begin();
    while (it != end) {
        const QRect rect = *it;
        const HIRect hirect = CGRectMake(rect.x() + dx, rect.y() + dy, rect.width(), rect.height());
        HIViewSetNeedsDisplayInRect(hiview, &hirect, true);
        ++it;
    }
} else
#endif
{
    if (HIViewGetNeedsDisplay(hiview)) {
        q->update(valid_rect ? r : q->rect());
        return;
    }
}

    HIRect scrollrect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    OSStatus err = HIViewScrollRect(hiview, valid_rect ? &scrollrect : 0, dx, dy);
    if (err) {
        // The only parameter that can go wrong, is the rect.
        qWarning("QWidget::scroll: Your rectangle was too big for the widget, clipping rect");
        scrollrect = CGRectMake(qMax(r.x(), 0), qMax(r.y(), 0),
                                qMin(r.width(), q->width()), qMin(r.height(), q->height()));
        HIViewScrollRect(hiview, valid_rect ? &scrollrect : 0, dx, dy);
    }
}

int QWidget::metric(PaintDeviceMetric m) const
{
    switch(m) {
    case PdmHeightMM: // 75 dpi is 3dpmm
        return (metric(PdmHeight)*100)/288;
    case PdmWidthMM: // 75 dpi is 3dpmm
        return (metric(PdmWidth)*100)/288;
    case PdmHeight:
    case PdmWidth: {
        HIRect rect;
        HIViewGetFrame(qt_mac_hiview_for(this), &rect);
        if(m == PdmWidth)
            return (int)rect.size.width;
        return (int)rect.size.height; }
    case PdmDepth:
        return 32;
    case PdmNumColors:
        return INT_MAX;
    case PdmDpiX:
    case PdmPhysicalDpiX: {
        extern float qt_mac_defaultDpi_x(); //qpaintdevice_mac.cpp
        return int(qt_mac_defaultDpi_x()); }
    case PdmDpiY:
    case PdmPhysicalDpiY: {
        extern float qt_mac_defaultDpi_y(); //qpaintdevice_mac.cpp
        return int(qt_mac_defaultDpi_y()); }
    default: //leave this so the compiler complains when new ones are added
        qWarning("QWidget::metric: Unhandled parameter %d", m);
        return QPaintDevice::metric(m);
    }
    return 0;
}

void QWidgetPrivate::createSysExtra()
{
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->wclass = 0;
    extra->topextra->group = 0;
    extra->topextra->windowIcon = 0;
    extra->topextra->resizer = 0;
    extra->topextra->isSetGeometry = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if(extra->topextra->group) {
        qt_mac_release_window_group(extra->topextra->group);
        extra->topextra->group = 0;
    }
}

void QWidgetPrivate::updateFrameStrut()
{
    Q_Q(QWidget);

    QWidgetPrivate *that = const_cast<QWidgetPrivate*>(this);

    that->data.fstrut_dirty = false;
    QTLWExtra *top = that->topData();

    Rect window_r;
    GetWindowStructureWidths(qt_mac_window_for(q), &window_r);
    top->frameStrut.setCoords(window_r.left, window_r.top, window_r.right, window_r.bottom);
}

void QWidgetPrivate::registerDropSite(bool on)
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created))
        return;
    SetControlDragTrackingEnabled(qt_mac_hiview_for(q), on);
}

void QWidget::setMask(const QRegion &region)
{
    Q_D(QWidget);
    // ### Paul: Consider making this cross-platform?
    if (region.isEmpty() && (!d->extra || d->extraData()->mask.isEmpty()))
        return;

    d->createExtra();
    d->extra->mask = region;
    if (!testAttribute(Qt::WA_WState_Created))
        return;

    if (isWindow())
        ReshapeCustomWindow(qt_mac_window_for(this));
    else
        HIViewReshapeStructure(qt_mac_hiview_for(this));
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
    setMask(QRegion());
}

extern "C" {
    typedef struct CGSConnection *CGSConnectionRef;
    typedef struct CGSWindow *CGSWindowRef;
    extern OSStatus CGSSetWindowAlpha(CGSConnectionRef, CGSWindowRef, float);
    extern CGSWindowRef GetNativeWindowFromWindowRef(WindowRef);
    extern CGSConnectionRef _CGSDefaultConnection();
}

void QWidgetPrivate::setWindowOpacity_sys(qreal level)
{
    Q_Q(QWidget);
    CGSSetWindowAlpha(_CGSDefaultConnection(),
                      GetNativeWindowFromWindowRef(qt_mac_window_for(q)), level);
}

struct QPaintEngineCleanupHandler
{
    inline QPaintEngineCleanupHandler() : engine(0) {}
    inline ~QPaintEngineCleanupHandler() { delete engine; }
    QPaintEngine *engine;
};

Q_GLOBAL_STATIC(QPaintEngineCleanupHandler, engineHandler)

QPaintEngine *QWidget::paintEngine() const
{
    QPaintEngine *&pe = engineHandler()->engine;
#ifdef QT_RASTER_PAINTENGINE
    if (!pe) {
        if(qgetenv("QT_MAC_USE_COREGRAPHICS").isNull())
            pe = new QRasterPaintEngine();
        else
            pe = new QCoreGraphicsPaintEngine();
    }
    if (pe->isActive()) {
        QPaintEngine *engine =
            qgetenv("QT_MAC_USE_COREGRAPHICS").isNull()
            ? (QPaintEngine*)new QRasterPaintEngine() : (QPaintEngine*)new QCoreGraphicsPaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
#else
    if (!pe)
        pe = new QCoreGraphicsPaintEngine();
    if (pe->isActive()) {
        QPaintEngine *engine = new QCoreGraphicsPaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
#endif
    return pe;
}

void QWidgetPrivate::setModal_sys()
{
    Q_Q(QWidget);

    if (!q->testAttribute(Qt::WA_WState_Created) || !q->isWindow())
        return;

    const QWidget * const windowParent = q->window()->parentWidget();
    const QWidget * const primaryWindow = windowParent ? windowParent->window() : 0;
    const bool primaryWindowModal = primaryWindow ? primaryWindow->testAttribute(Qt::WA_ShowModal) : false;
    const bool modal = q->testAttribute(Qt::WA_ShowModal);

    //setup the proper window class
    const WindowRef windowRef = qt_mac_window_for(q);
    WindowClass old_wclass;
    GetWindowClass(windowRef, &old_wclass);

    if (modal || primaryWindowModal) {
        if (!qt_mac_menu_buttons_explicitly_set(q->data->window_flags)){
            if (old_wclass == kDocumentWindowClass || old_wclass == kFloatingWindowClass || old_wclass == kUtilityWindowClass){
                // Only change the class to kMovableModalWindowClass if the no explicit jewels
                // are set (kMovableModalWindowClass can't contain them), and the current window class
                // can be converted to modal (according to carbon doc). Mind the order of
                // HIWindowChangeClass and ChangeWindowAttributes.
                WindowGroupRef group = GetWindowGroup(windowRef);
                HIWindowChangeClass(windowRef, kMovableModalWindowClass);
                quint32 tmpWattr = kWindowCloseBoxAttribute | kWindowHorizontalZoomAttribute;
                ChangeWindowAttributes(windowRef, tmpWattr, kWindowNoAttributes);
                ChangeWindowAttributes(windowRef, kWindowNoAttributes, tmpWattr);
                // If the window belongs to a qt-created group, set that group once more:
                if (data.window_flags & Qt::WindowStaysOnTopHint
                    || q->windowType() == Qt::Popup
                    || q->windowType() == Qt::ToolTip)
                    SetWindowGroup(windowRef, group);
            }
        }
    } else if(windowRef) {
        if (q->window()->d_func()->topData()->wattr |= kWindowCloseBoxAttribute)
            ChangeWindowAttributes(windowRef, kWindowCloseBoxAttribute, kWindowNoAttributes);
        if (q->window()->d_func()->topData()->wattr |= kWindowHorizontalZoomAttribute)
            ChangeWindowAttributes(windowRef, kWindowHorizontalZoomAttribute, kWindowNoAttributes);

        WindowClass newClass = q->window()->d_func()->topData()->wclass;
        if (old_wclass != newClass && newClass != 0){
            WindowGroupRef group = GetWindowGroup(windowRef);
            HIWindowChangeClass(windowRef, newClass);
            // If the window belongs to a qt-created group, set that group once more:
            if (data.window_flags & Qt::WindowStaysOnTopHint
                || q->windowType() == Qt::Popup
                || q->windowType() == Qt::ToolTip)
                SetWindowGroup(windowRef, group);
        }
    }

    // Make sure that HIWindowChangeClass didn't remove drag support
    // or reset the opaque size grip setting:
    SetAutomaticControlDragTrackingEnabledForWindow(windowRef, true);
    macUpdateOpaqueSizeGrip();
}

void QWidgetPrivate::macUpdateHideOnSuspend()
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created) || !q->isWindow() || q->windowType() != Qt::Tool)
        return;
    if(q->testAttribute(Qt::WA_MacAlwaysShowToolWindow))
        ChangeWindowAttributes(qt_mac_window_for(q), 0, kWindowHideOnSuspendAttribute);
    else
        ChangeWindowAttributes(qt_mac_window_for(q), kWindowHideOnSuspendAttribute, 0);
}

void QWidgetPrivate::macUpdateOpaqueSizeGrip()
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created) || !q->isWindow())
        return;

    HIViewRef growBox;
    HIViewFindByID(HIViewGetRoot(qt_mac_window_for(q)), kHIViewWindowGrowBoxID, &growBox);
    if (!growBox)
        return;
    HIGrowBoxViewSetTransparent(growBox, !q->testAttribute(Qt::WA_MacOpaqueSizeGrip));
}

void QWidgetPrivate::macUpdateSizeAttribute()
{
    Q_Q(QWidget);
    QEvent event(QEvent::MacSizeChange);
    QApplication::sendEvent(q, &event);
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(children.at(i));
        if (w && (!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))
              && !q->testAttribute(Qt::WA_MacMiniSize) // no attribute set? inherit from parent
              && !w->testAttribute(Qt::WA_MacSmallSize)
              && !w->testAttribute(Qt::WA_MacNormalSize))
            w->d_func()->macUpdateSizeAttribute();
    }
    resolveFont();
}

void QWidgetPrivate::macUpdateIgnoreMouseEvents()
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created))
        return;

    if(q->isWindow()) {
        if(q->testAttribute(Qt::WA_TransparentForMouseEvents))
            ChangeWindowAttributes(qt_mac_window_for(q), kWindowIgnoreClicksAttribute, 0);
        else
            ChangeWindowAttributes(qt_mac_window_for(q), 0, kWindowIgnoreClicksAttribute);
        ReshapeCustomWindow(qt_mac_window_for(q));
    } else {
#ifndef kHIViewFeatureIgnoresClicks
#define kHIViewFeatureIgnoresClicks kHIViewIgnoresClicks
#endif
        if(q->testAttribute(Qt::WA_TransparentForMouseEvents))
            HIViewChangeFeatures(qt_mac_hiview_for(q), kHIViewFeatureIgnoresClicks, 0);
        else
            HIViewChangeFeatures(qt_mac_hiview_for(q), 0, kHIViewFeatureIgnoresClicks);
        HIViewReshapeStructure(qt_mac_hiview_for(q));
    }
}

void QWidgetPrivate::macUpdateMetalAttribute()
{
    Q_Q(QWidget);
    if (!q->testAttribute(Qt::WA_WState_Created) || !q->isWindow())
        return;

    if (q->isWindow()) {
        QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(q->layout());

        if (q->testAttribute(Qt::WA_MacBrushedMetal)) {
            if (layout)
                layout->updateHIToolBarStatus();
            ChangeWindowAttributes(qt_mac_window_for(q), kWindowMetalAttribute, 0);
        } else {
            ChangeWindowAttributes(qt_mac_window_for(q), 0, kWindowMetalAttribute);
            if (layout)
                layout->updateHIToolBarStatus();
        }
    }
}

QT_END_NAMESPACE
