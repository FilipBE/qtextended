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
#include "pickboardpicks.h"
#include "pickboardcfg.h"



#include <QPainter>
#include <QList>
#include <QBitmap>
#include <QLayout>
#include <QDialog>
#include <QMenu>
#include <QPushButton>
#include <QMessageBox>
#ifdef Q_WS_QWS
#include <QWSServer>
#include <qwindowsystem_qws.h>
#endif

#include <QMouseEvent>

void PickboardPicks::doMenu()
{
    QWidget* cause = (QWidget*)sender(); // evil

    QMenu popup(this);
    config()->fillMenu(popup);

    QPoint pos = cause->mapToGlobal(cause->rect().topRight());
    QSize sz = popup.sizeHint();
    pos.ry() -= sz.height();
    pos.rx() -= sz.width();
    popup.move(pos);
    config()->doMenu(popup.exec());
}

static const char *BS_xpm[] = {
"5 7 2 1",
"a c #000000",
". c None",
"...aa",
"..aaa",
".aaaa",
"aaaaa",
".aaaa",
"..aaa",
"...aa",
};
static const char *Del_xpm[] = {
"14 7 2 1",
"a c #000000",
". c None",
"aaa..aaaa.a...",
"a..a.a....a...",
"a..a.a....a...",
"a..a.aaa..a...",
"a..a.a....a...",
"a..a.a....a...",
"aaa..aaaa.aaaa"
};
static const char *Home_xpm[] = {
"20 7 2 1",
"a c #000000",
". c None",
"a..a..aa..a...a.aaaa",
"a..a.a..a.aa.aa.a...",
"a..a.a..a.a.a.a.a...",
"aaaa.a..a.a.a.a.aaa.",
"a..a.a..a.a...a.a...",
"a..a.a..a.a...a.a...",
"a..a..aa..a...a.aaaa"
};
static const char *PgUp_xpm[] = {
"20 7 2 1",
"a c #000000",
". c None",
"aaa.......a..a......",
"a..a......a..a......",
"a..a.aa...a..a.aaa..",
"aaa.a.....a..a.a..a.",
"a...a.aa..a..a.aaa..",
"a...a..a..a..a.a....",
"a....aaa...aa..a...."
};
static const char *PgDn_xpm[] = {
"20 7 2 1",
"a c #000000",
". c None",
"aaa.......aaa.......",
"a..a......a..a......",
"a..a.aa...a..a.a..a.",
"aaa.a.....a..a.aa.a.",
"a...a.aa..a..a.a.aa.",
"a...a..a..a..a.a..a.",
"a....aaa..aaa..a..a."
};
static const char *End_xpm[] = {
"14 7 2 1",
"a c #000000",
". c None",
"aaaa.a..a.aaa.",
"a....aa.a.a..a",
"a....a.aa.a..a",
"aaa..a..a.a..a",
"a....a..a.a..a",
"a....a..a.a..a",
"aaaa.a..a.aaa."
};
static const char *Enter_xpm[] = {
"14 7 2 1",
"a c #000000",
". c None",
".............a",
".............a",
"..a..........a",
".aa.........a.",
"aaaaaaaaaaaa..",
".aa...........",
"..a..........."
};
static const char *Esc_xpm[] = {
"14 7 2 1",
"a c #000000",
". c None",
"aaaa..aa...aa.",
"a....a..a.a..a",
"a....a....a...",
"aaa...aa..a...",
"a.......a.a...",
"a....a..a.a..a",
"aaaa..aa...aa."
};
static const char *Ins_xpm[] = {
"13 7 2 1",
"a c #000000",
". c None",
"aaa.a..a..aa.",
".a..aa.a.a..a",
".a..a.aa.a...",
".a..a..a..aa.",
".a..a..a....a",
".a..a..a.a..a",
"aaa.a..a..aa."
};
static const char *Up_xpm[] = {
"7 7 2 1",
"a c #000000",
". c None",
"...a...",
"..aaa..",
".a.a.a.",
"a..a..a",
"...a...",
"...a...",
"...a..."
};
static const char *Left_xpm[] = {
"7 7 2 1",
"a c #000000",
". c None",
"...a...",
"..a....",
".a.....",
"aaaaaaa",
".a.....",
"..a....",
"...a..."
};
static const char *Down_xpm[] = {
"7 7 2 1",
"a c #000000",
". c None",
"...a...",
"...a...",
"...a...",
"a..a..a",
".a.a.a.",
"..aaa..",
"...a..."
};
static const char *Right_xpm[] = {
"7 7 2 1",
"a c #000000",
". c None",
"...a...",
"....a..",
".....a.",
"aaaaaaa",
".....a.",
"....a..",
"...a..."
};
static const char *BackTab_xpm[] = {
"8 7 2 1",
"a c #000000",
". c None",
"a.......",
"a..a....",
"a.aa....",
"aaaaaaaa",
"a.aa....",
"a..a....",
"a......."
};
static const char *Tab_xpm[] = {
"8 7 2 1",
"a c #000000",
". c None",
".......a",
"....a..a",
"....aa.a",
"aaaaaaaa",
"....aa.a",
"....a..a",
".......a"
};
static const char *Space_xpm[] = {
"9 9 2 1",
"a c #000000",
". c None",
"aaaaaaaaa",
"a.......a",
"a.......a",
"a.......a",
"a.......a",
"a.......a",
"a.......a",
"a.......a",
"aaaaaaaaa"
};

PickboardPicks::PickboardPicks(QWidget* parent, Qt::WFlags f ) :
    QFrame(parent,f)
{
}

void PickboardPicks::initialise(void)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
    mode = 0;

    DictFilterConfig* dc = new DictFilterConfig(this);
    QStringList sets_a = tr("ABC DEF GHI JKL MNO PQR STU VWX YZ-'").split(' ');
    QStringList sets = tr("ABCÀÁÂÃÄÅÆÇ DEFÐÈÉÊË GHIÌÍÎÏ JKL MNOÑÒÓÔÕÖØ PQRÞ STUßÙÚÛÜ VWX YZ-'Ýÿ").split(' ');
    for (QStringList::ConstIterator it = sets.begin(), it_a = sets_a.begin(); it!=sets.end(); ++it,++it_a)
	dc->addSet(*it_a,*it);
    dc->addMode("123");
    dc->addMode("@*!?");
    dc->addMode(tr("KEY"));
    dc->addMode(tr("Space"));
    dc->addMode(tr("Back"));
    dc->addMode(tr("Enter"));
    dc->addMode(tr("Shift"));
    configs.append(dc);

    CharStringConfig* number = new CharStringConfig(this);
    number->addChar("0");
    number->addChar("1");
    number->addChar("2");
    number->addChar("3");
    number->addChar("4");
    number->addChar("5");
    number->addChar("6");
    number->addChar("7");
    number->addChar("8");
    number->addChar("9");
    number->addChar("."); // #### or "," in some locales
    configs.append(number);

    CharConfig* punc = new CharConfig(this);

    punc->addChar(0,"\"");
    punc->addChar(0,"`");
    punc->addChar(0,"'");
    punc->addChar(0,"\253");
    punc->addChar(0,"\273");
    punc->addChar(0,"\277");
    punc->addChar(1,"(");
    punc->addChar(1,")");
    punc->addChar(1,"[");
    punc->addChar(1,"]");
    punc->addChar(1,"{");
    punc->addChar(1,"}");

    punc->addChar(0,"+");
    punc->addChar(0,"-");
    punc->addChar(0,"*");
    punc->addChar(0,"/");
    punc->addChar(0,"=");
    punc->addChar(0,"_");
    punc->addChar(0,"$");
    punc->addChar(0,"&");
    punc->addChar(1,"|");
    punc->addChar(1,"@");
    punc->addChar(1,"\\");
    punc->addChar(1,"#");
    punc->addChar(1,"^");
    punc->addChar(1,"~");
    punc->addChar(1,"<");
    punc->addChar(1,">");

    punc->addChar(0,".");
    punc->addChar(0,"?");
    punc->addChar(0,"!");
    punc->addChar(0,",");
    punc->addChar(0,";");
    punc->addChar(1,":");
    punc->addChar(1,"\267");
    punc->addChar(1,"\277");
    punc->addChar(1,"\241");
    punc->addChar(1,"\367");

    punc->addChar(0,"$");
    punc->addChar(0,"\242");
    punc->addChar(0,"\245");
    punc->addChar(1,"\243");
    punc->addChar(1,"\244");
    punc->addChar(1,"\260");

    configs.append(punc);

    KeycodeConfig* keys = new KeycodeConfig(this);
    keys->addKey(0,QPixmap(Esc_xpm),Qt::Key_Escape, 27);
    keys->addKey(0,QPixmap(BS_xpm),Qt::Key_Backspace, 8);
    keys->addGap(0,10);

    keys->addKey(0,QPixmap(Ins_xpm),Qt::Key_Insert);
    keys->addKey(0,QPixmap(Home_xpm),Qt::Key_Home);
    keys->addKey(0,QPixmap(PgUp_xpm),Qt::Key_PageUp);

    keys->addGap(0,25);
    keys->addKey(0,QPixmap(Up_xpm),Qt::Key_Up);
    keys->addGap(0,15);

    keys->addKey(1,QPixmap(BackTab_xpm),Qt::Key_Tab, 9);
    keys->addGap(1,3);
    keys->addKey(1,QPixmap(Tab_xpm),Qt::Key_Tab, 9);
    keys->addGap(1,10);

    keys->addKey(1,QPixmap(Del_xpm),Qt::Key_Delete);
    keys->addGap(1,2);
    keys->addKey(1,QPixmap(End_xpm),Qt::Key_End);
    keys->addGap(1,3);
    keys->addKey(1,QPixmap(PgDn_xpm),Qt::Key_PageDown);

    keys->addGap(1,10);
    keys->addKey(1,QPixmap(Left_xpm),Qt::Key_Left);
    keys->addKey(1,QPixmap(Down_xpm),Qt::Key_Down);
    keys->addKey(1,QPixmap(Right_xpm),Qt::Key_Right);

    keys->addGap(1,13);
    keys->addKey(1,QPixmap(Space_xpm),Qt::Key_Space, ' ');

    keys->addGap(0,10);
    keys->addKey(0,QPixmap(Enter_xpm),Qt::Key_Return, 13);

    configs.append(keys);
}

PickboardPicks::~PickboardPicks()
{
}

QSize PickboardPicks::sizeHint() const
{
    return QSize(240,fontMetrics().lineSpacing()*2+3);
}

void PickboardPicks::drawContents(QPainter* p)
{
    config()->draw(p);
}

void PickboardPicks::mousePressEvent(QMouseEvent* e)
{
    config()->pickPoint(e->pos(),true);
}

void PickboardPicks::mouseDoubleClickEvent(QMouseEvent* e)
{
    config()->pickPoint(e->pos(),true);
}

void PickboardPicks::mouseReleaseEvent(QMouseEvent* e)
{
    config()->pickPoint(e->pos(),false);
}

void PickboardPicks::setMode(int m)
{
    mode = m;
}

void PickboardPicks::resetState()
{
    config()->reset();
}
