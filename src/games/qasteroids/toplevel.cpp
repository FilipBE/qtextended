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

#include "toplevel.h"
#include "ledmeter.h"

#include <qtopiaapplication.h>
#include <qdevicebuttonmanager.h>
#include <qsoftmenubar.h>

#include <QMenu>
#include <QKeyEvent>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>
#include <QDesktopWidget>

struct GameLevelStruct
{
    int    nrocks_;
    double rockSpeed_;
};

#define MAX_GAME_LEVELS      16

GameLevelStruct gameLevels[MAX_GAME_LEVELS] =
{
    { 1, 0.2  + 0.2 },
    { 1, 0.3  + 0.2 },
    { 1, 0.25 + 0.2 },
    { 1, 0.35 + 0.2 },
    { 2, 0.4  + 0.2 },
    { 2, 0.3  + 0.2 },
    { 2, 0.35 + 0.2 },
    { 2, 0.4  + 0.2 },
    { 3, 0.3  + 0.2 },
    { 3, 0.35 + 0.2 },
    { 3, 0.4  + 0.2 },
    { 3, 0.35 + 0.2 },
    { 4, 0.4  + 0.2 },
    { 4, 0.45 + 0.2 },
    { 4, 0.5  + 0.2 }
};


class RowWidget : public QWidget
{
    Q_OBJECT

  public:
    RowWidget(QWidget* parent);

  protected:
    void paintEvent(QPaintEvent* event);
};

RowWidget::RowWidget(QWidget* parent)
    : QWidget(parent)
{
    setPalette(parent->palette());
    setBackgroundRole(QPalette::Window);
}

void RowWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(),Qt::black);
}

/*!
  \internal
  \class KAstTopLevel

  \brief Class KAstTopLevel provides the implementation of
         the main window for the Asteroids game.

  It constructs all the widgets and lays them out. The main
  widget it constructs is the single instance of
  \l {KAsteroidsView} {KAsteroidsView}.

 */

/*!
  The standard constructor creates and lays out all the
  widgets. There is a top row widget for displaying data
  values, a bottom row widget for displaying data values,
  and a large view widget in the middle where the game is
  animated and played.
 */
KAstTopLevel::KAstTopLevel(QWidget* parent, Qt::WFlags fl)
    : QMainWindow(parent,fl),
      view_(0),
      scoreLCD_(0),
      levelLCD_(0),
      shipsLCD_(0),
      teleportsLCD_(0),
      brakesLCD_(0),
      shieldLCD_(0),
      shootLCD_(0),
      powerMeter_(0),
      shipDestroyed(":sound/qasteroids/shipdestroyed"),
      rockDestroyed(":sound/qasteroids/rockdestroyed"),
      missileFired(":sound/qasteroids/missilefired"),
      gameEnded_(false),
      shipCount_(0),
      score_(0),
      currentLevel_(0)
{
    // This call was an empty function and has now been removed
    // QtopiaApplication::grabKeyboard();

    setWindowTitle(tr("Asteroids"));
    QPalette p = buildPalette();
    QWidget* mainWin = new QWidget(this);
    setCentralWidget(mainWin);
    mainWin->setPalette(p);

    /*
      Give that "main" widget a vertical box layout. To the
      vertical layout, add a top horizontal layout, the game
      view graphics widget, and a bottom horizontal layout.
     */
    QVBoxLayout* vb = new QVBoxLayout(mainWin);
    vb->setSpacing(0);
    vb->setMargin(0);
    vb->addWidget(buildTopRow(mainWin));
    view_ = buildAsteroidsView(mainWin);
    QSizePolicy policy( QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding );
    view_->setSizePolicy( policy );
    vb->addWidget(view_,10);
    vb->addWidget(buildBottomRow(mainWin));

    // These are for debugging purposes only
    //actions_.insert(Qt::Key_4,Populate_Rocks);
    //actions_.insert(Qt::Key_6,Populate_Powerups);

    //actions_.insert(Qt::Key_0,Launch);
    actions_.insert(Qt::Key_Up,Thrust);
    actions_.insert(Qt::Key_Left,RotateLeft);
    actions_.insert(Qt::Key_Right,RotateRight);
    actions_.insert(Qt::Key_Down,Brake);

    actions_.insert(Qt::Key_NumberSign,Teleport);
    actions_.insert(Qt::Key_Asterisk,Pause);
    actions_.insert(Qt::Key_Select,Shoot);
    actions_.insert(Qt::Key_Space,Shoot);
    actions_.insert(Qt::Key_0,Shield);
    /*QSoftMenuBar::setLabel(this,
                           Qt::Key_0,
                           "qasteroids/ship/ship0000",
                           tr("Launch"));*/
    contextMenu_ = QSoftMenuBar::menuFor(this);

    connect(contextMenu_, SIGNAL(aboutToShow()), view_, SLOT(setPaused()));

    QString s = tr("Select (OK)");
    view_->constructMessages(s);
    view_->setCanPause(false);
    setFocusPolicy(Qt::StrongFocus);
}

/*!
  The destructor has nothing to do.
 */
KAstTopLevel::~KAstTopLevel()
{
    // nothing.
}

/*!
  Construct the palette for the asteroids game. This can
  probably be improved.
 */
QPalette KAstTopLevel::buildPalette()
{
    QBrush foreground(Qt::darkGreen);
    QBrush background(Qt::black);
    QBrush light(QColor(128,128,128));
    QBrush dark(QColor(64,64,64));
    QBrush mid(Qt::black);
    QBrush text(Qt::darkGreen);
    QBrush base(Qt::black);
    QPalette p(foreground,
	       background,
	       light,
	       dark,
	       mid,
	       text,
	       text,
	       base,
	       background);
    return p;
}

/*!
  Build a widget containing the top row of widgets for the
  asteroids game, and return a pointer to it. The top row
  widget contains several widgets laid out horizontally.
  Use the palette from the \a parent. Each widget we add
  becomes a child of the \a parent.
 */
QWidget*
KAstTopLevel::buildTopRow(QWidget* parent)
{
    QWidget* w = new RowWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(w);
    layout->setSpacing(0);
    layout->setMargin(0);

    QPalette palette = parent->palette();
    QFont labelFont(QApplication::font());
    labelFont.setPointSize(5);
    QLabel* label = 0;

    int layoutSpace = qApp->desktop()->width() > 200 ? 5 : 3;
    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/teleport"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    teleportsLCD_ = new QLCDNumber(1,parent);
    teleportsLCD_->setFrameStyle(QFrame::NoFrame);
    teleportsLCD_->setSegmentStyle(QLCDNumber::Flat);
    teleportsLCD_->setPalette(palette);
    teleportsLCD_->setFixedHeight(16);
    layout->addWidget(teleportsLCD_);
    layout->addStretch(1);

    /*
      Add an LCD widget to show the current game score. Give
      it five digits, because some players are very good.
     */
    scoreLCD_ = new QLCDNumber(5,parent);
    scoreLCD_->setFrameStyle(QFrame::NoFrame);
    scoreLCD_->setSegmentStyle(QLCDNumber::Flat);
    scoreLCD_->setFixedHeight(16);
    scoreLCD_->setPalette(palette);
    layout->addWidget(scoreLCD_);
    layout->addStretch(1);

    /*
      Add a label widget to for the game level LCD. The label
      text is "Level." Then add the game level LCD widget, with
      two digits. It can only go to 16.
     */
    label = new QLabel(tr("Lvl", "short for level"),parent);
    label->setFont(labelFont);
    label->setPalette(palette);
    layout->addWidget(label);
    levelLCD_ = new QLCDNumber(2,parent);
    levelLCD_->setFrameStyle(QFrame::NoFrame);
    levelLCD_->setSegmentStyle(QLCDNumber::Flat);
    levelLCD_->setFixedHeight(16);
    levelLCD_->setPalette(palette);
    layout->addWidget(levelLCD_);
    layout->addStretch(1);


    /*
      Add a label widget for the ship count LCD. Then add the ship
      count LCD with a single digit. The maximum number of ships is 3.
     */
    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/ship/ship0000"));
    label->setFont(labelFont);
    label->setPalette(palette);
    layout->addWidget(label);
    shipsLCD_ = new QLCDNumber(1,parent);
    shipsLCD_->setFrameStyle(QFrame::NoFrame);
    shipsLCD_->setSegmentStyle(QLCDNumber::Flat);
    shipsLCD_->setFixedHeight(16);
    shipsLCD_->setPalette(palette);
    layout->addWidget(shipsLCD_);

    return w;
}

/*!
  Build the asteroids game view widget as a child of the
  \a parent. Connect all its signals to slots in the top
  level class instance.
 */
KAsteroidsView* KAstTopLevel::buildAsteroidsView(QWidget* parent)
{
    KAsteroidsView* v = new KAsteroidsView(parent);
    connect(v,SIGNAL(shipKilled()),SLOT(slotShipKilled()));
    connect(v,SIGNAL(missileFired()),SLOT(slotMissileFired()));
    connect(v,SIGNAL(updateScore(int)),SLOT(slotUpdateScore(int)));
    connect(v,SIGNAL(allRocksDestroyed()),SLOT(slotNewGameLevel()));
    connect(v,SIGNAL(updateVitals()),SLOT(slotUpdateVitals()));
    return v;
}

/*!
  Build a widget containing the bottom row of widgets for
  the asteroids game, and return a pointer to it. The bottom
  row widget contains several widgets laid out horizontally.
  Use the palette from the \a parent. Each widget we add
  becomes a child of the \a parent.
 */
QWidget*
KAstTopLevel::buildBottomRow(QWidget* parent)
{
    QWidget* w = new RowWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(w);
    layout->setSpacing(0);
    layout->setMargin(0);

    QPalette palette = parent->palette();
    QFont labelFont(QApplication::font());
    labelFont.setPointSize(6);
    QLabel* label = 0;

    int layoutSpace = qApp->desktop()->width() > 200 ? 5 : 3;
    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/brake"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    brakesLCD_ = new QLCDNumber(1,parent);
    brakesLCD_->setFrameStyle(QFrame::NoFrame);
    brakesLCD_->setSegmentStyle(QLCDNumber::Flat);
    brakesLCD_->setPalette(palette);
    brakesLCD_->setFixedHeight(16);
    layout->addWidget(brakesLCD_);

    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/shield"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    shieldLCD_ = new QLCDNumber(1,parent);
    shieldLCD_->setFrameStyle(QFrame::NoFrame);
    shieldLCD_->setSegmentStyle(QLCDNumber::Flat);
    shieldLCD_->setPalette(palette);
    shieldLCD_->setFixedHeight(16);
    layout->addWidget(shieldLCD_);

    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/shoot"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    shootLCD_ = new QLCDNumber(1,parent);
    shootLCD_->setFrameStyle(QFrame::NoFrame);
    shootLCD_->setSegmentStyle(QLCDNumber::Flat);
    shootLCD_->setPalette(palette);
    shootLCD_->setFixedHeight(16);
    layout->addWidget(shootLCD_);
    layout->addStretch(1);

    powerMeter_ = new KALedMeter(parent);
    powerMeter_->setFrameStyle(QFrame::Box | QFrame::Plain);
    powerMeter_->setMaxRawValue(MAX_SHIP_POWER_LEVEL);
    powerMeter_->setMeterLevels(qApp->desktop()->width() > 200 ? 15 : 10);
    powerMeter_->setPalette(palette);
    powerMeter_->setFixedSize(qApp->desktop()->width() > 200 ? 40 : 40, 12);
    layout->addWidget(powerMeter_);
    return w;
}

/*!
  If the event is for a key not used by the game, ignore it,
  ie allow it to be consumed higher up the food chain. If it
  is a key used by the game, but it is an auto repeat key,
  accept the event but don't actually do anything with it.

  So this function can both ignore and accept an event, and
  they both mean the event is consumed. If this function
  returns true, it means the event has been consumed and the
  caller should do nothing with it, not even accept it or
  ignore it.
 */
bool
KAstTopLevel::eventConsumed(QKeyEvent* e) const
{
    if (!actions_.contains(e->key())) {
        e->ignore();
        return true;
    }
    if (e->isAutoRepeat()) {
        e->accept();
        return true;
    }
    return false;
}

/*!
  Process each key press event. Key presses that are meant
  for the game are accepted whether they are acted on or not.
 */
void
KAstTopLevel::keyPressEvent(QKeyEvent* event)
{
    if (eventConsumed(event))
        return;

    Action a = actions_[ event->key() ];

    switch (a) {
    case RotateLeft:
        if (KSprite::ship())
            KSprite::ship()->rotateLeft(true);
            break;

    case RotateRight:
        if (KSprite::ship())
            KSprite::ship()->rotateRight(true);
        break;

    case Thrust:
        if (KSprite::ship())
            KSprite::ship()->startEngine();
        break;

    case Shoot:
        if (KSprite::ship()) {
            KSprite::ship()->startShooting();
            break;
        }

    case Launch:
        if (!shipCount_ && !KFragment::exploding()) {
            startNewGame();
        } else if (!KSprite::ship() && !KFragment::exploding()) {
            view_->newShip();
            view_->hideText();
        } else {
            event->ignore();
            return;
        }
        break;

    case NewGame:
        startNewGame();
        break;

    case Shield:
        if (KSprite::ship())
            view_->raiseShield();
        break;

    case Teleport:
        if (KSprite::ship())
            KSprite::ship()->teleport();
        break;

    case Brake:
        if (KSprite::ship())
            KSprite::ship()->startBraking();
        break;

    case Populate_Powerups:
        populatePowerups();
        break;

    case Populate_Rocks:
        populateRocks();
        break;

    case Pause:
        view_->pause();
        break;

    default:
        event->ignore();
        return;
    }
    event->accept();
}

/*!
  Process each key release event. Key presses that are meant
  for the game are accepted whether they are acted on or not.
 */
void
KAstTopLevel::keyReleaseEvent(QKeyEvent* event)
{
    if (eventConsumed(event))
        return;

    Action a = actions_[ event->key() ];

    switch (a) {
    case RotateLeft:
        if (KSprite::ship())
            KSprite::ship()->rotateLeft(false);
        break;

    case RotateRight:
        if (KSprite::ship())
            KSprite::ship()->rotateRight(false);
        break;

    case Thrust:
        if (KSprite::ship())
            KSprite::ship()->stopEngine();
        break;

    case Brake:
        if (KSprite::ship())
            KSprite::ship()->stopBraking();
        break;

    case Shield:
        break;

    case Teleport:
        break;

    case Shoot:
        if (KSprite::ship())
            KSprite::ship()->stopShooting();
        break;

    default:
        event->ignore();
        return;
    }

    event->accept();
}

void KAstTopLevel::populatePowerups()
{
    if (!KSprite::ship())
	return;
    qreal x = KSprite::ship()->x();
    qreal y = KSprite::ship()->y();
    qreal vx = KSprite::ship()->velocityX();
    qreal vy = KSprite::ship()->velocityY();
    for (int i=0; i<5; ++i) {
	KPowerup* new_pup = new KEnergyPowerup();
	double r = (0.5 - KSprite::randDouble()) * 4.0;
	new_pup->setPos(x+(i*r+r),y+(i*r+r));
	new_pup->setVelocity(vx+(i*r+r),vy+(i*r+r));
	new_pup->show();
	new_pup->wrap();
	new_pup = new KTeleportPowerup();
	r = (0.5 - KSprite::randDouble()) * 4.0;
	new_pup->setPos(x+(i*r+r),y+(i*r+r));
	new_pup->setVelocity(vx+(i*r+r),vy+(i*r+r));
	new_pup->show();
	new_pup->wrap();
	new_pup = new KBrakePowerup();
	r = (0.5 - KSprite::randDouble()) * 4.0;
	new_pup->setPos(x+(i*r+r),y+(i*r+r));
	new_pup->setVelocity(vx+(i*r+r),vy+(i*r+r));
	new_pup->show();
	new_pup->wrap();
	new_pup = new KShieldPowerup();
	r = (0.5 - KSprite::randDouble()) * 4.0;
	new_pup->setPos(x+(i*r+r),y+(i*r+r));
	new_pup->setVelocity(vx+(i*r+r),vy+(i*r+r));
	new_pup->show();
	new_pup->wrap();
	new_pup = new KShootPowerup();
	r = (0.5 - KSprite::randDouble()) * 4.0;
	new_pup->setPos(x+(i*r+r),y+(i*r+r));
	new_pup->setVelocity(vx+(i*r+r),vy+r);
	new_pup->show();
	new_pup->wrap();
    }
}

void KAstTopLevel::populateRocks()
{
    if (!KSprite::ship())
	return;
    static double x_multiplier[4] = { 1.0, 1.0, -1.0, -1.0 };
    static double y_multiplier[4] = { -1.0, 1.0, -1.0, 1.0 };

    double dx = 0.25;
    double dy = 0.25;
    for (int i = 0; i < 20; i++) {
	double r = 0.25/2 - (KSprite::randDouble() * 0.25);
	KRock* newRock = new KSmallRock();
	qreal x = KSprite::ship()->x();
	qreal y = KSprite::ship()->y();
	newRock->setPos(x + (x_multiplier[i%4] * 10 * KSprite::randInt(10)),
			y + (y_multiplier[i%4] * 10 * KSprite::randInt(10)));
	newRock->setVelocity(dx + (x_multiplier[i%4] * 0.25) + r + 5.0,
			     dy + (y_multiplier[i%4] * 0.25) + r + 5.0);
	newRock->setImage(KSprite::randInt(32));
	newRock->show();
	newRock->wrap();
    }
}

/*!
  Handles the Show event \a e. Calls the base class's
  showEvent() function with \a e and resumes the game,
  ie takes the game out of its paused state. Gives the
  game the keyboard input focus.
 */
void KAstTopLevel::showEvent(QShowEvent* e)
{
    QMainWindow::showEvent(e);
    if(view_->isPaused() && view_->canPause())
        view_->pause();
    setFocus();
}

/*!
  Handles the Hide event \a e. Calls the base class's
  hideEvent() function with \a e and puts the game into
  its paused state.
 */
void KAstTopLevel::hideEvent(QHideEvent* e)
{
    QMainWindow::hideEvent(e);
    if(!view_->isPaused() && view_->canPause())
        view_->pause();
}

/*!
  Takes the game in or out of its paused state and gives the
  game the keyboard input focus.
 */
bool KAstTopLevel::event( QEvent * e )
{
    switch(e->type())
    {
        case QEvent::WindowActivate:
            if(view_->isPaused() && view_->canPause())
                view_->pause();
            setFocus();
            break;
        case QEvent::WindowDeactivate:
            if(!view_->isPaused() && view_->canPause())
                view_->pause();
            break;
        default:
            break;
    }
    return QMainWindow::event(e);
}

/*!
  This function is called to start a new game.
 */
void KAstTopLevel::startNewGame()
{
#if 0
    QSoftMenuBar::setLabel(this,
                           Qt::Key_Select,
                           "qasteroids/powerups/shoot",
                           tr("Shoot"));
#endif
    gameEnded_ = false;
    shipCount_ = 3;
    score_ = 0;
    scoreLCD_->display(0);
    currentLevel_ = 0;
    levelLCD_->display(currentLevel_+1);
    shipsLCD_->display(shipCount_);
    view_->newGame();
    KRock::setRockSpeed(gameLevels[0].rockSpeed_);
    KRock::createRocks(gameLevels[0].nrocks_);
    view_->newShip();
    view_->hideText();
}

/*!
  This slot is called to advance to the next game level, once
  all the rocks at the current level have been destroyed. The
  game level count is incremented if we haven't already reached
  the maximum. The new rock speed is set, and then the required
  number of rocks for the new level are created. The game level
  LCD is updated to show the new level.
 */
void KAstTopLevel::slotNewGameLevel()
{
    currentLevel_++;

    if (currentLevel_ >= MAX_GAME_LEVELS)
        currentLevel_ = MAX_GAME_LEVELS - 1;

    KRock::reset();
    KRock::setRockSpeed(gameLevels[currentLevel_-1].rockSpeed_);
    KRock::createRocks(gameLevels[currentLevel_-1].nrocks_);

    levelLCD_->display(currentLevel_+1);
}

/*!
  Plays a sound indicating a missile was fired.
 */
void KAstTopLevel::slotMissileFired()
{
    missileFired.play();
}

/*!
  Decrements the ships remaining count and displays the new
  count. Plays a sound indicating a ship was destroyed.

  If any ships remain, the ship killed message is reported.
  Otherwise, the game over message is reported, and the left
  button of the context menu is cleared.
 */
void KAstTopLevel::slotShipKilled()
{
    shipCount_--;
    shipsLCD_->display(shipCount_);

    shipDestroyed.play();

    if (shipCount_ > 0)
	view_->reportShipKilled();
    else {
        view_->setCanPause(false);
        endGame();
        reportStatistics();
    }
    QSoftMenuBar::setLabel(this,
                           Qt::Key_0,
                           "qasteroids/ship/ship0000",
                           tr("Launch"));
    //QSoftMenuBar::clearLabel(this,Qt::Key_Context1);
    if (!contextMenu_)
        contextMenu_ = QSoftMenuBar::menuFor(this);
}

/*!
  A private function that returns true if a game that started
  has now ended.
  \internal.
 */
bool KAstTopLevel::gameEnded() const
{
    return gameEnded_;
}

/*!
  A private function that tells the game to end.
  \internal
 */
void KAstTopLevel::endGame()
{
    KSprite::markDying(true);
}

/*!
  This slot is called whenever the updateScore signal is
  emitted. At the moment, the only time an updateScore signal
  is emitted is when a missile destroys an asteroid. \a key
  specifies the size of the rock that was hit by the missile.

  This slot increments the score by an appropriate amount
  and displays the updated score. It also plays the rock
  destroyed sound.
 */
void KAstTopLevel::slotUpdateScore(int key)
{
    score_ += 10 * key;
    rockDestroyed.play();
    scoreLCD_->display(score_);
}

/*!
  This function is called when the game is over. It computes
  a rock shooting efficiency number but does nothing with it.
  It reports the game over message.
 */
void KAstTopLevel::reportStatistics()
{
    QString r("0.00");
    if (KMissile::shotsFired()) {
	double d =
	    (double)KRock::rocksDestroyed() / KMissile::shotsFired() * 100.0;
	r = QString::number(d,'g',2);
    }

    view_->reportGameOver();
}

/*!
  This slot function updates all the LCD widgets when the
  updateVitals() signal is emitted.
 */
void KAstTopLevel::slotUpdateVitals()
{
    if (KSprite::ship()) {
	brakesLCD_->display(KSprite::ship()->brakeForce());
	shieldLCD_->display(KSprite::shield()->strength());
	shootLCD_->display(KSprite::ship()->firePower());
	teleportsLCD_->display(KSprite::ship()->teleportCount());
	powerMeter_->setValue(KSprite::ship()->powerLevel());
    }
    else {
	brakesLCD_->display(0);
	shieldLCD_->display(0);
	shootLCD_->display(0);
	teleportsLCD_->display(0);
	powerMeter_->setValue(MAX_SHIP_POWER_LEVEL);
    }
}

#include <toplevel.moc>
