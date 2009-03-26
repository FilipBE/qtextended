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

#include <QtCore>

/*
    Simple test app for spygrind.

    This app can be run under spygrind to do a simple sanity check on the
    generated call tree and measured times.
*/

class TestObject : public QObject
{
Q_OBJECT
public:
    TestObject()
        : QObject(0)
    {
        QObject::connect(this, SIGNAL(doublyConnectedToCheap()), SLOT(cheap()));
        QObject::connect(this, SIGNAL(doublyConnectedToCheap()), SLOT(cheap()));
        QObject::connect(this, SIGNAL(connectedToExpensive()), SLOT(expensive()));
        QObject::connect(this, SIGNAL(connectedToExpensive()), SLOT(somewhatExpensive()));

        QObject::connect(this, SIGNAL(connectedToMalloc()), SLOT(mallocsAndFrees49000Bytes()));
        QObject::connect(this, SIGNAL(connectedToMalloc()), SLOT(leaks56000BytesInMainThreadAnd59000BytesElsewhere()));
    }

    void run()
    {
        emit connectedToExpensive();
        emit doublyConnectedToCheap();
        emit connectedToMalloc();
    }

public slots:
    void expensive()
    {
        static qreal foo = 1.0;
        for (int i = 0; i < 500000000; ++i) {
            foo *= 2.0;
            foo /= 2.0;
        }
    }

    void somewhatExpensive()
    {
        static qreal foo = 1.0;
        for (int i = 0; i < 250000000; ++i) {
            foo *= 2.0;
            foo /= 2.0;
        }
    }

    void mallocsAndFrees49000Bytes()
    {
        free(malloc(49000));
    }

    void leaks56000BytesInMainThreadAnd59000BytesElsewhere()
    {
        size_t size = 56000;
        if (QCoreApplication::instance()->thread() != QThread::currentThread())
            size = 59000;
        (void)malloc(size);
    }

    void cheap()
    {}

signals:
    void connectedToExpensive();
    void doublyConnectedToCheap();
    void connectedToMalloc();
};

class ThreadedTestObject : public TestObject
{
    Q_OBJECT
};

class TestThread : public QThread
{
    Q_OBJECT
public:
    void run()
    {
        ThreadedTestObject o;
        o.run();
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    TestThread thread;
    thread.start();

    TestObject obj;
    obj.run();

    thread.wait();

    return 0;
}

#include "main.moc"

