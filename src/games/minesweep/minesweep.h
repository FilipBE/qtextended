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
#ifndef MINESWEEP_H
#define MINESWEEP_H

#include <QMainWindow>

class MineField;
class QScrollArea;
class QHBoxLayout;
class QEvent;
class ResultIndicator;

class MineSweep : public QMainWindow
{
    Q_OBJECT
public:
    MineSweep( QWidget* parent = 0, Qt::WFlags f = 0 );
    ~MineSweep();

public slots:
    void gameOver( bool won );
    void newGame();
    void showPoint(int x, int y);

protected slots:
    void beginner();
    void advanced();
    void expert();
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent*);
    bool eventFilter(QObject *obj, QEvent *event);

protected:
    QScrollArea *scroll;
    MineField* field;
    ResultIndicator *resultIndicator;

private:
    void readConfig();
    void writeConfig() const;
    void newGame(int);
};

#endif
