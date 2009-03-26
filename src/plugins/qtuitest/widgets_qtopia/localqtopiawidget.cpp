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

#include "localqtopiawidget.h"

#include "testcallmanager.h"
#include "testoptionsmenu.h"
#include "testwidgetslog.h"

#include <localwidget.h>

#include <QApplication>
#include <QRect>
#include <QRegion>
#include <QStackedWidget>
#include <QWSInputMethod>
#include <QWidget>
#include <QtopiaInputMethod>

/*!
    Finds and returns a pointer to the widget of \a type in this process.
    Only returns the raw object, does not wrap it in any kind of test widget.
*/
QObject* LocalQtopiaWidget::find(QtUiTest::WidgetType type)
{
    struct Find {
        static QObject* softMenu() {
            if (qApp->type() != qApp->GuiServer)
                return 0;

            static QObject* ret = 0;
            if (!ret) {
                ret = LocalWidget::find("QAbstractContextLabel");
                if (ret) {
                    // Most context labels are placeholder widgets which contain
                    // a themed view.  Return that themed view instead, where
                    // possible.
                    // Works fine if the context label itself is a themed view.
                    QObject* themedView = LocalWidget::find("QThemedView", ret);
                    if (themedView) ret = themedView;
                }
            }
            return ret;
        }

        static QObject* inputMethod() {
            if (qApp->type() != qApp->GuiServer)
                return 0;

            static QObject* selector = 0;
            if (!selector) selector = LocalWidget::find("InputMethodSelector");

            QtopiaInputMethod *qim = 0;

            /*
                FIXME: this could potentially cause an input method to be
                created if one doesn't already exist.
            */

            if (selector)
                if (!QMetaObject::invokeMethod(selector, "current",
                        Q_RETURN_ARG(QtopiaInputMethod*,qim)))
                    qWarning("QtUitest: error getting current input method");

            QObject* ret = 0;
            if (qim) {
                ret = qim->inputWidget();
                if (!ret) ret = qim->inputModifier();
            }

            TestWidgetsLog() << "selector:" << selector << "qim:" << qim << "ret:" << ret;
            return ret;
        }

        static QObject* launcher() {
            if (qApp->type() != qApp->GuiServer)
                return 0;

            struct LauncherFilter {
                static bool filter(QObject const* object)
                {
                    // When rotation is used, multiple launcher views exist.
                    // The active one is whichever one is current in its stack widget.
                    QStackedWidget* stack = qobject_cast<QStackedWidget*>(object->parent());
                    if (stack) {
                        return stack->currentWidget() == object;
                    }
                    return true;
                }
            };

            static QObject* ret = 0;
            if (!ret) ret = LocalWidget::find("PhoneLauncherView", 0, LauncherFilter::filter);
            return ret;
        }

        static QObject* homeScreen() {
            TestWidgetsLog() << "Trying to find homescreen.";
            if (qApp->type() != qApp->GuiServer)
                return 0;

            static QObject* ret = 0;
            if (!ret) ret = LocalWidget::find("QAbstractHomeScreen");
            TestWidgetsLog() << "I think homescreen is" << ret;
            return ret;
        }
    };

    if (QtUiTest::SoftMenu == type) {
        return Find::softMenu();
    }
    if (QtUiTest::InputMethod == type) {
        return Find::inputMethod();
    }
    if (QtUiTest::Launcher == type) {
        return Find::launcher();
    }
    if (QtUiTest::HomeScreen == type) {
        return Find::homeScreen();
    }
    if (QtUiTest::CallManager == type) {
        static TestCallManager cm;
        return &cm;
    }
    if (QtUiTest::OptionsMenu == type) {
        static TestOptionsMenu om;
        return &om;
    }

    return LocalWidget::find(type);
}

