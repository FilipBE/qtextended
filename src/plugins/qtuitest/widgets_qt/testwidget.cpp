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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "testwidget.h"
#include "testwidgetslog.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QScrollBar>
#include <QVariant>
#include <QWidget>

TestWidget::TestWidget(QObject* _q)
    : q(qobject_cast<QWidget*>(_q))
{ q->installEventFilter(this); }

bool TestWidget::eventFilter(QObject*,QEvent* e)
{
    if (e->type() == QEvent::FocusIn) {
        emit gotFocus();
    }
    return false;
}

const QRect& TestWidget::geometry() const
{ return q->geometry(); }

QRect TestWidget::rect() const
{ return q->rect(); }

bool TestWidget::isVisible() const
{ return q->isVisible(); }

QRegion TestWidget::visibleRegion() const
{ return q->visibleRegion(); }

bool TestWidget::ensureVisibleRegion(const QRegion& region)
{
    TestWidgetsLog() << "region" << region << "visibleRegion" << visibleRegion();
    if (region.intersected(visibleRegion()) == region) return true;

    if (!QtUiTest::mousePreferred()) {
        /*
            FIXME this is technically not correct, since we're assuming
            that giving the widget keyboard focus makes the entire widget
            visible.  A low priority fix until we come across a case where
            this matters.
        */
        return setFocus();
    }

    /* Try to find a scroll area which contains this widget, then scroll. */

    QAbstractScrollArea *sa = 0;
    QWidget *parent = q->parentWidget();
    while (parent && !sa) {
        sa = qobject_cast<QAbstractScrollArea*>(parent);
        parent = parent->parentWidget();
    }
    TestWidgetsLog() << "sa:" << sa << "region:" << region;

    if (!sa) return false;

    /* Figure out the points to click for scrolling in each direction */
    QScrollBar *vbar = sa->verticalScrollBar();
    QScrollBar *hbar = sa->horizontalScrollBar();
    QPoint up    = vbar->mapToGlobal(QPoint(vbar->width()/2,5));
    QPoint down  = vbar->mapToGlobal(QPoint(vbar->width()/2,vbar->height()-5));
    QPoint left  = hbar->mapToGlobal(QPoint(5,              hbar->height()/2));
    QPoint right = hbar->mapToGlobal(QPoint(hbar->width()-5,hbar->height()/2));

    QRect brect_origin = region.boundingRect();
    QRect brect = brect_origin;
    brect.moveTopLeft(q->mapTo(sa, brect.topLeft()));

    static const int MAX_CLICKS = 200;
    int clicks = 0;

    TestWidgetsLog() << "Now attempting to scroll so that visibleRegion"
            << "contains rect" << brect;

    /* Handle up... */
    while (brect.top() < 0) {
        if (!vbar->isVisible()) return false;
        TestWidgetsLog() << "up" << brect << sa->rect();
        QtUiTest::mouseClick(up);
        brect = brect_origin;
        brect.moveTopLeft(q->mapTo(sa, brect.topLeft()));
        if (++clicks > MAX_CLICKS) return false;
    }
    /* Handle down... */
    while (brect.bottom() > sa->height()) {
        if (!vbar->isVisible()) return false;
        TestWidgetsLog() << "down" << brect << sa->rect();
        QtUiTest::mouseClick(down);
        brect = brect_origin;
        brect.moveTopLeft(q->mapTo(sa, brect.topLeft()));
        if (++clicks > MAX_CLICKS) return false;
    }
    /* Handle left... */
    while (brect.left() < 0) {
        if (!hbar->isVisible()) return false;
        TestWidgetsLog() << "left" << brect << sa->rect();
        QtUiTest::mouseClick(left);
        brect = brect_origin;
        brect.moveTopLeft(q->mapTo(sa, brect.topLeft()));
        if (++clicks > MAX_CLICKS) return false;
    }
    /* Handle right... */
    while (brect.right() > sa->width()) {
        if (!hbar->isVisible()) return false;
        TestWidgetsLog() << "right" << brect << sa->rect();
        QtUiTest::mouseClick(right);
        brect = brect_origin;
        brect.moveTopLeft(q->mapTo(sa, brect.topLeft()));
        if (++clicks > MAX_CLICKS) return false;
    }
    return true;
}

const QObjectList &TestWidget::children() const
{ return q->children(); }

QObject* TestWidget::parent() const
{ return q->parent(); }

QString TestWidget::windowTitle() const
{ return q->windowTitle(); }

QPoint TestWidget::mapToGlobal(QPoint const &local) const
{
    return q->mapToGlobal(local);
}

QPoint TestWidget::mapFromGlobal(QPoint const &global) const
{
    return q->mapFromGlobal(global);
}

bool TestWidget::hasFocus() const
{
    return q->hasFocus();
}

Qt::WindowFlags TestWidget::windowFlags() const
{
    return q->windowFlags();
}

bool TestWidget::canEnter(QVariant const&) const
{
    return false;
}

bool TestWidget::enter(QVariant const& item, bool noCommit)
{
    Q_UNUSED(noCommit);
    if (!hasFocus() && !setFocus()) return false;

    using namespace QtUiTest;

    /* If there's text currently in the field then erase it first */
    if (QtUiTest::mousePreferred()) {
        return false;
    }

    foreach (QChar const& c, item.toString()) {
        TestWidgetsLog() << asciiToModifiers(c.toLatin1());
        keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
    }
    return true;
}

void TestWidget::focusOutEvent()
{
}

#ifdef Q_WS_QWS
bool TestWidget::hasEditFocus() const
{ return q->hasEditFocus(); }
#endif

bool TestWidget::setEditFocus(bool enable)
{
    if (hasEditFocus() == enable) return true;

    if (!hasFocus()) {
        if (!setFocus()) return false;
    }

    // It is possible that giving us regular focus also gave us edit focus.
    if (hasEditFocus() == enable) return true;

#ifdef Q_WS_QWS
    if (!QtUiTest::keyClick(q, QtUiTest::Key_Activate)) return false;
    if (hasEditFocus() != enable && !QtUiTest::waitForEvent(q, enable ? QEvent::EnterEditFocus : QEvent::LeaveEditFocus)) {
        return false;
    }
    return (hasEditFocus() == enable);
#else
    return true;
#endif
}

bool TestWidget::canWrap(QObject *o)
{ return qobject_cast<QWidget*>(o); }

QWidget* TestWidget::focusWidget()
{
    QWidget *w = QApplication::focusWidget();
    if (!w) w = QApplication::activeWindow();
    if (!w) w = QApplication::activePopupWidget();
    if (!w) w = QApplication::activeModalWidget();

    return w;
}

QString TestWidget::printable(QString const& str)
{
    QString ret(str);

    ret.remove(QChar(0x200E));
    ret.remove(QChar(0x200F));
    ret.remove(QChar(0x00AD)); // equivalent to Qtopia::dehyphenate

    return ret;
}
