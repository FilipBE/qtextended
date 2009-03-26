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

#include "testabstractbutton.h"
#include "testwidgetslog.h"

#include <QAbstractButton>
#include <QTimer>
#include <QTime>

TestAbstractButton::TestAbstractButton(QObject *_q)
    : TestGenericTextWidget(_q), q(qobject_cast<QAbstractButton*>(_q)),
      m_pressed(false)
{
    if (!connect(q, SIGNAL(pressed()), this, SIGNAL(activated())))
        Q_ASSERT(false);
    if (!connect(q, SIGNAL(toggled(bool)), this, SLOT(on_toggled(bool))))
        Q_ASSERT(false);
}

void TestAbstractButton::on_toggled(bool state)
{ emit stateChanged(state); }

Qt::CheckState TestAbstractButton::checkState() const
{ return q->isChecked() ? Qt::Checked : Qt::Unchecked; }

bool TestAbstractButton::setCheckState(Qt::CheckState state)
{
    if (state == checkState()) return true;
    if (!q->isCheckable()) {
        QtUiTest::setErrorString("This abstract button is not checkable.");
        return false;
    }
    bool ret = activate(QtUiTest::NoOptions);
    TestWidgetsLog() << "activated:" << ret;
    if (ret && (state != checkState()) && !QtUiTest::waitForSignal(q, SIGNAL(toggled(bool)))) {
        QtUiTest::setErrorString("Successfully activated button, but check state did not change "
                "to expected value.");
        return false;
    }
    TestWidgetsLog() << "state:" << ret;
    return ret;
}

bool TestAbstractButton::activate()
{
    return activate((QtUiTest::InputOption)0);
}

bool TestAbstractButton::activate(QtUiTest::InputOption opt)
{
    if (QtUiTest::mousePreferred()) {
        /* Make sure button can respond to press. */
        int t;
        m_pressed = false;
        QPoint p;
        for (t = 0; t < 2; ++t) {
            // FIXME rewrite this to take styles into account.
            if (q->isCheckable())
                p = QPoint(15,15);
            else
                p = visibleRegion().boundingRect().center();
            ensureVisiblePoint(p);

            QtUiTest::mousePress(mapToGlobal(p), Qt::LeftButton, opt);
            QtUiTest::waitForSignal(q, SIGNAL(pressed()));
            if (q->isDown())
                break;
        }
        if (!q->isDown()) {
            qWarning("QtUitest: bug?  Button did not react to mouse press.");
            return false;
        }
        if (t > 0)
            qWarning("QtUitest: bug?  Button did not react to mouse "
                     "press on first try.");
        QtUiTest::mouseClick(mapToGlobal(p), Qt::LeftButton, opt);
        return true;
    } else {
        Qt::Key key = Qt::Key(QtUiTest::Key_ActivateButton);
        Qt::KeyboardModifiers mod = 0;

        if (q->focusPolicy() == Qt::NoFocus) {
            /* Handle buttons which need to be activated by a shortcut */
            QKeySequence ks = q->shortcut();
            TestWidgetsLog() << "Can't give focus to button; need to use shortcut"
                    << ks.toString();
            if (ks.isEmpty()) {
                QtUiTest::setErrorString("Button has NoFocus policy set and does not appear "
                        "to have any shortcut set.  Therefore, it is impossible to activate "
                        "using only the keyboard.");
                return false;
            }

            int key_and_mod = ks[0];
            mod = QFlag(key_and_mod & (0xfe000000));
            key = Qt::Key(key_and_mod & (0x01ffffff));
        }
        else {
            if (!hasFocus()) setFocus();
            if (!hasFocus()) return false;
        }
        if (!QtUiTest::keyClick(q, SIGNAL(pressed()), key, mod, opt))
            return false;
        if (q->isDown() && !QtUiTest::waitForSignal(q, SIGNAL(released()))) {
            QtUiTest::setErrorString("Button did not become released after key click");
            return false;
        }
        return true;
    }
    return false;
}

bool TestAbstractButton::canWrap(QObject *o)
{ return qobject_cast<QAbstractButton*>(o); }

