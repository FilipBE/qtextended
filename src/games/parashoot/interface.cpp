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
#include "codes.h"
#include "man.h"
#include "base.h"
#include "cannon.h"
#include "bullet.h"
#include "helicopter.h"

#include <QTimer>
#include <QtopiaApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolBar>
#include <QMenu>
#include <QSoftMenuBar>
#include <QLabel>
#include <QStyle>
#include <QSound>
#include <QKeyEvent>

ParaShoot::ParaShoot(QWidget* parent, Qt::WFlags f) :
    QMainWindow(parent,f),
    kaboom(":sound/parashoot/landmine"),
    ohdear(":sound/parashoot/crmble01"),
    bang(":sound/parashoot/collide01"),
    fanfare(":sound/level_up"),
    splat1(":sound/parashoot/lose"),
    splat2(":sound/parashoot/bang"),
    cannon(NULL),
    base(NULL),
    updatespeed(80),
    gamestopped(true),
    finished(true),
    waitover(true),
    score(0),
    gamemessage(NULL),
    gamemessagebkg(NULL)
{
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setFocusPolicy(Qt::NoFocus);
    //view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    //view->setOptimizationFlag(QGraphicsView::DontClipPainter);
    QtopiaApplication::setInputMethodHint( this, QtopiaApplication::AlwaysOff );
    advanceTimer=new QTimer(this);
    advanceTimer->setInterval(updatespeed);
    advanceTimer->setSingleShot(false);
    connect(advanceTimer, SIGNAL(timeout()), scene, SLOT(advance()));

    QToolBar* toolbar = new QToolBar(this);
    toolbar->setMovable( false );
    addToolBar(toolbar);

    setWindowTitle( tr("ParaShoot") );

    QMenu *menu = QSoftMenuBar::menuFor( this );
    QAction *action = menu->addAction( QIcon(":image/parashoot/ParaShoot"), tr("New Game"), this, SLOT(newGame()));
    action->setWhatsThis( tr("Start a new game") );
    action->setEnabled( true );

    levelscore = new QLabel(toolbar);
    levelscore->setBackgroundRole( QPalette::Button );
    levelscore->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    toolbar->addWidget( levelscore );
    showScore(0,0);

    setCentralWidget(view);

    autoDropTimer = new QTimer(this);
    connect (autoDropTimer, SIGNAL(timeout()), this, SLOT(play()) );

    pauseTimer = new QTimer(this);
    connect(pauseTimer, SIGNAL(timeout()), this, SLOT(wait()) );

    setFocusPolicy(Qt::StrongFocus);
    showMessage( tr( "Press select\nfor new game") );
}

ParaShoot::~ParaShoot()
{
}

void ParaShoot::resizeEvent(QResizeEvent *)
{
    QSize s = centralWidget()->size();
    int offset = style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) + 2;
    scene->setSceneRect(offset, offset, s.width() - offset, s.height() - offset);

    QPixmap bgpixmap = QPixmap(":image/parashoot/sky").scaled((int)scene->width(),
        (int)scene->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QPalette palette;
    palette.setBrush( view->backgroundRole(), QBrush(bgpixmap) );
    view->setPalette( palette );

    if (base) {
        base->reposition();
    }

    if (cannon) {
        cannon->reposition();
    }
    if(gamemessage != NULL) {
        showMessage(gamemessage->text());
    }
}

bool ParaShoot::event( QEvent * e )
{
    switch(e->type())
    {
        case QEvent::WindowActivate:
            if ( gamestopped )
            {
                advanceTimer->setInterval(updatespeed);
                advanceTimer->start();
                gamestopped = false;
            }
            break;
        case QEvent::WindowDeactivate:
            if ( !gamestopped )
            {
                advanceTimer->stop();
                gamestopped = true;
            }
            break;
        default:
            break;
    }
    return QMainWindow::event(e);
}


void ParaShoot::showScore( int score, int level )
{
    levelscore->setText(tr("     Level: %1       Score: %2   ").arg(score).arg(level) );
}


void ParaShoot::newGame()
{
    if (pauseTimer->isActive())
        pauseTimer->stop();
    clear();
    Man::setManCount(0);
    score = 0;
    Bullet::setShotCount(0);
    Man::setKillCount(0);
    Helicopter::setKillCount(0);
    Bullet::setNobullets(0);
    nomen = 2;
    Bullet::setLimit(nomen);
    level = 0;
    updatespeed = 80;
    showScore(0,0);
    finished = gamestopped = false;
    Helicopter::deleteAll();
    waitover = true;
    base = new Base(scene);
    cannon = new Cannon(scene);
    connect( cannon, SIGNAL(score(int)), this, SLOT(increaseScore(int)));
    autoDropTimer->start(100);
    advanceTimer->start(updatespeed);
}


void ParaShoot::clear()
{
   advanceTimer->stop();
   autoDropTimer->stop();
   clearMessage();
   QList<QGraphicsItem *> l = scene->items();
   for (QList<QGraphicsItem *>::Iterator it=l.begin(); it!=l.end(); ++it) {
        scene->removeItem(*it);
        delete *it;
   }
}

void ParaShoot::gameOver()
{
    autoDropTimer->stop();
    advanceTimer->stop();
    Helicopter::silenceAll();

    int shots = Bullet::getShotCount();
    int shotsFired = cannon->shotsFired();
    if ( shotsFired == 0 )
        shotsFired = 1;

    showMessage(
        tr( "GAME OVER!\n"
        "Your Score: %1\n"
        "Parachuters Killed: %2\n"
        "Helicopters Killed: %3\n"
        "Accuracy: %4%" )
        .arg( score ).arg( Man::killCount() ).arg( Helicopter::killCount() ).arg( shots * 100 / shotsFired ) );
    finished = gamestopped = true;
    waitover = false;
    pauseTimer->start(3000);
}

void ParaShoot::wait()
{
    waitover = true;
    pauseTimer->stop();

    int shots = Bullet::getShotCount();
    int shotsFired = cannon->shotsFired();
    if ( shotsFired == 0 )
        shotsFired = 1;

    showMessage(
        tr( "GAME OVER!\n"
        "Your Score: %1\n"
        "Parachuters Killed: %2\n"
        "Helicopters Killed: %3\n"
        "Accuracy: %4%\n"
        "Press select\n"
        "for new game")
        .arg( score ).arg( Man::killCount() ).arg( Helicopter::killCount() ).arg( shots * 100 / shotsFired ) );
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
}

void ParaShoot::play()
{
     QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
     if (Man::getManCount() < nomen ) {
          new Man(scene);
     }
     if (Base::baseDestroyed()) {
          gameOver();
          return;
     }
}

void ParaShoot::increaseScore(int x)
{
    score += x;
    if ( score / 150 != (score-x) / 150 )
       levelUp();
    showScore(level,score);
}

void ParaShoot::levelUp()
{
    level++;
    int stage = level % 3;
    switch(stage) {
      case 0:
        nomen++;
        Bullet::setLimit(nomen);
        fanfare.play();
        break;
      case 1:
        new Helicopter(scene);
        fanfare.play();
        break;
      case 2:
        moveFaster();
        fanfare.play();
        break;
      default: return;
    }
}

void ParaShoot::moveFaster()
{
   if (updatespeed > 50)
       updatespeed = updatespeed-5;
   else
       updatespeed = updatespeed-3;
   advanceTimer->setInterval(updatespeed);
}

void ParaShoot::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Back || event->key() == Qt::Key_No)
        QMainWindow::keyPressEvent(event);
    if ( gamestopped || finished ) {
        if ( finished && waitover )
            newGame();
        else
            return;
    } else {
        switch(event->key()) {
          case Qt::Key_Up:
          case Qt::Key_F1:
          case Qt::Key_F9:
          case Qt::Key_Space:
          case Qt::Key_Select:
            cannon->shoot();
            break;
          case Qt::Key_Left:
            cannon->pointCannon(Cannon::Left);
            lastcannonkey=Qt::Key_Left;
            break;
          case Qt::Key_Right:
            cannon->pointCannon(Cannon::Right);
            lastcannonkey=Qt::Key_Right;
            break;
          default:
            return;
        }
    }
}

void ParaShoot::keyReleaseEvent(QKeyEvent* event)
{
    if ( lastcannonkey == event->key() )
        cannon->pointCannon(Cannon::NoDir);
}

void ParaShoot::showMessage(const QString &text)
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
    while( ( gamemessage->boundingRect().width() > (scene->width()-15.0) ||
        gamemessage->boundingRect().height() > (scene->height()-15.0) ) && --size );

    qreal w = gamemessage->boundingRect().width();
    qreal h = gamemessage->boundingRect().height();
    qreal left=(scene->width() - w)/2.0;
    qreal top=(scene->height() - h)/2.0;
    gamemessage->setPos(left, top);
    gamemessagebkg->setRect(QRectF(QPointF(left - 5.0, top - 5.0), QSizeF(w+10.0, h+10.0)));
    gamemessagebkg->show();
    gamemessage->show();
    scene->update();
}

void ParaShoot::clearMessage()
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

ParaShoot *ParaShoot::instance()
{
    return qobject_cast<ParaShoot *>(QtopiaApplication::instance()->mainWidget());
}

