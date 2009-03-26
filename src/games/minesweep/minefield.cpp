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
#include "minefield.h"

#include <qtopiaapplication.h>

#include <QSettings>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPixmapCache>
#include <QTimer>
#include <QStyle>



static void minefield_drawIcon(QPainter *p, const QPixmap &pixmap, int dimens, int margin)
{
    p->drawPixmap(margin, margin, pixmap.scaled(dimens - margin*2, dimens - margin*2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

static void minefield_drawWinButton(QPixmap &pixmap, const QRect &rect, const QPalette &palette, bool sunken, bool selected)
{
    QPainter p1;
    QBrush brush = (sunken ? palette.button().color().darker(115) : palette.button());
    p1.begin(&pixmap);
    pixmap.fill();
    if (selected) {
        QPalette palette2 = palette;
        palette2.setBrush( QPalette::Light, palette.brush( QPalette::HighlightedText ) );
        palette2.setBrush( QPalette::Mid, palette.brush( QPalette::HighlightedText ) );
        palette2.setBrush( QPalette::Shadow, palette.brush( QPalette::HighlightedText ) );
        qDrawWinButton( &p1, rect, palette2, sunken, &palette2.highlight());
    } else {
        qDrawWinButton( &p1, rect, palette, sunken, &brush);
    }
}


class Mine //: public Qt
{
public:
    enum MineState {
        Hidden = 0,
        Empty,
        Mined,
        Flagged,
#ifdef MARK_UNSURE
        Unsure,
#endif
        Exploded,
        Wrong
    };

    Mine( MineField* );
    void paint( QPainter *painter, int x, int y, int width, int height );

    QSize sizeHint() const { return QSize( field->preferredGrid(), field->preferredGrid() ); }

    void activate( bool sure = true );
    void setHint( int );

    void setState( MineState );
    MineState state() const { return st; }

    bool isMined() const { return mined; }
    void setMined( bool m ) { mined = m; }

    bool selected() const { return mSelected; }
    void setSelected( bool f ) { mSelected = f; }

    static void cellsNeedRepaint();

    static QPixmap *minePixmap;
    static QPixmap *flagPixmap;

private:
    void getPixmap(QPixmap &pixmap, int width, int height);

    static void resetDefaultPixmaps(int cellWidth, int cellHeight);
    static void cacheCommonPixmaps(int cellWidth, int cellHeight);
    static QPixmap generatePixmap(MineState state, bool selected, int width, int height, int hint = 0);

    bool mSelected;
    bool mined;
    int hint;

    MineState st;
    MineField *field;

    static QPixmap *raisedCell;
    static QPixmap *raisedCurrentCell;
    static QPixmap *sunkenCell;
    static QPixmap *sunkenCurrentCell;
    static bool cacheEmpty;
};

QPixmap* Mine::raisedCell = 0;
QPixmap* Mine::raisedCurrentCell = 0;
QPixmap* Mine::sunkenCell = 0;
QPixmap* Mine::sunkenCurrentCell = 0;
bool Mine::cacheEmpty = true;

QPixmap* Mine::minePixmap = 0;
QPixmap* Mine::flagPixmap = 0;


Mine::Mine( MineField *f )
{
    mSelected = false;
    mined = false;
    st = Hidden;
    hint = 0;
    field = f;
}

void Mine::activate( bool sure )
// sure is true if the mine is being triggered (as in left-clicked),
// and false if it is being flagged/deflagged (as in right-clicked)
{
    if ( !sure ) {
        switch ( st ) {
        case Hidden:
            setState( Flagged );
            break;
        case Flagged:
#ifdef MARK_UNSURE
            setState( Unsure );
            break;
        case Unsure:
#endif
            setState( Hidden );
        default:
            break;
        }
    } else if ( st == Flagged ) {
        return;
    } else {
        if ( mined ) {
            setState( Exploded );
        } else {
            setState( Empty );
        }
    }
}

void Mine::setState( MineState s )
{
    st = s;
}

void Mine::setHint( int h )
{
    hint = h;
}

void Mine::cellsNeedRepaint()
{
    QPixmapCache::clear();
    cacheEmpty = true;
}

void Mine::paint( QPainter *painter, int x, int y, int width, int height )
{
    QPixmap pixmap;
    getPixmap(pixmap, width, height);
    painter->drawPixmap(x, y, pixmap);
}

void Mine::getPixmap(QPixmap &pixmap, int width, int height)
{
    if (cacheEmpty) {
        resetDefaultPixmaps(width, height);
        cacheCommonPixmaps(width, height);
        cacheEmpty = false;
    }

    QString key = QString("st=%1, sel=%2, hint=%3").arg(st).arg(mSelected).arg(hint);
    if (!QPixmapCache::find(key, pixmap)) {
        pixmap = generatePixmap(st, mSelected, width, height, hint);
        QPixmapCache::insert(key, pixmap);
    }
}

void Mine::resetDefaultPixmaps(int cellWidth, int cellHeight)
{
    QPalette palette;
    QRect rect(0, 0, cellWidth, cellHeight);

    if (raisedCell)
        delete raisedCell;
    raisedCell = new QPixmap(cellWidth, cellHeight);
    minefield_drawWinButton(*raisedCell, rect, palette, false, false);

    if (raisedCurrentCell)
        delete raisedCurrentCell;
    raisedCurrentCell = new QPixmap(cellWidth, cellHeight);
    minefield_drawWinButton(*raisedCurrentCell, rect, palette, false, true);

    if (sunkenCell)
        delete sunkenCell;
    sunkenCell = new QPixmap(cellWidth, cellHeight);
    minefield_drawWinButton(*sunkenCell, rect, palette, true, false);

    if (sunkenCurrentCell)
        delete sunkenCurrentCell;
    sunkenCurrentCell = new QPixmap(cellWidth, cellHeight);
    minefield_drawWinButton(*sunkenCurrentCell, rect, palette, true, true);
}

void Mine::cacheCommonPixmaps(int cellWidth, int cellHeight)
{
    // cache the more commonly displayed pixmaps
    MineState states[3] = { Hidden, Flagged, Empty };
    for (int i=0; i<3; i++) {
        QPixmapCache::insert(
                QString("st=%1, sel=%2, hint=0").arg(states[i]).arg(true),
                generatePixmap(states[i], true, cellWidth, cellHeight));
        QPixmapCache::insert(
                QString("st=%1, sel=%2, hint=0").arg(states[i]).arg(false),
                generatePixmap(states[i], false, cellWidth, cellHeight));
    }
    // numbered hints 1-8
    for (int i=1; i<9; i++) {
        QPixmapCache::insert(
                QString("st=%1, sel=%2, hint=%3").arg(Empty).arg(true).arg(i),
                generatePixmap(Empty, true, cellWidth, cellHeight, i));
        QPixmapCache::insert(
                QString("st=%1, sel=%2, hint=%3").arg(Empty).arg(false).arg(i),
                generatePixmap(Empty, false, cellWidth, cellHeight, i));
    }
}

QPixmap Mine::generatePixmap(MineState state, bool selected, int width, int height, int hint)
{
    if (!raisedCell || !raisedCurrentCell || !sunkenCell || !sunkenCurrentCell)
        resetDefaultPixmaps(width, height);

    QRect rect(0, 0, width, height);

    switch (state) {
        case Hidden:
            return QPixmap(selected ? *raisedCurrentCell : *raisedCell);
        case Mined:
        {
            QPixmap pixmap = QPixmap(selected ? *sunkenCurrentCell : *sunkenCell);
            QPainter painter(&pixmap);
            minefield_drawIcon(&painter, *minePixmap, width, width/5);
            return pixmap;
        }
        case Flagged:
        {
            QPixmap pixmap = QPixmap(selected ? *raisedCurrentCell : *raisedCell);
            QPainter painter(&pixmap);
            minefield_drawIcon(&painter, *flagPixmap, width, width/5);
            return pixmap;
        }
        case Exploded:
        {
            QPixmap pixmap = QPixmap(selected ? *sunkenCurrentCell : *sunkenCell);
            QPainter painter(&pixmap);
            minefield_drawIcon(&painter, *minePixmap, width, width/5);
            painter.setPen( Qt::red );
            painter.drawText( rect, Qt::AlignCenter, "X" );
            return pixmap;
        }
        case Wrong:
        {
            QPixmap pixmap = QPixmap(selected ? *raisedCurrentCell : *raisedCell);
            QPainter painter(&pixmap);
            minefield_drawIcon(&painter, *flagPixmap, width, width/5);
            painter.setPen( Qt::red );
            painter.drawText( rect, Qt::AlignCenter, "X" );
            return pixmap;
        }
#ifdef MARK_UNSURE
        case Unsure:
        {
            QPixmap pixmap = QPixmap(selected ? *raisedCurrentCell : *raisedCell);
            QPainter painter(&pixmap);
            painter.drawText(rect, Qt::AlignCenter, "?" );
            return pixmap;
        }
#endif
        case Empty:
        {
            QPixmap pixmap = QPixmap(selected ? *sunkenCurrentCell : *sunkenCell);
            QPainter painter(&pixmap);
            if (hint > 0) {
                switch( hint ) {
                case 1:
                    painter.setPen( Qt::blue );
                    break;
                case 2:
                    painter.setPen( QColor( Qt::green ).darker() );
                    break;
                case 3:
                    painter.setPen( Qt::red );
                    break;
                case 4:
                    painter.setPen( QColor( Qt::darkYellow ) );
                    break;
                case 5:
                    painter.setPen( Qt::darkMagenta );
                    break;
                case 6:
                    painter.setPen( Qt::darkRed );
                    break;
                default:
                    painter.setPen( Qt::black );
                    break;
                }
                painter.drawText( rect, Qt::AlignCenter,
                                  QString::number( hint ) );
            }
            return pixmap;
        }
    }

    // shouldn't happen, all states have been taken care of
    return *raisedCell;
}


/*
  MineField implementation
*/

MineField::MineField( QWidget* parent )
: QFrame( parent )
{
    topMargin = 0;
    leftMargin = 0;

    _minGrid = 16;
    _preferredGrid = 22;

    if(style() && style()->inherits("QThumbStyle"))
    {
        this->physicalDpiX();
        _minGrid = int(_minGrid  * 4.0f / 283.0f * this->physicalDpiX());
        _preferredGrid = int(_preferredGrid * 4.0f / 283.0f * this->physicalDpiX());
    };

    setState( GameOver );
    setFrameStyle(NoFrame);
    setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    setFocusPolicy( Qt::NoFocus );

    // generate right-click events when stylus is held down
    QtopiaApplication::setStylusOperation(this, QtopiaApplication::RightOnHold);

    flagAction = NoAction;
    currRow = currCol = -1;
    minecount=0;
    mineguess=0;
    nonminecount=0;
    cellSize = -1;
    numRows = 0;
    numCols = 0;
    mines = NULL;

    Mine::minePixmap = new QPixmap(":image/mine");
    Mine::flagPixmap = new QPixmap(":image/flag");
}

MineField::~MineField()
{
    for ( int i = 0; i < numCols*numRows; i++ )
        delete mines[i];
    delete[] mines;
}

void MineField::setState( State st )
{
    stat = st;
}

void MineField::setup( int level )
{
    currRow = 0; currCol = 0;
    lev = level;
    setState( Waiting );

    int i;
    for ( i = 0; i < numCols*numRows; i++ )
        delete mines[i];
    delete[] mines;
    switch( lev ) {
    case 1:
        numRows = 9 ;
        numCols = 9 ;
        minecount = 12;
        break;
    case 2:
        numRows = 13;
        numCols = 13;
        minecount = 33;
        break;
    case 3:
        numCols = 18;
        numRows = 18;
        minecount = 66 ;
        break;
    }
    mines = new Mine* [numRows*numCols];
    for ( i = 0; i < numCols*numRows; i++ )
        mines[i] = new Mine( this );

    if( mines[0] )
        mines[0]->setSelected( true );

    nonminecount = numRows*numCols - minecount;
    mineguess = minecount;
    emit mineCount( mineguess );

    setCellSize(findCellSize());

    update( 0, 0, numCols*cellSize, numRows*cellSize) ;
    updateGeometry();
    QTimer::singleShot(0, this, SLOT(currentPointChanged()));
}

void MineField::paintEvent(QPaintEvent* event)
{
    const QRect &eventRect = event->rect();

    int clipx = eventRect.left();
    int clipy = eventRect.top();
    int clipw = eventRect.width();
    int cliph = eventRect.height();

    QPainter p(this);

    int c1 = clipx / cellSize;
    int c2 = ( clipx + clipw - 1 ) / cellSize;
    int r1 = clipy / cellSize;
    int r2 = ( clipy + cliph - 1 ) / cellSize;

    for ( int c = c1; c <= c2 ; c++ ) {
        for ( int r = r1; r <= r2 ; r++ ) {
            int x = c * cellSize;
            int y = r * cellSize;
            Mine *m = mine( r, c );
            if ( m )
            {
                if( r == currRow && c == currCol )
                    m->setSelected( true );
                else
                    m->setSelected( false );
                m->paint(&p, x, y, cellSize, cellSize);
            };
        }
    }
}

void MineField::setAvailableRect( const QRect &r )
{
    availableRect = r;
    setCellSize( findCellSize() );
    QTimer::singleShot(0, this, SLOT(currentPointChanged()));
}

int MineField::findCellSize()
{
    int w = availableRect.width() - 2;
    int h = availableRect.height() - 2;

    int size = qMin( w/numCols, h/numRows );
    if (size < _minGrid)
        size = _minGrid;
    return size;
}

QSize MineField::sizeHint() const {
    return QSize(cellSize*numCols, cellSize*numRows);
}

void MineField::setCellSize( int size )
{
    int w2 = size*numCols;
    int h2 = size*numRows;

    // Don't rely on the change in cellsize to force a resize,
    // as it's possible to have the same size cells when going
    // from a large play area to a small one.

    setGeometry(0, 0, w2, h2);
    updateGeometry();

    if (size != cellSize)
        Mine::cellsNeedRepaint();
    cellSize = size;
}

QSize MineField::minimumSize() const
{
    return QSize(cellSize*numCols, cellSize*numRows);
}

void MineField::placeMines()
{
    int mines = minecount;
    while ( mines ) {
        int col = int((double(rand()) / double(RAND_MAX)) * numCols);
        int row = int((double(rand()) / double(RAND_MAX)) * numRows);

        Mine* m = mine( row, col );

        // place a mine even if the square is flagged, or else player can
        // manipulate the placement of the mines by flagging lots of squares
        // before the game starts
        if ( m && !m->isMined()
             && (m->state() == Mine::Hidden || m->state() == Mine::Flagged) ) {
            m->setMined( true );
            mines--;
        }
    }
}

void MineField::updateCell( int r, int c )
{
    update( c*cellSize, r*cellSize, cellSize, cellSize );
}

void MineField::mousePressEvent( QMouseEvent* e )
{
    if ( state() == GameOver )
        return;

    int c = e->pos().x() / cellSize;
    int r = e->pos().y() / cellSize;
    if ( !onBoard( r, c ) ){
        currCol = currRow = -1;
        return;
    }

    updateCell( currRow, currCol );
    currRow = r;
    currCol = c;
    updateCell( currRow, currCol );

    if ( e->button() == Qt::RightButton ) {
        // flag/unflag this cell
        flagAction = FlagNext;
        updateMine(r,c);
    }
}

void MineField::mouseReleaseEvent( QMouseEvent* e )
{
    if( state() == GameOver ) {
        emit newGameSelected();
        return;
    }

    // Using QtopiaApplication::RightOnHold, so will get a left-click mouse up
    // event with (-1,-1) coords after the right-click mouse up event. Ignore this.
    if ( e->x() == -1 && e->y() == -1 )
        return;

    int c = e->x() / cellSize;
    int r = e->y() / cellSize;

    if ( onBoard( r, c ) && c == currCol && r == currRow ) {
        if ( e->button() == Qt::LeftButton ) {
            Mine *m = mine(r, c);
            if (m) {
                if (m->state() == Mine::Flagged) {
                    if (flagAction != FlagNext) {
                        // allow unflag via left-click - it's annoying to have to
                        // hold down stylus and generate right-click to unflag
                        flagAction = FlagNext;
                        updateMine(r,c);
                    }
                } else {
                    // mine this cell
                    if (flagAction != FlagNext)
                        cellClicked(r, c );
                }
            }
        }
        emit currentPointChanged(currCol*cellSize, currRow*cellSize);
    }

    if ( flagAction == FlagNext ) {
        flagAction = NoAction;
    }
}

void MineField::keyPressEvent( QKeyEvent *e )
{
    if( state() == GameOver ) {
        emit newGameSelected();
        return;
    }

    int row = currRow, col = currCol;
    int key = e->key();

    switch( key )
    {
        case Qt::Key_Select:
            if ( !e->isAutoRepeat() ) {
                QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress,
                        QPoint(currCol * cellSize + (cellSize/2),
                               currRow * cellSize + (cellSize/2)),
                        Qt::LeftButton, 0, 0);
                QApplication::postEvent(this, event);
            }
            return; // return immediately!

        case Qt::Key_Asterisk:
            // flag/unflag this field
            if ( !e->isAutoRepeat() ) {
                flagAction = FlagNext;
                updateMine( currRow, currCol );
            }
            break;
        case Qt::Key_Up:
            --row;
            break;
        case Qt::Key_Down:
            ++row;
            break;
        case Qt::Key_Left:
            --col;
            break;
        case Qt::Key_Right:
            ++col;
            break;
        default:
            QFrame::keyPressEvent( e );
            return;     // ignore Key_Back, etc.
    }

    if( (currRow != row || currCol != col) && onBoard( row, col ) )
    {
        //update affected mines
        updateCell( currRow, currCol );
        currRow = row;
        currCol = col;
        updateCell( currRow, currCol );
        emit currentPointChanged(currCol*cellSize, currRow*cellSize);
    }

    flagAction = NoAction;
}

void MineField::keyReleaseEvent( QKeyEvent *e )
{
    if (e->key() == Qt::Key_Select && !e->isAutoRepeat() ) {
        QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease,
                QPoint(currCol * cellSize + (cellSize/2),
                       currRow * cellSize + (cellSize/2)),
                Qt::LeftButton, 0, 0);
        QApplication::postEvent(this, event);
    }
}

int MineField::getHint( int row, int col )
{
    int hint = 0;
    for ( int c = col-1; c <= col+1; c++ )
        for ( int r = row-1; r <= row+1; r++ ) {
            Mine* m = mine( r, c );
            if ( m && m->isMined() )
                hint++;
        }

    return hint;
}

void MineField::setHint( int row, int col )
{
    Mine *m = mine( row, col );
    if ( !m )
        return;

    int hint = getHint( row, col );

    if ( !hint ) {
        for ( int c = col-1; c <= col+1; c++ )
            for ( int r = row-1; r <= row+1; r++ ) {
                Mine* m = mine( r, c );
                if ( m && m->state() == Mine::Hidden ) {
                    m->activate( true );
                    nonminecount--;
                    setHint( r, c );
                    updateCell( r, c );
                }
            }
    }

    m->setHint( hint );
    updateCell( row, col );
}

/*
  Only place mines after first click, since it is pointless to
  kill the player before the game has started.
*/

void MineField::cellClicked( int row, int col )
{
    if ( state() == GameOver )
        return;
    if ( state() == Waiting ) {
        Mine* m = mine( row, col );
        if ( !m )
            return;
        m->setState( Mine::Empty );
        nonminecount--;
        placeMines();
        setState( Playing );
        //emit gameStarted();

        updateMine( row, col );
    } else { // state() == Playing
        updateMine( row, col );
    }
}

void MineField::updateMine( int row, int col )
{
    Mine* m = mine( row, col );
    if ( !m )
        return;

    bool wasFlagged = m->state() == Mine::Flagged;
    bool wasEmpty =  m->state() == Mine::Empty;

    m->activate( flagAction == NoAction );

    if ( m->state() == Mine::Exploded ) {
        setState( GameOver );
        emit gameOver( false );
        return;
    } else if ( m->state() == Mine::Empty ) {
        setHint( row, col );
        if ( !wasEmpty )
            nonminecount--;
    }

    if ( flagAction != NoAction ) {
        if ( m->state() == Mine::Flagged ) {
            if (/*mineguess > 0*/ true) {  //TODO: Address the implications of this change, especially as relates to negative numbers displayed on the LCD display on a PDA
                --mineguess;
                emit mineCount( mineguess );
                if ( m->isMined() )
                    --minecount;
            } else {
                m->setState(Mine::Hidden);
            }
        } else if ( wasFlagged ) {
            ++mineguess;
            emit mineCount( mineguess );
            if ( m->isMined() )
                ++minecount;
        }
    }

    updateCell( row, col );

    if ( !nonminecount ) {
        setState( GameOver );
        emit gameOver( true );
    }
}

void MineField::showMines()
{
    for ( int c = 0; c < numCols; c++ ) {
        for ( int r = 0; r < numRows; r++ ) {
            Mine* m = mine( r, c );
            if ( !m )
                continue;
            if ( m->isMined() && m->state() == Mine::Hidden )
                m->setState( Mine::Mined );
            if ( !m->isMined() && m->state() == Mine::Flagged )
                m->setState( Mine::Wrong );

            updateCell( r, c );
        }
    }
}

void MineField::currentPointChanged()
{
    emit currentPointChanged(currCol*cellSize, currRow*cellSize);
}

void MineField::paletteChange( const QPalette &o )
{
    Mine::cellsNeedRepaint();
    QFrame::paletteChange( o );
}

void MineField::writeConfig(QSettings& cfg) const
{
    cfg.beginGroup("Field");
    cfg.setValue("Level",lev);
    cfg.setValue("CurrentRow", currRow);
    cfg.setValue("CurrentColumn", currCol);
    QString grid="";
    if ( stat == Playing ) {
        for ( int x = 0; x < numCols; x++ )
            for ( int y = 0; y < numRows; y++ ) {
                char code='A'+(x*17+y*101)%21; // Reduce the urge to cheat
                const Mine* m = mine( y, x );
                int st = (int)m->state(); if ( m->isMined() ) st+=5;
                grid += code + st;
            }
    }
    cfg.setValue("Grid",grid);
    cfg.endGroup();
}

void MineField::readConfig(QSettings& cfg)
{
    cfg.beginGroup("Field");
    lev = cfg.value("Level",1).toInt();
    setup(lev);
    flagAction = NoAction;
    currRow = cfg.value("CurrentRow", 0).toInt();
    currCol = cfg.value("CurrentColumn", 0).toInt();
    QString grid = cfg.value("Grid").toString();
    int x;
    if ( !grid.isEmpty() ) {
        int i=0;
        minecount=0;
        mineguess=0;
        for ( x = 0; x < numCols; x++ ) {
            for ( int y = 0; y < numRows; y++ ) {
                char code='A'+(x*17+y*101)%21; // Reduce the urge to cheat
                int st = (char)grid[i++].unicode()-code;
                Mine* m = mine( y, x );
                if ( st >= 5 ) {
                    st-=5;
                    m->setMined(true);
                    minecount++;
                    mineguess++;
                }
                    m->setState((Mine::MineState)st);
                switch ( m->state() ) {
                  case Mine::Flagged:
                    if (m->isMined())
                        minecount--;
                    mineguess--;
                    break;
                  case Mine::Empty:
                    --nonminecount;
                    break;
                default:
                    break;
                }
            }
        }
        for ( x = 0; x < numCols; x++ ) {
            for ( int y = 0; y < numRows; y++ ) {
                Mine* m = mine( y, x );
                if ( m->state() == Mine::Empty )
                    m->setHint(getHint(y,x));
            }
        }
    }
    setState( Playing );
    cfg.endGroup();
    emit mineCount( mineguess );
    QTimer::singleShot(0, this, SLOT(currentPointChanged()));
}

