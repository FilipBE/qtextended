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

#include "gfxtimeline.h"
#include <QDebug>
#include <sys/time.h>
#include <signal.h>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QEvent>
#include <QCoreApplication>
#include "gfxeasing.h"

static void (*CallbackFunc)() = 0;

void setClockCallback(void (*callback)())
{
    CallbackFunc = callback;
}

//
// GfxAppClock stuff
//
class GfxAppClock : public QObject
{
public:
    GfxAppClock();

    struct Op
    {
        enum Type { Tick, Pause };
        Type type;
        long long startTime;
        long long endTime;

        GfxClock *target;
    };

    void tickFor(int time, GfxClock *);
    void pauseFor(int time, GfxClock *);
    void cancel(GfxClock *);

    void setSlowMode(bool);
    bool slowMode();

    void setClockThread();
    QMutex *lock();
    GfxGlobalClock *globalClock();

protected:
    virtual bool event(QEvent *);
    virtual void timerEvent(QTimerEvent *);

private:
    long long newTime() const;

    int tick();

    bool _inTick;
    int _timerLength;
    int _timerId;

    // Current time
    mutable struct timeval start_time;
    mutable long long _time;
    mutable bool _timeValid;
    bool _inPause;
    long long time() const;

    // Active operations ordered by end time
    void add(const Op &);
    int _tickOps;
    QList<Op> _ops;

    bool _slowMode;
    QThread *_clockThread;
    QMutex _lock;
    volatile bool _threadRunning;
    volatile bool _threadWaiting;
    QWaitCondition _wait;
    GfxGlobalClock _globalClock;

    QSet<GfxClock *> _remTargets;
};

Q_GLOBAL_STATIC(GfxAppClock, appClock);

GfxAppClock::GfxAppClock()
: _inTick(false), _timerLength(0), _timerId(0), _time(0), _timeValid(false), 
  _inPause(false), _tickOps(0), _slowMode(false), _clockThread(0), 
  _threadRunning(false)
{
    ::gettimeofday(&start_time, 0);
    _time = 0;
}

bool GfxAppClock::event(QEvent *e)
{
    if(e->type() == QEvent::User) {
        if(_inPause && _timeValid)
            _timeValid = false;
    }
    return QObject::event(e);
}

void GfxAppClock::timerEvent(QTimerEvent *)
{
    _globalClock.emitAboutToTick();

    if(_timerId) {
        killTimer(_timerId);
        _timerId = 0;
        _timerLength = 0;
    }

    int t = tick();
    if(t != -1)
        _timerId = startTimer(t);

    if(!_timerId || _inPause)
        _timeValid = false;

    _globalClock.emitTick();
}

int GfxAppClock::tick()
{
    _inPause = false;
    _inTick = true;
    long long newTime = this->newTime();
    _timeValid = true;

    // First expunge dead ops
    while(!_ops.isEmpty() && _ops.first().endTime <= newTime) {
        Op op = _ops.takeFirst();
        if(op.type == Op::Tick)
            --_tickOps;
        _time = op.endTime;
        op.target->_active = false;
        op.target->tick(op.endTime - op.startTime);
    }

    // Now advance to current time
    _time = newTime;

    // Tick all active timers
    QList<Op> active = _ops;
    for(int ii = 0; ii < active.count(); ++ii) {
        const Op &op = active.at(ii);
        if(_remTargets.isEmpty() || !_remTargets.contains(op.target))
            if(op.type == Op::Tick)
                op.target->tick(newTime - op.startTime);
    }

    int rv = -1;
    // If only pause in queue, slow timer appropriately
    if(_ops.isEmpty()) {
    } else if(!_tickOps) {
        _timerLength = _ops.first().endTime - newTime;
        rv = _timerLength;
        _inPause = true;
    } else {
        _timerLength = 5;
        rv = _timerLength;
    }
    _inTick = false;
    _remTargets.clear();

    return rv;
}

long long GfxAppClock::time() const
{
    if(!_timeValid) {
        _timeValid = true;
        _time = newTime();

        if(_inPause)
            QCoreApplication::postEvent(const_cast<GfxAppClock *>(this), new QEvent(QEvent::User));
    }
    return _time;
}

#define SLOW_MODE_FACTOR 50
long long GfxAppClock::newTime() const
{
    struct timeval curr_time;
    ::gettimeofday(&curr_time, 0);

    long long nt = (curr_time.tv_sec - start_time.tv_sec) * 1000 +
        (curr_time.tv_usec - start_time.tv_usec) / 1000;

    if(_slowMode)
        nt /= SLOW_MODE_FACTOR;

    if(nt < _time) {
        qWarning() << "GfxAppClock: EEEK Time appears to have gone backwards";
        nt = _time + 1;

        long long  adj_time = nt;
        start_time = curr_time;
        long long ms = qMin((long long)start_time.tv_usec, adj_time * 1000);
        start_time.tv_usec -= ms;
        adj_time -= ms / 1000;

        start_time.tv_sec -= adj_time / 1000;
    }

    return nt;
}

void GfxAppClock::setSlowMode(bool slowMode)
{
    if(slowMode == _slowMode)
        return;

    long long nt = newTime();
    if(slowMode) {
        nt *= SLOW_MODE_FACTOR;
    }

    ::gettimeofday(&start_time, 0);
    if(start_time.tv_usec > nt * 1000) {
        start_time.tv_usec -= nt * 1000;
    } else {
        nt -= start_time.tv_usec / 1000;
        start_time.tv_usec = 0;

        start_time.tv_sec -= nt / 1000;
        nt = nt % 1000;
        if(nt) {
            start_time.tv_sec--;
            start_time.tv_usec = (1000 - nt) * 1000;
        }
    }
    _slowMode = slowMode;
}

bool GfxAppClock::slowMode()
{
    return _slowMode;
}

static volatile bool alarm_ok = false;
static int fds[2];
static void alarm_sig(int)
{    
    char p;
    if(alarm_ok) {
        ::write(fds[1], &p, 1);
        alarm_ok = false;
    } 
}

void GfxAppClock::setClockThread()
{
    lock()->lock();
    _clockThread = QThread::currentThread();
    lock()->unlock();
    qWarning() << "pipe:" << pipe(fds);

    int ms = 5;
    qWarning() << "signal:" << signal(SIGALRM, alarm_sig);
    struct itimerval val;
    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = ms * 1000;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = ms * 1000;

    bool rr = !QString(getenv("GFX_USE_REALTIME")).isEmpty();
    if(rr) {
         struct sched_param p;
         ::bzero(&p, sizeof(struct sched_param));
         p.sched_priority = 1;
         // int rv = sched_setscheduler(0, SCHED_RR, &p);
         int rv = nice(-10);

        qWarning() << "scheduler:" << rv;
    }

    int rv = setitimer(ITIMER_REAL, &val, 0);
    qWarning() << "setitimer:" << rv;

    struct timeval last;
    ::gettimeofday(&last, 0);


    while(true) {
        char p;
        alarm_ok = true;
        _threadWaiting = true;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 30000;
        fd_set s;
        FD_ZERO(&s);
        FD_SET(fds[0], &s);
        if(1 == ::select(fds[0] + 1, &s, 0, 0, &tv))
            ::read(fds[0], &p, 1);
        _threadWaiting = false;
        struct timeval n;
        ::gettimeofday(&n, 0);
#if 0
        int diff = (n.tv_sec - last.tv_sec) * 1000 + 
            (n.tv_usec - last.tv_usec) / 1000;
        if(diff > ((ms * 120) / 100))
            printf("XXXXXXXXXXXXXXXXXXXXXXX Diff error %d\n", diff);
#endif
        last = n;

        lock()->lock();
        _threadRunning = true;
        int rv = tick();
        if(CallbackFunc)
            CallbackFunc();
        if(rv == -1) {
            val.it_value.tv_sec = 0;
            val.it_value.tv_usec = 0;
            val.it_interval.tv_sec = 0;
            val.it_interval.tv_usec = 0;
            setitimer(ITIMER_REAL, &val, 0);
            _threadRunning = false;
            _timeValid = false;
            _wait.wait(lock());
            _threadRunning = true;
            val.it_value.tv_sec = 0;
            val.it_value.tv_usec = ms * 1000;
            val.it_interval.tv_sec = 0;
            val.it_interval.tv_usec = ms * 1000;
            setitimer(ITIMER_REAL, &val, 0);
        }
        lock()->unlock();
    }
}

QMutex *GfxAppClock::lock()
{
    return &_lock;
}

GfxGlobalClock *GfxAppClock::globalClock()
{
    return &_globalClock;
}

void GfxAppClock::tickFor(int time, GfxClock *target)
{
    Op op;
    op.type = Op::Tick;
    op.startTime = this->time();
    op.endTime = op.startTime + time;
    op.target = target;
    add(op);
}

void GfxAppClock::pauseFor(int time, GfxClock *target)
{
    Op op;
    op.type = Op::Pause;
    op.startTime = this->time();
    op.endTime = op.startTime + time;
    op.target = target;
    add(op);
}

void GfxAppClock::add(const Op &op)
{
    if(op.type == Op::Tick)
        _tickOps++;

    for(int ii = 0; ii < _ops.count(); ++ii) {
        if(op.endTime < _ops.at(ii).endTime) {
            _ops.insert(ii, op);
            return;
        }
    }

    _ops.append(op);

    if(!_inTick) {
        int timerlength;
        if(_tickOps)
            timerlength = 20;
        else
            timerlength = _ops.first().endTime - time();

        if(!_clockThread) {
            if(timerlength != _timerLength) {
                if(_timerId)
                    killTimer(_timerId);
                _timerLength = timerlength;
                _timerId = startTimer(timerlength);
            }
        } else if(QThread::currentThread() != _clockThread) {
            if(!_threadRunning) {
                _wait.wakeAll();
            } else {
            }
        }
    }
}

void GfxAppClock::cancel(GfxClock *t)
{
    for(int ii = 0; ii < _ops.count(); ++ii)
        if(_ops.at(ii).target == t) {
            if(_ops.at(ii).type == Op::Tick)
                --_tickOps;
            _ops.removeAt(ii);

            if(_inTick)
                _remTargets.insert(t);

            return;
        }

    qWarning() << "GfxAppClock: Attempt to remove non-existant GfxClock";
}

/*!
    \class GfxGlobalClock
    \inpublicgroup QtBaseModule
    \brief The GfxGlobalClock class ticks for all timelines.
*/

/*!
    \fn void GfxGlobalClock::tick()

    This signal is emitted whenever any timeline modifies GfxValues. Even if multiple
    GfxValues are changed, this signal is only emitted once for each clock tick.

    \sa aboutToTick()
*/

/*!
    \fn void GfxGlobalClock::aboutToTick()

    This signal is emitted immediately before a tick occurs.

    \sa tick()
*/

/*! \internal */
GfxGlobalClock::GfxGlobalClock()
{
}

void GfxGlobalClock::emitAboutToTick()
{
    emit aboutToTick();
}

void GfxGlobalClock::emitTick()
{
    emit tick();
}

GfxClock::GfxClock()
: _active(false)
{
}

GfxClock::~GfxClock()
{
    cancelClock();
}

bool GfxClock::isActive() const
{
    return _active;
}

void GfxClock::tickFor(int time)
{
    Q_ASSERT(time >= 0);

    cancelClock();
    if(time == 0) {
        tick(0);
        return;
    }

    _active = true;

    GfxAppClock *clock = appClock();
    if(clock)
        clock->tickFor(time, this);
}

void GfxClock::pauseFor(int time)
{
    Q_ASSERT(time >= 0);
    cancelClock();

    if(time == 0) {
        tick(0);
        return;
    }

    _active = true;
    GfxAppClock *clock = appClock();
    if(clock)
        clock->pauseFor(time, this);
}

void GfxClock::cancelClock()
{
    if(_active) {
        GfxAppClock *clock = appClock();
        if(clock)
            clock->cancel(this);
        _active = false;
    }
}

void GfxClock::tick(int /*time*/)
{
    // Does nothing
}

void GfxClock::callbackEvent()
{
    // Does nothing
}

//
// Timeline stuff
//

struct Update {
    Update(GfxValue *_g, qreal _v)
        : g(_g), v(_v) {}
    Update(const GfxEvent &_e)
        : g(0), v(0), e(_e) {}

    GfxValue *g;
    qreal v;
    GfxEvent e;
};

struct GfxTimeLinePrivate
{
    GfxTimeLinePrivate(GfxTimeLine *);

    struct Op {
        enum Type {
            Pause,
            Set,
            Move,
            MoveBy,
            Accel,
            AccelDistance,
            Execute
        };
        Op() {}
        Op(Type t, int l, qreal v, qreal v2, int o, 
           const GfxEvent &ev = GfxEvent(), const GfxEasing &es = GfxEasing())
            : type(t), length(l), value(v), value2(v2), order(o), event(ev),
              easing(es) {}
        Op(const Op &o)
            : type(o.type), length(o.length), value(o.value), value2(o.value2),
              order(o.order), event(o.event), easing(o.easing) {}
        Op &operator=(const Op &o) {
            type = o.type; length = o.length; value = o.value; 
            value2 = o.value2; order = o.order; event = o.event; 
            easing = o.easing;
            return *this;
        }

        Type type;
        int length;
        qreal value;
        qreal value2;

        int order;
        GfxEvent event;
        GfxEasing easing;
    };
    struct TimeLine
    {
        TimeLine() : length(0), consumedOpLength(0), base(0.) {}
        QList<Op> ops;
        int length;
        int consumedOpLength;
        qreal base;
    };

    int length;
    int syncPoint;
    typedef QHash<GfxTimeLineObject *, TimeLine> Ops;
    Ops ops;
    GfxTimeLine *q;

    void add(GfxTimeLineObject &, const Op &);
    qreal value(const Op &op, int time, qreal base, bool *) const;

    int advance(int);

    bool clockRunning;
    int prevTime;

    int order;
    static bool slowMode;

    QList<QPair<int, Update> > *updateQueue;
};
bool GfxTimeLinePrivate::slowMode = false;

GfxTimeLinePrivate::GfxTimeLinePrivate(GfxTimeLine *parent)
: length(0), syncPoint(0), q(parent), clockRunning(false), prevTime(0), order(0), updateQueue(0)
{
}

void GfxTimeLinePrivate::add(GfxTimeLineObject &g, const Op &o)
{
    if(g._t && g._t != q) {
        qWarning() << "GfxTimeLine: Cannot modify a GfxValue owned by"
                   << "another timeline.";
        return;
    }
    g._t = q;

    Ops::Iterator iter = ops.find(&g);
    if(iter == ops.end()) {
        iter = ops.insert(&g, TimeLine());
        if(syncPoint > 0)
            q->pause(g, syncPoint);
    }
    if(!iter->ops.isEmpty() &&
       o.type == Op::Pause &&
       iter->ops.last().type == Op::Pause) {
        iter->ops.last().length += o.length;
        iter->length += o.length;
    } else {
        iter->ops.append(o);
        iter->length += o.length;
    }

    if(iter->length > length)
        length = iter->length;

    if(!clockRunning) {
        q->cancelClock();
        prevTime = 0;
        clockRunning = true;
        q->GfxClock::tickFor(100000);
    }
}

qreal GfxTimeLinePrivate::value(const Op &op, int time, qreal base, bool *changed) const
{
    Q_ASSERT(time >= 0);
    Q_ASSERT(time <= op.length);
    *changed = true;

    switch(op.type) {
        case Op::Pause:
            *changed = false;
            return base;
        case Op::Set:
            return op.value;
        case Op::Move:
            if(time == 0) {
                return base;
            } else if(time == (op.length)) {
                return op.value;
            } else {
                if(op.easing.isLinear()) {
                    qreal delta = op.value - base;
                    qreal pTime = (qreal)(time) / (qreal)op.length;
                    return base + delta * pTime;
                } else {
                    return op.easing.valueAt(time, base, op.value, op.length);
                }
            }
        case Op::MoveBy:
            if(time == 0) {
                return base;
            } else if(time == (op.length)) {
                return base + op.value;
            } else {
                if(op.easing.isLinear()) {
                    qreal delta = op.value;
                    qreal pTime = (qreal)(time) / (qreal)op.length;
                    return base + delta * pTime;
                } else {
                    return op.easing.valueAt(time, base, base + op.value, op.length);
                }
            }
        case Op::Accel:
            if(time == 0) {
                return base;
            } else {
                qreal t = (qreal)(time) / 1000.0f;
                qreal delta = op.value * t + 0.5f * op.value2 * t * t;
                return base + delta;
            }
        case Op::AccelDistance:
            if(time == 0) {
                return base;
            } else if(time == (op.length)) {
                return base + op.value2;
            } else {
                qreal t = (qreal)(time) / 1000.0f;
                qreal accel = -1.0f * 1000.0f * op.value / (qreal)op.length;
                qreal delta = op.value * t + 0.5f * accel * t * t;
                return base + delta;

            }
        case Op::Execute:
            op.event.execute();
            *changed = false;
            return -1;
    }

    return base;
}

/*!
    \class GfxTimeLine
    \inpublicgroup QtBaseModule
    \brief The GfxTimeLine class provides a timeline for controlling animations.

    GfxTimeLine is similar to QTimeLine except:
    \list
    \i It updates GfxValue instances directly, rather than maintaining a single
    current value.

    For example, the following animates a simple value over 200 milliseconds:
    \code
    GfxValue v(<starting value>);
    GfxTimeLine tl;
    tl.move(v, 100., 200);
    tl.start()
    \endcode

    If your program needs to know when values are changed, it can either
    connect to the GfxTimeLine's updated() signal, or inherit from GfxValue
    and reimplement the GfxValue::setValue() method.

    \i Supports multiple GfxValue, arbitrary start and end values and allows
    animations to be strung together for more complex effects.

    For example, the following animation moves the x and y coordinates of
    an object from wherever they are to the position (100, 100) in 50
    milliseconds and then further animates them to (100, 200) in 50
    milliseconds:

    \code
    GfxValue x(<starting value>);
    GfxValue y(<starting value>);

    GfxTimeLine tl;
    tl.start();

    tl.move(x, 100., 50);
    tl.move(y, 100., 50);
    tl.move(y, 200., 50);
    \endcode

    \i All GfxTimeLine instances share a single, synchronized clock.

    Actions scheduled within the same event loop tick are scheduled
    synchronously against each other, regardless of the wall time between the
    scheduling.  Synchronized scheduling applies both to within the same
    GfxTimeLine and across separate GfxTimeLine's within the same process.

    \endlist

    Currently easing functions are not supported.
*/


/*!
    Construct a new GfxTimeLine with the specified \a parent.
*/
GfxTimeLine::GfxTimeLine(QObject *parent)
: QObject(parent)
{
    d = new GfxTimeLinePrivate(this);
}

/*!
    Destroys the time line.  Any inprogress animations are canceled, but not
    completed.
*/
GfxTimeLine::~GfxTimeLine()
{
    for(GfxTimeLinePrivate::Ops::Iterator iter = d->ops.begin();
            iter != d->ops.end();
            ++iter)
        iter.key()->_t = 0;

    delete d; d = 0;
}

/*!
    Pause \a gfxValue for \a time milliseconds.
*/
void GfxTimeLine::pause(GfxTimeLineObject &obj, int time)
{
    if(time <= 0) return;
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Pause, time, 0., 0., d->order++);
    d->add(obj, op);
}

/*!
    Execute the \a event.
 */
void GfxTimeLine::execute(const GfxEvent &event)
{
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Execute, 0, 0, 0., d->order++, event);
    d->add(*event.eventObject(), op);
}

/*!
    Set the \a value of \a gfxValue.
*/
void GfxTimeLine::set(GfxValue &gfxValue, qreal value)
{
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Set, 0, value, 0., d->order++);
    d->add(gfxValue, op);
}

/*!
    Decelerate \a gfxValue from the starting \a velocity to zero at the
    given \a acceleration rate.  Although the \a acceleration is technically
    a deceleration, it should always be positive.  The GfxTimeLine will ensure
    that the deceleration is in the opposite direction to the initial velocity.
*/
int GfxTimeLine::accel(GfxValue &gfxValue, qreal velocity, qreal acceleration)
{
    if((velocity > 0.0f) ==  (acceleration > 0.0f))
        acceleration = acceleration * -1.0f;

    int time = static_cast<int>(-1000 * velocity / acceleration);
    Q_ASSERT(time > 0);

    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Accel, time, velocity, acceleration, d->order++);
    d->add(gfxValue, op);

    return time;
}

/*!
    \overload

    Decelerate \a gfxValue from the starting \a velocity to zero at the
    given \a acceleration rate over a maximum distance of maxDistance.

    If necessary, GfxTimeLine will reduce the acceleration to ensure that the
    entire operation does not require a move of more than \a maxDistance.
    \a maxDistance should always be positive.
*/
int GfxTimeLine::accel(GfxValue &gfxValue, qreal velocity, qreal acceleration, qreal maxDistance)
{
    Q_ASSERT(acceleration >= 0.0f && maxDistance >= 0.0f);

    qreal maxAccel = (velocity * velocity) / (2.0f * maxDistance);
    if(maxAccel > acceleration)
        acceleration = maxAccel;

    if((velocity > 0.0f) ==  (acceleration > 0.0f))
        acceleration = acceleration * -1.0f;

    int time = static_cast<int>(-1000 * velocity / acceleration);
    Q_ASSERT(time > 0);

    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Accel, time, velocity, acceleration, d->order++);
    d->add(gfxValue, op);

    return time;
}

/*!
    Decelerate \a gfxValue from the starting \a velocity to zero over the given
    \a distance.  This is like accel(), but the GfxTimeLine calculates the exact
    deceleration to use.

    \a distance should be positive.
*/
int GfxTimeLine::accelDistance(GfxValue &gfxValue, qreal velocity, qreal distance)
{
    Q_ASSERT((distance >= 0.0f) == (velocity >= 0.0f));

    int time = static_cast<int>(1000 * (2.0f * distance) / velocity);

    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::AccelDistance, time, velocity, distance, d->order++);
    d->add(gfxValue, op);

    return time;
}

/*!
    Linearly change the \a gfxValue from its current value to the given
    \a destination value over \a time milliseconds.
*/
void GfxTimeLine::move(GfxValue &gfxValue, qreal destination, int time)
{
    if(time <= 0) return;
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Move, time, destination, 0.0f, d->order++);
    d->add(gfxValue, op);
}

/*!
    Change the \a gfxValue from its current value to the given \a destination
    value over \a time milliseconds using the \a easing curve.
 */
void GfxTimeLine::move(GfxValue &gfxValue, qreal destination, const GfxEasing &easing, int time)
{
    if(time <= 0) return;
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::Move, time, destination, 0.0f, d->order++, GfxEvent(), easing);
    d->add(gfxValue, op);
}

/*!
    Linearly change the \a gfxValue from its current value by the \a change amount
    over \a time milliseconds.
*/
void GfxTimeLine::moveBy(GfxValue &gfxValue, qreal change, int time)
{
    if(time <= 0) return;
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::MoveBy, time, change, 0.0f, d->order++);
    d->add(gfxValue, op);
}

/*!
    Change the \a gfxValue from its current value by the \a change amount over
    \a time milliseconds using the \a easing curve.
 */
void GfxTimeLine::moveBy(GfxValue &gfxValue, qreal change, const GfxEasing &easing, int time)
{
    if(time <= 0) return;
    GfxTimeLinePrivate::Op op(GfxTimeLinePrivate::Op::MoveBy, time, change, 0.0f, d->order++, GfxEvent(), easing);
    d->add(gfxValue, op);
}

/*!
    Cancel (but don't complete) all scheduled actions for \a gfxValue.
*/
void GfxTimeLine::reset(GfxValue &gfxValue)
{
    if(!gfxValue._t)
        return;
    if(gfxValue._t != this) {
        qWarning() << "GfxTimeLine: Cannot reset a GfxValue owned by another timeline.";
        return;
    }
    remove(&gfxValue);
    gfxValue._t = 0;
}

/*!
    Synchronize the end point of \a gfxValue to the endpoint of \a syncTo
    within this timeline.

    Following operations on \a gfxValue in this timeline will be scheduled after
    all the currently scheduled actions on \a syncTo are complete.  In
    psuedo-code this is equivalent to:
    \code
    GfxTimeLine::pause(gfxValue, min(0, length_of(syncTo) - length_of(gfxValue)))
    \endcode
*/
void GfxTimeLine::sync(GfxValue &gfxValue, GfxValue &syncTo)
{
    GfxTimeLinePrivate::Ops::Iterator iter = d->ops.find(&syncTo);
    if(iter == d->ops.end())
        return;
    int length = iter->length;

    iter = d->ops.find(&gfxValue);
    if(iter == d->ops.end()) {
        pause(gfxValue, length);
    } else {
        int glength = iter->length;
        pause(gfxValue, length - glength);
    }
}

/*!
    Synchronize the end point of \a gfxValue to the endpoint of the longest
    action currently scheduled in the timeline.

    In psuedo-code, this is equivalent to:
    \code
    GfxTimeLine::pause(gfxValue, length_of(timeline) - length_of(gfxValue))
    \endcode
*/
void GfxTimeLine::sync(GfxValue &gfxValue)
{
    GfxTimeLinePrivate::Ops::Iterator iter = d->ops.find(&gfxValue);
    if(iter == d->ops.end()) {
        pause(gfxValue, d->length);
    } else {
        pause(gfxValue, d->length - iter->length);
    }
}

/*!
    Synchronize all currently and future scheduled values in this timeline to
    the longest action currently scheduled.

    For example:
    \code
    value1->setValue(0.);
    value2->setValue(0.);
    value3->setValue(0.);
    GfxTimeLine tl;
    ...
    tl.move(value1, 10, 200);
    tl.move(value2, 10, 100);
    tl.sync();
    tl.move(value2, 20, 100);
    tl.move(value3, 20, 100);
    \endcode

    will result in:

    \table
    \header \o \o 0ms \o 50ms \o 100ms \o 150ms \o 200ms \o 250ms \o 300ms
    \row \o value1 \o 0 \o 2.5 \o 5.0 \o 7.5 \o 10 \o 10 \o 10
    \row \o value2 \o 0 \o 5.0 \o 10.0 \o 10.0 \o 10.0 \o 15.0 \o 20.0
    \row \o value2 \o 0 \o 0 \o 0 \o 0 \o 0 \o 10.0 \o 20.0
    \endtable
*/

void GfxTimeLine::sync()
{
    for(GfxTimeLinePrivate::Ops::Iterator iter = d->ops.begin();
            iter != d->ops.end();
            ++iter)
        pause(*iter.key(), d->length - iter->length);
    d->syncPoint = d->length;
}

/*! 
    \internal 

    Temporary hack.
 */
void GfxTimeLine::setSyncPoint(int sp)
{
    d->syncPoint = sp;
}

/*! 
    \internal 
 
    Temporary hack.
 */
int GfxTimeLine::syncPoint() const
{
    return d->syncPoint;
}

/*!
    Returns true if the timeline is active.  An active timeline is one where
    GfxValue actions are still pending.
*/
bool GfxTimeLine::isActive() const
{
    return !d->ops.isEmpty();
}

/*!
    Completes the timeline.  All queued actions are played to completion, and then discarded.  For example,
    \code
    GfxValue v(0.);
    GfxTimeLine tl;
    tl.move(v, 100., 1000.);
    // 500 ms passes
    // v.value() == 50.
    tl.complete();
    // v.value() == 100.
    \endcode
*/
void GfxTimeLine::complete()
{
    d->advance(d->length);
}

/*!
    Resets the timeline.  All queued actions are discarded and GfxValue's retain their current value. For example,
    \code
    GfxValue v(0.);
    GfxTimeLine tl;
    tl.move(v, 100., 1000.);
    // 500 ms passes
    // v.value() == 50.
    tl.clear();
    // v.value() == 50.
    \endcode
*/
void GfxTimeLine::clear()
{
    for(GfxTimeLinePrivate::Ops::ConstIterator iter = d->ops.begin(); iter != d->ops.end(); ++iter)
        iter.key()->_t = 0;
    d->ops.clear();
    d->length = 0;
    d->syncPoint = 0;
}

/*!
    If \a slowMode is true, set this (and all other timelines) into slow mode
    otherwise disable slow mode.  In slow mode, the time clock is run 50x
    slower than normal, which is useful for debugging animations.
*/
void GfxTimeLine::setSlowMode(bool slowMode)
{
    return appClock()->setSlowMode(slowMode);
}

/*!
    Returns true if slow mode is on, otherwise false.
*/
bool GfxTimeLine::slowMode()
{
    return appClock()->slowMode();
}

/*!
    Use the calling thread as the thread for the clock.  
 */
void GfxTimeLine::setClockThread()
{
    appClock()->setClockThread();
}

/*!
    Return a global lock that the clock takes out (if it has been rebased by
    a call to setClockThread()).
 */
QMutex *GfxTimeLine::lock()
{
    return appClock()->lock();
}

/*!
    Return the global clock that ticks for all timelines.
 */
GfxGlobalClock *GfxTimeLine::globalClock()
{
    return appClock()->globalClock();
}

/*!
    \fn void GfxTimeLine::updated()

    Emitted each time the timeline modifies GfxValues.  Even if multiple
    GfxValues are changed, this signal is only emitted once for each clock tick.
*/

void GfxTimeLine::tick(int v)
{
    int timeChanged = v - d->prevTime;
    if(!timeChanged)
        return;
    d->prevTime = v;
    int pauseTime = d->advance(timeChanged);
    emit updated();

    // Do we need to stop the clock?
    if(d->ops.isEmpty()) {
        GfxClock::cancelClock();
        d->prevTime = 0;
        d->clockRunning = false;
        emit completed();
    } else if(pauseTime) {
        GfxClock::cancelClock();
        d->prevTime = 0;
        GfxClock::pauseFor(pauseTime);
        d->clockRunning = false;
    } else if(!GfxClock::isActive()) {
        GfxClock::cancelClock();
        d->prevTime = 0;
        d->clockRunning = true;
        GfxClock::tickFor(100000); // "infinity"
    }
}

bool operator<(const QPair<int, Update> &lhs,
               const QPair<int, Update> &rhs)
{
    return lhs.first < rhs.first;
}

int GfxTimeLinePrivate::advance(int t)
{
    int pauseTime = -1;

    // XXX - surely there is a more efficient way?
    while(t) {
        pauseTime = -1;
        // Minimal advance time
        int advanceTime = t;
        for(Ops::Iterator iter = ops.begin(); iter != ops.end(); ++iter) {
            TimeLine &tl = *iter;
            Op &op = tl.ops.first();
            int length = op.length - tl.consumedOpLength;

            if(length < advanceTime) {
                advanceTime = length;
                if(advanceTime == 0)
                    break;
            }
        }
        t -= advanceTime;

        // Process until then.  A zero length advance time will only process 
        // sets.
        QList<QPair<int, Update> > updates;

        for(Ops::Iterator iter = ops.begin(); iter != ops.end(); ) {
            GfxValue *v = static_cast<GfxValue *>(iter.key());
            TimeLine &tl = *iter;
            Q_ASSERT(!tl.ops.isEmpty());

            do {
                Op &op = tl.ops.first();
                if(advanceTime == 0 && op.length != 0)
                    continue;

                if(tl.consumedOpLength == 0 && 
                   op.type != Op::Pause && 
                   op.type != Op::Execute)
                    tl.base = v->value();

                if((tl.consumedOpLength + advanceTime) == op.length) {
                    if(op.type == Op::Execute) {
                        updates << qMakePair(op.order, Update(op.event));
                    } else {
                        bool changed = false;
                        qreal val = value(op, op.length, tl.base, &changed);
                        if(changed)
                            updates << qMakePair(op.order, Update(v, val));
                    }
                    tl.length -= qMin(advanceTime, tl.length);
                    tl.consumedOpLength = 0;
                    tl.ops.removeFirst();
                } else {
                    tl.consumedOpLength += advanceTime;
                    bool changed = false;
                    qreal val = value(op, tl.consumedOpLength, tl.base, &changed);
                    if(changed)
                        updates << qMakePair(op.order, Update(v, val));
                    tl.length -= qMin(advanceTime, tl.length);
                    break;
                }

            } while(!tl.ops.isEmpty() && advanceTime == 0 && tl.ops.first().length == 0);


            if(tl.ops.isEmpty()) {
                iter = ops.erase(iter);
                v->_t = 0;
            } else {
                if(tl.ops.first().type == Op::Pause && pauseTime != 0) {
                    int opPauseTime = tl.ops.first().length - tl.consumedOpLength;
                    if(pauseTime == -1 || opPauseTime < pauseTime)
                        pauseTime = opPauseTime;
                } else {
                    pauseTime = 0;
                }
                ++iter;
            }
        }

        length -= qMin(length, advanceTime);
        syncPoint -= advanceTime;

        qSort(updates.begin(), updates.end());
        updateQueue = &updates;
        for(int ii = 0; ii < updates.count(); ++ii) {
            const Update &v = updates.at(ii).second;
            if(v.g)
                v.g->setValue(v.v);
            else
                v.e.execute();
        }
        updateQueue = 0;
    }

    return pauseTime;
}

void GfxTimeLine::remove(GfxTimeLineObject *v)
{
    GfxTimeLinePrivate::Ops::Iterator iter = d->ops.find(v);
    Q_ASSERT(iter != d->ops.end());

    int len = iter->length;
    d->ops.erase(iter);
    if(len == d->length) {
        // We need to recalculate the length
        d->length = 0;
        for(GfxTimeLinePrivate::Ops::Iterator iter = d->ops.begin();
                iter != d->ops.end();
                ++iter) {

            if(iter->length > d->length)
                d->length = iter->length;

        }
    }
    if(d->ops.isEmpty()) {
        GfxClock::cancelClock();
        d->clockRunning = false;
    } else if(!GfxClock::isActive()) {
        GfxClock::cancelClock();
        d->prevTime = 0;
        d->clockRunning = true;
        GfxClock::tickFor(100000); // "infinity"
    }

    if(d->updateQueue) {
        for(int ii = 0; ii < d->updateQueue->count(); ++ii) {
            if(d->updateQueue->at(ii).second.g == v ||
               d->updateQueue->at(ii).second.e.eventObject() == v) {
                d->updateQueue->removeAt(ii);
                --ii;
            }
        }
    }


}

/*!
    \class GfxValue
    \inpublicgroup QtBaseModule
    \brief The GfxValue class is modified by GfxTimeLine.
*/

/*!
    \fn GfxValue::GfxValue(qreal value = 0)

    Construct a new GfxValue with an initial \a value.
*/

/*!
    \fn qreal GfxValue::value() const

    Return the current value.
*/

/*!
    \fn void GfxValue::setValue(qreal value)

    Set the current \a value.
*/

/*!
    \fn GfxTimeLine *GfxValue::timeLine() const

    If a GfxTimeLine is operating on this value, return a pointer to it,
    otherwise return null.
*/


/*!
    \class GfxValueGroup
    \inpublicgroup QtBaseModule
    \brief The GfxValueGroup class provides a container to organize groups
    of GfxValues that can be modified by GfxTimeline.
*/

/*!
    \fn GfxValueGroup::GfxValueGroup(qreal value = 0)

    Construct a new GfxValueGroup with an initial \a value.
*/

/*!
    \fn void GfxValueGroup::setValue(qreal value)

    Set the current \a value of the group.
*/

/*!
    \fn void GfxValueGroup::addGfxValue(GfxValue *gfxValue)
    
    Add \a gfxValue to the group. Updates to the value of the group will
    be applied to \a gfxValue.
*/

/*!
    \fn void GfxValueGroup::removeGfxValue(GfxValue *gfxValue)
    
    Remove \a gfxValue from the group. Updates to the value of the group will
    no longer be applied to \a gfxValue.
*/

GfxTimeLineObject::GfxTimeLineObject()
: _t(0)
{
}

GfxTimeLineObject::~GfxTimeLineObject()
{
    if(_t) {
        _t->remove(this);
        _t = 0;
    }
}

GfxEvent::GfxEvent()
: d0(0), d1(0), d2(0), d3(0)
{
}

GfxEvent::GfxEvent(const GfxEvent &o)
: d0(o.d0), d1(o.d1), d2(o.d2), d3(o.d3)
{
}

GfxEvent &GfxEvent::operator=(const GfxEvent &o)
{
    d0 = o.d0;
    d1 = o.d1;
    d2 = o.d2;
    d3 = o.d3;
    return *this;
}

void GfxEvent::execute() const
{
    d0(d1, d2);
}

GfxTimeLineObject *GfxEvent::eventObject() const
{
    return d3;
}

QGfxValue::QGfxValue(qreal v, QObject *parent)
: QObject(parent), GfxValue(v)
{
}

void QGfxValue::setValue(qreal v)
{
    if(v != value()) {
        GfxValue::setValue(v);
        emit valueChanged(v);
    }
}

