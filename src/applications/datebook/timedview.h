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

#ifndef TIMEDVIEW_H
#define TIMEDVIEW_H

#include <QRect>
#include <QDateTime>
#include <QWidget>
#include <qappointment.h>
#include <qappointmentmodel.h>

class QItemSelectionModel;
class QAbstractItemDelegate;

class TimeManagerData;
class TimeManager : public QWidget
{
public:
    TimeManager(QWidget *parent = 0);
    virtual ~TimeManager();

    virtual int minimumGapHeight() const;
    int minimumHeight() const;

    virtual void resetMarks();
    virtual void clearMarks();
    virtual void populateMarks();
    virtual void addMark(int minutes);

    virtual int markPosition(int minutes) const;
    virtual int markMinutes(int position, int direction = 0) const;

    QList<int> marks() const;

    QSize minimumSizeHint() const;

    void setDaySpan(int, int);

    int dayStart() const;
    int dayEnd() const;

    void cacheLayout();

protected:
    // to pick up font changes
    void changeEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    void drawFiller(QPainter *, const QRect &);
    void drawItem(QPainter *, const QRect &, int minutes, int markpos);

    TimeManagerData *d;
};

class CompressedTimeManagerData;
class CompressedTimeManager : public TimeManager
{
public:
    CompressedTimeManager(QWidget *parent = 0);
    void populateMarks();
    void addMark(int minutes);
    void clearMarks();

    void setIdealHeight(int ideal);
    int idealHeight() const;

private:
    CompressedTimeManagerData *d;
};

class TimedViewData;
class TimedView : public QWidget
{
    Q_OBJECT

public:
    TimedView(QWidget *parent = 0);
    virtual ~TimedView();

    virtual void setDate(const QDate &d);
    QDate date() const;

    virtual void setDaySpan(int start, int end);

    virtual QDateTime start() const;
    virtual QDateTime end() const;

    void setModel(QOccurrenceModel *);
    void setTimeManager(TimeManager *);

    QOccurrenceModel *model() const;
    TimeManager *timeManager() const;

    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex &);

    QModelIndex index(const QPoint& point) const;
    QDateTime timeAtPoint(const QPoint& globalPoint, int direction = 0) const;

    void setItemDelegate(QAbstractItemDelegate *);
    QAbstractItemDelegate *itemDelegate() const;

    QRect occurrenceRect(const QModelIndex &index) const;

signals:
    void selectionChanged(const QModelIndex&);

public slots:
    void reset();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    TimedViewData *d;
};
#endif
