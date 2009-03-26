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

#include "testtabbar.h"

#include "testwidgetslog.h"

#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

TestTabBar::TestTabBar(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QTabBar*>(_q))
{
    connect(q,    SIGNAL(currentChanged(int)),
            this, SLOT(on_currentChanged(int)));
}

void TestTabBar::on_currentChanged(int index)
{ emit selected(q->tabText(index)); }

QString TestTabBar::text() const
{ return list().join("\n"); }

QString TestTabBar::selectedText() const
{ return q->tabText(q->currentIndex()); }

QStringList TestTabBar::list() const
{
    QStringList ret;
    for (int i = 0, max = q->count(); i < max; ++i)
        ret << q->tabText(i);
    return ret;
}

QRect TestTabBar::visualRect(QString const &item) const
{
    for (int i = 0, max = q->count(); i < max; ++i)
        if (q->tabText(i) == item)
            return q->visibleRegion().intersected(QRegion(q->tabRect(i))).boundingRect();

    return QRect();
}

bool TestTabBar::ensureVisible(QString const& item)
{
    /* First, find desired index. */
    int desired = -1;
    for (int i = 0; i < q->count() && -1 == desired; ++i) {
        if (q->tabText(i) == item)
            desired = i;
    }
    if (-1 == desired) return false;

    QToolButton* leftB = 0;
    QToolButton* rightB = 0;

    QList<QToolButton*> buttons = q->findChildren<QToolButton*>();
    QRegion buttonRegion;
    foreach (QToolButton* b, buttons) {
        if (b->arrowType() == Qt::LeftArrow) {
            leftB = b;
            buttonRegion |= b->geometry();
        } else if (b->arrowType() == Qt::RightArrow) {
            rightB = b;
            buttonRegion |= b->geometry();
        }
    }
    QRect buttonRect = buttonRegion.boundingRect();

    int clicks = 0;
    /* While desired tab isn't visible... */
    while (q->visibleRegion().subtracted(buttonRect).intersected( q->tabRect(desired) ).isEmpty()
        && clicks < 50) {

        TestWidgetsLog() << "visible:" << q->visibleRegion().boundingRect() << "buttons:" << buttonRect << "tab:" << q->tabRect(desired);

        QObject* button = 0;

        /* Shall we go to the left or the right? */
        if (q->tabRect(desired).left() >= q->visibleRegion().subtracted(buttonRect).boundingRect().right())
            button = rightB;
        else
            button = leftB;

        QtUiTest::ActivateWidget* aw
            = qtuitest_cast<QtUiTest::ActivateWidget*>(button);
        if (!aw) return false;
        aw->activate();

        ++clicks;
    }

    return !q->visibleRegion().subtracted(buttonRect).intersected( q->tabRect(desired) ).isEmpty();
}

bool TestTabBar::canSelect(QString const& item) const
{
    for (int i = 0, max = q->count(); i < max; ++i) {
        TestWidgetsLog() << "tabText" << q->tabText(i) << "item" << item;
        if (q->tabText(i) == item)
            return true;
    }
    return false;
}

bool isAncestor(QObject* parent, QObject* child)
{
    bool ret = false;
    QObject* p = child;
    while (p && !ret) {
        ret = (p == parent);
        p = p->parent();
    }
    return ret;
}

bool TestTabBar::select(QString const& item)
{
    using namespace QtUiTest;

    QStringList allTabs;
    int desired = -1;
    for (int i = 0, max = q->count(); i < max && -1 == desired; ++i) {
        if (q->tabText(i) == item)
            desired = i;
        allTabs << q->tabText(i);
    }
    if (-1 == desired) {
        setErrorString(
            "Could not select tab '" + item + "' because there is no tab with that text.\n"
            "Available tabs are: " + allTabs.join(","));
        return false;
    }
    if (q->currentIndex() == desired) return true;

    QString originalTab = q->tabText(q->currentIndex());

    if (mousePreferred()) {
        if (!ensureVisible(item)) return false;
        QPoint p = visualRect(item).center();
        if (!ensureVisiblePoint(p)) return false;
        mouseClick(mapToGlobal(p));
        QString selected = selectedText();
        if (selected != item) {
            setErrorString(
                "Clicked on tab '" + item + "' but it did not appear to become selected.\n"
                "Selected tab is: " + selected);
            return false;
        }
        return true;
    }

    // In Qt/Embedded, the left/right key can be used when navigate focus is
    // within a child of the tab widget and no widget has edit focus.
    // On other platforms, it is necessary to move focus to the tab bar.

#ifdef Q_WS_QWS
    /* If focus is not currently within the tab widget, press up/down until
     * it is.
     */
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(q->parent());
    if (tabWidget) {
        QWidget* focus = qApp->focusWidget();
        int clicks = 0;
        while (!isAncestor(tabWidget, focus) && clicks < 50) {
            keyClick(Qt::Key_Down);
            focus = qApp->focusWidget();
            ++clicks;
        }
        if (!isAncestor(tabWidget, focus)) {
            setErrorString(
                "Can't change tabs: couldn't get focus inside of tab widget by "
                "pressing the Down key.");
            return false;
        }
    }
#else
    if (!setFocus()) return false;
#endif

    Qt::Key key;
    int diff = 0;
    if (desired < q->currentIndex()) {
        key = Qt::Key_Left;
        diff = q->currentIndex() - desired;
    }
    else {
        key = Qt::Key_Right;
        diff = desired - q->currentIndex();
    }
    for (int i = 0; i < diff; ++i) {
#ifdef Q_WS_QWS
        // It's possible that the current tab _is_ or switching to another tab brings us _into_
        // edit mode on a text area, in which case further left/right keys may be grabbed.
        // focusOutEvent() will fix that up.
        if (Widget* focus = qtuitest_cast<Widget*>
                (findWidget(Focus))) {
            focus->focusOutEvent();
        }
#else
        if (!setFocus()) return false;
#endif
        if (!keyClick(q, SIGNAL(currentChanged(int)), key))
            return false;
    }

    if (q->currentIndex() == desired) return true;
    setErrorString(
        QString("Can't change tabs: pressed the %1 key %2 times which should have "
                "moved from '%3' to '%4', but the current tab ended up as '%5'.")
        .arg((key == Qt::Key_Right) ? "Right" : "Left")
        .arg(diff)
        .arg(originalTab)
        .arg(item)
        .arg(q->tabText(q->currentIndex()))
    );
    return false;
}

bool TestTabBar::inherits(QtUiTest::WidgetType type) const
{ return (QtUiTest::TabBar == type); }

bool TestTabBar::canWrap(QObject *o)
{ return qobject_cast<QTabBar*>(o); }

