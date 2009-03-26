/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** WARNING: Use of this file may require additional third party patent licensing.
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

#include "symbolpicker.h"
#include <qtopiaipcenvelope.h>
#include <qtranslatablesettings.h>
#include <qtopiaapplication.h>

#include <qapplication.h>
#include <qtimer.h>
#include <qstyle.h>
#include <QPainter>
#include <QDesktopWidget>

#define cr_width 12
#define cr_height 12
static unsigned char cr_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x08, 0x02,
    0x0c, 0x02, 0xfe, 0x03, 0x0c, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 };


SymbolPicker::SymbolPicker(QWidget *parent)
    : Picker(parent)
{
    QTranslatableSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
    cfg.beginGroup("IMethod"); // No tr

    int charsPerRow = cfg.value("picker_width",13).toInt();
    setNumCols(charsPerRow);
    chars = cfg.value("picker_chars").toString();
    if ( chars.isEmpty() ) {
	// Default: toLatin1 non-letter printable characters, and newline
	int ch;
	chars += QChar('\n');
	for (ch='!'; ch<='/'; ch++) chars += QChar(ch);
	for (ch=':'; ch<='@'; ch++) chars += QChar(ch);
	for (ch='['; ch<='`'; ch++) chars += QChar(ch);
	for (ch='{'; ch<='~'; ch++) chars += QChar(ch);
	for (ch=161; ch<=191; ch++) chars += QChar(ch);
	chars += QChar(215);
	chars += QChar(247);
	chars += QChar(0x20A6);
	chars += QChar(0x20A9);
	chars += QChar(0x20AC);
	chars += QChar(0x2022);
    }
    QFont fnt(font());
    int fontPointSize = cfg.value("picker_font_point_size",-1).toInt();

    int cw=0;
    bool pointSizeManuallySet = fontPointSize != -1;
    // No size for font set in defaultbuttons.conf, so choose a sensible 
    // default by taking the current font, and shrinking it until the 
    // symbolpicker will fit on the desktop.
    fontPointSize = fnt.pointSize();

    do {
        if ( cw ) // not first time round
            if(pointSizeManuallySet) 
                break;
            else
                fontPointSize -= 2;
        fnt.setPointSize(fontPointSize);
        QFontMetrics fm(fnt);
        cw = 1;
        for ( int i=0; i<(int)chars.length(); i++ ) {
            int w = fm.width(chars[i]);
            if ( w > cw ) cw = w;
        }
    } while ( (cw+2) * charsPerRow > qApp->desktop()->width() && fnt.pointSize() > 6 );

    setFont(fnt);

    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() + 1 );
    setCellWidth( cw + 2 );

    setNumRows((chars.length()+charsPerRow-1)/charsPerRow);

    symbols = new QChar* [numRows()];
    int ch=0;
    for (int i = 0; i < numRows(); i++) {
	symbols[i] = new QChar [charsPerRow];
	for (int c = 0; c < charsPerRow; c++)
	    symbols[i][c] = (ch < (int)chars.length()) ? QChar(chars[ch++]) : QChar();
    }

    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(sendSymbolForPos(int,int)));
}

SymbolPicker::~SymbolPicker()
{
    for (int i = 0; i < numRows(); i++)
	delete [] symbols[i];
    delete [] symbols;
}

// do press and release for proper click handling.
void SymbolPicker::sendSymbolForPos(int row, int col)
{
    // TODO also send Qt::Key_...
    if (!symbols[row][col].isNull())
	emit symbolClicked(symbols[row][col].unicode(), 0);
}

void SymbolPicker::drawCell(QPainter *p, int row, int col, bool selected)
{
    QChar u;
    QFontMetrics fm(font());
    p->setBrush(Qt::SolidPattern);
    if ( selected ) {
	p->setPen( palette().highlightedText().color() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), palette().highlight());
    } else {
	p->setPen( palette().text().color() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), palette().base() );
    }
    if (symbols[row][col] == '\n') {
	QBitmap bm = QBitmap::fromData(QSize(cr_width, cr_height), cr_bits);
	bm.setMask(bm);
	p->drawPixmap((cellWidth()-bm.width())/2,
		(cellHeight()-bm.height())/2, bm);
    } else {
	u = QChar(symbols[row][col]);
	if (!u.isNull() && fm.inFont(u))
	    p->drawText(0, 0, cellWidth(), cellHeight(), Qt::AlignCenter, QString(u));
    }
}



