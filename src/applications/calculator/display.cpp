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
#include <QtopiaApplication>
#include <QPixmap>
#include <QPainter>
#include <QTimer>

#include "display.h"
#include "engine.h"

// Lcd display class
MyLcdDisplay::MyLcdDisplay(QWidget *p)
    :QScrollArea(p)
{
    setWhatsThis( tr("Displays the current input or result") );
    lcdPixmap = 0;
    lcdPainter = 0;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    bigFont.setFamily("dejavu");
    bigFont.setPointSize(11);
    setFont(bigFont);

    if ( !Qtopia::mousePreferred() ) {
        setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
        viewport()->setBackgroundRole( QPalette::NoRole );
        setFrameStyle(NoFrame);
    } else { //pda or touchscreen
        setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed));
    }
}

/*
    QAbstractScrollArea::minimumSizeHint() is too big. Override it.
*/
QSize MyLcdDisplay::minimumSizeHint() const
{
    QSize sz = QScrollArea::minimumSizeHint();
    return QSize(1,1);
}

MyLcdDisplay::~MyLcdDisplay() {
    delete lcdPixmap;
    delete lcdPainter;
}

QSize MyLcdDisplay::sizeHint() const
{
    if (!Qtopia::mousePreferred())
        return QScrollArea::sizeHint();
    else
        return QSize(-1,fontMetrics().lineSpacing()+2*frameWidth());
}

static const int pmHeight = 500;

void MyLcdDisplay::readStack() {
    bool stateOk = systemEngine->checkState();

    int visibleWidth = viewport()->size().width();

    if (!lcdPixmap) {
        lcdPixmap = new QPixmap(visibleWidth-3,pmHeight);
    }
    lcdPixmap->fill(Qt::transparent);

    if ( lcdPainter )
       delete lcdPainter;
    lcdPainter = new QPainter();

    lcdPainter->begin(lcdPixmap);
    lcdPainter->setPen(QApplication::palette().color(QPalette::Text));

    verticalOffset=0; // top margin
    int horizontalOffset = 10; // right margin
    if (!stateOk) {
        QRect r = QFontMetrics(QApplication::font()).boundingRect( 5, 5,
                visibleWidth-horizontalOffset, 80, 
                layoutDirection() == Qt::LeftToRight ? Qt::AlignRight : Qt::AlignLeft, 
                systemEngine->errorString);
        //the alignment below doesn't really matter. r already defines what is necessary
        lcdPainter->drawText( r, Qt::AlignRight, systemEngine->errorString );
        verticalOffset=25;
    } else {
        if (Qtopia::mousePreferred()) {
            QPixmap *tmp;
            if (systemEngine->dStack.isEmpty())
                return;
            int myoffset = 10;
            tmp = systemEngine->dStack.top()->draw();

            int drawPoint = qMax(visibleWidth - tmp->width(),0);
            int srcStart = qMax(tmp->width() - visibleWidth,0);

            lcdPainter->drawPixmap(drawPoint - myoffset,verticalOffset,*tmp,srcStart, 0, -1, -1);
            verticalOffset += tmp->height();
        }
        else {
            niStack = new QStack<QString*>();
            ndStack = new QStack<Data*>();
            while (!systemEngine->iStack.isEmpty())
                niStack->push(systemEngine->iStack.pop());
            while (!systemEngine->dStack.isEmpty())
                ndStack->push(systemEngine->dStack.pop());
            dataLeft = 1;
            while (!niStack->isEmpty() || !ndStack->isEmpty()) {
                horizontalOffset = 10;
                horizontalOffset = drawNextItem(horizontalOffset,true, visibleWidth);
            }

            delete niStack;
            delete ndStack;
        }
    }
    lcdPainter->end();

    QWidget *w = viewport();
    //w->resize(visibleWidth, verticalOffset);
    //w->update(0, 0, visibleWidth, verticalOffset);
    w->update();
}

int MyLcdDisplay::drawNextItem(int hoffset,bool newline, int visibleWidth)
{
    QPixmap *tmp;
    int myoffset = hoffset;
    if (!niStack->isEmpty() && *(niStack->top()) == "Open brace impl") { // No tr
        Instruction *tmpi = systemEngine->resolve(*(niStack->top()));
        tmp = tmpi->draw();
        systemEngine->iStack.push(niStack->pop());
        myoffset += drawNextItem(hoffset,false, visibleWidth);
    } else if (dataLeft && !ndStack->isEmpty()) {
        tmp = ndStack->top()->draw();
        systemEngine->dStack.push(ndStack->pop());
        dataLeft--;
    } else if (!niStack->isEmpty()) {
        Instruction *tmpi = systemEngine->resolve(*(niStack->top()));
        tmp = tmpi->draw();
        dataLeft = tmpi->argCount - 1;
        systemEngine->iStack.push(niStack->pop());
        if (tmpi->name != "EvaluateLine") // No tr
            myoffset += drawNextItem(hoffset,false, visibleWidth);
    } else
        return 0;
    int drawPoint = qMax(visibleWidth - tmp->width(),0);
    int srcStart = qMax(tmp->width() - visibleWidth,0);
    lcdPainter->drawPixmap(drawPoint - myoffset,verticalOffset,*tmp,srcStart, 0, -1, -1);
    if (newline) {
        verticalOffset += tmp->height();
        if ( lcdPixmap->height() - verticalOffset < 50 ) {
            lcdPainter->end();
            QPixmap *old = lcdPixmap;
            lcdPixmap = new QPixmap( visibleWidth-3, old->height()+pmHeight );
            lcdPixmap->fill(Qt::transparent);
            lcdPainter->begin(lcdPixmap);
            lcdPainter->drawPixmap( 0, 0, *old );
            lcdPainter->setPen(Qt::color1);
            delete old;
        }
    }
    return myoffset + tmp->width();
}

void MyLcdDisplay::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);
    QPainter wPainter(viewport());

    if (lcdPixmap) {
        int offset = qMin(0, viewport()->size().height() - verticalOffset);
        wPainter.drawPixmap(1,offset,*lcdPixmap);
    }
}

void MyLcdDisplay::resizeEvent(QResizeEvent *re)
{
    QScrollArea::resizeEvent(re);
    if (lcdPixmap) {
        delete lcdPixmap;
        lcdPixmap = 0;
        QTimer::singleShot(0, this, SLOT(readStack()));
    }
}
