/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDBus module of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSINTEGRATOR_P_H
#define QDBUSINTEGRATOR_P_H

#include <qdbus_symbols_p.h>

#include "qcoreevent.h"
#include "qeventloop.h"
#include "qhash.h"
#include "qobject.h"
#include "private/qobject_p.h"
#include "qlist.h"
#include "qpointer.h"
#include "qsemaphore.h"

#include "qdbusconnection.h"
#include "qdbusmessage.h"
#include "qdbusconnection_p.h"

QT_BEGIN_NAMESPACE

class QDBusConnectionPrivate;

// Really private structs used by qdbusintegrator.cpp
// Things that aren't used by any other file

class QDBusErrorHelper: public QObject
{
    Q_OBJECT
    friend class QDBusConnectionPrivate;
public:
    inline QDBusErrorHelper(QObject *target, const char *member)
    { connect(this, SIGNAL(pendingCallError(QDBusError,QDBusMessage)), target, member, Qt::QueuedConnection); }
signals:
    void pendingCallError(const QDBusError &, const QDBusMessage &);
};

class QDBusReplyWaiter: public QEventLoop
{
    Q_OBJECT
public:
    QDBusMessage replyMsg;

public slots:
    void reply(const QDBusMessage &msg);
    void error(const QDBusError &error, const QDBusMessage &msg);
};

struct QDBusPendingCall
{
    QPointer<QObject> receiver;
    QList<int> metaTypes;
    int methodIdx;
    DBusPendingCall *pending;
    const QDBusConnectionPrivate *connection;
    const char *errorMethod;
    QDBusMessage message;
};

struct QDBusSlotCache
{
    struct Data
    {
        int flags;
        int slotIdx;
        QList<int> metaTypes;
    };
    typedef QMultiHash<QString, Data> Hash;
    Hash hash;
};

class QDBusCallDeliveryEvent: public QMetaCallEvent
{
public:
    QDBusCallDeliveryEvent(const QDBusConnection &c, int id, QObject *sender,
                           const QDBusMessage &msg, const QList<int> &types, int f = 0)
        : QMetaCallEvent(id, sender, -1), connection(c), message(msg), metaTypes(types), flags(f)
        { }

    int placeMetaCall(QObject *object)
    {
        QDBusConnectionPrivate::d(connection)->deliverCall(object, flags, message, metaTypes, id());
        return -1;
    }

private:
    QDBusConnection connection; // just for refcounting
    QDBusMessage message;
    QList<int> metaTypes;
    int flags;
};

class QDBusActivateObjectEvent: public QMetaCallEvent
{
public:
    QDBusActivateObjectEvent(const QDBusConnection &c, QObject *sender,
                             const QDBusConnectionPrivate::ObjectTreeNode &n,
                             int p, const QDBusMessage &m, QSemaphore *s = 0)
        : QMetaCallEvent(-1, sender, -1, 0, 0, 0, s), connection(c), node(n),
          pathStartPos(p), message(m), handled(false)
        { }
    ~QDBusActivateObjectEvent();

    int placeMetaCall(QObject *);

private:
    QDBusConnection connection; // just for refcounting
    QDBusConnectionPrivate::ObjectTreeNode node;
    int pathStartPos;
    QDBusMessage message;
    bool handled;
};

class QDBusConnectionCallbackEvent : public QEvent
{
public:
    QDBusConnectionCallbackEvent()
        : QEvent(User), subtype(Subtype(0))
    { }

    union {
        DBusTimeout *timeout;
        DBusWatch *watch;
    };
    int fd;
    int extra;

    enum Subtype {
        AddTimeout = 0,
        RemoveTimeout,
        AddWatch,
        RemoveWatch,
        ToggleWatch
    } subtype;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDBusSlotCache)

#endif
