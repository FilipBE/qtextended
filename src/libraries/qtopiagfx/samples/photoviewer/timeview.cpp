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

#include "timeview.h"
#include <QPainter>
#include <GfxPainter>
#include "imagecollection.h"
#include <QDir>
#include "softkeybar.h"

int qHash(const QDate &d)
{
    return d.year() ^ d.month() + d.day();
}

TimeView::TimeView(GfxCanvasItem *p)
: GfxCanvasItem(p), ci(0), tlb(0), pos(this), de(this)
{
    SoftKeyBar::setLabel(this, SoftKeyBar::Middle, "Explore");
    SoftKeyBar::setLabel(&de, SoftKeyBar::Middle, "Done");

    de.visible().setValue(0.);
    de.y().setValue(270);
    QObject::connect(&de, SIGNAL(dateChanged(QDate)), this, SLOT(entryChanged(QDate)));
    QObject::connect(&de, SIGNAL(dateSelected(QDate)), this, SLOT(entrySelected(QDate)));

    QList<TimeLineBar::Day> timeInfo;

    QDir dir("collections/");
    QHash<QDate, TimeLineBar::Day> dates;
    QHash<QDate, QStringList> dfiles;
    foreach(QString collection,
            dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {

        QDir files("collections/" + collection, "*.png");
        foreach(QString file, files.entryList()) {
            QString ifile = "collections/" + collection + "/" + file;
            QFileInfo fi(ifile);
            QDate d = fi.lastModified().date();

            int year = 2007;
            int month = qrand() % 2 + 1;
            int day = qrand() % 5 + 1;
            d = QDate(year, month, day);

            dates[d].date = d;
            dates[d].occurrences++;
            dfiles[d].append(ifile);
        }

        files = QDir("collections/" + collection, "*.jpg");
        foreach(QString file, files.entryList()) {
            QString ifile = "collections/" + collection + "/" + file;
            QFileInfo fi(ifile);
            QDate d = fi.lastModified().date();

            int year = 2007;
            int month = qrand() % 2 + 1;
            int day = qrand() % 5 + 1;
            d = QDate(year, month, day);

            dates[d].date = d;
            dates[d].occurrences++;
            dfiles[d].append(ifile);
        }


    }

    for(QHash<QDate, TimeLineBar::Day>::Iterator iter = dates.begin();
            iter != dates.end(); ++iter) {
        timeInfo << iter.value();
        tvi[iter.key()] = 
            new ImageCollection(&pos, QString(), dfiles[iter.key()], false);
        tvi[iter.key()]->setState(ImageCollection::Hidden);
        tvi[iter.key()]->listItem()->height().setValue(208);
        tvi[iter.key()]->listItem()->y().setValue(20);
        QObject::connect(tvi[iter.key()]->listItem(), SIGNAL(activated(GfxCanvasListItem*)), this, SLOT(imageListActivated()));
        tvi[iter.key()]->viewingItem()->setParent(this);
        tvi[iter.key()]->viewingItem()->visible().setValue(1.);
        tvi[iter.key()]->viewingItem()->x().setValue(119);
        tvi[iter.key()]->viewingItem()->y().setValue(160);
        tvi[iter.key()]->viewingItem()->z().setValue(10000);
    }


    tlb = new TimeLineBar(timeInfo, &pos);
    tlb->x().setValue(120);
    tlb->y().setValue(270);
    tlb->z().setValue(2000);

    QObject::connect(tlb, SIGNAL(dayChanged(QDate)), this, SLOT(dayChanged(QDate)));
    QObject::connect(tlb, SIGNAL(daySelected(QDate)), this, SLOT(daySelected(QDate)));
    GfxCanvasReflection *r = new GfxCanvasReflection(QSize(240, 30), 0, -31, &pos);
    canvas()->addDynamicItem(r);
    r->y().setValue(229);
    r->z().setValue(1000);

    dayChanged(tlb->currentDate());
    setFocused(true);
}


GfxCanvasItem *TimeView::focusProxy()
{
    if(de.visible().value())
        return &de;
    else
        return tlb;
}

void TimeView::entryChanged(const QDate &d)
{
    tlb->setDate(d);
}

void TimeView::entrySelected(const QDate &d)
{
    tl.move(pos.y(), 0, 150);
    de.visible().setValue(0.);
    setFocused(true);
    de.visible().setValue(1.);
    tl.move(de.visible(), 0., 150);
}

void TimeView::showDateSelector()
{
    tvi[oldDate]->setState(ImageCollection::Time);
    tlb->setFocused(true);
    tlb->setActivated(true);

    de.visible().setValue(.001);
    tl.move(pos.y(), -40, 150);
    tl.move(de.visible(), 1., 150);
    de.setFocused(true);
}

void TimeView::keyReleaseEvent(QKeyEvent *e)
{
    if(tvi[oldDate]->state() == ImageCollection::View) {
        e->accept();
        imageListActivated();
    } else if(e->key() == Qt::Key_Back && oldDate.isValid()) {
        tvi[oldDate]->setState(ImageCollection::Time);
        tlb->setFocused(true);
        tlb->setActivated(true);
        e->accept();
    }
}

void TimeView::daySelected(const QDate &)
{
    tvi[oldDate]->setState(ImageCollection::Expanded);
    tvi[oldDate]->listItem()->setFocused(true);
    if(tvi[oldDate]->listItem()->current() == -1)
        tvi[oldDate]->listItem()->setCurrent(0);
    tlb->setActivated(false);
}

void TimeView::dayChanged(const QDate &d)
{
    if(ci) {
        if(d < oldDate) {
            tl.reset(ci->x());
            tl.move(ci->x(), 360, 300);
        } else {
            tl.reset(ci->x());
            tl.move(ci->x(), -120, 300);
        }
    }

    ci = imgDate(d);

    if(d < oldDate)
        ci->x().setValue(-120);
    else
        ci->x().setValue(360);
    ci->y().setValue(145);
    tl.move(ci->x(), 120, 300);
    oldDate = d;
}

GfxCanvasItem *TimeView::imgDate(const QDate &d)
{
    tvi[d]->setState(ImageCollection::Time);
    return tvi[d]->timeItem();
}

void TimeView::imageListActivated()
{
    if(tvi[oldDate]->state() == ImageCollection::Expanded) {
        tvi[oldDate]->setState(ImageCollection::View);
    } else if(tvi[oldDate]->state() == ImageCollection::View) {
        tvi[oldDate]->setState(ImageCollection::Expanded);
    }
}

#define HEIGHT 16
#define LOWER_HEIGHT 14
#define MIN_SPACING 20
#define MAX_SPACING 50

bool operator<(const TimeLineBar::Day &lhs, const TimeLineBar::Day &rhs)
{
    return lhs.date < rhs.date;
} 

TimeLineBar::TimeLineBar(const QList<Day> &days, GfxCanvasItem *p)
: GfxCanvasItem(p), _days(days), selected(0), _activated(true)
{
    qSort(_days.begin(), _days.end());
    _timeline = timeline();

    _highlight = highlight();
    _highlight->setParent(_timeline);


    _highlight->x().setValue(items[0]->mapTo(_timeline, QPoint(0, 0)).x());
    _timeline->x().setValue(-items[0]->mapTo(this, QPoint(0, 0)).x());

}

void TimeLineBar::setDate(const QDate &d)
{
    int closest = -1;
    int diff = -1;
    for(int ii = 0; ii < _days.count(); ++ii) {
        int cdiff = qAbs(_days.at(ii).date.daysTo(d)); 
        if(closest == -1 || cdiff < diff) {
            closest = ii;
            diff = cdiff;
        }
    }

    if(closest != -1)
        setSelected(closest);
}

void TimeLineBar::setSelected(int oselected)
{
    if(oselected != selected) {
        if(months[selected] != months[oselected]) {
            tl.reset(months[selected]->visible());
            tl.move(months[selected]->visible(), .6, 150);
            tl.reset(months[oselected]->visible());
            tl.move(months[oselected]->visible(), 1., 150);
        }
        if(years[selected] != years[oselected]) {
            tl.reset(years[selected]->visible());
            tl.move(years[selected]->visible(), .6, 150);
            tl.reset(years[oselected]->visible());
            tl.move(years[oselected]->visible(), 1., 150);
        }
        QPoint hp = 
            items[oselected]->mapTo(_timeline, QPoint(0, 0));

        tl.reset(_highlight->x());
        tl.move(_highlight->x(), hp.x(), 150);

        QPoint gp = _timeline->mapTo(this, hp);

        int diff = -gp.x();
        tl.reset(_timeline->x());
        tl.moveBy(_timeline->x(), diff, 400);

        selected = oselected;
        emit dayChanged(_days[selected].date);
    }
}

void TimeLineBar::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Select:
        e->accept();
        break;
    default:
        GfxCanvasItem::keyPressEvent(e);
        return;
    }

    if(e->key() == Qt::Key_Select) 
        emit daySelected(_days[selected].date);
}

void TimeLineBar::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Select:
        e->accept();
        break;
    default:
        GfxCanvasItem::keyPressEvent(e);
        return;
    }

    int oselected = selected;
    if(e->key() == Qt::Key_Left) 
        oselected--;
    else if(e->key() == Qt::Key_Right)
        oselected++;
    if(oselected < 0)
        oselected = 0;
    else if(oselected >= _days.count())
        oselected = _days.count() - 1;

    if(oselected == selected)
        return;

    setSelected(oselected);
}

GfxCanvasItem *TimeLineBar::timeline()
{
    items.resize(_days.count());
    months.resize(_days.count());
    years.resize(_days.count());

    int x = 0;
    QDate d;
    GfxCanvasItem *m = 0;
    GfxCanvasItem *y = 0;
    GfxCanvasItem *rv = new GfxCanvasItem(this);
    for(int ii = 0; ii < _days.count(); ++ii) {
        if(_days.at(ii).date.year() != d.year()) {
            GfxCanvasItem *i = number(_days.at(ii).date.year());
            i->setParent(rv);
            i->x().setValue(x);
            i->y().setValue(-6);
            x += i->boundingRect().width();
            y = i;
            if(ii != 0)
                i->visible().setValue(.6f);
        }
        if(_days.at(ii).date.month() != d.month() ||
                _days.at(ii).date.year() != d.year()) {
            GfxCanvasItem *item = itemForMonth(ii);
            if(ii != 0)
                item->visible().setValue(.6f);
            m = item;
            item->setParent(rv);
            d = _days.at(ii).date;
            item->x().setValue(x);
            x += item->boundingRect().width() + 4;
        }
        months[ii] = m;
        years[ii] = y;
    }

    return rv;
}

GfxCanvasItem *TimeLineBar::number(int num)
{
    QImage img(8, HEIGHT * 2 + 20 + 4, QImage::Format_RGB16);
    QString str = QString::number(num);
    for(int ii = str.count() - 2; ii >= 0; --ii)
        str.insert(ii + 1, "\n");
    img.fill(0);
    QFont f;
    f.setPointSize(10);
    QPainter p(&img);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(img.rect(), Qt::AlignHCenter | Qt::AlignVCenter, str);
    return new GfxCanvasImage(img, this);
}

GfxCanvasItem *TimeLineBar::highlight()
{
    GfxCanvasItem *rv = new GfxCanvasItem(this);

    GfxCanvasColor *cc = 0;
    cc = new GfxCanvasColor(Qt::black, QSize(5, 1), rv);
    cc->x().setValue(-MIN_SPACING / 2);
    cc = new GfxCanvasColor(Qt::black, QSize(5, 1), rv);
    cc->x().setValue(MIN_SPACING / 2);
    cc = new GfxCanvasColor(Qt::white, QSize(MIN_SPACING, 1), rv);
    cc->y().setValue(-HEIGHT - 2);
    cc = new GfxCanvasColor(Qt::white, QSize(MIN_SPACING + 1, 1), rv);
    cc->y().setValue(HEIGHT + 3);
    cc = new GfxCanvasColor(Qt::white, QSize(1, 2 * HEIGHT + 5), rv);
    cc->x().setValue(-MIN_SPACING / 2);
    cc = new GfxCanvasColor(Qt::white, QSize(1, 2 * HEIGHT + 5), rv);
    cc->x().setValue(MIN_SPACING / 2);

    return rv;
}

GfxCanvasItem *TimeLineBar::itemForDay(int idx)
{
    const Day &ti = _days.at(idx);
    GfxCanvasItem *rv = new GfxCanvasItem(this);

    QImage img(MIN_SPACING, HEIGHT * 2 + 4, QImage::Format_RGB16);
    img.fill(0);
    QPainter p(&img);

    QFont f;
    f.setPointSize(12);
    p.setFont(f);
    p.setPen(Qt::white);
    p.translate(img.width() / 2, HEIGHT);
    drawDay(ti, p);

    GfxCanvasImage *cimg = new GfxCanvasImage(img, rv);
    cimg->y().setValue(2);
    return rv;
}

void TimeLineBar::drawDay(const Day &ti, QPainter &p)
{
    QFontMetrics fm(p.font());
    QString day = QString::number(ti.date.day());
    QRect s = fm.boundingRect(day);
    s = s.translated(-s.width() / 2, -s.height() / 2 + 2);
    p.drawText(s, Qt::AlignHCenter | Qt::AlignVCenter, day);
    p.drawLine(0, -4, 0, 0);

    for(int ii = 0; ii < ti.occurrences && ii <= 6; ++ii) {
        int xx = (ii % 2) * 4 - 2;
        int yy = (ii / 2) * 3 + 3;

        if(ii == 6)
            p.fillRect(QRect(xx, yy, 6, 2), QColor(200, 200, 200));
        else
            p.fillRect(QRect(xx, yy, 2, 2), QColor(200, 200, 200));
    }
}

QDate TimeLineBar::currentDate() const
{
    return _days.at(selected).date;
}

void TimeLineBar::setActivated(bool activated)
{
    if(_activated == activated)
        return;

    _activated = activated;
    qreal v = _activated?1.0:0.6f;

    tl.reset(months[selected]->visible());
    tl.reset(_highlight->visible());

    tl.move(months[selected]->visible(), v, 150);
    tl.move(_highlight->visible(), v, 150);
}

GfxCanvasItem *TimeLineBar::itemForMonth(int idx)
{
    GfxCanvasItem *rv = new GfxCanvasItem(this);

    QDate d = _days.at(idx).date;

    QFont f;
    f.setPointSize(16);
    QImage img = GfxPainter::string(QDate::longMonthName(d.month()), Qt::white, f);
    GfxCanvasImage *cimg = new GfxCanvasImage(img, rv);
    cimg->x().setValue(img.width() / 2);
    cimg->y().setValue(-HEIGHT - img.height() / 2);

    GfxCanvasColor *cc = new GfxCanvasColor(Qt::white, 
                                            QSize(1, HEIGHT), rv);
    cc->y().setValue(-HEIGHT / 2);

    int dayOffset = MIN_SPACING;
    bool first = true;
    do {
        if(!first) {
            int ddiff = _days.at(idx).date.day() - _days.at(idx - 1).date.day();
            ddiff *= MIN_SPACING;
            if(ddiff > MAX_SPACING)
                ddiff = MAX_SPACING;
            dayOffset += ddiff;
        }
        first = false;

        GfxCanvasItem *item = itemForDay(idx);
        item->setParent(rv);
        item->x().setValue(dayOffset);
        items[idx] = item;

    } while(++idx < _days.count() && 
            _days.at(idx).date.month() == d.month());

    dayOffset += MIN_SPACING;
    dayOffset = qMax(dayOffset, img.width());

    cc = new GfxCanvasColor(Qt::white, QSize(dayOffset, 1), rv);
    cc->x().setValue(dayOffset / 2);
    cc = new GfxCanvasColor(Qt::white, QSize(1, LOWER_HEIGHT), rv);
    cc->y().setValue(LOWER_HEIGHT / 2);
    cc->x().setValue(dayOffset);

    return rv;
}

DateEntry::DateEntry(GfxCanvasItem *parent)
: GfxCanvasItem(parent), focused(0)
{
    {
        QImage d = GfxPainter::string("Day", Qt::white);
        GfxCanvasImage *i = new GfxCanvasImage(d, this);
        i->rotate().setValue(-90.);
        i->x().setValue(26);
    }
    {
        QImage d = GfxPainter::string("Month", Qt::white);
        GfxCanvasImage *i = new GfxCanvasImage(d, this);
        i->rotate().setValue(-90.);
        i->x().setValue(78);
    }
    {
        QImage d = GfxPainter::string("Year", Qt::white);
        GfxCanvasImage *i = new GfxCanvasImage(d, this);
        i->rotate().setValue(-90.);
        i->x().setValue(130);
    }

    GfxCanvasRoundedRect *rr = new GfxCanvasRoundedRect(this);
    rr->setColor(Qt::white);
    rr->setCornerCurve(5);
    rr->setLineWidth(1);
    rr->width().setValue(40);
    rr->height().setValue(30);
    rr->x().setValue(50);
    rr->y().setValue(0);
    rr = new GfxCanvasRoundedRect(this);
    rr->setColor(Qt::blue);
    rr->setCornerCurve(5);
    rr->setLineWidth(3);
    rr->width().setValue(42);
    rr->height().setValue(32);
    rr->x().setValue(50);
    rr->y().setValue(0);
    focus[0] = rr;

    rr = new GfxCanvasRoundedRect(this);
    rr->setColor(Qt::white);
    rr->setCornerCurve(5);
    rr->setLineWidth(1);
    rr->width().setValue(40);
    rr->height().setValue(30);
    rr->x().setValue(102);
    rr->y().setValue(0);
    rr = new GfxCanvasRoundedRect(this);
    rr->setColor(Qt::blue);
    rr->setCornerCurve(5);
    rr->setLineWidth(3);
    rr->width().setValue(42);
    rr->height().setValue(32);
    rr->x().setValue(102);
    rr->y().setValue(0);
    focus[1] = rr;

    rr = new GfxCanvasRoundedRect(this);
    rr->setColor(Qt::white);
    rr->setCornerCurve(5);
    rr->setLineWidth(1);
    rr->width().setValue(80);
    rr->height().setValue(30);
    rr->x().setValue(174);
    rr->y().setValue(0);
    rr = new GfxCanvasRoundedRect(this);
    rr->setColor(Qt::blue);
    rr->setCornerCurve(5);
    rr->setLineWidth(3);
    rr->width().setValue(82);
    rr->height().setValue(32);
    rr->x().setValue(174);
    rr->y().setValue(0);
    focus[2] = rr;

    focus[0]->visible().setValue(0.6);
    focus[1]->visible().setValue(0.);
    focus[2]->visible().setValue(0.);

    QFont f;
    f.setPointSize(14);

    txt[0] = new GfxCanvasText(QSize(int(focus[0]->width().value()), int(focus[0]->height().value())), this);
    txt[0]->setFont(f);
    txt[0]->setText("01");
    txt[0]->x().setValue(focus[0]->x().value());
    txt[0]->y().setValue(focus[0]->y().value());

    txt[1] = new GfxCanvasText(QSize(int(focus[1]->width().value()), int(focus[1]->height().value())), this);
    txt[1]->setFont(f);
    txt[1]->setText("01");
    txt[1]->x().setValue(focus[1]->x().value());
    txt[1]->y().setValue(focus[1]->y().value());

    txt[2] = new GfxCanvasText(QSize(int(focus[2]->width().value()), int(focus[2]->height().value())), this);
    txt[2]->setFont(f);
    txt[2]->setText("2008");
    txt[2]->x().setValue(focus[2]->x().value());
    txt[2]->y().setValue(focus[2]->y().value());

    date = QDate(2008, 1, 1);
}

void DateEntry::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Select:
            break;
        default:
            return;
    }
    e->accept();

    if(e->key() == Qt::Key_Select) 
        emit dateSelected(date);
}

void DateEntry::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Select:
            break;
        default:
            return;
    }
    e->accept();

    if(e->key() == Qt::Key_Select) 
        return;

    if(e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        QDate d = date;

        int inv = (e->key() == Qt::Key_Up)?1:-1;
        if(focused == 0)
            d = d.addDays(inv);
        else if(focused == 1)
            d = d.addMonths(inv);
        else if(focused == 2)
            d = d.addYears(inv);

        if(d != date) {
            if(d.isValid())
                setDate(d);
            emit dateChanged(d);
        }

        return;
    }

    int of = focused;
    if(e->key() == Qt::Key_Left) {
        of--;
    } else if(e->key() == Qt::Key_Right) {
        of++;
    }

    if(of < 0 || of > 2 || of == focused)
        return;

    tl.complete();
    tl.move(focus[focused]->visible(), 0., 150);
    tl.pause(focus[of]->visible(), 150);
    focused = of;
    tl.move(focus[focused]->visible(), .6, 150);
}

void DateEntry::setDate(const QDate &d)
{
    QString day = QString::number(d.day());
    QString month = QString::number(d.month());
    QString year = QString::number(d.year());

    while(day.length() < 2) day.prepend("0");
    while(month.length() < 2) month.prepend("0");
    while(year.length() < 4) year.prepend("0");

    if(day != txt[0]->text()) setItem(0, day);
    if(month != txt[1]->text()) setItem(1, month);
    if(year != txt[2]->text()) setItem(2, year);

    date = d;
}

void DateEntry::setItem(int idx, const QString &val)
{
    int oldval = txt[idx]->text().toInt();
    int newval = val.toInt();

    int inv = (oldval > newval)?1:-1;

    GfxCanvasText *old = txt[idx];
    GfxCanvasText *newt = new GfxCanvasText(old->size(), this);
    newt->setFont(old->font());
    newt->x().setValue(old->x().value());
    newt->y().setValue(focus[idx]->y().value() + inv * 30);
    newt->setText(val);
    newt->visible().setValue(0.);

    tl.moveBy(old->y(), -inv * 30, 150);
    tl.moveBy(newt->y(), -inv * 30, 150);
    tl.move(old->visible(), 0, 150);
    tl.move(newt->visible(), 1, 150);
    tl.pause(*old, 150);
    tl.execute(old->destroyEvent());

    txt[idx] = newt;
}

