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

#include "testmenu.h"
#include "testwidgetslog.h"

#include <QMenu>
#include <QAction>

TestMenu::TestMenu(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QMenu*>(_q)), lastAction(0)
{
    // For accurate ordering of events recording, these connections
    // must come before all others.
    QtUiTest::connectFirst(q,    SIGNAL(hovered(QAction*)),
                        this, SLOT(on_hovered(QAction*)));
}

void TestMenu::on_hovered(QAction* a)
{
    if (lastAction)
        QtUiTest::disconnectFirst(lastAction, 0, this, 0);
    lastAction = a;
    if (lastAction) {
        QtUiTest::connectFirst(lastAction, SIGNAL(toggled(bool)),
                            this,       SLOT(on_toggled(bool)));
        QtUiTest::connectFirst(lastAction, SIGNAL(triggered(bool)),
                            this,       SLOT(on_triggered(bool)));
    }
}

void TestMenu::on_toggled(bool state)
{ emit stateChanged(state); }

QString textForAction(QMenu* m, QAction* a)
{
    QString ret;
    foreach (QAction* child, m->actions()) {
        if (child == a) {
            ret = a->text();
            ret.replace("/","\\/");
        } else if (child->menu()) {
            ret = textForAction(child->menu(), a);
            if (!ret.isEmpty())
                ret.prepend(child->menu()->title().replace("/","\\/") + "/");
        }
        if (!ret.isEmpty())
            break;
    }
    return ret;
}

void TestMenu::on_triggered(bool)
{
    /*
        Both the top level and submenus emit the triggered() signal.
        We only want to emit selected() if we are the top level menu.
    */
    bool top_level = true;
    if (q->menuAction()) {
        foreach (QWidget* w, q->menuAction()->associatedWidgets()) {
            if (qobject_cast<QMenu*>(w)) {
                top_level = false;
                break;
            }
        }
    }

    if (!top_level) return;

    emit selected(textForAction(q,lastAction));
}

QString TestMenu::text() const
{
    return list().join("\n");
}

QString TestMenu::selectedText() const
{
    QString ret;

    QAction *active = q->activeAction();
    if (!active) return ret;

    QMenu *child = active->menu();
    if (!child || !child->activeAction())
        return active->text().replace("/", "\\/");

    return active->text().replace("/", "\\/") + "/" + qtuitest_cast<QtUiTest::TextWidget*>(child)->selectedText();
}

QStringList TestMenu::list() const
{
    QStringList ret;

    /* Iterate through every action */
    foreach( QAction *a, q->actions() ) {
        if (!a->isEnabled())
            continue;
        QString t = a->text();
        t.replace("/","\\/");
        if (!t.isEmpty()) ret << t;
        if (a->menu() && a->menu() != q) {
            QStringList sub = qtuitest_cast<QtUiTest::ListWidget*>(a->menu())->list();
            foreach (QString s, sub) {
                if (!s.isEmpty()) ret << t + "/" + s;
            }
        }
    }

    return ret;
}

QRect TestMenu::visualRect(QString const &item) const
{
    TestWidgetsLog() << item;
    static QRegExp slashRe("[^\\\\]/");
    QRect ret;

    QStringList items = list();

    foreach (QString s, items) {
        // Remove all separators, because they don't take up space
        if (s.isEmpty()) items.removeAll(s);
        // Remove all non-toplevel items because they don't affect height
        if (s.contains(slashRe)) items.removeAll(s);
    }

    int ind;
    if (0 != (ind = item.indexOf(slashRe)+1)) {
        QString top(item.left(ind));
        QString rest(item.mid(ind+1));

        QAction *a(q->activeAction());
        QString t = a->text();
        t.replace("/","\\/");
        if (!a || t != top || !a->menu()) {
            TestWidgetsLog() << "The desired submenu" << item << "is not visible";
            return ret;
        }

        ret = qtuitest_cast<QtUiTest::ListWidget*>(a->menu())->visualRect(rest);
        ret.moveTopLeft( q->mapFromGlobal( a->menu()->mapToGlobal( ret.topLeft() ) ) );
        return ret;
    }

    if (!items.contains(item)) {
        TestWidgetsLog() << "Menu does not contain" << item;
        return ret;
    }

    foreach( QAction *a, q->actions() ) {
        QString t = a->text();
        t.replace("/","\\/");
        if (t == item) {
            ret = q->actionGeometry(a);
            TestWidgetsLog() << "Found menu item with geometry" << ret << q->isVisible();
            break;
        }
    }

    return ret;
}

Qt::CheckState TestMenu::checkState() const
{
    QAction *active = q->activeAction();
    return (active && active->isChecked()) ? Qt::Checked : Qt::Unchecked;
}

bool TestMenu::ensureVisible(QString const&)
{ return false; }

bool TestMenu::canSelect(QString const &item) const
{
    return list().contains(item);
}

bool TestMenu::select(QString const &item)
{
    using namespace QtUiTest;

    if (!isVisible()) {
        setErrorString("Can't select " + item + " because menu is not visible");
        return false;
    }

    TestWidgetsLog() << item;
    static QRegExp slashRe("[^\\\\]/");

    QString topItem = item;
    QString rest;

    int ind;
    if (0 != (ind = item.indexOf(slashRe)+1)) {
        topItem = item.left(ind);
        rest = item.mid(ind+1);
    }

    QRect r = visualRect(topItem);
    if (r.isNull()) {
        setErrorString("Can't find visual rect for item " + topItem + " in menu");
        return false;
    }

    QPoint pos = r.center();

    if (mousePreferred()) {
        while (!rect().contains(pos)) {
            QRect visibleRect = visibleRegion().boundingRect();
            QPoint globalPos = mapToGlobal(visibleRect.topLeft());
            if (pos.y() < globalPos.y()) {
                TestWidgetsLog() << "click to scroll up";
                mouseClick(QPoint(globalPos.x() + visibleRect.width() / 2, globalPos.y() + 8));
                QtUiTest::wait(200);
            } else {
                TestWidgetsLog() << "click to scroll down";
                mouseClick(QPoint(globalPos.x() + visibleRect.width() / 2, globalPos.y() + visibleRect.height() - 8));
                QtUiTest::wait(200);
            }

            pos = visualRect(topItem).center();
        }
        TestWidgetsLog() << "click on item";
        if (!mouseClick(q, mapToGlobal(pos)))
            return false;
    } else {
        Qt::Key key;
        if ( pos.y() > q->actionGeometry(q->activeAction()).center().y() )
            key = Qt::Key_Down;
        else
            key = Qt::Key_Up;

        while ( selectedText() != topItem ) {
            TestWidgetsLog() << "key" << ((key == Qt::Key_Down) ? "down" : "up");
            if (!keyClick(q, key)) return false;
        }

        TestWidgetsLog() << "select key";
        // If this is the last item, it should be trigged...
        if (item == topItem) {
            if (!keyClick(q->activeAction(), SIGNAL(triggered(bool)), QtUiTest::Key_Select))
                return false;
        }
        // ...but if it has a submenu, it won't be; rather, its menu will be shown.
        else if (q->activeAction() && q->activeAction()->menu()) {
            if (!keyClick(q->activeAction()->menu(), QtUiTest::Key_Select))
                return false;
        }

        // And if it doesn't have a submenu and isn't the final item... well, that shouldn't
        // happen, but that will be caught below.
    }

    if (topItem != item) {
        QAction *a(q->activeAction());
        if (!a || !a->menu()) {
            setErrorString("Failed to select " + item + " because " +
                    (!a ? "focus could not be given to a parent item."
                        : "an item was expected to have a submenu, but didn't."));
            return false;
        }

        // Wait until the menu is visible
        if (!a->menu()->isVisible() && !waitForEvent(a->menu(), QEvent::Show)) {
            setErrorString("Failed to select " + item + " because a submenu did not become "
                    "visible when it should have.");
            return false;
        }

        TestWidgetsLog() << "calling select() on child";
        return qtuitest_cast<SelectWidget*>(a->menu())->select(rest);
    }
    return true;
}

/*
    NOTE: here we are assuming that all menus are the global options menu,
    or a child of it.
*/
bool TestMenu::inherits(QtUiTest::WidgetType type) const
{ return (QtUiTest::OptionsMenu == type); }

bool TestMenu::canWrap(QObject *o)
{ return qobject_cast<QMenu*>(o); }

