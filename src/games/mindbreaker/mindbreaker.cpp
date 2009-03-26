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

#include "mindbreaker.h"

#include <QSettings>
#include <QtopiaApplication>

#include <QPainter>
#include <QPixmap>
#include <QToolButton>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QGraphicsRectItem>
#include <QToolBar>
#include <QToolButton>
#include <QSoftMenuBar>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QMenu>

static int adjusted_panel_height;
static int adjusted_panel_width;

static int adjusted_bin_margin;
static int adjusted_peg_size;
static int adjusted_answerpeg_size;

static int adjusted_title_height;
static int adjusted_title_width;

static int adjusted_first_peg_x_diff;
static int adjusted_first_peg_y_diff;
static int adjusted_peg_spacing;

static int adjusted_answerpegx;
static int adjusted_answerpegy;
static int adjusted_answerpeg_xdiff;
static int adjusted_answerpeg_ydiff;

static int adjusted_board_height;
static int adjusted_board_width;

static void setupBoardSize(int w, int h)
{
    adjusted_panel_width = w * 3/4;
    adjusted_title_width = w * 3/4;

    adjusted_title_height = h/10;
    adjusted_panel_height = (h-adjusted_title_height)/9;

    adjusted_bin_margin = w * 10/240;
    adjusted_peg_size = adjusted_panel_height*3/4;
    adjusted_answerpeg_size = qMin(adjusted_panel_width*15/180,adjusted_panel_height*15/25);

    // looks a bit dodgy on larger sizes
    if ( adjusted_peg_size > 40 )
        adjusted_peg_size = 40;

    adjusted_first_peg_x_diff = w * 31/240-adjusted_peg_size/2;
    adjusted_first_peg_y_diff = (adjusted_panel_height - adjusted_peg_size)/2;
    adjusted_peg_spacing = w * 30/240;

    // looks a bit dodgy on larger sizes (still does though, but not as much...)
    if ( adjusted_answerpeg_size > 22 )
        adjusted_answerpeg_size = 22;

    adjusted_answerpegx = adjusted_panel_width * 159/180 - adjusted_answerpeg_size/2;
    adjusted_answerpegy = adjusted_panel_height/3 - adjusted_answerpeg_size/2;
    adjusted_answerpeg_xdiff = adjusted_panel_width * 10/180;
    adjusted_answerpeg_ydiff = adjusted_panel_height * 9/25;

    adjusted_board_height = adjusted_title_height + (adjusted_panel_height * 9);
    adjusted_board_width = adjusted_panel_width + (adjusted_bin_margin * 2) + adjusted_peg_size;
}


/* helper class,  */
class Peg : public QGraphicsRectItem
{
public:
    Peg(QGraphicsScene *canvas, int type, int go = -1, int pos = -1);
    int type() const { return Type; }
    void advance(int phase);

    bool hit( const QPointF &) const;

/* a placed peg is one that has been set down on the board correctly and
   should not be moved, only copied */
    bool placed() const;
    void setPlaced(bool);

    int pegGo() const;
    int pegPos() const;
    void setPegPos(int);

    int pegType() const;

    static void buildImages();
    static QImage imageForType(int t);

    static int eggLevel;

    enum { Type = UserType + 1 };

    virtual void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
private:
    static QVector<QImage *> normalPegs;
    static QVector<QImage *> specialPegs;

    bool isplaced;
    int pegtype;
    int peg_go;
    int peg_pos;

    int aniStep;
};
int Peg::eggLevel = 0;
QVector<QImage *> Peg::normalPegs;
QVector<QImage *> Peg::specialPegs;

void Peg::buildImages()
{
    QImage pegs(":image/mindbreaker/pegs");
    int x = 0;
    int y = 0;
    int i;
    eggLevel = 0;
    normalPegs.resize(10);
    for (i = 0; i < 6; i++) {
        normalPegs[i] = new QImage(pegs.copy(x, y, peg_size, peg_size).
                                scaled(adjusted_peg_size, adjusted_peg_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
        x += peg_size;
    }
    specialPegs.resize(5);
    for (i = 0; i < 5; i++) {
        specialPegs[i] = new QImage(pegs.copy(x,y,peg_size, peg_size).
                                scaled(adjusted_peg_size, adjusted_peg_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
        x += peg_size;
    }

    QImage image(":image/mindbreaker/mindbreaker-sprites");
    /* copy from master image to functional images */
    x = 0;
    y = panel_height;
    normalPegs[8] = new QImage( image.copy(x, y, panel_width, panel_height).
                        scaled( adjusted_panel_width, adjusted_panel_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                      );
    y += panel_height;
    y += title_height;
    normalPegs[9] = new QImage(image.copy(x, y, title_width, title_height).
                        scaled( adjusted_title_width, adjusted_title_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                    );
    y += title_height;

    x = 6 * peg_size;
    normalPegs[6] = new QImage(image.copy(x, y, answerpeg_size, answerpeg_size).
                        scaled( adjusted_answerpeg_size, adjusted_answerpeg_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
    x += answerpeg_size;
    normalPegs[7] = new QImage(image.copy(x, y, answerpeg_size, answerpeg_size).
                        scaled( adjusted_answerpeg_size, adjusted_answerpeg_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
}

QImage Peg::imageForType(int t)
{
    if (eggLevel > t ) {
        if( t < 5) {
            return *specialPegs[t];
        } else {
            return *normalPegs[qrand() % 6];
        }
    }
    return *normalPegs[t];
}

Peg::Peg(QGraphicsScene *canvas , int t, int g, int p)
        : QGraphicsRectItem(NULL)
{
    setRect(x(), y(), normalPegs[t]->width(), normalPegs[t]->height() );
    pegtype = t;
    isplaced = false;
    peg_pos = p;
    peg_go = g;
    aniStep = qrand() % 6;
    canvas->addItem(this);
}

void Peg::advance(int phase) {
    if (phase == 0)
        aniStep = (++aniStep) % 6;
    else {
        hide();
        show();
    }
}

void Peg::paint ( QPainter * painter, const QStyleOptionGraphicsItem *, QWidget * )
{
    //QGraphicsRectItem::paint(painter, option, widget);
    if ((pegtype == 5) && eggLevel > 5) {
        painter->drawImage(0, 0, *normalPegs[aniStep]);
    } else
        painter->drawImage(0, 0, imageForType(pegtype));
}

bool Peg::hit( const QPointF &p ) const
{
    QPoint point(QPointF(p.x() - x(), p.y()-y()).toPoint());
    if (!normalPegs[pegtype]->valid(point))
        return false;
    QRgb pixel = normalPegs[pegtype]->pixel(point);
    return (qAlpha(pixel ) != 0);
}

inline bool Peg::placed() const
{
    return isplaced;
}

inline int Peg::pegGo() const
{
    return peg_go;
}

inline int Peg::pegPos() const
{
    return peg_pos;
}

inline void Peg::setPegPos(int p)
{
    peg_pos = p;
}

inline void Peg::setPlaced(bool p)
{
    isplaced = p;
}

inline int Peg::pegType() const
{
    return pegtype;
}

/*
    Load the main image, copy from it the pegs, the board, and the answer image
    and use these to create the tray, answer and board
*/
MindBreaker::MindBreaker( QWidget * parent, Qt::WindowFlags flags )
   : QMainWindow(parent, flags)
{
    QtopiaApplication::setInputMethodHint(this,QtopiaApplication::AlwaysOff);
    setMinimumSize(160,210);
    setWindowTitle(tr("Mindbreaker"));

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);
    //layout()->addStretch();
    board = new MindBreakerBoard(new QGraphicsScene, this);
    setCentralWidget(board);
    //layout()->addStretch();

    QToolBar *tb = new QToolBar(this);
    tb->setMovable(false);
    addToolBar(tb);

    QMenu *menu = QSoftMenuBar::menuFor( this );
    QAction *action = menu->addAction( QIcon(":image/mindbreaker/Mindbreaker"), tr("New Game"), board, SLOT(clear()));
    action->setWhatsThis( tr("Start a new game") );
    action->setEnabled( true );
    QAction *resetscore = menu->addAction( QIcon(), tr("Reset Score"), board, SLOT(resetScore()));
    resetscore->setWhatsThis( tr("Reset scores") );
    resetscore->setEnabled( true );

    score = new QLabel(tb);
    score->setText("");
    score->setMaximumHeight(20);
    tb->addWidget(score);

    connect(board, SIGNAL(scoreChanged(int,int)), this, SLOT(setScore(int,int)));

    int a, b;
    board->getScore(&a, &b);
    setScore(a,b);

    layout()->setSizeConstraint(QLayout::SetNoConstraint);

    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
}

void MindBreaker::setScore(int turns, int games)
{
    double average;
    double total_turns = turns;
    double total_games = games;

    if(total_games > 0)
        average = total_turns / total_games;
    else
        average = 0.0;

    score->setText(tr("win avg: %1 turns (%2 games)").arg(average, 0, 'g', 2).arg(games));
}

void MindBreaker::resizeEvent( QResizeEvent *e )
{
    board->fixSize();
    QMainWindow::resizeEvent( e );
}


MindBreakerBoard::MindBreakerBoard( QGraphicsScene * gscene, QWidget * parent )
           : QGraphicsView(gscene, parent),
            moving(0), game_over(false), total_turns(0), total_games(0)
{
    setFrameStyle( NoFrame );
    setupBoardSize(qApp->desktop()->width(),qApp->desktop()->height());
    scene()->setSceneRect(0, 0, 100, 100);
    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);

    qsrand(QDateTime::currentDateTime().toTime_t());

    QTimer *advanceTimer = new QTimer(this);
    advanceTimer->setInterval(500);
    connect(advanceTimer, SIGNAL(timeout()), scene(), SLOT(advance()));
    advanceTimer->start();
    current_highlight = 0;

    widthTimer = new QTimer( this );
    connect(widthTimer, SIGNAL(timeout()), this, SLOT(doFixSize()) );

    setMaximumWidth( qMin(qApp->desktop()->height(),qApp->desktop()->width()) );
    //doFixSize(); // build images... needs to be done before reading config.
    //readConfig(); // first read... to ensure initial labels and side look right.
}

void MindBreakerBoard::readConfig()
{
    QSettings c(QSettings::UserScope, "MindBreaker");
    for (int i = 0; i < 4; i++) {
        answer[i] = qrand() % 6;
        current_guess[i] = 6;
    }
    c.beginGroup("Score");
    if(c.contains("Turns"))
        total_turns = c.value("Turns").toInt();
    else
        total_turns = 0;
    if(c.contains("Games"))
        total_games = c.value("Games").toInt();
    else
        total_games = 0;
    checkScores();
    c.endGroup();
}

MindBreakerBoard::~MindBreakerBoard()
{
    int i;
    if (game_over) {
        current_go = 0;
        /* clear the answer, clear the guess */
        for (i = 0; i < 4; i++) {
            answer[i] = qrand() % 6;
            current_guess[i] = 6;
        }
    }
    writeConfig();
}

void MindBreakerBoard::writeConfig()
{
    QSettings c(QSettings::UserScope, "MindBreaker");
    c.beginGroup("Score");
    /* write the score */

    c.setValue("Turns", total_turns);
    c.setValue("Games", total_games);
    c.endGroup();
}

void MindBreakerBoard::getScore(int *a, int *b)
{
    *a = total_turns;
    *b = total_games;
    return;
}

void MindBreakerBoard::fixSize()
{
    hide();
    setMaximumWidth( parentWidget()->height() );
    widthTimer->setSingleShot(true);
    widthTimer->start(20);
}

void MindBreakerBoard::doFixSize()
{
    QSize s = size();
    int fw = frameWidth();
    s.setWidth(s.width() - fw);
    s.setHeight(s.height() - fw);

    /* min size is 200 x 260 */
/*
    if (s.width() < adjusted_board_width)
        s.setWidth(adjusted_board_width);

    if (s.height() < adjusted_board_height)
        s.setHeight(adjusted_board_height);
*/

    if ( current_highlight ) // non-first resize
        writeConfig();

    setupBoardSize(s.width() - fw, s.height() - fw);
    scene()->setSceneRect(0, 0, s.width() - fw, s.height() - fw);
    Peg::buildImages(); // must be done BEFORE any pegs are made

    QImage image(":image/mindbreaker/mindbreaker-sprites");

    /* copy from master image to functional images */
    int x = 0;
    int y = 0;
    panelImage = image.copy(x, y,  panel_width, panel_height).
                scaled( adjusted_panel_width, adjusted_panel_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    y += panel_height;
    y += panel_height;

    titleImage = image.copy(x, y, title_width, title_height).
                scaled( adjusted_title_width, adjusted_title_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    show();

    delete current_highlight;
    current_highlight = new Peg(scene(), 8);
    current_highlight->setPlaced(true);
    current_highlight->setPos(0, adjusted_board_height - ((current_go + 1) * adjusted_panel_height));
    current_highlight->setZValue(0);
    current_highlight->show();

    /* set up the game */
    //readConfig();

    /* draw initial screen */
    //drawBackground();
    //canvas()->update();
    clear();

    readConfig();
}

void MindBreakerBoard::placeGuessPeg(int pos, int pegId)
{
        int x = adjusted_first_peg_x_diff + (pos * adjusted_peg_spacing);
        int y = adjusted_board_height - ((current_go + 1) * adjusted_panel_height)
                + adjusted_first_peg_y_diff;

        Peg *peg = new Peg(scene(), pegId, current_go, pos);
        peg->setPegPos(pos);
        peg->setPlaced(true);
        peg->setPos(x, y);
        peg->setZValue(2);
        peg->show();
}

void MindBreakerBoard::paintBackground()
{
    int i, j, x, y, x_gap, y_gap;
    QPixmap background  = QPixmap(QSizeF(scene()->width(), scene()->height()).toSize());

    QPainter painter(&background);

    painter.fillRect(QRectF(0.0, 0.0, scene()->width(), scene()->height()), QColor(0,0,0));
    /* very first thing is to draw the bins, as everything else needs
     * to be drawn over them */

    QPen pen(QColor(85, 45, 27), 4);
    painter.setPen(pen);
    x_gap = (int)scene()->width() - (adjusted_panel_width + (2 * adjusted_bin_margin));
    //x_gap += peg_size >> 1;
    if (x_gap < 1)
        x_gap = 1;

    y_gap = adjusted_board_height / 6;
    y_gap -= (2 * adjusted_bin_margin);
    //y_gap += peg_size >> 1;
    if (y_gap < 1)
        y_gap = 1;
    x = adjusted_panel_width + adjusted_bin_margin - (adjusted_peg_size >> 1);
    y = adjusted_bin_margin - (adjusted_peg_size >> 1) + 2;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 10; j++) {
            int rx = x + (qrand() % x_gap);
            int ry = y + (qrand() % y_gap);
            painter.drawImage(rx,ry, Peg::imageForType(i));
        }
        y += adjusted_board_height / 6;
    }
    /* now draw the surrounding boxes */
    x_gap = (int)scene()->width() - adjusted_panel_width;
    if (x_gap < 1) x_gap = 1;
    y_gap = adjusted_board_height / 6;
    x = adjusted_panel_width;
    y = 1;

    for (i = 0; i < 6; i++) {
        painter.drawRect(x, y, x_gap, y_gap);
        y += y_gap;
    }

    x = 0;
    y = 0;

    painter.drawImage(x,y, titleImage);
    y = adjusted_title_height;
    /* now nine gues panels */
    for (i = 0; i < 9; i ++) {
        painter.drawImage(x, y, panelImage);
        y += adjusted_panel_height;
    }

    QCoreApplication::flush ();
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(background));
    setPalette(palette);
}

void MindBreakerBoard::checkGuess()
{
    int i,j;
    int num_white = 0;
    int num_black = 0;
    int copy_answer[4];
    int copy_guess[4];

    for(i = 0; i < 4; i++) {
        copy_answer[i] = answer[i];
        copy_guess[i] = current_guess[i];
        if (current_guess[i] == 6)
            return;
        if (answer[i] == current_guess[i]) {
            num_black++;
            copy_answer[i] = 6;
            copy_guess[i] = 7;
        }
    }

    /* now sure that user has completed a 'guess' */
    for (i = 0; i < 4; i++) {
        if (copy_guess[i] == 7)
            continue; // already marked for a black
        for (j = 0; j < 4; j++) {
            if(copy_guess[i] == copy_answer[j]) {
                copy_answer[j] = 6;
                num_white++;
                break;
            }
        }
    }

    int x = adjusted_answerpegx;
    int y = (adjusted_board_height - ((current_go + 1) * adjusted_panel_height)) + adjusted_answerpegy;

    if (num_black == 4)
        game_over = true;

    while(num_black > 0) {
        Peg *p = new Peg(scene(), 7);
        p->setPlaced(true);
        p->setPos(x, y);
        p->setZValue(1);
        p->show();
        num_black--;

        if (x == adjusted_answerpegx)
            x = adjusted_answerpegx + adjusted_answerpeg_xdiff;
        else  {
            x = adjusted_answerpegx;
            y += adjusted_answerpeg_ydiff;
        }
    }
    while(num_white > 0){
        Peg *p = new Peg(scene(), 6);
        p->setPlaced(true);
        p->setPos(x, y);
        p->setZValue(1);
        p->show();
        num_white--;

        if (x == adjusted_answerpegx)
            x = adjusted_answerpegx + adjusted_answerpeg_xdiff;
        else  {
            x = adjusted_answerpegx;
            y += adjusted_answerpeg_ydiff;
        }
    }
    /* move to next go */
    for(i = 0; i < 4; i++) {
        past_guesses[4*current_go+i] = current_guess[i];
        current_guess[i] = 6;
    }

    current_go++;
    if((current_go > 8) || game_over) {
        total_games++;
        if(!game_over)
            total_turns += 10;
        else
            total_turns += current_go;

        emit scoreChanged(total_turns, total_games);
        Peg *p = new Peg(scene(), 9);
        game_over = true;
        p->setPlaced(true);
        p->setPos(0, 0);
        p->setZValue(0);
        p->show();

        for (i = 0; i < 4; i++) {
            p = new Peg(scene(), answer[i], -1);
            p->setPos(adjusted_first_peg_x_diff + (i * adjusted_peg_spacing), adjusted_first_peg_y_diff);
            p->setZValue(3);
            p->show();
        }
    } else {
       current_highlight->setPos(current_highlight->x(), adjusted_board_height - ((current_go + 1) * adjusted_panel_height));
    }
    scene()->update();
}

void MindBreakerBoard::clear()
{
    int i;
    /* reset the game board */
    game_over = false;
    /* clear the answer, clear the guess */
    for (i = 0; i < 4; i++) {
        answer[i] = qrand() % 6;
        current_guess[i] = 6;
    }
    current_go = 0;

    QList<QGraphicsItem *> list = scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if (*it == current_highlight)
            continue;
        if (*it)
            delete *it;
    }

    current_highlight->setPos(current_highlight->x(), adjusted_board_height - ((current_go + 1) * adjusted_panel_height));
    checkScores();
    paintBackground();
    scene()->update();
}

void MindBreakerBoard::resetScore()
{
    /* Are you sure? */

    if (QMessageBox::information(this, tr( "Reset Statistics" ),
            tr( "<qt>Reset the win ratio?</qt>" ),
            tr( "OK" ), tr( "Cancel" ) ) == 0) {
        total_turns = 0;
        total_games = 0;
        Peg::eggLevel = 0;
        paintBackground();
        scene()->update();
        emit scoreChanged(total_turns, total_games);
    }
}

/* EVENTS */

void MindBreakerBoard::mousePressEvent(QMouseEvent *e)
{
    if (game_over) {
        null_press = true;
        null_point = e->pos();
        moving = 0;
        return;
    }

    copy_press = false;
    null_press = false;
    /* ok, first work out if it is one of the bins that
       got clicked */
    if (e->x() > adjusted_panel_width) {
        /* its a bin, but which bin */
        int bin = (e->y() + 2) / (adjusted_board_height / 6);
        if (bin > 5)
            return; // missed everything

        /* make new peg... set it moving */
        moving_pos = e->pos();
        moving = new Peg(scene(), bin, current_go);
        moving->setPos(e->x() - (adjusted_peg_size >> 1), e->y() - (adjusted_peg_size >> 1));
        moving->setZValue(5);
        moving->show();
        scene()->update();
        return;
    }

    QList<QGraphicsItem *> l = scene()->items(e->pos());
    for (QList<QGraphicsItem *>::Iterator it=l.begin(); it !=l.end(); ++it) {
        if ( (*it)->type() == Peg::Type) {
            Peg *item = qgraphicsitem_cast<Peg *>(*it);
            if (!item->hit(e->pos()))
                continue;
            if (item->pegType() > 5) {
                null_press = true;
                null_point = e->pos();
                continue; /* not a color peg */
            }
            if (item->placed()) {
                /* copy */
                if(item->pegGo() == -1)
                    return;
                if(item->pegGo() == current_go) {
                    copy_press = true;
                    copy_peg = item;
                }
                moving = new Peg(scene(),
                                 item->pegType(), current_go);
                moving->setPos(e->x() - (adjusted_peg_size >> 1), e->y() - (adjusted_peg_size >> 1));
                moving->setZValue(5);
                moving->show();
                moving_pos = QPointF(e->x(), e->y());
                scene()->update();
                return;
            }
            moving = qgraphicsitem_cast<Peg *>(*it);
            moving_pos = e->pos();
            scene()->update();
            return;
        }
    }
    null_press = true;
    null_point = e->pos();
    moving = 0;
}

void MindBreakerBoard::mouseMoveEvent(QMouseEvent* e)
{
    if (moving ) {
        moving->moveBy(e->pos().x() - moving_pos.x(),
                       e->pos().y() - moving_pos.y());
        moving_pos = e->pos();
        scene()->update();
        return;
    }
}

void MindBreakerBoard::mouseReleaseEvent(QMouseEvent* e)
{
    /* time to put down the peg */
    if(moving) {
        if(copy_press) {
            /* check if collided with original. if so, delete both */
            copy_press = false;
            QList<QGraphicsItem *> l = scene()->items(e->pos());
            for (QList<QGraphicsItem *>::Iterator it=l.begin(); it !=l.end(); ++it) {
                if (*it == copy_peg)
                    copy_press = true;
            }
            if (copy_press) {
                current_guess[copy_peg->pegPos()] = 6;
                delete copy_peg;
                delete moving;
                copy_press = false;
                moving = 0;
                copy_peg = 0;
                scene()->update();
                return;
            }
        }

        /* first work out if in y */
        if (e->y() > (adjusted_board_height - (current_go * adjusted_panel_height))) {
            delete moving;
            moving = 0;
            scene()->update();
            return;
        }
        if (e->y() < (adjusted_board_height - ((current_go + 1) * adjusted_panel_height))) {
            delete moving;
            moving = 0;
            scene()->update();
            return;
        }
        /* ok, a valid go, but which peg */
        int x_bar = adjusted_first_peg_x_diff - (adjusted_peg_size >> 1);
        x_bar += adjusted_peg_spacing;
        int pos = 0;
        if (e->x() > x_bar)
            pos = 1;
        x_bar += adjusted_peg_spacing;
        if (e->x() > x_bar)
            pos = 2;
        x_bar += adjusted_peg_spacing;
        if (e->x() > x_bar)
            pos = 3;
        x_bar += adjusted_peg_spacing;

        if (e->x() > x_bar) {
            /* invalid x */
            delete moving;
            moving = 0;
            scene()->update();
            return;
        }

        int x = adjusted_first_peg_x_diff + (pos * adjusted_peg_spacing);
        int y = adjusted_board_height - ((current_go + 1) * adjusted_panel_height)
                + adjusted_first_peg_y_diff;
        moving->setPegPos(pos);
        moving->setPos(x, y);
        moving->setZValue(2);

        /* remove all other pegs from this position */
        QList<QGraphicsItem *> l = scene()->items(QPointF(x,y));
        for (QList<QGraphicsItem *>::Iterator it=l.begin(); it !=l.end(); ++it) {
            if ( (*it)->type() == Peg::Type ) {
                Peg *item = qgraphicsitem_cast<Peg *>(*it);
                if ((item != moving) && (item != current_highlight))
                    delete item;
            }
        }
        current_guess[pos] = moving->pegType();

        moving->setPlaced(true);
        scene()->update();
        //return;
    }
    moving = 0;
    null_point -= e->pos();
    if(qAbs(null_point.x())+qAbs(null_point.y()) < 6) {
        if (game_over)
            clear();
        else
            checkGuess();
    }
}

void MindBreakerBoard::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    fixSize();
}


/* Easter egg function... beat the clock */
void MindBreakerBoard::checkScores()
{
    double games = total_games;
    double turns = total_turns;
    double g = games / 10.0;
    Peg::eggLevel = 0;

    double break_even = 5.0;
    if (g < 1.0)
        return;
    double avg = turns / games;
    g--;
    while (break_even >= 0.0) {
        if (avg >= (break_even + g))
            return;
        // score a peg.
        break_even -= 1.0;
        Peg::eggLevel = int(5.0 - break_even);
    }
}
