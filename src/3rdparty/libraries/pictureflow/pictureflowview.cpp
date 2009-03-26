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

#include "pictureflowview.h"
#include <QDebug>
#include <QItemSelectionModel>
#include <QAbstractItemDelegate>
#include <QPainter>

class PictureFlowViewPrivate : public QObject
{
    Q_OBJECT
public:
    QAbstractItemModel *model;
    QItemSelectionModel *selectionModel;
    PictureFlowView *q;
    int modelRole;
    bool skipUpdates;
    bool lockCurrentUpdates;
    int savedCenterIndex;
    QAbstractItemDelegate *delegate;

public slots:
    void partialRefresh();
    void fullRefresh();
    void updateImages(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void processInsertedImages(const QModelIndex &index, int from, int to);
    void processRemovedImages(const QModelIndex &index, int from, int to);
    void processClicked(int);
    void processCenterChanged(int);
    void setCurrentIndex(const QModelIndex &current, const QModelIndex &previous );
};

PictureFlowView::PictureFlowView(QWidget* parent)
    : PictureFlow(parent)
{
    d = new PictureFlowViewPrivate;
    d->q = this;
    d->model = NULL;
    d->selectionModel = NULL;
    d->modelRole = Qt::DecorationRole;
    d->skipUpdates = false;
    d->lockCurrentUpdates = false;
    d->savedCenterIndex = -1;
    d->delegate = NULL;
    connect(this, SIGNAL(centerIndexChanged(int)), d, SLOT(processCenterChanged(int)));
    connect(this, SIGNAL(clicked(int)), d, SLOT(processClicked(int)));
}

PictureFlowView::~PictureFlowView()
{
    delete d;
}

void PictureFlowView::setModel(QAbstractItemModel * model)
{
    if(d->model != NULL)
        disconnect(d->model, 0, d, 0);
    d->model = model;
    d->fullRefresh();
    if(d->model != NULL) {
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), d, SLOT(updateImages(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(layoutChanged()), d, SLOT(fullRefresh()));
        connect(d->model, SIGNAL(modelReset()), d, SLOT(fullRefresh()));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)), d, SLOT(processInsertedImages(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)), d, SLOT(processInsertedImages(QModelIndex,int,int)));
        setSelectionModel( new QItemSelectionModel(model,model) );
    }
}

QAbstractItemModel *PictureFlowView::model() const
{
    return d->model;
}


QModelIndex PictureFlowView::currentModelIndex()
{
    if(d->model)
        return d->model->index(centerIndex(), 0);
    else
        return QModelIndex();
}

void PictureFlowView::setModelRole(int role)
{
    d->modelRole = role;
    d->fullRefresh();
}

int PictureFlowView::modelRole()
{
    return d->modelRole;
}

void PictureFlowView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (d->selectionModel)
        disconnect( d->selectionModel, 0, this, 0 );

    d->selectionModel = selectionModel;
    connect( d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             d, SLOT(setCurrentIndex(QModelIndex,QModelIndex)) );

    if ( selectionModel->currentIndex().isValid() )
        setCenterIndex( selectionModel->currentIndex().row() );
}

QItemSelectionModel* PictureFlowView::selectionModel()
{
    return d->selectionModel;
}

void PictureFlowViewPrivate::updateImages( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
    if ( skipUpdates )
        return;

    if (!model || model->rowCount() <= 0) {
        q->clear();
        return;
    }

    int start = qMax(topLeft.row(), 0);
    int end = qMin(bottomRight.row(), model->rowCount()-1 );

    //qDebug() << "PictureFlowViewPrivate::updateImages" << start << end;

    for (int i = start; i <= end; ++i)
        q->setSlide(i, QImage());

    partialRefresh();
}

void PictureFlowViewPrivate::setCurrentIndex(const QModelIndex &current, const QModelIndex &previous )
{
    Q_UNUSED( previous );
    if ( current.isValid() && !lockCurrentUpdates ) {
        savedCenterIndex = current.row();
        q->setCenterIndex( savedCenterIndex );
    }
}

void PictureFlowViewPrivate::processInsertedImages( const QModelIndex &index, int from, int to )
{
    if ( skipUpdates )
        return;

    Q_UNUSED(index);

    if ( to == model->rowCount()-1 ) {
        while(model->rowCount() > q->slideCount())
            q->addSlide(QImage());
        partialRefresh();
    } else
        fullRefresh();

    if (to - from + 1 == model->rowCount())
        emit q->currentChanged(model->index(0, 0));
}

void PictureFlowViewPrivate::processRemovedImages( const QModelIndex &index, int from, int to )
{
    if ( skipUpdates )
        return;

    Q_UNUSED(index);

    for ( int i=from; i<=to; i++ ) {
        q->removeSlide(from);
    }

    if(q->centerIndex() >= q->slideCount())
        q->setCenterIndex(q->slideCount()-1);
    partialRefresh();
}

void PictureFlowViewPrivate::partialRefresh()
{
    if ( skipUpdates )
        return;

    //qDebug() << "PictureFlowViewPrivate::partialRefresh";
    if (model==NULL) {
        q->clear();
        return;
    }

    // load up 10 either side and drop/clear out the old index pixmaps
    for(int i=0;i<q->slideCount();i++)
    {
        if(i<q->centerIndex()-10||i>q->centerIndex()+10)  {
            if(!q->slide(i).isNull())
                q->setSlide(i, QImage());
        } else if (q->slide(i).isNull()) {
            QImage slide;
            if(delegate != NULL) {
                slide=QImage(q->slideSize(), QImage::Format_RGB32);
                QPainter painter(&slide);
                delegate->paint(&painter, QStyleOptionViewItem(), model->index(i, 0));
            } else {
                QVariant v=model->data(model->index(i, 0), modelRole);
                if(v.canConvert<QImage>())
                    slide=v.value<QImage>();
            }
            if(q->slide(i).cacheKey() != slide.cacheKey())
                q->setSlide(i, slide);
        }
    }
}

void PictureFlowViewPrivate::fullRefresh()
{
    if ( skipUpdates )
        return;

    //qDebug() << "PictureFlowViewPrivate::fullRefresh";
    q->clear();
    if(model != NULL) {
        savedCenterIndex = qBound( 0, savedCenterIndex, model->rowCount()-1 );

        for(int i=0;i<model->rowCount();i++) {
            if(i<savedCenterIndex-10||i>savedCenterIndex+10)
                q->addSlide(QImage());
            else {
                QImage slide;
                if(delegate != NULL) {
                    slide=QImage(q->slideSize(), QImage::Format_RGB32);
                    QPainter painter(&slide);
                    delegate->paint(&painter, QStyleOptionViewItem(), model->index(i, 0));
                } else {
                    QVariant v=model->data(model->index(i, 0), modelRole);
                    if(v.canConvert<QImage>())
                        slide=v.value<QImage>();
                }
                q->addSlide(slide);
            }
        }

        q->setCenterIndex( savedCenterIndex );
        q->currentChanged( model->index(savedCenterIndex,0) );
    }
    savedCenterIndex = -1;
}

void PictureFlowViewPrivate::processClicked( int row )
{
    if( model!=NULL ) {
        QModelIndex index = model->index(row,0);

        if ( index.isValid() )
            emit q->activated( index );
    }
}

void PictureFlowViewPrivate::processCenterChanged( int row )
{
    if ( skipUpdates )
        return;

    if( model!=NULL ) {
        QModelIndex index = model->index(row,0);

        if ( index.isValid() ) {
            lockCurrentUpdates = true;
            selectionModel->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
            emit q->currentChanged( index );
            lockCurrentUpdates = false;
        }
    }
    partialRefresh();
}


void PictureFlowView::hideEvent(QHideEvent *e)
{
    d->skipUpdates = true;
    d->savedCenterIndex = centerIndex();
    clear();

    PictureFlow::hideEvent(e);
}

void PictureFlowView::showEvent(QShowEvent *e)
{
    d->skipUpdates = false;
    d->fullRefresh();

    PictureFlow::showEvent(e);
}

#include "pictureflowview.moc"

