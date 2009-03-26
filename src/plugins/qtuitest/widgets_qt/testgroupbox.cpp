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

#include "testgroupbox.h"
#include "testwidgetslog.h"

#include <QGroupBox>

TestGroupBox::TestGroupBox(QObject *_q)
    : TestGenericTextWidget(_q), q(qobject_cast<QGroupBox*>(_q))
{
    connect(q, SIGNAL(toggled(bool)), this, SLOT(on_toggled(bool)));
}

void TestGroupBox::on_toggled(bool state)
{ emit stateChanged(state); }

Qt::CheckState TestGroupBox::checkState() const
{ return q->isChecked() ? Qt::Checked : Qt::Unchecked; }

bool TestGroupBox::setCheckState(Qt::CheckState state)
{
    TestWidgetsLog() << state << checkState() << q->isCheckable();
    if (state == checkState()) return true;
    if (!q->isCheckable()) return false;

    if (QtUiTest::mousePreferred()) {
        // FIXME rewrite this to take styles into account.
        QPoint p(15, 15);
        if (!ensureVisiblePoint(p)) return false;
        if (!QtUiTest::mouseClick(q, SIGNAL(toggled(bool)), mapToGlobal( p ) )) return false;
    } else {
        if (!setFocus()) {
            QtUiTest::setErrorString("Couldn't toggle group box check state: couldn't give "
                    "focus to group box.");
            return false;
        }
        if (!QtUiTest::keyClick(q, SIGNAL(toggled(bool)), QtUiTest::Key_Activate )) return false;
    }

    return state == checkState();
}

bool TestGroupBox::canWrap(QObject *o)
{ return qobject_cast<QGroupBox*>(o); }

