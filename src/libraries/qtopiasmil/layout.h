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

#ifndef SMILLAYOUT_H
#define SMILLAYOUT_H

#include <element.h>
#include <module.h>
#include <qmap.h>

class SmilRootLayout : public SmilElement
{
public:
    SmilRootLayout(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void process();
    void paint(QPainter *p);

private:
    QSize size;
};

class SmilRegion : public SmilElement
{
public:
    SmilRegion(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void process();
    void setRect(const QRect &r);
    void paint(QPainter *p);

    void addReferrer(SmilElement *e);

    enum Fit { FillFit, HiddenFit, MeetFit, ScrollFit, SliceFit };
    enum ShowBackground { ShowAlways, ShowActive };

    struct Position {
        Position() : value(0), type(Unspecified) {}
        int value;
        enum Type { Unspecified, Absolute, Percent };
        Type type;
    };

    Position parsePosition(const QString &val);

    Fit fit;
    QString regionName;
    ShowBackground showBackground;
    int zIndex;
    Position left;
    Position right;
    Position width;
    Position top;
    Position bottom;
    Position height;
    SmilElementList referrers;
};

class SmilLayout : public SmilElement
{
public:
    SmilLayout(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);
    virtual ~SmilLayout();
};

//===========================================================================

class SmilLayoutModule : public SmilModule
{
public:
    SmilLayoutModule();
    virtual ~SmilLayoutModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlStreamAttributes &atts);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlStreamAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elementNames() const;
    virtual QStringList attributeNames() const;

    SmilRegion *findRegion(const QString &id);

protected:
    QList<SmilLayout*> layouts;
};

//===========================================================================

class RegionAttribute : public SmilModuleAttribute
{
public:
    RegionAttribute(SmilLayoutModule *m, SmilElement *e, const QXmlStreamAttributes &atts);

    void process();

private:
    QString rgn;
    SmilLayoutModule *layoutModule;
};

#endif
