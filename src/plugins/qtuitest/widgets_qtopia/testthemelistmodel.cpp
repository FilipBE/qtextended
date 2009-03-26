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

#include "testthemelistmodel.h"
#include "testthemedview.h"

#include "testabstractitemview.h"
#include "testwidgetslog.h"

#include <ThemeListModelEntry>
#include <QThemeListModelEntry>
#include <QThemeTextItem>
#include <QThemeTemplateInstanceItem>

TestThemeListModel::TestThemeListModel(QObject* _q)
    : TestAbstractItemModel(_q)
{ TestWidgetsLog() << _q; }

QStringList TestThemeListModel::list() const
{
    TestWidgetsLog() << model;

    class ThemeListModelListGetter
    {
    public:
        ThemeListModelListGetter(QAbstractItemView *view)
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
                    list << TestWidget::printable(ti->text());
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
        QThemeListModelListGetter(QAbstractItemView *view)
         : model(qobject_cast<QThemeListModel*>(view->model())) {};

        QStringList getList() {
            QStringList list;
            for (int row = model->rowCount()-1; row >= 0; --row) {
                QThemeListModelEntry* e = model->themeListModelEntry(model->index(row));
                if (!e) continue;
                QList<QThemeTextItem*> items = allChildren(e->templateInstance());
                qSort(items.begin(), items.end(), TestThemedView::themeItemLessThan);
                foreach (QThemeTextItem *i, items) {
                    list << TestWidget::printable(i->text());
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

    if (qobject_cast<ThemeListModel*>(model)) {
        list = ThemeListModelListGetter(view).getList();
    } else if (qobject_cast<QThemeListModel*>(model)) {
        list = QThemeListModelListGetter(view).getList();
    }

    return list;
}

bool TestThemeListModel::canWrap(QObject* o)
{
    return TestAbstractItemModel::canWrap(o) && (qobject_cast<ThemeListModel*>(o) || qobject_cast<QThemeListModel*>(o));
}

