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

#ifndef GFXTIMELINE_H
#define GFXTIMELINE_H

#include <QObject>
#include "gfx.h"

QTOPIAGFX_EXPORT void setClockCallback(void (*callback)());

class GfxEasing;
class QMutex;
class QTOPIAGFX_EXPORT GfxClock
{
public:
    GfxClock();
    virtual ~GfxClock();

    void tickFor(int time);
    void pauseFor(int time);
    void callback();
    void cancelClock();
    bool isActive() const;
protected:
    virtual void tick(int time);
    virtual void callbackEvent();

private:
    friend class GfxAppClock;
    bool _active;
};

class QTOPIAGFX_EXPORT GfxGlobalClock : public QObject
{
    Q_OBJECT
signals:
    void aboutToTick();
    void tick();
private:
    GfxGlobalClock();
    void emitTick();
    void emitAboutToTick();
    friend class GfxAppClock;
};

template<class T>
class GfxClockMember : public GfxClock
{
public:
    typedef void (T::*Method)(int);
    GfxClockMember(T *t, Method m)
    : _object(t), _method(m) {}

protected:
    virtual void tick(int time)
    { (_object->*_method)(time); }

private:
    T *_object;
    Method _method;
};

class GfxValue;
class GfxEvent;
class GfxTimeLinePrivate;
class GfxTimeLineObject;
class QTOPIAGFX_EXPORT GfxTimeLine : public QObject, private GfxClock
{
Q_OBJECT
public:
    GfxTimeLine(QObject *parent = 0);
    ~GfxTimeLine();

    void pause(GfxTimeLineObject &, int);
    void execute(const GfxEvent &);
    void set(GfxValue &, qreal);

    int accel(GfxValue &, qreal velocity, qreal accel);
    int accel(GfxValue &, qreal velocity, qreal accel, qreal maxDistance);
    int accelDistance(GfxValue &, qreal velocity, qreal distance);

    void move(GfxValue &, qreal destination, int time = 500);
    void move(GfxValue &, qreal destination, const GfxEasing &, int time = 500);
    void moveBy(GfxValue &, qreal change, int time = 500);
    void moveBy(GfxValue &, qreal change, const GfxEasing &, int time = 500);

    void sync();
    void setSyncPoint(int);
    int syncPoint() const;

    void sync(GfxValue &);
    void sync(GfxValue &, GfxValue &);

    void reset(GfxValue &);

    void complete();
    void clear();
    bool isActive() const;

    static bool slowMode();
    static void setSlowMode(bool);

    static void setClockThread();
    static QMutex *lock();
    static GfxGlobalClock *globalClock();
signals:
    void updated();
    void completed();

protected:
    virtual void tick(int);

private:
    void remove(GfxTimeLineObject *);
    friend class GfxTimeLineObject;
    friend class GfxTimeLinePrivate;
    GfxTimeLinePrivate *d;
};

class QTOPIAGFX_EXPORT GfxTimeLineObject
{
public:
    GfxTimeLineObject();
    virtual ~GfxTimeLineObject();

protected:
    friend class GfxTimeLine;
    friend class GfxTimeLinePrivate;
    GfxTimeLine *_t;
};

class QTOPIAGFX_EXPORT GfxValue : public GfxTimeLineObject
{
public:
    GfxValue(qreal v = 0.) : _v(v) {}

    qreal value() const { return _v; }
    virtual void setValue(qreal v) { _v = v; }

    GfxTimeLine *timeLine() const { return _t; }
private:
    friend class GfxTimeLine;
    friend class GfxTimeLinePrivate;
    qreal _v;
};

class QGfxValue : public QObject, public GfxValue
{
    Q_OBJECT
public:
    QGfxValue(qreal v = 0., QObject * = 0);

    virtual void setValue(qreal);

signals:
    void valueChanged(qreal);
};

class QTOPIAGFX_EXPORT GfxValueGroup : public GfxValue
{
public:
    GfxValueGroup(qreal v = 0.) : GfxValue(v) {}

    void addGfxValue(GfxValue *v)
    {
        m_values.append(v);
        v->setValue(value());
    }

    void removeGfxValue(GfxValue *v)
    {
        m_values.removeAll(v);
    }

    virtual void setValue(qreal v) {
        GfxValue::setValue(v);
        for(int i = 0; i < m_values.count(); ++i)
            m_values[i]->setValue(v);
    }
   
private:
    QList<GfxValue *> m_values;
};

class QTOPIAGFX_EXPORT GfxEvent
{
public:
    GfxEvent();
    GfxEvent(const GfxEvent &o);

    template<class T>
    GfxEvent(GfxTimeLineObject *b, T *c, void (T::*func)())
    : d0(callFunc<T>), d1((void *)c), d2((void (GfxEvent::*)())func), d3(b)
    {
    }


    GfxEvent &operator=(const GfxEvent &o);
    void execute() const;
    GfxTimeLineObject *eventObject() const;

private:
    typedef void (*CallFunc)(void *c, void (GfxEvent::*f)());

    template <class T>
    static void callFunc(void *c, void (GfxEvent::*f)())
    {
        T *cls = (T *)c;
        void (T::*func)() = (void (T::*)())f;
        (cls->*func)();
    }
    CallFunc d0;
    void *d1;
    void (GfxEvent::*d2)();
    GfxTimeLineObject *d3;
};

class GfxValueSink
{
public:
    virtual ~GfxValueSink() {}
    virtual void valueChanged(GfxValue *, qreal old, qreal newValue) = 0;
};

class GfxValueNotifying : public GfxValue
{
public:
    GfxValueNotifying(GfxValueSink *s, qreal v = 0.) : GfxValue(v), _sink(s) {}

    virtual void setValue(qreal v)
    {
        qreal old = value();
        GfxValue::setValue(v);
        _sink->valueChanged(this, old, v);
    }

private:
    GfxValueSink *_sink;
};

#endif
