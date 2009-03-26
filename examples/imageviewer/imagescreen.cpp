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

#include "imagescreen.h"
#include "iviewer.h"
#include "infoscreen.h"
#include "listscreen.h"
#include <QPainter>
#include <QMenu>
#include <QSoftMenuBar>
#include <qtopiaapplication.h>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>


ImageScreen::ImageScreen(IViewer *viewer)
: QWidget(viewer), _viewer(viewer)
{
    _rotation = 0;
    _zoom     = 1;

    QVBoxLayout *boxLayout = new QVBoxLayout(this);
    _zoomSlider            = new QSlider(Qt::Horizontal,this);
    _zoomSlider->setRange(0.1,10);
    _zoomSlider->setValue(_zoom);
    _zoomSlider->setSingleStep(0.1);
    boxLayout->addWidget(_zoomSlider);
    setLayout(boxLayout);
    _zoomSlider->hide();

    createActions();
    createMenu();

    QSoftMenuBar::setLabel(this,Qt::Key_Select,"FullScreen","FullScreen");
}

void ImageScreen::setImage(const QContent& content)
{
    _content = content;
}

void ImageScreen::createActions()
{

    _rotateLeftAction = new QAction("Left", this);
    connect(_rotateLeftAction, SIGNAL(triggered()),
            this, SLOT(onRotateRight()));

    _rotateRightAction = new QAction("Right", this);
    connect(_rotateRightAction, SIGNAL(triggered()),
            this, SLOT(onRotateLeft()));

    _zoomInAction = new QAction("In", this);
    connect(_zoomInAction, SIGNAL(triggered()),
            this, SLOT(onZoomIn()));

    _zoomOutAction = new QAction("Out", this);
    connect(_zoomOutAction, SIGNAL(triggered()),
            this, SLOT(onZoomOut()));

    _showInfoAction = new QAction("Show Info",this);
    connect(_showInfoAction, SIGNAL(triggered()),
            this, SLOT(onShowInfo()));

    _fullScreenAction = new QAction("Fullscreen",this);
    connect(_fullScreenAction, SIGNAL(triggered()),
            this, SLOT(onFullScreen()));
}

void ImageScreen::createMenu()
{
    QMenu* menu = QSoftMenuBar::menuFor(this);

    QMenu* rotateMenu = menu->addMenu("Rotate");
    rotateMenu->addAction(_rotateLeftAction);
    rotateMenu->addAction(_rotateRightAction);

    QMenu* zoomMenu = menu->addMenu("Zoom");
    zoomMenu->addAction(_zoomInAction);
    zoomMenu->addAction(_zoomOutAction);

    menu->addAction(_fullScreenAction);
    menu->addAction(_showInfoAction);
}

void ImageScreen::keyPressEvent (QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Back:
        doBack();
        break;
    case Qt::Key_Select:
        onFullScreen();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void ImageScreen::doBack()
{
    if (_viewer->isFullScreen())
        onFullScreen();
    _viewer->setCurrentWidget((QWidget*)_viewer->listScreen());
}

void ImageScreen::paintEvent(QPaintEvent* event)
{
    _image = new QImage(_content.file());

    int x  = width()/2-_image->width()/2;
    int y  = height()/2-_image->height()/2;
    int x2 = width()/2;
    int y2 = height()/2;

    QPointF c(x2, y2);
    QPainter painter(this);
    painter.translate(c);
    painter.rotate(_rotation);
    painter.scale(_zoom, _zoom);
    painter.translate(-c);
    painter.drawImage(QPoint(x, y), *_image);
    painter.end();
}

void ImageScreen::onRotateLeft()
{
    _rotation -= 90.0;
    update();
}

void ImageScreen::onRotateRight()
{
    _rotation += 90.0;
    update();
}

void ImageScreen::onZoomOut()
{
    _zoom -= 0.5;
    update();
}

void ImageScreen::onZoomIn()
{
    _zoom += 0.5;
    update();
}

void ImageScreen::onShowInfo()
{
    InfoScreen *infoScreen = new InfoScreen(_viewer);
    infoScreen->setImage(_content);
    infoScreen->showMaximized();
}

void ImageScreen::onFullScreen()
{
    if (_viewer->isFullScreen()) {
        QSoftMenuBar::menuFor(this)->addAction(_showInfoAction);
    } else {
        QSoftMenuBar::menuFor(this)->removeAction(_showInfoAction);
    }
    _viewer->toggleFullScreen();
}
