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

#include "testqtopiafactory.h"

#include "localqtopiawidget.h"
#include "remotewidget.h"
#include "remotewidgetadaptor.h"
#include "testcontextlabel.h"
#include "testdialer.h"
#include "testiconselector.h"
#include "testnumberdisplay.h"
#include "testphonecalc.h"
#include "testphonelauncherview.h"
#include "testphonequickdialerscreen.h"
#include "testphonetouchdialerscreen.h"
#include "testpkim.h"
#include "testpredictivekeyboard.h"
#include "testsmoothlist.h"
#include "testthemedhomescreen.h"
#include "testthemedview.h"
#include "testthemelistmodel.h"

#include "testwidgetslog.h"

#include <QApplication>

TestQtopiaWidgetsFactory::TestQtopiaWidgetsFactory()
{
    RemoteWidgetAdaptor::instance();
}

QObject* TestQtopiaWidgetsFactory::find(QtUiTest::WidgetType type)
{
    if (QObject *o = LocalQtopiaWidget::find(type))
        return o;

    if (qApp->type() != QApplication::GuiServer)
        return RemoteWidget::find(type);

    return 0;
}

QObject* TestQtopiaWidgetsFactory::create(QObject* o)
{
    QObject* ret = 0;

#define TRY(Klass)           \
    if (Klass::canWrap(o)) { \
        ret = new Klass(o);  \
        break;               \
    }

    /* Order is important here; classes should be listed in order of
     * most to least derived. */

    do {
        /* Server widgets */
        TRY(TestPhoneLauncherView);
        TRY(TestNumberDisplay);
        TRY(TestContextLabel);
        TRY(TestThemedHomeScreen);
        TRY(TestPhoneQuickDialerScreen);
        TRY(TestPhoneTouchDialerScreen);
        TRY(TestDialer);
        TRY(TestPhoneCalc);

        /* Input method widgets */
        TRY(TestKeyboardWidget);
        TRY(TestPkIM);

        /* Qt Extended widgets */
        TRY(TestThemedView);
        TRY(TestThemeListModel);
        TRY(TestSmoothList);
        TRY(TestIconSelector);
    } while(0);

    TestWidgetsLog() << o << ret;

    return ret;
}

QStringList TestQtopiaWidgetsFactory::keys() const
{
    /* Order doesn't matter here. */
    return QStringList()
        << "ContextLabel"
        << "Dialer"
        << "FormPhone"
        << "NumberDisplay"
        << "PhoneLauncher"
        << "PhoneMainMenu"
        << "PhoneQuickDialerScreen"
        << "PhoneTouchDialerScreen"
        << "PkIM"
        << "QSmoothList"
        << "QThemeListModel"
        << "QThemedView"
        << "ThemeListModel"
        << "ThemedHomeScreen"
        << "ThemedView"
        ;
}

#include <qplugin.h>
Q_EXPORT_PLUGIN2(qtuitestwidgets, TestQtopiaWidgetsFactory)

