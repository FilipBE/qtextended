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

#include "testpkim.h"
#include "testwidgetslog.h"

#include <QVariant>
#include <QWSServer>
#include <QWSWindow>
#include <QtopiaIpcEnvelope>

TestPkIM::TestPkIM(QObject *_q)
    : q(_q)
{}

bool TestPkIM::canEnter(QVariant const& item) const
{
    return item.canConvert<QString>();
}

/*
    FIXME: implement this function to properly use PkIM.
    Currently it bypasses the input methods entirely.
*/
bool TestPkIM::enter(QVariant const& item, bool noCommit)
{
    Q_UNUSED(noCommit);
    if (!canEnter(item)) return false;

    QString text = item.toString();

    foreach (QWSWindow* win, QWSServer::instance()->clientWindows()) {
        QtopiaIpcEnvelope env("QPE/InputMethod", "inputMethodHint(QString,int)");
        env << QString() << win->winId();
    }
    /* Wait for inputMethodHint to take effect. */
    QtUiTest::wait(100);

    using namespace QtUiTest;
    foreach (QChar c, item.toString()) {
        TestWidgetsLog() << asciiToModifiers(c.toLatin1());
        keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
    }

    return true;
}

bool TestPkIM::canWrap(QObject *o)
{ return o->inherits("PkIM"); }

