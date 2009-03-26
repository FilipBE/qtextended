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

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QtGui>

class QColorButton;

class SideBar : public QWidget
{
Q_OBJECT
public:
    SideBar(QWidget *parent = 0, Qt::WindowFlags f = 0);

    void setThumbNail(const QImage &thumbnail);
    virtual QSize sizeHint() const;
signals:
    void playButtonPushed();
    void addToQueueButtonPushed();
protected:
    virtual void resizeEvent(QResizeEvent *event);
private:
    QLabel *thumbnail;
    QPushButton *playButton;
    QPushButton *addToQueueButton;
    QImage originalThumbnail;
    QPixmap noImage;
};
#endif
