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

#ifndef IMAGESCREEN_H
#define IMAGESCREEN_H

#include <QWidget>
#include <QKeyEvent>
#include <QContent>
#include <QImage>

class IViewer;
class QSlider;

class ImageScreen : public QWidget 
{
    Q_OBJECT
public:
    ImageScreen(IViewer *viewer);
    void setImage(const QContent &content);
private:
    void createActions();
    void createMenu();
    void paintEvent(QPaintEvent *event);
    void doBack();
protected:
    void keyPressEvent (QKeyEvent *event);
private slots:
    void onRotateLeft();
    void onRotateRight();
    void onZoomIn();
    void onZoomOut();
    void onShowInfo();
    void onFullScreen();
private:
    IViewer *_viewer;
    QImage  *_image;
    QContent _content;
    qreal    _rotation;
    qreal    _zoom;
    QAction *_rotateLeftAction;
    QAction *_rotateRightAction;
    QAction *_zoomInAction;
    QAction *_zoomOutAction;
    QAction *_showInfoAction;
    QAction *_fullScreenAction;
    QSlider *_zoomSlider;
};

#endif
