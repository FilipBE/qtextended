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

#include "qsmoothlistwidget_p.h"
#include <QStandardItemModel>

class QSmoothListWidgetPrivate
{
public:
    QSmoothListWidgetPrivate(QObject *parent=0)
        :sorting(false), sortOrder(Qt::AscendingOrder), model(new QStandardItemModel(parent))
    {
    }

    ~QSmoothListWidgetPrivate()
    {
    }

    bool sorting;
    Qt::SortOrder sortOrder;
    QStandardItemModel *model;
};


/*!
    \class QSmoothListWidgetItem
    \inpublicgroup QtBaseModule
    \brief The QSmoothListWidgetItem class provides an item for use with the
    QSmoothListWidget item view class.

    \ingroup model-view

    QSmoothListWidgetItem is used to represent items in a list provided by the
    QSmoothListWidget class. Each item can hold several pieces of information,
    and will display these appropriately.

    The item view convenience classes use a classic item-based interface
    rather than a pure model/view approach. For a more flexible list view
    widget, consider using the QSmoothList class with a standard model.

    List items can be automatically inserted into a list when they are
    constructed by specifying the list widget:

    They can also be created without a parent widget, and later inserted into
    a list (see \l{QSmoothListWidget::insertItem()}).

    QSmoothListWidgetItem is designed to be a drop-in replacement for simple uses
    of QSmoothListWidgetItem, when you are using a QSmoothListWidget instead of a
    QSmoothListWidget. QSmoothListWidgetItem does behave slightly differently though,
    most notably in that it cannot be hidden.

    List items are typically used to display text() and an icon(). These are
    set with the setText() and setIcon() functions. The appearance of the text
    can be customized with setFont(), setForeground(), and setBackground().
    Text in list items can be aligned using the setTextAlignment() function.

    By default, items are enabled, selectable, and checkable.
    Each item's flags can be changed by calling setFlags() with the appropriate
    value (see \l{Qt::ItemFlags}). Checkable items can be checked, unchecked and
    partially checked with the setCheckState() function. The corresponding
    checkState() function indicates what check state the item currently has.

    \sa QListWidgetItem, QSmoothListWidget, QSmoothList
*/
/*!
    \fn int QSmoothListWidgetItem::type() const

    Returns the type passed to the QSmoothListWidgetItem constructor.
 */

/*!
    \fn virtual void QSmoothListWidgetItem::setData(int role, const QVariant& value)

    Sets the item's data for the given \a role to the specified \a value.

    Same as QStandardItem::setData() but with the parameters reversed.

    Included for compatibilty with QListWidgetItem.
*/
/*!
    \fn QSmoothListWidget *QSmoothListWidgetItem::smoothListWidget() const

    Returns the smooth list widget that contains the item.
*/
/*!
    \fn QSmoothListWidgetItem::QSmoothListWidgetItem(QSmoothListWidget *parent, int type)

    Constructs an empty smooth list widget item of the specified \a type with the
    given \a parent.
    If the parent is not specified, the item will need to be inserted into a
    smooth list widget with QSmoothListWidget::insertItem().

    \sa type()
 */
/*!
    \fn QSmoothListWidgetItem::QSmoothListWidgetItem(const QString &text, QSmoothListWidget *parent, int type)

    Constructs an empty smooth list widget item of the specified \a type with the
    given \a text and \a parent.
    If the parent is not specified, the item will need to be inserted into a
    smooth list widget with QSmoothListWidget::insertItem().

    \sa type()
 */
/*!
    \fn QSmoothListWidgetItem::QSmoothListWidgetItem(const QIcon &icon, const QString &text, QSmoothListWidget *parent, int type)

    Constructs an empty smooth list widget item of the specified \a type with the
    given \a icon, \a text and \a parent.
    If the parent is not specified, the item will need to be inserted into a
    smooth list widget with QSmoothListWidget::insertItem().

    \sa type()
 */

/*!
    \class QSmoothListWidget
    \inpublicgroup QtBaseModule
    \brief The QSmoothListWidget class provides an item-based list widget.

    \ingroup model-view


    QSmoothListWidget is a convenience class that provides a list view similar to
    the one supplied by QListView, but with a classic item-based interface
    for adding and removing items. QSmoothListWidget uses an internal model to
    manage each QSmoothListWidgetItem in the list.

    QSmoothListWidget is a drop-in replacement for QListWidget, but using a QSmoothList.
    This means that unlike a QListWidget a QSmoothListWidget does not handle editing,
    multiple selected items or drag and drop. The other noticable difference is that
    \l QSmoothListWidget::scrollToItem() takes a \l QSmoothList::ScrollHint as its second
    parameter, not a \l QAbstractItemView::ScrollHint.

    For a more flexible list view widget, use the QSmoothList or the QListView
    class with a standard model.

    There are two ways to add items to the list: they can be constructed with
    the smooth list widget as their parent widget, or they can be constructed with
    no parent widget and added to the list later. If a smooth list widget already
    exists when the items are constructed, the first method is easier to use:

    If you need to insert a new item into the list at a particular position,
    it is more required to construct the item without a parent widget and
    use the insertItem() function to place it within the list.  The smooth list
    widget will take ownership of the item.

    For multiple items, insertItems() can be used instead. The number of
    items in the list is found with the count() function.
    To remove items from the list, use takeItem().

    The current item in the list can be found with currentItem(), and changed
    with setCurrentItem(). The user can also change the current item by
    navigating with the keyboard or clicking on a different item. When the
    current item changes, the currentItemChanged() signal is emitted with the
    new current item and the item that was previously current.

    \sa QSmoothListWidgetItem, QSmoothList, QListWidget, {Model/View Programming}
 */
/*!
    \fn void QSmoothListWidget::addItem(QSmoothListWidgetItem *item)

    Inserts the \a item at the the end of the smooth list widget.

    \warning A QSmoothListWidgetItem can only be added to a
    QSmoothListWidget once. Adding the same QSmoothListWidgetItem multiple
    times to a QSmoothListWidget will result in undefined behavior.

    \sa insertItem()
 */

/*!
    \fn void QSmoothListWidget::addItem(const QString &label)

    Inserts an item with the text \a label at the end of the smooth list
    widget.
 */

/*!
    \fn void QSmoothListWidget::addItems(const QStringList &labels)

    Inserts items with the text \a labels at the end of the smooth list widget.

    \sa insertItems()
 */

/*!
    \fn void QSmoothListWidget::itemPressed(QSmoothListWidgetItem *item)

    This signal is emitted with the specified \a item when a mouse button is pressed
    on an item in the widget.

    \sa itemClicked(), itemDoubleClicked()
 */

/*!
    \fn void QSmoothListWidget::itemClicked(QSmoothListWidgetItem *item)

    This signal is emitted with the specified \a item when a mouse button is clicked
    on an item in the widget.

    \sa itemPressed(), itemDoubleClicked()
 */

/*!
    \fn void QSmoothListWidget::itemDoubleClicked(QSmoothListWidgetItem *item)

    This signal is emitted with the specified \a item when a mouse button is double
    clicked on an item in the widget.

    \sa itemClicked(), itemPressed()
 */

/*!
    \fn void QSmoothListWidget::itemActivated(QSmoothListWidgetItem *item)

    This signal is emitted when the \a item is activated. The \a item
    is activated when the user clicks or double clicks on it,
    depending on the system configuration. It is also activated when
    the user presses the activation key (on Windows and X11 this is
    the \gui Return key, on Mac OS X it is \key{Ctrl+0}).
 */

/*!
    \fn void QSmoothListWidget::currentItemChanged(QSmoothListWidgetItem *current, QSmoothListWidgetItem *previous)

    This signal is emitted whenever the current item changes. The \a
    previous item is the item that previously had the focus, \a
    current is the new current item.
 */

/*!
  \fn void QSmoothListWidget::currentRowChanged(int currentRow)

  This signal is emitted whenever the current item changes. The \a currentRow
  is the row of the current item. If there is no current item, the \a currentRow is -1.
 */

/*!
    Constructs an empty QSmoothListWidget with the given \a parent.
 */
QSmoothListWidget::QSmoothListWidget(QWidget *parent)
    :QSmoothList(parent), d(new QSmoothListWidgetPrivate(this))
{
    this->setModel(d->model);
    connect(this, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(emit_currentItemRowChanged(QModelIndex,QModelIndex)));
    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(emit_itemActivated(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(emit_itemClicked(QModelIndex)));
    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(emit_itemDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(pressed(QModelIndex)),
            this, SLOT(emit_itemPressed(QModelIndex)));
}

/*!
    Destroys the smooth list widget and all its items.
 */
QSmoothListWidget::~QSmoothListWidget()
{
    delete d;
}

void QSmoothListWidget::emit_currentItemRowChanged ( const QModelIndex& current, const QModelIndex& previous )
{
    QSmoothListWidgetItem *curItem =
        static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(current));
    QSmoothListWidgetItem *prevItem =
        static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(previous));
    emit currentItemChanged(curItem, prevItem);
    emit currentRowChanged(current.row());
}

void QSmoothListWidget::emit_itemActivated (const QModelIndex &index)
{
    emit itemActivated(static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(index)));
}

void QSmoothListWidget::emit_itemClicked (const QModelIndex &index)
{
    emit itemClicked(static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(index)));
}

void QSmoothListWidget::emit_itemDoubleClicked (const QModelIndex &index)
{
    emit itemDoubleClicked(static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(index)));
}

void QSmoothListWidget::emit_itemPressed (const QModelIndex &index)
{
    emit itemPressed(static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(index)));
}

void QSmoothListWidget::addItem(const QString &label)
{
    new QSmoothListWidgetItem(label, this);
}

void QSmoothListWidget::addItem(QSmoothListWidgetItem *item)
{
    insertItem(d->model->rowCount(),item);
}

void QSmoothListWidget::addItems(const QStringList &labels)
{
    foreach(QString label, labels)
        addItem(label);
}

/*!
  \property QSmoothListWidget::count
  \brief the number of items in the list.
 */
int QSmoothListWidget::count() const
{
    return d->model->rowCount();
}

/*!
  Returns the current item.
 */
QSmoothListWidgetItem * QSmoothListWidget::currentItem () const
{
    return static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(currentIndex()));
}

/*!
  \property QSmoothListWidget::currentRow
  \brief the row of the current item.

  The row will also be selected.
 */
int QSmoothListWidget::currentRow () const
{
    return currentIndex().row();
}

/*!
  Finds items with the text that matches the string \a text using the given \a flags.
 */
QList<QSmoothListWidgetItem *> QSmoothListWidget::findItems ( const QString & text, Qt::MatchFlags flags ) const
{
    QList<QSmoothListWidgetItem*> ret;
    foreach(QStandardItem* item, d->model->findItems(text,flags))
        ret << static_cast<QSmoothListWidgetItem*>(item);

    return ret;
}

/*!
    Inserts the \a item at the position in the list given by \a row.

    \sa addItem()
 */
void QSmoothListWidget::insertItem ( int row, QSmoothListWidgetItem * item )
{
    d->model->insertRow(row, item);
    item->d->smoothListWidget = this;

    if(d->sorting)
        d->model->sort(0,d->sortOrder);
}

/*!
    Inserts an item with the text \a label in the list widget at the
    position given by \a row.

    \sa addItem()
 */
void QSmoothListWidget::insertItem ( int row, const QString & label )
{
    insertItem(row, new QSmoothListWidgetItem(label));
}

/*!
    Inserts items from the list of \a labels into the list, starting at the
    given \a row.

    \sa insertItem(), addItem()
 */
void QSmoothListWidget::insertItems ( int row, const QStringList & labels )
{
    foreach(QString label, labels)
        insertItem(row++, label);
}

bool QSmoothListWidget::isSortingEnabled () const
{
    return d->sorting;
}

/*!
    Returns the item that occupies the given \a row in the list if one has been
    set; otherwise returns 0.

    \sa row()
 */
QSmoothListWidgetItem * QSmoothListWidget::item ( int row ) const
{
    return static_cast<QSmoothListWidgetItem*>
        (d->model->itemFromIndex(d->model->index(row,0)));
}

/*!
    Returns a pointer to the item at the coordinates \a p.
 */
QSmoothListWidgetItem * QSmoothListWidget::itemAt ( const QPoint & p ) const
{
    return static_cast<QSmoothListWidgetItem*>
        (d->model->itemFromIndex(indexAt(p)));
}

/*!
    \fn QSmoothListWidgetItem *QSmoothListWidget::itemAt(int x, int y) const
    \overload

    Returns a pointer to the item at the coordinates (\a x, \a y).
 */
QSmoothListWidgetItem * QSmoothListWidget::itemAt ( int x, int y ) const
{
    return itemAt(QPoint(x,y));
}

/*!
    Returns the row containing the given \a item.

    \sa item()
 */
int QSmoothListWidget::row ( const QSmoothListWidgetItem * item ) const
{
    return d->model->indexFromItem(item).row();
}

/*!
  Sets the current item to \a item.

  Depending on the current selection mode, the item may also be selected.
*/
void QSmoothListWidget::setCurrentItem ( QSmoothListWidgetItem * item )
{
    setCurrentIndex(d->model->indexFromItem(item));
}

void QSmoothListWidget::setCurrentRow ( int row )
{
    setCurrentIndex(d->model->index(row,0));
}

/*!
    \property QSmoothListWidget::sortingEnabled
    \brief whether sorting is enabled

    If this property is true, sorting is enabled for the list; if the
    property is false, sorting is not enabled. The default value is false.
*/
void QSmoothListWidget::setSortingEnabled ( bool enable )
{
    d->sorting = enable;
    if(d->sorting)
        d->model->sort(0, d->sortOrder);
}

/*!
  Sorts all the items in the list widget according to the specified \a order.
 */
void QSmoothListWidget::sortItems ( Qt::SortOrder order)
{
    d->sortOrder = order;
    d->model->sort(0, order);
}

/*!
    Removes and returns the item from the given \a row in the smooth list widget; otherwise returns 0.

    Items removed from a smooth list widget will not be managed by Qt, and will need to be deleted manually.

    \sa insertItem(), addItem()
 */
QSmoothListWidgetItem * QSmoothListWidget::takeItem ( int row )
{
    QSmoothListWidgetItem *ret = static_cast<QSmoothListWidgetItem*>(d->model->takeItem(row));
    ret->d->smoothListWidget = 0;
    return ret;
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a item.
 */
QRect QSmoothListWidget::visualItemRect ( const QSmoothListWidgetItem * item ) const
{
    return visualRect(d->model->indexFromItem(item));
}

/*!
    Removes all items and selections in the view.

    Note that all items will be permanently deleted.
 */
void QSmoothListWidget::clear()
{
    d->model->clear();
}

/*!
    Scrolls the view if necessary to ensure that the \a item is
    visible. The \a hint parameter specifies more precisely where the
    \a item should be located after the operation.

    Note that QSmoothList::ScrollHint is used instead of QAbstractItemView::ScrollHint
 */
void QSmoothListWidget::scrollToItem ( const QSmoothListWidgetItem * item, QSmoothList::ScrollHint hint)
{
    scrollTo(d->model->indexFromItem(item), hint);
}

/*!
  Returns the QModelIndex assocated with the given \a item.
 */
QModelIndex QSmoothListWidget::indexFromItem ( QSmoothListWidgetItem * item ) const
{
    return d->model->indexFromItem(item);
}

/*!
  Returns a pointer to the QSmoothListWidgetItem assocated with the given \a index.
 */
QSmoothListWidgetItem * QSmoothListWidget::itemFromIndex ( const QModelIndex & index ) const
{
    return static_cast<QSmoothListWidgetItem*>(d->model->itemFromIndex(index));
}

