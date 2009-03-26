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

#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <QWSMouseHandler>
#include <QDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QWSServer>

class QTimer;
class QPoint;

class Calibrate : public QDialog
{
    Q_OBJECT
public:
    Calibrate(QWidget* parent=0, Qt::WFlags f=0);
    virtual ~Calibrate();

    void showEvent( QShowEvent *e );
    void hideEvent( QHideEvent *e );

private:
    QPoint fromDevice( const QPoint &p );
    bool sanityCheck();
    void moveCrosshair( QPoint pt );

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void keyPressEvent( QKeyEvent *);
    virtual void keyReleaseEvent( QKeyEvent *);
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );

private slots:
    void timeout();
    void doGrab();
    void menuTriggered(QAction *);

private:
    void store();
    void reset();
    QPixmap logo;
    QWSPointerCalibrationData goodcd,cd;
    QWSPointerCalibrationData::Location location;
    QPoint crossPos;
    QPoint penPos;
    QTimer *timer;
    int dx;
    int dy;
    bool showCross;
    bool pressed;
    bool anygood;
};

#endif
