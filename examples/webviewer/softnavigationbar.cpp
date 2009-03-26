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

#include "softnavigationbar.h"
#include <QAction>
#include <QSoftMenuBar>
#include <QEvent>
#include <QKeyEvent>

SoftNavigationBar::SoftNavigationBar(QWidget* parent)
    : QObject(parent)
{
    mainWindow = parent;
    backAction = 0;
    forwardAction = 0;
    mainWindow->installEventFilter(this);
}

void SoftNavigationBar::setBack(QAction *action)
{
    backAction = action;
    connect(backAction, SIGNAL(changed()), this, SLOT(actionChanged()));
    actionChanged();
}

void SoftNavigationBar::setForward(QAction *action)
{
    forwardAction = action;
    connect(forwardAction, SIGNAL(changed()), this, SLOT(actionChanged()));
    actionChanged();
}

void SoftNavigationBar::actionChanged()
{
    if (backAction && backAction->isEnabled())
        QSoftMenuBar::setLabel(mainWindow, Qt::Key_Context2, QSoftMenuBar::Previous);
    else
        QSoftMenuBar::setLabel(mainWindow, Qt::Key_Context2, QSoftMenuBar::NoLabel);
    if (forwardAction && forwardAction->isEnabled())
        QSoftMenuBar::setLabel(mainWindow, Qt::Key_Context1, QSoftMenuBar::Next);
    else
        QSoftMenuBar::setLabel(mainWindow, Qt::Key_Context1, QSoftMenuBar::NoLabel);
}

bool SoftNavigationBar::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Context1 && forwardAction) {
            if (forwardAction->isEnabled())
                forwardAction->trigger();
            return true;
        } else if (ke->key() == Qt::Key_Context2 && backAction) {
            if (backAction->isEnabled())
                backAction->trigger();
            return true;
        }
    }
    return false;
}
