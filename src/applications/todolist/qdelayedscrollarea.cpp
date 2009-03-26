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

#include "qdelayedscrollarea.h"
#include <QEvent>
#include <QDebug>
#include <QLayout>

QDelayedScrollArea::QDelayedScrollArea(int index, QWidget *parent) : QScrollArea(parent), i(index)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
    setFrameStyle(QFrame::NoFrame);
    viewport()->installEventFilter(this);
}

QDelayedScrollArea::QDelayedScrollArea(QWidget *parent) : QScrollArea(parent), i(-1) {}

QDelayedScrollArea::~QDelayedScrollArea() {}

void QDelayedScrollArea::showEvent(QShowEvent *event)
{
    emit aboutToShow(i);

    QScrollArea::showEvent(event);
}

