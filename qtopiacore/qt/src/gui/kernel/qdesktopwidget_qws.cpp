/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qdesktopwidget.h"
#include "qscreen_qws.h"
#include "private/qapplication_p.h"

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

QDesktopWidget::QDesktopWidget()
    : QWidget(0, Qt::Desktop)
{
    setObjectName(QLatin1String("desktop"));
}

QDesktopWidget::~QDesktopWidget()
{
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return true;
}

int QDesktopWidget::primaryScreen() const
{
    return 0;
}

int QDesktopWidget::numScreens() const
{
    QScreen *screen = QScreen::instance();
    if (!screen)
        return 0;

    const QList<QScreen*> subScreens = screen->subScreens();
    return qMax(subScreens.size(), 1);
}

QWidget *QDesktopWidget::screen(int)
{
    return this;
}

const QRect QDesktopWidget::availableGeometry(int screenNo) const
{
    const QScreen *screen = QScreen::instance();
    if (screenNo == -1)
        screenNo = 0;
    if (!screen || screenNo < 0)
        return QRect();

    const QList<QScreen*> subScreens = screen->subScreens();
    if (!subScreens.isEmpty()) {
        if (screenNo >= subScreens.size())
            return QRect();
        screen = subScreens.at(screenNo);
    }

    QApplicationPrivate *ap = QApplicationPrivate::instance();
    const QRect r = ap->maxWindowRect(screen);
    if (!r.isEmpty())
        return r;

    return screen->region().boundingRect();
}

const QRect QDesktopWidget::screenGeometry(int screenNo) const
{
    const QScreen *screen = QScreen::instance();
    if (screenNo == -1)
        screenNo = 0;
    if (!screen || screenNo < 0)
        return QRect();

    const QList<QScreen*> subScreens = screen->subScreens();
    if (subScreens.size() == 0 && screenNo == 0)
        return screen->region().boundingRect();

    if (screenNo >= subScreens.size())
        return QRect();

    return subScreens.at(screenNo)->region().boundingRect();
}

int QDesktopWidget::screenNumber(const QWidget *w) const
{
    if (!w)
        return 0;

    QRect frame = w->frameGeometry();
    if (!w->isWindow())
        frame.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));
    const QPoint midpoint = (frame.topLeft() + frame.bottomRight()) / 2;
    return screenNumber(midpoint);
}

int QDesktopWidget::screenNumber(const QPoint &p) const
{
    const QScreen *screen = QScreen::instance();
    if (!screen || !screen->region().contains(p))
        return -1;

    const QList<QScreen*> subScreens = screen->subScreens();
    if (subScreens.size() == 0)
        return 0;

    for (int i = 0; i < subScreens.size(); ++i)
        if (subScreens.at(i)->region().contains(p))
            return i;

    return -1;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
}

QT_END_NAMESPACE
