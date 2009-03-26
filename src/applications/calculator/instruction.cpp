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

#include "instruction.h"
#include "engine.h"

#include <QApplication>
#include <QPalette>
#include <QPainter>

Instruction::Instruction() {
    precedence = 0;
    name = type = retType = "NULL";
    argCount = 0;
    cache = 0;
}

Instruction::~Instruction() {
    delete cache;
};
void Instruction::eval(){qWarning("empty instruction called");}

QPixmap* Instruction::draw() {
    if (!cache) {
        QFont myFont = QFont( "dejavu", 9, QFont::Bold );
        QRect bRect = QFontMetrics(myFont).boundingRect(0,0,240,20,Qt::AlignLeft,displayString);
        cache = new QPixmap(bRect.size());
        cache->fill(Qt::transparent);
        QPainter p(cache);
        p.setPen(QApplication::palette().color(QPalette::Text));
        p.setFont(myFont);
        p.drawText(0,0,bRect.width(),bRect.height(),Qt::AlignRight,displayString);
    }
    return cache;
}
