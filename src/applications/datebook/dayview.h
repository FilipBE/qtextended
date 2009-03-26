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
#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <qappointment.h>
#include <qappointmentmodel.h>

#include <QDate>
#include <QWidget>

#include "timedview.h"
#include "appointmentlist.h"

class QLabel;
class QScrollArea;
class QPushButton;

namespace DuplicatedFromCalendarWidget 
{
    class QCalendarTextNavigator;
};

class DayView : public QWidget
{
    Q_OBJECT

public:
    DayView(QWidget *parent = 0, const QCategoryFilter& c = QCategoryFilter(), QSet<QPimSource> set = QSet<QPimSource>());

    QModelIndex currentIndex() const;
    QAppointment currentAppointment() const;
    QOccurrence currentOccurrence() const;

    QDate currentDate() const;

    bool allDayFolded() const;
    bool allDayFoldingAvailable() const;
    void setAllDayFolded(bool f);

public slots:

    void selectDate( const QDate & );
    void selectDate( int year, int month );

    void setDaySpan( int starthour, int endhour );

    void setVisibleSources(QSet<QPimSource> set);

    void firstTimed();
    void firstAllDay();
    void lastTimed();
    void lastAllDay();
    void nextOccurrence();
    void previousOccurrence();

    void prevDay();
    void nextDay();

    //void setCurrentIndex(const QModelIndex &);
    void setCurrentOccurrence(const QOccurrence &o);
    void setCurrentAppointment(const QAppointment &a);

    void timedSelectionChanged(const QModelIndex &index);
    void allDayOccurrenceChanged(const QModelIndex &index);
    void allDayOccurrenceActivated(const QModelIndex &index);

    void updateHiddenIndicator(int hidden);

    void modelsReset();
    void categorySelected( const QCategoryFilter &c );

private slots:
    void updateView();

signals:
    void removeOccurrence( const QOccurrence& );
    void editOccurrence( const QOccurrence& );
    void removeAppointment( const QAppointment& );
    void editAppointment( const QAppointment& );
    void beamAppointment( const QAppointment& );
    void newAppointment();
    void newAppointment( const QString & );
    void newAppointment( const QDateTime &dstart, const QDateTime &dend );
    void showDetails();
    void dateChanged();
    void selectionChanged();
    void closeView();

protected:
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent( QMouseEvent * event );
    bool eventFilter(QObject *o, QEvent *e);
    void resizeEvent(QResizeEvent *);
    bool event(QEvent *event);

private:
    void updateHeaderText();
    QOccurrenceModel *currentModel() const;

    QPushButton *mNextDay, *mPrevDay;

    QLabel *mWeekdayLabel;
    QLabel *mDateLabel;
    TimedView *mTimedView;
    AppointmentList *mAllDayList;
    QLabel *mHiddenIndicator;

    QOccurrenceModel *timedModel;
    QOccurrenceModel *allDayModel;
    CompressedTimeManager *mTimeManager;
    QScrollArea *mScrollArea;

    QUniqueId lastSelectedTimedId;
    QUniqueId lastSelectedAllDayId;

    QDate targetDate;
    QTimer *cacheDelayTimer;
    DuplicatedFromCalendarWidget::QCalendarTextNavigator *nav;

    bool allDayFocus;
};


#endif
