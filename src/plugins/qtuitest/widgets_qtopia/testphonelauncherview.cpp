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

#include "testphonelauncherview.h"
#include "testwidgetslog.h"

#include <QContent>
#include <QContentSet>
#include <Qtopia>

TestPhoneLauncherView::TestPhoneLauncherView(QObject *_q)
: TestWidget(_q), q(_q)
{
    init();
    /* Need to reinitialize some parameters when the widget changes */
    q->installEventFilter(this);
    if (!connect(q, SIGNAL(pressed(QContent)), this, SLOT(on_pressed(QContent))))
        Q_ASSERT(0);
}

void TestPhoneLauncherView::on_pressed(QContent c)
{ emit selected(Qtopia::dehyphenate(c.name())); }

void TestPhoneLauncherView::init()
{
    QMetaObject::invokeMethod(q, "rows", Qt::DirectConnection,
            Q_RETURN_ARG(int, rows));
    QMetaObject::invokeMethod(q, "columns", Qt::DirectConnection,
            Q_RETURN_ARG(int, columns));
    QMetaObject::invokeMethod(q, "itemDimensions", Qt::DirectConnection,
            Q_ARG(int*, &itemWidth),
            Q_ARG(int*, &itemHeight));

    TestWidgetsLog() << "rows" << rows << "columns" << columns
        << "dimensions (" << itemWidth << "x" << itemHeight << ")";
}

bool TestPhoneLauncherView::eventFilter(QObject* o, QEvent* e)
{
    if (o == q && e->type() == QEvent::Resize)
        init();
    return false;
}

QStringList selectableContent(QContent *c)
{
    QStringList list;
    if (!c) return list;
    list << Qtopia::dehyphenate(c->name());

    if (c->type().startsWith("Folder/")) {
        QString cat = c->type().mid(7);
        QContentFilter filters = (QContentFilter( QContent::Application ) | QContentFilter( QContent::Folder ))
            & QContentFilter( QContentFilter::Category, cat );

        QContentSet set( filters );
        QStringList sublist;
        for (int i = 0; i < set.count(); ++i) {
            QContent content = set.content(i);
            sublist << selectableContent(&content);
        }
        for (int i = 0; i < sublist.count(); ++i) {
            sublist[i].prepend(cat + "/");
        }
        list << sublist;
    }
    return list;
}

QStringList TestPhoneLauncherView::list() const
{
    QStringList ret;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QContent *c = itemAt(i,j);
            if (!c) continue;
            ret << selectableContent(c);
        }
    }

    return ret;
}

QRect TestPhoneLauncherView::visualRect(QString const &item) const
{
    QRect ret;

    for ( int i = rows; i >= 0 && ret.isNull(); --i ) {
        for ( int j = columns; j >= 0 && ret.isNull(); --j ) {
            QContent *c = itemAt(i,j);
            if (!c) continue;
            if (Qtopia::dehyphenate(c->name()) != item) continue;

            ret = QRect(j*itemWidth, i*itemHeight,
                    itemWidth, itemHeight);
            TestWidgetsLog() << item << "found at (" << i << "," << j << ")"
                    << "rect" << ret;
        }
    }

    return ret;
}

bool TestPhoneLauncherView::canSelect(QString const& item) const
{ return list().contains(item); }

bool TestPhoneLauncherView::select(QString const& _item)
{
    using namespace QtUiTest;

    if (!canSelect(_item)) {
        return false;
    }

    QString item(_item);
    QString subItem;
    if (item.contains("/")) {
        item = _item.left(_item.indexOf("/"));
        subItem = _item.mid(item.length()+1);
    }

    /* If not visible, raise self.
     * Try to do so in two different ways:
     *  - selecting "Menu" from soft menu
     *  - selecting "mainmenu" from home screen
     */
    while (visibleRegion().isEmpty()) {
        TestWidgetsLog() << "isVisible" << isVisible() << "visibleRegion" << visibleRegion() << "geometry" << geometry();
        QString softMenuError;
        QString homeScreenError;
        QObject* o = findWidget(SoftMenu);
        SelectWidget* sw =
            qtuitest_cast<SelectWidget*>(o);
        TestWidgetsLog() << "Trying to raise self using" << o;
        if (sw && sw->canSelect("Menu")) {
            if (sw->select("Menu")) {
                if (visibleRegion().isEmpty() && !waitForEvent(q, QEvent::Show)) {
                    setErrorString("Selected \"Menu\" from soft menu, but launcher menu did not become visible.");
                    return false;
                }
                break;
            } else {
                softMenuError = errorString();
            }
        }

        o = findWidget(HomeScreen);
        sw = qtuitest_cast<SelectWidget*>(o);
        TestWidgetsLog() << "Trying to raise self using" << o;
        if (sw && sw->canSelect("mainmenu")) {
            if (sw->select("mainmenu")) {
                if (visibleRegion().isEmpty() && !waitForEvent(q, QEvent::Show)) {
                    setErrorString("Selected \"mainmenu\" from homescreen, but launcher menu did not become visible.");
                    return false;
                }
                break;
            } else {
                homeScreenError = errorString();
            }
        }

        setErrorString(QString(
            "The launcher could not be raised.  Could not select 'Menu' from soft menu%1 "
            "or 'mainmenu' icon from home screen%2.")
            .arg(softMenuError.isEmpty()   ? QString() : QString(" (%1)").arg(softMenuError))
            .arg(homeScreenError.isEmpty() ? QString() : QString(" (%1)").arg(homeScreenError))
        );
        return false;
    }

    if (Qtopia::mousePreferred()) {
        QtUiTest::mouseClick(mapToGlobal(visualRect(item).center()));
        if (subItem.isEmpty()) return true;
        SelectWidget* sw = qtuitest_cast<SelectWidget*>(findWidget(Focus));
        return sw && sw->canSelect(subItem) && sw->select(subItem);
    }

    int desired_row = -1;
    int desired_column = -1;
    int row = -1;
    int column = -1;

    QContent* current = currentItem();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QContent *c = itemAt(i,j);
            if (current == c) {
                row = i;
                column = j;
            }
            if (Qtopia::dehyphenate(c->name()) == item) {
                desired_row = i;
                desired_column = j;
            }
            if (row != -1 && desired_row != -1) break;
        }
    }

    if (-1 == row || -1 == desired_row) {
        setErrorString(
            "Couldn't select " + _item + " from launcher menu because no " + item
            + " icon was found on the main launcher view.");
        return false;
    }

    if (row != desired_row || column != desired_column) {
        Qt::Key hKey = static_cast<Qt::Key>(0);
        int hClicks = 0;
        Qt::Key vKey = static_cast<Qt::Key>(0);
        int vClicks = 0;

        if (desired_row > row) {
            vKey = Qt::Key_Down;
            vClicks = desired_row - row;
            TestWidgetsLog() << item << "down" << vClicks;
        }
        if (desired_row < row) {
            vKey = Qt::Key_Up;
            vClicks = row - desired_row;
            TestWidgetsLog() << item << "up" << vClicks;
        }
        if (desired_column > column) {
            hKey = Qt::Key_Right;
            hClicks = desired_column - column;
            TestWidgetsLog() << item << "right" << hClicks;
        }
        if (desired_column < column) {
            hKey = Qt::Key_Left;
            hClicks = column - desired_column;
            TestWidgetsLog() << item << "left" << hClicks;
        }

        for (int i = 0; i < vClicks; ++i)
            QtUiTest::keyClick(vKey);
        for (int i = 0; i < hClicks; ++i)
            QtUiTest::keyClick(hKey);

        current = currentItem();
        for (int i = 0; i < QtUiTest::maximumUiTimeout() && (!current || Qtopia::dehyphenate(current->name()) != item); i += 100, QtUiTest::wait(100)) {
            current = currentItem();
        }
        if (!current || Qtopia::dehyphenate(current->name()) != item) {
            setErrorString(
                "Failed to navigate to " + item + " in main launcher view. "
                "It didn't become highlighted after pressing directional keys.");
            return false;
        }
    }
    QtUiTest::keyClick(Qt::Key_Select);
    QObject* focus = 0;
    for (int i = 0; i < QtUiTest::maximumUiTimeout() && (!focus || focus == q); i += 200) {
        QtUiTest::waitForSignal(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), 200);
        focus = findWidget(Focus);
    }

    TestWidgetsLog() << _item << "subItem" << subItem;
    if (subItem.isEmpty()) return true;
    SelectWidget* sw = qtuitest_cast<SelectWidget*>(focus);
    ListWidget*   lw = qtuitest_cast<ListWidget*>  (focus);
    TestWidgetsLog() << sw << (sw ? sw->canSelect(subItem) : false);
    if (!sw) {
        QString error =
            "After opening the '" + item + "' menu from the launcher, could not "
            "find a selectable focus widget.";
        if (focus) {
            QDebug(&error) << " Focus:" << focus;
        }
        setErrorString(error);
        return false;
    }
    if (!sw->canSelect(subItem)) {
        setErrorString(
            "'" + subItem + "' is not a valid choice in the '" + item + "' submenu.");
        if (lw)
            setErrorString(errorString() + "\nValid choices are: "
                + lw->list().join(","));
        return false;
    }
    return sw->select(subItem);
}

bool TestPhoneLauncherView::inherits(QtUiTest::WidgetType type) const
{ return (QtUiTest::Launcher == type); }

bool TestPhoneLauncherView::canWrap(QObject *o)
{ return o && o->inherits("PhoneLauncherView"); }

QContent* TestPhoneLauncherView::itemAt(int i, int j) const
{
    QContent *c = 0;
    QMetaObject::invokeMethod(q, "itemAt", Qt::DirectConnection,
            Q_RETURN_ARG(QContent*, c),
            Q_ARG(int, i),
            Q_ARG(int, j));
    return c;
}

QContent* TestPhoneLauncherView::currentItem() const
{
    QContent *c = 0;
    QMetaObject::invokeMethod(q, "currentItem", Qt::DirectConnection,
            Q_RETURN_ARG(QContent*, c));
    return c;
}

