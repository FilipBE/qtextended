/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QCOLUMNVIEW_P_H
#define QCOLUMNVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qcolumnview.h"

#ifndef QT_NO_QCOlUMNVIEW

#include <private/qabstractitemview_p.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qtimeline.h>
#include <QtGui/qabstractitemdelegate.h>
#include <QtGui/qabstractitemview.h>
#include <QtGui/qitemdelegate.h>
#include <qlistview.h>
#include <qevent.h>
#include <qscrollbar.h>

QT_BEGIN_NAMESPACE

class QColumnViewPreviewColumn : public QAbstractItemView {

public:
    QColumnViewPreviewColumn(QWidget *parent) : QAbstractItemView(parent), previewWidget(0) {
    }

    void setPreviewWidget(QWidget *widget) {
        previewWidget = widget;
        setMinimumWidth(previewWidget->minimumWidth());
    }

    void resizeEvent(QResizeEvent * event){
        if (!previewWidget)
            return;
        previewWidget->resize(
                qMax(previewWidget->minimumWidth(), event->size().width()),
                previewWidget->height());
        QSize p = viewport()->size();
        QSize v = previewWidget->size();
        horizontalScrollBar()->setRange(0, v.width() - p.width());
        horizontalScrollBar()->setPageStep(p.width());
        verticalScrollBar()->setRange(0, v.height() - p.height());
        verticalScrollBar()->setPageStep(p.height());

        QAbstractScrollArea::resizeEvent(event);
    }

    QRect visualRect(const QModelIndex &) const
    {
        return QRect();
    }
    void scrollTo(const QModelIndex &, ScrollHint)
    {
    }
    QModelIndex indexAt(const QPoint &) const
    {
        return QModelIndex();
    }
    QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers)
    {
        return QModelIndex();
    }
    int horizontalOffset () const {
        return 0;
    }
    int verticalOffset () const {
        return 0;
    }
    QRegion visualRegionForSelection(const QItemSelection &) const
    {
        return QRegion();
    }
    bool isIndexHidden(const QModelIndex &) const
    {
        return false;
    }
    void setSelection(const QRect &, QItemSelectionModel::SelectionFlags)
    {
    }
private:
    QWidget *previewWidget;
};

class Q_AUTOTEST_EXPORT QColumnViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QColumnView)

public:
    QColumnViewPrivate();
    ~QColumnViewPrivate();
    void initialize();

    QAbstractItemView *createColumn(const QModelIndex &index, bool show);

    void updateScrollbars();
    void closeColumns(const QModelIndex &parent = QModelIndex(), bool build = false);
    void doLayout();
    void setPreviewWidget(QWidget *widget);

    void _q_gripMoved(int offset);
    void _q_changeCurrentColumn();
    void _q_clicked(const QModelIndex &index);

    QList<QAbstractItemView*> columns;
    QVector<int> columnSizes; // used during init and corner moving
    bool showResizeGrips;
    int offset;
    QTimeLine currentAnimation;
    QWidget *previewWidget;
    QAbstractItemView *previewColumn;
};

/*!
 * This is a delegate that will paint the triangle
 */
class QColumnViewDelegate : public QItemDelegate
{

public:
    explicit QColumnViewDelegate(QObject *parent = 0) : QItemDelegate(parent) {};
    ~QColumnViewDelegate() {};

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};
#endif // QT_NO_QCOLUMNVIEW


QT_END_NAMESPACE
#endif //QCOLUMNVIEW_P_H

