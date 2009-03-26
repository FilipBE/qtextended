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
#ifndef MINEFIELD_H
#define MINEFIELD_H

#include <QFrame>
#include <QHash>

class Mine;
class QSettings;

class MineField : public QFrame
{
    Q_OBJECT
public:
    MineField( QWidget* parent = 0 );
    ~MineField();

    enum State { Waiting, Playing, GameOver };

    State state() const { return stat; }

    void readConfig(QSettings&);
    void writeConfig(QSettings&) const;

    int level() const { return lev; }

    void setAvailableRect( const QRect & );

    QSize sizeHint() const;
    QSize minimumSize() const;
    int preferredGrid() const { return _preferredGrid;};

public slots:
    void setup( int level );
    void showMines();

signals:
    void gameOver( bool won );
    void mineCount( int );
    void currentPointChanged(int x, int y);
    void newGameSelected();

protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void keyReleaseEvent( QKeyEvent* );
    void paintEvent( QPaintEvent* );

    int getHint( int row, int col );
    void setHint( int r, int c );
    void updateMine( int row, int col );
    void paletteChange( const QPalette & );
    void updateCell( int r, int c );
    bool onBoard( int r, int c ) const { return r >= 0 && r < numRows && c >= 0 && c < numCols; }
    Mine *mine( int row, int col ) { return onBoard(row, col ) ? mines[row+numCols*col] : 0; }
    const Mine *mine( int row, int col ) const { return onBoard(row, col ) ? mines[row+numCols*col] : 0; }
    int _minGrid;
    int _preferredGrid;

protected slots:
    void cellClicked( int row, int col );
    void currentPointChanged();

private:
    int findCellSize();
    void setCellSize( int );

    State stat;
    void setState( State st );
    void placeMines();
    enum FlagAction { NoAction, FlagOn, FlagNext };
    FlagAction flagAction;
    int currRow;
    int currCol;
    int numRows, numCols;

    int minecount;
    int mineguess;
    int nonminecount;
    int lev;
    QRect availableRect;
    int cellSize;
    Mine **mines;

    int topMargin;
    int leftMargin;

    int sizeHintX;
    int sizeHintY;
};

#endif
