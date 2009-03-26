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

#ifndef QT_MAC_P_H
#define QT_MAC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qglobal.h"
#include "QtCore/qvariant.h"
#include "private/qcore_mac_p.h"
#include <ApplicationServices/ApplicationServices.h>

#include "QtGui/qpainter.h"
#include "QtGui/qwidget.h"
#include "private/qwidget_p.h"
#ifndef QT_MAC_NO_QUICKDRAW
#include "qpaintdevice.h"
#endif

QT_BEGIN_NAMESPACE

/* Event masks */
// internal Qt types
const UInt32 kEventClassQt = 'cute'; // Event class for our own Carbon events.
enum {
    //AE types
    typeAEClipboardChanged = 1,
    //types
    typeQWidget = 1,  /* QWidget *  */
    typeMacTimerInfo = 2, /* MacTimerInfo * */
    typeQEventDispatcherMac = 3, /* QEventDispatcherMac * */
    //params
    kEventParamMacTimer = 'qtim',     /* typeMacTimerInfo */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    kEventParamQEventDispatcherMac = 'qevd', /* typeQEventDispatcherMac */
    //events
    kEventQtRequestSelect = 12,
    kEventQtRequestContext = 13,
    kEventQtRequestMenubarUpdate = 14,
    kEventQtRequestTimer = 15,
    kEventQtRequestShowSheet = 17,
    kEventQtRequestActivate = 18,
    kEventQtRequestSocketAct = 19,
    kEventQtRequestWindowChange = 20
};

class QMacBlockingFunction //implemented in qeventdispatcher_mac.cpp
{
private:
    class Object;
    static Object *block;
public:
    inline QMacBlockingFunction()  { addRef(); }
    inline ~QMacBlockingFunction() { subRef(); }
    static void addRef();
    static void subRef();
    static bool blocking() { return block != 0; }
};

class Q_GUI_EXPORT QMacCocoaAutoReleasePool
{
private:
    void *pool;
public:
    QMacCocoaAutoReleasePool();
    ~QMacCocoaAutoReleasePool();

    inline void *handle() const { return pool; }
};

class Q_GUI_EXPORT QMacWindowChangeEvent
{
private:
    static QList<QMacWindowChangeEvent*> *change_events;
public:
    QMacWindowChangeEvent() {
    }
    virtual ~QMacWindowChangeEvent() {
    }
    static inline void exec(bool ) {
    }
protected:
    virtual void windowChanged() = 0;
    virtual void flushWindowChanged() = 0;
};

class QMacCGContext
{
    CGContextRef context;
public:
    QMacCGContext(QPainter *p); //qpaintengine_mac.cpp
    inline QMacCGContext() { context = 0; }
    inline QMacCGContext(const QPaintDevice *pdev) {
        extern CGContextRef qt_mac_cg_context(const QPaintDevice *);
        context = qt_mac_cg_context(pdev);
    }
    inline QMacCGContext(CGContextRef cg, bool takeOwnership=false) {
        context = cg;
        if(!takeOwnership)
            CGContextRetain(context);
    }
    inline QMacCGContext(const QMacCGContext &copy) : context(0) { *this = copy; }
    inline ~QMacCGContext() {
        if(context)
            CGContextRelease(context);
    }
    inline bool isNull() const { return context; }
    inline operator CGContextRef() { return context; }
    inline QMacCGContext &operator=(const QMacCGContext &copy) {
        if(context)
            CGContextRelease(context);
        context = copy.context;
        CGContextRetain(context);
        return *this;
    }
    inline QMacCGContext &operator=(CGContextRef cg) {
        if(context)
            CGContextRelease(context);
        context = cg;
        CGContextRetain(context); //we do not take ownership
        return *this;
    }
};

class QMacPasteboardMime;
class QMimeData;

class QMacPasteboard
{
    struct Promise {
        Promise() : itemId(0), convertor(0) { }
        Promise(int itemId, QMacPasteboardMime *c, QString m, QVariant d, int o=0) : itemId(itemId), offset(o), convertor(c), mime(m), data(d) { }
        int itemId, offset;
        QMacPasteboardMime *convertor;
        QString mime;
        QVariant data;
    };
    QList<Promise> promises;

    PasteboardRef paste;
    uchar mime_type;
    mutable QPointer<QMimeData> mime;
    mutable bool mac_mime_source;
    static OSStatus promiseKeeper(PasteboardRef, PasteboardItemID, CFStringRef, void *);
    void clear_helper();
public:
    QMacPasteboard(PasteboardRef p, uchar mime_type=0);
    QMacPasteboard(uchar mime_type);
    QMacPasteboard(CFStringRef name=0, uchar mime_type=0);
    ~QMacPasteboard();

    bool hasFlavor(QString flavor) const;
    bool hasOSType(int c_flavor) const;

    PasteboardRef pasteBoard() const;
    QMimeData *mimeData() const;
    void setMimeData(QMimeData *mime);

    QStringList formats() const;
    bool hasFormat(const QString &format) const;
    QVariant retrieveData(const QString &format, QVariant::Type) const;

    void clear();
    bool sync() const;
};

#ifdef Q_WS_MAC64
# define QT_MAC_NO_QUICKDRAW
#endif

extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp
#ifndef QT_MAC_NO_QUICKDRAW

extern WindowPtr qt_mac_window_for(const QWidget*); //qwidget_mac.cpp
class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
    bool valid_gworld;
    void init();

public:
    inline QMacSavedPortInfo() { init(); }
    inline QMacSavedPortInfo(QPaintDevice *pd) { init(); setPaintDevice(pd); }
    inline QMacSavedPortInfo(QWidget *w, bool set_clip = false)
        { init(); setPaintDevice(w, set_clip); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRect &r)
        { init(); setPaintDevice(pd); setClipRegion(r); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRegion &r)
        { init(); setPaintDevice(pd); setClipRegion(r); }
    ~QMacSavedPortInfo();
    static inline bool setClipRegion(const QRect &r);
    static inline bool setClipRegion(const QRegion &r);
    static inline bool setClipRegion(QWidget *w)
        { return setClipRegion(w->d_func()->clippedRegion()); }
    static inline bool setPaintDevice(QPaintDevice *);
    static inline bool setPaintDevice(QWidget *, bool set_clip=false, bool with_child=true);
};

inline bool
QMacSavedPortInfo::setClipRegion(const QRect &rect)
{
    Rect r;
    SetRect(&r, rect.x(), rect.y(), rect.right()+1, rect.bottom()+1);
    ClipRect(&r);
    return true;
}

inline bool
QMacSavedPortInfo::setClipRegion(const QRegion &r)
{
    if(r.isEmpty())
        return setClipRegion(QRect());
    RgnHandle rgn = r.handle();
    if(!rgn)
        return setClipRegion(r.boundingRect());
    SetClip(rgn);
    return true;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QWidget *w, bool set_clip, bool with_child)
{
    if (!w)
        return false;
    if(!setPaintDevice((QPaintDevice *)w))
        return false;
    if(set_clip)
        return setClipRegion(w->d_func()->clippedRegion(with_child));
    return true;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QPaintDevice *pd)
{
    if(!pd)
        return false;
    bool ret = true;
    extern GrafPtr qt_mac_qd_context(const QPaintDevice *); // qpaintdevice_mac.cpp
    if(pd->devType() == QInternal::Widget)
        SetPortWindowPort(qt_mac_window_for(static_cast<QWidget*>(pd)));
    else if(pd->devType() == QInternal::Pixmap || pd->devType() == QInternal::Printer)
        SetGWorld((GrafPtr)qt_mac_qd_context(pd), 0); //set the gworld
    return ret;
}


inline void
QMacSavedPortInfo::init()
{
    GetBackColor(&back);
    GetForeColor(&fore);
    GetGWorld(&world, &handle);
    valid_gworld = true;
    clip = NewRgn();
    GetClip(clip);
    GetPenState(&pen);
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    bool set_state = false;
    if(valid_gworld) {
        set_state = IsValidPort(world);
        if(set_state)
            SetGWorld(world,handle); //always do this one first
    } else {
        setPaintDevice(qt_mac_safe_pdev);
    }
    if(set_state) {
        SetClip(clip);
        SetPenState(&pen);
        RGBForeColor(&fore);
        RGBBackColor(&back);
    }
    DisposeRgn(clip);
}
#else
class QMacSavedPortInfo
{
public:
    inline QMacSavedPortInfo() { }
    inline QMacSavedPortInfo(QPaintDevice *) { }
    inline QMacSavedPortInfo(QWidget *, bool = false) { }
    inline QMacSavedPortInfo(QPaintDevice *, const QRect &) { }
    inline QMacSavedPortInfo(QPaintDevice *, const QRegion &) { }
    ~QMacSavedPortInfo() { }
    static inline bool setClipRegion(const QRect &) { return false; }
    static inline bool setClipRegion(const QRegion &) { return false; }
    static inline bool setClipRegion(QWidget *) { return false; }
    static inline bool setPaintDevice(QPaintDevice *) { return false; }
    static inline bool setPaintDevice(QWidget *, bool =false, bool =true) { return false; }
};
#endif

#ifdef check
# undef check
#endif

QT_END_NAMESPACE

#endif // QT_MAC_P_H
