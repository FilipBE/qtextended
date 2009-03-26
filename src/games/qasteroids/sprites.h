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

#ifndef SPRITES_H
#define SPRITES_H

#include <QGraphicsPixmapItem>
#include <QMap>

#define IMG_BACKGROUND ":image/qasteroids/bg"

#define ID_Base             QGraphicsItem::UserType
#define ID_ROCK_LARGE       ID_Base + 1024
#define ID_ROCK_MEDIUM      ID_Base + 1025
#define ID_ROCK_SMALL       ID_Base + 1026
#define ID_MISSILE          ID_Base + 1030
#define ID_FRAGMENT         ID_Base + 1040
#define ID_EXHAUST          ID_Base + 1041
#define ID_ENERGY_POWERUP   ID_Base + 1310
#define ID_TELEPORT_POWERUP ID_Base + 1311
#define ID_BRAKE_POWERUP    ID_Base + 1312
#define ID_SHIELD_POWERUP   ID_Base + 1313
#define ID_SHOOT_POWERUP    ID_Base + 1314
#define ID_SHIP             ID_Base + 1350
#define ID_SHIELD           ID_Base + 1351

#define MAX_SHIELD_AGE          350
#define MAX_POWERUP_AGE         500
#define EXPIRED_POWERUP         MAX_POWERUP_AGE+1
#define MAX_MISSILE_AGE         25
#define MAX_SHIP_POWER_LEVEL    1000
#define EMPTY_SHIP_POWER_LEVEL  0
#define FUEL_POWERUP_BONUS      250

class KShip;
class KShield;
class KMissile;
class KFragment;
class KExhaust;
class KAsteroidsView;

class KSprite : public QGraphicsPixmapItem
{
 public:
    KSprite();
    virtual ~KSprite();

    virtual void incrementAge() { }
    virtual void setMaximumAge(int ) { }
    virtual bool isExpired() const { return false; }
    virtual void expire() { }
    virtual void advance(int phase);
    virtual void advanceImage();
    virtual int type() const=0;
    virtual bool isRock() const { return false; }
    virtual bool isLargeRock() const { return false; }
    virtual bool isMediumRock() const { return false; }
    virtual bool isSmallRock() const { return false; }
    virtual bool isShip() const { return false; }
    virtual bool isShield() const { return false; }
    virtual bool isMissile() const { return false; }
    virtual bool isFragment() const { return false; }
    virtual bool isExhaust() const { return false; }
    virtual bool isPowerup() const { return false; }
    virtual bool isEnergyPowerup() const { return false; }
    virtual bool isTeleportPowerup() const { return false; }
    virtual bool isBrakePowerup() const { return false; }
    virtual bool isShieldPowerup() const { return false; }
    virtual bool isShootPowerup() const { return false; }
    virtual bool apply() const { return false; }
    virtual void markApply() { }

    int currentImage() const { return current_image_; }
    int frameCount() const { return frames().size(); }
    void setImage(int index);
    void resetImage();
    void markDead() { dead_ = true; }
    void markAlive() { dead_ = false; }
    bool isDead() const { return dead_; } 

    void setVelocity(qreal vx, qreal vy) { vx_ = vx; vy_ = vy; }
    qreal velocityX() const { return vx_; }
    qreal velocityY() const { return vy_; }
    void wrap();

    void ensureCache() const;
    // override
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

 public:
    static void reset();
    static void setView(KAsteroidsView* v);
    static void loadSprites();
    static int randInt(int range);
    static double randDouble();
    static QGraphicsScene* scene();
    static KShip* ship() { return ship_; }
    static KShield* shield() { return shield_; }
    static KShip* newShip();
    static bool dying() { return dying_; }
    static void markDying(bool d) { dying_ = d; }
    static bool shipKilled();

 protected:
    struct Frame {
        QPixmap pixmap;
        QPainterPath shape;
        QRectF boundingRect;
    };

 protected:
    static bool				dying_;
    static bool				shipKilled_;
    static bool                         spritesLoaded_;
    static KShip*			ship_;
    static KShield*			shield_;
    static QGraphicsScene*		scene_;
    static KAsteroidsView*		view_;
    static QMap<int,QList<Frame> >	shapemap_;

 protected:
    bool	dead_;
    int	current_image_;
    qreal	vx_;
    qreal	vy_;

    mutable const QList<Frame>* frameshape;
    const QList<Frame>& frames() const;
};

class KAgingSprite : public KSprite
{
 public:
    KAgingSprite(int max)
	: KSprite(), currentAge_(0), maximumAge_(max) { }
    virtual ~KAgingSprite() { }

    virtual void incrementAge() { currentAge_++; if (isExpired()) markDead(); }
    virtual void setMaximumAge(int max) { maximumAge_ = max; }
    virtual bool isExpired() const { return currentAge_ > maximumAge_; }
    virtual void expire() { currentAge_ = maximumAge_ + 1; markDead(); }

 protected:
    int currentAge_;
    int maximumAge_;
};

class KMissile : public KAgingSprite
{
 public:
    KMissile();
    virtual ~KMissile();

    virtual int type() const { return ID_MISSILE; }
    virtual bool isMissile() const { return true; }
    virtual void advance(int phase);

    static int shotsFired() { return shotsFired_; }
    static void reset() { shotsFired_ = 0; missiles_ = 0; }
    static int missiles() { return missiles_; }

 protected:
    static int			shotsFired_;
    static int			missiles_;
};

class KFragment : public KAgingSprite
{
 public:
    KFragment();
    virtual ~KFragment();

    virtual int type() const {  return ID_FRAGMENT; }
    virtual bool isFragment() const { return true; }
    virtual void advance(int phase);

    static bool exploding() { return (count_ > 0); }

 protected:
    static int			count_;
};

class KExhaust : public KAgingSprite
{
 public:
    KExhaust(double x, double y, double dx, double dy);
    virtual ~KExhaust() { }

    virtual int type() const {  return ID_EXHAUST; }
    virtual bool isExhaust() const { return true; }
    virtual void advance(int phase);

    static void add(double x, double y, double dx, double dy, int count);
};

class KPowerup : public KAgingSprite
{
 public:
    KPowerup() : KAgingSprite(MAX_POWERUP_AGE), apply_(false)  { }
    virtual ~KPowerup() { }

    virtual bool isPowerup() const { return true; }
    virtual bool apply() const { return apply_; }
    virtual void markApply() { apply_ = true; }

 public:
    static KPowerup* create();

 protected:
    bool apply_;
};

class KEnergyPowerup : public KPowerup
{
 public:
    KEnergyPowerup();
    virtual ~KEnergyPowerup() { }
    virtual int type() const { return ID_ENERGY_POWERUP; }
    virtual bool isEnergyPowerup() const { return true; }
    virtual void advance(int phase);
};

class KTeleportPowerup : public KPowerup
{
 public:
    KTeleportPowerup();
    virtual ~KTeleportPowerup() { }
    virtual int type() const { return ID_TELEPORT_POWERUP; }
    virtual bool isTeleportPowerup() const { return true; }
    virtual void advance(int phase);
};

class KBrakePowerup : public KPowerup
{
 public:
    KBrakePowerup();
    virtual ~KBrakePowerup() { }
    virtual int type() const { return ID_BRAKE_POWERUP; }
    virtual bool isBrakePowerup() const { return true; }
    virtual void advance(int phase);
};

class KShieldPowerup : public KPowerup
{
 public:
    KShieldPowerup();
    virtual ~KShieldPowerup() { }
    virtual int type() const { return ID_SHIELD_POWERUP; }
    virtual bool isShieldPowerup() const { return true; }
    virtual void advance(int phase);
};

class KShootPowerup : public KPowerup
{
 public:
    KShootPowerup();
    virtual ~KShootPowerup() { }
    virtual int type() const { return ID_SHOOT_POWERUP; }
    virtual bool isShootPowerup() const { return true; }
    virtual void advance(int phase);
};

class KRock : public KSprite
{
 public:
    KRock();
    virtual ~KRock();
    virtual void advanceImage();
    virtual bool isRock() const { return true; }

    void destroy();

 public:
    static bool allDestroyed() { return allRocksDestroyed_; }
    static void reset();
    static void setRockSpeed(double rs) { rockSpeed_ = rs; }
    static void createRocks(int count);
    static int rocksDestroyed() { return rocksDestroyed_; }

 protected:
    static int		rocksCreated_;
    static int		rocksDestroyed_;
    static bool		allRocksDestroyed_;
    static double	rockSpeed_;

 protected:
    int		skip_;
    int		cskip_;
    int		step_;
};

class KLargeRock : public KRock
{
 public:
    KLargeRock();
    virtual ~KLargeRock() { }

    virtual int type() const { return ID_ROCK_LARGE; }
    virtual bool isLargeRock() const { return true; }
    virtual void advance(int phase);
};

class KMediumRock : public KRock
{
 public:
    KMediumRock();
    virtual ~KMediumRock() { }

    virtual int type() const { return ID_ROCK_MEDIUM; }
    virtual bool isMediumRock() const { return true; }
    virtual void advance(int phase);
};

class KSmallRock : public KRock
{
 public:
    KSmallRock();
    virtual ~KSmallRock() { }

    virtual int type() const { return ID_ROCK_SMALL; }
    virtual bool isSmallRock() const { return true; }
    virtual void advance(int phase);
};

class KShield : public KSprite
{
 public:
    KShield();
    virtual ~KShield();

    virtual int type() const { return ID_SHIELD; }
    virtual bool isShield() const { return true; }
    virtual void advance(int phase);

    void resetPosition();

    int  strength() const { return strength_; }
    void setStrength(int t) { strength_ = t; }
    void incrementStrength();
    void reduceStrength(int delta);

    void drop();
    bool raise();
    bool isUp() const { return isUp_; }

 protected:
    bool	isUp_;
    int		strength_;
};

class KShip : public KSprite
{
 public:
    KShip();
    virtual ~KShip();

    virtual int type() const { return ID_SHIP; }
    virtual bool isShip() const { return true; }
    virtual void advance(int phase);

    void resetPosition();

    int powerLevel() const { return powerLevel_; }
    void increasePowerLevel(int delta);
    void reducePowerLevel(int delta);

    int teleportCount() const { return teleportCount_; }
    void teleport();
    void incrementTeleportCount();
    void decrementTeleportCount();
    void stopRotation();
    void rotateLeft(bool r);
    void rotateRight(bool r);

    void startEngine();
    void stopEngine();
    bool engineIsOn() const { return engineIsOn_; }

    bool isBraking() const { return isBraking_; }
    void startBraking();
    void stopBraking();
    int brakeForce() const { return brakeForce_; }
    void incrementBrakeForce();

    int firePower() const { return firePower_; }
    bool canShoot() const { return !nextShotDelay_; }
    bool isShooting() const { return isShooting_; }
    void startShooting() { isShooting_ = true; enableShooting(); }
    void stopShooting() { isShooting_ = false; enableShooting(); }
    void decrementNextShotDelay() { if (nextShotDelay_) --nextShotDelay_; }
    void delayShooting(int count) { nextShotDelay_ = count; }
    void enableShooting() { nextShotDelay_ = 0; }
    void incrementFirePower();

    static void reset();

 private:
    void applyBrakes() { isBraking_ = true; }
    void releaseBrakes() { isBraking_ = false; }
    void explode();

 protected:
    bool	teleport_;
    bool	rotateLeft_;
    bool	rotateRight_;
    bool	engineIsOn_;
    bool	isBraking_;
    bool	isShooting_;
    int	powerLevel_;
    int	teleportCount_;
    int	angleIndex_;
    int	brakeForce_;
    int	firePower_;
    int	rotateSlow_;
    int	rotationRate_;
    int	nextShotDelay_;

    double	dx_;
    double	dy_;
    double	angle_;
    double	cosangle_;
    double	sinangle_;
};

#endif
