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

#include "qsignalspycollector.h"
#include "qsignalspycallback_p.h"
#include "method_p.h"

#ifdef Q_OS_UNIX
# include "qunixsignalnotifier_p.h"
# define DUMP_SIGNAL SIGPROF
#endif

#define DUMP_PATH_ENV "SPYGRIND_OUT_PATH"

#include <QtCore>

typedef QPair<QMetaObject const*,int> MethodKey;
typedef QPair<Method*,Stamp>          MethodCall;

struct PerThreadData
{
    QStack<MethodCall>          callstack;
    Stamp                       tick;
    quint64                     totalBytesAllocated;
    quint64                     totalBytesFreed;
    QHash<MethodKey,Method*>    methods;
};

struct QSignalSpyCollectorPrivate
{
    QSignalSpyCollectorPrivate();

    QString cmd;

    QMutex methodsMutex;
    QHash<QObject*,QMetaObject const*>  metaobjects;

    Method                              mainMethod;
    QList<Method*>                      threadMethods;

    QHash<const void*,size_t>           allocs;

    QThreadStorage<PerThreadData*>      tls_d;

    // Lazily initializes the thread-local data for the calling thread.
    inline PerThreadData& tls()
    {
        if (!tls_d.hasLocalData()) {
            // We must be in a new thread.
            // Set up the initial data for this thread.
            PerThreadData* ptd = new PerThreadData;
            ptd->tick = Stamp::now();
            ptd->totalBytesAllocated = 0;
            ptd->totalBytesFreed     = 0;

            // We can't figure this out totally accurately, but we'll assume
            // that this thread started from a QThread::run() method pretty recently.
            Method* newThreadMethod = new Method;
            newThreadMethod->signature = QString("%1::run()").arg(QThread::currentThread()->metaObject()->className()).toLatin1();
            ptd->callstack.push(qMakePair(newThreadMethod, ptd->tick));

            {
                QMutexLocker lock(&methodsMutex);
                threadMethods << newThreadMethod;
            }

            tls_d.setLocalData(ptd);
        }

        return *tls_d.localData();
    }
};

QSignalSpyCollectorPrivate::QSignalSpyCollectorPrivate()
    : cmd(),
      methodsMutex(QMutex::Recursive),
      metaobjects(),
      mainMethod(),
      allocs(),
      tls_d()
{
    PerThreadData* ptd = new PerThreadData;
    ptd->tick = Stamp::now();
    ptd->totalBytesAllocated = 0;
    ptd->totalBytesFreed     = 0;

    mainMethod.signature = "main(int,char**)";
    ptd->callstack.push(qMakePair(&mainMethod, ptd->tick));

    tls_d.setLocalData(ptd);
}


// These exist solely to avoid a call to instance() on every signal/slot call.
QSignalSpyCollector*        s_instance = 0;
QSignalSpyCollectorPrivate* s_d        = 0;

#ifdef SPYGRIND_MALLOC_HOOK
extern QMutex malloc_hook_mutex;
void init_malloc_hooks();
void uninit_malloc_hooks();
#endif

struct HookFreeZone
{
#ifdef SPYGRIND_MALLOC_HOOK
    HookFreeZone()
        : lock(&malloc_hook_mutex)
    {
        uninit_malloc_hooks();
    }

    ~HookFreeZone()
    {
        init_malloc_hooks();
    }

    QMutexLocker lock;
#endif
};

void function_begin(QObject* caller, int method, void**)
{
    if (!s_d) return;
    if (QCoreApplication::closingDown()) return;

    HookFreeZone hfz; Q_UNUSED(hfz);

    if (s_d->cmd.isEmpty() && QCoreApplication::instance()) {
        s_d->cmd = QCoreApplication::arguments().join(" ");
    }

#ifdef Q_OS_UNIX
    static bool unix_sighandler_installed = false;
    if (!unix_sighandler_installed && QCoreApplication::instance()) {
        unix_sighandler_installed = true;
        if (!QObject::connect(  QUnixSignalNotifier::instance(SIGPROF), SIGNAL(raised()),
                                s_instance,                             SLOT(dumpToFile())))
            Q_ASSERT(0);
    }
#endif

    Stamp now = Stamp::now();
    now.bytesAllocated = s_d->tls().totalBytesAllocated;
    now.bytesFreed     = s_d->tls().totalBytesFreed;

    Method* called = 0;

    {
        QMutexLocker lock(&s_d->methodsMutex);

        QMetaObject const* mo = caller->metaObject();
        Q_ASSERT(mo);
        s_d->metaobjects[caller] = mo;

        MethodKey key = qMakePair(mo, method);
        QHash<MethodKey,Method*>::iterator found = s_d->tls().methods.find(key);
        if (found == s_d->tls().methods.end()) {
            Q_ASSERT(method >= 0);

            // When qt_metacall receives a code it can't handle, it's simply ignored.
            if (method >= mo->methodCount()) return;

            QByteArray sig = mo->method(method).signature();
            Q_ASSERT(!sig.isEmpty());

            Method* m = new Method;
            m->signature = mo->className();
            m->signature.append("::");
            m->signature.append(sig);
            found = s_d->tls().methods.insert(key, m);
        } else {
            Q_ASSERT(*found);
        }
        called = *found;

        // Increment call count in the calling method
        Method* mcaller = s_d->tls().callstack.top().first;
        Q_ASSERT(mcaller);
        ++mcaller->callees[called].callCount;

        // Entering a new method, so record exclusive time spent in calling method.
        mcaller->exclusiveStamp += (now - s_d->tls().tick);
    }

    s_d->tls().tick = now;
    s_d->tls().callstack.push(qMakePair(called, now));
}

// During function_end, the object may have been deleted, so it cannot be accessed at all.
void function_end(QObject* caller, int method)
{

    if (!s_d) return;
    if (QCoreApplication::closingDown()) return;

    HookFreeZone hfz; Q_UNUSED(hfz);

    Stamp now = Stamp::now();
    now.bytesAllocated = s_d->tls().totalBytesAllocated;
    now.bytesFreed     = s_d->tls().totalBytesFreed;

    {
        QMutexLocker lock(&s_d->methodsMutex);

        QMetaObject const* mo = 0;
        // If this is an object being destroyed, remove it from the metaobject hash.
        // QObject::destroyed(QObject*) is always method 0.
        if (!method) {
            mo = s_d->metaobjects.take(caller);
        } else {
            mo = s_d->metaobjects.value(caller);
        }
        // FIXME verify that this really should be silently ignored, rather than an assert
        if (!mo) return;

        Method* m = s_d->tls().methods.value(qMakePair(mo, method));
        if (!m || m != s_d->tls().callstack.top().first) return;

//qDebug("pop %s", s_d->callstack.top().first->signature.constData());

        MethodCall mcallee = s_d->tls().callstack.pop();
        MethodCall mcaller = s_d->tls().callstack.top();

        // Leaving a method.
        // Time since last tick is added to our exclusive time.
        mcallee.first->exclusiveStamp += (now - s_d->tls().tick);

        // Time between function enter and exit is added to our callers inclusive time.
        Q_ASSERT(mcaller.first->callees.contains(m));
        CalleeInfo& call = mcaller.first->callees[m];
        call.stamp += (now - mcallee.second);
    }

    s_d->tls().tick = now;
    Q_ASSERT(s_d->tls().callstack.count());
}

QSignalSpyCollector::QSignalSpyCollector()
    : d(new QSignalSpyCollectorPrivate)
{
    // We may be constructed before QCoreApplication, in which case we'll wait
    // till later to get the command.
    if (QCoreApplication::instance()) {
        d->cmd = QCoreApplication::arguments().join(" ");
    }

    static QString basepath = QString::fromLocal8Bit(qgetenv(DUMP_PATH_ENV));
    qDebug("spygrind: collecting data for process %d%s%s", ::getpid(),
            qPrintable(!basepath.isEmpty() ? QString("; will write to %1").arg(basepath) : QString()),
#ifdef Q_OS_UNIX
            qPrintable(QString("; will dump on signal %1").arg(DUMP_SIGNAL))
#else
            ""
#endif
    );

    QSignalSpyCallbackSet callbacks =
        { function_begin, function_begin, function_end, function_end };
    qt_register_signal_spy_callbacks(callbacks);

    s_instance = this;
    s_d        = d;

#ifdef SPYGRIND_MALLOC_HOOK
    init_malloc_hooks();
#endif
}

QSignalSpyCollector::~QSignalSpyCollector()
{
#ifdef SPYGRIND_MALLOC_HOOK
    uninit_malloc_hooks();
#endif

    QSignalSpyCallbackSet callbacks = { 0, 0, 0, 0 };
    qt_register_signal_spy_callbacks(callbacks);

    foreach (Method* m, d->mainMethod.allCallees())
        delete m;

    foreach (Method* m, d->threadMethods) {
        foreach (Method* sm, m->allCallees()) {
            delete sm;
        }
        delete m;
    }

    delete d;
    d          = 0;
    s_instance = 0;
    s_d        = 0;
}

QSignalSpyCollector* QSignalSpyCollector::instance()
{
    static QSignalSpyCollector collector;
    return &collector;
}

void QSignalSpyCollector::dumpToFile() const
{
    HookFreeZone hfz; Q_UNUSED(hfz);

    // Finish up the main() method.
    Method main = d->mainMethod;

    Stamp now = Stamp::now();
    now.bytesAllocated = d->tls().totalBytesAllocated;
    now.bytesFreed     = d->tls().totalBytesFreed;
    main.exclusiveStamp += (now - d->tls().tick);

    QList<Method*> methods = d->threadMethods;
    methods.prepend(&main);

    QString basepath = QString::fromLocal8Bit(qgetenv(DUMP_PATH_ENV));
    if (!basepath.isEmpty() && !basepath.endsWith("/"))
        basepath+="/";

    int i = 0;
    foreach (Method* m, methods) {
        QString filename = QString("%1spygrind.out.%2.thread%3").arg(basepath).arg(::getpid()).arg(++i);
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            qDebug("spygrind: writing data to %s", qPrintable(filename));
            file.write(m->toCallgrindFormat(d->cmd));
            file.close();
        } else {
            qWarning("spygrind: could not open %s for writing: %s", qPrintable(filename), qPrintable(file.errorString()));
        }
    }

}

void QSignalSpyCollector::recordMalloc (const void*, size_t size, const void* ptr)
{
    // It's not safe to touch thread-local storage while the app or thread is finishing up
    // Cannot use QThread::isFinished(), that tries to lock a mutex which is already locked
    // during a `malloc' in a QThread's constructor.
    if (QCoreApplication::closingDown() || !QAbstractEventDispatcher::instance(QThread::currentThread())) return;

    d->tls().totalBytesAllocated += size;

    d->allocs.insert(ptr, size);
}

void QSignalSpyCollector::recordRealloc(const void*, const void* orig, size_t size, const void* ptr)
{
    if (QCoreApplication::closingDown() || !QAbstractEventDispatcher::instance(QThread::currentThread())) return;

    QHash<const void*,size_t>::iterator iter = d->allocs.find(orig);
    if (iter == d->allocs.end()) return;

    d->tls().totalBytesFreed     += iter.value();
    d->tls().totalBytesAllocated += iter.value();

    d->allocs.remove(iter);
    d->allocs.insert(ptr, size);
}

void QSignalSpyCollector::recordFree   (const void*, const void* ptr)
{
    if (QCoreApplication::closingDown() || !QAbstractEventDispatcher::instance(QThread::currentThread())) return;

    QHash<const void*,size_t>::iterator iter = d->allocs.find(ptr);
    if (iter == d->allocs.end()) return;

    d->tls().totalBytesFreed   += iter.value();

    d->allocs.remove(iter);
}

