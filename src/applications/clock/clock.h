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
#ifndef CLOCK_H
#define CLOCK_H

#include "ui_clockbase.h"
#include <qdatetime.h>
#include <qwidget.h>

class QTimer;
class QLabel;
class QDialog;
class AnalogClock;
class QBoxLayout;
class QToolButton;

class Clock : public QWidget, Ui::ClockBase
{
    Q_OBJECT

public:
    Clock( QWidget *parent=0, Qt::WFlags fl=0 );
    ~Clock();

protected:
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

private slots:
    void updateClock();
    void changeClock( bool );
    void updateDateFormat();

private:
    QTimer *t;
    bool init;
    bool ampm;
    QString dateFormat;
    void getDateFormat();
};

#endif

