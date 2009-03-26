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

#include "albumview.h"
#include <pictureflowview.h>
#include <QMarqueeLabel>
#include <QMediaPlaylist>
#include <QFont>

AlbumView::AlbumView(QWidget *parent)
    : QWidget(parent)
{
    QPalette pal=palette();
    pal.setColor(QPalette::Window, Qt::black);
    pal.setColor(QPalette::WindowText, Qt::white);
    setPalette(pal);
    pfView=new PictureFlowView;
    pfView->setReflectionEffect(PictureFlow::NoReflection);
    artistLabel=new QLabel;
    QFont artistFont=artistLabel->font();
    artistFont.setPointSizeF(artistFont.pointSizeF()*1.33);
    artistFont.setBold(true);
    artistLabel->setFont(artistFont);
    albumLabel=new QLabel;
    QVBoxLayout *outerLayout=new QVBoxLayout;
    outerLayout->setSpacing(0);

    QFormLayout *innerLayout=new QFormLayout;
    innerLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    innerLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    innerLayout->setFormAlignment(Qt::AlignLeft);
    innerLayout->setLabelAlignment(Qt::AlignLeft);
    innerLayout->setSpacing(0);

    outerLayout->addItem(innerLayout);
    QLabel *aLabel=new QLabel(tr("Artist:  "));
    artistFont=aLabel->font();
    artistFont.setBold(true);
    aLabel->setFont(artistFont);
    innerLayout->addRow(new QLabel(tr("Artist:")), artistLabel);
    innerLayout->addRow(new QLabel, albumLabel);
    outerLayout->addWidget(pfView, 1);
    setLayout(outerLayout);
    connect(pfView, SIGNAL(activated(QModelIndex)), this, SIGNAL(clicked(QModelIndex)));
    connect(pfView, SIGNAL(currentChanged(QModelIndex)), this, SLOT(updateLabels(QModelIndex)));
}

AlbumView::~AlbumView()
{
    delete pfView;
    delete artistLabel;
    delete albumLabel;
}

void AlbumView::setModel(QAbstractItemModel *model, int displayRole)
{
    pfView->setModel(model);
    pfView->setModelRole(displayRole);
}

void AlbumView::setIndex(int index)
{
    pfView->showSlide(index);
}

void AlbumView::updateLabels(const QModelIndex& index)
{
    artistLabel->setText(pfView->model()->data(index, QMediaPlaylist::Artist).toString());
    albumLabel->setText(pfView->model()->data(index, QMediaPlaylist::Album).toString());
}

void AlbumView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    pfView->setSlideSize(QSize(pfView->size().height(), pfView->size().height()) * 0.75);
}
