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

#include "content.h"
#include <QXmlStreamAttributes>

SmilPrefetch::SmilPrefetch(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
}

SmilSwitch::SmilSwitch(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
}

SmilContentModule::SmilContentModule()
    : SmilModule()
{
}

SmilContentModule::~SmilContentModule()
{
}

SmilElement *SmilContentModule::beginParseElement(SmilSystem *sys, SmilElement *e, const QString &qName, const QXmlStreamAttributes &atts)
{
    if (qName == "switch") {
        return new SmilSwitch(sys, e, qName, atts);
    } else if (qName == "prefetch") {
        return new SmilPrefetch(sys, e, qName, atts);
    }

    return 0;
}

bool SmilContentModule::parseAttributes(SmilSystem *, SmilElement *, const QXmlStreamAttributes &)
{
    return false;
}

void SmilContentModule::endParseElement(SmilElement *, const QString &)
{
    //### we need to process now, and remove element where necessary.
}

QStringList SmilContentModule::elementNames() const
{
    QStringList l;
    l.append("switch");
    l.append("prefetch");
    return l;
}

