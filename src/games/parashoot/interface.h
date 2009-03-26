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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QSound>
class QLabel;
class QTimer;
class QGraphicsView;
class QGraphicsScene;
class Helicopter;
class Cannon;
class Base;
class QGraphicsSimpleTextItem;
class QGraphicsRectItem;

class ParaShoot : public QMainWindow
{
    Q_OBJECT

public:
    ParaShoot(QWidget* parent=0, Qt::WFlags f=0);
    ~ParaShoot();

    void clear();
    void gameOver();
    int mancount;
    void levelUp();
    void moveFaster();
    static ParaShoot *instance();

    QSound kaboom;
    QSound ohdear;
    QSound bang;
    QSound fanfare;
    QSound splat1;
    QSound splat2;

protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    virtual void resizeEvent(QResizeEvent *e);
    virtual bool event( QEvent * e );

private slots:
    void increaseScore(int);
    void newGame();
    void play();
    void wait();

private:
    void showScore( int score, int level );
    void showMessage(const QString &);
    void clearMessage();

    QGraphicsView *view;
    QGraphicsScene *scene;
    Cannon* cannon;
    Base* base;
    QLabel* levelscore;
    int nomen;
    int level;
    int oldscore;
    int updatespeed;
    QTimer *autoDropTimer;
    QTimer *pauseTimer;
    QTimer *advanceTimer;
    bool gamestopped;
    bool finished, waitover;
    int score;
    int lastcannonkey;

    QGraphicsSimpleTextItem *gamemessage;
    QGraphicsRectItem *gamemessagebkg;
};

#endif
