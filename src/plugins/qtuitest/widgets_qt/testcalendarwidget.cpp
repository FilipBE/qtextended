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

#include "testcalendarwidget.h"
#include "testwidgetslog.h"

#include <qtuitestnamespace.h>

#include <QAbstractItemView>
#include <QCalendarWidget>
#include <QMenu>
#include <QSpinBox>
#include <QToolButton>

/* FIXME replace this with canSelect */
QString const GetListRegExp = QString("QTUITEST_REGEX_");

TestCalendarWidget::TestCalendarWidget(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QCalendarWidget*>(_q))
{}

QStringList TestCalendarWidget::list() const
{
    QStringList ret;
    QtUiTest::ListWidget *calendarView
        = qtuitest_cast<QtUiTest::ListWidget*>(
                q->findChild<QAbstractItemView*>());

    if (!calendarView) {
        return ret;
    }

    ret = calendarView->list();

    /* Append months */
    QStringList months;
    for (int i = 1; i <= 12; ++i) {
        months << QDate::longMonthName(i);
    }
    ret << months;

    /* Append years */
    /* FIXME replace this with canSelect() */
    ret << (GetListRegExp + "[0-9]{4}");

    return ret;
}

/* FIXME make this less of a hack. */
QRect TestCalendarWidget::visualRect(QString const &item) const
{
    TestWidgetsLog() << item << "my geometry is" << geometry();

    QRect ret;

    QAbstractItemView *view = q->findChild<QAbstractItemView*>();
    QtUiTest::ListWidget *calendarView
        = qtuitest_cast<QtUiTest::ListWidget*>(view);

    if (!calendarView) {
        return ret;
    }

    ret = calendarView->visualRect(item);
    if (!ret.isNull()) {
        ret.moveTopLeft( q->mapFromGlobal( view->mapToGlobal(ret.topLeft()) ) );
        TestWidgetsLog() << item << "is a visible day at" << ret;
        return ret;
    }

    QToolButton *yearButton = 0;
    QToolButton *monthButton = 0;
    QSpinBox *yearSpin = q->findChild<QSpinBox*>();
    QMenu *monthMenu = 0;

    QList<QToolButton*> blist = q->findChildren<QToolButton*>();
    foreach(QToolButton *b, blist) {
        if (!monthButton && (monthMenu = b->menu())) {
            monthButton = b;
        }
        if (!b->menu()) {
            yearButton = b;
        }
    }
    TestWidgetsLog() << "monthButton" << monthButton << "yearButton" << yearButton;
    TestWidgetsLog() << "item" << item << "monthMenu" << monthMenu;

    if (yearButton && yearButton->isVisible() && yearButton->text() == item) {
        QPoint p = q->mapFromGlobal( yearButton->mapToGlobal(QPoint(yearButton->width()+5, yearButton->height()/2)) );
        ret = QRect(p.x() - 2, p.y() - 2, 5, 5);
        TestWidgetsLog() << "click near yearbutton";
    } else if (yearSpin && yearSpin->isVisible() && yearSpin->value() == item.toInt()) {
        TestWidgetsLog() << "confirm spinbox";
        QPoint p = q->mapFromGlobal( yearSpin->mapToGlobal(QPoint(yearSpin->width()+5, yearSpin->height()/2)) );
        ret = QRect(p.x() - 2, p.y() - 2, 5, 5);
    } else if (monthButton && monthButton->isVisible() && monthButton->text() == item) {
        QPoint p = q->mapFromGlobal( monthButton->mapToGlobal(QPoint(-5, monthButton->height()/2)) );
        ret = QRect(p.x() - 2, p.y() - 2, 5, 5);
        TestWidgetsLog() << "click near monthbutton";
    } else if (monthMenu && monthMenu->isVisible()
            && qtuitest_cast<QtUiTest::ListWidget*>(monthMenu)
            && qtuitest_cast<QtUiTest::ListWidget*>(monthMenu)->list().contains(item)) {
        ret = qtuitest_cast<QtUiTest::ListWidget*>(monthMenu)->visualRect(item);
        ret.moveTopLeft( q->mapFromGlobal( monthMenu->mapToGlobal(ret.topLeft()) ) );
        TestWidgetsLog() << "click on monthmenu";
    } else {
        do {
            QStringList items = list();
            if (items.contains(item)) {
                ret = QRect(-1, -1, 1, 1);
                ret.moveTopLeft( q->mapFromGlobal(QPoint(-1,-1)) );
                break;
            }
            foreach (QString s, items) {
                if (!s.startsWith(GetListRegExp)) continue;
                QRegExp re(s.mid(GetListRegExp.length()));
                if (re.exactMatch(item)) {
                    ret = QRect(-1, -1, 1, 1);
                    ret.moveTopLeft( q->mapFromGlobal(QPoint(-1,-1)) );
                    break;
                }
            }
            if (!ret.isNull()) break;
        } while(0);
    }

    TestWidgetsLog() << "returning rect" << ret;

    return ret;
}

/* FIXME implement these. */
bool TestCalendarWidget::canSelect(QString const&) const
{ return false; }

bool TestCalendarWidget::select(QString const&)
{ return false; }

bool TestCalendarWidget::ensureVisible(QString const&)
{ return false; }

bool TestCalendarWidget::canWrap(QObject *o)
{ return qobject_cast<QCalendarWidget*>(o); }


