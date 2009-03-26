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

#ifndef QEVENTDISPATCHER_MAC_P_H
#define QEVENTDISPATCHER_MAC_P_H

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

#include "QtGui/qwindowdefs.h"
#include "qhash.h"
#include "private/qeventdispatcher_unix_p.h"
#include "private/qt_mac_p.h"

QT_BEGIN_NAMESPACE

class QEventDispatcherMacPrivate;

class QEventDispatcherMac : public QEventDispatcherUNIX
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherMac)

public:
    explicit QEventDispatcherMac(QObject *parent = 0);
    ~QEventDispatcherMac();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    void registerTimer(int timerId, int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<TimerInfo> registeredTimers(QObject *object) const;

    void wakeUp();
    void flush();

private:
    friend int qt_mac_send_zero_timers();
    friend void qt_mac_select_timer_callbk(__EventLoopTimer*, void*);
    friend class QApplicationPrivate;
};

struct MacTimerInfo {
    int id;
    int interval;
    QObject *obj;
    bool pending;
    EventLoopTimerRef mac_timer;
    bool operator==(const MacTimerInfo &other)
    {
        return (id == other.id);
    }
};
typedef QList<MacTimerInfo> MacTimerList;

struct MacSocketInfo {
    MacSocketInfo() : socket(0), runloop(0), read(0), write(0) {}
    CFSocketRef socket;
    CFRunLoopSourceRef runloop;
    int read;
    int write;
};
typedef QHash<int, MacSocketInfo *> MacSocketHash;

class QEventDispatcherMacPrivate : public QEventDispatcherUNIXPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherMac)

public:
    QEventDispatcherMacPrivate();

    int zero_timer_count;
    MacTimerList *macTimerList;
    int activateTimers();

    MacSocketHash macSockets;
    QList<EventRef> queuedUserInputEvents;
    CFRunLoopSourceRef postedEventsSource;
private:
    static Boolean postedEventSourceEqualCallback(const void *info1, const void *info2);
    static void postedEventsSourcePerformCallback(void *info);
};

QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_MAC_P_H
