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

#include "waitindicator.h"
#include "qtopiaserverapplication.h"

#include <QValueSpaceItem>
#include <QPainter>
#include <QTimer>
#include <QDesktopWidget>
#include <QImageReader>


static const int FadeOutTime = 500;

/*!
  \class WaitIndicator
    \inpublicgroup QtBaseModule
  \brief The WaitIndicator class provides a widget that is shown while an application starts.
  \ingroup QtopiaServer::GeneralUI

  This task provides a widget that is the Qt Extended equivalent to an hour glass.
  It is only visible while the system is busy starting a Qt Extended application. In order to determine 
  whether an application is in the process of being started this task monitors the valuespace
  path \c /System/applications/Info/Busy.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  */

/*! \internal */
WaitIndicator::WaitIndicator()
    : QWidget(0, Qt::Tool|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint)
    , angle(180)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(pal);
    vsi = new QValueSpaceItem("/System/Applications/Info/BusyCount", this);
    connect(vsi, SIGNAL(contentsChanged()), this, SLOT(busyChanged()));
    timeline.setLoopCount(1);
    timeline.setStartFrame(10);
    timeline.setEndFrame(70);
    timeline.setUpdateInterval(100);
    timeline.setDuration(FadeOutTime);
    connect(&timeline, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    connect(&timeline, SIGNAL(finished()), this, SLOT(finished()));
    QSize dsize = QApplication::desktop()->screenGeometry().size();
    int size = qMin(dsize.width(), dsize.height()) / 4 + 1;
    QImageReader imgReader(":image/clock");
    imgReader.setQuality( 49 ); // Otherwise Qt smooth scales
    imgReader.setScaledSize(QSize(size, size));
    waitIcon = QPixmap::fromImage(imgReader.read());
}

/*! \internal */
void WaitIndicator::paintEvent(QPaintEvent* /*e*/)
{
    QPainter p(this);
//    p.setCompositionMode(QPainter::CompositionMode_Clear);
//    p.eraseRect(rect());
//    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(0,0,waitIcon);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::black, 2));
    p.translate(qreal(rect().width())/2.0, qreal(rect().height())/2.0);
    p.rotate(angle);
    p.drawLine(0,0,0,rect().height()/2-waitIcon.width()/8);
}

void WaitIndicator::frameChanged(int frame)
{
    setWindowOpacity(qreal(frame)/100.0);
}

void WaitIndicator::finished()
{
    if (timeline.direction() == QTimeLine::Backward) {
        hide();
        angle = 180;
        timer.stop();
    }
}

/*! \internal */
void WaitIndicator::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timer.timerId()) {
        angle += 6;
        if (angle >= 360)
            angle = 0;
        update();
    } else {
        QWidget::timerEvent(e);
    }
}

void WaitIndicator::busyChanged()
{
    if (vsi->value().toBool()) {
        timeline.setCurrentTime(FadeOutTime);
        if (!isVisible()) {
            QRect srect = QApplication::desktop()->availableGeometry();
            setGeometry(srect.x()+(srect.width()-waitIcon.width())/2,
                    srect.y()+(srect.height()-waitIcon.height())/2,
                    waitIcon.width(), waitIcon.height());
            show();
        }
        timer.start(250, this);
    } else {
        timeline.setDirection(QTimeLine::Backward);
        if(timeline.state() == QTimeLine::NotRunning)
            timeline.start();
    }
}

QTOPIA_TASK( WaitIndicator, WaitIndicator );
