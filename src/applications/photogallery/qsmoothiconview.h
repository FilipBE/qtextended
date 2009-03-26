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
#ifndef QSMOOTHICONVIEW_H
#define QSMOOTHICONVIEW_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiaglobal.h>
#include <QWidget>
#include <QTimeLine>
#include <QAbstractItemModel>
#include <QAbstractItemDelegate>

class ListItem;
class ItemHighlight;
class Scrollbar;
class QPushButton;
class QItemSelectionModel;

class QSmoothIconViewPrivate;
class QTOPIA_EXPORT QSmoothIconView : public QWidget
{
Q_OBJECT
public:
    enum ScrollHint { EnsureVisible, PositionAtTop,
                      PositionAtBottom, PositionAtCenter };
    enum SelectionMode { SingleSelection, NoSelection };
    enum Flow { LeftToRight, TopToBottom };

    QSmoothIconView(QWidget* = 0);
    virtual ~QSmoothIconView();

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel=0 );

    QAbstractItemDelegate *itemDelegate() const;
    void setItemDelegate(QAbstractItemDelegate *);

    void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel* selectionModel();

    QModelIndex currentIndex() const;
    QModelIndex indexAt(const QPoint &);
    QRect visualRect(const QModelIndex &);

    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode);

    int spacing() const;
    void setSpacing(int);

    QSize iconSize() const;
    void setIconSize( const QSize& );

    void setFixedRowCount( int rowCount );
    int fixedRowCount() const;

    Flow flow() const;
    void setFlow(Flow);

    void scrollTo(const QModelIndex &index, ScrollHint = EnsureVisible);
    void setEmptyText(const QString &);

    void setScrollbarEnabled(bool);

    Qt::TextElideMode textElideMode() const;
    void setTextElideMode(Qt::TextElideMode mode);

signals:
    void activated(const QModelIndex &index);
    void clicked(const QModelIndex &);
    void doubleClicked(const QModelIndex &);
    void pressed(const QModelIndex &);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

public slots:
    void reset();
    void setCurrentIndex(const QModelIndex &);
    void updateItem(const QModelIndex &);
    void ensureCurrentVisible();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void timerEvent(QTimerEvent *);

private slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &, int, int);
    void scrollViewport(const QPoint&);
    void scrollViewport(int);
    void setCurrentIndex(const QModelIndex &current, const QModelIndex &previous);
    void modelReset();

private:
    void init();
    void refresh();

    QSmoothIconViewPrivate *d;
};


#endif

