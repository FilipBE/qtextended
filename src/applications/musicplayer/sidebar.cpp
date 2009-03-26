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

#include "sidebar.h"
#include <QColorButton>

SideBar::SideBar(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent,f)
{
    thumbnail = new QLabel;
    thumbnail->setAlignment(Qt::AlignCenter);
    playButton = new QPushButton("Play");
    QPalette pal=playButton->palette();
    pal.setColor(QPalette::Base, qRgb(49, 129, 16));
    pal.setColor(QPalette::Text, Qt::white);
    playButton->setPalette(pal);

    addToQueueButton = new QPushButton("Add to\nQueue");
    pal=addToQueueButton->palette();
    pal.setColor(QPalette::Base, qRgb(41, 93, 139));
    pal.setColor(QPalette::Text, Qt::white);
    addToQueueButton->setPalette(pal);

    QVBoxLayout *layout=new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->addWidget(thumbnail, 1);
    layout->addWidget(playButton);
    layout->addWidget(addToQueueButton);
    setLayout(layout);
    connect(playButton, SIGNAL(clicked(bool)), this, SIGNAL(playButtonPushed()));
    connect(addToQueueButton, SIGNAL(clicked(bool)), this, SIGNAL(addToQueueButtonPushed()));
}

QSize SideBar::sizeHint() const
{
    return QSize(QApplication::desktop()->availableGeometry().width()/5, QApplication::desktop()->availableGeometry().height());
}

void SideBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    thumbnail->resize(event->size().width(), thumbnail->size().height());
    int sw = qMin(thumbnail->size().width(), thumbnail->size().height());
    if(noImage.size() != thumbnail->size()) {
        noImage=QPixmap(sw, sw);

        QPainter painter(&noImage);
        painter.setRenderHint(QPainter::Antialiasing);
        QPoint p1(sw*4/10, 0);
        QPoint p2(sw*6/10, sw);
        QLinearGradient linearGrad(p1, p2);
        linearGrad.setColorAt(0, Qt::darkGray);
        linearGrad.setColorAt(1, Qt::white);
        painter.setBrush(linearGrad);
        painter.fillRect(0, 0, sw, sw, QBrush(linearGrad));

/*        painter.setPen(QPen(QColor(64,64,64), 4));
        painter.setBrush(QBrush());
        painter.drawRect(2, 2, sw-3, sw-3);*/
        painter.setBrush(Qt::black);
        painter.drawText(noImage.rect(), Qt::AlignCenter, tr("No\nCover"));
        painter.end();
    }
    if(originalThumbnail.isNull())
        thumbnail->setPixmap(noImage);
    else
        thumbnail->setPixmap(QPixmap::fromImage(originalThumbnail.scaled(sw, sw, Qt::KeepAspectRatio)));
}

void SideBar::setThumbNail(const QImage &newthumbnail)
{
    if(newthumbnail.cacheKey() != originalThumbnail.cacheKey())
    {
        originalThumbnail = newthumbnail;
        int sw = qMin(thumbnail->size().width(), thumbnail->size().height());
        if(originalThumbnail.isNull())
            thumbnail->setPixmap(noImage);
        else
            thumbnail->setPixmap(QPixmap::fromImage(originalThumbnail.scaled(sw, sw, Qt::KeepAspectRatio)));
    }
}
