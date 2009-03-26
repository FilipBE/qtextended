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

#include "iviewer.h"
#include "listscreen.h"
#include "imagescreen.h"
#include <QtopiaApplication>
#include <QTimer>
#include <QKeyEvent>


IViewer::IViewer(QWidget *parent, Qt::WFlags /*f*/)
: QStackedWidget(parent)
{
    _listScreen  = 0;
    _imageScreen = 0;
    setCurrentWidget(listScreen());
    setWindowTitle("Image Viewer");
}

ListScreen* IViewer::listScreen()
{
    if (!_listScreen) {
        _listScreen = new ListScreen(this);
        addWidget(_listScreen);
    }
    return _listScreen;
}

ImageScreen* IViewer::imageScreen()
{
    if (!_imageScreen) {
        _imageScreen = new ImageScreen(this);
        addWidget(_imageScreen);
    }
    return _imageScreen;
}

void IViewer::toggleFullScreen()
{
    QString title = windowTitle();
    setWindowTitle( QLatin1String("_allow_on_top_"));
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    setWindowTitle(title);
}

void IViewer::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Back) {
        qWarning() << "Iviewer handles back event";
    }
    QWidget::keyPressEvent(ke);
}
