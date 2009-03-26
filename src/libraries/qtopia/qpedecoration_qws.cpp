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

#include <qconfig.h>
#include <QPainter>
#include <QTimer>
#include <QWhatsThis>
#include <QMenu>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QDebug>

#include "qtopiaipcenvelope.h"
#include "qtopiaservices.h"
#include "qpedecoration_p.h"
#include "qtopiaapplication.h"
#include "qlibrary.h"
#include "qwindowdecorationinterface.h"
#include "qpluginmanager.h"
#include <qtopialog.h>

#include <stdlib.h>

#include "phonedecoration_p.h"

#ifndef QT_NO_QWS_QPE_WM_STYLE

class DecorHackWidget : public QWidget
{
public:
    bool needsOk() {
        return (inherits("QDialog") && !inherits("QMessageBox")
                          && !inherits("QWizard") );
    }
};

static QImage scaleButton( const QImage &img, int height )
{
    if ( img.height() != 0 && img.height() != height ) {
        return img.scaled( img.width()*height/img.height(), height );
    } else {
        return img;
    }
}

//===========================================================================

static QImage *okImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
        delete i;
        i = new QImage(scaleButton(QImage(":image/qpe/pda/OKButton"),th));
    }
    return i;
}

static QImage *closeImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
        delete i;
        i = new QImage(scaleButton(QImage(":image/qpe/pda/CloseButton"),th));
    }
    return i;
}

static QImage *helpImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
        delete i;
        i = new QImage(scaleButton(QImage(":image/qpe/pda/HelpButton"),th));
    }
    return i;
}

static QImage *maximizeImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
        delete i;
        i = new QImage(scaleButton(QImage(":image/qpe/pda/MaximizeButton"),th));
    }
    return i;
}

QWindowDecorationInterface::~QWindowDecorationInterface()
{
}

int QWindowDecorationInterface::metric( Metric m, const WindowData *wd ) const
{
    switch ( m ) {
        case TitleHeight: {
            QDesktopWidget *desktop = QApplication::desktop();
            if (desktop->screenGeometry(desktop->primaryScreen()).width() > 320 )
                return 19;
            else
                return 15;
        }
        case LeftBorder:
        case RightBorder:
        case TopBorder:
        case BottomBorder:
            return 4;
        case OKWidth:
            return okImage(metric(TitleHeight,wd))->width()+1;
        case CloseWidth:
            return closeImage(metric(TitleHeight,wd))->width()+1;
        case HelpWidth:
            return helpImage(metric(TitleHeight,wd))->width()+1;
        case MaximizeWidth:
            return maximizeImage(metric(TitleHeight,wd))->width()+1;
        case CornerGrabSize:
            return 16;
    }

    return 0;
}

void QWindowDecorationInterface::drawArea( Area a, QPainter *p, const WindowData *wd ) const
{
    int th = metric( TitleHeight, wd );
    QRect r = wd->rect;

    switch ( a ) {
        case Border:
            {
                QRect br(r.x()-metric(LeftBorder,wd),
                        r.y()-th-metric(TopBorder,wd),
                        r.width()+metric(LeftBorder,wd)+metric(RightBorder,wd),
                        r.height()+th+metric(TopBorder,wd)+metric(BottomBorder,wd));
                qDrawWinPanel(p, br.x(), br.y(), br.width(),
                        br.height(), wd->palette, false,
                        &wd->palette.brush(QPalette::Background));
            }
            break;
        case Title:
            {
                QBrush titleBrush;
                QPen   titleLines;

                if ( wd->flags & WindowData::Active ) {
                    titleBrush = wd->palette.brush(QPalette::Active, QPalette::Highlight);
                    titleLines = titleBrush.color().dark();
                } else {
                    titleBrush = wd->palette.brush(QPalette::Active, QPalette::Background);
                    titleLines = titleBrush.color();
                }

                p->fillRect( r.x(), r.y()-th, r.width(), th, titleBrush);

                p->setPen( titleLines );
                for ( int i = r.y()-th; i < r.y(); i += 2 )
                    p->drawLine( r.left(), i, r.right(), i );
            }
            break;
        case TitleText:
            p->drawText( r.x()+3+metric(HelpWidth,wd), r.top()-th,
                r.width()-metric(OKWidth,wd)-metric(CloseWidth,wd),
                th, Qt::AlignVCenter, wd->caption);
            break;
    }
}

void QWindowDecorationInterface::drawButton( Button b, QPainter *p, const WindowData *wd, int x, int y, int, int, QDecoration::DecorationState state ) const
{
    QImage *img = 0;
    switch ( b ) {
        case OK:
            img = okImage(metric(TitleHeight,wd));
            break;
        case Close:
            img = closeImage(metric(TitleHeight,wd));
            break;
        case Help:
            img = helpImage(metric(TitleHeight,wd));
            break;
        case Maximize:
            img = maximizeImage(metric(TitleHeight,wd));
            break;
    }

    if ( img ) {
        if ((state & QDecoration::Pressed))
            p->drawImage(x+2, y+2, *img);
        else
            p->drawImage(x+1, y+1, *img);
    }
}

QRegion QWindowDecorationInterface::mask( const WindowData *wd ) const
{
    int th = metric(TitleHeight,wd);
    QRect rect( wd->rect );
    QRect r(rect.left() - metric(LeftBorder,wd),
            rect.top() - th - metric(TopBorder,wd),
            rect.width() + metric(LeftBorder,wd) + metric(RightBorder,wd),
            rect.height() + th + metric(TopBorder,wd) + metric(BottomBorder,wd));
    return QRegion(r);
}

class DefaultWindowDecoration : public QWindowDecorationInterface
{
public:
    DefaultWindowDecoration() {}
    QString name() const {
        return qApp->translate("WindowDecoration", "Default",
                "List box text for default window decoration");
    }
    QPixmap icon() const {
        return QPixmap();
    }
};

static QWindowDecorationInterface *wdiface = 0;
static QPluginManager *wdLoader = 0;

//===========================================================================

/*!
    \class QtopiaDecoration
    \inpublicgroup QtBaseModule
    \internal

    Supports Qt Extended window decoration plugins.
*/
bool QtopiaDecoration::helpExists() const
{
    if ( helpFile.isNull() ) {
        QStringList helpPath = Qtopia::helpPaths();
        QFileInfo fi( QString(qApp->argv()[0]) );
        QString hf = fi.baseName() + ".html";
        bool he = false;
        for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end() && !he; ++it)
            he = QFile::exists( *it + "/" + hf );
        ((QtopiaDecoration*)this)->helpFile = hf;
        if ( QtopiaService::app("Help").isNull() )
            he = false;
        ((QtopiaDecoration*)this)->helpexists = he;
        return he;
    }
    return helpexists;
}

QtopiaDecoration::QtopiaDecoration()
    : QDecorationDefault()
{
    if ( wdLoader ) {
        delete wdLoader;
        wdLoader = 0;
    } else {
        delete wdiface;
    }
    wdiface = new PhoneDecoration;

    helpexists = false; // We don't know (flagged by helpFile being null)
    imageOk = QImage( ":image/qpe/pda/OKButton" );
    imageClose = QImage( ":image/qpe/pda/CloseButton" );
    imageHelp = QImage( ":image/qpe/pda/HelpButton" );
    QDesktopWidget *desktop = QApplication::desktop();
    desktopRect = desktop->screenGeometry(desktop->primaryScreen());
}

QtopiaDecoration::QtopiaDecoration( const QString & )
    : QDecorationDefault()
{
    if ( wdLoader ) {
        delete wdLoader;
        wdLoader = 0;
    } else {
        delete wdiface;
    }
    wdiface = new PhoneDecoration;
    helpexists = false; // We don't know (flagged by helpFile being null)
    QDesktopWidget *desktop = QApplication::desktop();
    desktopRect = desktop->screenGeometry(desktop->primaryScreen());
}

QtopiaDecoration::~QtopiaDecoration()
{
}


int QtopiaDecoration::getTitleHeight( const QWidget *w )
{
    QWindowDecorationInterface::WindowData wd;
    windowData( w, wd );
    return wdiface->metric(QWindowDecorationInterface::TitleHeight,&wd);
}

/*
    If rect is empty, no frame is added. (a hack, really)
*/
QRegion QtopiaDecoration::region(const QWidget *widget, const QRect &rect, int type)
{
    QWindowDecorationInterface::WindowData wd;
    windowData( widget, wd );
    wd.rect = rect;

    int titleHeight = wdiface->metric(QWindowDecorationInterface::TitleHeight,&wd);
    int okWidth = wdiface->metric(QWindowDecorationInterface::OKWidth,&wd);
    int closeWidth = wdiface->metric(QWindowDecorationInterface::CloseWidth,&wd);
    int helpWidth = wdiface->metric(QWindowDecorationInterface::HelpWidth,&wd);
    int grab = wdiface->metric(QWindowDecorationInterface::CornerGrabSize,&wd);

    bool rtl = QApplication::layoutDirection() == Qt::RightToLeft;

    QRegion region;

    Qt::WindowFlags wf = widget ? widget->windowFlags() : (Qt::WindowFlags)0;
    if (helpWidth > 0 && !helpExists())
        helpWidth = 0;

    switch ((int)type) {
        case Menu:
            break;
        case Maximize: {
            if (!widget->inherits("QDialog") && desktopRect.width() > 350) {
                int left;
                //not a dialog -> no ok button available
                if ( rtl ) {
                    left = rect.left() + closeWidth;
                } else {
                    int maximizeWidth = wdiface->metric(QWindowDecorationInterface::MaximizeWidth,&wd);
                    left = rect.right() - maximizeWidth - closeWidth;
                }
                QRect r(left, rect.top() - titleHeight, closeWidth, titleHeight);
                if ( (rtl && (r.right() < rect.right())) ||
                        ( !rtl && (r.left() > rect.left())) )
                    region = r;
            }
            break;
        }
        case Minimize:
            if ( ((DecorHackWidget *)widget)->needsOk() && okWidth > 0) {
                QRect r(rect.right() - okWidth,
                    rect.top() - titleHeight, okWidth, titleHeight);
                if ( rtl )
                    r.moveLeft( rect.left() );
                if (( !rtl && r.left() > rect.left() + helpWidth) ||
                        (rtl && (r.right() < rect.right() ) ))
                    region = r;
            }
            break;
        case Close:
            {
                int left;
                if ( rtl )
                    left = rect.left();
                else
                    left = rect.right() - closeWidth;
                if ( ((DecorHackWidget *)widget)->needsOk() )
                {
                    if ( rtl )
                        left += okWidth;
                    else
                        left -= okWidth;
                }
                QRect r(left, rect.top() - titleHeight, closeWidth, titleHeight);
                if ((!rtl && r.left() > rect.left() ) ||
                     (rtl && r.right() < rect.right()))
                    region = r;
            }
            break;
        case Title:
            if ( titleHeight || !widget->isMaximized() ) {
                int width = rect.width() - helpWidth - closeWidth;
                if ( ((DecorHackWidget *)widget)->needsOk() )
                    width -= okWidth;
                if (!widget->inherits("QDialog") && desktopRect.width() > 350) {
                    width -= wdiface->metric(QWindowDecorationInterface::MaximizeWidth,&wd);
                }
                QRect r(rect.left()+helpWidth, rect.top() - titleHeight,
                        width, titleHeight);
                if ( rtl ) {
                    r.moveRight( rect.right() - helpWidth);
                }
                if (r.width() > 0)
                    region = r;
            }
            break;
        case Help:
            if ( helpWidth>0 || (wf & Qt::WindowContextHelpButtonHint) == Qt::WindowContextHelpButtonHint ) {
                QRect r(rect.left(), rect.top() - titleHeight,
                          helpWidth, titleHeight);
                if ( rtl )
                    r.moveLeft( rect.right() - helpWidth );
                if ( (rtl && r.left() > rect.left()) ||
                        ( !rtl && r.right() <  rect.right() ) )
                    region = r;
            }
            break;
        case Top:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                int b = wdiface->metric(QWindowDecorationInterface::TopBorder,&wd);
                region = m & QRect( br.left()+grab, br.top(),
                                    br.width()-2*grab, b );
            }
            break;
        case Left:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                int b = wdiface->metric(QWindowDecorationInterface::LeftBorder,&wd);
                region = m & QRect( br.left(), br.top()+grab,
                                    b, br.height()-2*grab );
            }
            break;
        case Right:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                int b = wdiface->metric(QWindowDecorationInterface::RightBorder,&wd);
                region = m & QRect( rect.right(), br.top()+grab,
                                    b, br.height()-2*grab );
            }
            break;
        case Bottom:
            {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                int b = wdiface->metric(QWindowDecorationInterface::BottomBorder,&wd);
                region = m & QRect( br.left()+grab, rect.bottom(),
                                    br.width()-2*grab, b );
            }
            break;
        case TopLeft:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                int tb = wdiface->metric(QWindowDecorationInterface::TopBorder,&wd);
                int lb = wdiface->metric(QWindowDecorationInterface::LeftBorder,&wd);
                QRegion crgn( br.left(), br.top(), grab, tb );
                crgn |= QRect( br.left(), br.top(), lb, grab );
                region = m & crgn;
            }
            break;
        case TopRight:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                int tb = wdiface->metric(QWindowDecorationInterface::TopBorder,&wd);
                int rb = wdiface->metric(QWindowDecorationInterface::RightBorder,&wd);
                QRegion crgn( br.right()-grab, br.top(), grab, tb );
                crgn |= QRect( br.right()-rb, br.top(), rb, grab );
                region = m & crgn;
            }
            break;
        case BottomLeft:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                region = m & QRect( br.left(), br.bottom()-grab, grab, grab );
            }
            break;
        case BottomRight:
            if ( !widget->isMaximized() ) {
                QRegion m = wdiface->mask(&wd);
                QRect br = m.boundingRect();
                region = m & QRect( br.right()-grab, br.bottom()-grab, grab, grab );
            }
            break;
        case All:
            region = wdiface->mask(&wd);
            region -= rect;
            break;
        default:
            region = QDecorationDefault::region(widget, rect, type);
            break;
    }

    return region;
}

// This is how we stop the mouse from interacting with
// windows.  regionAt() is used to determine what area the mouse has been
// clicked.  By returning QDecoration::None no mouse action will be performed.
int QtopiaDecoration::regionAt(const QWidget * /*w*/, const QPoint & /*point*/)
{
    return QDecoration::None;
}

bool QtopiaDecoration::paint(QPainter *painter, const QWidget *widget, int decRegion, DecorationState state)
{
    QWindowDecorationInterface::WindowData wd;
    windowData( widget, wd );

    bool paintAll = (decRegion == int(All));
    bool handled = false;

//    int titleWidth = getTitleWidth(widget);
    const QRect titleRect = region(widget, widget->rect(), Title).boundingRect();
    int titleHeight = wdiface->metric(QWindowDecorationInterface::TitleHeight,&wd);

    QRect rect(widget->rect());

    // title bar rect
    QRect tbr( rect.left(), rect.top() - titleHeight, rect.width(), titleHeight );

#ifndef QT_NO_PALETTE
    if (paintAll || decRegion & Borders) {
        int lb = wdiface->metric(QWindowDecorationInterface::LeftBorder,&wd);
        int rb = wdiface->metric(QWindowDecorationInterface::RightBorder,&wd);
        int bb = wdiface->metric(QWindowDecorationInterface::BottomBorder,&wd);
        if (titleHeight > 0 || lb > 0 || rb > 0 || bb > 0)
            wdiface->drawArea( QWindowDecorationInterface::Border, painter, &wd );
        handled |= true;
    }

    if ((paintAll || decRegion & Title) && titleRect.width() > 0 && titleRect.height() > 0) {
        const QPalette &pal = widget->palette();
        QBrush titleBrush;
        QPen   titlePen;

        if ( wd.flags & QWindowDecorationInterface::WindowData::Active ) {
            titleBrush = pal.brush(QPalette::Highlight);
            titlePen   = pal.color(QPalette::HighlightedText);
        } else {
            titleBrush = pal.brush(QPalette::Background);
            titlePen   = pal.color(QPalette::Text);
        }

        wdiface->drawArea( QWindowDecorationInterface::Title, painter, &wd );

        // Draw caption
        painter->setPen(titlePen);
        QFont f( QApplication::font() );
        f.setWeight( QFont::Bold );
        painter->setFont(f);
        wdiface->drawArea( QWindowDecorationInterface::TitleText, painter, &wd );
        handled |= true;
    }
#endif //QT_NO_PALETTE

    if (paintAll || decRegion & Help) {
        paintButton( painter, widget, Help, state );
        handled |= true;
    }
    if (paintAll || decRegion & Close) {
        paintButton( painter, widget, Close, state );
        handled |= true;
    }
    if (paintAll || decRegion & Minimize) {
        paintButton( painter, widget, Minimize, state );
        handled |= true;
    }
    if (paintAll || decRegion & Maximize) {
        paintButton( painter, widget, Maximize, state );
        handled |= true;
    }

    return handled;
}

void QtopiaDecoration::paintButton(QPainter *painter, const QWidget *w,
                        int type, int state)
{
    QWindowDecorationInterface::WindowData wd;
    windowData( w, wd );

    int helpWidth = wdiface->metric(QWindowDecorationInterface::HelpWidth,&wd);
    QWindowDecorationInterface::Button b;
    switch ((int)type) {
        case Close:
            b = QWindowDecorationInterface::Close;
            break;
        case Minimize:
            if ( ((DecorHackWidget *)w)->needsOk() )
                b = QWindowDecorationInterface::OK;
            else if ( helpWidth > 0 && helpExists() )
                b = QWindowDecorationInterface::Help;
            else
                return;
            break;
        case Help:
            if ( helpWidth > 0 && helpExists() )
                b = QWindowDecorationInterface::Help;
            else
                return;
            break;
        case Maximize:
            b = QWindowDecorationInterface::Maximize;
            break;
        default:
            return;
    }

    int titleHeight = wdiface->metric(QWindowDecorationInterface::TitleHeight,&wd);
    QRect rect(w->rect());
    QRect tbr( rect.left(), rect.top() - titleHeight, rect.width(), titleHeight );
    QRect brect(region(w, w->rect(), type).boundingRect());

    if (brect.width() > 0 && brect.height() > 0) {
        const QPalette &pal = w->palette();
        if ( wd.flags & QWindowDecorationInterface::WindowData::Active )
            painter->setPen( pal.color(QPalette::HighlightedText) );
        else
            painter->setPen( pal.color(QPalette::Text) );

        QRegion oldClip = painter->clipRegion();
        painter->setClipRegion( QRect(brect.x(), tbr.y(), brect.width(), tbr.height()) ); // reduce flicker
        wdiface->drawArea( QWindowDecorationInterface::Title, painter, &wd );
        wdiface->drawButton( b, painter, &wd, brect.x(), brect.y(), brect.width(), brect.height(), (QDecoration::DecorationState)state );
        painter->setClipRegion( oldClip );
    }
}

void QtopiaDecoration::buildSysMenu( QWidget *widget, QMenu *menu )
{
    Q_UNUSED(widget)
    menu->addAction(new QDecorationAction("Close", menu, Close));
}

#ifndef QT_NO_DIALOG
class HackDialog : public QDialog
{
public:
    void acceptIt() {
        if ( qobject_cast<QMessageBox*>(this) )
            qApp->postEvent( this, new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter,Qt::NoModifier) );
        else
            accept();
    }
};
#endif


void QtopiaDecoration::regionClicked(QWidget *widget, int region)
{
    switch (region) {
        case Maximize:
            {
                // find out how much space the decoration needs
                QWindowDecorationInterface::WindowData wd;
                windowData(widget, wd);

                int th = wdiface->metric(QWindowDecorationInterface::TitleHeight, &wd);

                QDesktopWidget *desktop = QApplication::desktop();
                QRect nr = desktop->availableGeometry(desktop->screenNumber(widget));
                nr.setTop(nr.top()+th);
                widget->setGeometry(nr);
            }
            break;

        case Minimize:
#ifndef QT_NO_DIALOG
            // We use the minimize button as an "accept" button.
            if ( widget->inherits( "QDialog" ) ) {
                HackDialog *d = (HackDialog *)widget;
                d->acceptIt();
            }
#endif
            else if ( ((DecorHackWidget *)widget)->needsOk() ) {
                QTimer::singleShot(0, widget, SLOT(accept()));
            } else {
                help( widget );
            }
            break;

        case Help:
            help(widget);
            break;

        default:
            QDecorationDefault::regionClicked(widget, region);
    }
}

void QtopiaDecoration::help(QWidget *w)
{
    if ( helpExists() ) {
        QString hf = helpFile;
        QFileInfo fi( QString(qApp->argv()[0]) );
        QString localHelpFile = fi.baseName() + "-" + w->objectName() + ".html";
        QStringList helpPath = Qtopia::helpPaths();
        for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
            if ( QFile::exists( *it + "/" + localHelpFile ) ) {
                hf = localHelpFile;
                break;
            }
        }
        qLog(Help) << ">>> Using help " << hf << "<<<";
        {
            QtopiaServiceRequest env("Help", "setDocument(QString)");
            env << hf;
            env.send();
        }
    } else if ( w && (w->windowFlags() & Qt::WindowContextHelpButtonHint) == Qt::WindowContextHelpButtonHint ) {
        QWhatsThis::enterWhatsThisMode();
        QWhatsThis::showText( QCursor::pos(), qApp->translate("QtopiaDecoration",
                    "<Qt>Comprehensive help is not available for this application, "
                    "however there is context-sensitive help.<p>To use context-sensitive help:<p>"
                    "<ol><li>click and hold the help button."
                    "<li>when the title bar shows <b>What's this...</b>, "
                    "click on any control.</ol></Qt>" ) );
        QWhatsThis::leaveWhatsThisMode();
    }
}

void QtopiaDecoration::windowData( const QWidget *w, QWindowDecorationInterface::WindowData &wd ) const
{
    wd.window = w;
    wd.rect = w->rect();
    wd.caption = w->windowTitle();
    wd.palette = qApp->palette();
    wd.flags = 0;
    wd.flags |= w->isMaximized() ? QWindowDecorationInterface::WindowData::Maximized : 0;
    wd.flags |= (w->windowType() == Qt::Dialog) ? QWindowDecorationInterface::WindowData::Dialog : 0;
    const QWidget *active = qApp->activeWindow();
    wd.flags |= w == active ? QWindowDecorationInterface::WindowData::Active : 0;
    wd.reserved = 1;
}

/*
#ifndef QT_NO_POPUPMENU
QMenu *QtopiaDecoration::menu(QWSManager*, const QWidget*, const QPoint&)
{
    return 0;
}
#endif
*/




#endif // QT_NO_QWS_QPE_WM_STYLE
