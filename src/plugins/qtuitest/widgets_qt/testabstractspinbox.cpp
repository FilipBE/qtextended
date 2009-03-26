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

#include "testabstractspinbox.h"
#include "testwidgetslog.h"

#include <QPointer>
#include <QSpinBox>
#include <QValidator>
#include <QVariant>

TestAbstractSpinBox::TestAbstractSpinBox(QObject *_q)
    : TestGenericTextWidget(_q), q(qobject_cast<QAbstractSpinBox*>(_q))
{
//TestWidgetsLog();
}

bool TestAbstractSpinBox::canEnter(QVariant const& item) const
{
    if (!item.canConvert<QString>()) return false;

    int dontcare = 0;
    QString text = item.toString();
    return (QValidator::Acceptable==q->validate(text, dontcare));
}

bool TestAbstractSpinBox::enter(QVariant const& item, bool noCommit)
{
    if (!canEnter(item)) return false;

    if (!hasFocus() && !setFocus()) return false;

    using namespace QtUiTest;

    QPointer<QObject> safeThis = this;

    QObject *inputMethod = findWidget(InputMethod);
    InputWidget* iw = qtuitest_cast<InputWidget*>(inputMethod);

    if (!safeThis) {
        setErrorString("Widget was destroyed while entering text.");
        return false;
    }

    TestWidgetsLog() << iw;

    bool ret = false;

    /* Use input method if available... */
    if (iw) {
        ret = iw->canEnter(item) && iw->enter(item, noCommit);
    } else {
        /* ...otherwise, generate key clicks ourself */
        foreach (QChar c, item.toString()) {
            TestWidgetsLog() << asciiToModifiers(c.toLatin1());
            keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
        }
    }

    if (!safeThis) {
        setErrorString("Widget was destroyed while entering text.");
        return false;
    }

    if (mousePreferred()) {
        return ret;
    }

    if (!noCommit) {
        keyClick(QtUiTest::Key_Activate);
    }

    return ret;
}

bool TestAbstractSpinBox::canWrap(QObject *o)
{ return qobject_cast<QAbstractSpinBox*>(o); }

