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

#include "timedview.h"

#include <qappointment.h>
#include <qtimestring.h>
#include <qtopialog.h>
#include <qappointmentview.h>

#include <QList>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionViewItem>
#include <QStyleOption>

class LayoutItem
{
public:
    LayoutItem( const QOccurrence &e, const QModelIndex &i )
        : mOccurrence(e), mIndex(i), mOffset(0), mWidth(1) { }

    void setOffset(int i) { mOffset = i; }
    void setWidth(int i) { mWidth = i; }
    int offset() const { return mOffset; }
    int width() const { return mWidth; }

    QOccurrence occurrence() const { return mOccurrence; }
    QAppointment appointment() const { return mOccurrence.appointment(); }
    QModelIndex modelIndex() const { return mIndex; }

private:
    QOccurrence mOccurrence;
    QModelIndex mIndex;
    int mOffset;
    int mWidth;
};

class TimeManagerData
{
public:
    int mMinHeight;
    QMap<int, int> mMarks;
    int ds;
    int de;
};

TimeManager::TimeManager(QWidget *parent) : QWidget(parent)
{
    d = new TimeManagerData();
    d->ds = 60*8;
    d->de = 60*17;

    resetMarks(); // actually puts in default marks as well.
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
}

TimeManager::~TimeManager()
{
    delete d;
}

int TimeManager::minimumGapHeight() const
{
    QFontMetrics fm(font());
    return fm.height() + fm.descent() + 1;
}

int TimeManager::minimumHeight() const
{
    int mGap = minimumGapHeight();
    if (d->mMarks.count() < 2)
        return 0;
    return mGap * (d->mMarks.count() + 1);
}

QSize TimeManager::minimumSizeHint() const
{
    QFontMetrics fm(font());
    QSize result(fm.width(QTimeString::localHM(QTime(23, 0), QTimeString::Short)) + 6, minimumHeight());
    return result;
}

void TimeManager::resetMarks()
{
    clearMarks();
    populateMarks();
}

void TimeManager::clearMarks()
{
    d->mMarks.clear();

    //  Ensure that the beginning and end of the day will always be displayed
    d->mMarks.insert(d->ds, -1);
    d->mMarks.insert(d->de, -1);
}

void TimeManager::populateMarks()
{
    for (int i = 0; i <= 24; ++i)
        d->mMarks.insert(i*60, -1);
    cacheLayout();
}

void TimeManager::addMark(int minutes)
{
    if (d->mMarks.contains(minutes))
        return;
    d->mMarks.insert(minutes, -1);
}

int TimeManager::markPosition(int minutes) const
{
    return d->mMarks.value(minutes, -1);
}

// Return the minutes of the mark closest to
// this position.  The optional direction
// argument restricts it to marks before,
// after, or both.  We don't assume that the
// map is in any type of order, so we iterate through
// the whole lot
int TimeManager::markMinutes(int position, int direction) const
{
    QMutableMapIterator<int, int> it(d->mMarks);

    int minDistance = -1;
    int minutes = -1;
    int distance;

    while(it.hasNext()) {
        it.next();
        distance = it.value() - position;

        if ((direction < 0 && distance > 0)
            || (direction > 0 && distance < 0))
            continue;

        if (distance < 0)
            distance = -distance;
        if (minDistance < 0 || distance < minDistance) {
            minDistance = distance;
            minutes = it.key();
        }
    }
    return minutes;
}

QList<int> TimeManager::marks() const
{
    return d->mMarks.keys();
}

void TimeManager::changeEvent(QEvent *e)
{
    switch(e->type()) {
        case QEvent::FontChange:
            cacheLayout();
            break;
        default:
            break;
    }
}

void TimeManager::paintEvent(QPaintEvent *e)
{
    // repaint the items.  can be overriddent? should be done via delegate?
    // style?
    // shift up half a block for where drawn, (e.g. on line, not just above)
    // block height? the same.

    int ctop = e->rect().top();
    int cbottom = e->rect().bottom();

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().base());
    painter.drawRect(e->rect());

    int hmgap = minimumGapHeight() >> 1;
    int top = -1;
    int bottom = -1;
    int minutes = -1;

    QMutableMapIterator<int, int> it(d->mMarks);
    while(it.hasNext()) {
        it.next();

        top = it.value()-hmgap;
        bottom = it.value()+hmgap;
        minutes = it.key();

        // have a bottom and a top
        if (bottom >= ctop && top <= cbottom) {
            QRect itemR(0, top, width(), bottom-top);
            drawItem(&painter, itemR, minutes, bottom-hmgap);
        }
    }
}

void TimeManager::drawItem(QPainter *painter, const QRect &rect, int minutes, int )
{
    // TODO might need changing for rtl
    static QTextOption o( Qt::AlignRight );

    QRect r( rect.x() + 3, rect.y(), rect.width() - 6, rect.height() );

    QString s = QTimeString::localHM( QTime( minutes/60, minutes % 60 ), QTimeString::Short );
    painter->setPen( palette().color( QPalette::Text ) );
    painter->drawText( r, s, o );
}

void TimeManager::resizeEvent(QResizeEvent *)
{
    cacheLayout();
}

void TimeManager::cacheLayout()
{
    updateGeometry();

    // NOTE these lines dictate how far apart marks are placed.
    // can also include extra marks at this point.
    int mgap = minimumGapHeight();
    int theight = height() - 1;
    int c = d->mMarks.count();

    int e;
    int es;
    int er;
    if (c > 0) {
        e = theight - c*mgap;
        es = e/c;
        er = e%c;
    } else {
        e = 0;
        es = 0;
        er = 0;
    }

    int firstgap = (mgap + es) >> 1;
    // as only stretches downwards, just look for the -1 entry and
    // continue.
    int last = -1;
    QMutableMapIterator<int, int> it(d->mMarks);
    while(it.hasNext()) {
        it.next();
        int v = it.value();
        if (last == -1) {
            v = firstgap;
        } else {
            // might have to make previous wider.
            // gap = what?
            v = last + mgap + es;
        }
        if (er > 0) {
            --er;
            ++v;
        }
        last = v;
        it.setValue(v);
    }

    update();
}

void TimeManager::setDaySpan(int daystart, int dayend)
{
    // NOTE, there is an argument for allowing a span exceeding a day.
    daystart = qBound(0, daystart, 24*60);
    dayend = qBound(daystart, dayend, 24*60 - 1);

    d->ds = daystart;
    d->de = dayend;

    cacheLayout();
}

int TimeManager::dayStart() const
{
    return d->ds;
}

int TimeManager::dayEnd() const
{
    return d->de;
}

class CompressedTimeManagerData
{
public:
    int mIdealHeight;
    QList<int> addedMarks;  // Keep track of externally added marks, which are the uncompressible ones
};

CompressedTimeManager::CompressedTimeManager(QWidget *parent)
    : TimeManager(parent)
{
    d = new CompressedTimeManagerData;
    d->mIdealHeight = -1;
}

void CompressedTimeManager::setIdealHeight(int ideal)
{
    d->mIdealHeight = ideal;
    updateGeometry();
    populateMarks();
}

int CompressedTimeManager::idealHeight() const
{
    return d->mIdealHeight;
}

void CompressedTimeManager::addMark(int minutes)
{
    d->addedMarks.append(minutes);
    TimeManager::addMark(minutes);
}

void CompressedTimeManager::clearMarks()
{
    d->addedMarks.clear();
    TimeManager::clearMarks();
}

void CompressedTimeManager::populateMarks()
{
    TimeManager::clearMarks();
    foreach (int mark, d->addedMarks)
        TimeManager::addMark(mark);

    if (d->mIdealHeight >= 0) {
        QList<int> m = marks();
        int goalLineCount = ( d->mIdealHeight / minimumGapHeight() ) - 1;
        int minutes = dayEnd() - dayStart();
        int hours = minutes / 60;
        int currentMinute = dayStart();
        int currentCount = m.count();
        int increment;
        int closeEnough;

        //  Show only hours if there is not much room. If enough room even show 30 or 15 minute gaps
        if (goalLineCount >= (hours << 1)) {
            if (goalLineCount >= (hours << 2) ) {
                increment = 15;
                closeEnough = 5;
            } else {
                increment = 30;
                closeEnough = 10;
            }
        } else {
            increment = 60;
            closeEnough = 15;
        }

        //  Insert marks, skip ones that already have something near them
        QList<int>::ConstIterator it = m.constBegin();
        while (currentCount < goalLineCount && currentMinute < dayEnd()) {
            while (it != m.constEnd() && *it < currentMinute - closeEnough)
                ++it;

            if (it == m.constEnd() || *it > currentMinute + closeEnough) {
                TimeManager::addMark(currentMinute);
                currentCount++;
            }

            currentMinute += increment;
        }
        cacheLayout();
    }
}

class TimedViewData
{
public:
    TimedViewData()
        : model(0), timeManager(0),
        delegate(0), shownDate(QDate::currentDate())
    {}

    QOccurrenceModel *model;
    TimeManager *timeManager;
    QAbstractItemDelegate *delegate;
    QDate shownDate;
    QModelIndex currentIndex;

    QDateTime start() const
    { return QDateTime(shownDate, QTime(0,0)); }

    QDateTime end() const
    { return QDateTime(shownDate, QTime(23,59)); }

    void addMarksFor(const QOccurrence &o)
    {
        if (!timeManager)
            return;

        if (o.startInCurrentTZ() >= start())
            timeManager->addMark( dateTimeInMinutes( o.startInCurrentTZ() ) );
        else if (o.startInCurrentTZ() < start())
            timeManager->addMark( dateTimeInMinutes( start() ) );

        if (o.endInCurrentTZ() <= end())
            timeManager->addMark( dateTimeInMinutes( o.endInCurrentTZ() ) );
        else if (o.endInCurrentTZ() > end())
            timeManager->addMark( dateTimeInMinutes( end() ) );
    }

    inline int dateTimeInMinutes( const QDateTime &dt )
    {
        return ( dt.time().hour() * 60 + dt.time().minute() ) - ( dt.date().daysTo( shownDate ) * 24 * 60 );
    }

    void updateMarks();

    QList<LayoutItem> layoutItems;
};

void TimedViewData::updateMarks()
{
    if (!model || !timeManager || !shownDate.isValid())
        return;

    // ensure any changes to the model have updated the cache
    timeManager->clearMarks();

    // iterate through the model, and for each item that has an end or
    // start intersecting with the current date add to the time manager.

    int blockBegin = 0;
    int maxColumns = 0;
    layoutItems.clear();
    QMultiMap<QDateTime, int> columns;
    for( int i = 0; i < model->rowCount(); i++ ) {
        QOccurrence o = model->occurrence( i );
        if( o.appointment().isAllDay())
            continue;

        addMarksFor( o );
        LayoutItem l( o, model->index( i, 0 ) );

        //  Check for any columns that will have emptied by this point
        QMap<QDateTime, int>::iterator cit;
        for( cit = columns.begin(); cit != columns.end(); cit++ )
        {
            if( cit.key() != QDateTime() )
            {
                while( cit != columns.end() && cit.key() <= o.startInCurrentTZ() )
                {
                    columns.insert( QDateTime(), cit.value() );
                    cit = columns.erase( cit );
                }

                break;
            }
        }

        //  If all the columns are empty, reset the column calculations.
        if( columns.count() && ( columns.end() - 1 ).key() == QDateTime() ) {
            for( int j = blockBegin; j < i; ++j )
                layoutItems[j].setWidth( maxColumns );
            columns.clear();
            blockBegin = i;
            maxColumns = 0;
        }

        //  Is there an empty column to put this item into?
        if( columns.contains( QDateTime() ) ) {
            l.setOffset( columns.take( QDateTime() ) );
            columns.insert( o.endInCurrentTZ(), l.offset() );
        } else {
            l.setOffset( columns.count() );
            columns.insert( o.endInCurrentTZ(), columns.count() );
            if( columns.count() > maxColumns )
                maxColumns = columns.count();
        }

        layoutItems.append(l);
    }

    for( int j = blockBegin; j < layoutItems.count(); ++j )
        layoutItems[j].setWidth( maxColumns );

    timeManager->populateMarks();
}

TimedView::TimedView(QWidget *parent)
    : QWidget(parent)
{
    d = new TimedViewData();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

TimedView::~TimedView()
{
    delete d;
}

void TimedView::setDate(const QDate &date)
{
    Q_ASSERT(date.isValid());

    QDateTime start(date, QTime(0,0));
    QDateTime end(date.addDays(1), QTime(0,0));

    d->shownDate = date;
    d->model->setRange(start, end);
}

void TimedView::reset()
{
    d->currentIndex = QModelIndex();
    d->updateMarks();
    update();
    emit selectionChanged(d->currentIndex);
}

QDate TimedView::date() const
{
    return d->shownDate;
}

QDateTime TimedView::start() const
{
    return d->start();
}

QDateTime TimedView::end() const
{
    return d->end();
}

// Try to avoid changing current selection...
void TimedView::setDaySpan(int start, int end)
{
    d->timeManager->setDaySpan(start, end);
    d->updateMarks();
    update();
}

void TimedView::setModel(QOccurrenceModel *m)
{
    d->model = m;
    connect( d->model, SIGNAL(modelReset()), this, SLOT(reset()) );

    reset();
}

void TimedView::setTimeManager(TimeManager *m)
{
    d->timeManager = m;
    reset();
}


QOccurrenceModel *TimedView::model() const
{
    return d->model;
}

TimeManager *TimedView::timeManager() const
{
    return d->timeManager;
}

QModelIndex TimedView::index(const QPoint& point) const
{
    QPoint mappedPoint = mapFromGlobal(point);

    int itemCount = d->layoutItems.count();
    for (int i = 0; i < itemCount; i++) {
        LayoutItem item = d->layoutItems[i];

        int top = d->timeManager->markPosition(
            d->dateTimeInMinutes( item.occurrence().startInCurrentTZ() ) );

        int bottom = d->timeManager->markPosition(
            d->dateTimeInMinutes( item.occurrence().endInCurrentTZ() ) );

        if( top == -1 )
            top = 0;
        if( bottom == -1 )
            bottom = height();

        int w = width() / item.width();
        int left = item.offset() * w;
        QRect rect(left, top, w, bottom - top);

        if (rect.contains( mappedPoint ) ) {
            return item.modelIndex();
        }
    }

    return QModelIndex();
}

QDateTime TimedView::timeAtPoint(const QPoint& point, int direction) const
{
    QPoint mappedPoint = mapFromGlobal(point);
    if (rect().contains(mappedPoint)) {
        int minutes = d->timeManager->markMinutes(mappedPoint.y(), direction);
        if (minutes > 0) {
            return QDateTime(d->shownDate, QTime( minutes / 60, minutes % 60 ));
        }
    }
    return QDateTime();
}

void TimedView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    // base background
    painter.fillRect( e->rect(), palette().base() );

    // and for each mark...
    painter.setPen(Qt::gray);
    if (d->timeManager) {
        QList<int> marks = d->timeManager->marks();
        foreach (int mark, marks) {
            int position = d->timeManager->markPosition(mark);
            painter.drawLine(0, position, width(), position);
        }
    }

    // draw in each of the precalc'd appointments / occurrences
    QStyleOptionViewItem option;
    int itemCount = d->layoutItems.count();
    for(int i = 0; i < itemCount; i++) {
        LayoutItem item = d->layoutItems[i];
        int top = d->timeManager->markPosition(d->dateTimeInMinutes(item.occurrence().startInCurrentTZ()));
        int bottom = d->timeManager->markPosition(d->dateTimeInMinutes(item.occurrence().endInCurrentTZ()));

        if (top == -1)
            top = 0;
        if (bottom == -1)
            bottom = height() - 1;

        int w = width() / item.width();
        int left = item.offset() * w;

        bool selected = (d->currentIndex == item.modelIndex());
        if (selected)
            option.state |= QStyle::State_Selected;

        option.rect.setCoords(left, top, left + w - 1, bottom);

        d->delegate->paint(&painter, option, item.modelIndex());

        if (selected)
            option.state &= ~QStyle::State_Selected;
    }
}

QModelIndex TimedView::currentIndex() const
{
    return d->currentIndex;
}

void TimedView::setCurrentIndex(const QModelIndex &i)
{
    d->currentIndex = i;
    emit selectionChanged(i);
    update();
}

void TimedView::resizeEvent(QResizeEvent *)
{
}

void TimedView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    d->delegate = delegate;
}

QAbstractItemDelegate *TimedView::itemDelegate() const
{
    return d->delegate;
}

QRect TimedView::occurrenceRect(const QModelIndex &index) const
{
    int itemCount = d->layoutItems.count();
    for (int i = 0; i < itemCount; i++) {
        LayoutItem item = d->layoutItems[i];
        if (item.modelIndex() == index) {
            int top = d->timeManager->markPosition( d->dateTimeInMinutes( item.occurrence().startInCurrentTZ() ) );
            int bottom = d->timeManager->markPosition( d->dateTimeInMinutes( item.occurrence().endInCurrentTZ() ) );

            if( top == -1 )
                top = 0;
            if( bottom == -1 )
                bottom = height() - 1;

            int w = width() / item.width();
            int left = item.offset() * w;

            return QRect(left, top, w, bottom - top);
        }
    }

    return QRect();
}


