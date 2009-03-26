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

#ifndef RADIOPLAYER_H
#define RADIOPLAYER_H

#include <QMainWindow>
#include <QIcon>
#include <QAction>
#include <QKeyEvent>

#include "ui_radioplayer.h"
#include "radiobandmanager.h"

class RadioPresetsModel;

class RadioPlayer : public QMainWindow
{
    Q_OBJECT
public:
    RadioPlayer( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~RadioPlayer();

protected:
    void keyPressEvent( QKeyEvent *e );

public slots:
    void setMute( bool value );
    void setStation( const QString& band, RadioBand::Frequency frequency );
    void setDocument( const QString& doc );

private slots:
    void muteOrUnmute();
    void toggleSpeaker();
    void removeVolumeDisplay();
    void changeBand();
    void addToSpeedDial();
    void presets();
    void addToPresets();
    void scanFoundStation( RadioBand::Frequency frequency, int band );
    void updateScanActions();
    void updateStationDetails();

private:
    void initUi();
    void updateMute();
    void updateSpeaker();
    void adjustVolume( int diff );

private:
    Ui_RadioPlayer *ui;
    RadioBandManager *radio;
    QMenu *menu;
    QAction *muteAction;
    QAction *addToSpeedDialAction;
    QAction *presetsAction;
    QAction *addToPresetsAction;
    QAction *scanForwardAction;
    QAction *scanBackwardAction;
    QAction *scanAllAction;
    QAction *stopScanAction;
    QAction *speakerAction;
    QTimer *volumeDisplayTimer;
    QAction **bandActions;
    int numBandActions;
    RadioPresetsModel *presetsModel;
    QString stationGenre;
};

#endif
