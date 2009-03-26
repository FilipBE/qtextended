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
#include "light.h"

#include <qtopiaapplication.h>
#include <custom.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <QDebug>
#include <QLayout>
#include <QScrollArea>
#include <QSettings>
#include <QTabWidget>
#include <qsoftmenubar.h>
#include <QMenu>
#include <QAction>
#include <QValueSpaceItem>

LightSettings::LightSettings( QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl), isStatusView( false )
{
    setWindowTitle(tr("Power Management"));
    QVBoxLayout * baseLayout = new QVBoxLayout( this );
    baseLayout->setMargin( 0 );

    QWidget * container = new QWidget();

    QScrollArea *sView = new QScrollArea;
    sView->setFocusPolicy(Qt::NoFocus);
    sView->setFrameStyle(QFrame::NoFrame);
    sView->setWidget( container );
    sView->setWidgetResizable( true );
    sView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *lightLayout = new QVBoxLayout(container);
    lightLayout->setMargin( 0 );
    b = new LightSettingsContainer();
    QtopiaApplication::setInputMethodHint( b->interval_dim, QtopiaApplication::AlwaysOff );
    QtopiaApplication::setInputMethodHint( b->interval_lightoff, QtopiaApplication::AlwaysOff );
    QtopiaApplication::setInputMethodHint( b->interval_suspend, QtopiaApplication::AlwaysOff );

    lightLayout->addWidget(b);

    baseLayout->addWidget(sView);

    // add context menu to push its status to Profiles
    contextMenu = QSoftMenuBar::menuFor( this );
    QAction* actionCapture = new QAction( QIcon( ":icon/Note" ), tr( "Add to current profile" ), this );
    contextMenu->addAction( actionCapture );
    connect( actionCapture, SIGNAL(triggered()), this, SLOT(pushSettingStatus()) );
    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
        this, SLOT(receive(QString,QByteArray)) );

    connect( b->interval_dim, SIGNAL(valueChanged(int)), this, SLOT(updateLightOffMinValue(int)) );
    connect( b->interval_dim, SIGNAL(valueChanged(int)), this, SLOT(updateSuspendMinValue(int)) );
    connect( b->interval_lightoff, SIGNAL(valueChanged(int)), this, SLOT(updateSuspendMinValue(int)) );

    b->officon->setPixmap(QPixmap(":image/off").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    b->brighticon->setPixmap(QPixmap(":image/Light").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QSettings hwConfig("Trolltech", "Hardware");
    hwConfig.beginGroup("PowerManagement");
    batteryMode.canSuspend = hwConfig.value("CanSuspend", false).toBool();
    externalMode.canSuspend = hwConfig.value("CanSuspendAC", false).toBool();
    hwConfig.endGroup();

    b->notnetworkedsuspend->hide();

    if (batteryMode.canSuspend || externalMode.canSuspend) {
        b->interval_suspend->setEnabled(true);
    } else {
        b->interval_suspend->hide();
        b->label_suspend->hide();
    }

    QSettings config("Trolltech","qpe");

    config.beginGroup("BatteryPower");
    batteryMode.intervalDim = config.value( "Interval_Dim", 20 ).toInt();
    batteryMode.intervalLightOff = config.value("Interval_LightOff", 30).toInt();
    batteryMode.intervalSuspend = config.value("Interval", 60).toInt();
    batteryMode.brightness = config.value("Brightness", 255).toInt();
    batteryMode.brightness = qMax(1,batteryMode.brightness * qpe_sysBrightnessSteps() / 255);
    batteryMode.initBrightness = batteryMode.brightness;
    batteryMode.dim = config.value("Dim", true).toBool();
    batteryMode.lightoff = config.value("LightOff", false).toBool();
    batteryMode.suspend = config.value("Suspend", true).toBool();
    batteryMode.networkedsuspend = config.value("NetworkedSuspend", true).toBool();
    config.endGroup();

    config.beginGroup("ExternalPower");
    externalMode.intervalDim = config.value( "Interval_Dim", 20 ).toInt();
    externalMode.intervalLightOff = config.value("Interval_LightOff", 30).toInt();
    externalMode.intervalSuspend = config.value("Interval", 240).toInt();
    externalMode.brightness = config.value("Brightness", 255).toInt();
    externalMode.brightness = qMax(1,externalMode.brightness * qpe_sysBrightnessSteps() / 255);
    externalMode.initBrightness = externalMode.brightness;
    externalMode.dim = config.value("Dim", true).toBool();
    externalMode.lightoff = config.value("LightOff", false).toBool();   //default to leave on
    externalMode.suspend = config.value("Suspend", true).toBool();
    externalMode.networkedsuspend = config.value("NetworkedSuspend",false).toBool();
    config.endGroup();

    //must set min > 0 the screen will become completely black
    int maxbright = qpe_sysBrightnessSteps();
    b->brightness->setMaximum( maxbright );
    b->brightness->setMinimum( 1 );
    b->brightness->setTickInterval( qMax(1,maxbright/16) );
    b->brightness->setSingleStep( qMax(1,maxbright/16) );
    b->brightness->setPageStep( qMax(1,maxbright/16) );

    currentMode = &batteryMode;
    applyMode();

    //trigger first update of spinboxes minima
    updateLightOffMinValue( b->interval_dim->value() );
    updateSuspendMinValue( b->interval_dim->value() );

    connect(b->powerSource, SIGNAL(currentIndexChanged(int)),
            this, SLOT(powerTypeChanged(int)));
    if ( powerStatus.wallStatus() == QPowerStatus::Available ) {
        b->powerSource->setCurrentIndex(1);
    }

    connect(b->brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));

    QtopiaChannel *channel = new QtopiaChannel("Qtopia/PowerStatus", this);
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(sysMessage(QString,QByteArray)));
}

LightSettings::~LightSettings()
{
}

static void set_fl(int bright)
{
    QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)" );
    e << bright;
    e.send();
}

void LightSettings::reject()
{
    // Set settings for current power source
    if ( powerStatus.wallStatus() == QPowerStatus::Available )
        currentMode = &externalMode;
    else
        currentMode = &batteryMode;

    set_fl(currentMode->initBrightness);

    // Restore brightness settings
    QSettings config("Trolltech", "qpe");
    config.beginGroup("BatteryPower");
    config.setValue("Brightness", (batteryMode.initBrightness * 255 + qpe_sysBrightnessSteps()/2) / qpe_sysBrightnessSteps());
    config.endGroup();
    config.beginGroup("ExternalPower");
    config.setValue("Brightness", (externalMode.initBrightness * 255 + qpe_sysBrightnessSteps()/2) / qpe_sysBrightnessSteps());
    config.endGroup();

    QDialog::reject();
}

void LightSettings::accept()
{
    if ( qApp->focusWidget() )
        qApp->focusWidget()->clearFocus();

    if ( isStatusView ) {
        if ( isFromActiveProfile )
            saveConfig();
        pushSettingStatus();
    } else {
        saveConfig();
        // if the current profile has power management item
        // update the details with the new setting
        QSettings cfg( "Trolltech", "PhoneProfile" );
        cfg.beginGroup( "Profiles" );
        QString activeProfile = cfg.value( "Selected", 1 ).toString();
        cfg.endGroup();
        cfg.beginGroup( "Profile " + activeProfile );
        QString settings = cfg.value( "SettingList" ).toString();
        if ( settings.contains( "light-and-power" ) )
            pushSettingStatus();
    }
    QDialog::accept();
    close();
}

void LightSettings::saveConfig()
{
    // safe call, always one selected.
    powerTypeChanged( b->powerSource->currentIndex() );

    // Set settings for current power source
    if ( powerStatus.wallStatus() == QPowerStatus::Available )
        currentMode = &externalMode;
    else
        currentMode = &batteryMode;

    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("BatteryPower");
    writeMode(cfg, &batteryMode);
    cfg.endGroup();
    cfg.beginGroup("ExternalPower");
    writeMode(cfg, &externalMode);
    cfg.sync();

    int i_dim =      (currentMode->dim ? currentMode->intervalDim : 0);
    int i_lightoff = (currentMode->lightoff ? currentMode->intervalLightOff : 0);
    int i_suspend =  (currentMode->suspend ? currentMode->intervalSuspend : 0);

    set_fl(currentMode->brightness);

    QtopiaServiceRequest e("QtopiaPowerManager", "setIntervals(int,int,int)" );
    e << i_dim << i_lightoff << i_suspend;
    e.send();
}

void LightSettings::writeMode(QSettings &config, PowerMode *mode)
{
    config.setValue( "Dim", mode->dim );
    config.setValue( "LightOff", mode->lightoff );
    config.setValue( "Interval_Dim", mode->intervalDim );
    config.setValue( "Interval_LightOff", mode->intervalLightOff );
    config.setValue( "Brightness", (mode->brightness * 255 + qpe_sysBrightnessSteps()/2) / qpe_sysBrightnessSteps() );
    if (mode->canSuspend) {
        config.setValue( "Interval", mode->intervalSuspend );
        config.setValue( "Suspend", mode->suspend );
    } else {
        config.setValue( "Interval", 0 );
        config.setValue( "Suspend", false );
    }
}

void LightSettings::applyMode()
{
    b->interval_dim->setValue( currentMode->intervalDim );
    if ( !currentMode->dim )
        b->interval_dim->setMinimum( currentMode->intervalDim );
    b->interval_lightoff->setValue( currentMode->intervalLightOff );
    if ( !currentMode->lightoff )
        b->interval_lightoff->setMinimum( currentMode->intervalLightOff );
    b->interval_suspend->setValue( currentMode->intervalSuspend );
    if ( !currentMode->suspend || !currentMode->canSuspend )
        b->interval_suspend->setMinimum( currentMode->intervalSuspend );

    b->brightness->setValue(currentMode->brightness);
    b->interval_suspend->setEnabled( currentMode->canSuspend );
    b->notnetworkedsuspend->setChecked( !currentMode->networkedsuspend );
}

void LightSettings::applyBrightness()
{
    // slot called, but we haven't changed the powerMode values yet
    currentMode->brightness = b->brightness->value();
    set_fl(currentMode->brightness);

    QSettings config("Trolltech", "qpe");
    if (currentMode == &batteryMode)
        config.beginGroup("BatteryPower");
    else
        config.beginGroup("ExternalPower");

    config.setValue("Brightness", (currentMode->brightness * 255 + qpe_sysBrightnessSteps()/2) / qpe_sysBrightnessSteps());
}

void LightSettings::powerTypeChanged(int index)
{
    PowerMode *newMode = &batteryMode;

    if ( index == 1 )
        newMode = &externalMode;

    /*  store everytime (so we can store from accept)   */
    currentMode->intervalDim = b->interval_dim->value();
    currentMode->intervalLightOff = b->interval_lightoff->value();
    currentMode->intervalSuspend = b->interval_suspend->value();
    currentMode->brightness = b->brightness->value();
    currentMode->dim = (b->interval_dim->value() != b->interval_dim->minimum());
    currentMode->lightoff = (b->interval_lightoff->value() != b->interval_lightoff->minimum());
    currentMode->suspend = (b->interval_suspend->value() != b->interval_suspend->minimum());
    currentMode->networkedsuspend = !b->notnetworkedsuspend->isChecked();

    /*  Radio buttons toggled   */
    if ( newMode != currentMode ) {
        currentMode = newMode;
        applyMode();
    }
}

void LightSettings::sysMessage(const QString& msg, const QByteArray& data)
{
    QDataStream s((QByteArray *)&data, QIODevice::ReadOnly);
    if (msg == "brightnessChanged(int)" ) {
        QValueSpaceItem item(QLatin1String("/Hardware/ScreenSaver"));
        const int level = item.value("CurrentLevel",0).toInt();
        if ( level > 0 )
            return;

        int bright;
        s >> bright;
        currentMode->brightness = bright;
 
        b->brightness->disconnect();
        b->brightness->setValue(bright);
        connect(b->brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));
    }
}

void LightSettings::pushSettingStatus()
{
    // send QCop message to record its current status to a selected profile.
    QtopiaServiceRequest e( "SettingsManager", "pushSettingStatus(QString,QString,QString)" );
    e << QString( "light-and-power" ) << QString( windowTitle() ) << status();
    e.send();
}

void LightSettings::pullSettingStatus()
{
    QtopiaServiceRequest e( "SettingsManager", "pullSettingStatus(QString,QString,QString)" );
    e << QString( "light-and-power" ) << QString( windowTitle() ) << status();
    e.send();
}

QString LightSettings::status()
{
    // capture current status
    QString result;
    result = QString::number( (b->interval_dim->value() != b->interval_dim->maximum()) ) + ",";
    result += QString::number( b->interval_dim->value() ) + ",";
    result += QString::number( (b->interval_lightoff->value() != b->interval_lightoff->maximum()) ) + ",";
    result += QString::number( b->interval_lightoff->value() ) + ",";
    result += QString::number( b->brightness->value() ) + ",";
    return result;
}

void LightSettings::setStatus( const QString& details )
{
    QStringList sl = details.split( ',' );
    currentMode = &batteryMode;
    currentMode->dim = sl.at( 0 ).toInt();
    currentMode->intervalDim = sl.at( 1 ).toInt();
    currentMode->lightoff = sl.at( 2 ).toInt();
    currentMode->intervalLightOff = sl.at( 3 ).toInt();
    currentMode->brightness = sl.at( 4 ).toInt();
    applyMode();
}

void LightSettings::receive( const QString& msg, const QByteArray& data )
{
    QDataStream ds((QByteArray *)&data, QIODevice::ReadOnly);
    if (msg == "Settings::setStatus(bool,QString)") {
        // must show widget to keep running
        QtopiaApplication::instance()->showMainWidget();
        isStatusView = true;
        QSoftMenuBar::removeMenuFrom( this, contextMenu );
        delete contextMenu;
        QString details;
        ds >> isFromActiveProfile;
        ds >> details;
        setStatus( details );
    } else if ( msg == "Settings::activateSettings(QString)" ) {
        QString details;
        ds >> details;
        setStatus( details );
        saveConfig();
        hide();
    } else if ( msg == "Settings::pullSettingStatus()" ) {
        pullSettingStatus();
        hide();
    } else if ( msg == "Settings::activateDefault()" ) {
        PowerMode mode;
        batteryMode = mode;
        applyMode();
        saveConfig();
        hide();
    }
}

void LightSettings::updateLightOffMinValue( int )
{
    int minValue = 10;
    
    if ( b->interval_dim->value() != b->interval_dim->minimum() ) {
        minValue = qMax(minValue, b->interval_dim->value());
    }
    
    bool wasSuspendMin = (b->interval_lightoff->value() == b->interval_lightoff->minimum());
    b->interval_lightoff->setMinimum( minValue - 10 );
    if ( wasSuspendMin ) 
        b->interval_lightoff->setValue( minValue - 10  );
    else if ( b->interval_lightoff->value() < minValue )
        b->interval_lightoff->setValue( minValue );
}

void LightSettings::updateSuspendMinValue( int )
{
    int minValue = 10;

    if ( b->interval_dim->value() != b->interval_dim->minimum() )
        minValue = qMax(minValue, b->interval_dim->value());
    if ( b->interval_lightoff->value() != b->interval_lightoff->minimum() )
        minValue = qMax(minValue, b->interval_lightoff->value());

    bool wasSuspendMin = (b->interval_suspend->value() == b->interval_suspend->minimum());
    b->interval_suspend->setMinimum( minValue - 10 );
    if ( wasSuspendMin ) 
        b->interval_suspend->setValue( minValue - 10  );
    else if ( b->interval_suspend->value() < minValue )
        b->interval_suspend->setValue( minValue );
}
