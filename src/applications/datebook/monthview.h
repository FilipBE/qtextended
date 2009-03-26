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
#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include <qappointment.h>
#include <qappointmentmodel.h>

#include <QDateTime>
#include <QCalendarWidget>

class QTimer;

class MonthView : public QCalendarWidget
{
    Q_OBJECT

public:
    MonthView(QWidget *parent = 0, const QCategoryFilter& c = QCategoryFilter(), QSet<QPimSource> set = QSet<QPimSource>());
    ~MonthView();

    bool eventFilter(QObject *o, QEvent *e);

signals:
    void closeView();

public slots:
    void updateModelRange(int year, int month);
    void categorySelected( const QCategoryFilter &c );
    void setVisibleSources( QSet<QPimSource> set);

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void resetFormatsNow();
    void resetFormatsSoon();
    void resetFormats() const;

private:
    QList<QOccurrence> daysAppointments;

    QOccurrenceModel *model;

    mutable QTimer *dirtyTimer;
    mutable bool dirtyModel;
};

#endif
