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

#include "singleview_p.h"

#include <qthumbnail.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qpainter.h>
#include <qevent.h>
#include <qcontent.h>
#include <QImageReader>

SingleView::SingleView( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f ), model_( 0 ), selection_( 0 )
{
    right_pressed = false;

    // Enable stylus press events
    QtopiaApplication::setStylusOperation( this, QtopiaApplication::RightOnHold );

    // Accept focus
    setFocusPolicy( Qt::StrongFocus );

    // Respond to file changes
    connect( qApp,
        SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
        this,
        SLOT(contentChanged(QContentIdList,QContent::ChangeType)));
}

void SingleView::setModel( QAbstractListModel* model )
{
    model_ = model;

    setSelectionModel( new QItemSelectionModel( model_ ) );
}

QAbstractListModel* SingleView::model() const
{
    return model_;
}

void SingleView::setSelectionModel( QItemSelectionModel* selection )
{
    // Disconnect current selection model
    if( selection_ ) {
        disconnect( selection_, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)) );
    }
    selection_ = selection;
    // Connect new selection model
    if( selection_ ) {
        connect( selection_, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)) );
    }
}

QItemSelectionModel* SingleView::selectionModel() const
{
    return selection_;
}

void SingleView::currentChanged( const QModelIndex& index, const QModelIndex& /*previous*/ )
{
    // Retrieve currently selected
    current_file = qvariant_cast<QString>( model_->data( index, Qt::UserRole ) );

    // Clear buffer and update widget
    buffer = QPixmap();
    update();
}

void SingleView::paintEvent( QPaintEvent* )
{
    // If image not loaded, load into buffer
    if( buffer.isNull() ) {
        // Load scaled image to fit widget keeping original wight height ratio
        buffer = loadThumbnail( current_file, size() );
    }

    QPainter painter( this );

    // Cacluculate offset to center image
    QPoint offset( ( width() - buffer.width() ) / 2, ( height() - buffer.height() ) / 2 );

    // Draw image
    painter.drawPixmap( offset, buffer );
}

void SingleView::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() ) {
    // If select key, emit select signal
    case Qt::Key_Select:
    case Qt::Key_Enter:
        emit selected();
        break;
    // Move current selection down one row
    case Qt::Key_Right:
    case Qt::Key_Up:
        moveForward();
        break;
    // Move current selection up one row
    case Qt::Key_Left:
    case Qt::Key_Down:
        moveBack();
        break;
    default:
        e->ignore();
    }
}

void SingleView::mousePressEvent( QMouseEvent* e )
{
    Q_UNUSED(e);
    emit selected();
}

void SingleView::moveForward()
{
    // Move current selection down one row
    QModelIndex index = selection_->currentIndex();
    index = model_->index( index.row() + 1 );
    if( !index.isValid() ) {
        index = model_->index( 0 );
    }
    selection_->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
}

void SingleView::moveBack()
{
    // Move current selection up one row
    QModelIndex index = selection_->currentIndex();
    index = model_->index( index.row() - 1 );
    if( !index.isValid() ) {
        index = model_->index( model_->rowCount() - 1 );
    }
    selection_->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
}

QPixmap SingleView::loadThumbnail( const QString &filename, const QSize &size )
{
    QImageReader reader( filename );

    QImage image;

    bool scaled = false;

    if( reader.supportsOption( QImageIOHandler::Size ) && reader.supportsOption( QImageIOHandler::ScaledSize ) )
    {
        QSize maxSize = reader.size();

        maxSize.scale( size.boundedTo( reader.size() ), Qt::KeepAspectRatio );

        reader.setQuality( 49 ); // Otherwise Qt smooth scales
        reader.setScaledSize( maxSize );

        scaled = true;
    }

    if( reader.read( &image ) )
    {
        if( !scaled )
        {
            QSize maxSize = size.boundedTo( image.size() );

            image = image.scaled( maxSize, Qt::KeepAspectRatio, Qt::FastTransformation );
        }

        return QPixmap::fromImage( image );
    }
    else
        return QPixmap();
}

void SingleView::contentChanged(const QContentIdList & /*idList*/, const QContent::ChangeType /*changeType*/)
{
    QModelIndex idx = selectionModel()->currentIndex();
    if ( idx.isValid() ) {
        currentChanged(idx,idx);
    }
}
