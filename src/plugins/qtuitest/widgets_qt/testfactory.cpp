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

#include "testfactory.h"

#include "localwidget.h"
#include "testabstractbutton.h"
#include "testabstractitemmodel.h"
#include "testabstractitemview.h"
#include "testabstractspinbox.h"
#include "testcalendarwidget.h"
#include "testcheckbox.h"
#include "testcombobox.h"
#include "testdateedit.h"
#include "testdatetimeedit.h"
#include "testgenericcheckwidget.h"
#include "testgenerictextwidget.h"
#include "testgroupbox.h"
#include "testlabel.h"
#include "testlineedit.h"
#include "testmenu.h"
#include "testpushbutton.h"
#include "testtabbar.h"
#include "testtextedit.h"
#include "testtimeedit.h"
#include "testwidget.h"

#include "testwidgetslog.h"

#include <QApplication>

TestWidgetsFactory::TestWidgetsFactory()
{
}

QObject* TestWidgetsFactory::find(QtUiTest::WidgetType type)
{
    if (QObject *o = LocalWidget::find(type))
        return o;

    return 0;
}

QObject* TestWidgetsFactory::create(QObject* o)
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
        /* Qt widgets */
        TRY(TestDateEdit);
        TRY(TestTimeEdit);
        TRY(TestDateTimeEdit);
        TRY(TestCalendarWidget);
        TRY(TestDateTimeEdit);
        TRY(TestAbstractItemModel);
        TRY(TestAbstractItemView);
        TRY(TestTabBar);
        TRY(TestMenu);
        TRY(TestLabel);
        TRY(TestAbstractSpinBox);
        TRY(TestGroupBox);
        TRY(TestCheckBox);
        TRY(TestComboBox);
        TRY(TestLineEdit);
        TRY(TestTextEdit);
        TRY(TestPushButton);
        TRY(TestAbstractButton);
        TRY(TestGenericCheckWidget);
        TRY(TestGenericTextWidget);
        TRY(TestWidget);
    } while(0);

    TestWidgetsLog() << o << ret;

    return ret;
}

QStringList TestWidgetsFactory::keys() const
{
    /* Order doesn't matter here. */
    return QStringList()
        << "QAbstractButton"
        << "QAbstractItemModel"
        << "QAbstractItemView"
        << "QAbstractSlider"
        << "QAbstractSpinBox"
        << "QCalendarWidget"
        << "QCheckBox"
        << "QComboBox"
        << "QDateEdit"
        << "QDateTimeEdit"
        << "QGroupBox"
        << "QLCDNumber"
        << "QLabel"
        << "QLineEdit"
        << "QMenu"
        << "QMessageBox"
        << "QProgressBar"
        << "QPushButton"
        << "QTabBar"
        << "QTextEdit"
        << "QTimeEdit"
        << "QWidget"
        ;
}

#include <qplugin.h>
Q_EXPORT_PLUGIN2(qttestwidgets, TestWidgetsFactory)

