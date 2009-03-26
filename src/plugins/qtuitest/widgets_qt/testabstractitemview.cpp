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

#include "testabstractitemview.h"
#include "testwidgetslog.h"

#include <QListView>
#include <QTimer>
#include <QScrollBar>

#include <qtuitestnamespace.h>

#ifdef QTOPIA_TARGET
# include <Qtopia>
# include <QContentSetModel>
#endif

/*****************************************************************************/

/*! If \a model is an asynchronously updating model in the middle of an update,
 *  wait for the update to complete.
 */
void TestAbstractItemView::waitForModelUpdate(QAbstractItemModel* model)
{
#ifdef QTOPIA_TARGET
    if (QContentSetModel *cmodel = qobject_cast<QContentSetModel*>(model)) {
        /* If an update is _likely_ to occur soon, wait for a little while */
        if (0 == cmodel->rowCount() && !cmodel->updateInProgress()) {
            QtUiTest::waitForSignal(cmodel, SIGNAL(updateStarted()), 200);
        }
        if (cmodel->updateInProgress()) {
            QtUiTest::waitForSignal(cmodel, SIGNAL(updateFinished()));
        }
        QtUiTest::setErrorString(QString());
    }
#else
    Q_UNUSED(model);
#endif
}

TestAbstractItemView::TestAbstractItemView(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QAbstractItemView*>(_q))
{
    QtUiTest::connectFirst(q,    SIGNAL(activated(QModelIndex)),
                        this, SLOT(on_activated(QModelIndex)));
    // Unfortunately, some people connect to "pressed" rather than "activated",
    // so we have to as well, because not all selections go via "activated".
    QtUiTest::connectFirst(q,    SIGNAL(pressed(QModelIndex)),
                        this, SLOT(on_activated(QModelIndex)));
}

void TestAbstractItemView::on_activated(QModelIndex const& ind)
{
    // Timer discourages duplicate signal emission.
    if (m_lastActivatedTimer.elapsed() > 500 || m_lastActivatedTimer.elapsed() < 0) {
        TestWidgetsLog() << "emit selected" << ind.data().toString();
        emit selected(printable(ind.data().toString()));
        m_lastActivatedTimer.start();
    } else {
        TestWidgetsLog() << "Would emit selected" << ind.data().toString()
                << "except we have already done that recently.";
    }
}

QString TestAbstractItemView::selectedText() const
{
    TestWidgetsLog();
    waitForModelUpdate(q->model());
    return printable(q->currentIndex().data().toString());
}

QString TestAbstractItemView::text() const
{
    TestWidgetsLog();
    return list().join("\n");
}

QStringList TestAbstractItemView::list() const
{
    using namespace QtUiTest;

    TestWidgetsLog() << q;
    waitForModelUpdate(q->model());
    QStringList ret;

    /* FIXME get rid of this special case */
    {
        static bool pass_through = false;
        ListWidget *parent;
        if (!pass_through
            && q->inherits("QCalendarView")
            && (parent = qtuitest_cast<ListWidget*>(q->parent())))
        {
            pass_through = true;
            ret = parent->list();
            pass_through = false;
            return ret;
        }
    }

    // Allow testwidgets to make decisions based on the view associated with this item.
    QVariant view = QVariant::fromValue((QObject*)q);
    q->model()->setProperty("_q_qtuitest_itemview", view);
    TestWidgetsLog() << "_q_qtuitest_itemview is" << q->model()->property("_q_qtuitest_itemview").value<QObject*>();
    ListWidget* lw = qtuitest_cast<ListWidget*>(q->model());
    if (!lw) {
        QString model;
        QDebug(&model) << q->model();
        setErrorString("Could not find a ListWidget interface for model " + model);
        return QStringList();
    }
    return lw->list();
}

QRect TestAbstractItemView::visualRect(QString const &item) const
{
    using namespace QtUiTest;

    TestWidgetsLog();
    waitForModelUpdate(q->model());

    QRect ret;

    /* FIXME get rid of this special case */
    {
        static bool pass_through = false;
        ListWidget *parent;
        if (!pass_through
                && q->inherits("QCalendarView")
                && (parent = qtuitest_cast<ListWidget*>(q->parent())))
        {
            pass_through = true;
            ret = parent->visualRect(item);
            pass_through = false;
            ret.moveTopLeft( q->mapFromGlobal( q->parentWidget()->mapToGlobal( ret.topLeft() ) ) );
            return ret;
        }
    }

    // Allow testwidgets to make decisions based on the view associated with this item.
    q->model()->setProperty("_q_qtuitest_itemview", q);
    ListWidget* lw = qtuitest_cast<ListWidget*>(q->model());
    if (!lw) {
        QString model;
        QDebug(&model) << q->model();
        setErrorString("Could not find a ListWidget interface for model " + model);
    } else {
        ret = lw->visualRect(item);
    }

    return ret;
}

bool TestAbstractItemView::isMultiSelection() const
{ return (q->selectionMode() > QAbstractItemView::SingleSelection); }

bool TestAbstractItemView::canSelect(QString const &item) const
{
    if (q->selectionMode() == QAbstractItemView::NoSelection)
        return false;

    return list().contains(item);
}

bool TestAbstractItemView::canSelectMulti(QStringList const &items) const
{
    if (!isMultiSelection())
        return false;

    QSet<QString> itemSet = items.toSet();
    return ((itemSet & list().toSet()) == itemSet);
}

bool TestAbstractItemView::select(QString const &item)
{
    /* FIXME fix calendar widget and remove this code */
    if (q->inherits("QCalendarView")) return false;
    TestWidgetsLog() << item;

    if (!canSelect(item)) {
        TestWidgetsLog() << "can't select" << item;
        return false;
    }

    if (!setFocus() || !hasFocus()) {
        QtUiTest::setErrorString("Couldn't give focus to item view");
        return false;
    }

    if (QtUiTest::mousePreferred()) {

        if (!ensureVisible(item)) {
            TestWidgetsLog() << "couldn't make" << item << "visible";
            return false;
        }
        QPoint pos = visualRect(item).center();
        TestWidgetsLog() << "after ensureVisible, item is at" << pos;

        QtUiTest::mouseClick(mapToGlobal(pos), Qt::LeftButton);
        return true;

    } else {
        // Consume pending key events, if any.
        while (QtUiTest::waitForEvent(q, QEvent::KeyRelease, 200, Qt::QueuedConnection))
        {}

        QStringList allItems = list();

        const int maxtries = 100;
        int desiredIndex = allItems.indexOf(item);
        int currentIndex = allItems.indexOf(selectedText());

        TestWidgetsLog() << "desiredIndex=" << desiredIndex << ", currentIndex=" << currentIndex;

        // Move horizontally (if necessary)
        int desiredPos = visualRect(item).center().x();
        int currentPos = visualRect(selectedText()).center().x();

        for (int i = 0; i < maxtries && desiredPos != currentPos; ++i) {
            Qt::Key key;
            if (desiredPos < currentPos) {
                key = Qt::Key_Left;
                TestWidgetsLog() << "Left (desired=" << desiredPos << ", current=" << currentPos << ")";
            } else {
                key = Qt::Key_Right;
                TestWidgetsLog() << "Right (desired=" << desiredPos << ", current=" << currentPos << ")";
            }
            if (!QtUiTest::keyClick(q, key)) return false;
            currentPos = visualRect(selectedText()).center().x();
        }
        if (desiredPos != currentPos) {
            QtUiTest::setErrorString(QString(
                "Left/right keys failed to move highlight horizontally; desired position %1, "
                "current position %2").arg(desiredPos).arg(currentPos));
            return false;
        }

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
        QString selected = selectedText();
        TestWidgetsLog() << "selectedText() now" << selected;
        if (selected != item) {
            QtUiTest::setErrorString(QString(
                "Up/down keys should have caused item %1 to become selected, but item %2 "
                "is selected instead.").arg(item).arg(selected));
            return false;
        }
        TestWidgetsLog() << "hit activate key";
        if (!QtUiTest::keyClick(q, QtUiTest::Key_Activate)) return false;
        return true;
    }

    return false;
}

bool TestAbstractItemView::selectMulti(QStringList const &items)
{
    if (!canSelectMulti(items)) return false;

    TestWidgetsLog() << items;

    return false;
}

bool TestAbstractItemView::ensureVisible(QString const &item)
{
    QPoint p = visualRect(item).center();
    if (rect().contains(p)) {
        TestWidgetsLog() << item << "is already visible";
        return true;
    }

    if (!QtUiTest::mousePreferred())
        return false;

    /* Figure out the points to click for scrolling in each direction */
    QScrollBar *vbar = q->verticalScrollBar();
    QScrollBar *hbar = q->horizontalScrollBar();
    QPoint up    = vbar->mapToGlobal(QPoint(vbar->width()/2,5));
    QPoint down  = vbar->mapToGlobal(QPoint(vbar->width()/2,vbar->height()-5));
    QPoint left  = hbar->mapToGlobal(QPoint(5,              hbar->height()/2));
    QPoint right = hbar->mapToGlobal(QPoint(hbar->width()-5,hbar->height()/2));



    /* While p is above rect... */
    while (p.y() < rect().top()) {
        if (!vbar->isVisible()) return false;
        TestWidgetsLog() << item << "up"
                << "\nrect:" << rect() << "p:" << p;
        QtUiTest::mouseClick(up);
        p = visualRect(item).center();
    }
    /* While p is below rect... */
    while (p.y() > rect().bottom()) {
        if (!vbar->isVisible()) return false;
        TestWidgetsLog() << item << "down"
                << "\nrect:" << rect() << "p:" << p;
        QtUiTest::mouseClick(down);
        p = visualRect(item).center();
    }
    /* While p is left of rect... */
    while (p.x() < rect().left()) {
        if (!hbar->isVisible()) return false;
        TestWidgetsLog() << item << "left"
                << "\nrect:" << rect() << "p:" << p;
        QtUiTest::mouseClick(left);
        p = visualRect(item).center();
    }
    /* While p is right of rect... */
    while (p.x() > rect().right()) {
        if (!hbar->isVisible()) return false;
        TestWidgetsLog() << item << "right"
                << "\nrect:" << rect() << "p:" << p;
        QtUiTest::mouseClick(right);
        p = visualRect(item).center();
    }

    if (!rect().contains(p)) {
        TestWidgetsLog() << item << "failed"
                << "\nrect:" << rect() << "p:" << p;
        return false;
    }

    return true;
}

bool TestAbstractItemView::canWrap(QObject* o)
{ return qobject_cast<QAbstractItemView*>(o); }

