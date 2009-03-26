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
#include "slideshowview.h"
#include "durationslider.h"
#include "imageview.h"
#include "photogallery.h"
#include "titlewindow.h"
#include "imageloader.h"

#include <QToolButton>
#include <QLabel>
#include <QStyle>
#include <QBoxLayout>
#include <QContent>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QDebug>
#include <QThread>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QtopiaApplication>

#include <gfxpainter.h>

#include <private/homewidgets_p.h>

SlideShowView::SlideShowView(QWidget *parent)
    : QWidget(parent)
    , m_model(0)
    , m_selectionModel(0)
    , m_enableModelUpdates(true)
    , m_advanceTimerId(-1)
{
    m_imageLoader = new ImageLoader(this);
    connect( m_imageLoader, SIGNAL(loaded(const QImage&,bool)), this,  SLOT(setImage(const QImage&)) );

    m_timeoutSlider = new DurationSlider;
    m_timeoutSlider->setRange(5, 60);
    m_timeoutSlider->setValue(20);
    m_timeoutSlider->setTracking(false);

    QLinearGradient gradient(0.0, 0.0, 0.0, 1.0);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.0, PhotoGallery::blue);
    gradient.setColorAt(1.0, PhotoGallery::blue.lighter(300));

    QPalette palette = m_timeoutSlider->palette();
    palette.setColor(QPalette::WindowText, PhotoGallery::blue);
    palette.setBrush(QPalette::Highlight, QBrush(gradient));
    palette.setColor(QPalette::HighlightedText, Qt::white);

    m_timeoutSlider->setPalette(palette);

    QBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_timeoutSlider);

    setLayout(layout);

    m_pauseButton = new QToolButton;
    m_pauseButton->setIcon(QIcon());
    m_pauseButton->setText( tr("Pause") );
    connect(m_pauseButton, SIGNAL(clicked()), this, SLOT(pause()));

    m_title = new QLabel;
    m_title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_title->setIndent(style()->pixelMetric(QStyle::PM_ButtonMargin));

    QFont font = m_title->font();
    font.setBold(true);
    m_title->setFont(font);

    HomeActionButton *backButton = new HomeActionButton(QtopiaHome::Red);
    backButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    backButton->setText(tr("Exit Slideshow"));
    connect(backButton, SIGNAL(clicked()), this, SIGNAL(back()));

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->setSpacing(0);
    controlsLayout->setMargin(0);
    controlsLayout->addWidget(m_pauseButton);
    controlsLayout->addWidget(m_title);
    controlsLayout->addWidget(backButton);

    m_controls = new TitleWindow(this);
    m_controls->setLayout(controlsLayout);
    connect(m_pauseButton, SIGNAL(clicked()), m_controls, SLOT(resetHideTimer()));

    palette = m_controls->palette();
    palette.setColor(QPalette::Window, QColor(0,0,0,128));

    m_controls->setPalette(palette);
}

SlideShowView::~SlideShowView()
{
}


void SlideShowView::setModel( QAbstractItemModel *model, QItemSelectionModel *selectionModel )
{
    m_model = model;

    if (m_selectionModel)
        disconnect( m_selectionModel, 0, this, 0 );

    m_selectionModel = selectionModel;

    connect( m_selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(setCurrentIndex(QModelIndex,QModelIndex)) );

    if ( selectionModel->currentIndex().isValid() )
        setCurrentIndex( selectionModel->currentIndex(), QModelIndex() );
}

void SlideShowView::start()
{
    if (m_advanceTimerId == -1) {
        m_pauseButton->setText(tr("Pause"));

        m_controls->fadeOut();

        m_advanceTimerId = startTimer(1000);

        QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
    }
}

void SlideShowView::stop()
{
    if (m_advanceTimerId != -1) {
        m_pauseButton->setText(tr("Continue"));

        killTimer(m_advanceTimerId);

        m_advanceTimerId = -1;

        QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
    }
}

int SlideShowView::timeout() const
{
    return m_timeoutSlider->value();
}

void SlideShowView::setTimeout(int timeout)
{
    m_timeoutSlider->setValue(timeout);
}

void SlideShowView::setImage(const QContent &imageContent)
{
    m_title->setText(imageContent.name());
    m_rotation = PhotoGallery::rotation(imageContent);
    m_imageLoader->loadImage( imageContent, size() );

    if ( imageContent.isNull() )
        m_image = QImage();

    update();
}

void SlideShowView::setImage(const QImage& image)
{
    if ( !isVisible() )
        return;

    m_image = image;
    if (m_rotation != 0)
        m_image = m_image.transformed(QTransform().rotate(m_rotation));

    m_advanceTime.start();

    update();
}

void SlideShowView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton )
        m_pressedPos = event->pos();
    else
        QWidget::mousePressEvent( event );
}

void SlideShowView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (!m_pressedPos.isNull()
            && (event->pos() - m_pressedPos).manhattanLength() > PhotoGallery::clickThreshold()) {
            m_pressedPos = QPoint();
        }
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void SlideShowView::mouseReleaseEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton) {
        if (!m_pressedPos.isNull()) {
            switch (m_controls->state()) {
            case TitleWindow::Visible:
            case TitleWindow::FadingIn:
                m_controls->fadeOut();
                break;
            case TitleWindow::Hidden:
            case TitleWindow::FadingOut:
                m_controls->fadeIn();
                break;
            }
        }
    } else {
        QWidget::mouseReleaseEvent(event);
    }

    m_pressedPos = QPoint();
}

void SlideShowView::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_advanceTimerId) {
        if (m_advanceTime.isValid() && m_advanceTime.elapsed() >= m_timeoutSlider->value() * 1000)  {
            m_advanceTime = QTime();

            nextImage();
        }
    } else {
        QWidget::timerEvent(event);
    }
}

void SlideShowView::pause()
{
    if (m_advanceTimerId == -1) {
        start();
    } else {
        stop();
    }
}

void SlideShowView::paintEvent(QPaintEvent *event)
{
    if ( !m_image.isNull() ) {
        QSize imageSize = m_image.size();
        imageSize.scale( geometry().size(), Qt::KeepAspectRatio );
        QRect imageRect( QPoint(0,0), imageSize );
        imageRect.moveCenter( geometry().center() );

        GfxPainter p(this,event);
        p.drawImage( imageRect, m_image );
    }
}



void SlideShowView::setCurrentIndex( const QModelIndex &index, const QModelIndex& )
{
    setCurrentIndex( index );
}

void SlideShowView::setCurrentIndex( const QModelIndex&index )
{
    if ( m_model && index.isValid() && m_enableModelUpdates ) {
        if ( m_image.isNull() )
            m_image = m_model->data( index, Qt::DecorationRole).value<QImage>();

        setImage( qvariant_cast<QContent>(index.data(QContentSetModel::ContentRole)) );
        update();
    }
}

void SlideShowView::nextImage()
{
    int rows = m_model->rowCount();

    if ( rows > 1 ) {
        int row = m_selectionModel->currentIndex().row();
        m_selectionModel->setCurrentIndex( m_model->index( ( row + 1 ) % rows, 0 ), QItemSelectionModel::ClearAndSelect );
    }
}

void SlideShowView::enableModelUpdates()
{
    if ( !m_enableModelUpdates ) {
        m_enableModelUpdates = true;
        if ( m_selectionModel )
            setCurrentIndex(m_selectionModel->currentIndex());
    }
}

void SlideShowView::disableModelUpdates()
{
    m_enableModelUpdates = false;
    stop();
    m_image = QImage();
    setImage(QContent());
}

