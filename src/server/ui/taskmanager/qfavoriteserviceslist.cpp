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

#include "qfavoriteserviceslist.h"
#include <QSpeedDial>
#include <QMouseEvent>
#include <QtopiaServiceHistoryModel>
#include <QFavoriteServicesModel>
#include <QWaitWidget>
#include <qtopiaapplication.h>
#include <qexpressionevaluator.h>
#include <qsoftmenubar.h>
#include <qtopiaserviceselector.h>
#include <QDialog>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QDebug>
#include <QObject>
#include <QPainter>
#include <QtopiaStyle>
#include <QtopiaItemDelegate>
#include <QSortFilterProxyModel>
#include <QListView>
#include <QTimer>

class QFavoriteServicesElidedFilterModel : public QFavoriteServicesModel
{
    Q_OBJECT
    //Elides the text so there is room for the speeddial input
    public:
        QFavoriteServicesElidedFilterModel(QObject *parent = 0) : QFavoriteServicesModel(parent),
        m_width(0), m_fm(0) {};
        void filter(int width, QFontMetrics *fm = 0);//If either = 0 it doesn't filter
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    private:
        int m_width;
        QFontMetrics* m_fm;
};

class QFavoriteServicesDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
    public:
        QFavoriteServicesDelegate(QSmoothList* parent, QFavoriteServicesElidedFilterModel*filter=0);
        ~QFavoriteServicesDelegate();

        virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    private:
        QSmoothList* parentList;
        QFavoriteServicesElidedFilterModel *filterModel;
        int iconSize;
        int textHeight;
        int itemHeight;
};

class QFavoriteServicesMoveModel: public QSortFilterProxyModel
{
    Q_OBJECT
    //Provides the 'moving' functionality of the move dialog
    public:
        QFavoriteServicesMoveModel(int row, QSmoothList *view, QObject *parent = 0);
        QModelIndex mapFromSource ( const QModelIndex & sourceIndex) const;
        QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;
    public slots:
        void reload(const QModelIndex &current, const QModelIndex &previous);

    private:
        int m_row;
        mutable int m_currentRow;
        QSmoothList *m_view;
};

class QFavoriteServicesMoveDialog : public QDialog
{
    Q_OBJECT
    public:
        QFavoriteServicesMoveDialog(int startRow, QWidget *parent);
        int moveChoice() const {return m_moveChoice;}
    private slots:
        void store(const QModelIndex &choice);
    private:
        int m_moveChoice;
        QSmoothList *list;
        QFavoriteServicesElidedFilterModel *model;
};

class QFavoriteServicesListPrivate
{
    public:
        QFavoriteServicesListPrivate(QFavoriteServicesList *parentList)
                : parent(parentList)
                , model(new QFavoriteServicesElidedFilterModel(parentList))
                , delegate(new QFavoriteServicesDelegate(parentList,model))
                {
                    serviceSelector = 0;
                }

        QAction *addAction;
        QAction *delAction;
        QAction *assignAction;
        QAction *removeAction;
        QAction *moveAction;
        QMenu *itemMenu;
        QtopiaServiceSelector *serviceSelector;
        QFavoriteServicesList *parent;
        QFavoriteServicesElidedFilterModel *model;
        QFavoriteServicesDelegate *delegate;
};

//QFavoriteServicesList Implementation
/*!
    \class QFavoriteServicesList
    \inpublicgroup QtBaseModule
    \brief The QFavoriteServicesList widget presents a list of services.

    An implementation of a UI for the Favorite services list.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    \fn void QFavoriteServicesList::selected( const QModelIndex & index)

    This signal is emitted when a model index is selected. \a index is the index that was selected.
*/

/*!
    Constructs a QFavoriteServicesList object with the given \a parent.
 */
QFavoriteServicesList::QFavoriteServicesList(QWidget* parent) : QSmoothList(parent)
{
    d = new QFavoriteServicesListPrivate(this);
    setModel(d->model);
    setItemDelegate(d->delegate);
    setWindowTitle( tr( "Favorites" ) );
    connect(this,SIGNAL(activated(QModelIndex)),
            this,SLOT(select(QModelIndex)));
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    if(Qtopia::mousePreferred ()) {
        QtopiaApplication::setStylusOperation (this, QtopiaApplication::RightOnHold);
        d->itemMenu = new QMenu(this);
    } else {
        d->itemMenu = contextMenu;
    }
    d->addAction = new QAction( QIcon( ":icon/edit" ),  tr("Add", "Add action"), this);
    connect( d->addAction, SIGNAL(triggered()), this, SLOT(addService()) );
    contextMenu->addAction(d->addAction);
    d->delAction = new QAction( QIcon( ":icon/trash" ),  tr("Delete"), this);
    connect( d->delAction, SIGNAL(triggered()), this, SLOT(removeCurrentService()) );
    d->itemMenu->addAction(d->delAction);
    d->assignAction = new QAction( QIcon( ":icon/edit" ),  tr("Assign Speed Dial", "Assign Speed Dial Number"), this);
    connect( d->assignAction, SIGNAL(triggered()), this, SLOT(editCurrentSpeedDial()) );
    d->itemMenu->addAction(d->assignAction);
    d->removeAction = new QAction( QIcon( ":icon/edit" ),  tr("Remove Speed Dial", "Remove Speed Dial Number"), this);
    connect( d->removeAction, SIGNAL(triggered()), this, SLOT(removeCurrentSpeedDial()) );
    d->itemMenu->addAction(d->removeAction);
    d->moveAction = new QAction( QIcon( ":icon/up" ),  tr("Move","Alter the action's position in the list"), this);
    connect( d->moveAction, SIGNAL(triggered()), this, SLOT(moveCurrentService()) );
    d->itemMenu->addAction(d->moveAction);
    connect(this, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(selectionChanged(QModelIndex,QModelIndex)));
    connect(d->model, SIGNAL(modelReset()),
            this, SLOT(selectionChange()));
    d->addAction->setVisible(true);
    d->removeAction->setVisible(false);
    d->moveAction->setVisible(false);
    d->delAction->setVisible(false);
    d->assignAction->setVisible(false);
    setCurrentIndex(d->model->index(0,0));
    selectionChange();
}

/*!
    Destroys the QFavoriteServicesList
 */
QFavoriteServicesList::~QFavoriteServicesList()
{
    delete d;
}

/*!
  \internal
*/
void QFavoriteServicesList::selectionChange()
{
    QTimer::singleShot(20, this, SLOT(selectionChanged()));//This must be served the signal after the smoothlist
}

void QFavoriteServicesList::selectionChanged()
{
    selectionChanged(currentIndex(),QModelIndex());
}

/*!
  \internal
  Updates available menu items depending on the current index
*/
void QFavoriteServicesList::selectionChanged(const QModelIndex &current,
        const QModelIndex &/*previous*/)
{
    if(!current.isValid()){
        d->delAction->setVisible(false);
        d->assignAction->setVisible(false);
    }else{
        d->delAction->setVisible(true);
        d->assignAction->setVisible(true);
    }
    if(current.isValid() && d->model->rowCount() > 1){
        d->moveAction->setVisible(true);
    }else{
        d->moveAction->setVisible(false);
    }
    if(current.isValid() && !d->model->speedDialInput(current).isEmpty()){
        d->removeAction->setVisible(true);
    }else{
        d->removeAction->setVisible(false);
    }
}
/*!
    Returns the number of rows in the list
*/
int QFavoriteServicesList::rowCount()
{
    return d->model->rowCount();
}

/*!
    Sets the current row to \a row, as long as 0 <= \a row < rowCount()
*/
void QFavoriteServicesList ::setCurrentRow(int row)
{
    setCurrentIndex(d->model->index(row,0));
}

/*!
    \internal

    Catches selection events and emits an event
    Also attempts the selected service request.
 */
void QFavoriteServicesList::select(const QModelIndex& index)
{

    QtopiaServiceDescription desc = d->model->description(index);
    if(!desc.isNull()){
        if(desc.request().send())
            QtopiaServiceHistoryModel::insert(desc);
        emit selected(index);
    }

}

/*!
    \internal
    Used to create menus for press and hold
*/
void QFavoriteServicesList::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton){
        setCurrentIndex(indexAt(event->pos()));
        QAction *selectedAction = d->itemMenu->exec(event->globalPos());
        if(selectedAction){
            //selectedAction->trigger();
        }
        event->accept();
    } else {
        QSmoothList::mousePressEvent(event);
    }
}

/*!
\internal
 */
void QFavoriteServicesList::addService()
{
    if(!d->serviceSelector){
        QWaitWidget *waitWidget = new QWaitWidget(this);
        waitWidget->show();
        QtopiaApplication::processEvents(QEventLoop::AllEvents,1000);

        d->serviceSelector = new QtopiaServiceSelector(this);
        d->serviceSelector->addApplications();
        QtopiaApplication::setMenuLike(d->serviceSelector,true);

        delete waitWidget;
    }
    QtopiaServiceDescription desc;
    if(d->serviceSelector->edit(tr("Favorites"),desc)){
        if(!d->model->indexOf(desc).isValid()){
            d->model->insert(QModelIndex(),desc);
        }
        setCurrentIndex(d->model->indexOf(desc));
    }
}

/*!
\internal
 */
void QFavoriteServicesList::removeCurrentService()
{
    int oldRow = currentIndex().row();
    d->model->remove(currentIndex());
    setCurrentIndex(d->model->index(qMax(0,oldRow-1)));
}
/*!
\internal
 */
void QFavoriteServicesList::moveCurrentService(){
    int newRow = currentIndex().row();
    QFavoriteServicesMoveDialog dlg(currentIndex().row(),this);
    dlg.setObjectName("movefavorites");
    if ( QtopiaApplication::execDialog(&dlg) ){
        newRow = dlg.moveChoice();
        d->model->move(currentIndex(),d->model->index(newRow));
        setCurrentIndex(d->model->index(newRow));
    }
}

/*!
\internal
*/
void QFavoriteServicesList::editCurrentSpeedDial()
{
    QtopiaServiceDescription desc = d->model->description(currentIndex());
    QString ret = QSpeedDial::addWithDialog(desc.label(),desc.iconName(),desc.request(),this);
    update(currentIndex());
}

/*!
\internal
*/
void QFavoriteServicesList::removeCurrentSpeedDial()
{
    QSpeedDial::set(d->model->speedDialInput(currentIndex()), QtopiaServiceDescription());
    update(currentIndex());
}

//QFavoriteServicesDelegate implementation

QFavoriteServicesDelegate::QFavoriteServicesDelegate(QSmoothList* parent,
        QFavoriteServicesElidedFilterModel *filter):QtopiaItemDelegate(parent)
{
    parentList = parent;
    filterModel = filter;
  /*  iconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
    QFontMetrics fm(parent->font());
    textHeight = fm.height();

    if(iconSize > textHeight)
        itemHeight = iconSize;
    else
        itemHeight = textHeight; */
}

QFavoriteServicesDelegate::~QFavoriteServicesDelegate()
{
}

void QFavoriteServicesDelegate::paint(QPainter * painter,
                                      const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QFavoriteServicesElidedFilterModel *model = (QFavoriteServicesElidedFilterModel*)index.model();

    if(model){
        QString input = model->data(model->index(index.row(),0,QModelIndex()),
                                    Qt::UserRole).toString();
        if(input.isEmpty()){
            QtopiaItemDelegate::paint(painter,option,index);
            return;
        }

        QFontMetrics fm(option.font);
        int width = option.rect.width();
        int smallIconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
        smallIconSize = smallIconSize>>1;
        QPixmap speedDialIcon = QIcon(":icon/speeddial/speeddial").pixmap(QSize(smallIconSize,smallIconSize));
        QPixmap pixmap = model->data(model->index(index.row(),0,QModelIndex()),
                                     Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize);

        width -= (speedDialIcon.width() + fm.width(QLatin1String(" 00")) + 1);//Save space for SpeedDial #
        if(!pixmap.isNull())
            width -=  pixmap.width() + fm.width(QLatin1String(" "));

        if(filterModel)
            filterModel->filter(width,&fm);
        QtopiaItemDelegate::paint(painter,option,index);
        if(filterModel)
            filterModel->filter(0,&fm);

        bool rtl = qApp->layoutDirection() == Qt::RightToLeft;
        int x=width+1;
        if(!pixmap.isNull())
            x+= pixmap.width() + fm.width(QLatin1String(" "));
        int y = option.rect.y();
        width = (speedDialIcon.width() + fm.width(QLatin1String(" 00")));
        int height = option.rect.height()-1;
        //Right Aligned (if rtl, left aligned)
        if ( !rtl ) {
            painter->drawPixmap(x, y+(speedDialIcon.height()/2), speedDialIcon);
            x+=speedDialIcon.width()+fm.width(QLatin1String(" "));
            width = fm.width(QLatin1String("00"));
        } else {
            painter->drawPixmap(fm.width(QLatin1String(" 00")), y+(speedDialIcon.height()/2),
                                speedDialIcon);
            width = fm.width(QLatin1String("00"));
        }
        QTextOption to;
        to.setAlignment( QStyle::visualAlignment(qApp->layoutDirection(),
                         Qt::AlignLeft) | Qt::AlignVCenter);
        painter->setPen(Qt::white);
        painter->drawText(QRect(x-1, y+1, width, height), input, to);
    }
}

QSize QFavoriteServicesDelegate::sizeHint(const QStyleOptionViewItem &/*option*/,
        const QModelIndex &/*index*/) const
{
    return QSize(parentList->width(), parentList->iconSize().height()+4);
}

//QFavoriteServicesMoveDialog Implementation
QFavoriteServicesMoveDialog::QFavoriteServicesMoveDialog(int startRow, QWidget *parent):QDialog(parent)
{
    m_moveChoice = startRow;
    setWindowModality(Qt::WindowModal);
    setWindowState(windowState() | Qt::WindowMaximized);
    QtopiaApplication::setMenuLike(this,true);
    QSoftMenuBar::menuFor(this);
    list = new QSmoothList(this);
    model = new QFavoriteServicesElidedFilterModel(this);
    QFavoriteServicesMoveModel *proxyModel = new QFavoriteServicesMoveModel(startRow,list,this);
    proxyModel->setSourceModel(model);
    list->setModel(proxyModel);
    list->setItemDelegate(new QFavoriteServicesDelegate(list,model));
    list->setCurrentIndex(model->index(startRow));

    connect(list, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            proxyModel, SLOT(reload(QModelIndex,QModelIndex)));

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);
    vb->addWidget(list);
    connect(list, SIGNAL(activated(QModelIndex)),
            this, SLOT(store(QModelIndex)));
    setWindowTitle(tr("Move Favorites"));
}

void QFavoriteServicesMoveDialog::store(const QModelIndex &choice)
{
    m_moveChoice = choice.row();
    accept();
}
//QFavoriteServicesElidedFilterModel Implementation
void QFavoriteServicesElidedFilterModel::filter(int width, QFontMetrics *fm)//If either = 0 it doesn't filter
{
    m_width = width;
    m_fm = fm;
}

QVariant QFavoriteServicesElidedFilterModel::data(const QModelIndex & index, int role )const
{
    QVariant ret = QFavoriteServicesModel::data(index,role);
    if(m_width && m_fm && role == Qt::DisplayRole){
        QString elided = ret.toString();
        elided = m_fm->elidedText(elided,Qt::ElideRight,m_width);
        return QVariant(elided);
    }
    return ret;
}

//QFavoriteServicesMoveModel Implementation
QFavoriteServicesMoveModel::QFavoriteServicesMoveModel(int row, QSmoothList *list, QObject *parent)
    :QSortFilterProxyModel(parent)
{
    m_row = row;
    m_view = list;
    m_currentRow = row;
}

QModelIndex QFavoriteServicesMoveModel::mapFromSource( const QModelIndex & sourceIndex ) const
{
    if(m_currentRow == -1)
        return QModelIndex();

    int offset = 0;
    if(sourceIndex.row()==m_currentRow)
        offset=m_row-sourceIndex.row();
    else if(sourceIndex.row()>=m_row&&sourceIndex.row()<m_currentRow)
        offset = 1;
    else if(sourceIndex.row()<=m_row&&sourceIndex.row()>m_currentRow)
        offset =-1;
    return index(sourceIndex.row()+offset,0,QModelIndex());
}

QModelIndex QFavoriteServicesMoveModel::mapToSource( const QModelIndex & proxyIndex ) const
{
    if(m_currentRow == -1)
        return QModelIndex();

    int offset = 0;
    if(proxyIndex.row()==m_currentRow)
        offset=m_row-proxyIndex.row();
    else if(proxyIndex.row()>=m_row&&proxyIndex.row()<m_currentRow)
        offset = 1;
    else if(proxyIndex.row()<=m_row&&proxyIndex.row()>m_currentRow)
        offset =-1;
    return index(proxyIndex.row()+offset,0,QModelIndex());
}

void QFavoriteServicesMoveModel::reload(const QModelIndex &current, const QModelIndex &previous)
{
    QModelIndex first,second;
    if(current.row() > previous.row()){
        first = previous;
        second = current;
    } else {
        first = current;
        second = previous;
    }
    m_currentRow = current.row();
    invalidate();
    emit dataChanged(first,second);
}


#include "qfavoriteserviceslist.moc"
