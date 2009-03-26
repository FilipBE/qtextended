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

#include "testpushbutton.h"
#include "testwidgetslog.h"

#include <QMenu>
#include <QPushButton>

TestPushButton::TestPushButton(QObject *_q)
    : TestAbstractButton(_q), q(qobject_cast<QPushButton*>(_q))
{}

QStringList TestPushButton::list() const
{
    QStringList ret;

    QtUiTest::ListWidget *menu
        = qtuitest_cast<QtUiTest::ListWidget*>(q->menu());

    if (menu)
        ret = menu->list();

    return ret;
}

QRect TestPushButton::visualRect(QString const &item) const
{
    TestWidgetsLog();
    QRect ret;
    QtUiTest::ListWidget *lMenu
        = qtuitest_cast<QtUiTest::ListWidget*>(q->menu());

    if (lMenu)
        ret = lMenu->visualRect(item);
    return ret;
}

bool TestPushButton::ensureVisible(QString const& item)
{
    bool ret = false;

    QtUiTest::ListWidget *menu
        = qtuitest_cast<QtUiTest::ListWidget*>(q->menu());

    if (menu)
        ret = menu->ensureVisible(item);

    return ret;
}

bool TestPushButton::canSelect(QString const &item) const
{ return list().contains(item); }

bool TestPushButton::select(QString const &item)
{
    bool ret = false;

    QtUiTest::SelectWidget *menu
        = qtuitest_cast<QtUiTest::SelectWidget*>(q->menu());

    if (menu)
    {
        activate();
        ret = menu->select(item);
    }

    return ret;
}

bool TestPushButton::canWrap(QObject *o)
{ return qobject_cast<QPushButton*>(o); }

