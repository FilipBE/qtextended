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

#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <qsound.h>

#include <qmainwindow.h>
#include <qmap.h>

#include "view.h"

class KALedMeter;
class QLCDNumber;
class QHBoxLayout;
class QMenu;

class KAstTopLevel : public QMainWindow
{
    Q_OBJECT
 public:
    KAstTopLevel(QWidget* parent=0, Qt::WFlags fl=0);
    virtual ~KAstTopLevel();

 private:
    void startNewGame();
    bool gameEnded() const;
    void endGame();
    void playSound(const char* snd);
    void readSoundMapping();
    void reportStatistics();

    QWidget* buildTopRow(QWidget* parent);
    QWidget* buildBottomRow(QWidget* parent);
    KAsteroidsView* buildAsteroidsView(QWidget* parent);
    QPalette buildPalette();
    bool eventConsumed(QKeyEvent* e) const;
    void populateRocks();
    void populatePowerups();

 protected:
    virtual void showEvent(QShowEvent* );
    virtual void hideEvent(QHideEvent* );
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual bool event( QEvent * e );

 private slots:
    void slotNewGameLevel();
    void slotMissileFired();
    void slotShipKilled();
    void slotUpdateScore(int key);
    void slotUpdateVitals();

 private:
    KAsteroidsView* view_;
    QLCDNumber* scoreLCD_;
    QLCDNumber* levelLCD_;
    QLCDNumber* shipsLCD_;

    QLCDNumber* teleportsLCD_;
    QLCDNumber* brakesLCD_;
    QLCDNumber* shieldLCD_;
    QLCDNumber* shootLCD_;
    KALedMeter* powerMeter_;

    QSound shipDestroyed;
    QSound rockDestroyed;
    QSound missileFired;

    bool	gameEnded_;
    int	shipCount_;
    int	score_;
    int	currentLevel_;

    enum Action {
	Launch,
	Thrust,
	RotateLeft,
	RotateRight,
	Shoot,
	Teleport,
	Brake,
	Shield,
	Pause,
	NewGame,
	Populate_Powerups,
	Populate_Rocks
    };

    QMap<int,Action>    actions_;

    QMenu*              contextMenu_;
};

#endif

