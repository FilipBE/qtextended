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

#ifndef SMILMODULE_H
#define SMILMODULE_H

#include <qstring.h>
#include <qstringlist.h>

class SmilElement;
class SmilSystem;
class SmilEvent;
class QXmlStreamAttributes;
class QPainter;

class SmilModule
{
public:
    virtual ~SmilModule();
    virtual SmilElement *beginParseElement(SmilSystem *sys, SmilElement *parent, const QString &qName, const QXmlStreamAttributes &) = 0;
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlStreamAttributes &atts) = 0;
    virtual void endParseElement(SmilElement *parent, const QString &qName) = 0;
    virtual QStringList elementNames() const;
    virtual QStringList attributeNames() const;

    virtual void process();
};

class SmilModuleAttribute
{
public:
    SmilModuleAttribute(SmilElement *e, const QXmlStreamAttributes &atts);
    virtual ~SmilModuleAttribute();

    virtual QString name() const;
    virtual void process();
    virtual void reset();
    virtual void event(SmilElement *, SmilEvent *);
    virtual void paint(QPainter *p);

    SmilElement *element;
};

#endif
