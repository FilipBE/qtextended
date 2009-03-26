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

#include "testphonecalc.h"
#include "testwidgetslog.h"

#include <QVariant>
#include <Qtopia>

using namespace QtUiTest;

TestPhoneCalc::TestPhoneCalc(QObject *_q)
: TestWidget(_q), q(_q)
{
}

bool TestPhoneCalc::canEnter(QVariant const& item) const
{
    bool ok;
    double d = item.toDouble(&ok);

    // Calculator with keypad UI can only handle +ve numbers
    return ok && (d >= 0);
}

bool TestPhoneCalc::enter(QVariant const& item, bool noCommit)
{
    Q_UNUSED(noCommit);
    if (!canEnter(item)) return false;

    TestWidgetsLog() << item;

    Q_ASSERT(!Qtopia::mousePreferred());

    foreach (QChar c, item.toString()) {
        if (c == '.')
            c = '*';
        if (!keyClick(q, asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) ))
            return false;
    }

    return true;
}

bool TestPhoneCalc::canSelect(QString const& item) const
{
    static const QString allowed = "+-/x*=";

    TestWidgetsLog() << item;

    return (item.length() == 1) && (allowed.contains(item[0]));
}

bool TestPhoneCalc::select(QString const& item)
{
    TestWidgetsLog() << item;

    Qt::Key key;

    if (item == "+") {
        key = Qt::Key_Up;
    } else if (item == "-") {
        key = Qt::Key_Down;
    } else if (item == "/") {
        key = Qt::Key_Left;
    } else if (item == "x" || item == "*") {
        key = Qt::Key_Right;
    } else if (item == "=") {
        key = Qt::Key_Select;
    } else
        return false;

    if (!keyClick(q, key)) {
        return false;
    }

    return true;
}

bool TestPhoneCalc::canWrap(QObject *o)
{ return o && o->inherits("FormPhone"); }

