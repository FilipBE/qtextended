/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtTest module of the Qt Toolkit.
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

#ifndef QTESTACCESSIBLE_H
#define QTESTACCESSIBLE_H

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

#ifndef QT_NO_ACCESSIBILITY

#define QTEST_ACCESSIBILITY

#define QVERIFY_EVENT(object, child, event) \
    QVERIFY(QTestAccessibility::verifyEvent(object, child, (int)event))

#include <QtCore/qlist.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qapplication.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Test)

class QObject;

struct QTestAccessibilityEvent
{
    QTestAccessibilityEvent(QObject* o = 0, int c = 0, int e = 0)
        : object(o), child(c), event(e) {}

    bool operator==(const QTestAccessibilityEvent &o) const
    {
        return o.object == object && o.child == child && o.event == event;
    }

    QObject* object;
    int child;
    int event;
};

typedef QList<QTestAccessibilityEvent> EventList;

class QTestAccessibility
{
public:
    static void initialize()
    {
        if (!instance()) {
            instance() = new QTestAccessibility;
            qAddPostRoutine(cleanup);
        }
    }
    static void cleanup()
    {
        delete instance();
        instance() = 0;
    }
    static void clearEvents() { eventList().clear(); }
    static EventList events() { return eventList(); }
    static bool verifyEvent(const QTestAccessibilityEvent& ev)
    {
        if (eventList().isEmpty())
            return FALSE;
        return eventList().takeFirst() == ev;
    }

    static bool verifyEvent(QObject *o, int c, int e)
    {
        return verifyEvent(QTestAccessibilityEvent(o, c, e));
    }

private:
    QTestAccessibility()
    {
        QAccessible::installUpdateHandler(updateHandler);
        QAccessible::installRootObjectHandler(rootObjectHandler);
    }

    ~QTestAccessibility()
    {
        QAccessible::installUpdateHandler(0);
        QAccessible::installRootObjectHandler(0);
    }

    static void rootObjectHandler(QObject *object)
    {
        //    qDebug("rootObjectHandler called %p", object);
        if (object) {
            QApplication* app = qobject_cast<QApplication*>(object);
            if ( !app )
                qWarning("QTEST_ACCESSIBILITY: root Object is not a QApplication!");
        } else {
            qWarning("QTEST_ACCESSIBILITY: root Object called with 0 pointer");
        }
    }

    static void updateHandler(QObject *o, int c, QAccessible::Event e)
    {
        //    qDebug("updateHandler called: %p %d %d", o, c, (int)e);
        eventList().append(QTestAccessibilityEvent(o, c, (int)e));
    }

    static EventList &eventList()
    {
        static EventList list;
        return list;
    }

    static QTestAccessibility *&instance()
    {
        static QTestAccessibility *ta = 0;
        return ta;
    }
};

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
