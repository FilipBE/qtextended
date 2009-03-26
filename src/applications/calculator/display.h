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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QStack>
#include <QScrollArea>
#include "data.h"

class MyLcdDisplay:public QScrollArea {
    Q_OBJECT
public:
    MyLcdDisplay(QWidget *p=0);
    ~MyLcdDisplay();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    void readStack();

protected:
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *re);

private:
    QPixmap *lcdPixmap;
    QPainter *lcdPainter;
    int drawNextItem(int,bool,int);
    int dataLeft,verticalOffset;
    QStack<QString*> *niStack;
    QStack<Data*> *ndStack;
    QFont bigFont;
};

#endif
