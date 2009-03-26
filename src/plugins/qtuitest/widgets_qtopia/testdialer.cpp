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

#include "testdialer.h"
#include "testwidgetslog.h"
#include "localwidget.h"

#include <Qtopia>
#include <QVariant>
#include <ThemedView>

using namespace QtUiTest;

struct DialerMap {
    static QString charToLabel(QChar const& c) {
        if ('1' == c) return "one";
        if ('2' == c) return "two";
        if ('3' == c) return "three";
        if ('4' == c) return "four";
        if ('5' == c) return "five";
        if ('6' == c) return "six";
        if ('7' == c) return "seven";
        if ('8' == c) return "eight";
        if ('9' == c) return "nine";
        if ('*' == c) return "star";
        if ('0' == c) return "zero";
        if ('#' == c) return "hash";
        return QString();
    }
};

TestDialer::TestDialer(QObject *_q)
    : TestThemedView(_q)
{
    QObject* dialerLe = LocalWidget::find("DialerLineEdit", _q);
    if (dialerLe)
        connect(dialerLe, SIGNAL(textEdited(QString)),
                this, SLOT(on_textEdited(QString)));
}

void TestDialer::on_textEdited(QString const& item)
{ emit entered(item); }

bool TestDialer::canEnter(QVariant const& item) const
{
    TestWidgetsLog() << item;
    QString str = item.toString();
    foreach (QChar c, str) {
        if (DialerMap::charToLabel(c).isEmpty())
            return false;
    }
    return true;
}

bool TestDialer::enter(QVariant const& item, bool noCommit)
{
    Q_UNUSED(noCommit);
    TestWidgetsLog() << item;
    if (!canEnter(item)) return false;

    if (!Qtopia::mousePreferred()) {
        foreach (QChar c, item.toString()) {
            keyClick(asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()));
        }
        return true;
    }

    foreach (QChar c, item.toString()) {
        QPoint p = visualRect( DialerMap::charToLabel(c) ).center();
        TestWidgetsLog() << "point for" << c << "is" << mapToGlobal(p);
        if (p.isNull()) return false;
        mouseClick( mapToGlobal(p) );
    }
    return true;
}

bool TestDialer::canWrap(QObject *o)
{ return o && o->inherits("Dialer"); }

