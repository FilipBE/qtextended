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

#ifndef SMILSTRUCTURE_H
#define SMILSTRUCTURE_H

#include <module.h>
#include <timing.h>


class Smil : public SmilElement
{
public:
    Smil(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);
};

class SmilHead : public SmilElement
{
public:
    SmilHead(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);
};

class SmilBody : public SmilSeq
{
public:
    SmilBody(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void setState(State);
};

//===========================================================================

class SmilStructureModule : public SmilModule
{
public:
    SmilStructureModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlStreamAttributes &);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlStreamAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elementNames() const;
};

#endif

