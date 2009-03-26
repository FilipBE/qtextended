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

#ifndef WORLDTIME_H
#define WORLDTIME_H

// Qt4 Headers
#include <QTimerEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <QResizeEvent>

#include "cityinfo.h"
#include <qtopiaglobal.h>


const int CITIES = 6;    // the number of cities...

#define WORLDTIME_EXPORT

class QTimeZone;
class QWorldmapDialog;
class QComboBox;
class QWorldmap;

class WORLDTIME_EXPORT WorldTime : public QStackedWidget
{
    Q_OBJECT
public:
    WorldTime(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~WorldTime();

public slots:
    void slotNewTz( const QTimeZone& zone );
    void slotSetZone();
    void slotNewTzCancelled();
    void saveChanges();
    void cancelChanges();

signals:
    void timeZoneListChange();

protected:
    bool isEditMode;
    void timerEvent( QTimerEvent* );
    void keyPressEvent( QKeyEvent* );

private slots:
    void showTime();
    void editMode();
    void viewMode();
    void resetButtons();
    void showMap();
    void initWorldDialog();

private:
    bool changed;
    int selButton;
    int isHighlighted;
    int timerId;
    int maxVisibleZones;

    QWorldmapDialog *worldMapDialog;
    QWidget *zones;

    void readInTimes( void );   // a method to get information from the config
    void writeTimezoneChanges();
   
    // a spot to hold the time zone for each city
    QString strCityTz[CITIES];
    QList<QPushButton *> listCities;
    QList<CityInfo *> listTimes;
    void resizeEvent(QResizeEvent * event ) ;
};

#endif
