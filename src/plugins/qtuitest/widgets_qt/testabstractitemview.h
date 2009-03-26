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

#ifndef TESTABSTRACTITEMVIEW_H
#define TESTABSTRACTITEMVIEW_H

#include "testwidget.h"

#include <QAbstractItemView>
#include <QTime>

class TestAbstractItemView : public TestWidget,
    public QtUiTest::TextWidget, public QtUiTest::ListWidget,
    public QtUiTest::SelectWidget
{
    Q_OBJECT
    Q_INTERFACES(
            QtUiTest::TextWidget
            QtUiTest::ListWidget
            QtUiTest::SelectWidget)

public:
    TestAbstractItemView(QObject*);

    virtual QString text() const;
    virtual QString selectedText() const;

    virtual QStringList list() const;
    virtual QRect visualRect(QString const&) const;
    virtual bool ensureVisible(QString const&);

    virtual bool isMultiSelection() const;

    virtual bool canSelect(QString const&) const;
    virtual bool canSelectMulti(QStringList const&) const;
    virtual bool select(QString const&);
    virtual bool selectMulti(QStringList const&);

    static bool canWrap(QObject*);
    static void waitForModelUpdate(QAbstractItemModel*);

signals:
    void selected(const QString&);

private slots:
    void on_activated(QModelIndex const&);

private:
    QAbstractItemView *q;
    QTime m_lastActivatedTimer;
};

template <typename T>
class QModelViewIterator
{
public:
    QModelViewIterator(T* view)
        : m_view(view)
    {}

    virtual ~QModelViewIterator()
    {}

    void iterate()
    {
        QModelIndexList seen;
        iterate(QModelIndex(), &seen);
    }

protected:
    T* view() const
    { return m_view; }
    QAbstractItemModel* model() const
    { return m_view->model(); }

    virtual void visit(QModelIndex const&) = 0;

private:
    void iterate(QModelIndex const &index, QModelIndexList *seen)
    {
        if (index.isValid()) {
            visit(index);
            (*seen) << index;
        }
        for (int i = 0, max_i = model()->rowCount(index); i < max_i; ++i) {
            for (int j = 0, max_j = (view() && (view()->inherits("QListView") || view()->inherits("QSmoothList"))
                        ? 1
                        : model()->columnCount(index));
                    j < max_j; ++j) {
                QModelIndex child;
                if (model()->hasIndex(i, j, index)) {
                    child = model()->index(i, j, index);
                }
                if (child.isValid() && !seen->contains(child)) {
#if 0
                    /* Very verbose! */
                    Debug() << "child at (" << i << "," << j << ")" << child.data();
#endif
                    iterate(child, seen);
                }
            }
        }
    }

    T *m_view;
};


#endif

