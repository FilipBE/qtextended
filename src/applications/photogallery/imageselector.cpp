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
#include "imageselector.h"
#include "thumbmodel.h"
#include "photogallery.h"
#include "qsmoothiconview.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QLabel>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QContentFilter>
#include <QItemSelectionModel>
#include <QSmoothList>

#include <private/homewidgets_p.h>

#ifdef USE_PICTUREFLOW
#include <pictureflowview.h>
#endif

class ThumbDelegate : public QAbstractItemDelegate
{
public:
    ThumbDelegate(QObject *parent = 0);

    void paint(
            QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

ThumbDelegate::ThumbDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

void ThumbDelegate::paint(
            QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant decoration = index.data(Qt::DecorationRole);

    switch (decoration.type()) {
    case QVariant::Icon:
        qvariant_cast<QIcon>(decoration).paint(painter, option.rect);
        break;
    case QVariant::Pixmap:
        painter->drawPixmap(option.rect, qvariant_cast<QPixmap>(decoration));
        break;
    case QVariant::Image:
        painter->drawImage(option.rect, qvariant_cast<QImage>(decoration));
        break;
    default:
        break;
    }

    if ( option.state & QStyle::State_Selected ) {
        painter->save();
        painter->setPen( option.palette.color( QPalette::Highlight ) );
        QRect selectionRect = option.rect;
        selectionRect.setWidth( option.rect.width()-1 );
        selectionRect.setHeight( option.rect.height()-1 );
        painter->drawRect( selectionRect );
        painter->restore();
    }
}

QSize ThumbDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sizeHint = qvariant_cast<QSize>(index.data(Qt::SizeHintRole));

    if (sizeHint.isValid()) {
        sizeHint.setWidth((sizeHint.width() * option.decorationSize.height()) / sizeHint.height());
        sizeHint.setHeight(option.decorationSize.height());

        return sizeHint;
    } else {
        return option.decorationSize;
    }
}

ImageSelector::ImageSelector(View view, QWidget *parent)
    : QWidget(parent)
    , m_contentSet(QContentSet::Asynchronous)
    , m_iconView(0)
#ifdef USE_PICTUREFLOW
    , m_flowView(0)
    , m_flowLabel(0)
    , m_flowWidget(0)
#endif
{
    m_contentSet.setSortCriteria(QContentSortCriteria(
            QContentSortCriteria::Property, QLatin1String("none/CreationDate")));

#ifdef USE_PICTUREFLOW
    m_viewButton = new HomeActionButton(QtopiaHome::Green);
    m_viewButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    connect(m_viewButton, SIGNAL(clicked()), this, SLOT(toggleView()));
#endif

    m_title = new QLabel;
    m_title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_title->setIndent(style()->pixelMetric(QStyle::PM_ButtonMargin));
    m_title->setAutoFillBackground(true);

    QLinearGradient gradient(0.0, 0.0, 0.0, 1.0);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(0.5, PhotoGallery::lightGrey.darker(110));
    gradient.setColorAt(1.0, PhotoGallery::lightGrey);

    QPalette palette = m_title->palette();
    palette.setBrush(QPalette::Window, gradient);
    palette.setColor(QPalette::WindowText, PhotoGallery::blue);
    m_title->setPalette(palette);

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
#ifdef USE_PICTUREFLOW
    titleLayout->addWidget(m_viewButton);
#endif
    titleLayout->addWidget(m_title);
    titleLayout->addWidget(backButton);

    m_stack = new QStackedWidget;

    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addLayout(titleLayout);
    layout->addWidget(m_stack);

    setLayout(layout);

    m_model = new ThumbModel(&m_contentSet, this);
    m_selectionModel = new QItemSelectionModel( m_model, this );

    setView(view);
}

ImageSelector::~ImageSelector()
{
}

ImageSelector::View ImageSelector::view() const
{
    return m_view;
}

void ImageSelector::setView(View view)
{
    switch(view) {
    case IconView:
        if (!m_iconView) {
            m_iconView = new QSmoothIconView;
            m_iconView->setFlow(QSmoothIconView::TopToBottom);
            m_iconView->setItemDelegate(new ThumbDelegate(m_iconView));
            m_iconView->setFixedRowCount( 2 );
            m_iconView->setSpacing(6);
            m_iconView->setModel( m_model, m_selectionModel );
            m_iconView->setEmptyText( "No images in this album" );

            QPalette palette = m_iconView->palette();
            palette.setColor(QPalette::Base, Qt::black);
            palette.setColor(QPalette::Text, PhotoGallery::blue);
            m_iconView->setPalette(palette);

            connect(m_iconView, SIGNAL(activated(QModelIndex)),
                    this, SLOT(imageSelected(QModelIndex)));

            m_stack->addWidget(m_iconView);
        }
#ifdef USE_PICTUREFLOW
        m_viewButton->setText(tr("Flow\nview"));
#endif
        m_stack->setCurrentWidget(m_iconView);
        break;
#ifdef USE_PICTUREFLOW
    case FlowView:
        if (!m_flowView) {
            m_flowView = new PictureFlowView;
            m_flowView->setReflectionEffect(PictureFlow::NoReflection);
            m_flowView->setAspectRatioMode(Qt::KeepAspectRatio);
            m_flowView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            m_flowView->setModel(m_model);
            m_flowView->setSelectionModel( m_selectionModel );

            m_flowLabel = new QLabel;
            m_flowLabel->setAutoFillBackground(true);
            m_flowLabel->setAlignment(Qt::AlignCenter);

            QPalette palette = m_flowLabel->palette();
            palette.setColor(QPalette::Window, Qt::black);
            palette.setColor(QPalette::WindowText, Qt::white);

            m_flowLabel->setPalette(palette);

            QBoxLayout *layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(m_flowView);
            layout->addWidget(m_flowLabel);

            m_flowWidget = new QWidget;
            m_flowWidget->setLayout(layout);

            m_stack->addWidget(m_flowWidget);

            connect(m_flowView, SIGNAL(activated(QModelIndex)),
                    this, SLOT(imageSelected(QModelIndex)));
            connect(m_flowView, SIGNAL(currentChanged(QModelIndex)),
                    this, SLOT(flowIndexChanged(QModelIndex)));
        }
        m_viewButton->setText(tr("Index\nview"));
        m_flowLabel->setText(m_flowView->currentModelIndex().data(Qt::DisplayRole).toString());
        m_stack->setCurrentWidget(m_flowWidget);
        break;
#endif
    }
    emit viewChanged(m_view = view);
}

QContent ImageSelector::currentImage() const
{
    return m_model->content( currentIndex() );
}

int ImageSelector::count() const
{
    return m_model->rowCount();
}

void ImageSelector::setAlbum(const QString &name, const QString &categoryId)
{
    m_title->setText(name);

    if (!categoryId.isNull()) {
        m_contentSet.setCriteria(QContentFilter::mimeType(QLatin1String("image/*"))
            & QContentFilter::category(categoryId));
    } else {
        m_contentSet.setCriteria(QContentFilter());
    }
}

void ImageSelector::moveNext()
{
    int row = currentIndex().row() + 1;
    if ( row >= count() )
        row = 0;

    QModelIndex index = m_model->index( row );
    setCurrentIndex( index );
    emit currentChanged(qvariant_cast<QContent>(index.data(QContentSetModel::ContentRole)));

}

void ImageSelector::movePrevious()
{
    int row = currentIndex().row() - 1;
    if ( row < 0 )
        row = count() - 1;

    QModelIndex index = m_model->index( row );
    setCurrentIndex( index );
    emit currentChanged(qvariant_cast<QContent>(index.data(QContentSetModel::ContentRole)));
}

void ImageSelector::startSlideShow()
{
    QModelIndex index = currentIndex();

    if (index.isValid())
        emit startSlideShow(qvariant_cast<QContent>(index.data(QContentSetModel::ContentRole)));
}

QModelIndex ImageSelector::currentIndex() const
{
    return m_selectionModel->currentIndex();
}


void ImageSelector::setCurrentIndex(const QModelIndex& index)
{
    m_selectionModel->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
}

#ifdef USE_PICTUREFLOW
void ImageSelector::flowIndexChanged(const QModelIndex &index)
{
    m_flowLabel->setText(index.data(Qt::DisplayRole).toString());
}
#endif
void ImageSelector::rotateAnticlockwise()
{
    QContent image = qvariant_cast<QContent>(currentIndex().data(QContentSetModel::ContentRole));

    if (!image.isNull())
        PhotoGallery::setRotation(&image, (PhotoGallery::rotation(image) + 270) % 360);
}

void ImageSelector::rotateClockwise()
{
    QContent image = qvariant_cast<QContent>(currentIndex().data(QContentSetModel::ContentRole));

    if (!image.isNull())
        PhotoGallery::setRotation(&image, (PhotoGallery::rotation(image) + 90) % 360);
}

