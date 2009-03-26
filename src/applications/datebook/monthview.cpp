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
#include "monthview.h"

#include <QPainter>
#include <QResizeEvent>
#include <QTextFormat>
#include <QTimeString>
#include <QTimer>

static QColor normalBgColor(0,0,0);
static bool foundNColor = false;

MonthView::MonthView(QWidget *parent, const QCategoryFilter& c, QSet<QPimSource> set)
    : QCalendarWidget(parent)
{
    setObjectName("monthview");

    setVerticalHeaderFormat(NoVerticalHeader);
    setFirstDayOfWeek( Qtopia::weekStartsOnMonday() ? Qt::Monday : Qt::Sunday );

    QDate start = QDate::currentDate();
    start.setYMD(start.year(), start.month(), 1);
    QDate end = start.addDays(start.daysInMonth() - 1);

    model = new QOccurrenceModel(QDateTime(start, QTime(0, 0, 0)), QDateTime(end.addDays(1), QTime(0, 0)), this);
    if (set.count() > 0)
        model->setVisibleSources(set);
    model->setCategoryFilter(c);

    connect(model, SIGNAL(modelReset()), this, SLOT(resetFormatsSoon()));
    connect(this, SIGNAL(currentPageChanged(int,int)), this, SLOT(updateModelRange(int,int)));

    // Since we don't know if we'll get a model reset from the model
    // at startup, force a timer
    dirtyTimer = new QTimer();
    dirtyTimer->setSingleShot(true);
    dirtyTimer->setInterval(0);
    connect(dirtyTimer, SIGNAL(timeout()), this, SLOT(resetFormats()));

    resetFormatsSoon();

    // XXX find the QCalendarView class so we can handle Key_Back properly :/
    // [this comes from qtopiaapplication.cpp]
    QWidget *table = findChild<QWidget*>("qt_calendar_calendarview");
    if (table)
        table->installEventFilter(this);
}

MonthView::~MonthView()
{
}

void MonthView::paintEvent(QPaintEvent* p)
{
    resetFormats();
    QCalendarWidget::paintEvent(p);
}

void MonthView::resetFormatsNow()
{
    dirtyModel = true;
    resetFormats();
}

void MonthView::resetFormatsSoon()
{
    dirtyModel = true;
    dirtyTimer->start();
    update();
}

void MonthView::resetFormats() const
{
    if (dirtyModel) {
        dirtyModel = false;
        const_cast<MonthView*>(this)->setDateTextFormat(QDate(),QTextFormat().toCharFormat()); // clear formats
        for (int i = 0; i < model->rowCount(); ++i) {
            // get just the data needed for drawing.
            QDateTime f = model->data(model->index(i, QAppointmentModel::Start), Qt::EditRole).toDateTime();
            QDateTime t = model->data(model->index(i, QAppointmentModel::End), Qt::EditRole).toDateTime();
            bool isAllDay = model->data(model->index(i, QAppointmentModel::AllDay), Qt::EditRole).toBool();

            if (!foundNColor) {
                normalBgColor = palette().button().color();
                foundNColor = true;
            }

            for (QDate i = f.date(); i <= t.date(); i = i.addDays(1)) {
                // get item.
                QTextCharFormat fmt = dateTextFormat(i);
                bool set=false;

                if (isAllDay) {
                    fmt.setBackground(normalBgColor);
                    set = true;
                } else {
                    // Weed out things that end at midnight (e.g should be previous day)
                    if (t != QDateTime(t.date()) || i != t.date()) {
                        fmt.setFontWeight(QFont::Bold);
                        set = true;
                    }
                }

                if ( set )
                    const_cast<MonthView*>(this)->setDateTextFormat(i,fmt);
            }
        }
    }
}

void MonthView::updateModelRange(int year, int month)
{
    QDate start(year, month, 1);
    QDate end = start.addDays(start.daysInMonth() - 1);

    model->setRange(QDateTime(start, QTime(0, 0, 0)), QDateTime(end.addDays(1), QTime(0, 0)));
}

void MonthView::categorySelected( const QCategoryFilter &c )
{
    model->setCategoryFilter( c );
}

void MonthView::setVisibleSources( QSet<QPimSource> set)
{
    model->setVisibleSources(set);
}

bool MonthView::eventFilter(QObject *o, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Back) {
            emit closeView();
            ke->accept();
            return true;
        }
    }
    return QCalendarWidget::eventFilter(o, e);
}

