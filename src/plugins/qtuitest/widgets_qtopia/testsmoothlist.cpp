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

#include "testsmoothlist.h"
#include "testabstractitemview.h"
#include "testwidgetslog.h"

#include <QSmoothList>
#include <QListView>
#include <QTimer>
#include <QScrollBar>

#include <QtUiTest>
#include <Qtopia>
#include <QContentSetModel>

/* FIXME remove this when ThemeItem special case is removed */
#include <ThemedView>
#include <QThemedView>
#include <QThemeTextItem>
#include <QThemeTemplateInstanceItem>
#include <QThemeListModel>
#include "testthemedview.h"

/*****************************************************************************/

TestSmoothList::TestSmoothList(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QSmoothList*>(_q))
{
    connect(q, SIGNAL(activated(QModelIndex)),
            this, SLOT(on_activated(QModelIndex)));
}

void TestSmoothList::on_activated(QModelIndex const& ind)
{ emit selected(Qtopia::dehyphenate(ind.data().toString())); }

QString TestSmoothList::selectedText() const
{
    TestWidgetsLog();
    TestAbstractItemView::waitForModelUpdate(q->model());
    return Qtopia::dehyphenate(q->currentIndex().data().toString());
}

QString TestSmoothList::text() const
{
    TestWidgetsLog();
    return list().join("\n");
}

QStringList TestSmoothList::list() const
{
    TestWidgetsLog() << q;
    TestAbstractItemView::waitForModelUpdate(q->model());
    QStringList ret;

    class QModelListGetter : public QModelViewIterator<QSmoothList>
    {
    public:
        QModelListGetter(QSmoothList *view)
         : QModelViewIterator<QSmoothList>(view) {};

        QStringList getList() {
            list.clear();
            iterate();
            return list;
        }
    protected:
        virtual void visit(QModelIndex const &index)
        { list << Qtopia::dehyphenate(index.data().toString()); }

        QStringList list;
    };

    /* FIXME get rid of this special case */
    class ThemeListModelListGetter
    {
    public:
        ThemeListModelListGetter(QSmoothList *view)
         : model(qobject_cast<ThemeListModel*>(view->model())) {};

        QStringList getList() {
            QStringList list;
            for (int row = model->rowCount()-1; row >= 0; --row) {
                ThemeListModelEntry* e = model->themeListModelEntry(model->index(row));
                if (!e) continue;
                QList<ThemeItem*> items = allChildren(e->templateInstance(), ThemedView::Text);
                qSort(items.begin(), items.end(), TestThemedView::old_themeItemLessThan);
                foreach (ThemeItem *i, items) {
                    ThemeTextItem *ti = static_cast<ThemeTextItem*>(i);
                    list << Qtopia::dehyphenate(ti->text());
                }
            }
            return list;
        }
    protected:
        QList<ThemeItem*> allChildren(ThemeItem *item, int type = -1) {
            QList<ThemeItem*> ret;
            foreach (ThemeItem *item, item->children()) {
                if (-1 != type && item->rtti() != type) continue;
                ret << item;
                ret << allChildren(item, type);
            }
            return ret;
        }
        ThemeListModel *model;
    };
    class QThemeListModelListGetter
    {
    public:
        QThemeListModelListGetter(QSmoothList *view)
         : model(qobject_cast<QThemeListModel*>(view->model())) {};

        QStringList getList() {
            QStringList list;
            for (int row = model->rowCount()-1; row >= 0; --row) {
                QThemeListModelEntry* e = model->themeListModelEntry(model->index(row));
                if (!e) continue;
                QList<QThemeTextItem*> items = allChildren(e->templateInstance());
                qSort(items.begin(), items.end(), TestThemedView::themeItemLessThan);
                foreach (QThemeTextItem *i, items) {
                    list << Qtopia::dehyphenate(i->text());
                }
            }
            return list;
        }
    protected:
        QList<QThemeTextItem*> allChildren(QThemeItem *item) {
            QList<QThemeTextItem*> ret;
            foreach (QGraphicsItem *i, item->children()) {
                QThemeTextItem *ti = qgraphicsitem_cast<QThemeTextItem*>(i);
                if (!ti) continue;
                ret << ti;
                ret << allChildren(ti);
            }
            return ret;
        }
        QThemeListModel *model;
    };


    QStringList list;

    if (q->model()->inherits("ThemeListModel")) {
        list = ThemeListModelListGetter(q).getList();
    } else if (q->model()->inherits("QThemeListModel")) {
        list = QThemeListModelListGetter(q).getList();
    } else
        list = QModelListGetter(q).getList();

    return list;
}

QRect TestSmoothList::visualRect(QString const &item) const
{
    TestWidgetsLog();
    TestAbstractItemView::waitForModelUpdate(q->model());

    QRect ret;

    class QModelRectGetter : public QModelViewIterator<QSmoothList>
    {
    public:
        QModelRectGetter(QSmoothList *view, QString const &item)
         : QModelViewIterator<QSmoothList>(view), matches(0), m_item(item) {};
        QRect rect;
        int matches;

    protected:
        void visit(QModelIndex const &index) {
            if (Qtopia::dehyphenate(index.data().toString()) == m_item) {
                ++matches;
                rect = view()->visualRect(index);
            }
        }
    private:
        QString m_item;
    };

    QModelRectGetter rectGetter(q, item);
    rectGetter.iterate();

    // No matching item
    if (!rectGetter.matches) {
        TestWidgetsLog() << "no matching item for" << item;
    }

    // More than one matching item
    else if (rectGetter.matches > 1) {
        qWarning("QtUitest: more than one item matches '%s' in item view", qPrintable(item));
        TestWidgetsLog() << rectGetter.matches << "matches for" << item;
    }

    else
        ret = rectGetter.rect;

    return ret;
}

bool TestSmoothList::isMultiSelection() const
{ return (q->selectionMode() > QSmoothList::SingleSelection); }

bool TestSmoothList::canSelect(QString const &item) const
{
    if (q->selectionMode() == QSmoothList::NoSelection)
        return false;

    return list().contains(item);
}

bool TestSmoothList::canSelectMulti(QStringList const &items) const
{
    if (!isMultiSelection())
        return false;

    QSet<QString> itemSet = items.toSet();
    return ((itemSet & list().toSet()) == itemSet);
}

bool TestSmoothList::select(QString const &item)
{
    TestWidgetsLog() << item;

    if (!canSelect(item)) {
        TestWidgetsLog() << "can't select" << item;
        return false;
    }

    if (!setFocus()) {
        QtUiTest::setErrorString(
            "While attempting to select '" + item + "', could not give focus to self.");
        return false;
    }

    if (Qtopia::mousePreferred()) {

        if (!ensureVisible(item)) {
            TestWidgetsLog() << "couldn't make" << item << "visible";
            return false;
        }
        QPoint pos = visualRect(item).center();
        TestWidgetsLog() << "after ensureVisible, item is at" << pos;

        QtUiTest::mouseClick(mapToGlobal(pos), Qt::LeftButton);
        return true;

    } else {

        if (!hasFocus()) {
            QtUiTest::setErrorString(
                "While attempting to select '" + item + "', could not give focus to self.");
            return false;
        }

        // Consume any pending key events
        QtUiTest::waitForEvent(q, QEvent::KeyRelease, 200, Qt::QueuedConnection);

        QStringList allItems = list();

        const int maxtries = 100;
        int desiredIndex = allItems.indexOf(item);
        int currentIndex = allItems.indexOf(selectedText());

        TestWidgetsLog() << "desiredIndex=" << desiredIndex << ", currentIndex=" << currentIndex;

        // Move vertically
        Qt::Key key;
        if (desiredIndex > currentIndex)
            key = Qt::Key_Down;
        else
            key = Qt::Key_Up;

        for (int i = 0; i < maxtries && selectedText() != item; ++i) {
            TestWidgetsLog() << "keyClick" << (key==Qt::Key_Down ? "Down" : "Up");
            if (!QtUiTest::keyClick(q, key)) return false;
        }
        TestWidgetsLog() << "selectedText() now" << selectedText();
        if (selectedText() != item) {
            QtUiTest::setErrorString(QString(
                "Pressed up/down keys %1 times, but selected item is '%2' "
                "rather than '%3'.")
                    .arg(maxtries)
                    .arg(selectedText())
                    .arg(item));
            return false;
        }
        TestWidgetsLog() << "hit select key";
        QtUiTest::keyClick(Qt::Key_Select, 0);
        return true;
    }

    return false;
}

bool TestSmoothList::selectMulti(QStringList const &items)
{
    if (!canSelectMulti(items)) return false;

    TestWidgetsLog() << items;

    return false;
}

bool TestSmoothList::ensureVisible(QString const &item)
{
    QPoint p = visualRect(item).center();
    if (rect().contains(p)) {
        TestWidgetsLog() << item << "is already visible";
        return true;
    }

    if (!Qtopia::mousePreferred())
        return false;

    /* Figure out the points to click for scrolling in each direction */
    QPoint up    = q->mapToGlobal(QPoint(q->width()/2,5));
    QPoint down  = q->mapToGlobal(QPoint(q->width()/2,q->height()-5));

    /* While p is above rect, flick down */
    while (p.y() < rect().top()) {
        QPoint drag = up;
        QtUiTest::mousePress(drag);
        // Drag upwards at rate of 2 pixels per 10 ms.
        while (drag.y() < down.y() && p.y() < rect().top()) {
            QtUiTest::wait(10);
            drag.setY( drag.y() + 2 );
            QtUiTest::mousePress(drag);
            p = visualRect(item).center();
        }
        QtUiTest::mouseRelease(drag);
        TestWidgetsLog() << item << "down"
                << "\nrect:" << rect() << "p:" << p;
        // Wait for any animations to complete.
        QtUiTest::wait(500);
        p = visualRect(item).center();
    }
    /* While p is below rect, flick up */
    while (p.y() > rect().bottom()) {
        QPoint drag = down;
        QtUiTest::mousePress(drag);
        // Drag downwards at rate of 2 pixels per 10 ms.
        while (drag.y() > up.y() && p.y() > rect().bottom()) {
            QtUiTest::wait(10);
            drag.setY( drag.y() - 2 );
            QtUiTest::mousePress(drag);
            p = visualRect(item).center();
        }
        QtUiTest::mouseRelease(drag);
        TestWidgetsLog() << item << "up"
                << "\nrect:" << rect() << "p:" << p;
        // Wait for any animations to complete.
        QtUiTest::wait(500);
        p = visualRect(item).center();
    }

    if (!rect().contains(p)) {
        TestWidgetsLog() << item << "failed"
                << "\nrect:" << rect() << "p:" << p;
        return false;
    }

    return true;
}

bool TestSmoothList::canWrap(QObject* o)
{ return qobject_cast<QSmoothList*>(o); }

