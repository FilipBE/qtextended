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
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include "ui_stopwatchbase.h"
#include <qdatetime.h>

class QTimer;
class QLabel;
class QAnalogClock;
class QBoxLayout;
class QToolButton;

class StopWatch : public QWidget, Ui::StopWatchBase
{
    Q_OBJECT
public:
    StopWatch( QWidget *parent=0, Qt::WFlags fl=0 );
    ~StopWatch();

private slots:
    void stopStartStopWatch();
    void resetStopWatch();
    void prevLap();
    void nextLap();
    void lapTimeout();
    void updateClock();
    void changeClock( bool );

protected:
    void updateLap();
    void setSwatchLcd( QLCDNumber *lcd, int ms, bool showMs );
    bool eventFilter( QObject *o, QEvent *e );
    void showEvent(QShowEvent *e);

private:
    QTimer *t;
    QTime swatch_start;
    int swatch_totalms;
    QVector<int> swatch_splitms;
    bool swatch_running;
    int swatch_currLap;
    int swatch_dispLap;
    QToolButton *prevLapBtn;
    QToolButton *nextLapBtn;
    QTimer *lapTimer;
    QAnalogClock* analogStopwatch;
    QLCDNumber* stopwatchLcd;
    QBoxLayout *swLayout;
    bool init;
    int totalTime;
};

#endif

