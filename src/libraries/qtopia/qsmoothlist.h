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

#ifndef QSMOOTHLIST_P_H
#define QSMOOTHLIST_P_H

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

class QSmoothListPrivate;
class QTOPIA_EXPORT QSmoothList : public QWidget
{
Q_OBJECT
public:
    enum ScrollHint { EnsureVisible, PositionAtTop, PositionAtBottom, PositionAtCenter, ImmediateVisible };
    enum SelectionMode { SingleSelection, NoSelection };

    QSmoothList(QWidget * = 0, Qt::WFlags = 0);
    virtual ~QSmoothList();

    void setModel(QAbstractItemModel *);
    void setItemDelegate(QAbstractItemDelegate *);

    void setIconSize(const QSize &);
    QSize iconSize() const;
    Qt::TextElideMode textElideMode() const;
    void setTextElideMode(Qt::TextElideMode mode);
    QModelIndex currentIndex() const;
    QModelIndex indexAt(const QPoint &) const;
    QAbstractItemDelegate *itemDelegate() const;
    QAbstractItemModel *model() const;
    QRect visualRect(const QModelIndex &) const;
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode);
    void scrollTo(const QModelIndex &index, ScrollHint = EnsureVisible);
    void setEmptyText(const QString &);

    void setOpaqueItemBackground(bool opaque);
    bool hasOpaqueItemBackground() const;

    QSize sizeHint() const;
    
    void setExpanded(const QModelIndex& index, bool set);
    bool isExpanded(const QModelIndex& index) const;

signals:
    void activated(const QModelIndex &index);
    void clicked(const QModelIndex &);
    void doubleClicked(const QModelIndex &);
    void pressed(const QModelIndex &);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void expanded(const QModelIndex& index);
    void collapsed(const QModelIndex& index);

public slots:
    void reset();
    void clearSelection();
    void setCurrentIndex(const QModelIndex &, ScrollHint hint = EnsureVisible);
    void update(const QModelIndex &);
    void scrollToBottom();
    void scrollToTop();

    void expand(const QModelIndex& index);
    void collapse(const QModelIndex& index);
    void expandAll();
    void collapseAll();

protected slots:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsRemoved(const QModelIndex &, int start, int end);
    virtual void modelAboutToBeReset();
    virtual void modelReset();

private slots:
    void selectTimeout();
    void updateHighlight();
    void updateCompleted();
    void tryHideScrollBar();
    void doUpdate(bool force = false);
    void tickStart();
    void emitActivated();
    void backgroundChanged(const QPixmap &);

protected:
    virtual bool event(QEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void moveEvent(QMoveEvent *);

private:
    void init();
    void go(int dir, bool autoRepeatEvent);
    void ensureVisibleItemsLoaded(bool force = false);
    void refresh(bool animated = true);
    void fixupPosition(int period = 0);
    void fixupHighlight(int period = 0);
    void ensureCurrentItemVisible(int period = 0);
    void prepareFlick(qreal velocity);
    bool flickList(qreal velocity);
    void scrollTo(int idx, ScrollHint hint);
    void scrollTo(qreal, int from = -1);
    qreal maxListPos() const;
    void hideScrollBar();
    void showScrollBar(int pause = 0);
    bool scrollBarIsVisible() const;
    void invalidateItem(int idx);
    void updateCurrentIndex(int idx);

    friend class QSmoothListPrivate;

    QSmoothListPrivate *d;
};

#endif
