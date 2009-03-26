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

#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <QGraphicsView>

class Snake;
class QLabel;
class QGraphicsSimpleTextItem;
class QGraphicsRectItem;

class SnakeGame : public QGraphicsView
{
    Q_OBJECT

public:
    SnakeGame(QWidget* parent=0, Qt::WFlags f=0);
    ~SnakeGame();

protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void resizeEvent(QResizeEvent*);
    virtual void focusOutEvent(QFocusEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseDoubleClickEvent(QMouseEvent*);

private slots:
    void newGame(bool start=true);
    void gameOver();
    void endWait();
    void levelUp();

private:
    void clear();
    void createWalls();
    void createMice();
    void showMessage(const QString &);
    void clearMessage();
    void pause();
    void resume();

    QGraphicsScene *scene;
    Snake* snake;
    int screenwidth;
    int screenheight;
    int level;
    int stage;
    int mice;
    bool waitover;
    bool gamestopped;
    bool paused;

    QGraphicsSimpleTextItem *gamemessage;
    QGraphicsRectItem *gamemessagebkg;
};

#endif
