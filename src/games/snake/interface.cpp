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

#include "interface.h"
#include "sprites.h"
#include "snake.h"

#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QTimer>
#include <stdlib.h>
#include <QtopiaApplication>
#include <QSoftMenuBar>
#include <QToolBar>
#include <QLabel>
#include <QGraphicsTextItem>
#include <QDesktopWidget>
#include <QGraphicsSceneMouseEvent>

///////////////////////////////////////////////////////////////////////////
// Simple sprite classes for the mice and walls
///////////////////////////////////////////////////////////////////////////

class Mouse : public QAnimatedPixmapItem
{
public:
    Mouse(QGraphicsScene* scene, int x, int y)
        : QAnimatedPixmapItem(scene)
    {
        setSequence(SpriteDB::spriteCache());
        setFrame(SpriteDB::mouse());
        setPos(x, y);
        show();
    }

    virtual int type() const
    {
        return mouse_rtti;
    }
};

class Wall : public QAnimatedPixmapItem
{
public:
    Wall(QGraphicsScene* scene, int x, int y, bool left, bool right, bool up, bool down)
        : QAnimatedPixmapItem(scene)
    {
        setSequence(SpriteDB::spriteCache());
        setFrame(SpriteDB::wall(left, right, up, down));
        setZValue(-100);
        setPos(x, y);
        show();
    }

    virtual int type() const
    {
        return wall_rtti;
    }
};

///////////////////////////////////////////////////////////////////////////
// Main game class
///////////////////////////////////////////////////////////////////////////

SnakeGame::SnakeGame(QWidget* parent, Qt::WFlags)
    : QGraphicsView(parent)
    , snake(0)
    , waitover(true)
    , gamestopped(true)
    , paused(false)
    , gamemessage(0)
    , gamemessagebkg(0)
{
    setWindowTitle(tr("Snake"));

    scene = new QGraphicsScene(this);

    scene->setBackgroundBrush(QBrush(SpriteDB::spriteCache().at(SpriteDB::ground())));
    QRect workarea=QApplication::desktop()->availableGeometry();
    QRectF sceneRect;
    if(workarea.width() > workarea.height() && workarea.width() > 320)
        sceneRect = QRectF(0,0, 320, workarea.height()*320/workarea.width());
    else if(workarea.width() < workarea.height() && workarea.height() > 320)
        sceneRect = QRectF(0,0, workarea.width()*320/workarea.height(), 320);
    else
        sceneRect = QRectF(0,0, workarea.width(), workarea.height());
    scene->setSceneRect(sceneRect);

    setScene(scene);
    setFrameStyle(QFrame::NoFrame);
    fitInView(sceneRect, Qt::KeepAspectRatio);

    setFocusPolicy(Qt::StrongFocus);
    QtopiaApplication::setInputMethodHint( this, QtopiaApplication::AlwaysOff );

    (void)QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, "icons/play", tr("Play"));
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Back );

    // Seed the random number generator for randomly positioning the targets
    QTime midnight(0, 0, 0);
    srand(midnight.secsTo(QTime::currentTime()) );
}

SnakeGame::~SnakeGame()
{
    delete snake;
}

void SnakeGame::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    QRectF sceneRect = scene->sceneRect();
    fitInView(sceneRect, Qt::KeepAspectRatio);

    // Calculate the number of tiles on the board for use in laying out the board.
    int tilesize = SpriteDB::tileSize();
    screenwidth = int(sceneRect.width() / tilesize);
    screenheight = int(sceneRect.height() / tilesize);

    // End any running game and layout the board ready to start again.
    newGame(false);
}

void SnakeGame::pause()
{
    snake->stop();
    if(Qtopia::mousePreferred())
        showMessage(tr("Game Paused:\nClick to resume"));
    else
        showMessage(tr("Game Paused:\nPress Play\nkey to resume"));
    paused = true;
    QSoftMenuBar::setLabel( this, Qt::Key_Select, "icons/play", tr("Play"));
}

void SnakeGame::resume()
{
    paused = false;
    clearMessage();
    snake->start();
    QSoftMenuBar::setLabel( this, Qt::Key_Select, "icons/pause", tr("Pause"));
}

void SnakeGame::focusOutEvent(QFocusEvent *)
{
    // We're losing focus, so pause the game if it's running.  We make the user
    // explicitly resume the game when we regain focus rather than starting
    // again automatically, which they might not be ready for.
    if (!gamestopped && !paused)
        pause();
}

void SnakeGame::newGame(bool start)
{
    QSoftMenuBar::setLabel( this, Qt::Key_Select, "icons/play", tr("Play"));

    // End any previous game
    clear();

    // Setup the new game
    snake = new Snake(scene);
    connect(snake, SIGNAL(dead()), this, SLOT(gameOver()) );
    connect(snake, SIGNAL(ateMouse()), this, SLOT(levelUp()) );
    level = 1;
    stage = 0;
    mice = 0;
    gamestopped = false;
    waitover = true;
    if (start) {
        paused = false;
        snake->start();
        QSoftMenuBar::setLabel( this, Qt::Key_Select, "icons/pause", tr("Pause"));
    }
    else {
        paused = true;
        if(Qtopia::mousePreferred())
            showMessage(tr("Click to start"));
        else
            showMessage(tr("Press Play\nkey to start"));
    }

    createWalls();
    createMice();
}

//
// Create walls and obstacles.
//
void SnakeGame::createWalls()
{
    int tilesize = SpriteDB::tileSize();

    int right = tilesize*screenwidth-tilesize;
    int bottom = tilesize*screenheight-tilesize;

    // corners
    new Wall(scene, 0, 0, false, true, false, true);
    new Wall(scene, right, bottom, true, false, true, false);
    new Wall(scene, right, 0, true, false, false, true);
    new Wall(scene, 0, bottom, false, true, true, false);

    // top and bottom sides
    for (int i = 1; i+1 < screenwidth; ++i) {
        new Wall(scene, i*tilesize, 0, true, true, false, false);
        new Wall(scene, i*tilesize, bottom, true, true, false, false);
    }

    // left and right sides
    for (int i = 1; i+1 < screenheight; ++i) {
        new Wall(scene, 0, i*tilesize, false, false, true, true);
        new Wall(scene, right, i*tilesize, false, false, true, true);
    }

    // now make a couple of mid screen walls
    int obwidth = (screenwidth-2) >> 1;
    int obstart = ((screenwidth-2) >> 2) + 1;
    int oboffset = (screenheight-2) / 3;
    for (int i = obstart; i < obstart+obwidth; ++i) {
        new Wall(scene, i*tilesize, tilesize+oboffset*tilesize,
                     i!=obstart, i!=obstart+obwidth-1, false, false);
        new Wall(scene, i*tilesize, tilesize+2*oboffset*tilesize,
                     i!=obstart, i!=obstart+obwidth-1, false, false);
    }
}

void SnakeGame::levelUp()
{
    // If there are no mice left, replace them and go up a stage.
    // At every third stage go up a level.
    mice--;
    if (mice == 0) {
        stage++;
        if (stage == 3) {
           level++;
           snake->increaseSpeed();
           stage = 0;
        }
        createMice();
    }
}

void SnakeGame::createMice()
{
    // Create randomly placed mice equal to the level number.

// TODO: Better way to handle running out of space, e.g. restart the game
// at higher difficulty when snake reaches some percentage of the empty space.

    int tilesize = SpriteDB::tileSize();

    for (int i = 0; i < level; ++i) {
        int max_position_tries = 100;
        int x;
        int y;
        do {
            x = (rand() % (screenwidth-2)) * tilesize + tilesize;
            y = (rand() % (screenheight-2)) * tilesize + tilesize;
        } while (!scene->items(QPoint(x,y)).isEmpty() && --max_position_tries);

        if (max_position_tries > 0) {
            new Mouse(scene, x, y);
            mice++;
        }
        else
            break;  // can't place any more mice
    }
}

void SnakeGame::clear()
{
    clearMessage();
    delete snake;
    snake = 0;
    QList<QGraphicsItem *> l = scene->items();
    for (QList<QGraphicsItem *>::Iterator it=l.begin(); it!=l.end(); ++it) {
        delete *it;
    }
    scene->update();
}

void SnakeGame::showMessage(const QString &text)
{
#define DEFAULT_TEXT_SIZE 12

    if (!gamemessage) {
        gamemessage = new QGraphicsSimpleTextItem;
        gamemessage->setZValue(100);
        gamemessage->setBrush(QBrush(palette().color(QPalette::Normal, QPalette::HighlightedText)));
        scene->addItem(gamemessage);
    }
    if (!gamemessagebkg) {
        gamemessagebkg = new QGraphicsRectItem;
        gamemessagebkg->setZValue(99);
        gamemessagebkg->setBrush(palette().color(QPalette::Normal, QPalette::Highlight));
        scene->addItem(gamemessagebkg);
    }
    gamemessage->setText(text);

    int size = DEFAULT_TEXT_SIZE;
    QFont fnt( QApplication::font() );
    fnt.setPointSize( size );
    fnt.setBold( true );

    do {
        fnt.setPointSize( size );
        gamemessage->setFont( fnt );
    }
    while( ( gamemessage->boundingRect().width() > (scene->width()-15) ||
        gamemessage->boundingRect().height() > (scene->height()-15) ) && --size );

    int w = int(gamemessage->boundingRect().width());
    int h = int(gamemessage->boundingRect().height());
    gamemessage->setPos(scene->width()/2 -w/2, scene->height()/2 -h/2);
    gamemessagebkg->setRect(QRect(QPoint(int(scene->width()/2 -w/2 - 5), int(scene->height()/2 -h/2 - 5)), QSize(w+10, h+10)));
    gamemessagebkg->show();
    gamemessage->show();
    scene->update();
}

void SnakeGame::clearMessage()
{
    if (gamemessage) {
        delete gamemessage;
        gamemessage = 0;
    }
    if (gamemessagebkg) {
        delete gamemessagebkg;
        gamemessagebkg = 0;
    }
    scene->update();
}

void SnakeGame::gameOver()
{
    QSoftMenuBar::setLabel( this, Qt::Key_Select, "icons/play", tr("Play"));
    if (snake)
        showMessage( tr( "GAME OVER!\nYour Score: %1" ).arg( snake->getScore() ) );
    gamestopped = true;
    waitover = false;
    QTimer::singleShot(2000, this, SLOT(endWait()));
}

void SnakeGame::endWait()
{
    waitover = true;
    if (snake)
    {
        if(Qtopia::mousePreferred())
            showMessage(tr("GAME OVER!\nYour Score: %1\nClick to start\nnew game").arg(snake->getScore()));
        else
            showMessage(tr("GAME OVER!\nYour Score: %1\nPress Play\nkey to start\nnew game").arg(snake->getScore()));
    }
}

void SnakeGame::keyPressEvent(QKeyEvent* event)
{
    switch( event->key() ) {
    case Qt::Key_Select:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (paused) {
            resume();
        } else if (gamestopped) {
            if (waitover) newGame(true);
        } else {
            pause();
        }
        event->accept();
        break;

    default:
        if (!paused && !gamestopped && snake && snake->go( event->key() ))
            event->accept();
        else
            event->ignore();
        break;
    }
}

int orientation(const QPointF &p1, const QPointF &p2, const QPointF &point)
{
    qreal orient = ((p2.x() - p1.x()) * (point.y() - p1.y())) - ((point.x() - p1.x()) * (p2.y() - p1.y()));
    if(orient > 0.0)
        return 1;             // Orientaion is to the right-hand side
    else if(orient < 0.0)
        return -1;            // Orientaion is to the left-hand side
    else
        return 0;             // Orientaion is neutral aka collinear
}

bool pointInTriangle(const QPointF &t1, const QPointF &t2, const QPointF &t3, const QPointF &p)
{
    int orient1 = orientation(t1, t2, p);
    int orient2 = orientation(t2, t3, p);
    int orient3 = orientation(t3, t1, p);

    if (orient1 == orient2 && orient2 == orient3)
        return true;
    else if (orient1 == 0)
        return (orient2 == 0) || (orient3 == 0);
    else if (orient2 == 0)
        return (orient1 == 0) || (orient3 == 0);
    else if (orient3 == 0)
        return (orient2 == 0) || (orient1 == 0);
    else
        return false;
}

void SnakeGame::mousePressEvent(QMouseEvent* mouseEvent)
{
    if (!gamestopped && !paused && snake)
    {
        // calculate the quadrant.
        if(pointInTriangle(rect().topLeft(), rect().topRight(), rect().center(), mouseEvent->pos()))
        {
            if(snake->go(Qt::Key_Up))
                mouseEvent->accept();
        }
        else if(pointInTriangle(rect().topRight(), rect().bottomRight(), rect().center(), mouseEvent->pos()))
        {
            if(snake->go(Qt::Key_Right))
                mouseEvent->accept();
        }
        else if(pointInTriangle(rect().bottomLeft(), rect().bottomRight(), rect().center(), mouseEvent->pos()))
        {
            if(snake->go(Qt::Key_Down))
                mouseEvent->accept();
        }
        else if(pointInTriangle(rect().topLeft(), rect().bottomLeft(), rect().center(), mouseEvent->pos()))
        {
            if(snake->go(Qt::Key_Left))
                mouseEvent->accept();
        }
        // todo later, maybe give a bit of visual feedback of the quadrant pressed
    }
    else if ( paused ) {
        mouseEvent->accept();
        resume();
    }
    else if (gamestopped) {
        mouseEvent->accept();
        if (waitover)
            newGame(true);
    }
}

void SnakeGame::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if (!gamestopped && !paused && snake)
    {
        mouseEvent->accept();
        pause();
    }
}
