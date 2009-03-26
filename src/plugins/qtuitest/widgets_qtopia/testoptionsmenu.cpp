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

#include "testoptionsmenu.h"
#include "testwidgetslog.h"

#include <QSoftMenuBar>
#include <QMenu>

TestOptionsMenu::TestOptionsMenu()
    : LocalWidget(QSoftMenuBar::activeMenu())
{}

using namespace QtUiTest;

bool raiseOptionsMenu()
{
    QObject* sm = findWidget(SoftMenu);
    if (!sm) {
        setErrorString("Could not find the soft menu widget while attempting to raise the "
                       "options menu.");
        return false;
    }
    SelectWidget* sw = qtuitest_cast<SelectWidget*>(sm);
    ListWidget* lw   = qtuitest_cast<ListWidget*>(sm);
    if (!sw) {
        QString error;
        QDebug(&error) << "Found the soft menu widget" << sm << "but it is not a SelectWidget.";
        setErrorString(error);
        return false;
    }

    TestWidgetsLog() << sm << sw << lw;

    if (!sw->canSelect("Options") && QSoftMenuBar::activeMenu() && QSoftMenuBar::activeMenu()->isVisible()) return true;

    static const int MAX_TRIES = 3;
    for (int t = 0; t < MAX_TRIES; ++t) {

        // Wait until Options is an available button
        for (int i = 0; i < QtUiTest::maximumUiTimeout() && !sw->canSelect("Options"); i += 50, QtUiTest::wait(50))
        {}

        if (!sw->canSelect("Options") || !sw->select("Options")) {
            setErrorString("Could not raise the options menu: couldn't select 'Options' "
                           "from soft menu.");
            if (lw) setErrorString(errorString() +
                    " Soft menu contents are: " + lw->list().join(","));
            return false;
        }

        for (int i = 0;
                i < QtUiTest::maximumUiTimeout() && (sw->canSelect("Options") || !QSoftMenuBar::activeMenu() || !QSoftMenuBar::activeMenu()->isVisible());
                i += 50, QtUiTest::wait(50))
        {}

        if (!sw->canSelect("Options") && QSoftMenuBar::activeMenu() && QSoftMenuBar::activeMenu()->isVisible()) {
            if (t > 0)
                qWarning("QtUitest: bug?  Took more than one try to open options menu.");
            return true;
        }
    }

    setErrorString("Despite selecting 'Options' from the soft menu several times, "
                   "options menu didn't raise.");
    if (lw) setErrorString(errorString() +
            " Soft menu contents are: " + lw->list().join(","));
    return false;
}

bool hideOptionsMenu()
{
    SelectWidget* sw = qtuitest_cast<SelectWidget*>(findWidget(SoftMenu));
    if (!sw) return false;

    if (!sw->canSelect("Hide") && (!QSoftMenuBar::activeMenu() || !QSoftMenuBar::activeMenu()->isVisible())) return true;

    /* Closing the options menu can cause a menu scroll instead, depending on timing.
     * Try to close a few times.
     */
    static const int MAX_TRIES = 3;
    for (int t = 0; t < MAX_TRIES; ++t) {
        if (!sw->canSelect("Hide") || !sw->select("Hide"))
            return false;

        for (int i = 0;
                i < QtUiTest::maximumUiTimeout() && sw->canSelect("Hide") && QSoftMenuBar::activeMenu() && QSoftMenuBar::activeMenu()->isVisible();
                i += 50, QtUiTest::wait(50))
        {}

        TestWidgetsLog() << t << "activeMenu" << QSoftMenuBar::activeMenu();
        if (!(sw->canSelect("Hide") && QSoftMenuBar::activeMenu() && QSoftMenuBar::activeMenu()->isVisible())) return true;
    }
    return false;
}

bool TestOptionsMenu::canSelect(const QString& item) const
{
    TestWidgetsLog() << item;
    /* Bring up options menu first, if not currently up */
    bool raised = false;
    if (!wrappedObject()) {
        TestWidgetsLog() << item << "Going to raise the options menu.";
        if (!raiseOptionsMenu()) return false;
        raised = true;
        TestWidgetsLog() << item << "Successfully raised the options menu.";
        const_cast<TestOptionsMenu*>(this)->setWrappedObject(QSoftMenuBar::activeMenu());
        TestWidgetsLog() << item << wrappedObject();
        if (!wrappedObject()) return false;
    }

    bool ret = LocalWidget::canSelect(item);

    /* Hide options menu, if we raised it. */
    if (raised) {
        TestWidgetsLog() << item << ret << "Going to hide the options menu.";
        ret = ret && hideOptionsMenu();
        const_cast<TestOptionsMenu*>(this)->setWrappedObject(0);
        TestWidgetsLog() << ret << "after hiding the options menu.";
    }
    return ret;
}

QStringList TestOptionsMenu::list() const
{
    /* Bring up options menu first, if not currently up */
    QStringList ret;
    bool raised = false;
    if (!wrappedObject()) {
        TestWidgetsLog() << "Must raise the options menu to ensure that it exists.";
        if (!raiseOptionsMenu()) return ret;
        raised = true;
        const_cast<TestOptionsMenu*>(this)->setWrappedObject(QSoftMenuBar::activeMenu());
        if (!wrappedObject()) return ret;
    }

    ret = LocalWidget::list();

    /* Hide options menu, if we raised it. */
    if (raised) {
        TestWidgetsLog() << "Raised the options menu and its contents are" << ret;
        hideOptionsMenu();
        const_cast<TestOptionsMenu*>(this)->setWrappedObject(0);
    }
    return ret;
}

bool TestOptionsMenu::select(const QString& item)
{
    TestWidgetsLog() << item;
    /* Bring up options menu first, if not currently up */
    TestWidgetsLog() << item << "Going to raise the options menu (if necessary).";
    if (!raiseOptionsMenu()) return false;
    setWrappedObject(QSoftMenuBar::activeMenu());
    TestWidgetsLog() << item << "Successfully raised the options menu; its contents are now" << list();

    return LocalWidget::select(item);
}

bool TestOptionsMenu::inherits(QtUiTest::WidgetType type) const
{ return LocalWidget::inherits(type) || (QtUiTest::OptionsMenu == type); }

void *TestOptionsMenu::qt_metacast(const char *_clname)
{
    TestWidgetsLog() << _clname;
    if (!_clname) return 0;

    /* Make sure we are wrapping the correct object. */
    QMenu* menu = QSoftMenuBar::activeMenu();
    setWrappedObject((menu && menu->isVisible()) ? menu : 0);

    /* Force a few casts to always succeed. */
    if (!strcmp(_clname, "QtUiTest::SelectWidget"))
	return static_cast< QtUiTest::SelectWidget*>(const_cast< TestOptionsMenu*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.SelectWidget/1.0"))
	return static_cast< QtUiTest::SelectWidget*>(const_cast< TestOptionsMenu*>(this));

    if (!strcmp(_clname, "QtUiTest::ListWidget"))
	return static_cast< QtUiTest::ListWidget*>(const_cast< TestOptionsMenu*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.ListWidget/1.0"))
	return static_cast< QtUiTest::ListWidget*>(const_cast< TestOptionsMenu*>(this));

    return LocalWidget::qt_metacast(_clname);
}

