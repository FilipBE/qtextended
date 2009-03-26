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

#include "smil.h"
#include "system.h"
#include "structure.h"
#include "layout.h"
#include "content.h"
#include "timing.h"
#include "media.h"
#include "transfer.h"
#include <qstack.h>
#include <qmap.h>
#include <qfile.h>
#include <qpainter.h>
#include <QXmlStreamReader>
#include <QDebug>

class SmilParser
{
public:
    SmilParser(SmilSystem *s);

    bool parse(QXmlStreamReader& xml);

private:
    bool endDocument();
    bool startElement(const QStringRef& namespaceURI, const QStringRef& localName, const QStringRef& qName, const QXmlStreamAttributes& atts);
    bool endElement(const QStringRef& namespaceURI, const QStringRef& localName, const QStringRef& qName);
    bool characters(const QStringRef& ch);

    SmilSystem *sys;
    QStack<SmilElement*> parseStack;
    SmilElement *root;
    SmilElement *current;
    int ignore;
};

SmilParser::SmilParser(SmilSystem *s)
    : sys(s), root(0), current(0), ignore(0)
{
    sys->addModule("Structure", new SmilStructureModule());
    sys->addModule("Content", new SmilContentModule());
    sys->addModule("Layout", new SmilLayoutModule());
    sys->addModule("Timing", new SmilTimingModule());
    sys->addModule("Media", new SmilMediaModule());
}

bool SmilParser::endDocument()
{
    if (root) {
        sys->setRootElement(root);
        return true;
    }

    return false;
}

bool SmilParser::startElement(const QStringRef&/*namespaceURI*/, const QStringRef&/*localName*/, const QStringRef& qName, const QXmlStreamAttributes& atts)
{
    QString elementName(qName.toString());

    SmilElement *e = 0;
    QMap<QString, SmilModule *>::ConstIterator it;
    for (it = sys->modules().begin(); it != sys->modules().end(); ++it) {
        e = (*it)->beginParseElement(sys, current, elementName, atts);
        if (e) {
            if (current)
                current->addChild(e);
            parseStack.push(e);
            break;
        }
    }

    if (e) {
        current = e;
        if (!root)
            root = e;
        for (it = sys->modules().begin(); it != sys->modules().end(); ++it) {
            (*it)->parseAttributes(sys, e, atts);
        }
    } else {
        ++ignore;
    }

    return true;
}

bool SmilParser::endElement(const QStringRef&/*namespaceURI*/, const QStringRef&/*localName*/, const QStringRef& qName)
{
    QString elementName(qName.toString());

    if (ignore) {
        ignore--;
    } else {
        QMap<QString, SmilModule *>::ConstIterator it;
        for (it = sys->modules().begin(); it != sys->modules().end(); ++it) {
            (*it)->endParseElement(current, elementName);
        }
        if (parseStack.top() != root)
            parseStack.pop();
        current = parseStack.top();
    }

    return true;
}

bool SmilParser::characters(const QStringRef& ch)
{
    if (current && !ignore) {
        current->addCharacters(ch.toString());
    }

    return true;
}

bool SmilParser::parse(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            startElement(xml.namespaceUri(), xml.name(), xml.qualifiedName(), xml.attributes());
        } else if (xml.isEndElement()) {
            endElement(xml.namespaceUri(), xml.name(), xml.qualifiedName());
        } else if (xml.isCharacters()) {
            characters(xml.text());
        }
    }

    if (xml.hasError()) {
        qWarning("%s (line %lld:%lld)", qPrintable(xml.errorString()), xml.lineNumber(), xml.columnNumber());
        return false;
    } else {
        endDocument();
        return true;
    }
}

class SmilViewPrivate
{
public:
    SmilViewPrivate() : sys(0), parser(0) {}

    SmilSystem *sys;
    SmilParser *parser;
};


SmilView::SmilView(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f), d(0)
{
    d = new SmilViewPrivate;
}

SmilView::~SmilView()
{
    clear();
    delete d;
}

bool SmilView::setSource(const QString &str)
{
    clear();
    d->sys = new SmilSystem();
    d->sys->setTarget(this);
    connect(d->sys->transferServer(), SIGNAL(transferRequested(SmilDataSource*,QString)),
            this, SIGNAL(transferRequested(SmilDataSource*,QString)));
    connect(d->sys->transferServer(), SIGNAL(transferCancelled(SmilDataSource*,QString)),
            this, SIGNAL(transferCancelled(SmilDataSource*,QString)));
    connect(d->sys, SIGNAL(finished()), this, SIGNAL(finished()));
    d->parser = new SmilParser(d->sys);
    QXmlStreamReader reader;
    reader.addData(str);
    if (!d->parser->parse( reader )){
        qWarning("Unable to parse SMIL file");
        clear();
        return false;
    } else {
        QPalette pal;
        pal.setColor(QPalette::Window, d->sys->rootColor());
        setPalette(pal);
    }

    return true;
}

void SmilView::play()
{
    if (d->sys)
        d->sys->play();
}

void SmilView::reset()
{
    if (d->sys)
        d->sys->reset();
}

SmilElement *SmilView::rootElement() const
{
    if (d->sys)
        return d->sys->rootElement();
    return 0;
}

void SmilView::paintEvent(QPaintEvent *)
{
    if (d->sys) {
        // ### handle layer z-order.
        QPainter p(this);
        p.setClipRegion(d->sys->dirtyRegion());
        d->sys->paint(&p);
        d->sys->setDirty(QRect());
    }
}

void SmilView::clear()
{
    delete d->sys;
    delete d->parser;
    d->sys = 0;
    d->parser = 0;
}

