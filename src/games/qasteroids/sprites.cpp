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

#include "sprites.h"
#include <QGraphicsScene>
#include <QBitmap>
#include "view.h"

#include <stdlib.h>
#include <math.h>

#define PI_X_2                  6.283185307
#ifndef M_PI
#define M_PI			3.141592654
#endif

#define BRAKE_ON_COST           4
#define FRAG_IMAGE_COUNT       16
#define MAX_BRAKING_FORCE	5
#define MAX_FIREPOWER		5
#define MAX_SHIELD_STRENGTH	5
#define MAX_SHIP_SPEED          8
#define MISSILE_SPEED          10.0
//#define ROCK_IMAGE_COUNT        1
#define ROCK_IMAGE_COUNT       32
#define ROCK_SPEED_MULTIPLIER  10.0
//#define ROCK_SPEED_MULTIPLIER   2.5
#define ROTATION_RATE           2
#define SHIELD_HIT_COST        30
#define SHIELD_IMAGE_COUNT	6
#define SHIELD_ON_COST          1
#define SHIP_IMAGE_COUNT       32
#define SHIP_STEPS             64

int KMissile::missiles_ = 0;
int KMissile::shotsFired_ = 0;

int KFragment::count_ = 0;

int KRock::rocksCreated_ = 0;
int KRock::rocksDestroyed_ = 0;
bool KRock::allRocksDestroyed_ = false;
double KRock::rockSpeed_ = 0.0;

bool KSprite::dying_ = false;
bool KSprite::shipKilled_ = false;
bool KSprite::spritesLoaded_ = false;
KShip* KSprite::ship_ = 0;
KShield* KSprite::shield_ = 0;
QGraphicsScene*	KSprite::scene_ = 0;
KAsteroidsView*	KSprite::view_ = 0;

QMap<int,QList<KSprite::Frame> > KSprite::shapemap_;

static struct
{
    int	id_;
    const char* path_;
    int	frames_;
} animations_[] =
{
//  { ID_ROCK_LARGE,       "rock1/rock1\%1.png",    ROCK_IMAGE_COUNT },
    { ID_ROCK_MEDIUM,      "rock2/rock2\%1.png",    ROCK_IMAGE_COUNT },
    { ID_ROCK_SMALL,       "rock3/rock3\%1.png",    ROCK_IMAGE_COUNT },
    { ID_SHIP,             "ship/ship\%1.png",      SHIP_IMAGE_COUNT },
    { ID_MISSILE,          "missile/missile.png",   0 },
    { ID_FRAGMENT,         "bits/bits\%1.png",      FRAG_IMAGE_COUNT },
    { ID_EXHAUST,          "exhaust/exhaust.png",   0 },
    { ID_ENERGY_POWERUP,   "powerups/energy.png",   0 },
    { ID_TELEPORT_POWERUP, "powerups/teleport.png", 0 },
    { ID_BRAKE_POWERUP,    "powerups/brake.png",    0 },
    { ID_SHIELD_POWERUP,   "powerups/shield.png",   0 },
    { ID_SHOOT_POWERUP,    "powerups/shoot.png",    0 },
    { ID_SHIELD,           "shield/shield\%1.png",  SHIELD_IMAGE_COUNT },
    { 0,                   0,                       0 }
};

/*! \class KSprite
  \brief The KSprite class is the base class for all the
         animated elements that appear on the screen in the
         asteroids game.

  The KSprite class inherits QGraphicsPixmapItem. It uses
  a list of pixmaps to draw and animate the item. The list
  represents the animation. The list can contain a single
  pixmap for items that are not animated. For example, the
  powerups are not animated, but the rocks and the ship are
  animated.

  The state of a KSprite includes the index into the
  list of pixmaps of the current image being displayed.
  It also includes the x and y components of the item's
  velocity on the screen, which is used for computing its
  next position.
  \internal
*/

/*!
  \internal
  The constructor gives the sprite to the scene, sets the
  x and y velocity componets to 0.0, and sets the current
  image to be the first image in the sprite's image list.

  It also marks the new sprite as not dead. If a sprite
  ever gets marked dead, it must destroy itself when its
  advance() function is called during phase one of the
  scene animation. But note that this is not true for the
  ship and shield. These two sprites are created once
  and maintained forever. They are handled differently
  with respect to death.
 */
KSprite::KSprite()
    : QGraphicsPixmapItem(0,scene()),
      dead_(false),
      current_image_(0),
      vx_(0.0),
      vy_(0.0),
      frameshape(0)
{
}

/*!
  \internal
  The destructor is virtual, but it does nothing.
  An instance of KSprite does not own its list
  of images.
 */
KSprite::~KSprite()
{
    // nothing.
}

/*! \fn void KSprite::incrementAge()
  \internal
  This function does nothing in the base class. In the
  \l KAgingSprite subclass, it increments the current age.
 */

/*! \fn void KSprite::setMaximumAge(int max)
  \internal
  This function does nothing in the base class. In the
  \l KAgingSprite subclass, it sets the maximum age for the
  sprite to \a max.
 */

/*! \fn bool KSprite::isExpired() const
  \internal
  This function returns false in the base class. In the
  \l KAgingSprite subclass, it returns true if the sprite's
  current age is greater than its maximum allowed age.
 */

/*! \fn void KSprite::expire()
  \internal
  This function does nothing in the base class. In the
  \l KAgingSprite subclass, it sets the sprite's current
  age to be greater than its maximum allowed age, so that
  the sprite will be destroyed when it is next processed.
 */

/*!
  This function is called by QGraphicsScene::advance().
  It is called twice for each item at each animation step,
  first with \a phase = 0, and then with \a phase = 1.

  In phase 0, each item moves itself. In phase 1, each item
  that is responsible for handling collisions detects its
  collisions and handles them.

  Note: Each subclass of this base class must implement
  advance() and call this function when \a phase == 0.
  \internal
 */
void
KSprite::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	if ((vx_ != 0.0) || (vy_ != 0.0)) {
	    moveBy(vx_,vy_);
	    wrap();
	}
    }
}

/*!
  \internal
  If the sprite is animated, its image list contains more
  than one image. In that case, increment the current image
  counter to the next one in the list, or the first one in
  the list if we have reached the last image, and then get
  that pixmap from the list and set it as the item's pixmap
  for display purposes.
 */
void KSprite::advanceImage()
{
    prepareGeometryChange();
    current_image_ = (current_image_+1) % frameCount();
}

const QList<KSprite::Frame>& KSprite::frames() const
{
    if ( !frameshape )
    {
        if(!spritesLoaded_)
            loadSprites();
        frameshape = &shapemap_[type()];
    }
    return *frameshape;
}

QRectF KSprite::boundingRect() const
{
    return frames().at(current_image_).boundingRect;
}

QPainterPath KSprite::shape() const
{
    return frames().at(current_image_).shape;
}

void KSprite::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->drawPixmap(0, 0, frames().at(current_image_).pixmap);
}


/*! \fn int KSprite::type() const
  This function returns 0 in the base class. In subclasses,
  it returns the integer Id of the specific kind of sprite
  the instnace represents.
  \internal
 */

/*! \fn bool KSprite::isRock() const
  This function returns false in the base class. In rock
  subclasses, it returns true.
  \internal
 */

/*! \fn bool KSprite::isLargeRock() const
  This function returns false in the base class. In the
  large rock subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isMediumRock() const
  This function returns false in the base class. In the
  medium rock subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isSmallRock() const
  This function returns false in the base class. In the
  small rock subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isShip() const
  This function returns false in the base class. In the
  ship subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isShield() const
  This function returns false in the base class. In the
  shield subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isMissile() const
  This function returns false in the base class. In the
  missile subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isFragment() const
  This function returns false in the base class. In the
  fragment subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isExhaust() const
  This function returns false in the base class. In the
  exhaust subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isPowerup() const
  This function returns false in the base class. In the
  powerup subclasses, it returns true.
  \internal
 */

/*! \fn bool KSprite::isEnergyPowerup() const
  This function returns false in the base class. In the
  energy powerup subclass, it returns true.
  \internal
*/

/*! \fn bool KSprite::isTeleportPowerup() const
  This function returns false in the base class. In the
  teleport powerup subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isBrakePowerup() const
  This function returns false in the base class. In the
  brake powerup subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isShieldPowerup() const
  This function returns false in the base class. In the
  shield powerup subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::isShootPowerup() const
  This function returns false in the base class. In the
  shoot powerup subclass, it returns true.
  \internal
 */

/*! \fn bool KSprite::apply()
  \internal
  Returns true if the sprite is a powerup, the powerup
  collided with the ship, and the ship did not have its
  shield up.
 */

/*! \fn void KSprite::markApply()
  \internal
  This function does nothing in the base class. In some
  subclasses, it marks the sprite to be applied, in some
  sense, to another sprite it might eventually collide
  with.

  eg, when a \l {KPowerup} {powerup} collides with the ship,
  the powerup's value is given to the ship.
 */

/*! \fn int KSprite::currentImage() const
  Returns the index in the list of pixmaps of the pixmap
  currently being displayed. For sprites that are not
  animated, the value returned is always 0.
  \internal
 */

/*! \fn int KSprite::frameCount() const
  Returns the number of images in the animation list.
  Always greater than or equal to 1. For sprites that
  are not animated, the value returned is always 1.
  \internal
 */

/*!
  This function sets the animation's current image
  to the one specified by \a index.
  \internal
 */
void KSprite::setImage(int index)
{
    prepareGeometryChange();
    current_image_ = index % frameCount();
}

/*!
  This function sets the animation's current image
  to the first one in its image list.
  \internal
 */
void KSprite::resetImage()
{
    prepareGeometryChange();
    current_image_ = 0;
}

/*! \fn void KSprite::setVelocity(qreal vx, qreal vy)
  Sets the item's x and why component velocities to \a vx
  and \a vy. These are used to compute each successive
  position on the screen for the animated item.
  \internal
 */

/*! \fn qreal KSprite::velocityX() const
  Returns the x component of the item's velocity.
  \internal
 */

/*! \fn qreal KSprite::velocityY() const
  Returns the y component of the item's velocity.
  \internal
 */

/*!
  \internal
  When a symbol goes off the edge of the screen, this function
  puts it back on the screen somewhere else.
 */
void
KSprite::wrap()
{
    int dx = (int)boundingRect().width() / 2;
    int dy = (int)boundingRect().height() / 2;
    int tmp_x = int(x() + dx);
    int tmp_y = int(y() + dy);

    if (tmp_x > scene_->width())
        setPos(0,y());
    else if (tmp_x < 0)
        setPos(scene_->width() - dx,y());

    if (tmp_y > scene_->height())
        setPos(x(),0);
    else if (tmp_y < 0)
        setPos(x(),scene_->height() - dy);
}

/*!
  A static function that resets the sprite management
  data structures.
  \internal
 */
void KSprite::reset()
{
    if(!spritesLoaded_)
        KSprite::loadSprites();
    dying_ = false;
    KRock::reset();
    KMissile::reset();
}

/*!
  A static function that sets the global view pointer to
  \a v, which must not be null. The global view is the
  asteroids game board. The sprite avance() functions
  use it for calling functions to emit signals.
  \internal
 */
void KSprite::setView(KAsteroidsView* v)
{
    view_ = v;
}

/*!
  \internal
  A static function for reading the sprite files containing
  pixmaps for displaying animated elements seen in the game.
 */
void KSprite::loadSprites()
{
    QString sprites_prefix = IMG_BACKGROUND;
    int sep = sprites_prefix.lastIndexOf("/");
    sprites_prefix.truncate(sep);

    int i = 0;
    QString file_name;
    QString base = sprites_prefix + '/';
    while (animations_[i].id_) {
	QList<QPixmap> p;
	if (animations_[i].frames_) {
	    for (int j=0; j<animations_[i].frames_; ++j) {
		QString s(animations_[i].path_);
		file_name = base + s.arg(j,4,10,QLatin1Char('0'));
		QPixmap pixmap(file_name);
		p.insert(j,pixmap);
	    }
	}
	else {
	    file_name = base + QString(animations_[i].path_);
	    QPixmap pixmap(file_name);
	    p.insert(0,pixmap);
	}

        QList<Frame> frameshape;
        for (int f = 0; f < p.size(); ++f) {
            QPixmap pixmap = p.at(f);
            Frame frame;
            frame.pixmap = pixmap;
            QPainterPath path;
            QBitmap m = pixmap.mask();
            if (m.width())
                path.addRegion(QRegion(m));
            else
                path.addRegion(QRect(pixmap.rect()));
            frame.shape = path;
            frame.boundingRect = path.controlPointRect();
            frameshape << frame;
        }
	shapemap_.insert(animations_[i].id_,frameshape);

        i++;
    }
    spritesLoaded_ = true;
}

/*!
  \internal
  Generate a random integer in the range 0..\arange -1
  and return it.
 */
int KSprite::randInt(int range)
{
    return rand() % range;
}

/*!
  \internal
  Generate and return a random double.
 */
double KSprite::randDouble()
{
    int v = rand();
    return (double)v / (double)RAND_MAX;
}

/*!
  \internal
  Returns a pointer to the scene containing all the sprites.
  Creates the scene first, if a scene does not yet exist.
  Ownership of the scene is held by MyGraphicsView.
 */
QGraphicsScene*
KSprite::scene()
{
    if (!scene_) {
	scene_ = new QGraphicsScene();
	scene_->setBackgroundBrush(QPixmap(IMG_BACKGROUND));
	scene_->setItemIndexMethod(QGraphicsScene::NoIndex);
    }
    return scene_;
}

/*! \fn KShip* KSprite::ship()
  \internal
  A static function that returns a pointer to the singleton
  ship sprite. Note that the ship does not exist until the
  \l KSprite::newShip() function has been called.
 */

/*! \fn KShield* KSprite::shield()
  \internal
  A static function that returns a pointer to the singleton
  shield sprite. Note that the shield does not exist until
  the \l KSprite::newShip() function has been called, which
  constructs both the ship and the shield.
 */

/*!
  \internal
  A static function that constructs the singleton ship sprite
  and the singleton shield sprite, if they do not yet exist.
  It returns a pointer to the ship  sprite. Note that both the
  ship and shield are created at the same time. Note also that
  they are both deleted at the same time, ie deleting the ship
  causes the shield to be deleted at the same time.
 */
KShip* KSprite::newShip()
{
    if (!ship_) {
	ship_ = new KShip();
	shield_ = new KShield();
        ship_->resetPosition();
        ship_->show();
        shield_->resetPosition();
        shield_->show();
    }
    return ship_;
}

/*! \fn bool KSprite::dying()
  Returns true if the game is ending and all the sprites
  should die gracefully.
 */

/*! \fn void KSprite::markDying(bool d)
  Set the static, dying flag to \a d. If the dying flag is
  set, when a sprite's advance() function is called in phase
  0, mark the sprite dead. The sprite will then be deleted
  when its advance() function is called in phase 1.

  Call this function with \a d = 0, after the game has ended
  and the advance() function has been called for each sprite,
  ie after all the sprites have been deleted.
 */

/*!
  A static function that returns true the first time it is
  called after a ship has been destroyed. Once it returns
  true, it returns false on all subsequent calls until the
  nex ship is killed.
 */
bool KSprite::shipKilled()
{
    if (shipKilled_) {
	shipKilled_ = false;
	return true;
    }
    return false;
}

/*! \class KAgingSprite
  \brief The KAgingSprite class is the base class for all the
         sprites that age and die.

  The aging sprites include the missile, the powerup, and the
  ship's shield and fragments.
  \internal
 */

/*! \fn void KAgingSprite::incrementAge()
  This virtual function increments the sprite's current age
  by one unit of time or distance, however the sprite's age
  is measured.

  For example, powerups live a certain amount of time,
  while missiles travel a certain distance. It doesn't
  make any difference in the implementation, but there
  it is.
  \internal
 */

/*! \fn void KAgingSprite::setMaximumAge(int max)
  Sets the sprite's maximum allowed age to \a max.
  \internal
 */

/*! \fn bool KAgingSprite::isExpired() const
  This function returns true if the item has past its use-by
  date, ie when its current age is greater than its maximum
  age.
  \internal
 */

/*! \fn void KAgingSprite::expire()
  This function sets the sprite's current age to be greater
  than its maximum allowed age, so that the sprite will be
  removed and deleted when it is next processed.
  \internal
 */

/*! \class KMissile
  \brief Class KMissile contains the missile pixmap.

  It inherits KSprite but it isn't animated, because
  there is nothing to animate. Its pixmap list contains
  only one image, which just looks like a yellow dot on
  the screen. It doesn't look at all like a missile.

  A missile, once fired, travels in a straight line until
  it reaches its maximum range. Then it dies. To implement
  this, the internel state of the class contains a current
  age value and a maximum age value. The current age is
  imcremented each time the animation is advanced.
  \internal
 */

/*!
  The constructor creates a new missile that will live to
  the maximum missile age. It increments the total number
  of shots fired, and it increments the number of current
  shots, ie, missiles on their way.
 */
KMissile::KMissile() : KAgingSprite(MAX_MISSILE_AGE)
{
    ++shotsFired_;
    ++missiles_;
}

/*!
  The virtual destructor decrements the number of current
  missiles, but not below 0.
 */
KMissile::~KMissile()
{
    if (missiles_ >0)
	--missiles_;
}

/*!
  In \a phase 0, increment the missile's age and return if
  the missile has expired. If not, move the missile based
  on its current position and velocity. Then get the its
  list of colliding sprites and traverse the list. When a
  rock is found in the list, mark both the rock and the
  missile dead and return.

  In phase 1, if the missile is marked dead, delete it;
  \internal
 */
void KMissile::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	if (isDead())
	    return;
	KSprite::advance(phase);
        QList<QGraphicsItem*> hits = collidingItems();
        QList<QGraphicsItem*>::iterator i;
        for (i=hits.begin(); i!=hits.end(); ++i) {
	    if ((*i)->type() > ID_Base) {
		KSprite* sprite = (KSprite*)(*i);
		if (sprite->isRock()) {
		    sprite->markDead();
		    markDead();
		    break;
		}
	    }
        }
    }
    else if (isDead() || dying())
	delete this;
}

/*! \class KFragment
  \brief Class KFragment is a piece of the exploded ship.
  \internal
 */

/*!
  Constructor. A ship fragment live for a count of 7.
  \internal
 */
KFragment::KFragment()
    : KAgingSprite(3)
{
    ++count_;
}

/*!
  The destructor decrements the static count of existing
  ship fragments.
  \internal
 */
KFragment::~KFragment()
{
    if (count_ > 0)
	--count_;
}

/*!
  In \a phase 0, increment the fragment's age and return if
  the fragment has expired. If not, move the fragment based
  on its current position and velocity and advance its image.

  In phase 1, if the fragment is marked dead, delete it;
  \internal
 */
void KFragment::advance(int phase)
{
    if (phase == 0) {
	incrementAge();
	if (isDead()) 
	    return;
	advanceImage();
	KSprite::advance(phase);
    }
    else if (isDead())
	delete this;
}

/*!
  A static function that returns true if any ship fragments exist.
  \internal
bool KFragment::exploding()
{
    return (count_ > 0);
}
 */

/*! \class KRock
  \brief Class KRock is the base class for three sizes of rock.

  The rock base class generates and holds some random values
  used when computing the next image to display. This makes
  the rocks look like they are tumbling.

  The static count of rocks created is incremented.
  \internal
 */
KRock::KRock() : KSprite()
{
    skip_ = randInt(2);
    cskip_ = skip_;
    step_ = randInt(2) ? -1 : 1;
    ++rocksCreated_;
    allRocksDestroyed_ = false;
}

/*!
  The destructor increments the static count of rocks destroyed.
  \internal
 */
KRock::~KRock()
{
    ++rocksDestroyed_;
    if (rocksDestroyed_ >=rocksCreated_)
	allRocksDestroyed_ = true;
}

/*!
  \internal
  Reset the rock speed and clear the counts of rocks created
  and destroyed.
 */
void KRock::reset()
{
    rockSpeed_ = 1.0;
    rocksCreated_ = 0;
    rocksDestroyed_ = 0;
    allRocksDestroyed_ = false;
}

/*!
  Sets the current image for this rock to the next image in
  the animation sequence. The way it computes which image is
  next is a bit obscure. It skips over some.
  \internal
 */
void KRock::advanceImage()
{
    if (cskip_-- <= 0) {
	setImage((currentImage()+step_+frameCount()) % frameCount());
	cskip_ = qAbs(skip_);
    }
}

/*!
  \internal
  A static function that adds \a count new rocks to the scene.
  Called when a new game starts and when a new game level starts.
 */
void KRock::createRocks(int count)
{
    for (int i=0; i<count; i++) {
        KRock* rock = new KMediumRock();
        double dx = rockSpeed_;
	dx = qAbs(dx);
        double dy = dx;
        rock->setImage(randInt(ROCK_IMAGE_COUNT));
	int j = randInt(4);
	switch (j) {
	    case 0:
		rock->setPos(5,5);
		break;
	    case 1:
		dy = -dy;
                rock->setPos(5,scene_->height() - 25);
		break;
	    case 2:
		dx = -dx;
                rock->setPos(scene_->width() - 25,5);
		break;
	    case 3:
		dx = -dx;
		dy = -dy;
                rock->setPos(scene_->width() - 25,scene_->height() - 25);
		break;
	    default:
		dy = -dy;
		rock->setPos(5,5);
		break;
        }
        rock->setVelocity(dx,dy);
        rock->show();
    }
    allRocksDestroyed_ = false;
}
/*!
  \internal

  This function is called when a large rock or a medium rock
  is shattered by a missile. It is not called for small rocks.

  Destroy this rock because it was either hit by a missile
  fired by the ship, or it was hit by the ship itself while
  the ship's shield was up.

  If this rock is a large rock, remove it from the board and
  break it into 4 medium rocks. If this rock is a medium
  rock, remove it from the board and break it into four
  small rocks.

  An appropriate rockDestroyed signal is emitted so the game
  score can be updated.

  Additionally, a powerup might be created as a consequence
  of destroying the rock.
 */
void KRock::destroy()
{
    if (isSmallRock())
        return;
    /*
      Break large rocks into medium rocks and medium rocks
      into small rocks.
     */
    if (isLargeRock())
        view_->reportRockDestroyed(1);
    else if (isMediumRock())
        view_->reportRockDestroyed(5);

    static double x_multiplier[4] = { 1.0, 1.0, -1.0, -1.0 };
    static double y_multiplier[4] = { -1.0, 1.0, -1.0, 1.0 };

    double dx = velocityX() + velocityX() * 0.1;
    double dy = velocityY() + velocityY() * 0.1;

    double maxRockSpeed = ROCK_SPEED_MULTIPLIER * rockSpeed_;
    if (dx > maxRockSpeed)
	dx = maxRockSpeed;
    else if (dx < -maxRockSpeed)
	dx = -maxRockSpeed;
    if (dy > maxRockSpeed)
	dy = maxRockSpeed;
    else if (dy < -maxRockSpeed)
	dy = -maxRockSpeed;

    /*
      When the old rock explodes, we create four new, smaller
      rocks in its place. If the old rock is a large one, create
      four medium size rocks. If the old rock is a medium one,
      create four small ones. If the old rock is already small,
      we don't create anything. We don't even get into this loop
      if the old rock is small.
    */
    for (int i = 0; i < 4; i++) {
	double r = (rockSpeed_/2 - (randDouble() * rockSpeed_)) * 3.0;
	KRock* newRock = 0;
	if (isLargeRock())
	    newRock = new KMediumRock();
	else
	    newRock = new KSmallRock();

	/*
	  Each new rock is given an initial position which
	  is offset from the old rock's last position by the
	  width of one quadrant of the old rock's bounding box.
	  Each of the new rocks is positioned in a different
	  quadrant of the old rock's bounding box.
	*/
	qreal quadrant = newRock->boundingRect().width()/4;
	newRock->setPos(x() + (x_multiplier[i] * quadrant),
			y() + (y_multiplier[i] * quadrant));
	newRock->setVelocity(dx + (x_multiplier[i] * rockSpeed_) + r,
			     dy + (y_multiplier[i] * rockSpeed_) + r);
	newRock->setImage(randInt(ROCK_IMAGE_COUNT));
	newRock->show();
    }
    /*
      Note: This rock is actually deleted as the last
      statement of the caller. See KLargeRock::advance()
      and KMediumRock::advance().
    */
}

/*! \class KShip
  \brief The class KShip is the ship in the asteroids game.

  Only one instance of this class is created. Don't create
  more than one, or you will experience strange things.
  \internal
 */

/*!
  The constructor for the ship. No more than one ship exists
  at any time.
  \internal
 */
KShip::KShip()
    : teleport_(false),
      rotateLeft_(false),
      rotateRight_(false),
      engineIsOn_(false),
      isBraking_(false),
      isShooting_(false),
      powerLevel_(MAX_SHIP_POWER_LEVEL),
      teleportCount_(0),
      angleIndex_(0),
      brakeForce_(0),
      firePower_(0),
      rotateSlow_(0),
      rotationRate_(ROTATION_RATE),
      nextShotDelay_(0),
      dx_(0.0),
      dy_(0.0),
      angle_(0.0),
      cosangle_(cos(angle_)),
      sinangle_(sin(angle_))
{
    setVelocity(0.0,0.0);
}

/*!
  The destructor sets the static ship pointer to 0.
 */
KShip::~KShip()
{
    shipKilled_ = true;
    ship_ = 0;
}

/*!
  Increment the number of instant teleports avaialable and
  tell the game to update the count on the screen.
 */
void KShip::incrementTeleportCount()
{
    ++teleportCount_;
    view_->markVitalsChanged();
}

/*!
  Decrement the number of instant teleports avaialable and
  tell the game to update the count on the screen.
 */
void KShip::decrementTeleportCount()
{
    if (teleportCount_) {
	--teleportCount_;
	view_->markVitalsChanged();
    }
}

/*!
  \internal
  Stop the ship's rotation to left or right, and reset the
  rotation rate variables to their initial values.
 */
void KShip::stopRotation()
{
    rotateLeft_ = false;
    rotateRight_ = false;
    rotationRate_ = ROTATION_RATE;
    rotateSlow_ = 0;
}

/*!
  zzz
 */
void KShip::rotateLeft(bool r)
{
    rotateLeft_ = r;
    rotateSlow_ = 5;
}

/*!
 */
void KShip::rotateRight(bool r)
{
    rotateRight_ = r;
    rotateSlow_ = 5;
}

/*! \fn bool KShip::isBraking() const
  Returns true if the ship is stopping.
  I don't think this is needed.
 */

/*!
  \internal
  If the ship has any braking force available, start applying
  the brakes to slow down the ship.
 */
void KShip::startBraking()
{
    if (brakeForce())
        applyBrakes();
}

/*!
  \internal
  If the ship has any braking force available, release the
  brakes, ie stop applying the breaks. Also, if the ship is
  braking, stop the ship movement, and stop its rotation. I
  don't know if this is actually correct, but this is how it
  was programmed when I ported it to Qtopia 4.2.
 */
void KShip::stopBraking()
{
    if (brakeForce()) {
        if (isBraking()) {
            stopEngine();
	    stopRotation();
        }
        releaseBrakes();
    }
}

/*!
  \internal
  Turn on engine. While the engine is on, velocity increases.
 */
void KShip::startEngine()
{
    engineIsOn_ = (powerLevel_ > EMPTY_SHIP_POWER_LEVEL);
}

/*!
  \internal
  Turn off engine. The ship doesn't stop when you turn off
  the engine. It coasts.
 */
void KShip::stopEngine()
{
    engineIsOn_ = false;
}

/*! \fn bool KShip::engineIsOn() const
  Returns true if the engine is thrusting.
  I think this should be removed.
  \internal
 */

/*!
  Reset the ship's position to be the middle of the scene.
  \internal
 */
void KShip::resetPosition()
{
    setPos(scene()->width()/2, scene()->height()/2);
}

/*!
  Increase the ship's power level by \a delta. If the change
  increases the power level above the maximum, reset it to
  the maximum.
  \internal
 */
void KShip::increasePowerLevel(int delta)
{
    powerLevel_ += delta;
    if (powerLevel_ > MAX_SHIP_POWER_LEVEL)
	powerLevel_ = MAX_SHIP_POWER_LEVEL;
    view_->markVitalsChanged();
}

/*!
  Reduce the ship's power level by \a delta, which should be
  a positive number. If the reduction makes the power level
  negative, reset it to 0; If the ship's power level becomes
  0, set the shield strength to 0, but don't drop the shiled
  here if it is up. The shiled is only dropped in phase 1 of
  the scene animation.
  \internal
 */
void KShip::reducePowerLevel(int delta)
{
    powerLevel_ -= delta;
    if (powerLevel_ < 0) {
	powerLevel_ = 0;
        stopEngine();
	shield()->setStrength(0);
    }
    view_->markVitalsChanged();
}

/*!
  \internal
  If the braking force is less than the maximum, increment
  the braking force and tell the game to update the data
  on the screen.
 */
void KShip::incrementBrakeForce()
{
    if (brakeForce() < MAX_BRAKING_FORCE) {
	brakeForce_++;
	view_->markVitalsChanged();
    }
}

/*!
  \internal
  If the ship's fire power is less than the maximum, increment
  the fire power.
 */
void KShip::incrementFirePower()
{
    if (firePower_ < MAX_FIREPOWER) {
	++firePower_;
	view_->markVitalsChanged();
    }
}

/*!
  \include
  If any teleports are available, enable teleporting
  for the next timer interrup when the ship is processed.
 */
void
KShip::teleport()
{
    if (teleportCount_)
	teleport_ = true;
}

/*!
  The advance function does quite a lot for the ship sprite.
  In \a phase 0, if the ship is marked dead, just return. If
  not, move the ship using its current position and velocity.
  Then get the list of all collisions with the ship and run
  through the list.

  If the ship collides with a rock, then if the shield is
  up, destroy the rock. If the shiled is down (normal), mark
  the ship dead and return.

  If the ship collides with a powerup, then if the shield is
  up, mark the powerup destroyed. If the shield is not up,
  apply the powerup to the ship.

  In phase 1, if the ship is marked dead, explode the ship,
  delete it, and return. Otherwise, handle ship rotation,
  breaking, ship velocity, an teleporting. also update the
  image if the ship is rotating, and the exhaust image, if
  the engine is on. If the shiled is up, handle its image
  and age. Finally, in phase one, handle the firing of the
  missiles.
  \internal
 */
void KShip::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	if (isDead() || teleport_)
	    return;
	KSprite::advance(phase);
	QList<QGraphicsItem*> hits = ship_->collidingItems();
	QList<QGraphicsItem*>::Iterator i;
	for (i=hits.begin(); i!=hits.end(); ++i) {
	    if ((*i)->type() <= ID_Base)
		continue;
	    KSprite* sprite = (KSprite*)(*i);
	    if (sprite->isRock()) {
		if (shield_->isUp()) {
		    /*
		      The ship hit a rock with the shield up.
		      The rock is marked for death, which will
		      cause it to break up or just disappear in
		      in phase 1.

		      The shield's strength is reduced by an
		      amount commensurate with the rock size.
		      If the strength goes to 0, the shield
		      will be dropped in phase 1.
		     */
		    sprite->markDead();
		    int s = 1;
		    if (sprite->isLargeRock())
			s = 3;
		    else if (sprite->isMediumRock())
			s = 2;
		    int pl = s * (SHIELD_HIT_COST - (shield_->strength()*2));
		    shield_->reduceStrength(s);
		    reducePowerLevel(pl);
		}
		else {
		    /*
		      The ship hit a rock with the shield down.
		      Mark the ship dead and return. The ship
		      will be exploded in phase 1.
		     */
                    view_->setCanPause(false);
		    markDead();
		    shield_->markDead();
		    return;
		}
	    }
	    else if (sprite->isPowerup()) {
		if (shield_->isUp()) {
		    sprite->markDead();
		}
		else {
		    /*
		      The ship hit a powerup with the shield down.
		      Mark the powerup for apply. It will be applied
		      to the ship in phase 1, if the ship survives.
		      Also mark the powerup dead, ie consumed.
		     */
		    sprite->markApply();
		    sprite->markDead();
		    return;
		}
	    }
            else if (powerLevel() <= EMPTY_SHIP_POWER_LEVEL) {
                ship_->markDead();
                shield_->markDead();
            }
	}
    }
    else { // phase 1
	if (isDead() || dying()) {
	    explode(); // shatters the ship into spinning fragments.
	    delete this;
	    return;
	}
	if (rotateSlow_)
	    rotateSlow_--;

	if (rotateLeft_) {
	    angleIndex_ -= rotateSlow_ ? 1 : rotationRate_;
	    if (angleIndex_ < 0)
		angleIndex_ = SHIP_STEPS-1;
	    angle_ = angleIndex_ * PI_X_2 / SHIP_STEPS;
	    cosangle_ = cos(angle_);
	    sinangle_ = sin(angle_);
	}

	if (rotateRight_) {
	    angleIndex_ += rotateSlow_ ? 1 : rotationRate_;
	    if (angleIndex_ >= SHIP_STEPS)
		angleIndex_ = 0;
	    angle_ = angleIndex_ * PI_X_2 / SHIP_STEPS;
	    cosangle_ = cos(angle_);
	    sinangle_ = sin(angle_);
	}

	if (isBraking()) {
	    stopEngine();
	    stopRotation();
	    if ((fabs(dx_) < 2.5) && (fabs(dy_) < 2.5)) {
		dx_ = 0.0;
		dy_ = 0.0;
		setVelocity(dx_,dy_);
		releaseBrakes();
	    }
	    else {
		double motionAngle = atan2(-dy_,-dx_);
		if (angle_ > M_PI)
		    angle_ -= PI_X_2;
		double angleDiff = angle_ - motionAngle;
		if (angleDiff > M_PI)
		    angleDiff = PI_X_2 - angleDiff;
		else if (angleDiff < -M_PI)
		    angleDiff = PI_X_2 + angleDiff;
		double fdiff = fabs(angleDiff);
		if (fdiff > 0.08) {
		    if (angleDiff > 0)
			rotateLeft_ = true;
		    else if (angleDiff < 0)
			rotateRight_ = true;
		    if (fdiff > 0.6)
			rotationRate_ = brakeForce() + 1;
		    else if (fdiff > 0.4)
			rotationRate_ = 2;
		    else
			rotationRate_ = 1;

		    if (rotationRate_ > 5)
			rotationRate_ = 5;
		}
		else if ((fabs(dx_)>1) || (fabs(dy_)>1)) {
		    startEngine();
		    // we'll make braking a bit faster
		    dx_ += cosangle_/6 * (brakeForce() - 1);
		    dy_ += sinangle_/6 * (brakeForce() - 1);
		    reducePowerLevel(BRAKE_ON_COST);
		    KExhaust::add(ship_->x() + 10 - cosangle_*11,
				  ship_->y() + 10 - sinangle_*11,
				  dx_-cosangle_,
				  dy_-sinangle_,
				  brakeForce()+1);
		}
	    }
	}
	else if (engineIsOn()) {
	    /*
	      The ship has a terminal velocity, but trying
	      to go faster still uses fuel (can go faster
	      diagonally - don't care).
	    */
	    double thrustx = cosangle_/8;
	    double thrusty = sinangle_/8;
	    if (fabs(dx_ + thrustx) < MAX_SHIP_SPEED)
		dx_ += thrustx;
	    if (fabs(dy_ + thrusty) < MAX_SHIP_SPEED)
		dy_ += thrusty;
	    setVelocity(dx_,dy_);
	    reducePowerLevel(5);
	    KExhaust::add(x() + 10 - cosangle_*10,
			  y() + 10 - sinangle_*10,
			  dx_-cosangle_,
			  dy_-sinangle_,
			  3);
	}

	setImage(angleIndex_ >> 1);

	if (teleport_) {
	    int ra = rand() % 10;
	    if(ra == 0)
		ra += rand() % 20;
	    int xra = ra * 60 + ((rand() % 20) * (rand() % 20));
	    int yra = ra * 50 - ((rand() % 20) * (rand() % 20));
	    setPos(xra,yra);
	    teleport_ = false;
	    if (teleportCount_ > 0) {
		--teleportCount_;
		view_->markVitalsChanged();
	    }
	    wrap();
	}

	if (shield_->isUp()) {
	    /*
	      The shield's position always depends on the
	      ship's position.
	     */
	    static int sf = 0;
	    sf++;
	    if (sf % 2)
		shield_->advanceImage();
	    shield_->setPos(x()-5,y()-5);
	    shield_->show();
	}

	if (isShooting()) {
	    int maxMissiles = firePower_ + 2;
	    if (canShoot() && (KMissile::missiles() < maxMissiles)) {
		KMissile* missile = new KMissile();
		missile->setMaximumAge(12);
		missile->setPos(11 + x() + cosangle_ * 11,
				11 + y() + sinangle_ * 11);
		missile->setVelocity(dx_ + cosangle_ * MISSILE_SPEED,
				     dy_ + sinangle_ * MISSILE_SPEED);
		missile->show();
		reducePowerLevel(1);
		view_->reportMissileFired();
		int delay = 5 - firePower_;
		if (delay < 0)
		    delay = 0;
		delayShooting(delay); // delay firing next missile.
	    }
	    decrementNextShotDelay();
	}
    }
}

/*!
  A private function that explodes the ship into a number of
  ship \l {KFragment} {fragments}.
 */
void KShip::explode()
{
    KFragment* f;
    for (int i=0; i<8; i++) {
	f = new KFragment();
	f->setPos((x()+5) - (randDouble()*10), (y()+5) - (randDouble()*10));
	f->setImage(randInt(FRAG_IMAGE_COUNT));
	f->setVelocity(1 - (randDouble()*2), 1 - (randDouble()*2));
	f->setMaximumAge(60 + randInt(60));
	f->show();
    }
}

/*! \class KShield
  \brief The class KShield is the ship's shield in the asteroids game.

  Only one instance of this class is created. Don't create
  more than one, or you will experience strange things.
  \internal
 */

/*!
  The constructor for the ship's shield. No more than one
  shield exists at any time. The singleton shield is created
  when the ship is created.

  The shield is created by \l KSprite::newShip() right after
  it creates the new ship. The are both created in the center
  of the scene, both with velocity 0. Both are made visible.
  \internal
 */
KShield::KShield()
    : KSprite(), isUp_(true), strength_(1)
{
}

/*!
  The only thing the destructor does is set the static shield
  pointer to 0.
 */
KShield::~KShield()
{
    shield_ = 0;
}

/*!
  Reset the shield's position to be the ship's position.
  ie the shield always appears directly over the ship.
  Note also that the shield exists if and only if the
  ship exists.
  \internal
 */
void KShield::resetPosition()
{
    setPos(ship_->pos());
}

/*!
  \internal
  Drop the shield. The shield's hide() function is called,
  and the "shield is up" flag is turned off.
 */
void KShield::drop()
{
    hide();
    isUp_ = false;
}

/*!
  \internal
  If the ship's shield strength is greater than 0, raise the
  shield (ie show it), and reduce the ship's power level by
  an appropriate amount.

  Note that this function does not set any limits on how long
  shield will stay up. That responsibility belongs to the caller.
 */
bool KShield::raise()
{
    isUp_ = (strength_ > 0);
    if (isUp_) {
	--strength_;
	ship()->reducePowerLevel(SHIELD_ON_COST);
	shield_->show();
    }
    return isUp_;
}

/*! \fn bool KShield::isUp() const
  Return true if the ship's shield is currently up.
  \internal
 */

/*!
  \internel
  If the current shield strenght is less than the maximum
  shield strength, increment the current shield strength.
 */
void KShield::incrementStrength()
{
    if (strength_ < MAX_SHIELD_STRENGTH) {
	++strength_;
	view_->markVitalsChanged();
    }
}

/*!
  \internel
  If the current shield strenght is greater than or equal to
  \a delta, reduce the strength by \a delta. Otherwise set
  the strength to 0.

  Not that the shield is not dropped here. This function is
  called during phase 0 of the scene animation. In phase 1,
  if the shield is up but its strength has dropped to 0, it
  will be dropped in phase 1.
 */
void KShield::reduceStrength(int delta)
{
    strength_ -= delta;
    if (strength_ < 0)
	strength_ = 0;
    view_->markVitalsChanged();
}

/*!
  This function is only called by \l QGraphicsScene::advance().

  The shield's position is always computed from the ship's
  position, so the shield is not moved here. Neither is the
  shield destroyed here, because the shield is meant to
  exist for as long as the ship exists. The only thing that
  happens here is that in \a phase 1 the shield is either
  shown or hidden depending on whether it is marked as
  being up or not.

  The shield is created when the ship is created. The shield
  is deleted when the ship is deleted.

  \internal
 */
void KShield::advance(int phase)
{
    if ((phase == 0) && dying())
	markDead();
    else if (phase == 1) {
	if (isDead() || dying()) {
	    delete this;
	    return;
	}
	if (isUp())
	    show();
	else
	    hide();
    }
}

/*! \class KExhaust
  \brief Class KExhaust is the exaust seen when the ship's engine is on.

  Don't blink because it doesn't last long before it dies
  and is removed from the screen.
 */

/*!
  Create an exhaust sprite at position \a x, \a y with
  velocity \a dx, \a dy.
 */
KExhaust::KExhaust(double x, double y, double dx, double dy)
    : KAgingSprite(1)
{
    setPos(x + 2 - randDouble()*4, y + 2 - randDouble()*4);
    setVelocity(dx,dy);
}

/*!
  \internal
  Add \a count exhaust sprites and show them.
 */
void
KExhaust::add(double x, double y, double dx, double dy, int count)
{
    for (int i=0; i<count; i++) {
        KExhaust* e = new KExhaust(x,y,dx,dy);
	e->show();
    }
}

/*!
  In phase 0, age the exhaust and mark it dead if it has
  expired. Also advance the exhaust image and move the
  exhaust with the ship.

  In phase 1, if the exhaust has been marked dead,
  destroy it.
  \internal
 */
void KExhaust::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	if (isDead())
	    return;
	advanceImage();
	KSprite::advance(phase);
    }
    else if (isDead() || dying())
	delete this;
}

/*! \class KPowerup
  \brief Class KPowerup is the base class of all the powerup classes.

  A powerup is a reward that is generated sometimes when a
  small rock is destroyed.  Whenever the player destroys a
  small rock, a random number is generated. If the random
  number is one of the magic, powerup-generating numbers,
  then the corresponding powerup is generated and displayed.
  It travels around the screen for a time, and if it ever
  collides with the ship, the ship picks up whatever value
  the powerup represents. If the powerup never collides
  with the ship, it eventually dies and is removed from the
  screen and deleted.
 */

/*! \fn KPowerup::KPowerup()
  Constructs a powerup.
  \internal
 */

/*!
  Generate a random number in a suitable range. If the
  generated number matches a member of a set of magic
  numbers, create a new powerup of the kind that maps
  to the generated number, and return a pointer to the
  new powerup. Otherwise return 0.
 */
KPowerup* KPowerup::create()
{
    switch (randInt(50)) {
        case 2:
        case 5:
	    return new KEnergyPowerup();
        case 10:
        case 11:
	    return new KTeleportPowerup();
        case 15:
	    return new KBrakePowerup();
        case 20:
        case 21:
	    return new KShieldPowerup();
        case 1:
        case 3:
        case 25:
	    return new KShootPowerup();
    }
    return 0;
}

/*!
  Constructor.
  \internal
 */
KEnergyPowerup::KEnergyPowerup()
    : KPowerup()
{
}

/*!
  In phase 0, age the powerup sprite and move it
  based on its current position and velocity.

  In phase 1, if the powerup has been marked for
  being applied to the ship, increase the ship's
  power level.
  \internal
 */
void KEnergyPowerup::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	KSprite::advance(phase);
    }
    else {
	if (apply() && ship_)
	    ship_->increasePowerLevel(FUEL_POWERUP_BONUS);
	if (isDead() || dying())
	    delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KTeleportPowerup::KTeleportPowerup()
    : KPowerup()
{
}

/*!
  In phase 0, age the powerup sprite and move it
  based on its current position and velocity.

  In phase 1, if the powerup has been marked for
  being applied to the ship, increase the ship's
  teleport count.
  \internal
 */
void KTeleportPowerup::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	KSprite::advance(phase);
    }
    else {
	if (apply() && ship_)
	    ship_->incrementTeleportCount();
	if (isDead() || dying())
	    delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KBrakePowerup::KBrakePowerup()
    : KPowerup()
{
}

/*!
  In phase 0, age the powerup sprite and move it
  based on its current position and velocity.

  In phase 1, if the powerup has been marked for
  being applied to the ship, increase the ship's
  braking capability.
  \internal
 */
void KBrakePowerup::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	KSprite::advance(phase);
    }
    else {
	if (apply() && ship_)
	    ship_->incrementBrakeForce();
	if (isDead() || dying())
	    delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KShieldPowerup::KShieldPowerup()
    : KPowerup()
{
}

/*!
  In phase 0, age the powerup sprite and move it
  based on its current position and velocity.

  In phase 1, if the powerup has been marked for
  being applied to the ship, increase the ship's
  shield strength.
  \internal
 */
void KShieldPowerup::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	KSprite::advance(phase);
    }
    else {
	if (apply() && ship_)
	    shield_->incrementStrength();
	if (isDead() || dying())
	    delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KShootPowerup::KShootPowerup()
    : KPowerup()
{
}

/*!
  In phase 0, age the powerup sprite and move it
  based on its current position and velocity.

  In phase 1, if the powerup has been marked for
  being applied to the ship, increase the ship's
  fire power.
  \internal
 */
void KShootPowerup::advance(int phase)
{
    if (phase == 0) {
	if (dying())
	    markDead();
	else
	    incrementAge();
	KSprite::advance(phase);
    }
    else {
	if (apply() && ship_)
	    ship_->incrementFirePower();
	if (isDead() || dying())
	    delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KLargeRock::KLargeRock()
    : KRock()
{
}

/*!
  In phase 0, advance the rock's image and move the rock
  according to its current poisition and velocity.

  In phase 1, if the rock has been marked dead, then it
  has collided with the ship. Break it into smaller rocks.
  Then destroy the original rock.
  \internal
 */
void KLargeRock::advance(int phase)
{
    if (!isDead()) {
	if (!phase) {
	    KRock::advanceImage();
	    KSprite::advance(phase);
	}
    }
    if (phase && (isDead() || dying())) {
	if (!dying())
	    destroy();
	delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KMediumRock::KMediumRock()
    : KRock()
{
}

/*!
  In phase 0, advance the rock's image and move the rock
  according to its current poisition and velocity.

  In phase 1, if the rock has been marked dead, then it
  has collided with the ship. Break it into smaller rocks.
  Then destroy the original rock.
  \internal
 */
void KMediumRock::advance(int phase)
{
    if (!isDead()) {
	if (!phase) {
	    KRock::advanceImage();
	    KSprite::advance(phase);
	}
    }
    if (phase && (isDead() || dying())) {
	if (!dying())
	    destroy();
	delete this;
    }
}

/*!
  Constructor.
  \internal
 */
KSmallRock::KSmallRock()
    : KRock()
{
}

/*!
  In phase 0, advance the rock's image and move the rock
  according to its current poisition and velocity.

  In phase 1, if the rock has been marked dead, then it
  has collided with the ship. Destroy the rock. Sometimes
  create a powerup in its place.
  \internal
 */
void KSmallRock::advance(int phase)
{
    if (!isDead()) {
	if (!phase) {
	    KRock::advanceImage();
	    KSprite::advance(phase);
	}
    }
    if (phase && (isDead() || dying())) {
        view_->reportRockDestroyed(10);
	if (!dying()) {
	    KPowerup* new_pup = KPowerup::create();
	    if (new_pup) {
		double r = (0.5 - randDouble()) * 4.0;
		new_pup->setPos(x(),y());
		new_pup->setVelocity(velocityX()+r,velocityY()+r);
		new_pup->show();
	    }
	}
	delete this; // Don't do anything after deleting me.
    }
}
