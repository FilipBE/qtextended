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

#include "testnumberdisplay.h"
#include "testwidgetslog.h"

#include <QVariant>
#include <Qtopia>

using namespace QtUiTest;
static const char starKey = '*';

TestNumberDisplay::TestNumberDisplay(QObject *_q)
: TestWidget(_q), q(_q)
{
    connect(q, SIGNAL(numberChanged(QString)),
            this, SLOT(onNumberChanged(QString)));
}

void TestNumberDisplay::onNumberChanged(QString const& number)
{ emit entered(number); }

QString TestNumberDisplay::text() const
{
    QString ret;
    QMetaObject::invokeMethod(q, "wildcardNumber", Qt::DirectConnection,
            Q_RETURN_ARG(QString, ret));
    if (!ret.trimmed().isEmpty()) return ret;
    QMetaObject::invokeMethod(q, "number", Qt::DirectConnection,
            Q_RETURN_ARG(QString, ret));
    TestWidgetsLog() << q << ret;
    return ret;
}

bool TestNumberDisplay::canEnter(QVariant const& item) const
{
    if (!item.canConvert<QString>()) return false;

    static const QString allowed = "0123456789*#+pw";
    foreach (QChar c, item.toString())
        if (!allowed.contains(c)) return false;
    return true;
}

bool TestNumberDisplay::enter(QVariant const& item, bool noCommit)
{
    Q_UNUSED(noCommit);
    if (!canEnter(item)) return false;

    TestWidgetsLog() << item;

    Q_ASSERT(!Qtopia::mousePreferred());

    static const QString asteriskKey = "*+pw";

    foreach (QChar c, item.toString()) {
        if (asteriskKey.contains(c)) {
            int presses = asteriskKey.indexOf(c)+1;
            for (int i = 0; i < presses; ++i) {
                keyClick( asciiToKey(starKey), asciiToModifiers(starKey) );
            }
        } else {
            keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
        }
    }

    return true;
}

bool TestNumberDisplay::canWrap(QObject *o)
{ return o && o->inherits("NumberDisplay"); }

