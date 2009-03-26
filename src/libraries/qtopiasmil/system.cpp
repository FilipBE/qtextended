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

#include "system.h"
#include "transfer.h"
#include "element.h"
#include "module.h"
#include "timing.h"
#include "layout.h"
#include <QPainter>
#include <QWidget>

SmilSystem::SmilSystem() : QObject(), root(0)
{
    xferServer = new SmilTransferServer(this);
}

SmilSystem::~SmilSystem()
{
    delete root;
    QMap<QString,SmilModule *>::Iterator it;
    for (it = mods.begin(); it != mods.end(); ++it)
        delete (*it);
}

void SmilSystem::setRootElement(SmilElement *r)
{
    root = r;
    root->setRect(m_targetWidget->rect());
    QMap<QString, SmilModule *>::ConstIterator it;
    for (it = modules().begin(); it != modules().end(); ++it)
        (*it)->process();
    root->process();
}

SmilElement *SmilSystem::findElement(SmilElement *e, const QString &id) const
{
    if (!e)
        e = root;
    if (e->id() == id)
        return e;
    SmilElementList::ConstIterator it;
    for (it = e->children().begin(); it != e->children().end(); ++it) {
        SmilElement *fe = findElement(*it, id);
        if (fe)
            return fe;
    }

    return 0;
}

QColor SmilSystem::rootColor() const
{
    QColor col(Qt::white);

    SmilElementList::ConstIterator it;
    for (it = root->children().begin(); it != root->children().end(); ++it) {
        if ((*it)->name() == "root-layout") {
            col = ((SmilRootLayout*)(*it))->backgroundColor();
            break;
        }
    }

    return col;
}

void SmilSystem::play()
{
    SmilElementList::ConstIterator it;
    for (it = root->children().begin(); it != root->children().end(); ++it) {
        if ((*it)->name() == "body") {
            SmilTimingAttribute *tn = (SmilTimingAttribute*)(*it)->moduleAttribute("Timing");
            tn->setState(SmilElement::Startup);
        }
    }
}

void SmilSystem::reset()
{
    root->reset();
}

void SmilSystem::setDirty(const QRect &r)
{
    updRgn += r;
}

void SmilSystem::update(const QRect &r)
{
    updRgn += r;
    m_targetWidget->update(updRgn);
}

void SmilSystem::paint(QPainter *p)
{
    paint(root, p);
}

void SmilSystem::paint(SmilElement *e, QPainter *p)
{
    if (e->isVisible() && e->state() >= SmilElement::Active) {
        e->paint(p);
        SmilElementList::ConstIterator it;
        for (it = e->children().begin(); it != e->children().end(); ++it) {
            paint(*it, p);
        }
    }
}

void SmilSystem::bodyFinished()
{
    emit finished();
}

QWidget* SmilSystem::targetWidget() const
{
    return m_targetWidget;
}

