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

#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>
#include <char.h>
#include "pensettingswidget.h"

#include <QMouseEvent>

#define TITLE_WIDTH     30  // ### magic

/*!
  \class QIMPenSettingsWidget
  \inpublicgroup QtInputMethodsModule
  \brief The QIMPenSettingsWidget class provides an character input panel.

  Draws characters and allows input of characters.

  \ingroup qtopiahandwriting
*/

QIMPenSettingsWidget::QIMPenSettingsWidget( QWidget *parent, const char *name )
 : QWidget( parent )
{
    setObjectName( name );
    // charSets.setAutoDelete( true );
    inputStroke = 0;
    outputChar = 0;
    outputStroke = 0;
    mode = Waiting;
    currCharSet = 0;
    readOnly = false;
    // strokes.setAutoDelete( true );

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(timeout()));

    setBackgroundRole( QPalette::Base );
    /*
    setBackgroundColor( qApp->palette().color( QPalette::Active,
                                               QPalette::Base ) );
                                               */
    strokeColor = palette().color(QPalette::Text);
    setFixedHeight( 75 );
}

QIMPenSettingsWidget::~QIMPenSettingsWidget()
{
    while ( charSets.count() ) delete charSets.takeLast();
    while ( strokes.count() ) delete strokes.takeLast();
}

void QIMPenSettingsWidget::clear()
{
    timer->stop();
    mode = Waiting;
    QRect r( dirtyRect );
    QIMPenStrokeIterator it = strokes.begin();
    while ( it != strokes.end() ) {
        r |= (*it)->boundingRect();
        ++it;
    }
    outputChar = 0;
    outputStroke = 0;
    strokes.clear();
    if ( !r.isNull() ) {
        r.moveTopLeft( r.topLeft() - QPoint( 2, 2 ));
        r.setSize( r.size() + QSize( 4, 4 ) );
        repaint( r );
    } else {
        repaint();
    }
}

void QIMPenSettingsWidget::removeStroke()
{
    QRect r( dirtyRect );
    QIMPenStroke *st = strokes.count() ? strokes[0] : 0;
    QRect strokeRect;
    if ( st )
       strokeRect = st->boundingRect();
    r |= strokeRect;
    strokes.removeFirst();
    if ( !r.isNull() ) {
        r.moveTopLeft( r.topLeft() - QPoint( 2, 2 ));
        r.setSize( r.size() + QSize( 4, 4 ) );
        repaint( r );
    }
}

void QIMPenSettingsWidget::greyStroke()
{
    QRect r( dirtyRect );
    QIMPenStroke *st = strokes.count() ? strokes[0] : 0;
    QRect strokeRect;
    if ( st )
       strokeRect = st->boundingRect();
    r |= strokeRect;
    strokeColor.setAlpha(128);
    if ( !r.isNull() ) {
        r.moveTopLeft( r.topLeft() - QPoint( 2, 2 ));
        r.setSize( r.size() + QSize( 4, 4 ) );
        repaint( r );
    }
    strokeColor.setAlpha(255);
}

/*
  Insert a character set into the list.
*/
void QIMPenSettingsWidget::insertCharSet( QIMPenCharSet *cs, int stretch, int pos )
{
    CharSetEntry *e = new CharSetEntry;
    e->cs = cs;
    e->stretch = stretch;
    if ( pos < 0 )
        pos = charSets.count();
    charSets.insert( pos, e );
    currCharSet = 0;
    emit changeCharSet( currCharSet );
    emit changeCharSet( charSets.at(currCharSet)->cs );
    totalStretch = 0;
    CharSetEntryIterator it = charSets.begin();
    for ( ; it != charSets.end(); ++it )
        totalStretch += (*it)->stretch;
    update();
}

/*
  Remove a character set from the list.
*/
void QIMPenSettingsWidget::removeCharSet( int pos )
{
    if ( pos >= 0 && pos < (int)charSets.count() ) {
        charSets.removeAt( pos );
        currCharSet = 0;
        if ( charSets.count() ) {
            emit changeCharSet( currCharSet );
            emit changeCharSet( charSets.at(currCharSet)->cs );
        }
        totalStretch = 0;
        CharSetEntryIterator it = charSets.begin();
        for ( ; it != charSets.end(); ++it )
            totalStretch += (*it)->stretch;
        update();
    }
}

void QIMPenSettingsWidget::changeCharSet( QIMPenCharSet *cs, int pos )
{
    if ( pos >= 0 && pos < (int)charSets.count() ) {
        CharSetEntry *e = new CharSetEntry;
        e->cs = cs;
        e->stretch = charSets.at(pos)->stretch;
        delete charSets.takeAt( pos );
        charSets.insert( pos, e );
        if ( pos == currCharSet ) {
            emit changeCharSet( charSets.at(currCharSet)->cs );
        }
        update();
    }
}

void QIMPenSettingsWidget::clearCharSets()
{
    charSets.clear();
    while ( charSets.count() )
        delete charSets.takeLast();
    currCharSet = 0;
    update();
}

/*
  Display a character. \a speed determines how quickly the character is
  drawn.
*/
void QIMPenSettingsWidget::showCharacter( QIMPenChar *ch, int speed )
{
    outputChar = 0;
    outputStroke = 0;
    strokes.clear();
    penMoves.clear();
    mode = Output;
    repaint();
    if ( !ch || ch->isEmpty() ) {
        mode = Waiting;
        return;
    }

    outputChar = ch;
    outputStroke = outputChar->penStrokes()[0];
    if ( speed < 0 ) speed = 0;
    if ( speed > 20 ) speed = 20;
    speed = 50 - speed;
    pointIndex = 0;
    strokeIndex = 0;
    lastPoint = outputStroke->startingPoint();
    QRect br( outputChar->boundingRect() );
    lastPoint.setX( (width() - br.width()) / 2 + (lastPoint.x () - br.left()) );
    QPoint offset = lastPoint - outputStroke->startingPoint();
    br.moveTopLeft( br.topLeft() + QPoint( offset.x(), offset.y() ));
    dirtyRect |= br;
    timer->start( speed );
}

/*!
  Handle drawing/clearing of characters.
*/
void QIMPenSettingsWidget::timeout()
{
    if ( mode == Output )
    {
        const QVector<QIMPenGlyphLink> &chain = outputStroke->chain();
        if ( pointIndex < chain.count() ) {
            // QPainter paint( this );
            // paint.setBrush( Qt::black );
            for ( unsigned i = 0; i < 3 && pointIndex < chain.count(); i++ ) {
                lastPoint.rx() += chain[(int)pointIndex].dx;
                lastPoint.ry() += chain[(int)pointIndex].dy;
                pointIndex++;
                // paint.drawRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 );
                penMoves.append( QRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 ));
            }
            if ( penMoves.count() ) repaint();
        }
        if ( pointIndex >= chain.count() ) {
            QIMPenStrokeList strokes = outputChar->penStrokes();
            if ( strokeIndex < (int)strokes.count() - 1 ) {
                pointIndex = 0;
                strokeIndex++;
                outputStroke = strokes.at( strokeIndex );
                lastPoint = outputChar->startingPoint();
                QRect br( outputChar->boundingRect() );
                lastPoint.setX( (width() - br.width()) / 2
                                + (lastPoint.x () - br.left()) );
                QPoint off = lastPoint - outputChar->startingPoint();
                lastPoint = outputStroke->startingPoint() + off;
            } else {
                timer->stop();
                mode = Waiting;
            }
        }
    } else if ( mode == Waiting ) {
        QRect r( dirtyRect );
        if ( !r.isNull() ) {
            r.moveTopLeft( r.topLeft() - QPoint( 2, 2 ));
            r.setSize( r.size() + QSize( 4, 4 ) );
            repaint( r );
        }
    }
}

/*!
  If the point \a p is over one of the character set titles, switch
  to the set and return true.
*/
bool QIMPenSettingsWidget::selectSet( QPoint p )
{
    if ( charSets.count() ) {
        CharSetEntryIterator it = charSets.begin();
        int spos = 0;
        int idx = 0;
        for ( ; it != charSets.end(); ++it, idx++ ) {
            int setWidth = width() * (*it)->stretch / totalStretch;
            spos += setWidth;
            if ( p.x() < spos ) {
                if ( idx != currCharSet ) {
                    currCharSet = idx;
                    update( 0, 0, width(), 12 );
                    emit changeCharSet( currCharSet );
                    emit changeCharSet( charSets.at(currCharSet)->cs );
                }
                break;
            }
        }
    }

    return false;
}

/*!
  Hopefully returns a sensible size.
*/
QSize QIMPenSettingsWidget::sizeHint() const
{
    return QSize( TITLE_WIDTH * charSets.count(), 75 );
}

void QIMPenSettingsWidget::mousePressEvent( QMouseEvent *e )
{
    if ( !readOnly && e->button() == Qt::LeftButton && mode == Waiting ) {
        // if selectSet returns false the click was not over the
        // char set selectors.
        if ( !selectSet( e->pos() ) ) {
            // start of character input
            timer->stop();
            if ( outputChar ) {
                outputChar = 0;
                outputStroke = 0;
                repaint();
            }
            mode = Input;
            lastPoint = e->pos();
            emit beginStroke();
            inputStroke = new QIMPenStroke;
            strokes.append( inputStroke );
            inputStroke->beginInput( e->pos() );
            // QPainter paint( this );
            // paint.setBrush( Qt::black );
            // paint.drawRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 );
            penMoves.append( QRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 ));
            repaint();
        }
    }
}

void QIMPenSettingsWidget::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !readOnly && e->button() == Qt::LeftButton && mode == Input ) {
        mode = Waiting;
        inputStroke->endInput();
        if ( charSets.count() )
            emit stroke( inputStroke );
        inputStroke = 0;
    }
}

void QIMPenSettingsWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ( !readOnly && mode == Input ) {
        int dx = qAbs( e->pos().x() - lastPoint.x() );
        int dy = qAbs( e->pos().y() - lastPoint.y() );
        if ( dx + dy > 1 ) {
            if ( inputStroke->addPoint( e->pos() ) ) {
                // QPainter paint( this );
                // paint.setPen( Qt::black );
                // paint.setBrush( Qt::black );
                const QVector<QIMPenGlyphLink> &chain = inputStroke->chain();
                QPoint p( e->pos() );
                for ( int i = (int)chain.count()-1; i >= 0; i-- ) {
                    // paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
                    penMoves.append( QRect( p.x()-1, p.y()-1, 2, 2 ));
                    p.rx() -= chain[i].dx;
                    p.ry() -= chain[i].dy;
                    if ( p == lastPoint )
                        break;
                }
                if ( penMoves.count() ) repaint();

                /* ### use this when thick lines work properly on all devices
                paint.setPen( QPen( Qt::black, 2 ) );
                paint.drawLine( lastPoint, e->pos() );
                */
            }
            lastPoint = e->pos();
        }
    }
}

void QIMPenSettingsWidget::paintEvent( QPaintEvent * )
{
    QPainter paint( this );

    // draw guidelines
    paint.setPen( Qt::gray );
    paint.drawLine( 0, 0, width(), 0 );
    int y = height() / 3;
    paint.drawLine( 0, y, width(), y );
    y *= 2;
    paint.setPen( Qt::blue );
    paint.drawLine( 0, y, width(), y );
    paint.setPen( Qt::gray );

    paint.setPen( strokeColor );
    // paint.setBrush( Qt::black );
    foreach ( QRect r, penMoves )
    {
        paint.drawRect( r );
    }

    if ( !charSets.count() )
        return;

    // draw the character set titles
    QFont selFont( "helvetica", 8, QFont::Bold ); // no tr
    QFont font( "helvetica", 8 ); // no tr
    CharSetEntryIterator it = charSets.begin();
    int spos = 0;
    for ( ; it != charSets.end(); ++it ) {
        int setWidth = width() * (*it)->stretch / totalStretch;
        spos += setWidth;
        if ( it != charSets.end() ) {
            paint.drawLine( spos, 0, spos, 5 );
            paint.drawLine( spos, height()-1, spos, height()-6 );
        }
        paint.setFont( font );
        int w = paint.fontMetrics().width( (*it)->cs->title() );
        int tpos = spos - setWidth / 2;
        //string is not translated
        /*paint.drawText( tpos - w/2, 0, w, 12, QPainter::AlignCenter,
                        it.current()->cs->title() );*/
        Q_UNUSED( w );    // not sure why the above is commented out
        Q_UNUSED( tpos ); // so for the moment will keep w & tpos around
    }

    // draw any character that should be displayed when repainted.
    QPoint off;
    const QIMPenStrokeList *stk = 0;
    if ( outputChar && mode == Waiting ) {
        stk = &outputChar->penStrokes();
        QPoint p( outputChar->startingPoint() );
        QRect br( outputChar->boundingRect() );
        p.setX( (width() - br.width()) / 2 + (p.x () - br.left()) );
        off = p - outputChar->startingPoint();
    } else if ( mode == Waiting ) {
        stk = &strokes;
        strokeColor.setAlpha(128);
    }

    if ( stk && !stk->isEmpty() ) {
        paint.setPen( strokeColor );
        paint.setBrush( strokeColor );
        QIMPenStrokeConstIterator it = stk->begin();
        while ( it != stk->end() ) {
            QPoint p = (*it)->startingPoint() + off;
            paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
            const QVector<QIMPenGlyphLink> &chain = (*it)->chain();
            for ( int i = 0; i < (int)chain.count(); i++ ) {
                    p.rx() += chain[i].dx;
                    p.ry() += chain[i].dy;
                    paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
            }
            ++it;
            if (( it == stk->end() ) && mode == Waiting )
                strokeColor.setAlpha(255);
        }
    }

    dirtyRect = QRect();

    // debug
/*
    if ( input ) {
        QArray<int> sig = input->sig();
        for ( unsigned i = 0; i < sig.count(); i++ ) {
            paint.drawPoint( 200 + i, height()/2 - sig[i] / 8 );
        }
    }
*/
}

void QIMPenSettingsWidget::resizeEvent( QResizeEvent *e )
{
    if ( mode == Output )
        showCharacter( outputChar, 0 );

    QWidget::resizeEvent( e );
}

