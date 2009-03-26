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

#include "qiconselector.h"

#include <QListWidget>
#include <QIcon>
#include <QString>
#include <QList>
#include <QSize>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

class QIconSelectorData
{
public:
    QIconSelectorData()
    {
    }

    QIconSelectorData( const QIcon &icn, const QString &t )
    {
        icon = icn;
        text = t;
    }

    QString text;
    QIcon icon;
};

class QIconSelectorPrivate
{
public:
    QIconSelectorPrivate()
    {
        count = 0;
        current = -1;
        mouseWasInsidePopup = false;
    }

    ~QIconSelectorPrivate()
    {
        foreach (QIconSelectorData* d, data)
            delete d;
    }

    QListWidget *list;
    QList<QIconSelectorData*> data;
    int count;
    int current;
    bool mouseWasInsidePopup;
    QIcon icon;
    QWidget* mPopup;
    QSize iconSize;
};

/*!
  \class QIconSelector
    \inpublicgroup QtBaseModule
  \brief The QIconSelector class provides an icon-based combo box implementation.

  QIconSelector operates similarly to a combo box but it has an important difference. It deals with
  information that is represented as both an icon and as text. The widget itself displays only icons
  while the pop-up selector displays both icon and text. This makes it suitable for placement in areas
  where there is limited horizontal space.

  Here is a QIconSelector next to a text field. When down, it takes up very little room.
  \image qiconselect-down.png "QIconSelector"

  Here is the pop-up selector. Both the icons and text are visible.
  \image qiconselect-up.png "QIconSelector pop-up selector"
*/

/*!
  Constructs a QIconSelector with the given \a parent.
*/
QIconSelector::QIconSelector( QWidget *parent )
    : QToolButton( parent )
{
    init();
}

/*!
  Constructs a QIconSelector with initial icon \a icn and the given \a parent.
*/
QIconSelector::QIconSelector( const QIcon &icn, QWidget *parent )
    : QToolButton( parent )
{
    setIcon( icn );

    init();
}

/*!
  Destroys a QIconSelector.
*/
QIconSelector::~QIconSelector()
{
    delete d->list;
    delete d->mPopup;
    delete d;
}

/*!
  \internal
  Initialize the QIconSelector.
*/
void QIconSelector::init()
{
    d = new QIconSelectorPrivate();
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    d->iconSize = QSize(iconSize, iconSize);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    d->mPopup = new QWidget(0, Qt::Popup | Qt::WindowStaysOnTopHint);
    QSoftMenuBar::setLabel( d->mPopup, Qt::Key_Select, QSoftMenuBar::Select );
    QSoftMenuBar::setLabel( d->mPopup, Qt::Key_Back, QSoftMenuBar::Cancel );

    d->list = new QListWidget(d->mPopup);
    d->list->setMouseTracking(true);
    d->list->setAutoScroll(false);
    d->list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->list->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->list->setLineWidth(1);
    d->list->move(0, 0);
    connect(d->list, SIGNAL(currentRowChanged(int)), this, SLOT(itemChanged(int)));

    connect(this, SIGNAL(clicked()), this, SLOT(popup()));

    connect(QApplication::desktop(), SIGNAL(workAreaResized(int)),
            this, SLOT(workAreaResized()));
}

/*!
  \reimp
*/
QSize QIconSelector::sizeHint() const
{
    const int border = style()->pixelMetric(QStyle::PM_ButtonMargin) * 2;
    int w = 0, h = 0;
    if( !d->iconSize.isEmpty() ) {
        w = d->iconSize.width();
        h = d->iconSize.height();
    } else if( d->icon.isNull() ) {
        w = h = style()->pixelMetric(QStyle::PM_SmallIconSize);
    }

    return QSize( w+border, h+border ).expandedTo( QApplication::globalStrut() );
}

/*!
  Returns the text that is associated with the icon that is currently being displayed.
*/
QString QIconSelector::text() const
{
    if (d->current >= 0 && d->current < d->data.count()) {
        QIconSelectorData *cur = d->data[d->current];
        if (cur)
            return cur->text;
    }
    return "";
}

/*!
  Returns the number of items the QIconSelector contains.
*/
uint QIconSelector::count() const
{
    return d->data.count();
}

/*!
  Adds an item to the QIconSelector with icon \a icn and text \a text.
*/
void QIconSelector::insertItem( const QIcon &icn, const QString &text )
{
    d->data.insert(d->count, new QIconSelectorData(icn, text));
    if(!d->count)
        setCurrentIndex(d->count);
    ++d->count;
}

/*!
  Removes the item at index \a index.
*/
void QIconSelector::removeIndex( int index )
{
    d->data.removeAt( index );
    if( d->count > 0 )
        --d->count;
    setCurrentIndex( d->count ? d->count : -1 );
}

/*!
  Removes all items.

  Note that no icon will be visible.
*/
void QIconSelector::clear()
{
    d->data.clear();
    setCurrentIndex( -1 );
}

/*!
  Returns the index of the currently selected item.
  This will be \c -1 if no items are selected.
*/
int QIconSelector::currentIndex() const
{
    return d->current;
}

/*!
  Sets the current selected item to the item at index \a index.
  Set \a index to \c -1 to select no items. Note that no icon will be visible if
  no items are selected.
*/
void QIconSelector::setCurrentIndex( int index )
{
    if( index >= (int)d->data.count() || index < 0 )
        return;

    d->current = index;
    if( index == -1 )
        setIcon( QIcon() );
    else
    {
        setIcon( d->data.at( index )->icon );
        emit activated( index );
    }
}

/*!
  Sets the displayed icon to \a icn.
  Note that this does not change the selected item so using this function
  may confuse the user. It is provided so that a suitable icon can be
  displayed when nothing is selected.
*/
void QIconSelector::setIcon( const QIcon &icn )
{
    d->icon = icn;
    QToolButton::setIcon(icn);
    updateGeometry();
}

/*!
  Returns the icon that is being displayed.
*/
QIcon QIconSelector::icon() const
{
    return d->icon;
}

/*!
  Opens the pop-up selector.
*/
void QIconSelector::popup()
{
    d->list->clear();
    d->mouseWasInsidePopup = false;

    QListIterator<QIconSelectorData *> it( d->data );
    foreach(QIconSelectorData *data, d->data)
    {
        QListWidgetItem *item = new QListWidgetItem(data->text, d->list);
        item->setIcon(data->icon);
    }

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect(desktop->availableGeometry(desktop->screenNumber(this)));
    QPoint pos = mapToGlobal( QPoint(desktopRect.x(),desktopRect.y()+height()) );

    int x = pos.x();
    int y = pos.y();
    int totalItemsHeight = 0;

    for( int i = 0 ; i < d->list->count() ; ++i )
        totalItemsHeight += d->list->sizeHintForRow(i);

    int frameWidth = d->list->frameWidth();
    int totalHeight = totalItemsHeight+(frameWidth*2)+2;
    int w = d->list->sizeHintForColumn(0)+(frameWidth*2)+2;
    int h = totalHeight;

    //
    //  Ensure the entire widget will be visible
    //

    // fit for x, no resize. Assumes will w < desktopRect.width()
    // doesn't flip to left of x, rather squishes.
    if (x + w > desktopRect.right())
        x = desktopRect.right() - w;
    else if (x < desktopRect.left())
        x = desktopRect.left();

    // fit for y.  Try down, if not try up, else squish (e.g. snap to bottom)
    // first meet assumptions about y on screen and list height able to fit somehow.
    y = qMax(y, desktopRect.top());
    y = qMin(y, desktopRect.bottom());
    if (h > desktopRect.height()) {
        h = desktopRect.height();
    }

    d->mPopup->resize(w, h);
    d->list->resize(w - (frameWidth*2), h - (frameWidth*2));
    d->list->move(frameWidth, frameWidth);

    if (y + h > desktopRect.bottom()) {
        if (y - h >= desktopRect.top())
            y = y - h;
        else
            y = desktopRect.bottom() - h;
    }

    d->list->installEventFilter(this);
    d->list->viewport()->installEventFilter(this);
    d->mPopup->installEventFilter(this);

    bool block = d->list->signalsBlocked();
    d->list->blockSignals(true);
    d->list->setCurrentRow(currentIndex());
    d->list->blockSignals(block);

    d->mPopup->move(x, y);
    d->mPopup->show();
    d->mPopup->raise();

    d->list->update();
    d->list->setFocus();
}

/*!
  Closes the pop-up selector.
*/
void QIconSelector::popdown()
{
    d->list->clearFocus();
    d->list->removeEventFilter(this);
    d->list->viewport()->removeEventFilter(this);
    d->mPopup->removeEventFilter(this);
    d->mPopup->hide();
}

/*!
  Closes the pop-up selector and selects the item at index \a index.
*/
void QIconSelector::itemSelected( int index )
{
    popdown();
    setCurrentIndex( index );
}

/*!
  \internal

  Make sure the current item is visible.
*/
void QIconSelector::itemChanged( int index)
{
    d->list->scrollToItem(d->list->item(index));
}

/*!
  \internal

  Reposition the icon selector when the screen is resized.
*/
void QIconSelector::workAreaResized()
{
    if (d->mPopup->isVisible()) {
        popdown();
        popup();
    }
}

/*!
  \reimp
*/
bool QIconSelector::eventFilter( QObject *obj, QEvent *e ) //mostly copied from qcombobox
{
    if( !e ) return true;

    if( obj == d->list || obj == d->list->viewport() || obj == d->mPopup )
    {
        QKeyEvent *ke = (QKeyEvent *)e;
        QMouseEvent *me = (QMouseEvent *)e;
        switch( e->type() )
        {
            case QEvent::KeyPress:
            {
                switch( ke->key() )
                {
                    case Qt::Key_Up:
                        if(!(ke->modifiers() & Qt::AltModifier))
                        {
                            if(d->list->currentRow() > 0)
                                d->list->setCurrentRow(d->list->currentRow() - 1);
                            else
                                d->list->setCurrentRow(d->list->count() - 1);
                        }
                        return true;

                    case Qt::Key_Down:
                        if(!(ke->modifiers() & Qt::AltModifier))
                        {
                            if(d->list->currentRow() > -1 &&
                                    d->list->currentRow() < d->list->count() - 1)
                                d->list->setCurrentRow(d->list->currentRow() + 1);
                            else
                                d->list->setCurrentRow(0);
                        }
                        return true;


                    case Qt::Key_F4:
                    case Qt::Key_Escape:
                    case Qt::Key_Back:
                    case Qt::Key_No:
                        popdown();
                        return true;
                        break;


                    case Qt::Key_Select:
                    case Qt::Key_Enter:
                    case Qt::Key_Return:
                        itemSelected(d->list->currentRow());
                        return false;

                    default:
                        break;
                }
            }
            case QEvent::MouseButtonPress:
            {
                if(!d->list->rect().contains(me->pos()))
                    popdown();
                else
                    itemSelected(d->list->row(d->list->itemAt(me->pos())));
                return true;
            }
            default:
                break;
        }
    }

    return QWidget::eventFilter( obj, e );
}

/*!
  \reimp
*/
void QIconSelector::keyPressEvent( QKeyEvent *e )
{
    QToolButton::keyPressEvent( e );
}

/*!
  \fn QIconSelector::activated(int index)
  This signal is emitted when an item is activated. The item is indicated by \a index.
*/

