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

#include "structure.h"
#include "element.h"
#include "system.h"
#include <QXmlStreamAttributes>

Smil::Smil(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
    currentState = Active;
}

SmilHead::SmilHead(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
    currentState = Active;
    vis = true;
}

SmilBody::SmilBody(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilSeq(sys, p, n, atts)
{
}

void SmilBody::setState(State s)
{
    SmilSeq::setState(s);
    if (s == Post) {
        sys->bodyFinished();
    }
}

SmilStructureModule::SmilStructureModule() : SmilModule()
{
}

SmilElement *SmilStructureModule::beginParseElement(SmilSystem *sys, SmilElement *parent, const QString &qName, const QXmlStreamAttributes &atts)
{
    if (qName == "smil") {
        if (parent) {
            qWarning("smil element should be root");
        } else {
            return new Smil(sys, parent, qName, atts);
        }
    } else if (qName == "head") {
        return new SmilHead(sys, parent, qName, atts);
    } else if (qName == "body") {
        return new SmilBody(sys, parent, qName, atts);
    }

    return 0;
}

bool SmilStructureModule::parseAttributes(SmilSystem *, SmilElement *, const QXmlStreamAttributes &)
{
    return false;
}

void SmilStructureModule::endParseElement(SmilElement *, const QString &)
{
}

QStringList SmilStructureModule::elementNames() const
{
    QStringList l;
    l.append("smil");
    l.append("head");
    l.append("body");
    return l;
}

