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

#include "element.h"
#include "module.h"
#include "system.h"
#include <QDebug>
#include <QXmlStreamAttributes>

Duration Duration::operator*(const Duration &d)
{
    Duration r;
    if (isUnresolved() || d.isUnresolved()) {
        r.setUnresolved(true);
        r.setIndefinite(false);
    } else if (isIndefinite()) {
        if (d.isIndefinite()) {
            r.setIndefinite(true);
        } else {
            if (d.duration() != 0) {
                r.setIndefinite(true);
            } else {
                r.setDuration(0);
            }
        }
    } else {
        if (!d.isIndefinite()) {
            r.setDuration(duration() * d.duration());
            r.setIndefinite(false);
        } else {
            if (duration() != 0) {
                r.setIndefinite(true);
            } else {
                r.setDuration(0);
            }
        }
    }

    return r;
}

Duration Duration::operator+(const Duration &d)
{
    Duration r;
    if (isUnresolved() || d.isUnresolved()) {
        r.setUnresolved(true);
        r.setIndefinite(false);
    } else if (isIndefinite() || d.isIndefinite()) {
        r.setIndefinite(true);
    } else {
        r.setDuration(duration() + d.duration());
    }

    return r;
}

Duration Duration::operator-(const Duration &d)
{
    Duration r;
    if (isUnresolved() || d.isUnresolved()) {
        r.setUnresolved(true);
    } else if (isIndefinite() || d.isIndefinite()) {
        r.setIndefinite(true);
    } else {
        r.setDuration(duration() - d.duration());
    }

    return r;
}

bool Duration::operator!=(const Duration &d)
{
    return unresolved != d.unresolved
            || indefinite != d.indefinite
            || val != d.val;
}

bool operator<(const Duration &d1, const Duration &d2)
{
    if (d1.isValue()) {
        if (d2.isValue())
            return d1.duration() < d2.duration();
    } else if (d2.isValue()) {
        return false;
    } else if (d1.isIndefinite()) {
        if (!d2.isUnresolved())
            return false;
    } else {
        return false;
    }

    return true;
}

bool operator<=(const Duration &d1, const Duration &d2)
{
    if (d1.isValue()) {
        if (d2.isValue())
            return d1.duration() <= d2.duration();
    } else if (d2.isValue()) {
        return false;
    } else if (d1.isUnresolved() && !d2.isUnresolved()) {
        return false;
    }

    return true;
}

bool operator>(const Duration &d1, const Duration &d2)
{
    if (d1.isUnresolved() && !d2.isUnresolved())
        return true;

    if (d1.isIndefinite() && d2.isValue())
        return true;

    if (d1.isValue() && d2.isValue())
        return d1.duration() > d2.duration();

    return false;
}

QDebug& operator <<(QDebug& out, const Duration& d)
{
    out << d.duration();
    return out;
}

Duration min(const Duration &d1, const Duration &d2)
{
    Duration r;

    if (d1.isValue() && d1.duration() == 0
        || d2.isValue() && d2.duration() == 0) {
        r.setIndefinite(false);
    } else if (d1.isValue()) {
        if (d2.isValue()) {
            r.setDuration(qMin(d1.duration(), d2.duration()));
        } else {
            r.setDuration(d1.duration());
        }
    } else if (d2.isValue()) {
        r.setDuration(d2.duration());
    } else {
        r.setIndefinite(true);
    }

    return r;
}

Duration max(const Duration &d1, const Duration &d2)
{
    Duration r;
    if (d1.isValue() && d2.isValue()) {
        r.setDuration(qMax(d1.duration(), d2.duration()));
    } else if (d1.isUnresolved() || d2.isUnresolved()) {
        r.setUnresolved(true);
        r.setIndefinite(false);
    }

    return r;
}

SmilEvent::~SmilEvent()
{
}

SmilElement::SmilElement(SmilSystem *s, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : sys(s), prnt(p), ename(n), currentState(Idle), vis(true)
{
    eid = atts.value("id").toString();
}

SmilElement::~SmilElement()
{
    QList<SmilModuleAttribute*>::Iterator mit;
    for (mit = attributes.begin(); mit != attributes.end(); ++mit) {
        delete *mit;
    }

    SmilElementList::Iterator eit;
    for (eit = chn.begin(); eit != chn.end(); ++eit) {
        delete *eit;
    }
}

void SmilElement::addChild(SmilElement *e)
{
    chn.append(e);
}

SmilElement *SmilElement::findChild(const QString &id, const QString &nam, bool recurse) const
{
    SmilElement *e = 0;
    SmilElementList::ConstIterator it;
    for (it = chn.begin(); it != chn.end(); ++it) {
        SmilElement *ce = *it;
        if (recurse && ce->chn.count()) {
            e = ce->findChild(id, nam, recurse);
            if (e)
                break;
        }
        if (!id.isEmpty() && ce->id() != id)
            continue;
        if (!nam.isEmpty() && ce->name() == nam)
            return ce;
    }

    return e;
}

SmilModuleAttribute *SmilElement::moduleAttribute(const QString &name) const
{
    QList<SmilModuleAttribute*>::ConstIterator it;
    for (it = attributes.begin(); it != attributes.end(); ++it) {
        if ((*it)->name() == name)
            return *it;
    }

    return 0;
}

void SmilElement::addModuleAttribute(SmilModuleAttribute *m)
{
    attributes.append(m);
}

Duration SmilElement::implicitDuration()
{
    Duration d;
    d.setUnresolved(true);
    return d;
}

void SmilElement::setRect(const QRect &rr)
{
    r = rr;
}

void SmilElement::setBackgroundColor(const QColor &c)
{
    bg= c;
}

void SmilElement::setData(const QByteArray &, const QString &)
{
    if (isVisible())
        sys->update(rect());
}

void SmilElement::addCharacters(const QString &/*ch*/)
{
}

void SmilElement::seek(int /*ms*/)
{
}

void SmilElement::reset()
{
    SmilElementList::Iterator it;
    for (it = chn.begin(); it != chn.end(); ++it) {
        (*it)->reset();
    }

    QList<SmilModuleAttribute*>::ConstIterator mit;
    for (mit = attributes.begin(); mit != attributes.end(); ++mit) {
        (*mit)->reset();
    }
    setState(Idle);
}

void SmilElement::process()
{
    SmilElementList::Iterator it;
    for (it = chn.begin(); it != chn.end(); ++it) {
        (*it)->process();
    }
    processAttributes();
}


void SmilElement::paint(QPainter *p)
{
    QList<SmilModuleAttribute*>::ConstIterator mit;
    for (mit = attributes.begin(); mit != attributes.end(); ++mit) {
        (*mit)->paint(p);
    }
}

void SmilElement::event(SmilElement *e, SmilEvent *ev)
{
    QList<SmilModuleAttribute*>::ConstIterator mit;
    for (mit = attributes.begin(); mit != attributes.end(); ++mit) {
        (*mit)->event(e, ev);
    }
}

void SmilElement::setState(State s)
{
    currentState = s;
}

void SmilElement::setCurrentBegin(const Duration &begin)
{
    currentBegin = begin;
    SmilEvent e(SmilEvent::Begin);
    sendEvent(e);
}

void SmilElement::setCurrentEnd(const Duration &end)
{
    currentEnd = end;
    SmilEvent e(SmilEvent::End);
    sendEvent(e);
}

void SmilElement::addListener(SmilElement *e)
{
    listeners.append(e);
}

QColor SmilElement::parseColor(const QString &name)
{
    QColor col;
    if (!name.isEmpty() && name[0] == '#') {
        col.setNamedColor(name);
    } else if (name != "transparent") {
        // ### Support CSS system colors:
        // http://www.w3.org/TR/REC-CSS2/ui.html#system-colors
    }

    return col;
}

void SmilElement::sendEvent(SmilEvent &e)
{
    event(this, &e);    // we always send events to ourself
    SmilElementList::ConstIterator it;
    for (it = listeners.begin(); it != listeners.end(); ++it) {
        (*it)->event(this, &e);
    }
}

void SmilElement::processAttributes()
{
    QList<SmilModuleAttribute*>::ConstIterator mit;
    for (mit = attributes.begin(); mit != attributes.end(); ++mit) {
        (*mit)->process();
    }
}

