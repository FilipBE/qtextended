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

#include "testcombobox.h"
#include "testwidgetslog.h"

#include <qtuitestnamespace.h>

#include <QComboBox>
#include <QAbstractItemView>

TestComboBox::TestComboBox(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QComboBox*>(_q))
{
    TestWidgetsLog();
    connect(q, SIGNAL(activated(QString)), this, SIGNAL(selected(QString)));
}

QString TestComboBox::text() const
{ TestWidgetsLog(); return list().join("\n"); }

QString TestComboBox::selectedText() const
{ TestWidgetsLog(); return q->currentText(); }

QStringList TestComboBox::list() const
{
    QStringList ret;
    for (int i = 0, max = q->count(); i < max; ++i) {
        ret << q->itemText(i);
    }
    TestWidgetsLog() << ret;
    return ret;
}

QRect TestComboBox::visualRect(QString const &item) const
{
    TestWidgetsLog();
    QRect ret;
    QtUiTest::ListWidget *viewList
        = qtuitest_cast<QtUiTest::ListWidget*>(q->view());
    if (viewList) {
        ret = viewList->visualRect(item);
    }
    return ret;
}

bool TestComboBox::canSelect(QString const &item) const
{ return list().contains(item); }

bool TestComboBox::select(QString const &item)
{
    TestWidgetsLog() << item;
    if (!canSelect(item)) return false;
    if (selectedText() == item) return true;


    QtUiTest::Widget *wView
        = qtuitest_cast<QtUiTest::Widget*>(q->view());
    QtUiTest::SelectWidget *sView
        = qtuitest_cast<QtUiTest::SelectWidget*>(q->view());
    if (!wView || !sView) {
        QtUiTest::setErrorString("Can't find a pointer to the item view for this combobox.");
        return false;
    }

    TestWidgetsLog() << "view isVisible()" << wView->isVisible();

    /* Open the view if it is not currently open. */
    if (!wView->isVisible()) {
        if (QtUiTest::mousePreferred()) {
            /* May take more than one click. */
            for (int i = 0; i < 2 && !wView->isVisible(); ++i) {
                QPoint pos = rect().center();
                TestWidgetsLog() << "local pos" << pos;
                pos = mapToGlobal(pos);
                TestWidgetsLog() << "global pos" << pos;
                TestWidgetsLog() << "Going to click at" << pos << "to open the view";
                QtUiTest::mouseClick(pos, Qt::LeftButton);
            }
            // Combo box tries to detect and ignore "accidental" double clicks,
            // so wait for a bit to ensure any subsequent clicks aren't
            // ignored.
            QtUiTest::wait( qApp->doubleClickInterval() );
        } else {
            TestWidgetsLog() << "Going to give myself focus";
            if (!setFocus()) {
                QtUiTest::setErrorString("Could not give focus to combobox");
                return false;
            }
            QtUiTest::keyClick(QtUiTest::Key_Select);
            if (!q->view()->hasFocus() && !QtUiTest::waitForEvent( q->view(), QEvent::FocusIn)) {
                QtUiTest::setErrorString("Could not give focus to combobox popup.");
                return false;
            }
        }
    }

    /* Select the desired item from the view. */
    TestWidgetsLog() << "Calling select() on the view";
    bool ret = sView->select(item);

    // Wait up to 1 second for the view to disappear after selection.
    for (int i = 0; i < 1000 && wView->isVisible(); i+=100, QtUiTest::wait(100))
    {}

    if (!ret && QtUiTest::errorString().isEmpty()) {
        QtUiTest::setErrorString(
            "Selecting from embedded item view in combo box failed for an unknown reason.");
    }

    return ret;
}

bool TestComboBox::canWrap(QObject *o)
{ return qobject_cast<QComboBox*>(o); }

