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

#ifndef QTESTEVENTLOOP_H
#define QTESTEVENTLOOP_H

#include <QtTest/qtest_global.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Test)

class Q_TESTLIB_EXPORT QTestEventLoop : public QObject
{
    Q_OBJECT

public:
    inline QTestEventLoop(QObject *aParent = 0)
        : QObject(aParent), inLoop(false), _timeout(false), timerId(-1), loop(0) {}
    inline void enterLoop(int secs);


    inline void changeInterval(int secs)
    { killTimer(timerId); timerId = startTimer(secs * 1000); }

    inline bool timeout() const
    { return _timeout; }

    inline static QTestEventLoop &instance()
    {
        static QPointer<QTestEventLoop> testLoop;
        if (testLoop.isNull())
            testLoop = new QTestEventLoop(QCoreApplication::instance());
        return *static_cast<QTestEventLoop *>(testLoop);
    }

public Q_SLOTS:
    inline void exitLoop();

protected:
    inline void timerEvent(QTimerEvent *e);

private:
    bool inLoop;
    bool _timeout;
    int timerId;

    QEventLoop *loop;
};

inline void QTestEventLoop::enterLoop(int secs)
{
    Q_ASSERT(!loop);

    QEventLoop l;

    inLoop = true;
    _timeout = false;

    timerId = startTimer(secs * 1000);

    loop = &l;
    l.exec();
    loop = 0;
}

inline void QTestEventLoop::exitLoop()
{
    if (timerId != -1)
        killTimer(timerId);
    timerId = -1;

    if (loop)
        loop->exit();

    inLoop = false;
}

inline void QTestEventLoop::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;
    _timeout = true;
    exitLoop();
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
