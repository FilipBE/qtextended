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

#include "view.h"

#include <QVBoxLayout>
#include <QtopiaApplication>

#include <math.h>

//#define REFRESH_DELAY          20
#define REFRESH_DELAY          33
#define TEXT_SPEED              2

/*!
  \class MyGraphicsView
  \internal

  \brief The MyGraphicsView class is a subclass of QGraphicsView,
         which was created for the sole purpose of allowing us to
         implement our own resizeEvent() function.

  We need to re-implement that function to get the widget's new
  geometry once it has been resized to fit the space available. We
  do this by calling the base class resizeEvent(), which processes
  the resize event generated when the widget was resized to fit
  the available space.

  After processing that event, we get the width() and height() of
  the resized widget from the base class and use them to construct
  a rectangle at (0,0), which we use to set the scene geometry of
  our QGraphicsScene.
 */


/*!
  \internal

  We need to re-implement this virtual function here to get the
  widget's new geometry once it has been resized to fit the space
  available. We do this by calling the base class resizeEvent(),
  which processes the resize event generated when the widget was
  resized to fit the available space.

  After processing that event, we get the width() and height() of
  the resized widget from the base class and use them to construct
  a rectangle at (0,0), which we use to set the scene geometry of
  our QGraphicsScene.
 */
void MyGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    scene()->setSceneRect(0,0,width(),height());
}

/*!
  \class KAsteroidsView
  \brief The KAsteroidsView class contains the asteroids game's
         use of the \l {The Graphics View Framework} API.
  \internal

  The class creates a \l {QGraphicsScene} {scene} and a
  \l {MyGraphicsView} {view} for the scene. It also loads the
  \l {KSprite} {sprites} and causes the \l {KShip} {ship} and
  the \l {KShield} {shield} to be created.
 */

/*!
  \internal
 */
KAsteroidsView::KAsteroidsView(QWidget* parent)
    : QWidget(parent)
{
    qsrand( QDateTime::currentDateTime().toTime_t() );
    scene_ = KSprite::scene();
    textSprite_ = new QGraphicsSimpleTextItem(0,scene_);
    textPending_ = false;

    view_ = new MyGraphicsView(scene_,this);
    view_->setFrameStyle(QFrame::NoFrame);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setInteractive(false);
    view_->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    view_->setOptimizationFlag(QGraphicsView::DontClipPainter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(view_);
    layout->setSpacing(0);
    layout->setMargin(0);

    KSprite::setView(this);

    shieldTimer_ = new QTimer(this);
    connect(shieldTimer_, SIGNAL(timeout()), this, SLOT(dropShield()));

    mainTimer_ = new QTimer(this);
    connect(mainTimer_, SIGNAL(timeout()), this, SLOT(mainTimerEvent()));
    mainTimer_->setInterval(REFRESH_DELAY);
    mainTimer_->setSingleShot(false);
    timerEventCount_ = 0;

    vitalsChanged_ = false;
    instruct_user_ = true;
    game_paused_ = true;
    QTimer::singleShot(1, this, SLOT(mainTimerEvent()));
}

/*!
  \internal

  The destructor does not delete the scene or the view. The
  view has this KAsteroidsView as its parent, so we assume
  it will be deleted during the normal Qt hierarchical exit
  scheme. The scene has no parent. We assume it is deleted
  during the same Qt exit sequence.
 */
KAsteroidsView::~KAsteroidsView()
{
    mainTimer_->stop();
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
}

/*!
  \internal
  Start a new game. Emits updateVitals(). Starts the timer
  that generates the events during which the game board is
  updated.
 */
void KAsteroidsView::newGame()
{
    KSprite::reset();
    game_paused_ = false;
    setCanPause(true);
    mainTimer_->start();
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
    emit updateVitals();
}

/*!
  \internal
  Put the game in its paused state if \a p is true,
  and take it out of its paused state if \a p is false.

  This is a bit misleading, because the actual pause is
  maintained by a QMessageBox waiting for the user to
  press the "OK" button it puts up. As soon as "OK" is
  pressed, this function is called again with \a p set
  to false.
 */
void KAsteroidsView::pause()
{
    if(!canPause())
        return;
    if (!game_paused_) {
        mainTimer_->stop();
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
    }
    else if (game_paused_) {
        mainTimer_->start();
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
    }

    game_paused_ = !game_paused_;

    if(game_paused_) {
        showText(tr("Game Paused\nPress pause button\nto continue"));
        textSprite_->show();
    } else {
        hideText();
    }
}

void KAsteroidsView::setPaused(bool paused)
{
    if (game_paused_ != paused)
        pause();
}

/*!
  If there is no ship, create a new ship and raise the
  shield in case their is a rock in the same place.
 */
void KAsteroidsView::newShip()
{
    setCanPause(true);
    if (!KSprite::ship()) {
	KSprite::newShip();
	raiseShield();
    }
}

/*!
  \internal
  Raise the ship's shield, if it can be raised, and set a
  timer to turn it off later.
 */
void KAsteroidsView::raiseShield()
{
    if (KSprite::shield()->raise()) {
	shieldTimer_->setSingleShot(true);
	shieldTimer_->start(1000);
    }
}

/*!
  \internal
  Drop the shield. The shield's hide() function is called,
  and the "shield is up" flag is turned off.
 */
void KAsteroidsView::dropShield()
{
    KSprite::shield()->drop();
}

/*!
  \internal
  Displays a \a text message in a certain \a color, which
  then scrolls off the screen if \a scroll is true. In the
  QTopia phone version, \a scroll is ignored.
 */
void KAsteroidsView::showText(const QString& text)
{
    textSprite_->setText(text);
    textSprite_->setBrush(Qt::yellow);
    qreal x = (scene_->width()-textSprite_->boundingRect().width())/2;
    qreal y = (scene_->height()-textSprite_->boundingRect().height())/4;
    textDy_ = 0;
    textSprite_->setPos(x,y);
    textPending_ = true;
}

/*!
  \internal
  Starts the text scrolling up so that it scrolls off the
  screen.
 */
void KAsteroidsView::hideText()
{
    textDy_ = -TEXT_SPEED;
}

/*!
  \internal
  Construct a bunch of messages for communicating instructions
  to the user.
 */
void KAsteroidsView::constructMessages(const QString& t)
{
    firstGameMessage_ =
	tr("Press %1\nto start game.","").arg(t);
    shipKilledMessage_ =
        tr("Ship Destroyed.\nPress %1\nfor next ship.", "").arg(t);
    gameOverMessage_ =
	tr("GAME OVER.\nPress %1\nfor new game.","").arg(t);
    nextGameMessage_ = firstGameMessage_;
}

/*!
  \internal
  Display a message telling the user how to start the game
  so he doesn't sit there feeling stupid.
 */
void KAsteroidsView::reportStartGame()
{
    showText(nextGameMessage_);
}

/*!
  \internal
  Display a message indicating the ship has been destroyed,
  as if it wasn't obvious from all the bits and pieces flying
  around.
 */
void KAsteroidsView::reportShipKilled()
{
    showText(shipKilledMessage_);
}

/*!
  \internal
  Display a message indicating the game is over.
 */
void KAsteroidsView::reportGameOver()
{
    showText(gameOverMessage_);
    QTimer::singleShot(60000, mainTimer_, SLOT(stop()));
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
}

/*!
  \internal
  This is where most of the game processing happens. It is
  called for each timer event. The scene's
  \l {QGraphicsScene::advance()} {advance function} is called
  to advance the animation one step for each graphics item in
  the scene, which is accomplished by calling
  \l KSprite::advance() for each sprite. Note that KSprite
  inherits \l QGraphicsPismapItem. Objects that age and die
  are handled as well. Each sprite is responsible for
  detecting and interpreting its collisions with other
  sprites.
 */
void
KAsteroidsView::mainTimerEvent()
{
    if (instruct_user_) {
	reportStartGame();
	instruct_user_ = false;
    }

    /*!
      This is where the game happens. The advance() function
      at the scene level calls the advance function for each
      graphics item on the game board. This includes the ship,
      the asteroids, the powerups, and the shield. Animation
      and collisions are handled in these advance() functions.
     */
    scene_->advance();

    if (KSprite::shipKilled()) {
	/*
	  KSprite::shipKilled() only returns true the first
	  time it is called after the ship is destroyed.
	*/
	emit shipKilled();
	vitalsChanged_ = true;
    }

    if (textPending_ && !KFragment::exploding()) {
	textSprite_->show();
	textPending_ = false;
    }
    if (textSprite_->isVisible()) {
	QRectF br = textSprite_->sceneBoundingRect();
	if (br.y() <= -br.height()) {
            textSprite_->hide();
	    textDy_ = 0;
        }
        else if (textDy_)
            textSprite_->moveBy(0,textDy_);
    }

    if (KRock::allDestroyed() && !KSprite::dying())
        emit allRocksDestroyed();
    if (vitalsChanged_ && !(timerEventCount_ % 10)) {
        emit updateVitals();
        vitalsChanged_ = false;
    }
    timerEventCount_++;
    //view_->update();
}

/*!
  Emit the missileFired() signal to tell the game to play
  the launching missile sound.
 */
void
KAsteroidsView::reportMissileFired()
{
    emit missileFired();
}

/*!
  Emit the updateScore() signal with \a rock_size as the
  parameter.
 */
void
KAsteroidsView::reportRockDestroyed(int rock_size)
{
    emit updateScore(rock_size);
}
