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

#include <QApplication>
#include <QPalette>
#include <QPainter>
#include "data.h"

Data::Data(){
    clear();
    cache = 0;
};
Data::~Data() {
    delete cache;
};

bool Data::push(char,bool) {
    return false;
}

bool Data::del() {
    return true;
}

void Data::clear() {}

QString Data::getType() {
    return QString("NONE");
}

QString Data::getFormattedOutput() {
    return formattedOutput;
}

QPixmap* Data::draw() {
    QString currentOutput = getFormattedOutput();
    static QFont myFont = QFont( "dejavu", 10, QFont::Bold );
    if (cachedOutput != currentOutput) {
        delete cache;
        cache = 0;
    }
    if (!cache) {
        cachedOutput = currentOutput;
        QRect bRect = QFontMetrics(myFont).boundingRect(0,0,240,20,Qt::AlignLeft,cachedOutput);
        cache = new QPixmap(bRect.size());
        cache->fill(Qt::transparent);
        QPainter p(cache);
        p.setPen(QApplication::palette().color(QPalette::Text));
        p.setFont(myFont);
        p.drawText(0,0,bRect.width(),bRect.height(),Qt::AlignRight,cachedOutput);
    }
    return cache;
}
