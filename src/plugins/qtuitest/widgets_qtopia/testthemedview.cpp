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

#include "testthemedview.h"
#include "testwidgetslog.h"

#include <ThemedView>
#include <QThemedView>
#include <QThemeTextItem>
#include <QApplication>
#include <Qtopia>

// xxx All of the code starting with 'old_' should be removed if/when
// xxx ThemedView is removed.  ThemedView and QThemedView are both handled
// xxx here because the API is almost identical.

TestThemedView::TestThemedView(QObject *_q)
: TestWidget(_q), old_q(qobject_cast<ThemedView*>(_q)), q(qobject_cast<QThemedView*>(_q))
{
    if (q) {
        connect(q,    SIGNAL(itemPressed(QThemeItem*)),
                this, SLOT(on_itemPressed(QThemeItem*)));
    } else if (old_q) {
        connect(old_q,SIGNAL(itemPressed(ThemeItem*)),
                this, SLOT(old_on_itemPressed(ThemeItem*)));
    }
    TestWidgetsLog() << old_q << q;
}

void TestThemedView::on_itemPressed(QThemeItem* item)
{
    TestWidgetsLog() << q << item->name();
    emit selected(item->name());
}

void TestThemedView::old_on_itemPressed(ThemeItem* item)
{
    TestWidgetsLog() << old_q << item->itemName();
    QString text = item->itemName();
    if (item->rtti() == ThemedView::Text) {
        text = static_cast<ThemeTextItem*>(item)->text();
    }
    emit selected(text);
}

QString TestThemedView::text() const
{
    QString ret;

    if (old_q) {
        QList<ThemeTextItem*> textItems;

        /* Try to find any text items. */
        QList<ThemeItem*> genericItems = old_q->findItems(QString(), ThemedView::Text);
        foreach (ThemeItem *gi, genericItems)
            if (gi && gi->isVisible())
                textItems << static_cast<ThemeTextItem*>(gi);

        if (textItems.isEmpty())
            return ret;

        /* Sort by geometry, so we get text in the same order a
         * user would read it. */
        qSort(textItems.begin(), textItems.end(),
                TestThemedView::old_themeItemLessThan);

        foreach (ThemeTextItem *i, textItems) {
            ret += i->text() + "\n";
        }
    } else {
        QList<QThemeTextItem*> textItems;

        /* Try to find any text items. */
        QList<QGraphicsItem*> genericItems = q->items();
        QThemeTextItem* ti;
        foreach (QGraphicsItem *gi, genericItems)
            if (gi->isVisible() && (ti = qgraphicsitem_cast<QThemeTextItem*>(gi)))
                textItems << ti;

        if (textItems.isEmpty())
            return ret;

        /* Sort by geometry, so we get text in the same order a
         * user would read it. */
        qSort(textItems.begin(), textItems.end(),
                TestThemedView::themeItemLessThan);

        foreach (QThemeTextItem *i, textItems) {
            ret += i->text() + "\n";
        }
    }

    ret.chop(1);

    return ret;
}

/* FIXME: this function returns the internal name of each theme item.
 * It should be changed to return the user-visible text only.
 * This can't be fixed until SelectWidget is implemented for ThemedView.
 */
QStringList TestThemedView::list() const
{
    QStringList ret;
    if (old_q) {
        TestWidgetsLog() << old_q;
        foreach(ThemeItem *item, old_q->findItems(QString())) {
            ret << item->itemName();
        }
    } else {
        TestWidgetsLog() << q;
        foreach(QGraphicsItem *item, q->items()) {
            if (QThemeItem* ti = qgraphicsitem_cast<QThemeItem*>(item))
                ret << ti->name();
        }
    }
    return ret;
}

QRect TestThemedView::visualRect(QString const &item) const
{
    QRect ret;

    if (old_q) {
        QList<ThemeItem*> tItem = old_q->findItems(item);
        if (0 == tItem.count())
            tItem << old_itemWithText(item);
        if (1 == tItem.count())
            ret = tItem.at(0)->rect();
    } else {
        QThemeItem* tItem = q->findItem(item);
        if (!tItem)
            tItem = itemWithText(item);
        if (tItem) {
            // Get rect of the item in viewport coordinates.
            ret = q->mapFromScene(tItem->mapToScene(tItem->boundingRect())).boundingRect();
            // Move rect from viewport coordinates to local coordinates.
            ret.moveTopLeft(q->mapFromGlobal(q->viewport()->mapToGlobal(ret.topLeft())));
        }
    }

    return ret;
}

bool TestThemedView::canSelect(QString const& item) const
{
    TestWidgetsLog() << item;
    return Qtopia::mousePreferred() && list().contains(item);
}

bool TestThemedView::select(QString const& item)
{
    TestWidgetsLog() << item;
    if (!canSelect(item)) return false;

    QtUiTest::mouseClick( mapToGlobal( visualRect(item).center() ) );
    return true;
}

bool TestThemedView::old_themeItemLessThan(ThemeItem *i1, ThemeItem *i2)
{
    QPoint c1 = i1->rect().center();
    QPoint c2 = i2->rect().center();
    if (c1.y() != c2.y()) return (c1.y() < c2.y());

    bool const rtl = QApplication::isRightToLeft();
    return ((rtl && c1.x() > c2.x()) || (!rtl && c1.x() < c2.x()));
}

bool TestThemedView::themeItemLessThan(QThemeItem *i1, QThemeItem *i2)
{
    Q_ASSERT(i1->scene());
    Q_ASSERT(i1->scene()->views().count());
    QGraphicsView* q = i1->scene()->views().at(0);
    QPoint c1 = q->mapFromScene(i1->mapToScene(i1->boundingRect())).boundingRect().center();
    QPoint c2 = q->mapFromScene(i2->mapToScene(i2->boundingRect())).boundingRect().center();
    if (c1.y() != c2.y()) return (c1.y() < c2.y());

    bool const rtl = QApplication::isRightToLeft();
    return ((rtl && c1.x() > c2.x()) || (!rtl && c1.x() < c2.x()));
}

ThemeItem* TestThemedView::old_itemWithText(QString const& text) const
{
    QList<ThemeItem*> items = old_q->findItems(QString(), ThemedView::Text);
    ThemeItem *ret = 0;
    foreach (ThemeItem* item, items) {
        ThemeTextItem *textItem = static_cast<ThemeTextItem*>(item);
        if (textItem->text() == text) {
            if (ret) {
                qWarning("QtUitest: found multiple items in theme view with "
                         "text '%s'", qPrintable(text));
                return 0;
            }
            ret = textItem;
        }
    }
    return ret;
}

QThemeItem* TestThemedView::itemWithText(QString const& text) const
{
    QThemeItem *ret = 0;
    foreach (QGraphicsItem* item, q->items()) {
        QThemeTextItem *textItem = qgraphicsitem_cast<QThemeTextItem*>(item);
        if (textItem && textItem->text() == text) {
            if (ret) {
                qWarning("QtUitest: found multiple items in theme view with "
                         "text '%s'", qPrintable(text));
                return 0;
            }
            ret = textItem;
        }
    }
    return ret;
}

bool TestThemedView::canWrap(QObject *o)
{
    return qobject_cast<ThemedView*>(o)
        || qobject_cast<QThemedView*>(o);
}

