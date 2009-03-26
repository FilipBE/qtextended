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

#include <math.h>

#include "calibrate.h"
#include "uifactory.h"

#include <QScreen>
#include <QPainter>
#include <QDesktopWidget>
#include <QApplication>
#include <QMenu>
#include <qsoftmenubar.h>
#include <qtopialog.h>
#include <QFile>
#include <QTimer>

#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

Calibrate::Calibrate(QWidget* parent, Qt::WFlags f)
    : QDialog( parent, f )
{
    setObjectName("calibrate");
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowState(Qt::WindowFullScreen);
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    connect (contextMenu,SIGNAL(triggered(QAction*)),
             this,SLOT(menuTriggered(QAction*)));
}

Calibrate::~Calibrate()
{
    store();
}

void Calibrate::showEvent(QShowEvent *e )
{
    showCross = true;
    pressed = false;
    anygood = false;

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desk = desktop->screenGeometry(desktop->primaryScreen());
    const int offset = desk.width() / 10;
    setGeometry(desk);

    int size = (int)(qMin(desk.width(),  desk.height() ) / 1.875);
    logo = QPixmap(":image/Qtopia4_Logo-shaded-128px").scaled(size ,size);

    cd.screenPoints[QWSPointerCalibrationData::TopLeft] = QPoint( offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomLeft] = QPoint( offset, qt_screen->deviceHeight() - offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomRight] = QPoint( qt_screen->deviceWidth() - offset, qt_screen->deviceHeight() - offset );
    cd.screenPoints[QWSPointerCalibrationData::TopRight] = QPoint( qt_screen->deviceWidth() - offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::Center] = QPoint( qt_screen->deviceWidth()/2, qt_screen->deviceHeight()/2 );
    goodcd = cd;
    reset();

    if ( QWSServer::mouseHandler() ) {
      QString calFile = qgetenv("POINTERCAL_FILE");
      if (calFile.isEmpty())
        calFile = "/etc/pointercal";

      qLog(Input) << "Using calibration file " << calFile;
      anygood = QFile::exists(calFile);
     QWSServer::mouseHandler()->getCalibration(&goodcd);
      QWSServer::mouseHandler()->clearCalibration();
    }
    QDialog::showEvent(e);
    setVisible(true);
    setFocusPolicy( Qt::StrongFocus );
    setFocus();
    showFullScreen();
    QTimer::singleShot(0, this, SLOT(doGrab()) );
}

void Calibrate::store()
{
    if ( QWSServer::mouseHandler() && anygood ) {
      qLog(Input) << "Store calibration data to file";
        QFile calFile("/etc/pointercal");
        if(!QFile::exists("/etc/pointercal.orig"))
        calFile.copy("/etc/pointercal.orig");
      QWSServer::mouseHandler()->calibrate( &goodcd );
    }
}

void Calibrate::hideEvent(QHideEvent *e )
{
    store();
    reset();
    QDialog::hide();
    QDialog::reject();
    QDialog::hideEvent(e);
}

void Calibrate::reset()
{
    penPos = QPoint();
    location = QWSPointerCalibrationData::TopLeft;
    crossPos = fromDevice( cd.screenPoints[location] );
    releaseMouse();
    releaseKeyboard();
}

QPoint Calibrate::fromDevice( const QPoint &p )
{
    return qt_screen->mapFromDevice( p,
           QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight()) );
}

bool Calibrate::sanityCheck()
{
    QPoint tl = cd.devPoints[QWSPointerCalibrationData::TopLeft];
    QPoint tr = cd.devPoints[QWSPointerCalibrationData::TopRight];
    QPoint bl = cd.devPoints[QWSPointerCalibrationData::BottomLeft];
    QPoint br = cd.devPoints[QWSPointerCalibrationData::BottomRight];

    int p1 = (br.x() - tl.x()) * (br.x() - tl.x()) +
             (br.y() - tl.y()) * (br.y() - tl.y());

    int p2 = (bl.x() - tr.x()) * (bl.x() - tr.x()) +
             (bl.y() - tr.y()) * (bl.y() - tr.y());

    int d1 = (int)sqrt(p1);
    int d2 = (int)sqrt(p2);

    int avg = abs(d1 + d2) / 2;
    int tol= (int)avg/50; // tolerance of 2% in calibration points
    // Decrease 50 to increase percentage tolerance

    qLog(Input)<<"sanityCheck() d1:"<<d1<<", d2:"<<d2<<", average:"<<avg<<", tolerance:"<<tol;
    if( (d1 >= avg-tol) && (d1 <= avg+tol) ) {
        if( (d2 >= avg-tol) && (d2 <= avg+tol) ) {
            qLog(Input)<< "SanityCheck() return true";
        return true;
        }
    }
    qLog(Input)<< "SanityCheck() return false";
    return false;
}

void Calibrate::moveCrosshair( QPoint pt )
{
    showCross = false;
    repaint( crossPos.x()-20, crossPos.y()-20, 42, 42 );
    showCross = true;
    crossPos = pt;
    repaint( crossPos.x()-20, crossPos.y()-20, 42, 42 );
}

void Calibrate::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int y = height() / 60;

    QFont f = p.font(); f.setBold(true);
    p.setFont( f );
    p.drawText( 0, y, width(), height() - y, Qt::AlignHCenter|Qt::TextWordWrap,
                tr("Recalibration") );

    if ( !logo.isNull() ) {
        y = (height() - logo.height()) / (qMin(height(), width()) / 24);
        p.drawPixmap( (width() - logo.width()) / 2, y, logo );
        y += logo.height()+5;
    }

    y = height() / 2 + 65;

    f.setBold(false);
    p.setFont( f );
    QString rt(tr("Touch the crosshairs firmly\nand accurately to\nrecalibrate the screen."));
    p.drawText( 20, y, width()-40, height()-y, Qt::AlignHCenter|Qt::TextWordWrap, rt);

    if ( showCross ) {
        QRect rectangle(  crossPos.x() - 10 , crossPos.y() - 10, 20, 20);
        p.drawEllipse(rectangle);

        p.drawRect( crossPos.x()-1, crossPos.y()-20, 2, 19 );
        p.drawRect( crossPos.x()-1, crossPos.y()+1, 2, 19 );

        p.drawRect( crossPos.x()-20, crossPos.y()-1, 19, 2 );
        p.drawRect( crossPos.x()+1, crossPos.y()-1, 19, 2 );
    }
}

void Calibrate::keyPressEvent( QKeyEvent *e )
{
    // Because we're a full-screen application, we don't want another
    // program to start (as it will be hidden behind us) or receive key
    // events, so we must handle all key events. The only key events
    // that are meaningful to us are those that will cause us to exit -
    // all others are accepted and not acted on, except for the Hangup key.
    // The Hangup key is special because we must exit and then pass it
    // on to the server (by pretending to ignore it), so that the server
    // can terminate all other applications too.
    if (( e->key() == Qt::Key_Escape ) || ( e->key() == Qt::Key_Back ) ||
        ( e->key() == Qt::Key_Hangup )) {
        reset();
        QDialog::hide();
    }

    if ( e->key() == Qt::Key_Hangup )
        e->ignore();
    else
        e->accept();
}

void Calibrate::keyReleaseEvent( QKeyEvent *e )
{
    // Accept all key releases to prevent them getting passed to anything
    // that is hidden behind us.
    e->accept();
}

void Calibrate::mousePressEvent( QMouseEvent *e )
{
    pressed = true;
    // map to device coordinates
    QPoint devPos = qt_screen->mapToDevice( e->pos(),
           QSize(qt_screen->width(), qt_screen->height()) );
    if ( penPos.isNull() )
      penPos = devPos;
    else
      penPos = QPoint( (penPos.x() + devPos.x())/2,
                     (penPos.y() + devPos.y())/2 );
}

void Calibrate::mouseMoveEvent( QMouseEvent *e )
{
    if ( !pressed )
      return;
    // map to device coordinates
    QPoint devPos = qt_screen->mapToDevice( e->pos(),
                    QSize(qt_screen->width(), qt_screen->height()) );
    if ( penPos.isNull() )
      penPos = devPos;
    else
      penPos = QPoint( (penPos.x() + devPos.x())/2,
                     (penPos.y() + devPos.y())/2 );
}

void Calibrate::mouseReleaseEvent( QMouseEvent * )
{
    if ( !pressed )
      return;
    pressed = false;
    if ( timer->isActive() )
      return;

    bool doMove = true;
    qLog(Input)<< "Location:"<< location << penPos.x() << penPos.y();
    cd.devPoints[location] = penPos;
    if ( location < QWSPointerCalibrationData::LastLocation ) {
      location = (QWSPointerCalibrationData::Location)((int)location + 1);
    } else {
      if ( sanityCheck() ) {
        reset();
        anygood = true;
        goodcd = cd;
        hide();
        doMove = false;
      } else {
        location = QWSPointerCalibrationData::TopLeft;
      }
    }

    if ( doMove ) {
      QPoint target = fromDevice( cd.screenPoints[location] );
      dx = (target.x() - crossPos.x())/10;
      dy = (target.y() - crossPos.y())/10;
      timer->start( 30 );
    }
}

void Calibrate::timeout()
{
    QPoint target = fromDevice( cd.screenPoints[location] );

    bool doneX = false;
    bool doneY = false;
    QPoint newPos( crossPos.x() + dx, crossPos.y() + dy );

    if ( abs(crossPos.x() - target.x()) <= abs(dx) ) {
      newPos.setX( target.x() );
      doneX = true;
    }

    if ( abs(crossPos.y() - target.y()) <= abs(dy) ) {
      newPos.setY(target.y());
      doneY = true;
    }

    if ( doneX && doneY ) {
      penPos = QPoint();
      timer->stop();
    }
    moveCrosshair( newPos );
}

void Calibrate::doGrab()
{
/*
    if ( !QWidget::mouseGrabber() ) {
      grabMouse();
      keyboardGrabber();
    } else {
      QTimer::singleShot( 50, this, SLOT(doGrab()) );
    }
*/
    grabMouse();
    keyboardGrabber();
}

void Calibrate::menuTriggered(QAction * /*action*/)
{
    hide();
}

UIFACTORY_REGISTER_WIDGET( Calibrate );
