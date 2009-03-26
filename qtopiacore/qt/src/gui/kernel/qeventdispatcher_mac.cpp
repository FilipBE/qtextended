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
#include "qplatformdefs.h"
#include "private/qt_mac_p.h"
#include "qeventdispatcher_mac_p.h"
#include "qapplication.h"
#include "qevent.h"
#include "qhash.h"
#include "qsocketnotifier.h"
#include "private/qwidget_p.h"
#include "private/qthread_p.h"
#include "private/qapplication_p.h"

#ifndef QT_NO_THREAD
#  include "qmutex.h"

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE
#endif


/*****************************************************************************
  Externals
 *****************************************************************************/
extern void qt_event_request_timer(MacTimerInfo *); //qapplication_mac.cpp
extern MacTimerInfo *qt_event_get_timer(EventRef); //qapplication_mac.cpp
extern void qt_event_request_select(QEventDispatcherMac *); //qapplication_mac.cpp
extern void qt_event_request_updates(); //qapplication_mac.cpp
extern bool qt_mac_send_event(QEventLoop::ProcessEventsFlags, EventRef, WindowPtr =0); //qapplication_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern bool qt_is_gui_used; //qapplication.cpp

static EventLoopTimerUPP timerUPP = 0;

static inline CFRunLoopRef runLoopForCarbonLoop(EventLoopRef eventLoop)
{
    return reinterpret_cast<CFRunLoopRef>(const_cast<void *>(GetCFRunLoopFromEventLoop(eventLoop)));
}

/*****************************************************************************
  Timers stuff
 *****************************************************************************/

/* timer call back */
static void qt_mac_activate_timer(EventLoopTimerRef, void *data)
{
    MacTimerInfo *tmr = (MacTimerInfo *)data;
    if(QMacBlockingFunction::blocking()) { //just send it immediately
        /* someday this is going to lead to an infite loop, I just know it. I should be marking the
           pending here, and unmarking, but of course single shot timers are removed between now
           and the return (down 4 lines) */
        QTimerEvent e(tmr->id);
        QApplication::sendEvent(tmr->obj, &e);
        qApp->sendPostedEvents();
        return;
    }
    if(tmr->pending)
        return;
    tmr->pending = true;
    qt_event_request_timer(tmr);
}

void QEventDispatcherMac::registerTimer(int timerId, int interval, QObject *obj)
{
#ifndef QT_NO_DEBUG
    if (timerId < 1 || interval < 0 || !obj) {
        qWarning("QEventDispatcherUNIX::registerTimer: invalid arguments");
        return;
    } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::startTimer: timers cannot be started from another thread");
        return;
    }
#endif

    Q_D(QEventDispatcherMac);
    if (!d->macTimerList)
        d->macTimerList = new MacTimerList;

    MacTimerInfo t;
    t.id = timerId;
    t.interval = interval;
    t.obj = obj;
    t.mac_timer = 0;
    t.pending = true;
    if (interval) {
        if (!timerUPP)
            timerUPP = NewEventLoopTimerUPP(qt_mac_activate_timer);
        EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
        d->macTimerList->append(t); //carbon timers go at the end..
        if (InstallEventLoopTimer(GetMainEventLoop(), mint, mint,
                                  timerUPP, &d->macTimerList->last(), &d->macTimerList->last().mac_timer)) {
            qFatal("This cannot really happen, can it!?!");
            return; //exceptional error
        }
        d->macTimerList->last().pending = false;
    } else {
        d->zero_timer_count++;
        if(d->zero_timer_count == 1)
            wakeUp(); //if we are blocking we need to come out of that state
        d->macTimerList->prepend(t); //zero timers come first
        d->macTimerList->first().pending = false;
    }
}

static Boolean find_timer_event(EventRef event, void *data)
{
    return (qt_event_get_timer(event) == ((MacTimerInfo *)data));
}

bool QEventDispatcherMac::unregisterTimer(int id)
{
#ifndef QT_NO_DEBUG
    if (id < 1) {
        qWarning("QEventDispatcherUNIX::unregisterTimer: invalid argument");
        return false;
    } else if (thread() != QThread::currentThread()) {
        qWarning("QObject::killTimer: timers cannot be stopped from another thread");
        return false;
    }
#endif

    Q_D(QEventDispatcherMac);
    if(!d->macTimerList || id <= 0)
        return false;                                // not init'd or invalid timer
    for (int i = 0; i < d->macTimerList->size(); ++i) {
        const MacTimerInfo &t = d->macTimerList->at(i);
        if (t.id == id) {
            if (t.mac_timer) {
                RemoveEventLoopTimer(t.mac_timer);
                if (t.pending) {
                    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
                    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)&t);
                    DisposeEventComparatorUPP(fnc);
                }
            } else {
                d->zero_timer_count--;
            }
            d->macTimerList->removeAt(i);
            return true;
        }
    }
    return false;
}

bool QEventDispatcherMac::unregisterTimers(QObject *obj)
{
#ifndef QT_NO_DEBUG
    if (!obj) {
        qWarning("QEventDispatcherUNIX::unregisterTimers: invalid argument");
        return false;
    } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::killTimers: timers cannot be stopped from another thread");
        return false;
    }
#endif

    Q_D(QEventDispatcherMac);
    if(!d->macTimerList)                                // not initialized
        return false;
    MacTimerList removes;
    for (int i = 0; i < d->macTimerList->size(); ++i) {
        const MacTimerInfo &t = d->macTimerList->at(i);
        if (t.obj == obj) {
            if (t.mac_timer) {
                RemoveEventLoopTimer(t.mac_timer);
                if (t.pending) {
                    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
                    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)&t);
                    DisposeEventComparatorUPP(fnc);
                }
            } else {
                d->zero_timer_count--;
            }
            removes += t;
        }
    }
    for (MacTimerList::Iterator it = removes.begin(); it != removes.end(); ++it) {
        for (int i = 0; i < d->macTimerList->size(); ++i) {
            const MacTimerInfo &info = d->macTimerList->at(i);
            if (info.id == (*it).id)
                d->macTimerList->removeAt(i);
        }
    }
    return true;
}

QList<QEventDispatcherMac::TimerInfo>
QEventDispatcherMac::registeredTimers(QObject *object) const
{
    if (!object) {
        qWarning("QEventDispatcherUNIX:registeredTimers: invalid argument");
        return QList<TimerInfo>();
    }

    Q_D(const QEventDispatcherMac);
    QList<TimerInfo> list;
    if (!d->macTimerList)
        return list;
    for (int i = 0; i < d->macTimerList->size(); ++i) {
        const MacTimerInfo &t = d->macTimerList->at(i);
        if (t.obj == object)
            list << TimerInfo(t.id, t.interval);
    }
    return list;
}

/**************************************************************************
    Socket Notifiers
 *************************************************************************/
void qt_mac_select_timer_callbk(EventLoopTimerRef, void *me)
{
    QEventDispatcherMac *eloop = (QEventDispatcherMac*)me;
    if(QMacBlockingFunction::blocking()) { //just send it immediately
        timeval tm;
        memset(&tm, '\0', sizeof(tm));
        eloop->d_func()->doSelect(QEventLoop::AllEvents, &tm);
    } else {
        qt_event_request_select(eloop);
    }
}
void qt_mac_internal_select_callbk(int, int, QEventDispatcherMac *eloop)
{
     qt_mac_select_timer_callbk(0, eloop);
}

void qt_mac_socket_callback (CFSocketRef s, CFSocketCallBackType callbackType, CFDataRef address,
                                const void  *data,  void  *info ) {
    Q_UNUSED(address); Q_UNUSED(data); Q_UNUSED(s);
    QEventDispatcherMac *const eventDispatcher = reinterpret_cast<QEventDispatcherMac *>(info);
    Q_ASSERT(eventDispatcher);

    switch (callbackType) {
        case kCFSocketReadCallBack:
        case kCFSocketWriteCallBack:
            qt_mac_select_timer_callbk(0, eventDispatcher);
        break;
        default:
        break;
    };
}

/*
    Adds a loop source for the given socket to the current run loop.
*/
CFRunLoopSourceRef qt_mac_add_socket_to_runloop(const CFSocketRef socket)
{
    CFRunLoopSourceRef loopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, socket, 0);
    if (!loopSource)
        return 0;

    CFRunLoopAddSource(CFRunLoopGetCurrent(), loopSource, kCFRunLoopCommonModes);
    return loopSource;
}

/*
    Removes the loop source for the given socket from the current run loop.
*/
void qt_mac_remove_socket_from_runloop(const CFSocketRef socket, CFRunLoopSourceRef runloop)
{
    Q_ASSERT(runloop);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runloop, kCFRunLoopCommonModes);
    CFSocketDisableCallBacks(socket, kCFSocketReadCallBack);
    CFSocketDisableCallBacks(socket, kCFSocketWriteCallBack);
    CFRunLoopSourceInvalidate(runloop);
}

/*
    Register a QSocketNotifier with the mac event system by creating a CFSocket with
    with a read/write callback.

    Qt has separate socket notifiers for reading and writing, but on the mac there is
    a limitation of one CFSocket object for each native socket.
*/
void QEventDispatcherMac::registerSocketNotifier(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier);
    int nativeSocket = notifier->socket();
    int type = notifier->type();
#ifndef QT_NO_DEBUG
    if (nativeSocket < 0 || nativeSocket > FD_SETSIZE) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (notifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
        return;
    }
#endif

    Q_D(QEventDispatcherMac);
    QEventDispatcherUNIX::registerSocketNotifier(notifier);

    if (type == QSocketNotifier::Exception) {
        qWarning("QSocketNotifier::Exception is not supported on Mac OS X");
        return;
    }

    // Check if we have a CFSocket for the native socket, create one if not.
    MacSocketInfo *socketInfo = d->macSockets.value(nativeSocket);
    if (!socketInfo) {
        socketInfo = new MacSocketInfo();

        // Create CFSocket, specify that we want both read and write callbacks (the callbacks
        // are enabled/disabled later on).
        const int callbackTypes = kCFSocketReadCallBack | kCFSocketWriteCallBack;
        CFSocketContext context = {0, this, NULL, NULL, NULL};
        socketInfo->socket = CFSocketCreateWithNative(kCFAllocatorDefault, nativeSocket, callbackTypes, qt_mac_socket_callback, &context);
        if (CFSocketIsValid(socketInfo->socket) == false) {
            qWarning("QEventDispatcherMac::registerSocketNotifier: Failed to create CFSocket");
            return;
        }

        CFOptionFlags flags = CFSocketGetSocketFlags(socketInfo->socket);
        flags |= kCFSocketAutomaticallyReenableWriteCallBack; //QSocketNotifier stays enabled after a write
        flags &= ~kCFSocketCloseOnInvalidate; //QSocketNotifier doesn't close the socket upon destruction/invalidation
        CFSocketSetSocketFlags(socketInfo->socket, flags);

        // Add CFSocket to runloop.
        if(!(socketInfo->runloop = qt_mac_add_socket_to_runloop(socketInfo->socket))) {
            qWarning("QEventDispatcherMac::registerSocketNotifier: Failed to add CFSocket to runloop");
            CFSocketInvalidate(socketInfo->socket);
            CFRelease(socketInfo->socket);
            return;
        }

        // Disable both callback types by default. This must be done after
        // we add the CFSocket to the runloop, or else these calls will have
        // no effect.
        CFSocketDisableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
        CFSocketDisableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);

        d->macSockets.insert(nativeSocket, socketInfo);
    }

    // Increment read/write counters and select enable callbacks if necessary.
    if (type == QSocketNotifier::Read) {
        if (++socketInfo->read == 1)
             CFSocketEnableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
    } else if (type == QSocketNotifier::Write) {
        if (++socketInfo->write == 1)
             CFSocketEnableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
    }
}

/*
    Unregister QSocketNotifer. The CFSocket correspoding to this notifier is
    removed from the runloop of this is the last notifier that users
    that CFSocket.
*/
void QEventDispatcherMac::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier);
    int nativeSocket = notifier->socket();
    int type = notifier->type();
#ifndef QT_NO_DEBUG
    if (nativeSocket < 0 || nativeSocket > FD_SETSIZE) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
        return;
    }
#endif

    Q_D(QEventDispatcherMac);
    QEventDispatcherUNIX::unregisterSocketNotifier(notifier);

    if (type == QSocketNotifier::Exception) {
        qWarning("QSocketNotifier::Exception is not supported on Mac OS X");
        return;
    }
    MacSocketInfo *socketInfo = d->macSockets.value(nativeSocket);
    if (!socketInfo) {
        qWarning("QEventDispatcherMac::unregisterSocketNotifier: Tried to unregister a not registered notifier");
        return;
    }

    // Decrement read/write counters and disable callbacks if necessary.
    if (type == QSocketNotifier::Read) {
        if (--socketInfo->read == 0)
            CFSocketDisableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
    } else if (type == QSocketNotifier::Write) {
        if (--socketInfo->write == 0)
            CFSocketDisableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
    }

    // Remove CFSocket from runloop if this was the last QSocketNotifier.
    if (socketInfo->read <= 0 && socketInfo->write <= 0) {
        if (CFSocketIsValid(socketInfo->socket))
            qt_mac_remove_socket_from_runloop(socketInfo->socket, socketInfo->runloop);
        CFRunLoopSourceInvalidate(socketInfo->runloop);
        CFRelease(socketInfo->runloop);
        CFSocketInvalidate(socketInfo->socket);
        CFRelease(socketInfo->socket);
        delete socketInfo;
        d->macSockets.remove(nativeSocket);
    }
}

bool QEventDispatcherMac::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() || (qt_is_gui_used && GetNumEventsInQueue(GetMainEventQueue()));
}

bool QEventDispatcherMac::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherMac);
#if 0
    //TrackDrag says you may not use the EventManager things..
    if(qt_mac_in_drag) {
        qWarning("Qt: Cannot process events whilst dragging!");
        return false;
    }
#endif
    d->interrupt = false;
    emit awake();

#ifndef QT_MAC_NO_QUICKDRAW
    if(!qt_mac_safe_pdev) { //create an empty widget and this can be used for a port anytime
        QWidget *tlw = new QWidget;
        tlw->setAttribute(Qt::WA_DeleteOnClose);
        tlw->setObjectName(QLatin1String("empty_widget"));
        tlw->hide();
        qt_mac_safe_pdev = tlw;
    }
#endif

    bool retVal = false;
    for (;;) {
        QApplicationPrivate::sendPostedEvents(0, 0, d->threadData);

        if (d->activateTimers() > 0) //send null timers
            retVal = true;

        do {
            EventRef event;
            if (!(flags & QEventLoop::ExcludeUserInputEvents)
                    && !d->queuedUserInputEvents.isEmpty()) {
                // process a pending user input event
                event = d->queuedUserInputEvents.takeFirst();
            } else {
                OSStatus err = ReceiveNextEvent(0,0, kEventDurationNoWait, true, &event);
                if(err != noErr)
                    continue;
                // else
                if (flags & QEventLoop::ExcludeUserInputEvents) {
                     UInt32 ekind = GetEventKind(event),
                            eclass = GetEventClass(event);
                     switch(eclass) {
                         case kEventClassQt:
                             if(ekind != kEventQtRequestContext)
                                 break;
                             // fall through
                         case kEventClassMouse:
                         case kEventClassKeyboard:
                             d->queuedUserInputEvents.append(event);
                             continue;
                     }
                }
            }

            if (!filterEvent(&event) && qt_mac_send_event(flags, event))
                retVal = true;
            ReleaseEvent(event);
        } while(!d->interrupt && GetNumEventsInQueue(GetMainEventQueue()) > 0);

        bool canWait = (d->threadData->canWait
                        && !retVal
                        && !d->interrupt
                        && (flags & QEventLoop::WaitForMoreEvents)
                        && !d->zero_timer_count);
        if (canWait) {
            emit aboutToBlock();
            while(CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e20, true) == kCFRunLoopRunTimedOut);
            flags &= ~QEventLoop::WaitForMoreEvents;
            emit awake();
        } else {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, kEventDurationNoWait, true);
            break;
        }
    }
    return retVal;
}

int QEventDispatcherMacPrivate::activateTimers()
{
    if(!zero_timer_count)
        return 0;
    int ret = 0;
    for (int i = 0; i < macTimerList->size(); ++i) {
        const MacTimerInfo &t = macTimerList->at(i);
        if(!t.interval && !t.pending) {
            ret++;
            const_cast<MacTimerInfo &>(t).pending = true;
            MacTimerInfo tcopy = macTimerList->at(i);
            QTimerEvent e(t.id);
            QApplication::sendEvent(t.obj, &e);

            if (macTimerList->contains(tcopy))
                const_cast<MacTimerInfo &>(t).pending = false;

            if(ret == zero_timer_count)
                break;
        }
    }
    return ret;
}

void QEventDispatcherMac::wakeUp()
{
    Q_D(QEventDispatcherMac);
    CFRunLoopSourceSignal(d->postedEventsSource);
    CFRunLoopWakeUp(runLoopForCarbonLoop(GetMainEventLoop()));
}

void QEventDispatcherMac::flush()
{
    if(qApp) {
        QWidgetList tlws = QApplication::topLevelWidgets();
        for(int i = 0; i < tlws.size(); i++) {
            QWidget *tlw = tlws.at(i);
            if(tlw->isVisible())
                HIWindowFlush(qt_mac_window_for(tlw));

        }
    }
}

/* This allows the eventloop to block, and will keep things going - including keeping away
   the evil spinning cursor */
int qt_mac_send_zero_timers()
{
    if(QEventDispatcherMac *disp = qobject_cast<QEventDispatcherMac*>(QAbstractEventDispatcher::instance()))
        return disp->d_func()->activateTimers();
    return 0;
}

class QMacBlockingFunction::Object : public QObject
{
    QAtomicInt ref;
public:
    Object() { startTimer(10); }

    void addRef() { ref.ref(); }
    bool subRef() { return (ref.deref()); }

protected:
    void timerEvent(QTimerEvent *)
    {
        if(qt_mac_send_zero_timers())
            qApp->sendPostedEvents();
    }
};
QMacBlockingFunction::Object *QMacBlockingFunction::block = 0;
void QMacBlockingFunction::addRef()
{
    if(!block)
        block = new QMacBlockingFunction::Object;
    block->addRef();
}
void QMacBlockingFunction::subRef()
{
    Q_ASSERT(block);
    if(!block->subRef()) {
        delete block;
        block = 0;
    }
}

/*****************************************************************************
  QEventDispatcherMac Implementation
 *****************************************************************************/
QEventDispatcherMacPrivate::QEventDispatcherMacPrivate()
{
    macTimerList = 0;
    zero_timer_count = 0;
}

QEventDispatcherMac::QEventDispatcherMac(QObject *parent)
    : QEventDispatcherUNIX(*new QEventDispatcherMacPrivate, parent)
{
    Q_D(QEventDispatcherMac);
    CFRunLoopSourceContext context;
    bzero(&context, sizeof(CFRunLoopSourceContext));
    context.info = d->threadData;
    context.equal = QEventDispatcherMacPrivate::postedEventSourceEqualCallback;
    context.perform = QEventDispatcherMacPrivate::postedEventsSourcePerformCallback;
    d->postedEventsSource = CFRunLoopSourceCreate(0, 0, &context);
    Q_ASSERT(d->postedEventsSource);
    CFRunLoopAddSource(runLoopForCarbonLoop(GetMainEventLoop()), d->postedEventsSource,
                       kCFRunLoopCommonModes);

}


Boolean QEventDispatcherMacPrivate::postedEventSourceEqualCallback(const void *info1, const void *info2)
{
    return info1 == info2;
}

void QEventDispatcherMacPrivate::postedEventsSourcePerformCallback(void *)
{
}

QEventDispatcherMac::~QEventDispatcherMac()
{
    Q_D(QEventDispatcherMac);
    //timer cleanup
    d->zero_timer_count = 0;
    if(d->macTimerList) {
        for (int i = 0; i < d->macTimerList->size(); ++i) {
            const MacTimerInfo &t = d->macTimerList->at(i);
            if (t.mac_timer) {
                RemoveEventLoopTimer(t.mac_timer);
                if (t.pending) {
                    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
                    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)&t);
                    DisposeEventComparatorUPP(fnc);
                }
            }
        }
        delete d->macTimerList;
        d->macTimerList = 0;
    }
    if(timerUPP) {
        DisposeEventLoopTimerUPP(timerUPP);
        timerUPP = 0;
    }

    // Remove CFSockets from the runloop.
    for (MacSocketHash::ConstIterator it = d->macSockets.constBegin(); it != d->macSockets.constEnd(); ++it) {
        MacSocketInfo *socketInfo = (*it);
        if (CFSocketIsValid(socketInfo->socket)) {
            qt_mac_remove_socket_from_runloop(socketInfo->socket, socketInfo->runloop);
            CFRunLoopSourceInvalidate(socketInfo->runloop);
            CFRelease(socketInfo->runloop);
            CFSocketInvalidate(socketInfo->socket);
            CFRelease(socketInfo->socket);
        }
    }
    CFRunLoopRemoveSource(runLoopForCarbonLoop(GetMainEventLoop()), d->postedEventsSource, kCFRunLoopCommonModes);
    CFRelease(d->postedEventsSource);
    d->postedEventsSource = 0;
}


QT_END_NAMESPACE
