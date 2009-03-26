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

#include "fifteen.h"

#include <QSettings>
#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qimagedocumentselector.h>

#include <QBoxLayout>
#include <QAction>
#include <QPainter>
#include <QMenu>
#include <QMessageBox>
#include <QStringList>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHeaderView>

#include <stdlib.h>
#include <time.h>

static bool bShowNumber = false;
static bool bGameWon = false;
static int gPhysicalDpiX = 0;

FifteenMainWindow::FifteenMainWindow(QWidget *parent, Qt::WFlags fl)
  : QMainWindow( parent, fl )
{
    gPhysicalDpiX = this->physicalDpiX();

    // random seed
    srand(time(0));

    setWindowTitle(tr("Fifteen Pieces"));
    QWidget *vbox = new QWidget(this);
    QVBoxLayout *vlayout = new QVBoxLayout(vbox);
    vlayout->setMargin(0);
    PiecesView *view = new PiecesView( vbox );
    PiecesTable *table = new PiecesTable( this );
    view->setModel(table);
    view->setItemDelegate(new PiecesDelegate( this ));
    view->setShowGrid(false);
    vlayout->addWidget(view);
    view->setFocus();
    setCentralWidget(vbox);

    QAction *actionShuffle = new QAction( QIcon( ":image/Fifteen" ), tr("Shuffle"), this );
    connect( actionShuffle, SIGNAL(triggered()), table, SLOT(randomize()) );

    QAction *actionReset = new QAction( tr("Reset Pieces"), this );
    connect( actionReset, SIGNAL(triggered()), table, SLOT(reset()) );

    actionLoadImg = new QAction( tr( "Load Image"), this );
    connect( actionLoadImg, SIGNAL(triggered()), table, SLOT(loadImage()) );

    actionDeleteImg = new QAction( tr( "Delete Image"), this );
    connect( actionDeleteImg, SIGNAL(triggered()), table, SLOT(deleteImage()) );

    actionShowNum = new QAction( tr( "Show Numbers" ), this );
    actionShowNum->setCheckable( true );
    if ( bShowNumber )
        actionShowNum->setChecked( true );
    connect( actionShowNum, SIGNAL(triggered()), this, SLOT(showNumber()) );
    connect( actionShowNum, SIGNAL(triggered()), table, SLOT(showNumber()) );
    connect( table, SIGNAL(gameWon()), this, SLOT(showNumber()) );

    connect( table, SIGNAL(updateMenu(bool)), this, SLOT(updateMenu(bool)) );
    updateMenu( table->useImage() );

    QMenu* menu = QSoftMenuBar::menuFor(this);
    menu->addAction( actionShuffle );
    menu->addAction( actionReset );
    menu->addAction( actionLoadImg );
    menu->addAction( actionDeleteImg );
    menu->addAction( actionShowNum );

    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
}

void FifteenMainWindow::showNumber()
{
    actionShowNum->setChecked( bGameWon ? false : !bShowNumber );
    bShowNumber = bGameWon ? false : actionShowNum->isChecked();
}

void FifteenMainWindow::updateMenu( bool useImage )
{
    actionLoadImg->setEnabled( !useImage );
    actionLoadImg->setVisible( !useImage );
    actionDeleteImg->setEnabled( useImage );
    actionDeleteImg->setVisible( useImage );
    actionShowNum->setEnabled( useImage );
    actionShowNum->setVisible( useImage );
    actionShowNum->setChecked( bShowNumber );
    actionShowNum->setVisible( useImage );
}

PiecesTable::PiecesTable(QObject* parent)
    : QAbstractTableModel(parent), _randomized(false)
{
    // init arrays
    initMap();
    readConfig();
    if ( !_randomized )
        randomize();
    initColors();
}

QVariant PiecesTable::data(const QModelIndex &index, int role) const
{
    int number =  _map[index.row()*columnCount() + index.column()];
    switch(role) {
        default:
        case Qt::DisplayRole:
            return number + 1;
        case Qt::BackgroundColorRole:
            return _colors[number];
        case Qt::FontRole:
            if (qApp) {
                QFont f = qApp->font();
                f.setPixelSize(18);
                f.setBold( true );
                return f;
            }
            break;
        case Qt::UserRole:
            if ( _images.count() == 0 ) return 0;
            return _images.at( number );
        case Qt::UserRole + 1:
            return !_imgName.isEmpty();
    }
    return QVariant();
    // set font
}

PiecesTable::~PiecesTable()
{
    writeConfig();
}

void PiecesTable::writeConfig()
{
    QSettings cfg("Trolltech","Fifteen");
    cfg.beginGroup("Game");
    QStringList map;
    for (int i = 0; i < 16; i++)
    map.append( QString::number( _map[i] ) );
    cfg.setValue("Map", map.join(QString('-')));
    cfg.setValue("Randomized", _randomized );
    cfg.setValue("ImageName", _imgName);
    cfg.setValue("ShowNumber", bShowNumber);
}

void PiecesTable::readConfig()
{
    QSettings cfg("Trolltech","Fifteen");
    cfg.beginGroup("Game");
    QStringList map = cfg.value("Map").toString().split( '-');
    _randomized = cfg.value( "Randomized", false ).toBool();
    _imgName = cfg.value( "ImageName", QString() ).toString();
    bShowNumber = cfg.value( "ShowNumber", false ).toBool();
    int i = 0;
    for ( QStringList::Iterator it = map.begin(); it != map.end(); ++it ) {
        _map[i] = (*it).toInt();
        i++;
        if ( i > 15 ) break;
    }

    if ( !_imgName.isEmpty() ) {
        sliceImage();
        emit updateMenu( true );
    }
}

void PiecesTable::loadImage()
{
    QImageDocumentSelectorDialog sel;
    if (QtopiaApplication::execDialog(&sel)) {
        QContent doc = sel.selectedDocument();
        if(doc.fileKnown())
            _imgName = doc.fileName();
    }
    if ( _imgName.isEmpty() )
        return;
    sliceImage();
    emit updateMenu( true );
    bGameWon = bGameWon ? false : bGameWon;
}

void PiecesTable::sliceImage()
{
    _images.clear();
    QImage img(_imgName);
    int w = img.width() / 4;
    int h = img.height() / 4;
    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft;
    for ( int row = 0; row < rowCount() ; row++ ) {
        if ( rtl ) {
            for ( int col = columnCount() - 1 ; col >= 0 ; col-- )
                _images.append( img.copy(col * w, row * h, w, h) );
        } else {
            for ( int col = 0 ; col < columnCount() ; col++ )
                _images.append( img.copy(col * w, row * h, w, h) );
        }
    }
}

void PiecesTable::deleteImage()
{
    _imgName = "";
    _images.clear();
    bShowNumber = false;
    emit updateMenu( false );
    emit dataChanged(createIndex(0,0), createIndex(3,3));
}

void PiecesTable::showNumber()
{
    emit dataChanged(createIndex(0,0), createIndex(3,3));
}

QSize PiecesDelegate::cellSize(10,10);
QPolygon PiecesDelegate::light_border;
QPolygon PiecesDelegate::dark_border;

void PiecesDelegate::paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const
{
    int w = o.rect.width();
    int h = o.rect.height();
    int x2 = w - 1;
    int y2 = h - 1;

    int number = i.model()->data(i, Qt::DisplayRole).toInt();
    QVariant variant = i.model()->data(i, Qt::BackgroundColorRole);
    QColor color = variant.value<QColor>();
    QVariant font = i.model()->data(i, Qt::FontRole);
    if (!font.isNull()) {
        QFont f = font.value<QFont>();
        if (gPhysicalDpiX && gPhysicalDpiX >= 200)
            f.setPointSize((int)(gPhysicalDpiX/4)/8);
        p->setFont(f);
    }

    // draw cell background
    if(number == 16)
        p->setBrush(o.palette.window());
    else
        p->setBrush(color);

    p->setPen(Qt::NoPen);
    p->drawRect(o.rect);

    if (!bGameWon && number == 16) return;

    // get image from model
    // scale and draw the image
    variant = i.model()->data(i, Qt::UserRole + 1);
    bool useImage = variant.toBool();
    if (useImage) {
        variant = i.model()->data(i, Qt::UserRole);
        QImage img = variant.value<QImage>();
        QImage img2=img.scaled(w, h);
        p->drawImage( QPoint( o.rect.x(), o.rect.y() ), img2 );
        if( bShowNumber ) {
            p->setPen(Qt::black);
            p->drawText(o.rect.x() + 1, o.rect.y() + 1, x2, y2, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(number));
            p->setPen(Qt::white);
            p->drawText(o.rect.x(), o.rect.y(), x2, y2, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(number));
        }
        return;
    } else if (number == 16 )
        return;
    // draw borders
    if (h >= 40) {
        QPolygon l = light_border;
        l.translate(o.rect.topLeft());
        p->setBrush(color.light(130));
        p->drawPolygon(l);

        QPolygon d = dark_border;
        d.translate(o.rect.topLeft());
        p->setBrush(color.dark(130));
        p->drawPolygon(d);
    }

    // draw number
    p->setPen(Qt::black);
    p->drawText(o.rect.x(), o.rect.y(), x2, y2, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(number));
}

PiecesView::PiecesView(QWidget *parent) : QTableView(parent), _menu(0)
{
    setFrameStyle(NoFrame);
    verticalHeader()->hide();
    horizontalHeader()->hide();
    rtl = layoutDirection() == Qt::RightToLeft;
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PiecesView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    int cell_w = qMax(10, contentsRect().width() / model()->columnCount());
    int cell_h = qMax(10, contentsRect().height() / model()->rowCount());
    PiecesDelegate::cellSize = QSize(cell_w, cell_h);

    //
    // Calculate 3d-effect borders
    //
    int x_offset = cell_w - int(cell_w * 0.9);  // 10% should be enough
    int y_offset = cell_h - int(cell_h * 0.9);

    PiecesDelegate::light_border.setPoints(6,
    0, 0,
    cell_w, 0,
    cell_w - x_offset, y_offset,
    x_offset, y_offset,
    x_offset, cell_h - y_offset,
    0, cell_h);

    PiecesDelegate::dark_border.setPoints(6,
    cell_w, 0,
    cell_w, cell_h,
    0, cell_h,
    x_offset, cell_h - y_offset,
    cell_w - x_offset, cell_h - y_offset,
    cell_w - x_offset, y_offset);

    int i;
    for (i = 0; i < model()->rowCount(); ++i)
        verticalHeader()->resizeSection(i, cell_h);
    for (i = 0; i < model()->columnCount(); ++i)
        horizontalHeader()->resizeSection(i, cell_w);
}

void PiecesTable::initColors()
{
    _colors.clear();
    for (int r = 0; r < rowCount(); r++)
        for (int c = 0; c < columnCount(); c++)
            _colors.append(QColor(255 - 70 * c,255 - 70 * r, 150));
}

void PiecesTable::initMap()
{
    _map.clear();
    for ( int i = 0; i < 16; i++)
        _map.append(i);

    _randomized = false;
}

void PiecesTable::randomize()
{
    initMap();
    _randomized = true;
    bGameWon = false;
    // find the free position
    int pos = _map.indexOf(15);

    int move = 0;
    while ( move < 333 ) {

        int frow = pos / columnCount();
        int fcol = pos - frow * columnCount();

        // find click position
        int row = rand()%4;
        int col = rand()%4;

        // sanity check
        if ( row < 0 || row >= rowCount() ) continue;
        if ( col < 0 || col >= columnCount() ) continue;
        if ( row != frow && col != fcol ) continue;

        move++;

        // rows match -> shift pieces
        if(row == frow) {

            if (col < fcol) {
                for(int c = fcol; c > col; c--)
                    _map[c + row * columnCount()] = _map[ c-1 + row *columnCount()];
            } else if (col > fcol) {
                for(int c = fcol; c < col; c++)
                    _map[c + row * columnCount()] = _map[ c+1 + row *columnCount()];
            }
        }
        // cols match -> shift pieces
        else if (col == fcol) {

            if (row < frow) {
                for(int r = frow; r > row; r--)
                    _map[col + r * columnCount()] = _map[ col + (r-1) *columnCount()];
            } else if (row > frow) {
                for(int r = frow; r < row; r++)
                    _map[col + r * columnCount()] = _map[ col + (r+1) *columnCount()];
            }
        }
    // move free cell to click position
    _map[pos=(col + row * columnCount())] = 15;
    }
    emit dataChanged(createIndex(0,0), createIndex(3,3));
}

void PiecesTable::checkwin()
{
    if(!_randomized) return;

    int i;
    for (i = 0; i < 16; i++)
        if(i != _map[i])
            break;

    if (i == 16) {

        _randomized = false;
        emit gameWon();
    }
}

void PiecesView::setModel(QAbstractItemModel *m)
{
    QTableView::setModel(m);
    PiecesTable *pt = qobject_cast<PiecesTable *>(model());
    if (pt)
        connect(pt, SIGNAL(gameWon()), this, SLOT(announceWin()));
}

void PiecesView::announceWin()
{
    bGameWon = true;
    QMessageBox::information(this, tr("Fifteen Pieces"),
            tr("Congratulations!<br>You win the game!"));
}

void PiecesTable::reset()
{
    initMap();
    emit dataChanged(createIndex(0,0), createIndex(3,3));
}

QPoint PiecesTable::findPoint(int val)
{
    int i = _map.indexOf(val);
    if (i != -1) {
        int col = i % columnCount();
        int row = ( i - col ) / columnCount();
        return QPoint(col, row);
    }
    return QPoint(0,0);
}

void PiecesTable::pushLeft()
{
    QPoint fpos = findPoint(15);
    push(fpos + QPoint(1,0));
}

void PiecesTable::pushRight()
{
    QPoint fpos = findPoint(15);
    push(fpos + QPoint(-1,0));
}

void PiecesTable::pushUp()
{
    QPoint fpos = findPoint(15);
    push(fpos + QPoint(0,1));
}

void PiecesTable::pushDown()
{
    QPoint fpos = findPoint(15);
    push(fpos + QPoint(0,-1));
}

void PiecesView::keyPressEvent(QKeyEvent* e)
{
    PiecesTable *pt = qobject_cast<PiecesTable *>(model());
    if (!pt) {
        QTableView::keyPressEvent(e);
        return;
    }
    switch ( e->key() ) {
        case Qt::Key_Up: pt->pushUp(); e->accept(); break;
        case Qt::Key_Down: pt->pushDown(); e->accept(); break;
        case Qt::Key_Left:
            if ( rtl ) {
                pt->pushRight();
                e->accept();
                break;
            } else {
                pt->pushLeft();
                e->accept();
                break;
            }
        case Qt::Key_Right:
            if ( rtl ) {
                pt->pushLeft();
                e->accept();
                break;
            } else {
                pt->pushRight();
                e->accept();
                break;
            }

        default:
            QTableView::keyPressEvent(e);
        return;
    }
}

void PiecesView::mousePressEvent(QMouseEvent* e)
{
    PiecesTable *pt = qobject_cast<PiecesTable *>(model());
    if (!pt) {
        QTableView::mousePressEvent(e);
        return;
    }

    if (e->button() == Qt::RightButton) {

    // setup RMB pupup menu
    if(!_menu) {
        _menu = new QMenu(this);
        QAction *randomizeAction = new QAction( tr("R&andomize Pieces"), this );
        connect(randomizeAction, SIGNAL(triggered()), pt, SLOT(randomize()));
        QAction *resetAction = new QAction( tr("&Reset Pieces"), this );
        connect(resetAction, SIGNAL(triggered()), pt, SLOT(reset()));
        _menu->addAction(randomizeAction);
        _menu->addAction(resetAction);
        _menu->adjustSize();
    }

    // execute RMB popup and check result
    QAction *res = _menu->exec(mapToGlobal(e->pos()));
    if (res == randomizeAction)
        pt->randomize();
    else if (res == resetAction)
        pt->reset();
    } else {
        pt->push( indexAt( QPoint( e->x(), e->y() ) ) );
    }
}

void PiecesTable::push(int col, int row)
{
    // GAME LOGIC

    // find the free position
    int pos = _map.indexOf(15);
    if(pos < 0) return;

    int frow = pos / columnCount();
    int fcol = pos - frow * columnCount();

    // sanity check
    if (row < 0 || row >= rowCount()) return;
    if (col < 0 || col >= columnCount()) return;
    if ( row != frow && col != fcol ) return;

    // valid move?
    if(row != frow && col != fcol) return;

    // rows match -> shift pieces
    if(row == frow) {

        if (col < fcol) {
            for(int c = fcol; c > col; c--)
                _map[c + row * columnCount()] = _map[ c-1 + row *columnCount()];
        } else if (col > fcol) {
            for(int c = fcol; c < col; c++)
                _map[c + row * columnCount()] = _map[ c+1 + row *columnCount()];
        }
        emit dataChanged(createIndex(row, fcol), createIndex(row, col));
    }
    // cols match -> shift pieces
    else if (col == fcol) {

        if (row < frow) {
            for(int r = frow; r > row; r--)
                _map[col + r * columnCount()] = _map[ col + (r-1) *columnCount()];
        } else if (row > frow) {
            for(int r = frow; r < row; r++)
                _map[col + r * columnCount()] = _map[ col + (r+1) *columnCount()];
        }
        emit dataChanged(createIndex(frow, col), createIndex(row, col));
    }
    // move free cell to click position
    _map[col + row * columnCount()] = 15;

    // check if the player wins with this move
    checkwin();
}
