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

#ifndef SMILTIMING_H
#define SMILTIMING_H

#include <element.h>
#include <module.h>

#include <qdatetime.h>

class SmilTimingAttribute;
class SmilTimingModulePrivate;

class SmilTimingModule : public SmilModule
{
public:
    SmilTimingModule();
    virtual ~SmilTimingModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlStreamAttributes &atts);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlStreamAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elementNames() const;
    virtual QStringList attributeNames() const;

    void addNode(SmilTimingAttribute *);

    const QDateTime &startTime() const;



private:
    SmilTimingModulePrivate *d;
};

class TimingValue
{
public:
    TimingValue();
    explicit TimingValue(const QString &value);

    void parse(const QString &);

    enum Type { Unspecified, Offset, SyncBase, Event, Repeat, AccessKey,
                MediaMarker, WallclockSync, Indefinite, Media };
    void setType(Type type) { t = type; }
    Type type() const { return t; }

    void setOffset(int v) { tval = v; }
    int offset() const { return tval; }

    void setElementId(const QString &eid) { id = eid; }
    QString elementId() const { return id; }

    void setElement(SmilElement *e) { elmnt = e; }
    SmilElement *element() const { return elmnt; }

    void setValue(const QString &v) { val = v; }
    QString value() const { return val; }

    QDateTime wallClock() const { return wallclock; }

    static int parseDecimal(const QString &val);

    Duration duration() const;
    Duration resolvedTime() const { return resolved; }
    void setResolvedTime(Duration d) { resolved = d; }

private:
    int parseClock(const QString &);
    QDateTime parseWallClock(const QString &);

private:
    Type t;
    int tval;
    QString id;
    QString val;
    QDateTime wallclock;
    SmilElement *elmnt;
    Duration resolved;
};

bool operator<(const TimingValue &tv1, const TimingValue &tv2);
bool operator<=(const TimingValue &tv1, const TimingValue &tv2);
bool operator>(const TimingValue &tv1, const TimingValue &tv2);

class EndSync
{
public:
    EndSync() : t(Unspecified) {}

    enum Type { Unspecified, First, Last, All, Media, SmilElement };

    Type type() const { return t; }
    const QString &elementId() const { return eid; }

    void parse(const QString &);

private:
    Type t;
    QString eid;
};

class SmilTimingAttribute : public SmilModuleAttribute
{
public:
    SmilTimingAttribute(SmilElement *e, const QXmlStreamAttributes &atts);
    ~SmilTimingAttribute();

    void process();
    void reset();
    void event(SmilElement *e, SmilEvent *ev);
    void setState(SmilElement::State state);

    void updateImplicitTiming();
    bool isImplicitTiming() const;

    void timerEvent(int id);

    QString name() const;

    enum Fill { RemoveFill, FreezeFill, HoldFill, TransitionFill,
                AutoFill, InheritFill, DefaultFill };

    // attributes
    QList<TimingValue> beginList;
    TimingValue dur;
    QList<TimingValue> endList;
    TimingValue minTime;
    TimingValue maxTime;
    TimingValue repeatDur;
    int repeatCount;    // *1000
    EndSync endSync;
    Fill fill;
    Fill fillDefault;

    // Current state
    QList<TimingValue> instanceBeginList;
    QList<TimingValue> instanceEndList;
    QDateTime activeStartTime;
    int currentIteration;
    QTime startTime;

    SmilTimingAttribute *parent;
    QList<SmilTimingAttribute*> chn;

    void resolveTime(TimingValue &tv);
    int scheduleTimer(Duration d);
    void clearTimer(int id);
    void clearTimers();

private:
    void calcCurrentInstance();
    Duration calcIAD();
    void buildInstanceTimeLists();
    void calcCurrentBegin();
    void calcCurrentEnd();
    void calcSimpleDuration();
    void calcActiveDuration();
    TimingValue *findBeginTimeValue(TimingValue::Type type, const QString &value, SmilElement *e);
    TimingValue *findEndTimeValue(TimingValue::Type type, const QString &value, SmilElement *e);
    void applyFill();
    Fill parseFill(const QString &f, Fill def);
    int stateTimerId;
    int simpleTimerId;
};

class SmilPar : public SmilElement
{
public:
    SmilPar(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void process();
    void setState(State s);

    Duration implicitDuration();
};

class SmilSeq : public SmilElement
{
public:
    SmilSeq(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void process();
    void setState(State s);

    Duration implicitDuration();
};

#endif
