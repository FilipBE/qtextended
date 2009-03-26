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
#include "imageview.h"
#include "zoomslider.h"
#include "smoothimagemover.h"
#include "photogallery.h"
#include "titlewindow.h"
#include "imageloader.h"

#include <QToolButton>
#include <QLabel>
#include <QStyle>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QTimer>
#include <QPainter>
#include <QCategoryManager>
#include <QTransform>
#include <QThread>
#include <QtopiaChannel>
#include <QSoftMenuBar>
#include <QMenu>
#include <QSettings>
#include <QDebug>
#include <QFileSystem>
#include <QWaitWidget>
#include <QtopiaService>
#include <QtopiaServiceRequest>
#include <QImageReader>
#include <QImageWriter>
#include <QDesktopWidget>

#include <QAbstractItemModel>
#include <QItemSelectionModel>

#include <gfxpainter.h>
#include <qscreen_qws.h>
#include <qtopianamespace.h>

#include <private/homewidgets_p.h>

ImageView::ImageView(QWidget *parent)
: QWidget(parent)
, m_model(0)
, m_selectionModel(0)
, m_enableModelUpdates(true)
, m_smoothUpdateTimerId(-1)
, m_rotation(0)
, m_zoom(100)
, m_isLoading(false)
{
    m_imageMover = new SmoothImageMover(this);
    connect(m_imageMover, SIGNAL(positionChanged(const QPoint&)),  SLOT(setImageOffset(const QPoint&)));

    m_imageLoader = new ImageLoader();
    connect( m_imageLoader, SIGNAL(loaded(const QImage&,bool)), this,  SLOT(setImage(const QImage&,bool)) );

    m_zoomSteps << 25 << 40 << 63 << 100 << 160 << 250 << 400;

    m_zoomSlider = new ZoomSlider;
    m_zoomSlider->setRange(0, m_zoomSteps.count() - 1);
    m_zoomSlider->setValue(m_zoomSteps.indexOf(100));
    m_zoomSlider->setSingleStep(1);
    m_zoomSlider->setPageStep(1);
    m_zoomSlider->setVisible(false);
    connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));

    QBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(12);
    layout->addStretch();
    layout->addWidget(m_zoomSlider, 0, Qt::AlignRight);
    layout->addStretch();

    setLayout(layout);

    QToolButton *nextButton = new QToolButton;
    nextButton->setText(">");
    nextButton->setIcon(QIcon());
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextImage()));

    QToolButton *previousButton = new QToolButton;
    previousButton->setText("<");
    previousButton->setIcon(QIcon());
    connect(previousButton, SIGNAL(clicked()), this, SLOT(previousImage()));

    m_title = new QLabel;
    m_title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_title->setIndent(style()->pixelMetric(QStyle::PM_ButtonMargin));

    QFont font = m_title->font();
    font.setBold(true);
    m_title->setFont(font);

    HomeActionButton *backButton = new HomeActionButton(QtopiaHome::Red);
    backButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    backButton->setText(tr("Back"));

    connect(backButton, SIGNAL(clicked()), this, SIGNAL(back()));

    QBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(0);
    titleLayout->setMargin(0);
    titleLayout->addWidget(previousButton);
    titleLayout->addWidget(nextButton);
    titleLayout->addWidget(m_title);
    titleLayout->addWidget(backButton);

    m_controls = new TitleWindow(this);
    m_controls->setLayout(titleLayout);

    connect(nextButton, SIGNAL(clicked()), m_controls, SLOT(resetHideTimer()));
    connect(previousButton, SIGNAL(clicked()), m_controls, SLOT(resetHideTimer()));

    QPalette palette = m_controls->palette();
    palette.setColor(QPalette::Window, QColor(0, 0, 0, 128));

    m_controls->setPalette(palette);

    connect(m_controls, SIGNAL(visibilityChanged(bool)), this, SLOT(titleVisibilityChanged(bool)));
    connect(m_controls, SIGNAL(visibilityChanged(bool)), m_zoomSlider, SLOT(setVisible(bool)));
    connect(m_controls, SIGNAL(opacityChanged(int)), m_zoomSlider, SLOT(setOpacity(int)));

    QMenu *menu = QSoftMenuBar::menuFor(this);

    menu->addAction(tr("Set as background"), this, SLOT(setAsBackgroundImage()));

    if (!QtopiaService::apps(QLatin1String("Contacts")).isEmpty())
        menu->addAction(tr("Set as avatar"), this, SLOT(setAsAvatar()));

    connect(menu, SIGNAL(aboutToShow()), m_controls, SLOT(resetHideTimer()));
}

ImageView::~ImageView()
{
}

QAbstractItemModel *ImageView::model() const
{
    return m_model;
}

void ImageView::setModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel )
{
    if ( m_model )
        disconnect( m_model, 0, this, 0 );

    m_model = model;

    if (model) {
        connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));

        if ( selectionModel )
            setSelectionModel( selectionModel );
        else
            setSelectionModel( new QItemSelectionModel(model,model) );
    } else {
        setSelectionModel(0);
    }
}

void ImageView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (m_selectionModel)
        disconnect( m_selectionModel, 0, this, 0 );

    m_selectionModel = selectionModel;

    connect( m_selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(setCurrentIndex(QModelIndex,QModelIndex)) );

    if ( selectionModel->currentIndex().isValid() )
        setCurrentIndex( selectionModel->currentIndex(), QModelIndex() );
}

QItemSelectionModel* ImageView::selectionModel()
{
    return m_selectionModel;
}

void ImageView::setImage( const QContent &imageContent, int imageNumber )
{
    m_title->setText(imageContent.name());
    if ( !imageContent.isNull() ) {
        m_currentImage = imageNumber;
        m_content = imageContent;
        m_isLoading = true;

        m_image = QImage();

        foreach( int row, m_thumbnailsCache.keys() ) {
            if ( !( row == imageNumber || row == nextImageNumber() || row == previousImageNumber() )  )
                m_thumbnailsCache.remove( row );
        }

        foreach( int row, m_screenSizeImageCache.keys() ) {
            if ( !( row == imageNumber || row == nextImageNumber() || row == previousImageNumber() )  )
                m_screenSizeImageCache.remove( row );
        }

        if ( !m_screenSizeImageCache.contains(m_currentImage) )
            m_imageLoader->loadImage( imageContent, size() );

        m_rotation = PhotoGallery::rotation(m_content);

        updateBoundaries();
        zoomToFit();
    } else {
        m_imageLoader->loadImage( imageContent );
        m_image = QImage();
        m_screenSizeImageCache.clear();
        m_thumbnailsCache.clear();
    }
}


void ImageView::setImage( const QImage& image, bool downscaled )
{
    m_isLoading = false;
    if ( downscaled )
        m_screenSizeImageCache[m_currentImage] = image;
    else {
        m_image = image;
        m_screenSizeImageCache.remove(m_currentImage);
    }

    updateBoundaries();
    update();
}

QImage ImageView::image() const
{
    if ( !m_image.isNull() )
        return m_image;

    if ( m_screenSizeImageCache.contains( m_currentImage )  )
        return m_screenSizeImageCache[m_currentImage];

    if ( m_thumbnailsCache.contains( m_currentImage )  )
        return m_thumbnailsCache[m_currentImage];

    return QImage();
}

int ImageView::nextImageNumber() const
{
    int rows = m_model->rowCount();
    if ( rows > 1 )
        return ( m_currentImage + 1 ) % rows;
    else
        return -1;
}

int ImageView::previousImageNumber() const
{
    int rows = m_model->rowCount();
    if ( rows > 1 )
        return ( m_currentImage + rows - 1 ) % rows;
    else
        return -1;
}

QImage ImageView::nextThumbnail() const
{
    int row = nextImageNumber();

    if ( row != -1 ) {
        if ( m_screenSizeImageCache.contains( row )  )
            return m_screenSizeImageCache[row];

        if ( m_thumbnailsCache.contains( row )  )
            return m_thumbnailsCache[row];
    }

    return QImage();
}

QImage ImageView::previousThumbnail() const
{
    int row = previousImageNumber();

    if ( row != -1 ) {
        if ( m_screenSizeImageCache.contains( row )  )
            return m_screenSizeImageCache[row];

        if ( m_thumbnailsCache.contains( row )  )
            return m_thumbnailsCache[row];
    }

    return QImage();
}

void ImageView::setNextThumbnail( const QImage& img )
{
    m_thumbnailsCache[ nextImageNumber() ] = img;
    update();
}

void ImageView::setPreviousThumbnail( const QImage& img )
{
    m_thumbnailsCache[ previousImageNumber() ] = img;
    update();
}

void ImageView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton ) {
        m_pressedPos = event->pos();
        m_imageMover->startMoving( event->pos() );
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (!m_pressedPos.isNull()) {
            if ((event->pos() - m_pressedPos).manhattanLength() > PhotoGallery::clickThreshold()) {
                m_pressedPos = QPoint();
                m_controls->hide();
                m_imageMover->moveTo( event->pos() );
            }
        } else {
            m_imageMover->moveTo( event->pos() );
        }
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton ) {
        if (!m_pressedPos.isNull()) {
            m_imageMover->endMoving(m_pressedPos);

            switch (m_controls->state()) {
            case TitleWindow::Hidden:
            case TitleWindow::FadingOut:
                m_controls->fadeIn();
                break;
            case TitleWindow::Visible:
            case TitleWindow::FadingIn:
                m_controls->fadeOut();
            }

            m_pressedPos = QPoint();
        } else {
            m_imageMover->endMoving(event->pos());

            if ( m_zoom <= 100 ) {
                if ( m_imageOffset.x() < -width()/3 ) {
                    nextImage();
                } else if ( m_imageOffset.x() > width()/3 ) {
                    previousImage();
                }
            }
        }
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void ImageView::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_smoothUpdateTimerId) {
        update();
    } else {
        QWidget::timerEvent(event);
    }
}


void ImageView::paintEvent(QPaintEvent *event)
{
    if ( m_isLoading && image().isNull() ) {
        QPainter painter(this);
        QFont font = painter.font();
        font.setWeight(75);
        painter.setFont( font );
        painter.drawText(rect(), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextWordWrap, tr("Loading..."));
        return;
    }

    if ( m_smoothUpdateTimerId != -1 ) {
        killTimer( m_smoothUpdateTimerId );
        m_smoothUpdateTimerId = -1;
    }

    GfxPainter p(this,event);
    bool smoothScale = false;

    if ( !m_isLoading ) {
        if ( m_imageOffset != m_prevPainterOffset ) {
            smoothScale = false;
            m_prevPainterOffset = m_imageOffset;
            m_smoothUpdateTimerId = startTimer(100);
        } else {
            smoothScale = true;
        }
    }


    if ( !image().isNull() )
        p.drawImageTransformed( m_transform.toAffine(), image(), smoothScale );

    if ( m_zoom <= 100 ) {
        if ( !m_prevImageTransform.isIdentity() && !previousThumbnail().isNull() )
            p.drawImageTransformed( m_prevImageTransform.toAffine(), previousThumbnail(), false );
        if ( !m_nextImagetransform.isIdentity() && !nextThumbnail().isNull() )
            p.drawImageTransformed( m_nextImagetransform.toAffine(), nextThumbnail(), false );
    }
}

void ImageView::updateTransformation()
{
    m_transform = QTransform();
    QSize imageSize = image().size();

    int rotation = m_rotation;
    if ( m_isLoading )
        rotation = 0; // thumbnail is already rotated

    switch ( rotation ) {
        case 90:
            m_transform *= QTransform().rotate(90);
            m_transform *= QTransform().translate( imageSize.height(), 0 );
            imageSize = QSize( imageSize.height(), imageSize.width() );
            break;
        case 180:
            m_transform *= QTransform().rotate(180);
            m_transform *= QTransform().translate( imageSize.width(), imageSize.height() );
            break;
        case 270:
            m_transform *= QTransform().rotate(270);
            m_transform *= QTransform().translate( 0, imageSize.width() );
            imageSize = QSize( imageSize.height(), imageSize.width() );
            break;
        default:
            break;
    }

    double scaleFactor = qMin( double(geometry().width())/imageSize.width(),
                               double(geometry().height())/imageSize.height() );

    scaleFactor *= m_zoom/100.0;

    m_transform *= QTransform().scale( scaleFactor, scaleFactor );
    imageSize *= scaleFactor;

    m_imageRect = QRect( QPoint(0,0), imageSize );
    m_imageRect.moveCenter( geometry().center() );
    m_imageRect.moveTopLeft( m_imageRect.topLeft()+m_imageOffset );

    m_transform *= QTransform().translate( m_imageRect.left(), m_imageRect.top() );


    m_nextImagetransform = QTransform();
    m_prevImageTransform = QTransform();

    //next image transform:
    if ( m_zoom <= 100 && !nextThumbnail().isNull() ) {
        QTransform transform;
        imageSize = nextThumbnail().size();
        double scaleFactor = qMin( double(geometry().width())/imageSize.width(),
                                   double(geometry().height())/imageSize.height() );

        scaleFactor *= m_zoom/100.0;

        transform = QTransform().scale( scaleFactor, scaleFactor );
        imageSize *= scaleFactor;

        QRect imageRect = QRect( QPoint(0,0), imageSize );
        imageRect.moveCenter( geometry().center() );
        imageRect.moveTopLeft( imageRect.topLeft() + m_imageOffset );
        imageRect.moveLeft( imageRect.left()+ geometry().width() + geometry().width()/5  );

        transform *= QTransform().translate( imageRect.left(), imageRect.top() );

        m_nextImagetransform = transform;
    }

    if ( m_zoom <= 100 && !previousThumbnail().isNull() ) {
        QTransform transform;
        imageSize = previousThumbnail().size();
        double scaleFactor = qMin( double(geometry().width())/imageSize.width(),
                                   double(geometry().height())/imageSize.height() );

        scaleFactor *= m_zoom/100.0;

        transform = QTransform().scale( scaleFactor, scaleFactor );
        imageSize *= scaleFactor;

        QRect imageRect = QRect( QPoint(0,0), imageSize );
        imageRect.moveCenter( geometry().center() );
        imageRect.moveTopLeft( imageRect.topLeft() + m_imageOffset );
        imageRect.moveLeft( imageRect.left() - geometry().width() - geometry().width()/5  );

        transform *= QTransform().translate( imageRect.left(), imageRect.top() );

        m_prevImageTransform = transform;
    }
}

void ImageView::rotateClockwise()
{
    m_rotation += 90;
    m_rotation %= 360;
    m_imageOffset = QPoint(0,0);
    updateBoundaries();

    PhotoGallery::setRotation(&m_content, m_rotation);

    m_controls->resetHideTimer();
}

void ImageView::rotateAnticlockwise()
{
    m_rotation += 270;
    m_rotation %= 360;
    m_imageOffset = QPoint(0,0);
    updateBoundaries();

    PhotoGallery::setRotation(&m_content, m_rotation);

    m_controls->resetHideTimer();
}

void ImageView::addToFavorites()
{
    const QString categoryId(QLatin1String("PhotoAlbumFavorites"));

    QCategoryManager manager(QLatin1String("PhotoAlbums"));

    if (manager.ensureSystemCategory(categoryId, QLatin1String("Favorites"))) {
        QStringList categories = m_content.categories();

        if (!categories.contains(categoryId)) {
            m_content.setCategories(categories << categoryId);
            m_content.commit();
        }
    }

    m_controls->resetHideTimer();
}

#define HOMESCREEN_IMAGE_NAME QLatin1String(".HomescreenImage")
#define HOMESCREEN_IMAGE_PATH Qtopia::documentDir() + HOMESCREEN_IMAGE_NAME

void ImageView::setAsBackgroundImage()
{
    QString fileName;

    QImageReader reader(m_content.fileName());

    QSize imageSize = reader.size();
    QSize desktopSize = qApp->desktop()->size();

    if (imageSize.height() > imageSize.width() == desktopSize.height() > desktopSize.height())
        desktopSize.transpose();

    if (imageSize.height() > desktopSize.height() * 3 / 2
        && imageSize.width() > desktopSize.width() * 3 / 2) {
        if (reader.supportsOption(QImageIOHandler::ScaledSize)) {
            imageSize.scale(desktopSize, Qt::KeepAspectRatioByExpanding);

            reader.setScaledSize(imageSize);
            reader.setQuality(49);

            QImage image;

            if (reader.read(&image)) {
                QImageWriter writer(HOMESCREEN_IMAGE_PATH, "PNG");

                if (writer.write(image))
                    fileName = writer.fileName();
            }
        }
    } else if (QFileSystem::fromFileName(m_content.fileName()).isRemovable()) {
        fileName = HOMESCREEN_IMAGE_PATH;

        if (!copyImage(m_content, fileName)) {
            fileName = QString();
        }
    } else {
        fileName = m_content.fileName();
    }

    if (fileName.isNull()) {
        QMessageBox message(
                QMessageBox::Warning,
                tr("Background Image"),
                tr("Could not save image to permanent storage"),
                QMessageBox::Ok,
                this);
        message.setWindowModality(Qt::WindowModal);
        message.exec();
    } else {
        QSettings cfg("Trolltech","qpe");
        cfg.beginGroup("HomeScreen"); {
            cfg.setValue("HomeScreenPicture", fileName);
        }

        QtopiaChannel::send("QPE/System", "updateHomeScreenImage()");
    }

    m_controls->resetHideTimer();
}

bool ImageView::copyImage(const QContent &image, const QString &target)
{
    bool copied = false;

    if (QIODevice *source = image.open()) {
        QFile destination(target + QLatin1String(".part"));

        if (destination.open(QIODevice::WriteOnly)) {
            char buffer[65536];

            if (source->size() > 524288) {
                while (!source->atEnd()) {
                    qint64 bytesRead = source->read(buffer, 65536);

                    if (!(copied = (destination.write(buffer, bytesRead) == bytesRead)))
                        break;
                }
            } else {
                QWaitWidget wait(this);

                wait.setCancelEnabled(true);
                wait.show();

                while (!source->atEnd()) {
                    QCoreApplication::processEvents();

                    if (wait.wasCancelled()) {
                        copied = false;
                        break;
                    }

                    qint64 bytesRead = source->read(buffer, 65536);

                    if (!(copied = (destination.write(buffer, bytesRead) == bytesRead)))
                        break;
                }
            }
            destination.close();

            if (copied)
                copied = destination.rename(target);

            if (!copied)
                destination.remove();
    }

        source->close();

        delete source;
    }

    return copied;
}

void ImageView::setAsAvatar()
{
    QString fileName = m_content.fileName();

    if (!fileName.isNull()) {
        QtopiaServiceRequest request(
                QLatin1String("Contacts"),
                QLatin1String("setPersonalImage(QString)"));
        request << fileName;
        request.send();
    }

    m_controls->resetHideTimer();
}

void ImageView::zoomToFit()
{
    m_zoomSlider->setValue(m_zoomSteps.indexOf(100));
}

void ImageView::setZoom(int index)
{
    int oldZoom = m_zoom;
    m_zoom = m_zoomSteps.at(index);
    m_imageOffset = m_imageOffset * m_zoom / oldZoom;
    updateBoundaries();

    m_controls->resetHideTimer();

    if ( m_zoom > 100 && m_image.isNull() )
        m_imageLoader->loadImage( m_content ); // load full size image
}

void ImageView::setImageOffset(const QPoint &pos )
{
    m_imageOffset = pos;
    updateTransformation();
    update();
}

void ImageView::updateBoundaries()
{
    updateTransformation();
    int dw = qMax( 1, m_imageRect.width() - geometry().width() );
    int dh = qMax( 1, m_imageRect.height() - geometry().height() );

    QRect boundaries = QRect( -dw/2, -dh/2, dw, dh );

    m_imageMover->setAllowedRect( boundaries );
    m_imageMover->setPosition( m_imageOffset );

    if (m_zoom <= 100)
        m_imageMover->fixMovements(SmoothImageMover::Vertical);
    else
        m_imageMover->allowMovements(SmoothImageMover::Vertical);
}

void ImageView::titleVisibilityChanged(bool visible)
{
    if (visible) {
        QtopiaChannel::send(QLatin1String("QPE/System"), QLatin1String("showContextBar()"));
    } else if (isVisible() && window()->isActiveWindow()) {
        window()->raise();

        QSoftMenuBar::menuFor(this)->hide();
    }
}

void ImageView::dataChanged( const QModelIndex &from , const QModelIndex &to )
{
    if ( m_enableModelUpdates ) {
        int rows = m_model->rowCount();
        int row = m_selectionModel->currentIndex().row();

        if ( rows > 1 ) {
            int nextRow = ( row + 1 ) % rows;
            int prevRow = ( row - 1 + rows ) % rows;

            if ( nextRow >= from.row() && nextRow <= to.row() )
                setNextThumbnail( m_model->data( m_model->index( nextRow, 0), Qt::DecorationRole).value<QImage>() );

            if ( prevRow >= from.row() && prevRow <= to.row() )
                setPreviousThumbnail( m_model->data( m_model->index( prevRow, 0), Qt::DecorationRole).value<QImage>() );
        }
    }
}

void ImageView::enableModelUpdates()
{
    if ( !m_enableModelUpdates ) {
        m_enableModelUpdates = true;
        if ( m_selectionModel ) {
            QModelIndex index = m_selectionModel->currentIndex();
            if ( !m_thumbnailsCache.contains(index.row()) )
                m_thumbnailsCache.insert( index.row(), m_model->data( index, Qt::DecorationRole).value<QImage>() );
            setCurrentIndex( index );
        }
    }
}

void ImageView::disableModelUpdates()
{
    m_enableModelUpdates = false;
    setImage( QContent() );
}

void ImageView::setCurrentIndex(const QModelIndex& index, const QModelIndex& )
{
    setCurrentIndex( index );
}

void ImageView::setCurrentIndex(const QModelIndex& index)
{
    if ( m_model && index.isValid() && m_enableModelUpdates ) {
        setImage( qvariant_cast<QContent>(index.data(QContentSetModel::ContentRole)), index.row() );

        int rows = m_model->rowCount();

        if ( rows > 1 ) {
            int nextRow = ( index.row() + 1 ) % rows;
            int prewRow = ( index.row() - 1 + rows ) % rows;

            if ( nextThumbnail().isNull() )
                setNextThumbnail( m_model->data(m_model->index( nextRow, 0), Qt::DecorationRole).value<QImage>() );

            if ( previousThumbnail().isNull() )
                setPreviousThumbnail( m_model->data(m_model->index( prewRow, 0), Qt::DecorationRole).value<QImage>() );

        }
        updateTransformation();
    } else {
        setImage( QImage(), true );
        setNextThumbnail( QImage() );
        setPreviousThumbnail( QImage() );
    }
}

void ImageView::nextImage()
{
    m_imageMover->setPosition( QPoint( m_imageOffset.x() + width() + width()/5, m_imageOffset.y() ) );

    int rows = m_model->rowCount();
    int row = m_selectionModel->currentIndex().row();

    m_selectionModel->setCurrentIndex( m_model->index( ( row + 1 ) % rows, 0 ), QItemSelectionModel::ClearAndSelect );
}

void ImageView::previousImage()
{
    m_imageMover->setPosition( QPoint( m_imageOffset.x() - width() - width()/5, m_imageOffset.y() ) );

    int rows = m_model->rowCount();
    int row = m_selectionModel->currentIndex().row();

    m_selectionModel->setCurrentIndex( m_model->index( ( row - 1 + rows ) % rows, 0 ), QItemSelectionModel::ClearAndSelect );
}

