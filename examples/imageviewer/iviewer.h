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

#ifndef IVIEWER_H
#define IVIEWER_H

#include <QStackedWidget>
#include <QWidget>

class ListScreen;
class ImageScreen;
class QKeyEvent;

class IViewer : public QStackedWidget 
{
    Q_OBJECT
public:
    IViewer(QWidget *parent=0, Qt::WFlags f=0);
    ImageScreen *imageScreen();
    ListScreen  *listScreen();
    void toggleFullScreen();
    void keyPressEvent(QKeyEvent *ke);
private:
    ListScreen  *_listScreen;
    ImageScreen *_imageScreen;

};

#endif
