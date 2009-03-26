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

#include "e3_navipane.h"
#include "windowmanagement.h"

#include <QApplication>
#include <QTabBar>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QHBoxLayout>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <QCopChannel>
#endif
#include <QPair>
#include <QDebug>

class E3ArrowWidget : public QWidget
{
    Q_OBJECT
public:
    E3ArrowWidget(QWidget *p = 0) : QWidget(p), show(false), prev(false) {}
    ~E3ArrowWidget() {}

    bool show;
    bool prev;

protected:
    void paintEvent(QPaintEvent *);
};

void E3ArrowWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.eraseRect(rect());

    if (show) {
        QStylePainter sp(this);
        QStyleOption opt;
        opt.initFrom(this);
        opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
        bool rtl = QApplication::isRightToLeft();
        if (prev)
            sp.drawPrimitive(rtl ? QStyle::PE_IndicatorArrowRight : QStyle::PE_IndicatorArrowLeft, opt);
        else
            sp.drawPrimitive(rtl ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight, opt);
    }
}

class E3LocWidget : public QWidget
{
    Q_OBJECT
public:
    E3LocWidget(QWidget *p = 0) : QWidget(p), loc(E3NaviPane::NA)
    {
        QFont fn(font());
        fn.setPointSize(6);
        setFont(fn);
    }
    ~E3LocWidget() {}

    QString text;
    QPixmap pixmap;
    E3NaviPane::Location loc;

protected:
    void paintEvent(QPaintEvent *);
};

void E3LocWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int h = rect().height();
    if (!text.isEmpty()) {
        p.drawText(rect(), Qt::AlignHCenter | Qt::AlignBottom, text);
        h = p.fontMetrics().height();
    } else if (!pixmap.isNull()) {
        p.drawPixmap(QPoint(rect().center().x()-pixmap.width()/2, rect().bottom()-pixmap.height()), pixmap);
        h = pixmap.height();
    }

    if (loc != E3NaviPane::NA) {
        QStylePainter sp(this);
        QStyleOption opt;
        opt.initFrom(this);
        opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
        bool rtl = QApplication::isRightToLeft();
        if (loc != E3NaviPane::Beginning) {
            opt.rect = QRect(rtl ? rect().right()-12 : rect().left(), rect().bottom()-h, 12, h);
            sp.drawPrimitive(rtl ? QStyle::PE_IndicatorArrowRight : QStyle::PE_IndicatorArrowLeft, opt);
        }
        if (loc != E3NaviPane::End) {
            opt.rect = QRect(rtl ? rect().left() : rect().right()-12, rect().bottom()-h, 12, h);
            sp.drawPrimitive(rtl ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight, opt);
        }
    }
}

// Subclass just so that the style can tell that these are our real tabs.
class E3TabBar : public QTabBar
{
    Q_OBJECT
public:
    E3TabBar(QWidget *parent=0) : QTabBar(parent) {}
};

class E3TabPane : public QWidget
{
    Q_OBJECT
public:
    E3TabPane(QWidget *parent=0);

    E3ArrowWidget *prev;
    E3ArrowWidget *next;
    E3TabBar *tabBar;
};

E3TabPane::E3TabPane(QWidget *parent)
    : QWidget(parent), prev(0), next(0), tabBar(0)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);
    hb->setSpacing(0);

    prev = new E3ArrowWidget();
    prev->setMinimumWidth(12);
    prev->setMaximumWidth(12);
    prev->prev = true;
    hb->addWidget(prev);
    next = new E3ArrowWidget();
    next->setMinimumWidth(12);
    next->setMaximumWidth(12);
    hb->addWidget(next);
}

class E3NaviPanePrivate
{
public:
    QMap<IDData, E3TabPane*> tabPanes;
    QMap<IDData, E3LocWidget*> locHints;
    IDData activeId;
};

E3NaviPane::E3NaviPane(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
{
    d = new E3NaviPanePrivate;
    d->activeId = IDData((WId)-1,-1);
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);

    WindowManagement *man = new WindowManagement(this);
    QObject::connect(man, SIGNAL(windowActive(QString,QRect,WId)),
                     this, SLOT(activeChanged(QString,QRect,WId)));

    QCopChannel *channel = new QCopChannel("QPE/NaviPane", this);
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(received(QString,QByteArray)));
}

void E3NaviPane::setTabs(IDData id, QList<QPair<QString,QIcon> > tabs)
{
    E3TabPane *tabPane(0);
    E3TabBar *tabBar(0);

    if (d->tabPanes.contains(id)) {
        tabPane = d->tabPanes[id];
        tabBar = tabPane->tabBar;
        while (tabBar->count())
            tabBar->removeTab(0);
    } else {
        tabPane = new E3TabPane();
        d->tabPanes.insert(id, tabPane);
        d->tabPanes[id]->tabBar = new E3TabBar();
        tabBar = d->tabPanes[id]->tabBar;
        tabBar->setShape(QTabBar::TriangularNorth);
        ((QHBoxLayout*)tabPane->layout())->insertWidget(1, tabBar);
        layout()->addWidget(tabPane);
        tabPane->hide();
    }

    typedef QPair<QString,QIcon> IconData;
    foreach (IconData iconData, tabs)
        tabBar->addTab(iconData.second, iconData.first);

    if (tabBar->count() > 0)
        tabPane->next->show = true;

    if (id.first == d->activeId.first) {
        if (d->tabPanes.contains(id)) {
            d->tabPanes[id]->show();
            // TabBar overrides location hints
            if (d->locHints.contains(d->activeId))
                d->locHints[d->activeId]->hide();
            d->activeId = id;
        }
    }
}

void E3NaviPane::setLocationHint(IDData id, QString text, QIcon icon, Location loc)
{
    E3LocWidget *w(0);

    if (d->locHints.contains(id)) {
        w = d->locHints[id];
    } else {
        w = new E3LocWidget();
        layout()->addWidget(w);
        w->hide();
        d->locHints.insert(id, w);
    }

    w->text = text;
    w->loc = loc;
    if (!icon.isNull())
        w->pixmap = icon.pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));

    if (id.first == d->activeId.first) {
        d->activeId = id;
        if (d->locHints.contains(d->activeId)) {
            d->locHints[d->activeId]->show();
            d->locHints[d->activeId]->update();
        }
    }
}

void E3NaviPane::activeChanged(QString,QRect,WId win)
{
    // Hide any active tabbar
    if (d->activeId != IDData((WId)-1,-1) && win != d->activeId.first) {
        if (d->tabPanes.contains(d->activeId))
            d->tabPanes[d->activeId]->hide();
        if (d->locHints.contains(d->activeId))
            d->locHints[d->activeId]->hide();
    }

    d->activeId = IDData(win,-1);

    // If we have a tabbar for the new active window, show it.
    QMapIterator<IDData, E3TabPane*> itab(d->tabPanes);
    while (itab.hasNext()) {
        itab.next();
        if (itab.key().first == win) {
            d->activeId = IDData(win,itab.key().second);
            d->tabPanes[d->activeId]->show();
            break;
        }
    }

    QMapIterator<IDData, E3LocWidget*> iloc(d->locHints);
    while (iloc.hasNext()) {
        iloc.next();
        if (iloc.key().first == win) {
            IDData temp = IDData(win,iloc.key().second);
            if (itab.hasNext()) {
                d->locHints[temp]->hide();  // TabBar overrides location hints
            } else {
                d->activeId = temp;
                d->locHints[d->activeId]->show();
            }
            break;
        }
    }
}

void E3NaviPane::received(const QString &msg, const QByteArray &data)
{
    if (msg == "setTabs(int,int,QList<QPair<QString,QIcon> >)") {
        QDataStream ds(data);
        int id;
        ds >> id;
        int tid;
        ds >> tid;
        QList<QPair<QString,QIcon> > tabData;
        ds >> tabData;
        setTabs(IDData(id,tid), tabData);
    } else if (msg == "hideTabs(int,int)") {
        QDataStream ds(data);
        int id;
        ds >> id;
        int tid;
        ds >> tid;
        IDData pair(id,tid);
        if (d->tabPanes.contains(pair)) {
            d->tabPanes[pair]->hide();
            layout()->removeWidget(d->tabPanes[pair]);
            delete d->tabPanes.take(pair);
        }
    } else if (msg == "setCurrentTab(int,int,int)") {
        QDataStream ds(data);
        int id;
        int tid;
        int idx;
        ds >> id;
        ds >> tid;
        ds >> idx;
        IDData pair(id,tid);
        if (pair == d->activeId && d->tabPanes.contains(d->activeId)) {
            d->tabPanes[d->activeId]->tabBar->setCurrentIndex(idx);
            d->tabPanes[d->activeId]->prev->show = (idx == 0 ? false : true);
            d->tabPanes[d->activeId]->next->show = (idx == d->tabPanes[d->activeId]->tabBar->count()-1 ? false : true);
            d->tabPanes[d->activeId]->prev->update();
            d->tabPanes[d->activeId]->next->update();
        }
    } else if (msg == "setLocationHint(int,int,QString,QIcon,int)") {
        QDataStream ds(data);
        int id;
        ds >> id;
        int lid;
        ds >> lid;
        QString text;
        ds >> text;
        QIcon icon;
        ds >> icon;
        int loc;
        ds >> loc;
        setLocationHint(IDData(id,lid), text, icon, (Location)loc);
    }
}

#include "e3_navipane.moc"
