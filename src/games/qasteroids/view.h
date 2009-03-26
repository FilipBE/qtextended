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

#ifndef VIEW_H
#define VIEW_H

#include <qwidget.h>
#include <qlist.h>
#include <QHash>
#include <qtimer.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "sprites.h"

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
 public:
    explicit MyGraphicsView(QGraphicsScene* scene, QWidget* parent = 0)
        : QGraphicsView(scene,parent) { }

 protected:
    void resizeEvent(QResizeEvent* event);
};

class KAsteroidsView : public QWidget
{
    Q_OBJECT

 public:
    KAsteroidsView(QWidget* parent = 0);
    virtual ~KAsteroidsView();

    void newGame();
    void newShip();
    void raiseShield();
    void teleport();
    void pause();
    void reportMissileFired();
    void reportRockDestroyed(int rock_size);

    void showText(const QString &text);
    void hideText();

    void constructMessages(const QString& t);
    void reportStartGame();
    void reportShipKilled();
    void reportGameOver();
    void markVitalsChanged() { vitalsChanged_ = true; }

    bool isPaused() { return game_paused_; }

    bool canPause() { return canPause_; }
    void setCanPause(bool truth) { canPause_ = truth; }

public slots:
    void setPaused(bool paused = true);

 signals:
    void missileFired();
    void shipKilled();
    void updateScore(int key);
    void allRocksDestroyed();
    void updateVitals();

 private slots:
    void dropShield();
    void mainTimerEvent();

 private:
    QGraphicsScene* 	scene_;
    MyGraphicsView* 	view_;
    QGraphicsSimpleTextItem* 	textSprite_;

    bool		instruct_user_;
    bool 		game_paused_;
    bool                canPause_;
    bool 		vitalsChanged_;
    bool		textPending_;

    int 		textDy_;
    int 		timerEventCount_;

    QTimer*		shieldTimer_;
    QTimer*             mainTimer_;

    QString		nextGameMessage_;
    QString		firstGameMessage_;
    QString		shipKilledMessage_;
    QString		gameOverMessage_;
};

#endif
