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
#include "picker.h"
#include <QSoftMenuBar>
#include <private/contextkeymanager_p.h>
#include <QtopiaIpcEnvelope>

#include <QPainter>
#include <QScrollBar>
#include <QStyleOptionFocusRect>
#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QDebug>

/* hack to let us into the functions, as it has no data, cast to parent is safe */
class PickerScrollBar : public QScrollBar
{
public:
    PickerScrollBar( QWidget *parent )
	: QScrollBar( parent ) {}
    PickerScrollBar( Qt::Orientation o, QWidget *parent )
	: QScrollBar( o, parent ) {}

    void passMousePressEvent(QMouseEvent *e)
    {
	QScrollBar::mousePressEvent(e);
    }
    void passMouseMoveEvent(QMouseEvent *e)
    {
	QScrollBar::mouseMoveEvent(e);
    }
    void passMouseReleaseEvent(QMouseEvent *e)
    {
	QScrollBar::mouseReleaseEvent(e);
    }
};

Picker::Picker(QWidget *parent) :
    QAbstractScrollArea(parent),
    grabber(None), nCols(1), nRows(1), cHeight(10), cWidth(10),
    selRow(0), selCol(0), havePress(false)
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setFrameStyle(StyledPanel | Raised);
    setWindowTitle(QLatin1String("_allow_on_top_"));
    viewport()->setBackgroundRole( QPalette::Base );

    setContentsMargins(2,2,2,2);

    updateContentsSize();
}

Picker::~Picker()
{
}

void Picker::setNumRows(int n)
{
    nRows = n;
    updateContentsSize();
}

void Picker::setNumCols(int n)
{
    nCols = n;
    updateContentsSize();
}

void Picker::setCellHeight(int h)
{
    cHeight = h;
    updateContentsSize();
}

void Picker::setCellWidth(int w)
{
    cWidth = w;
    updateContentsSize();
}

void Picker::updateContentsSize()
{
    int contentsWidth = cWidth*nCols;
    int contentsHeight = cHeight*nRows;
    viewport()->resize(contentsWidth, contentsHeight);

    int sBarE = style()->pixelMetric(QStyle::PM_ScrollBarExtent);

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desk(desktop->availableGeometry());

    int w = desk.width() - sBarE - 2*frameWidth();
    int h = desk.height() - sBarE - 2*frameWidth();
    h = qMin(desk.height()/2, h); // height is special.

    bool needHScroll = contentsWidth-cWidth > w;
    bool needVScroll = contentsHeight-cHeight > h;

    verticalScrollBar()->setRange(0, needVScroll ? contentsHeight-h : 0);
    horizontalScrollBar()->setRange(0, needHScroll ? contentsWidth-w : 0);

    if (needHScroll && !needVScroll) {
	h = contentsHeight+sBarE;
    } else if (needVScroll && !needHScroll) {
	w = contentsWidth+sBarE;
    } else if (!needVScroll && !needHScroll) {
	w = contentsWidth;
	h = contentsHeight;
    }

    resize(w+frameWidth()*2, h+frameWidth()*2);
}

bool Picker::filterKey(int, int keycode, int,
                            bool isPress, bool)
{
    if (Qt::Key_Hangup == keycode)
        return false;

    if (isPress) {
	havePress = TRUE;
    } else if (havePress) {
	havePress = FALSE;
	int oldSelRow = selRow;
	int oldSelCol = selCol;
	switch (keycode) {
	    case Qt::Key_Back:
		return FALSE;
	    case Qt::Key_Select:
		hide();
		emit cellClicked(selRow, selCol);
		return TRUE;
	    case Qt::Key_Up:
		selRow--;
		if (selRow < 0)
		    selRow = nRows-1;
		break;
	    case Qt::Key_Down:
		selRow++;
		if (selRow >= nRows)
		    selRow = 0;
		break;
	    case Qt::Key_Left:
		selCol--;
		if (selCol < 0)
		    selCol = nCols-1;
		break;
	    case Qt::Key_Right:
		selCol++;
		if (selCol >= nCols)
		    selCol = 0;
		break;
	}

	if (oldSelRow != selRow || oldSelCol != selCol) {

	    //ensureVisible(...);
            int x = selCol*cWidth+cWidth/2;
            int y = selRow*cHeight+cHeight/2;
            int xmargin = cWidth/2;
            int ymargin = cHeight/2;
            int logicalX = QStyle::visualPos(layoutDirection(), viewport()->rect(), QPoint(x, y)).x();

            if (logicalX - xmargin < horizontalScrollBar()->value()) {
                horizontalScrollBar()->setValue(qMax(0, logicalX - xmargin));
            } else if (logicalX > horizontalScrollBar()->value() + viewport()->width() - xmargin) {
                horizontalScrollBar()->setValue(qMin(logicalX - viewport()->width() + xmargin, horizontalScrollBar()->maximum()));
            }

            if (y - ymargin < verticalScrollBar()->value()) {
                verticalScrollBar()->setValue(qMax(0, y - ymargin));
            } else if (y > verticalScrollBar()->value() + verticalScrollBar()->height() - ymargin) {
                verticalScrollBar()->setValue(qMin(y - verticalScrollBar()->height() + ymargin, verticalScrollBar()->maximum()));
            }

            viewport()->update();
	}
    }

    return TRUE;

    if (keycode == Qt::Key_Space) {
	hide();
	return true;
    }
    return false;
}

bool Picker::filterMouse(const QPoint &point, int buttons, int)
{
    static bool wasMousePress = false;

    // XXX lp may be out, may have to use different value for offset.
    QPoint gp = point;
    QPoint lp = gp-pos();

    PickerScrollBar *vsb = (PickerScrollBar *)verticalScrollBar();
    PickerScrollBar *hsb = (PickerScrollBar *)horizontalScrollBar();

    if (buttons & Qt::LeftButton) {
	// is either press, or hold
	if (wasMousePress) {

	    switch (grabber) {
		default:
		    break;
		case OutSide:
		    if (lp.x() < 0 || lp.y() < 0
			    || lp.x() > width() || lp.y() > height())
			grabber = None;
		    break;
		case VScroll:
		    {
			QMouseEvent ce(QEvent::MouseMove, vsb->mapFrom(this, lp),
				gp, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

			vsb->passMouseMoveEvent(&ce);
		    }
		    break;
		case HScroll:
		    {
			QMouseEvent ce(QEvent::MouseMove, hsb->mapFrom(this, lp),
				gp, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

			hsb->passMouseMoveEvent(&ce);
		    }
		    break;
	    }
	} else {
	    wasMousePress = true;

	    if ( vsb->isVisible() && vsb->frameGeometry().contains(vsb->mapFrom(this, lp)) ) {
		QMouseEvent ce(QEvent::MouseButtonRelease, vsb->mapFrom(this, lp),
			gp, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

		vsb->passMousePressEvent(&ce);
		grabber = VScroll;
	    } else if ( hsb->isVisible() && hsb->frameGeometry().contains(hsb->mapFrom(this, lp)) ) {
		QMouseEvent ce(QEvent::MouseButtonRelease, hsb->mapFrom(this, lp),
			gp, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

		hsb->passMousePressEvent(&ce);
		grabber = HScroll;
	    } else if (frameGeometry().contains(gp)) {
		grabber = View;
	    } else {
		grabber = OutSide;
	    }


	}
    } else if (wasMousePress) {
	wasMousePress = false;

	switch (grabber) {
	    default:
		break;
	    case OutSide:
		if (lp.x() < 0 || lp.y() < 0
			|| lp.x() > width() || lp.y() > height()) {
		    hide();
		}
		break;
	    case VScroll:
		{
		    QMouseEvent ce(QEvent::MouseButtonPress, vsb->mapFrom(this, lp),
			    gp, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

		    vsb->passMouseReleaseEvent(&ce);
		}
		break;
	    case HScroll:
		{
		    QMouseEvent ce(QEvent::MouseButtonPress, hsb->mapFrom(this, lp),
			    gp, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

		    hsb->passMouseReleaseEvent(&ce);
		}
		break;
	    case View:
		{
		    QPoint contentsPoint = viewport()->mapFrom(this, lp);
		    int xoffset = horizontalScrollBar()->value();
		    int yoffset = verticalScrollBar()->value();
		    contentsPoint += QPoint(xoffset, yoffset);

		    int row, col;
		    col = contentsPoint.x() / cWidth;
		    row = contentsPoint.y() / cHeight;

		    hide();
		    if (row >= 0 && row < nRows && col >= 0 && col < nCols)
			emit cellClicked(row, col);
		}
		break;
	}
	grabber = None;

    }

    return true;
}

void Picker::setMicroFocus( int, int y )
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desk(desktop->availableGeometry());
    if (y < desk.y())
        y = desk.y()+2;
    QFontMetrics fm(QApplication::font());
    if (y < (desk.height()-fm.height())/2) {
        move((desk.width()-width())/2, desk.y()+y+fm.height());
    } else {
        move((desk.width()-width())/2, desk.y()+y-2-height());
    }
}

void Picker::drawFrame(QPainter *p)
{
    QAbstractScrollArea::drawFrame(p);

    QStyleOptionFocusRect option;

    option.init(this);

    int ml, mr, mt, mb;
    getContentsMargins(&ml, &mt, &mr, &mb);

    option.rect = QRect(frameWidth()-ml,
	    frameWidth()-mt,
	    width()-frameWidth()*2+ml+mr,
	    height()-frameWidth()*2+mt+mb);

    option.backgroundColor = palette().base().color();

    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, p, this);
}

void Picker::paintEvent(QPaintEvent* event)
{
    //int xoffset = horizontalScrollBar()->value();
    int yoffset = verticalScrollBar()->value();

    QPainter painter(viewport());

    painter.setPen(palette().text().color());
    painter.setBrush(palette().base());

    int cy = event->rect().y()+yoffset;
    int ch = event->rect().height();

    int row = cy/cHeight;
    int y = row*cHeight;
    while (y < cy+ch && row < nRows) {
        if (row != 0)
            painter.drawLine( 0, y-cy, nCols*cWidth, y-cy);
	for ( int i = 0; i < nCols; i++ ) {
	    bool sel = ( row == selRow && i == selCol );
	    // Translate painter and draw the cell
	    int colp = i * cWidth; 
	    int rowp = row * cHeight;
	    painter.save();
	    painter.translate( colp, rowp-yoffset );
	    drawCell( &painter, row, i, sel );
	    painter.restore();

	}
	// fill in the little bit of space at the end.
	painter.fillRect(nCols*cWidth, y, cWidth, cHeight, palette().base());
	row++;
	y += cHeight;
    }
}

void Picker::showEvent(QShowEvent *e)
{
    {
        // Force the soft menu bar to show what we want rather than whatever
        // the active window wants to show.
        // Don't do this at home folks.
        QtopiaIpcEnvelope e1( "QPE/QSoftMenuBar", "setActiveOverride(int)" );
        e1 << int(winId());
    }

    ContextKeyManager *mgr = ContextKeyManager::instance();
    mgr->setStandard(this, Qt::Key_Select, QSoftMenuBar::Select);
    mgr->setStandard(this, Qt::Key_Back, QSoftMenuBar::Cancel);
    mgr->clearLabel(this, QSoftMenuBar::menuKey());

    QAbstractScrollArea::showEvent(e);
}

void Picker::hideEvent(QHideEvent *e)
{
    // Restore soft menu bar.
    QtopiaIpcEnvelope e1( "QPE/QSoftMenuBar", "clearActiveOverride()" );
    e1 << int(winId());

    QAbstractScrollArea::hideEvent(e);
}

