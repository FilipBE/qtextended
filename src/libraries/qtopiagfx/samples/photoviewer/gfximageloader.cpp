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

#include "gfximageloader.h"
#include <QEvent>
#include <QCoreApplication>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QWaitCondition>

class GfxImageLoaderEngine : public QThread
{
public:
    GfxImageLoaderEngine();
    virtual ~GfxImageLoaderEngine();

    void ensureRunning();

    void load(GfxImageLoader *out, const QString &filename,
              const QList<QSize> &sizes);
    void abort(GfxImageLoader *out);


protected:
    virtual void run();

private:
    struct GfxImageLoaderEvent : public QEvent
    {
        GfxImageLoaderEvent() : QEvent(QEvent::User) {}

        GfxImageLoader *out;
        int id;
        QList<QImage> imgs;
    };

    struct GfxImageLoaderProxy : public QObject
    {
        GfxImageLoaderProxy(QObject *p, GfxImageLoaderEngine *e)
            : QObject(p), _e(e) {}

        virtual bool event(QEvent *e)
        {
            _e->complete(static_cast<GfxImageLoaderEvent *>(e));
            return true;
        }

        GfxImageLoaderEngine *_e;
    };
    friend class GfxImageLoaderProxy;
    void complete(GfxImageLoaderEvent *);

    QMutex lock;
    QWaitCondition wait;
    struct Op {
        Op(const Op &o) : filename(o.filename), sizes(o.sizes), id(o.id), out(o.out) {}
        Op() : id(0), out(0) {}
        QString filename;
        QList<QSize> sizes;
        unsigned int id;
        GfxImageLoader *out;
    };
    typedef QHash<GfxImageLoader *, Op> Ops;
    Ops ops;
    Ops completedops;
    unsigned int id;
    GfxImageLoaderProxy *proxy;

    void runOp(const Op &);

    QList<QPair<QString, QImage> > cache;
    unsigned int cacheSize;
public:
    unsigned int maxCacheSize;
};
Q_GLOBAL_STATIC(GfxImageLoaderEngine, _engine);

static inline GfxImageLoaderEngine *engine()
{
    GfxImageLoaderEngine *e = _engine();
    e->ensureRunning();
    return e;
}

GfxImageLoaderEngine::GfxImageLoaderEngine()
: id(0), proxy(0), cacheSize(0), maxCacheSize(0)
{
}

GfxImageLoaderEngine::~GfxImageLoaderEngine()
{
}

void GfxImageLoaderEngine::ensureRunning()
{
    if(!proxy) {
        proxy = new GfxImageLoaderProxy(QCoreApplication::instance(), this);
        start();
    }
}

void GfxImageLoaderEngine::complete(GfxImageLoaderEvent *e)
{
    lock.lock();
    Ops::Iterator iter = completedops.find(e->out);
    bool exists = iter != completedops.end() && iter->id == e->id;
    if(iter != completedops.end()) 
        completedops.erase(iter);
    lock.unlock();
    if(exists) {
        e->out->_active = false;
        e->out->images(e->imgs);
    }
}

void GfxImageLoaderEngine::load(GfxImageLoader *out, const QString &filename, 
                                const QList<QSize> &sizes)
{
    lock.lock();

    bool wasEmpty = ops.isEmpty();
    Op op;
    op.filename = filename;
    op.sizes = sizes;
    op.id = id++;
    op.out = out;
    ops.insert(out, op);

    if(wasEmpty)
        wait.wakeAll();
    lock.unlock();
}

void GfxImageLoaderEngine::abort(GfxImageLoader *out)
{
    lock.lock();
    ops.remove(out);
    completedops.remove(out);
    lock.unlock();
}

void GfxImageLoaderEngine::run()
{
    lock.lock();

    while(true) {
        if(ops.isEmpty()) {
            wait.wait(&lock);
        } else {
            Op op = *ops.begin();
            lock.unlock();

            runOp(op);

            lock.lock();
        }
    }
}

void GfxImageLoaderEngine::runOp(const Op &op)
{
    QImage img;
    bool found = false;
    for(int ii = 0; ii < cache.count(); ++ii) {
        if(cache.at(ii).first == op.filename) {
            QPair<QString, QImage> p = cache.at(ii);
            cache.removeAt(ii);
            cache.append(p);
            img = p.second;
            found = true;
            break;
        }
    }
    if(!found) {
        img = QImage(op.filename);
        int contrib = img.height() * img.bytesPerLine();
        if(contrib <= maxCacheSize) {
            cacheSize += contrib;
            cache.append(qMakePair(op.filename, img));
        } 

        while(cacheSize > maxCacheSize) {
            int c = cache.first().second.bytesPerLine() * cache.first().second.height();
            cacheSize -= c;
            cache.removeFirst();
        }
    }

    QList<QImage> imgs;
    if(img.isNull()) {
        for(int ii = 0; ii < op.sizes.count(); ++ii)
            imgs << img;
    } else {
        for(int ii = 0; ii < op.sizes.count(); ++ii)
            imgs << img.scaled(op.sizes.at(ii), Qt::IgnoreAspectRatio, 
                               Qt::SmoothTransformation);
    }

    lock.lock();
    Ops::Iterator iter = ops.find(op.out);
    bool exists = iter != ops.end() && iter->id == op.id;
    if(exists) {
        op.out->filter(imgs);
        completedops.insert(op.out, op);
        ops.erase(iter);

        GfxImageLoaderEvent *e = new GfxImageLoaderEvent();
        e->out = op.out;
        e->id = op.id;
        e->imgs = imgs;
        QCoreApplication::instance()->postEvent(proxy, e);
    }
    lock.unlock();
}

GfxImageLoader::GfxImageLoader()
: _active(false)
{
}

GfxImageLoader::~GfxImageLoader()
{
    if(_active)
        engine()->abort(this);
}

void GfxImageLoader::loadImage(const QString &filename, 
                               const QList<QSize> &sizes)
{
    if(_active) {
        qWarning() << "GfxImageLoader: Image operation already in progress";
        return;
    }
    _active = true;

    engine()->load(this, filename, sizes);
}

void GfxImageLoader::filter(QList<QImage> &imgs)
{
}

void GfxImageLoader::start()
{
    engine();
}

unsigned int GfxImageLoader::cacheSize()
{
    return engine()->maxCacheSize;
}

void GfxImageLoader::setCacheSize(unsigned int size)
{
    engine()->maxCacheSize = size;
}

