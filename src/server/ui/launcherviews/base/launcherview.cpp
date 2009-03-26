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

#include "launcherview.h"
#include "uifactory.h"
#include <QtopiaApplication>
#include <QResizeEvent>
#include <QSoftMenuBar>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QtopiaServiceDescription>
#include <QMenu>
#include <QDesktopWidget>
#include <QSpeedDial>
#include <QPainter>
#include <QSet>
#include <QPixmap>
#include <QtopiaItemDelegate>

#include <QContentFilter>
#include <QKeyEvent>
#include <QAbstractProxyModel>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QMimeType>

////////////////////////////////////////////////////////////////
//
// LauncherViewListView implementation

void LauncherViewListView::currentChanged( const QModelIndex &current, const QModelIndex &previous ) {
    QListView::currentChanged( current, previous );
    emit currentIndexChanged( current, previous );
}

void LauncherViewListView::focusOutEvent(QFocusEvent *)
{
    // Don't need an update.
}

void LauncherViewListView::focusInEvent(QFocusEvent *)
{
    // Don't need an update.
}

bool LauncherViewListView::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        // suppress unneeded viewport update
        return true;
    default:
        break;
    }

    return QListView::viewportEvent(e);
}

void LauncherViewListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QModelIndex index = currentIndex();

    int scrollValue = verticalScrollBar()->value();

    QListView::rowsAboutToBeRemoved(parent, start, end);

    if (index.row() >= start && index.row() <= end && end + 1 < model()->rowCount(parent)) {
        selectionModel()->setCurrentIndex(
                model()->index( end + 1, index.column(), parent ),
                QItemSelectionModel::ClearAndSelect);
    }

    if (index.row() >= start) {
        int adjustedValue = index.row() > end
                ? scrollValue - end + start - 1
                : scrollValue - index.row() + start;

        verticalScrollBar()->setValue(adjustedValue > 0 ? adjustedValue : 0);
    }
}

void LauncherViewListView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    if (Qtopia::mousePreferred())
        selectionModel()->clearSelection();

    scrollTo(currentIndex());
}

//===========================================================================

class LauncherViewDelegate : public QtopiaItemDelegate
{
public:
    LauncherViewDelegate(QObject *parent=0)
        : QtopiaItemDelegate(parent)
    {
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
    {
        QSize sh = QtopiaItemDelegate::sizeHint(opt, index);
#ifdef QTOPIA_HOMEUI
        if (QWidget *widget = qobject_cast<QWidget*>(parent()))
            sh.setWidth(widget->width());
#endif
        return sh;
    }
};

/*!
  Returns a new launcher view instance with the given \a parent and \a flags. \a name
  is the name of the LauncherView subclass to be created.

  New launcher view subclasses can be registered
  via the UIFactory class and UIFACTORY_REGISTER_WIDGET().

  \code
    //customlauncherview.cpp
    #include "uifactory.h"
    #include "launcherview.h"

    class CustomLauncherView : public LauncherView {

    };

    UIFACTORY_REGISTER_WIDGET(CustomLauncherView);

    //code.cpp
    LauncherView *view = LauncherView::createLauncherView( "CustomLauncherView", parent, flags );
  \endcode

  Launcher view instances should be created via this factory method whenever the caller
  doesn't intend to actually include the declaration for \c CustomLauncherView.

  This function returns a null pointer if \a name doesn't exist.
  */
LauncherView* LauncherView::createLauncherView( const QByteArray &name, QWidget *parent, Qt::WFlags flags)
{
    QWidget *widget = UIFactory::createWidget( name, parent, flags );
    if ( !widget )
        return 0;

    if ( widget->inherits("LauncherView") ) {
        LauncherView *view = qobject_cast<LauncherView*>(widget);
        if (view)
            return view;
    }

    if ( widget )
        delete widget;
    return 0;
}

/*!
  \class LauncherView
    \inpublicgroup QtBaseModule
  \brief The LauncherView class provides the base for any type of laucnher view used by Qtopia.
  \ingroup QtopiaServer::GeneralUI

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa ApplicationLauncherView, DocumentLauncherView
*/

/*!
  Create a LauncherView instance with the given \a parent and \a flags.
*/
LauncherView::LauncherView( QWidget* parent, Qt::WFlags flags )
    : QWidget(parent, flags)
        , m_contentSet(0)
        , m_model(0)
        , m_mainLayout(0)
        , m_icons(0)
        , m_smoothList(0)
        , nColumns(1)
        , mNeedGridSize(false)
        , m_itemDelegate(0)
{
    init();
}

void LauncherView::init()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(2);

    m_contentSet = new QContentSet( QContentSet::Asynchronous, this);
    m_contentSet->setSortOrder(QStringList() << "name");
    m_model = new QContentSetModel(m_contentSet, this);
    m_itemDelegate = new LauncherViewDelegate(this);

    setViewMode(QListView::ListMode);
}

void LauncherView::initListView()
{
    if (!m_smoothList) {
        m_smoothList = new QSmoothList(this);
        QFont listFont(font());
        listFont.setBold(true);
        m_smoothList->setFont(listFont);
        m_smoothList->setItemDelegate(m_itemDelegate);
        m_mainLayout->addWidget(m_smoothList);
        setFocusProxy(m_smoothList);
        QSoftMenuBar::setLabel(m_smoothList, Qt::Key_Select, QSoftMenuBar::Select);
        QtopiaApplication::setStylusOperation( m_smoothList, QtopiaApplication::RightOnHold );

        connect( m_smoothList, SIGNAL(activated(QModelIndex)),
                SLOT(returnPressed(QModelIndex)) );
        connect( m_smoothList, SIGNAL(pressed(QModelIndex)),
                SLOT(itemPressed(QModelIndex)));
        connect( m_smoothList, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex,QModelIndex)) );
        m_smoothList->setModel(m_model);
    }
}

void LauncherView::initIconView()
{
    if (!m_icons) {
        m_icons = new LauncherViewListView(this);
        m_icons->setItemDelegate(m_itemDelegate);
        m_mainLayout->addWidget(m_icons);
        setFocusProxy(m_icons);

        QtopiaApplication::setStylusOperation( m_icons->viewport(), QtopiaApplication::RightOnHold );

        m_icons->setFrameStyle( QFrame::NoFrame );
        m_icons->setResizeMode( QListView::Fixed );
        m_icons->setSelectionMode( QAbstractItemView::SingleSelection );
        m_icons->setSelectionBehavior( QAbstractItemView::SelectItems );
        m_icons->setUniformItemSizes( true );
        m_icons->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_icons->setViewMode(QListView::IconMode);

        connect( m_icons, SIGNAL(clicked(QModelIndex)),
                SLOT(itemClicked(QModelIndex)));
        connect( m_icons, SIGNAL(activated(QModelIndex)),
                SLOT(returnPressed(QModelIndex)) );
        connect( m_icons, SIGNAL(pressed(QModelIndex)),
                SLOT(itemPressed(QModelIndex)));
        connect( m_icons, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex,QModelIndex)) );

        m_icons->setModel(m_model);
    }
}

/*!
  \fn LauncherView::~LauncherView()

  Destroys the LauncherView.
*/

/*!
  \fn QContentSetModel *LauncherView::model() const

  Returns the content model that the view is presenting.
*/

/*!
    Sets a new \a model for this view. Any existing model will be deleted.
*/
void LauncherView::setModel(QContentSetModel* model)
{
    QContentSetModel* old = m_model;
    m_model = model;

    if (m_icons)
        m_icons->setModel(model);
    if (m_smoothList)
        m_smoothList->setModel(model);

    delete old;
}

/*!
  \fn void LauncherView::returnPressed(const QModelIndex &item)

  \internal
*/

/*!
  \fn void LauncherView::itemClicked(const QModelIndex &item)
  \internal
*/

/*!
  \fn void LauncherView::itemPressed(const QModelIndex &item)
  \internal
*/

/*!
  \enum LauncherView::SortingStyle
  \internal
*/

/*!
  \fn void LauncherView::clicked(QContent content)

  This signal is emitted whenever the user clickes on an item. The activated
  item is passed as \a content object.
  */

/*!
  \fn void LauncherView::rightPressed(QContent content)

  This signal is emitted when the user clciks on \a content with the right mouse button.
  This signal may be used in conjunction with a context menu.
  */

/*!
  Sets the busy state of the currently selected item to \a on. This indicates to the user that
  the current selection is processed.
  */
void LauncherView::setBusy(bool on)
{
    setBusy(currentIndex(), on);
}

/*!
  \internal
  */
void LauncherView::setBusy(const QModelIndex &/*index*/, bool /*on*/)
{
/*
    // Enable this code to display a wait icon next to the busy item.
    if ( on )
        bpModel->setBusyItem(index);
    else
        bpModel->clearBusyItems();
*/
}

/*!
  \reimp
  */
void LauncherView::timerEvent ( QTimerEvent * event )
{
    QWidget::timerEvent( event );
}

/*!
    Sets the item delegate for the contained view and its model to \a delegate.
    The old delegate is deleted in the process.

    \sa QAbstractItemView::setItemDelegate()
*/
void LauncherView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    QAbstractItemDelegate *old = m_itemDelegate;
    m_itemDelegate = delegate;

    if (m_icons)
        m_icons->setItemDelegate(delegate);
    if (m_smoothList)
        m_smoothList->setItemDelegate(delegate);

    delete old;
}

/*!
  Sets the viewmode of the contained item view to \a m.

  \sa QListView::setViewMode()
*/
void LauncherView::setViewMode( QListView::ViewMode m )
{
    if(m==QListView::ListMode) {
        initListView();
        if (m_icons)
            m_icons->setVisible(false);
        m_smoothList->setVisible(true);
        setFocusProxy(m_smoothList);
    } else {
        initIconView();
        if (m_smoothList)
            m_smoothList->setVisible(false);
        m_icons->setVisible(true);
        setFocusProxy(m_icons);
    }
    calculateGridSize();
}

/*!
  Returns the used view mode.

  \sa QListView::viewMode()
*/
QListView::ViewMode LauncherView::viewMode() const
{
    if (m_icons && m_icons->isVisible())
        return QListView::IconMode;
    return QListView::ListMode;
}

/*!
  Sets the direction the items layout should \a flow.

  This property only affects the Icon view mode.

  \sa QListView::setFlow()
*/
void LauncherView::setFlow(QListView::Flow flow)
{
    if (m_icons) {
        if (flow == QListView::LeftToRight) {
            m_icons->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_icons->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else {
            m_icons->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            m_icons->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
        m_icons->setFlow(flow);
    }
}

/*!
  Returns the direction the items layout should flows.

  \sa QListView::flow()
*/
QListView::Flow LauncherView::flow() const
{
    if (m_icons)
        return m_icons->flow();

    return QListView::TopToBottom;
}

/*!
  Removes all items from the view
*/
void LauncherView::removeAllItems()
{
    m_contentSet->clear();
}

/*!
  Adds \a app to the view. \a resort is ignored at this stage.

  \sa removeItem()
  */
void LauncherView::addItem(QContent* app, bool resort)
{
    Q_UNUSED(resort);
    if(app == NULL)
        return;
    m_contentSet->add( *app );

}

/*!
  Removes \a app from the view.

  \sa addItem()
*/
void LauncherView::removeItem(const QContent &app) {
    m_contentSet->remove(app);
}

/*!
  This event handler can be reimplemented in a subclass to react to \c Enter
  or \c Return key press on the current \a item.
  The default implementation emits the clicked() signal.
  */
void LauncherView::handleReturnPressed(const QModelIndex &item) {
    emit clicked(m_model->content(item));
}

/*!
  This event handler can be reimplemented in a subclass to react when a mouse button
  is clicked with the specified \a item. \a item will be selected if \a setCurrentIndex is \c true.
*/
void LauncherView::handleItemClicked(const QModelIndex & item, bool setCurrentIndex)
{
    if(QApplication::mouseButtons () == Qt::LeftButton) {
        if (m_icons && setCurrentIndex)
            m_icons->setCurrentIndex( item );
        if (m_icons && Qtopia::mousePreferred())
            m_icons->selectionModel()->clearSelection();
        emit clicked(m_model->content(item));
    }
}

/*!
  This event handler can be reimplemented in a subclass to react when a mouse button
  is pressed with the specified \a item.
  */
void LauncherView::handleItemPressed(const QModelIndex & item)
{
    if(QApplication::mouseButtons () & Qt::RightButton)
    {
        if (m_icons)
            m_icons->setCurrentIndex( item );
        if (m_smoothList)
            m_smoothList->setCurrentIndex( item );
        emit rightPressed(m_model->content(item));
    }
}

/*!
  \reimp
  */
void LauncherView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent( e );
    calculateGridSize();
}

/*!
  \reimp
  */
void LauncherView::changeEvent(QEvent *e)
{
    if (m_icons && e->type() == QEvent::PaletteChange) {
        QPalette pal(palette());
        m_icons->setPalette(pal);
    }
    if (e->type() == QEvent::StyleChange)
        calculateGridSize();
    QWidget::changeEvent(e);
}

/*!
  \reimp
  */
void LauncherView::showEvent(QShowEvent *e)
{
    if(mNeedGridSize)
        calculateGridSize(true);

    QWidget::showEvent(e);
}

/*!
  \fn void LauncherView::currentChanged(const QModelIndex &current, const QModelIndex &previous)

  This slot is called when a new item becomes the current item.
  The previous current item is specified by the \a previous index,
  and the new item by the \a current index.
*/
void LauncherView::currentChanged(const QModelIndex &, const QModelIndex &)
{
}

/*!
  Sets the number of \a columns. This value defaults to 1.
  */
void LauncherView::setColumns(int columns)
{
    nColumns = columns;
    calculateGridSize();
}

/*!
  The view will be adjusted according to \a filter.
  */
void LauncherView::setFilter(const QContentFilter &filter)
{
    if( filter != mainFilter )
    {
        mainFilter = filter;

        m_contentSet->setCriteria( mainFilter & typeFilter & categoryFilter & auxiliaryFilter );
    }
}

/*!
  Returns the currently selected item.
  */
const QContent LauncherView::currentItem() const
{
    return m_model->content(currentIndex());
}

/*!
  Returns the index of the currently selected item.
*/
QModelIndex LauncherView::currentIndex() const
{
    if (viewMode() == QListView::IconMode)
        return m_icons->currentIndex();
    else
        return m_smoothList->currentIndex();
}

/*!
  \internal
  */
void LauncherView::calculateGridSize(bool force)
{
    if(!force && !isVisible()) {
        mNeedGridSize = true;
        return;
    }
    mNeedGridSize = false;

    QSize grSize;
    Q_ASSERT(m_model);

    int dw = width();
    int viewerWidth = dw - style()->pixelMetric(QStyle::PM_ScrollBarExtent) - 1;
    int iconHeight = 0;
    if ( viewMode() == QListView::IconMode ) {
        int grWidth = viewerWidth/nColumns;
        int grHeight = grWidth;
#ifdef QTOPIA_HOMEUI
        if (m_icons->flow() == QListView::TopToBottom) {
            // Qtopia Home hardcoded to 2 row grid.
            const int Rows = 2;
            grWidth = width()/nColumns;
            grHeight = (height() - style()->pixelMetric(QStyle::PM_ScrollBarExtent) - 1) / Rows;
        }
        iconHeight = grHeight;
#else
        iconHeight = grHeight - fontMetrics().height() * 2;
#endif

        grSize = QSize(grWidth, grHeight);
        QSize icoSize = QSize(iconHeight, iconHeight);
        m_icons->setIconSize(icoSize);
        m_icons->setGridSize(grSize);
    } else {
        m_smoothList->setIconSize(listIconSize(this));
    }
}

/*!
    Returns the size of icons the given \a widget should use if it is a launcher view in
    list mode. This can be used to make other list views look consistent with launcher views.
*/
QSize LauncherView::listIconSize(QWidget *widget)
{
    int viewHeight = widget->geometry().height();
    qreal scalingFactor = 1.65;
#ifndef QTOPIA_HOMEUI
    if (Qtopia::mousePreferred())
        scalingFactor = 1.8;
#endif

    int nbRow = int(qAbs(static_cast<int>(viewHeight / (widget->fontMetrics().height() * scalingFactor))));
    if(nbRow == 0)
        nbRow++;

    int iconHeight = qRound(viewHeight / nbRow);

    return QSize(iconHeight - 4, iconHeight - 4);
}

/*!
  \a filter is applied in addition to the main filter.

  \sa setFilter()
  */
void LauncherView::showType( const QContentFilter &filter )
{
    if( filter != typeFilter )
    {
        typeFilter = filter;

        m_contentSet->setCriteria( mainFilter & typeFilter & categoryFilter & auxiliaryFilter );
    }
}

/*!
  Only items that are of category \a filter will be shown.
  */
void LauncherView::showCategory( const QContentFilter &filter )
{
    if( filter != categoryFilter )
    {
        categoryFilter = filter;

        m_contentSet->setCriteria( mainFilter & typeFilter & categoryFilter & auxiliaryFilter );
    }
}

/*!
  \a filter acts as an auxiliary filter besides the main filter.
  */
void LauncherView::setAuxiliaryFilter( const QContentFilter &filter )
{
    if( filter !=  auxiliaryFilter )
    {
        auxiliaryFilter = filter;

        m_contentSet->setCriteria( mainFilter & typeFilter & categoryFilter & auxiliaryFilter );
    }
}

/*!
  This function can be reimplemented by subclasses to adjust the way how the view is reset.
  The default implementation selects the first item in the view.
  */
void LauncherView::resetSelection()
{
    if (m_icons && m_model && m_icons->model()->rowCount() && !Qtopia::mousePreferred()) {
        m_icons->setCurrentIndex(m_icons->model()->index(0,0));
    }
}

/*!
  Clears the current selection.
*/
void LauncherView::clearSelection()
{
    if (m_icons)
        m_icons->selectionModel()->clearSelection();
}
