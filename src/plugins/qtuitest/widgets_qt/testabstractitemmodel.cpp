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

#include "testabstractitemmodel.h"

#include "testabstractitemview.h"
#include "testwidgetslog.h"

TestAbstractItemModel::TestAbstractItemModel(QObject* _q)
    : model(qobject_cast<QAbstractItemModel*>(_q)),
      view (qobject_cast<QAbstractItemView*>(_q->property("_q_qtuitest_itemview").value<QObject*>()))
{}

QStringList TestAbstractItemModel::list() const
{
    class QModelListGetter : public QModelViewIterator<QAbstractItemView>
    {
    public:
        QModelListGetter(QAbstractItemView *view)
         : QModelViewIterator<QAbstractItemView>(view) {};

        QStringList getList() {
            list.clear();
            iterate();
            return list;
        }
    protected:
        virtual void visit(QModelIndex const &index)
        { list << TestWidget::printable(index.data().toString()); }

        QStringList list;
    };

    return QModelListGetter(view).getList();
}

QRect TestAbstractItemModel::visualRect(const QString& item) const
{
    QRect ret;

    class QModelRectGetter : public QModelViewIterator<QAbstractItemView>
    {
    public:
        QModelRectGetter(QAbstractItemView *view, QString const &item)
         : QModelViewIterator<QAbstractItemView>(view), matches(0), m_item(item) {};
        QRect rect;
        int matches;

    protected:
        void visit(QModelIndex const &index) {

            /* FIXME get rid of this special case */
            /* Don't consider the non-active dates in calendar */
            if (view()->inherits("QCalendarView")
                    && view()->palette().color(QPalette::Disabled, QPalette::Text)
                        == qvariant_cast<QColor>(index.data(Qt::TextColorRole))) {
                return;
            }

            if (TestWidget::printable(index.data().toString()) == m_item) {
                ++matches;
                rect = view()->visualRect(index);
            }
        }
    private:
        QString m_item;
    };

    QModelRectGetter rectGetter(view, item);
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

bool TestAbstractItemModel::canWrap(QObject* o)
{
    return qobject_cast<QAbstractItemModel*>(o) && qobject_cast<QAbstractItemView*>(o->property("_q_qtuitest_itemview").value<QObject*>());
}

