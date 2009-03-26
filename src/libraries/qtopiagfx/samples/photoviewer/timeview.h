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

#ifndef TIMEVIEW_H
#define TIMEVIEW_H

#include "gfxcanvas.h"
#include <QObject>
#include <QVector>
#include <QHash>

class ImageCollection;
class TimeLineBar;


class DateEntry : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    DateEntry(GfxCanvasItem *);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

signals:
    void dateChanged(const QDate &);
    void dateSelected(const QDate &);

private:
    void setDate(const QDate &);
    void setItem(int, const QString &);
    GfxCanvasRoundedRect *focus[3];
    GfxCanvasText *txt[3];
    int focused;
    GfxTimeLine tl;
    QDate date;
};


class TimeView : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    TimeView(GfxCanvasItem *);

    virtual void keyReleaseEvent(QKeyEvent *);
    virtual GfxCanvasItem *focusProxy();

public slots:
    void showDateSelector();

private slots:
    void dayChanged(const QDate &d);
    void daySelected(const QDate &);
    void entrySelected(const QDate &);
    void entryChanged(const QDate &);
    void imageListActivated();

private:
    GfxCanvasItem *imgDate(const QDate &d);

    QHash<QDate, ImageCollection *> tvi;
    GfxCanvasItem *ci;
    GfxTimeLine tl;
    TimeLineBar *tlb;
    QDate oldDate;
    GfxCanvasItem pos;
    DateEntry de;
};

class QPainter;
class TimeLineBar : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    struct Day
    {
    public:
        Day() : occurrences(0) {}
        Day(const Day &o) : date(o.date), occurrences(o.occurrences) {}
        Day(const QDate &d, int o) : date(d), occurrences(o) {}
        Day &operator=(const Day &o)
        { date = o.date; occurrences = o.occurrences; return *this; }

        QDate date;
        int occurrences;
    };

    TimeLineBar(const QList<Day> &, GfxCanvasItem *p);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);

    void setActivated(bool);
    QDate currentDate() const;
    void setDate(const QDate &);

signals:
    void dayChanged(const QDate &);
    void daySelected(const QDate &);

private:
    void setSelected(int);

    GfxCanvasItem *timeline();
    GfxCanvasItem *number(int num);
    GfxCanvasItem *highlight();
    GfxCanvasItem *itemForDay(int idx);
    void drawDay(const Day &ti, QPainter &p);
    GfxCanvasItem *itemForMonth(int idx);

    QList<Day> _days;
    QVector<GfxCanvasItem *> items;
    QVector<GfxCanvasItem *> months;
    QVector<GfxCanvasItem *> years;

    GfxCanvasItem *_timeline;
    GfxCanvasItem *_highlight;
    int selected;

    GfxTimeLine tl;

    bool _activated;
};

#endif
