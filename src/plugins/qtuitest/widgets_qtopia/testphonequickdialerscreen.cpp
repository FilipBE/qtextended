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

#include "testphonequickdialerscreen.h"
#include "testwidgetslog.h"
#include "localwidget.h"

using namespace QtUiTest;

TestPhoneQuickDialerScreen::TestPhoneQuickDialerScreen(QObject *_q)
 : TestWidget(_q), numberDisplay(LocalWidget::find("NumberDisplay", _q))
{}

QString TestPhoneQuickDialerScreen::text() const
{
    QString ret;
    if (TextWidget *tw = qtuitest_cast<TextWidget*>(numberDisplay))
        ret = tw->text();
    return ret;
}

bool TestPhoneQuickDialerScreen::canEnter(QVariant const& item) const
{
    if (InputWidget *iw = qtuitest_cast<InputWidget*>(numberDisplay))
        return iw->canEnter(item);
    return false;
}

bool TestPhoneQuickDialerScreen::enter(QVariant const& item, bool noCommit)
{
    if (InputWidget *iw = qtuitest_cast<InputWidget*>(numberDisplay))
        return iw->enter(item, noCommit);
    return false;
}

bool TestPhoneQuickDialerScreen::canWrap(QObject *o)
{ return o && o->inherits("PhoneQuickDialerScreen"); }

