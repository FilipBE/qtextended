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

#include "qtopiatabwidget.h"
#include "qdelayedscrollarea.h"

#include <QList>
#include <QDebug>

#define DELAY_LOAD_TABS

QtopiaTabWidget::QtopiaTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
}

QtopiaTabWidget::~QtopiaTabWidget()
{
}

// implemented to hide them
int QtopiaTabWidget::addTab(QWidget *child, const QString &label)
{
    Q_UNUSED(child)
    Q_UNUSED(label)
    return -1;
}

int QtopiaTabWidget::addTab(QWidget *child, const QIcon &icon, const QString &label)
{
    Q_UNUSED(child)
    Q_UNUSED(icon)
    Q_UNUSED(label)
    return -1;
}

int QtopiaTabWidget::insertTab(int index, QWidget *widget, const QString &label)
{
    Q_UNUSED(index)
    Q_UNUSED(widget)
    Q_UNUSED(label)
    return -1;
}

int QtopiaTabWidget::insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label)
{
    Q_UNUSED(index)
    Q_UNUSED(widget)
    Q_UNUSED(icon)
    Q_UNUSED(label)
    return -1;
}

QWidget *QtopiaTabWidget::widget(int index) const
{
    Q_UNUSED(index)
    return 0;
}

int QtopiaTabWidget::addTab(const QString &label)
{
#ifdef DELAY_LOAD_TABS
    QDelayedScrollArea *sc = new QDelayedScrollArea;
    sc->setWidgetResizable(true);
    sc->setFrameStyle(QFrame::NoFrame);
    sc->setIndex(QTabWidget::addTab(sc, label));
    unpreparedTabs.insert(sc->index(), sc);
    connect(sc, SIGNAL(aboutToShow(int)), this, SLOT(layoutTab(int)));
    return sc->index();
#else
    QScrollArea *sc = new QScrollArea;
    sc->setWidgetResizable(true);
    sc->setFrameStyle(QFrame::NoFrame);
    int index = QTabWidget::addTab(sc, label);
    emit prepareTab(index, sc);
    sc->setFocusPolicy(Qt::NoFocus);
    return index;
#endif
}

int QtopiaTabWidget::addTab(const QIcon &icon, const QString &label)
{
#ifdef DELAY_LOAD_TABS
    QDelayedScrollArea *sc = new QDelayedScrollArea;
    sc->setWidgetResizable(true);
    sc->setIndex(QTabWidget::addTab(sc, icon, label));
    sc->setFrameStyle(QFrame::NoFrame);
    unpreparedTabs.insert(sc->index(), sc);
    connect(sc, SIGNAL(aboutToShow(int)), this, SLOT(layoutTab(int)));
    return sc->index();
#else
    QScrollArea *sc = new QScrollArea;
    sc->setWidgetResizable(true);
    sc->setFrameStyle(QFrame::NoFrame);
    int index = QTabWidget::addTab(sc, icon, label);
    emit prepareTab(index, sc);
    sc->setFocusPolicy(Qt::NoFocus);
    return index;
#endif
}

void QtopiaTabWidget::layoutTab(int index)
{
    if (unpreparedTabs.contains(index)) {
        QDelayedScrollArea *sc = unpreparedTabs.value(index);
        emit prepareTab(index, sc);
        sc->setFocusPolicy(Qt::NoFocus);
        unpreparedTabs.remove(index);
    }
}
