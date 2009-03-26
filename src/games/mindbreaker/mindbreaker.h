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

#ifndef MINDBREAKER_H
#define MINDBREAKER_H

#include <QWidget>
#include <QMainWindow>
#include <QImage>
#include <QVector>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>

static const int panel_height = 26;
static const int panel_width = 180;

static const int title_height = 25;
static const int title_width = 180;

static const int bin_margin = 10;
static const int peg_size = 20;
static const int answerpeg_size = 13;

static const int first_peg_x_diff = 21;
static const int first_peg_y_diff = ((panel_height - peg_size) >> 1);
static const int peg_spacing = 30;

static const int answerpegx = 152;
static const int answerpegy = 2;
static const int answerpeg_diff = 9;

static const int board_height = (title_height + (panel_height * 9));
static const int board_width = (panel_width + (bin_margin * 2) + peg_size);

class Peg;
class QTimer;

class MindBreakerBoard : public QGraphicsView // QWidget
{
    Q_OBJECT
public:
    MindBreakerBoard( QGraphicsScene * scene, QWidget * parent = 0 );
    ~MindBreakerBoard();

    void getScore(int *, int *);

    void fixSize();

signals:
    void scoreChanged(int, int);

public slots:
    void clear();
    void resetScore();

private slots:
    void doFixSize();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent*);

private:
    void readConfig();
    void writeConfig();

    void paintBackground();
    void checkGuess();
    void checkScores();
    void placeGuessPeg(int pos, int pegId);

    QImage panelImage;
    QImage titleImage;

    Peg *moving;
    Peg *current_highlight;
    QPointF moving_pos;

    // the game stuff
    int answer[4];
    int current_guess[4];
    int past_guesses[4*9];
    int current_go;

    int null_press;
    QPointF null_point;
    bool copy_press;
    Peg *copy_peg;
    bool game_over;

    int total_turns;
    int total_games;

    QTimer *widthTimer;
};

class MindBreaker : public QMainWindow // QWidget
{
    Q_OBJECT
public:
    MindBreaker( QWidget * parent = 0, Qt::WindowFlags flags = 0 );

public slots:
    void setScore(int, int);

protected:
    void resizeEvent( QResizeEvent * );

private:
    MindBreakerBoard *board;
    QLabel *score;

};


#endif
