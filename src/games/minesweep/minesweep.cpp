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

#include "minesweep.h"
#include "minefield.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <QSettings>
#include <QMenu>
#include <QScrollArea>
#include <QAction>
#include <QPalette>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

#include <time.h>


class ResultIndicator : public QLabel
{
    Q_OBJECT
public:
    ResultIndicator( QWidget *parent );
    void showResult( bool won );

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void center();
    void shrink();
};

ResultIndicator::ResultIndicator( QWidget *parent)
    : QLabel(parent, Qt::Tool | Qt::FramelessWindowHint)
{
    setAlignment( Qt::AlignCenter );
    setAttribute( Qt::WA_NoSystemBackground, true );
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setBackgroundRole( QPalette::Window );
    setFocusPolicy(Qt::NoFocus);
}

void ResultIndicator::showResult( bool won )
{
    if ( won) {
        QPalette palette = this->palette();
        palette.setBrush(backgroundRole(), QBrush(Qt::black));
        setPalette(palette);

        setText( QObject::tr("You won!") );
        center();
        show();
        QTimer::singleShot(1500, this, SLOT(hide()));
    } else {
        QPalette palette = this->palette();
        palette.setBrush(backgroundRole(), QBrush(Qt::red));
        setPalette(palette);

        setText( QObject::tr("You exploded!") );
        QRect rect = geometry();
        rect.setSize( parentWidget()->size() );
        rect.moveTopLeft( parentWidget()->mapToGlobal(QPoint(0,0)) );
        setGeometry(rect);
        show();
        QTimer::singleShot(200, this, SLOT(shrink()));
    }
}

void ResultIndicator::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.fillRect(rect(), palette().brush(backgroundRole()));
    p.end();
    QLabel::paintEvent(e);
}

void ResultIndicator::center()
{
    QSize s = minimumSizeHint() * 3;
    QRect r = geometry();
    r.setSize(s);
    r.moveCenter( parentWidget()->geometry().center() );
    setGeometry(r);
}

void ResultIndicator::shrink()
{
    center();
    QTimer::singleShot(1000, this, SLOT(hide()));
}



MineSweep::MineSweep( QWidget* parent, Qt::WFlags f )
: QMainWindow( parent, f )
{
    srand(::time(0));   // for placing mines

    scroll = new QScrollArea(this);
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setFrameStyle(QFrame::NoFrame);
    scroll->setBackgroundRole(QPalette::Dark);
    scroll->setWidgetResizable(true);
    scroll->setAlignment(Qt::AlignCenter);
    setCentralWidget(scroll);

    field = new MineField;
    QSoftMenuBar::setLabel(field, Qt::Key_Select, QSoftMenuBar::Select);
    scroll->setWidget(field);
    setFocusProxy(field);

    connect( field, SIGNAL(newGameSelected()), this, SLOT(newGame()) );
    connect( field, SIGNAL(gameOver(bool)), this, SLOT(gameOver(bool)) );
    connect( field, SIGNAL(currentPointChanged(int,int)),
             this, SLOT(showPoint(int,int)));

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction( QIcon(":image/dead"), tr( "Expert" ),
                            this, SLOT(expert()) );
    contextMenu->addAction( QIcon(":image/worried"), tr( "Advanced" ),
                            this, SLOT(advanced()) );
    contextMenu->addAction( QIcon(":image/happy") , tr( "Beginner" ),
                            this, SLOT(beginner()) );

    resultIndicator = new ResultIndicator(this);
    resultIndicator->installEventFilter( this );

    QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);

    setWindowTitle( tr("Mine Hunt") );
    setWindowIcon( QPixmap( ":image/MineHunt" ) );

    readConfig();
}

MineSweep::~MineSweep()
{
}

void MineSweep::closeEvent(QCloseEvent *e) {
    writeConfig();
    QMainWindow::closeEvent(e);
}

void MineSweep::gameOver( bool won )
{
    resultIndicator->showResult(won);
    field->showMines();
}

void MineSweep::newGame()
{
    newGame(field->level());
}

void MineSweep::newGame(int level)
{
    field->setup( level );
}

void MineSweep::beginner()
{
    newGame(1);
}

void MineSweep::advanced()
{
    newGame(2);
}

void MineSweep::expert()
{
    newGame(3);
}

void MineSweep::writeConfig() const
{
    QSettings cfg("Trolltech","MineSweep");
    cfg.setValue("Playing", (field->state() == MineField::Playing));
    cfg.beginGroup("Panel");
    field->writeConfig(cfg);
    cfg.endGroup();
}

void MineSweep::readConfig()
{
    QSettings cfg("Trolltech","MineSweep");
    cfg.beginGroup("Panel");
    field->readConfig(cfg);
    cfg.endGroup();
    bool b = cfg.value("Playing", false).toBool();
    if ( !b ) {
        newGame();
    }
}

bool MineSweep::eventFilter(QObject *obj, QEvent *event)
{
    // don't allow key/mouse clicks on the minefield while result is
    // displayed, because player might accidentally clear the screen
    if (event->type() == QEvent::Show) {
        field->setEnabled(false);
    } else if (event->type() == QEvent::Hide) {
        field->setEnabled(true);
        field->setFocus();
    }

    return QObject::eventFilter(obj, event);
}

void MineSweep::showPoint(int x, int y){
    scroll->ensureVisible(x, y);
}

void MineSweep::resizeEvent( QResizeEvent *e ) {
    QMainWindow::resizeEvent(e);
    field->setAvailableRect( scroll->geometry() );
}

#include "minesweep.moc"
