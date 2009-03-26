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

#include "timing.h"
#include "system.h"
#include <qobject.h>
#include <QTimerEvent>
#include <QMutableMapIterator>
#include <QXmlStreamAttributes>
#include <QDebug>

class TimerEventManager : public QObject
{
public:
    TimerEventManager();

    int addTimer(SmilTimingAttribute *, int ms);
    void removeAllTimers(SmilTimingAttribute *);
    void removeTimer(int id);

    void timerEvent(QTimerEvent *);

protected:
    QMap<int,SmilTimingAttribute*> timers;
};

static TimerEventManager *timerEventManager = 0;

TimerEventManager::TimerEventManager()
{
}

int TimerEventManager::addTimer(SmilTimingAttribute *ta, int ms)
{
    int id = startTimer(ms >= 0 ? ms : 0);
    timers[id] = ta;

    return id;

}

void TimerEventManager::removeAllTimers(SmilTimingAttribute *ta)
{
    QMutableMapIterator<int,SmilTimingAttribute*> it(timers);
    while (it.hasNext()) {
        it.next();
        if (it.value() == ta) {
            killTimer(it.key());
            it.remove();
        }
    }
}

void TimerEventManager::removeTimer(int id)
{
    if (timers.contains(id)) {
        timers.remove(id);
        killTimer(id);
    }
}

void TimerEventManager::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
    if (timers.contains(id)) {
        killTimer(id);
        SmilTimingAttribute *ta = timers.value(id);
        timers.remove(id);
        ta->timerEvent(id);
    }
}

SmilPar::SmilPar(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
}

void SmilPar::process()
{
    SmilElement::process();
}

void SmilPar::setState(State s)
{
    SmilElement::setState(s);
    if (s == Active) {
        SmilElementList::ConstIterator it;
        for (it = chn.begin(); it != chn.end(); ++it) {
            SmilTimingAttribute *ta = (SmilTimingAttribute*)(*it)->moduleAttribute("Timing");
            if (ta)
                ta->setState(Startup);
            else
                (*it)->setState(Startup);
        }
    } else if (s == End) {
        SmilElementList::ConstIterator it;
        for (it = chn.begin(); it != chn.end(); ++it) {
            if ((*it)->state() >= SmilElement::Startup
                && (*it)->state() <= SmilElement::Active) {
                SmilTimingAttribute *ta = (SmilTimingAttribute*)(*it)->moduleAttribute("Timing");
                if (ta)
                    ta->setState(SmilElement::End);
                else
                    (*it)->setState(SmilElement::End);
            }
        }
    }
}

Duration SmilPar::implicitDuration()
{
    Duration d;
    SmilTimingAttribute *ta = (SmilTimingAttribute*)moduleAttribute("Timing");
    switch (ta->endSync.type()) {
        case EndSync::First:
            {
                if (chn.count()) {
                    SmilElementList::ConstIterator it = chn.begin();
                    SmilElement *last = *it;
                    Duration minDur = last->currentEnd;
                    ++it;
                    while (it != chn.end() && minDur.isValue()) {
                        SmilElement *e = *it;
                        Duration dur = e->currentEnd;
                        if (dur < minDur) {
                            minDur = dur;
                            last = e;
                        }
                        ++it;
                    }
                    d = minDur;
                } else {
                    d.setDuration(0);
                }
            }
            break;
        case EndSync::Last:
            {
                if (chn.count()) {
                    SmilElementList::ConstIterator it = chn.begin();
                    SmilElement *last = *it;
                    Duration maxDur = last->currentEnd;
                    ++it;
                    while (it != chn.end() && maxDur.isValue()) {
                        SmilElement *e = *it;
                        Duration dur = e->currentEnd;
                        if (maxDur < dur) {
                            maxDur = dur;
                            last = e;
                        }
                        ++it;
                    }
                    d = maxDur;
                } else {
                    d.setDuration(0);
                }
            }
            break;
        case EndSync::All:
            // ### implement
            break;
        case EndSync::Media:
            // ### unsupported
            break;
        case EndSync::SmilElement:
            {
                SmilElement *e = findChild(ta->endSync.elementId());
                if (e)
                    d = e->currentEnd;
            }
            break;
        default:
            d.setIndefinite(true);
            break;
    }
    return d;
}

SmilSeq::SmilSeq(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
}

void SmilSeq::process()
{
    SmilElement::process();
    if (children().count()) {
        SmilElement *e = children().last();
        TimingValue tv;
        tv.setType(TimingValue::SyncBase);
        tv.setElement(e);
        tv.setValue("end");
        SmilTimingAttribute *ta = (SmilTimingAttribute*)moduleAttribute("Timing");
        if (ta) {
            ta->endList.append(tv);
            e->addListener(this);    // tell me when child end is resolved
        }
    }
}

Duration SmilSeq::implicitDuration()
{
    Duration d(0);
    if (chn.count())
        d = chn.last()->currentEnd;
    return d;
}

void SmilSeq::setState(State s)
{
    SmilElement::setState(s);
    if (s == Active) {
        SmilElementList::ConstIterator it;
        for (it = chn.begin(); it != chn.end(); ++it) {
            SmilTimingAttribute *ta = (SmilTimingAttribute*)(*it)->moduleAttribute("Timing");
            if (ta)
                ta->setState(Startup);
            else
                (*it)->setState(Startup);
        }
    } else if (s == End) {
        SmilElementList::ConstIterator it;
        for (it = chn.begin(); it != chn.end(); ++it) {
            if ((*it)->state() >= SmilElement::Startup
                && (*it)->state() <= SmilElement::Active) {
                SmilTimingAttribute *ta = (SmilTimingAttribute*)(*it)->moduleAttribute("Timing");
                if (ta)
                    ta->setState(SmilElement::End);
                else
                    (*it)->setState(SmilElement::End);
            }
        }
    }
}

class SmilTimingModulePrivate
{
public:
    SmilTimingModulePrivate();

    SmilTimingAttribute *root;
    SmilTimingAttribute *current;
};

SmilTimingModulePrivate::SmilTimingModulePrivate()
    : root(0), current(0)
{
}

SmilTimingModule::SmilTimingModule()
    : SmilModule()
{
    d = new SmilTimingModulePrivate;
    if (!timerEventManager)
        timerEventManager = new TimerEventManager;
}

SmilTimingModule::~SmilTimingModule()
{
    delete d;
}

SmilElement *SmilTimingModule::beginParseElement(SmilSystem *sys, SmilElement *e, const QString &qName, const QXmlStreamAttributes &atts)
{
    if (qName == "par") {
        return new SmilPar(sys, e, qName, atts);
    } else if (qName == "seq") {
        return new SmilSeq(sys, e, qName, atts);
    }

    return 0;
}

bool SmilTimingModule::parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlStreamAttributes &atts)
{
    if (e->name() == "body" || e->name() == "par" || e->name() == "seq"
        || sys->module("Media")->elementNames().contains(e->name())) {
        SmilTimingAttribute *tn = new SmilTimingAttribute(e, atts);
        addNode(tn);
        return true;
    }
    for (int i = 0; i < atts.count(); i++) {
        if (attributeNames().contains(atts.at(i).name().toString())) {
            SmilTimingAttribute *tn = new SmilTimingAttribute(e, atts);
            addNode(tn);
            return true;
        }
    }

    return false;
}

void SmilTimingModule::endParseElement(SmilElement *e, const QString &/*qName*/)
{
    if (d->current && e == d->current->element)
        d->current = d->current->parent;
}

QStringList SmilTimingModule::elementNames() const
{
    QStringList l;
    l.append("par");
    l.append("seq");
    return l;
}

QStringList SmilTimingModule::attributeNames() const
{
    QStringList l;
    l.append("begin");
    l.append("dur");
    l.append("end");
    l.append("repeatDur");
    l.append("repeatCount");
    l.append("min");
    l.append("max");
    l.append("fill");
    l.append("fillDefault");
    l.append("endsync");
    return l;
}

void SmilTimingModule::addNode(SmilTimingAttribute *n)
{
    if (d->current)
        d->current->chn.append(n);
    else
        d->current = n;
}

SmilTimingAttribute::SmilTimingAttribute(SmilElement *e, const QXmlStreamAttributes &atts)
    :
    SmilModuleAttribute(e, atts),
    repeatCount(-1),
    parent(0),
    stateTimerId(0),
    simpleTimerId(0)
{
    QString val = atts.value("begin").toString();
    if (!val.isEmpty()) {
        QStringList list = val.split(';');
        for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
            TimingValue begin(val);
            beginList.append(begin);
        }
    }
    val = atts.value("dur").toString();
    if (!val.isEmpty())
        dur.parse(val);
    val = atts.value("end").toString();
    if (!val.isEmpty()) {
        QStringList list = val.split(';');
        for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
            TimingValue end(val);
            endList.append(end);
        }
    }
    val = atts.value("repeatDur").toString();
    if (!val.isEmpty())
        repeatDur.parse(val);
    val = atts.value("repeatCount").toString();
    if (!val.isEmpty())
        repeatCount = TimingValue::parseDecimal(val);
    val = atts.value("min").toString();
    if (val.isEmpty())
        val = "0s";
    minTime.parse(val);
    val = atts.value("max").toString();
    if (val.isEmpty())
        val = "indefinite";
    maxTime.parse(val);
    val == atts.value("fill").toString();
    fill = parseFill(val, DefaultFill);
    val == atts.value("fillDefault").toString();
    fillDefault = parseFill(val, InheritFill);
    if (e->name() == "par") {
        // handle endsync
        if (dur.type() == TimingValue::Unspecified) {
            val = atts.value("endsync").toString();
            if (val.isEmpty())
                val = "last"; // default
            bool haveRptDur = repeatDur.type() != TimingValue::Unspecified;
            bool haveRptCnt = repeatCount != -1;
            if (endList.count()) {
                if (haveRptDur || haveRptCnt) {
                    endSync.parse(val);
                }
            } else {
                endSync.parse(val);
            }
        }
    } else if (e->name() == "body") {
        if (!beginList.count())
            beginList.append(TimingValue("0"));
    }

    SmilElement *pelement = element->parent();
    if (pelement) {
        if (pelement->name() == "par") {
            // default begin for children of par
            if (!beginList.count())
                beginList.append(TimingValue("0"));
        } else if (pelement->name() == "seq" || pelement->name() == "body") {
            // default begin for children of seq
            if (pelement->children().count() == 1) {
                if (!beginList.count())
                    beginList.append(TimingValue("0"));
            } else if (pelement->children().count() > 1) {
                SmilElement *lelement = pelement->children().at(pelement->children().count()-2);
                TimingValue tv;
                tv.setType(TimingValue::SyncBase);
                tv.setElement(lelement);
                tv.setValue("end");
                if (beginList.count())
                    tv.setOffset(beginList.first().offset());
                beginList.clear();  // because there can only be an offset
                beginList.append(tv);
                lelement->addListener(element); // tell me when sibling end is resolved
            }
        }
    }
}

SmilTimingAttribute::~SmilTimingAttribute()
{
}

void SmilTimingAttribute::process()
{
    QList<TimingValue>::ConstIterator it;
    for (it = beginList.begin(); it != beginList.end(); ++it) {
        if ((*it).type() == TimingValue::SyncBase) {
            SmilElement *e = element->system()->findElement(0, (*it).elementId());
            if (e) {
                e->addListener(element);
            }
        }
    }
    buildInstanceTimeLists();
    calcCurrentBegin();
    calcCurrentEnd();
    calcCurrentInstance();
}

void SmilTimingAttribute::calcCurrentInstance()
{
    calcCurrentBegin();
    calcSimpleDuration();
    calcActiveDuration();
    calcCurrentEnd();
}

void SmilTimingAttribute::reset()
{
    // 1. remove past instance times related to events, repeat, accesskey, etc.
    // 2. reevaluate syncbase times.
    // 3. remove resolved syncbase time when common ascendant of syncbase AND
    //    dependant restarts.
    // 4. clear state
    clearTimers();
    QList<TimingValue>::Iterator it;
    for (it = instanceBeginList.begin(); it != instanceBeginList.end(); ++it)
        (*it).setResolvedTime(Duration());
    for (it = instanceEndList.begin(); it != instanceEndList.end(); ++it)
        (*it).setResolvedTime(Duration());
}

void SmilTimingAttribute::timerEvent(int id)
{
    if (id == stateTimerId) {
        stateTimerId = 0;
        int state = element->state() + 1;
        if (state > SmilElement::Post)
            state = SmilElement::Startup;
        setState((SmilElement::State)state);
    } else if (id == simpleTimerId) {
        simpleTimerId = 0;
        currentIteration++;
        int elapsed = currentIteration * element->simpleDuration.duration();
        if (elapsed < element->activeDuration.duration()) {
            simpleTimerId = scheduleTimer(element->simpleDuration.duration());
            SmilElementList::ConstIterator it;
            for (it = element->children().begin(); it != element->children().end(); ++it) {
                (*it)->reset();
                SmilTimingAttribute *ta = (SmilTimingAttribute*)(*it)->moduleAttribute("Timing");
                if (ta)
                    ta->setState(SmilElement::Startup);
                else
                    (*it)->setState(SmilElement::Startup);
            }
        }
    }
}

void SmilTimingAttribute::setState(SmilElement::State state)
{
    if (element->state() != state)
        element->setState(state);
    if (stateTimerId)
        clearTimer(stateTimerId);
    switch (state) {
        case SmilElement::Startup:
            currentIteration = 0;
            stateTimerId = scheduleTimer(0);
            startTime.start();
            break;
        case SmilElement::Waiting:
            if (element->currentBegin.isValue()) {
                Duration dur(element->currentBegin.duration()
                                - startTime.elapsed());
                stateTimerId = scheduleTimer(dur);
            }
            break;
        case SmilElement::Active:
            if (element->activeDuration.isValue()
                || element->activeDuration.isIndefinite()) {
                element->setVisible(true);
                if (element->simpleDuration.isValue()
                    && element->simpleDuration.duration()
                    && element->simpleDuration < element->activeDuration) {
                    simpleTimerId = scheduleTimer(element->simpleDuration.duration());
                }
                if (element->currentEnd.isValue()) {
                    Duration dur(element->currentEnd.duration()
                                    - startTime.elapsed());
                    stateTimerId = scheduleTimer(dur);
                }
            }
            break;
        case SmilElement::End:
            // end of my interval - compute next interval, notify dependants.
            stateTimerId = scheduleTimer(0);
            break;
        case SmilElement::Post:
            // perform fill
            clearTimers();
            applyFill();
            break;
        default:
            break;
    }
}

void SmilTimingAttribute::event(SmilElement *e, SmilEvent *ev)
{
    if (e == element) {
        if (element->state() >= SmilElement::Startup
                && element->state() <= SmilElement::End) {
            // need to reevaluate our own timing.
            setState(element->state());
        }
    } else {
        switch (ev->type()) {
            case SmilEvent::End:
                {
                    TimingValue *tv = findBeginTimeValue(TimingValue::SyncBase, "end", e);
                    if (tv && !e->currentEnd.isUnresolved()) {
                        TimingValue itv = *tv;
                        resolveTime(itv);
                        instanceBeginList.append(itv);
                        calcCurrentInstance();
                    }
                    tv = findEndTimeValue(TimingValue::SyncBase, "end", e);
                    if (tv && !e->currentEnd.isUnresolved()) {
                        TimingValue itv = *tv;
                        resolveTime(itv);
                        instanceEndList.append(itv);
                        calcCurrentInstance();
                    }
                }
                break;
            case SmilEvent::Begin:
                {
                    TimingValue *tv = findBeginTimeValue(TimingValue::SyncBase, "begin", e);
                    if (tv && !e->currentBegin.isUnresolved()) {
                        TimingValue itv = *tv;
                        resolveTime(itv);
                        instanceBeginList.append(itv);
                        calcCurrentInstance();
                    }
                    tv = findEndTimeValue(TimingValue::SyncBase, "begin", e);
                    if (tv && !e->currentEnd.isUnresolved()) {
                        TimingValue itv = *tv;
                        resolveTime(itv);
                        instanceEndList.append(itv);
                        calcCurrentInstance();
                    }
                }
                break;
            default:
                break;
        }
    }
}

void SmilTimingAttribute::calcSimpleDuration()
{
    element->simpleDuration = Duration();
    Duration implicit = element->implicitDuration();
    if (dur.type() == TimingValue::Unspecified
        && repeatDur.type() == TimingValue::Unspecified
        && repeatCount < 0 && endList.count()) {
        element->simpleDuration.setIndefinite(true);
    } else if (dur.type() == TimingValue::Offset) {
        element->simpleDuration.setDuration(dur.offset());
    } else if (dur.type() == TimingValue::Indefinite) {
        element->simpleDuration.setIndefinite(true);
    } else if (dur.type() == TimingValue::Unspecified
                && !implicit.isUnresolved()) {
        element->simpleDuration = implicit;
    } else if (dur.type() == TimingValue::Unspecified
                && implicit.isUnresolved()) {
        element->simpleDuration.setIndefinite(false);
        element->simpleDuration.setUnresolved(true);
    } else if (dur.type() == TimingValue::Media) {
        element->simpleDuration.setDuration(implicit.duration());
    }
}

void SmilTimingAttribute::calcActiveDuration()
{
    Duration pad;
    Duration endDur = element->currentEnd;
    Duration beginDur = element->currentBegin;
    element->activeDuration = Duration();
    if (!endDur.isUnresolved()
        && element->simpleDuration.isIndefinite()) {
        if (endDur.isValue()) {
            pad = endDur - beginDur;
        } else if (endDur.isIndefinite()) {
            pad.setIndefinite(true);
        } else if (endDur.isUnresolved()) {
            pad.setUnresolved(true);
        }
    } else if (!endDur.isValue()) {
        pad = calcIAD();
    } else {
        pad = min(calcIAD(), endDur - beginDur);
    }

    element->activeDuration = min(maxTime.duration(), max(minTime.duration(), pad));
}

void SmilTimingAttribute::buildInstanceTimeLists()
{
    QList<TimingValue>::ConstIterator it;
    for (it = beginList.begin(); it != beginList.end(); ++it) {
        const TimingValue &tv = *it;
        if (tv.type() == TimingValue::Offset
            || tv.type() == TimingValue::SyncBase) {
            instanceBeginList.append(tv);
        }
    }
    for (it = endList.begin(); it != endList.end(); ++it) {
        const TimingValue &tv = *it;
        if (tv.type() == TimingValue::Offset
            || tv.type() == TimingValue::SyncBase
            || tv.type() == TimingValue::Indefinite) {
            instanceEndList.append(tv);
        }
    }
}

void SmilTimingAttribute::calcCurrentBegin()
{
    Duration begin;
    begin.setUnresolved(true);

    QList<TimingValue>::Iterator it;
    for (it = instanceBeginList.begin(); it != instanceBeginList.end(); ++it)
        resolveTime((*it));
    qSort(instanceBeginList);
    for (it = instanceBeginList.begin(); it != instanceBeginList.end(); ++it) {
        Duration d = (*it).resolvedTime();
        if (d.isValue()) {
            begin = d;
            break;
        }
    }

    element->setCurrentBegin(begin);
}

void SmilTimingAttribute::calcCurrentEnd()
{
    Duration end;
    end.setUnresolved(true);

    if (element->currentBegin.isValue() && element->activeDuration.isValue()) {
        TimingValue t;
        t.setType(TimingValue::SyncBase);
        t.setElement(element);
        t.setValue("begin");
        t.setOffset(element->activeDuration.duration());
        t.setResolvedTime(element->currentBegin + element->activeDuration);
        instanceEndList.append(t);
    }

    QList<TimingValue>::Iterator it;
    for (it = instanceEndList.begin(); it != instanceEndList.end(); ++it)
        resolveTime((*it));
    qSort(instanceEndList);
    for (it = instanceEndList.begin(); it != instanceEndList.end(); ++it) {
        Duration d = (*it).resolvedTime();
        if (d.isValue() || d.isIndefinite()) {
            end = d;
            break;
        }
    }
    element->setCurrentEnd(end);
}

Duration SmilTimingAttribute::calcIAD()
{
    Duration p0 = element->simpleDuration;
    Duration p1;
    if (repeatCount < 0) {
        p1.setIndefinite(true);
    } else if (element->simpleDuration.isValue()) {
        p1.setDuration(element->simpleDuration.duration() * repeatCount / 1000);
    }

    Duration p2 = repeatDur.duration();
    if (repeatDur.type() == TimingValue::Unspecified)
        p2.setIndefinite(true);

    Duration iad;
    if (p0.isValue() && p0.duration() == 0) {
        iad.setDuration(0);
    } else if (repeatDur.type() == TimingValue::Unspecified
             && repeatCount < 0) {
        iad = p0;
    } else {
        Duration ind;
        ind.setIndefinite(true);
        iad = min(min(p1, p2), ind);
    }

    return iad;
}

TimingValue *SmilTimingAttribute::findBeginTimeValue(TimingValue::Type type,
                                            const QString &value, SmilElement *e)
{
    TimingValue *tv = 0;
    QList<TimingValue>::Iterator it;
    for (it = beginList.begin(); it != beginList.end(); ++it) {
        TimingValue &ctv = (*it);
        if (ctv.type() == type && ctv.value() == value
            && (ctv.element() == e || ctv.elementId() == e->id())) {
            tv = &ctv;
        }
    }

    return tv;
}

TimingValue *SmilTimingAttribute::findEndTimeValue(TimingValue::Type type,
                                            const QString &value, SmilElement *e)
{
    TimingValue *tv = 0;
    QList<TimingValue>::Iterator it;
    for (it = endList.begin(); it != endList.end(); ++it) {
        TimingValue &ctv = (*it);
        if (ctv.type() == type && ctv.value() == value
            && (ctv.element() == e || ctv.elementId() == e->id())) {
            tv = &ctv;
        }
    }

    return tv;
}

void SmilTimingAttribute::resolveTime(TimingValue &tv)
{
    if (!tv.resolvedTime().isUnresolved())
        return;
    switch (tv.type()) {
        case TimingValue::Offset:
            tv.setResolvedTime(tv.offset());
            break;
        case TimingValue::SyncBase:
            if (!tv.element())
                tv.setElement(element->system()->findElement(0, tv.elementId()));
            if (tv.element()) {
                Duration d;
                if (tv.value() == "end")
                    d = tv.element()->currentEnd + tv.offset();
                else
                    d = tv.element()->currentBegin + tv.offset();
                tv.setResolvedTime(d);
            }
            break;
        default:
            break;
    }
}

QString SmilTimingAttribute::name() const
{
    return "Timing";
}

int SmilTimingAttribute::scheduleTimer(Duration d)
{
    if (d.isValue()) {
        return timerEventManager->addTimer(this, d.duration());
    }

    qWarning("Cannot schedule Undefined or Indefinite duration");
    return -1;
}

void SmilTimingAttribute::clearTimer(int id)
{
    timerEventManager->removeTimer(id);
    if (id == stateTimerId)
        stateTimerId = 0;
    else if (id == simpleTimerId)
        simpleTimerId = 0;
}

void SmilTimingAttribute::clearTimers()
{
    timerEventManager->removeAllTimers(this);
    stateTimerId = 0;
    simpleTimerId = 0;
}

void SmilTimingAttribute::applyFill()
{
    // ### Think about using clipping to achieve this.
    Fill f = fill;

    if (fill == DefaultFill) {
        SmilTimingAttribute *ta = this;
        while (ta) {
            if (ta->fillDefault != InheritFill) {
                f = ta->fillDefault;
                break;
            }
            ta = ta->parent;
        }
        if (!ta) {
            f = AutoFill;
        }
    }

    switch (f) {
        case RemoveFill:
            element->setVisible(false);
            break;
        case AutoFill:
            if (dur.type() != TimingValue::Unspecified
                    || endList.count() != 0 || repeatCount >= 0
                    || repeatDur.type() != TimingValue::Unspecified) {
                element->setVisible(false);
            }
            break;
        default:
            // freeze
            break;
    }
}

SmilTimingAttribute::Fill SmilTimingAttribute::parseFill(const QString &val, Fill def)
{
    Fill fill = def;
    if (!val.isEmpty()) {
        if (val == "remove")
            fill = RemoveFill;
        else if (val == "freeze")
            fill = FreezeFill;
        else if (val == "hold")
            fill = HoldFill;
        else if (val == "transition")
            fill = TransitionFill;
        else if (val == "default")
            fill = DefaultFill;
        else if (val == "inherit")
            fill = InheritFill;
    }

    return fill;
}

static void resetElementTiming(SmilElement* e)
{
    e->simpleDuration = Duration();
    e->activeDuration = Duration();
    e->currentBegin = Duration();
    e->currentEnd = Duration();
}

void SmilTimingAttribute::updateImplicitTiming()
{
    resetElementTiming(element);
    reset();
    instanceEndList.clear();
    instanceBeginList.clear();
    buildInstanceTimeLists();
    calcCurrentInstance();

    if (element->currentEnd.isValue() && element->state() == SmilElement::Active) {
        Duration dur(element->currentEnd.duration() - startTime.elapsed());
        clearTimer(stateTimerId);
        stateTimerId = scheduleTimer(dur);
    }

    SmilElement* parentElement = element->parent();

    if(parentElement)
    {
        SmilTimingAttribute* ta = static_cast<SmilTimingAttribute*>(parentElement->moduleAttribute("Timing"));
        if(ta)
            ta->updateImplicitTiming();
    }
}

TimingValue::TimingValue()
    : t(Unspecified), tval(0), elmnt(0)
{
    resolved.setUnresolved(true);
}

TimingValue::TimingValue(const QString &value)
    : t(Unspecified), tval(0), elmnt(0)
{
    resolved.setUnresolved(true);
    parse(value);
}

Duration TimingValue::duration() const
{
    Duration d;
    if (type() == Unspecified || type() == Indefinite) {
        d.setIndefinite(true);
    } else if (type() == Offset) {
        d.setDuration(tval);
    } else {
        d.setUnresolved(true);
        d.setIndefinite(false);
    }

    return d;
}

void TimingValue::parse(const QString &v)
{
    QString value = v.simplified();
    if (value[0].isDigit() || value[0] == '+' || value[0] == '-') {
        t = Offset;
        tval = parseClock(value);
        resolved.setDuration(tval);
    } else if (value.startsWith("wallclock")) {
        t = WallclockSync;
        wallclock = parseWallClock(value.mid(9));
    } else if (value == "indefinite") {
        t = Indefinite;
        resolved.setIndefinite(true);
    } else if (value == "media") {
        t = Media;
    } else {
        QString tok = value;
        int sep = value.indexOf(QRegExp("[+-]"));
        if (sep >= 0) {
            tok = value.left(sep);
            tval = parseClock(value.mid(sep));
        }
        // check for event-value
        int dotSep = 0;
        do {
            dotSep = tok.indexOf('.', dotSep+1);
        } while (dotSep > 0 && tok[dotSep-1] == '\\');
        if (dotSep < 0 ) {
            t = Event;
            val = tok;
            return;
        }
        // check for syncbase-value
        sep = tok.indexOf(".begin");
        if (sep > 0 && sep == (int)tok.length()-6) {
            t = SyncBase;
            id = tok.left(sep);
            val = tok.mid(sep+1);
            return;
        }
        sep = tok.indexOf(".end");
        if (sep > 0 && sep == (int)tok.length()-4) {
            t = SyncBase;
            id = tok.left(sep);
            val = tok.mid(sep+1);
            return;
        }
        // check for a media-marker-value
        sep = tok.indexOf(".marker(");
        if (sep > 0) {
            t = MediaMarker;
            val = tok.mid(sep+8);
            val.truncate(val.length()-1);
            return;
        }
        // Must be an event-value with specified eventbase-element
        t = Event;
        id = tok.left(dotSep);
        val = tok.mid(dotSep+1);
    }
}

int TimingValue::parseClock(const QString &value)
{
    int ms = 0;
    if (value.contains(':')) {
        // clock value
        QStringList list = value.split(':');
        int hour = 0;
        int idx = 0;
        if (list.count() == 2) // full clock value
            hour = list[idx++].toInt();
        int minute = list[idx++].toInt();
        ms = parseDecimal(list[idx++]);
        ms += minute * 60000;
        ms += hour * 60000 * 60;
    } else {
        // timecount
        int length = value.length();
        if (value[length-1] == 'h') {
            // ### parse hours
        } else if (length > 3 && value.right(3) == "min") {
            // parse minutes
            QString smin = value.left(length-3);
            int sep = smin.indexOf('.');
            if (sep) {
                QString frac = smin.mid(sep+1) + "000";
                frac.truncate(3);
                ms = frac.toInt() * 60;
                smin.truncate(sep);
            }
            ms += smin.toInt() * 60000;
        } else if (length > 2 && value.right(2) == "ms") {
            ms = value.left(length-2).toInt();
        } else if (value[length-1] == 's') {
            ms = parseDecimal(value.left(length-1));
        } else if (value[length-1].isDigit()) {
            ms = parseDecimal(value);
        }
    }

    return ms;
}

int TimingValue::parseDecimal(const QString &val)
{
    int ms = 0;
    bool negative = false;

    QString ssec = val;

    if (ssec[0] == '-') {
        negative = true;
        ssec = ssec.mid(1);
    }
    if (ssec[0] == '+')
        ssec = ssec.mid(1);


    if (ssec[0].isDigit()) {
        QString sfrac;
        int sep = ssec.indexOf('.');
        if (sep >= 0) {
            sfrac = (ssec+"000").mid(sep+1, 3);
            ssec.truncate(sep);
        }

        ms = ssec.toInt() * 1000;
        if (!sfrac.isEmpty())
            ms += sfrac.toInt();
    }

    if (negative)
        ms = -ms;

    return ms;
}

QDateTime TimingValue::parseWallClock(const QString &/*value*/)
{
    // ### implement
    return QDateTime();
}

bool operator<(const TimingValue &tv1, const TimingValue &tv2)
{
    return tv1.resolvedTime() < tv2.resolvedTime();
}

bool operator<=(const TimingValue &tv1, const TimingValue &tv2)
{
    return tv1.resolvedTime() <= tv2.resolvedTime();
}

bool operator>(const TimingValue &tv1, const TimingValue &tv2)
{
    return tv1.resolvedTime() > tv2.resolvedTime();
}

void EndSync::parse(const QString &val)
{
    if (val == "first") {
        t = First;
    } else if (val == "last") {
        t = Last;
    } else if (val == "all") {
        t = All;
    } else if (val == "media") {
        t = Media;
    } else {
        t = SmilElement;
        eid = val;
    }
}

