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

#include "radioplayer.h"
#include "radioservice.h"
#include "radiopresets.h"

#include <QTimer>
#include <QMenu>
#include <QSoftMenuBar>
#include <QSpeedDial>
#include <QtopiaApplication>
#include <QMessageBox>
#include <QDebug>
#include <QtopiaSql>

RadioPlayer::RadioPlayer( QWidget *parent, Qt::WFlags f )
    : QMainWindow( parent, f ), bandActions(0), numBandActions(0)
{
    setWindowTitle(tr("Radio"));
    setWindowIcon(QIcon( "radioplayer/RadioPlayer"));

    presetsModel = new RadioPresetsModel( this );

    volumeDisplayTimer = new QTimer( this );
    volumeDisplayTimer->setSingleShot( true );
    connect( volumeDisplayTimer, SIGNAL(timeout()),
             this, SLOT(removeVolumeDisplay()) );

    radio = new RadioBandManager( this );
    connect( radio, SIGNAL(signalChanged()),
             this, SLOT(updateStationDetails()) );
    connect( radio, SIGNAL(scanStarted()), this, SLOT(updateScanActions()) );
    connect( radio, SIGNAL(scanStopped()), this, SLOT(updateScanActions()) );
    connect( radio, SIGNAL(scanProgress(RadioBand::Frequency,int)),
             this, SLOT(updateStationDetails()) );
    connect( radio, SIGNAL(scanFoundStation(RadioBand::Frequency,int)),
             this, SLOT(scanFoundStation(RadioBand::Frequency,int)) );

    QWidget *widget = new QWidget(this);

    ui = new Ui_RadioPlayer();
    ui->setupUi( widget );

    setCentralWidget(widget);

    menu = QSoftMenuBar::menuFor( this );

    muteAction = new QAction( tr( "Mute" ), this );
    connect( muteAction, SIGNAL(triggered()), this, SLOT(muteOrUnmute()) );
    muteAction->setWhatsThis( tr( "Mute or unmute radio sound" ) );
    menu->addAction( muteAction );

    speakerAction = new QAction( tr( "Speaker" ), this );
    connect( speakerAction, SIGNAL(triggered()), this, SLOT(toggleSpeaker()) );
    speakerAction->setWhatsThis( tr( "Transfer audio to and from the speaker" ) );
    menu->addAction( speakerAction );

    addToPresetsAction = new QAction( tr( "Add to Saved Stations ..." ), this );
    connect( addToPresetsAction, SIGNAL(triggered()),
             this, SLOT(addToPresets()) );
    addToPresetsAction->setWhatsThis( tr( "Add to list of saved stations" ) );
    menu->addAction( addToPresetsAction );

    presetsAction = new QAction( tr( "Saved Stations ..." ), this );
    connect( presetsAction, SIGNAL(triggered()), this, SLOT(presets()) );
    presetsAction->setWhatsThis( tr( "Select a saved station" ) );
    menu->addAction( presetsAction );

    scanForwardAction = new QAction( tr( "Scan Forward" ), this );
    connect( scanForwardAction, SIGNAL(triggered()), radio, SLOT(scanForward()) );
    scanForwardAction->setWhatsThis( tr( "Scan forward for a new station" ) );
    menu->addAction( scanForwardAction );

    scanBackwardAction = new QAction( tr( "Scan Backward" ), this );
    connect( scanBackwardAction, SIGNAL(triggered()), radio, SLOT(scanBackward()) );
    scanBackwardAction->setWhatsThis( tr( "Scan backward for a new station" ) );
    menu->addAction( scanBackwardAction );

    scanAllAction = new QAction( tr( "Scan All" ), this );
    connect( scanAllAction, SIGNAL(triggered()), radio, SLOT(scanAll()) );
    scanAllAction->setWhatsThis( tr( "Scan for all stations" ) );
    menu->addAction( scanAllAction );

    stopScanAction = new QAction( tr( "Stop Scan" ), this );
    connect( stopScanAction, SIGNAL(triggered()), radio, SLOT(stopScan()) );
    stopScanAction->setWhatsThis( tr( "Stop scanning for stations" ) );
    menu->addAction( stopScanAction );

    addToSpeedDialAction = new QAction( tr( "Add to Speed Dial ..." ), this );
    connect( addToSpeedDialAction, SIGNAL(triggered()),
             this, SLOT(addToSpeedDial()) );
    addToSpeedDialAction->setWhatsThis( tr( "Add station as a speed dial entry" ) );
    menu->addAction( addToSpeedDialAction );

    initUi();

    // Start the "Radio" service to respond to QCop requests.
    new RadioService( this );
}

RadioPlayer::~RadioPlayer()
{
    delete bandActions;
    delete ui;
}

void RadioPlayer::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Left ) {
        // Step left on current band.
        radio->stopScan();
        radio->stepBackward();
        updateStationDetails();
        e->accept();
        return;
    } else if ( e->key() == Qt::Key_MediaPrevious ) {
        if ( radio->signalDetectable() ) {
            // Scan backward for the previous station.
            radio->scanBackward();
        } else {
            // Step left on current band.
            radio->stepBackward();
            updateStationDetails();
        }
        e->accept();
        return;
    } else if ( e->key() == Qt::Key_Right ) {
        // Step right on current band.
        radio->stopScan();
        radio->stepForward();
        updateStationDetails();
        e->accept();
        return;
    } else if ( e->key() == Qt::Key_MediaNext ) {
        if ( radio->signalDetectable() ) {
            // Scan forward for the next station.
            radio->scanForward();
        } else {
            // Step right on current band.
            radio->stepForward();
            updateStationDetails();
        }
        e->accept();
        return;
    } else if ( (e->key() == Qt::Key_MediaPlay || e->key() == Qt::Key_MediaStop ||
                 e->key() == Qt::Key_Select) &&
                radio->scanning() ) {
        // These keys halt scanning if it is currently active.
        radio->stopScan();
        e->accept();
        return;
    } else if ( e->key() == Qt::Key_Up || e->key() == Qt::Key_VolumeUp ) {
        adjustVolume( 5 );
    } else if ( e->key() == Qt::Key_Down || e->key() == Qt::Key_VolumeDown ) {
        adjustVolume( -5 );
    } else if ( e->key() == Qt::Key_VolumeMute ) {
        muteOrUnmute();
    }

    QMainWindow::keyPressEvent(e);
}

void RadioPlayer::setMute( bool value )
{
    radio->setMuted( value );
    updateMute();
}

void RadioPlayer::setStation
        ( const QString& band, RadioBand::Frequency frequency )
{
    if ( !radio->setFrequency( band, frequency ) ) {
        QMessageBox::critical( this, tr("Set Station"),
                tr("<qt>The radio device does not support the %1 band.</qt>")
                        .arg( band ) );
    }
    updateStationDetails();
    updateSpeaker();
}

void RadioPlayer::setDocument( const QString& doc )
{
    QContent content( doc );
    if ( QMessageBox::question( this, tr("Import Stations"),
           tr("<qt>Do you wish to import the radio stations from %1?</qt>")
                .arg( content.name() ),
           QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes ) {
        presetsModel->importStationsFrom( content );
        presets();
    }
}

void RadioPlayer::muteOrUnmute()
{
    radio->setMuted( !radio->muted() );
    updateMute();
}

void RadioPlayer::toggleSpeaker()
{
    radio->setSpeakerActive( !radio->speakerActive() );
    updateSpeaker();
}

void RadioPlayer::removeVolumeDisplay()
{
    ui->volume->hide();
}

void RadioPlayer::changeBand()
{
    // Determine which of the band buttons was triggered.
    QAction *action = (QAction *)sender();
    for ( int index = 0; index < numBandActions; ++index ) {
        if ( action == bandActions[index] ) {
            if ( index != radio->band() ) {
                radio->setBand( index );
                updateStationDetails();
                updateSpeaker();
            }
            break;
        }
    }
}

void RadioPlayer::addToSpeedDial()
{
    if ( !radio->isValid() )
        return;
    QString stationName = ui->stationName->text();
    if ( stationName.isEmpty() )
        stationName = RadioBandManager::formatFrequency( radio->frequency() );
    QtopiaServiceRequest req( "Radio", "setStation(QString,qlonglong)" );
    req << radio->bandName();
    req << (qlonglong)( radio->frequency() );
    QString input = QSpeedDial::addWithDialog
        ( stationName, "radioplayer/RadioPlayer", req, this );
    if ( !input.isEmpty() ) {
        QtopiaServiceDescription desc
            ( req, stationName, "radioplayer/RadioPlayer" );
        QSpeedDial::set( input, desc );
    }
}

void RadioPlayer::presets()
{
    RadioPresetsDialog dialog( presetsModel, this );
    if ( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted ) {
        if ( !dialog.band().isEmpty() )
            setStation( dialog.band(), dialog.frequency() );
    }
    updateStationDetails();
    updateSpeaker();
}

void RadioPlayer::addToPresets()
{
    presetsModel->addToPresets
        ( radio->frequency(), radio->bandName(),
          RadioBandManager::formatFrequency( radio->frequency() ),
          ui->stationName->text(), stationGenre );
    presets();
}

void RadioPlayer::scanFoundStation( RadioBand::Frequency frequency, int band )
{
    Q_UNUSED(frequency);
    Q_UNUSED(band);
    updateStationDetails();
}

void RadioPlayer::updateScanActions()
{
    bool scanning = radio->scanning();
    bool isValid = radio->isValid() && radio->signalDetectable();
    scanForwardAction->setVisible( !scanning && isValid );
    scanForwardAction->setEnabled( !scanning && isValid );
    scanBackwardAction->setVisible( !scanning && isValid );
    scanBackwardAction->setEnabled( !scanning && isValid );
    //scanAllAction->setVisible( !scanning && isValid );
    //scanAllAction->setEnabled( !scanning && isValid );
    scanAllAction->setVisible( false );     // TODO
    scanAllAction->setEnabled( false );
    stopScanAction->setVisible( scanning && isValid );
    stopScanAction->setEnabled( scanning && isValid );
}

void RadioPlayer::initUi()
{
    // Get the list of radio bands supported by this device
    // and the current band that is selected.
    QList<RadioBand *> bands = radio->bands();
    int band = radio->band();

    // Configure the band selection actions on the menu.  No point
    // doing this if there is only one band to choose from.
    if ( bands.size() >= 2 ) {
        numBandActions = bands.size();
        bandActions = new QAction * [numBandActions];
        for ( int index = 0; index < numBandActions; ++index ) {
            QAction *action = new QAction( bands[index]->name(), this );
            action->setCheckable( true );
            action->setChecked( index == band );
            connect( action, SIGNAL(triggered()), this, SLOT(changeBand()) );
            menu->addAction( action );
            bandActions[index] = action;
        }
    } else {
        numBandActions = 0;
        bandActions = 0;
    }

    updateStationDetails();
    updateMute();
    updateSpeaker();
    updateScanActions();
}

void RadioPlayer::updateStationDetails()
{
    RadioPresetsModel::Info info;
    info = presetsModel->stationInfo( radio->frequency(), radio->bandName() );

    ui->frequency->setText
        ( RadioBandManager::formatFrequency( radio->frequency() ) );
    ui->band->setText( radio->bandName() );
    if ( info.isValid ) {
        ui->stationName->setText( info.name );
        ui->stationText->setText( info.genre );
        stationGenre = info.genre;
    } else {
        ui->stationName->setText( QString() );
        ui->stationText->setText( QString() );
        stationGenre = QString();
    }
    if ( radio->signal() )
        ui->signal->setPixmap(QPixmap( ":image/radioplayer/signal_present" ));
    else
        ui->signal->setPixmap(QPixmap( ":image/radioplayer/signal_none" ));
    if ( radio->stereo() )
        ui->stereo->setPixmap(QPixmap( ":image/radioplayer/stereo_present" ));
    else
        ui->stereo->setPixmap(QPixmap( ":image/radioplayer/stereo_none" ));
    ui->volume->hide();

    // Check the band action on the menu that is currently selected.
    if ( numBandActions ) {
        for ( int index = 0; index < numBandActions; ++index ) {
            if ( index == radio->band() )
                bandActions[index]->setChecked( true );
            else
                bandActions[index]->setChecked( false );
        }
    }
}

void RadioPlayer::updateMute()
{
    if ( radio->muted() )
        muteAction->setText( tr("Unmute") );
    else
        muteAction->setText( tr("Mute") );
}

void RadioPlayer::updateSpeaker()
{
    if ( !radio->speakerPresent() ) {
        speakerAction->setEnabled( false );
        speakerAction->setVisible( false );
    } else if ( radio->speakerActive() ) {
        speakerAction->setEnabled( true );
        speakerAction->setVisible( true );
        speakerAction->setText( tr("Headset") );
    } else {
        speakerAction->setEnabled( true );
        speakerAction->setVisible( true );
        speakerAction->setText( tr("Speaker") );
    }
}

void RadioPlayer::adjustVolume( int diff )
{
    radio->adjustVolume( diff );
    ui->volume->setValue( radio->volume() );
    ui->volume->show();
    volumeDisplayTimer->start( 2000 );
}
