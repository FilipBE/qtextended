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

#include "ringprofile.h"
#include <QSettings>
#include <qtopiaapplication.h>
#include <qtopiaservices.h>
#include <qspeeddial.h>
#include <qtranslatablesettings.h>
#include <qtopiaipcenvelope.h>
#include <QtopiaItemDelegate>
#include <QDialog>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QMessageBox>
#include <QLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>
#include <QSlider>
#include <qsoftmenubar.h>
#include <QKeyEvent>
#include <QDrmContentPlugin>
#include <QTimer>

#if defined(QTOPIA_TELEPHONY)
#include "ringtoneeditor.h"
#endif

static QIcon *note = 0;
static QIcon *systemowned = 0;

ProfileEditDialog::ProfileEditDialog(QWidget *parent, bool isNew)
    : QDialog(parent), settingTab(0), isActive(false), isLoading(false),
    deleteDays(false), editVolume(false)
{
    setObjectName( isNew ? "new" : "edit" );

    tabWidget = new QTabWidget( this );

    // construct info tab
    infoTab = new QWidget( tabWidget );

    QFormLayout *infoLayout = new QFormLayout;
    infoLayout->setSpacing( 6 );
    infoLayout->setMargin( 6 );

    profileName = new QLineEdit( infoTab );
    infoLayout->addRow( tr( "Name" ), profileName );

    masterVolume = new QSlider( Qt::Horizontal, infoTab );
    masterVolume->setMinimum( 0 );
    masterVolume->setMaximum( 5 );
    masterVolume->setTickPosition( QSlider::TicksBelow );
    masterVolume->setTickInterval( 1 );
    masterVolume->setSingleStep( 1 );
    infoLayout->addRow( tr( "Volume" ), masterVolume );

#if defined(QTOPIA_CELL) || defined(QTOPIA_VOIP)
    videoOption = new QComboBox( infoTab );
    videoOption->addItem( tr( "Always off" ) );
    videoOption->addItem( tr( "On for incoming calls" ) );
    videoOption->addItem( tr( "On for outgoing calls" ) );
    videoOption->addItem( tr( "Always on" ) );
    infoLayout->addRow( tr( "Video" ), videoOption );
#endif

#if defined(QTOPIA_TELEPHONY)
    autoAnswer = new QCheckBox( tr( "Auto answer" ), infoTab );
    infoLayout->addRow( autoAnswer );

    vibrateAlert = new QCheckBox( tr( "Vibrate active" ), infoTab );
    infoLayout->addRow( vibrateAlert );
#endif

    autoActivation = new QCheckBox( tr( "Auto activation" ), infoTab );
    infoLayout->addRow( autoActivation );

    editSchedule = new QPushButton( tr( "Edit" ), infoTab );
    editSchedule->setEnabled( false );
    infoLayout->addRow( editSchedule );

    infoTab->setLayout(infoLayout);

    QScrollArea *scrollArea1 = new QScrollArea();
    scrollArea1->setFocusPolicy(Qt::NoFocus);
    scrollArea1->setFrameStyle(QFrame::NoFrame);
    scrollArea1->setWidgetResizable( true );
    scrollArea1->setWidget( infoTab );
    scrollArea1->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    tabWidget->addTab( scrollArea1, tr( "Profile" ) );

#if defined(QTOPIA_TELEPHONY)
    // construct tone tab
    toneTab = new QWidget( tabWidget );
    QVBoxLayout *toneTabLayout = new QVBoxLayout( toneTab );

    // ring tone group box
    QGroupBox *ringToneGroup = new QGroupBox( tr( "Ring tone" ), toneTab );
    QFormLayout *ringToneLayout = new QFormLayout( ringToneGroup );

    ringType = new QComboBox( ringToneGroup );
    ringType->addItem( tr( "Off" ) );
    ringType->addItem( tr( "Once" ) );
    ringType->addItem( tr( "Continuous" ) );
    ringType->addItem( tr( "Ascending" ) );
    ringToneLayout->addRow( tr( "Alert" ), ringType );

    ringTone = new RingToneButton( ringToneGroup );
    ringToneLayout->addRow( tr( "Tone" ), ringTone );
    connect( ringTone, SIGNAL(selected(QContent)),
            this, SLOT(toneSelected(QContent)) );

    videoTone = new RingToneButton( ringToneGroup );
    videoTone->setVideoSelector( true );
    ringToneLayout->addRow( tr( "Video Tone" ), videoTone );
    connect( videoTone, SIGNAL(selected(QContent)),
            this, SLOT(toneSelected(QContent)) );

    toneTabLayout->addWidget( ringToneGroup );

    // message tone group box
    QGroupBox *messageToneGroup = new QGroupBox( tr( "Message tone" ), toneTab );
    QFormLayout *messageToneLayout = new QFormLayout( messageToneGroup );

    messageType = new QComboBox( messageToneGroup );
    messageType->addItem( tr( "Off" ) );
    messageType->addItem( tr( "Once" ) );
    messageType->addItem( tr( "Continuous" ) );
    messageType->addItem( tr( "Ascending" ) );
    messageToneLayout->addRow( tr( "Alert" ), messageType );

    messageAlertDuration = new QSpinBox( messageToneGroup );
    messageAlertDuration->setSuffix( tr( "s", "seconds" ) );
    messageToneLayout->addRow( tr( "Duration" ), messageAlertDuration );

    messageTone = new RingToneButton( messageToneGroup );
    messageToneLayout->addRow( tr( "Tone" ), messageTone );

    toneTabLayout->addWidget( messageToneGroup );

    QScrollArea *scrollArea2 = new QScrollArea();
    scrollArea2->setFocusPolicy(Qt::NoFocus);
    scrollArea2->setFrameStyle(QFrame::NoFrame);
    scrollArea2->setWidgetResizable( true );
    scrollArea2->setWidget( toneTab );
    scrollArea2->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    tabWidget->addTab( scrollArea2, tr( "Tones" ) );
#endif

    // properties tab
    settingTab =  new QWidget( tabWidget );
    QVBoxLayout *settingLayout = new QVBoxLayout( settingTab );
    settingListWidget = new QListWidget( settingTab );
    settingListWidget->setFrameStyle(QFrame::NoFrame);
    settingLayout->setMargin(0);
    settingLayout->addWidget( settingListWidget );
    tabWidget->addTab( settingTab, tr( "Properties" ) );

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    actionView = contextMenu->addAction( QIcon( ":icon/edit" ), tr( "View/Edit" ) );
    actionDelete = contextMenu->addAction( QIcon( ":icon/trash" ), tr( "Delete" ) );
    actionCapture = contextMenu->addAction( QIcon( ":icon/settings" ), tr("Capture properties") );
    setSoftMenu(0);

    // setup main layout
    QVBoxLayout *vLayout = new QVBoxLayout( this );
    vLayout->setMargin( 0 );
    vLayout->setSpacing( 0 );
    vLayout->addWidget( tabWidget );

    masterVolume->installEventFilter( this );

#if defined(QTOPIA_TELEPHONY)
    connect( profileName, SIGNAL(textChanged(QString)), this, SLOT(updateState()) );
    connect( masterVolume, SIGNAL(valueChanged(int)), this, SLOT(updateState()) );
    connect( videoOption, SIGNAL(activated(int)), this, SLOT(updateState()) );
    connect( vibrateAlert, SIGNAL(toggled(bool)), this, SLOT(updateState()) );
    connect( autoAnswer, SIGNAL(toggled(bool)), this, SLOT(updateState()) );
    connect( autoActivation, SIGNAL(toggled(bool)), this, SLOT(updateState()) );
    connect( editSchedule, SIGNAL(clicked()), this, SLOT(updateState()) );
    connect( ringType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()) );
    connect( ringTone, SIGNAL(clicked()), this, SLOT(updateState()) );
    connect( videoTone, SIGNAL(clicked()), this, SLOT(updateState()) );
    connect( messageType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()) );
    connect( messageAlertDuration, SIGNAL(valueChanged(int)), this, SLOT(updateState()) );
    connect( messageTone, SIGNAL(clicked()), this, SLOT(updateState()) );
#endif
    connect( actionView, SIGNAL(triggered()), this, SLOT(viewSetting()) );
    connect( actionDelete, SIGNAL(triggered()), this, SLOT(deleteSetting()) );
    connect( actionCapture, SIGNAL(triggered()), this, SLOT(pullSettingStatus()) );
    connect( autoActivation, SIGNAL(toggled(bool)), this, SLOT(enableEditSchedule(bool)) );
    connect( editSchedule, SIGNAL(clicked()), this, SLOT(showEditScheduleDialog()) );
    connect( tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setSoftMenu(int)) );
    connect( tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
}

ProfileEditDialog::~ProfileEditDialog()
{
    if ( deleteDays )
        delete [] days;
}

void ProfileEditDialog::setProfile(PhoneProfileItem *pItem)
{
    profile = pItem;
    setPhoneProfile();
    setSchedule();
    setSettings();
}

void ProfileEditDialog::setPhoneProfile()
{
    const QPhoneProfile &p = profile->profile();
    profileName->setText( p.name() );
    if ( p.isSystemProfile() ) {
        profileName->setReadOnly( true );
        profileName->setFrame( false );
        profileName->setFocusPolicy( Qt::NoFocus );
    } else {
        profileName->setFrame( false );
        profileName->setFrame( true );
        profileName->setFocusPolicy( Qt::StrongFocus );
    }

    masterVolume->setValue((int)p.volume());
#if defined(QTOPIA_TELEPHONY)
    videoOption->setCurrentIndex(p.videoOption());
    vibrateAlert->setChecked(p.vibrate());
    autoAnswer->setChecked(p.autoAnswer());
    ringType->setCurrentIndex((int)p.callAlert());
    // if none of the tone is valid use the system tone.
    if ( !p.callTone().isValid() && !p.videoTone().isValid() )
        ringTone->setTone(p.systemCallTone());
    else {
        ringTone->setTone(p.callTone());
        videoTone->setTone(p.videoTone());
    }
    messageType->setCurrentIndex((int)p.msgAlert());
    messageAlertDuration->setValue(p.msgAlertDuration()/1000);
    if ( p.messageTone().isValid() )
        messageTone->setTone(p.messageTone());
    else
        messageTone->setTone(p.systemMessageTone());

    ringTone->setEnabled(ringType->currentIndex() != 0 && masterVolume->value() != 0);
    messageType->setEnabled(masterVolume->value() != 0);
    messageTone->setEnabled(messageType->currentIndex() != 0 && masterVolume->value() != 0);
    messageAlertDuration->setEnabled(messageType->currentIndex() != 0 && messageType->currentIndex() != 1 );
#endif

    tabWidget->setCurrentIndex(0);
}

void ProfileEditDialog::setSettings()
{
    settings = profile->profile().applicationSettings();

    int c = settings.count();

    if ( c > 0 ) {
        QStringList keys = settings.keys();

        for ( int i = 0; i < c; i++ ) {
            QListWidgetItem *item = new QListWidgetItem( settings[keys.value(i)].description(), settingListWidget );
            item->setData( Qt::UserRole, QVariant( settings[keys.value(i)].applicationName() ) );
        }

        settingListWidget->setCurrentRow( 0 );
    }
}

void ProfileEditDialog::setSchedule()
{
    isLoading = true;
    schedule = profile->profile().schedule();
    autoActivation->setChecked( schedule.isActive() );
    isLoading = false;
}

void ProfileEditDialog::accept()
{
    // apply changes.
    QString pName = profileName->text();
    QPhoneProfile &p = profile->profile();
    p.setName(!pName.isEmpty() ? pName : tr( "Custom" ));

    p.setVolume(masterVolume->value());
#if defined(QTOPIA_TELEPHONY)
    p.setVideoOption((QPhoneProfile::VideoOption)videoOption->currentIndex());
    p.setVibrate(vibrateAlert->isChecked());
    p.setAutoAnswer(autoAnswer->isChecked());
    p.setCallAlert((QPhoneProfile::AlertType)ringType->currentIndex());
    p.setCallTone(ringTone->tone());
    p.setVideoTone(videoTone->tone());
    p.setMsgAlert((QPhoneProfile::AlertType)messageType->currentIndex());
    p.setMsgAlertDuration(messageAlertDuration->value() * 1000);
    p.setMessageTone(messageTone->tone());
#endif
    p.setSchedule( schedule );
    p.setApplicationSettings( settings );

    QDialog::accept();
}

#if defined(QTOPIA_TELEPHONY)
void ProfileEditDialog::updateState()
{
    int volumeIndex = masterVolume->value();
    ringTone->setEnabled(ringType->currentIndex() != 0 && volumeIndex != 0);
    videoTone->setEnabled(ringType->currentIndex() != 0);
    messageType->setEnabled(masterVolume->value() != 0);
    messageTone->setEnabled(messageType->currentIndex() != 0 && volumeIndex != 0);
    messageAlertDuration->setEnabled(messageType->currentIndex() != 0 && messageType->currentIndex() != 1);
    // set the current volume to be used in preview
    QObject *obj = sender();
    if ( obj == masterVolume ) {
        ringTone->setVolume( volumeIndex );
        videoTone->setVolume( volumeIndex );
        messageTone->setVolume( volumeIndex );
    }
    if ( obj == ringTone ) {
        ringTone->setVolume( volumeIndex );
    } else if ( obj == videoTone ) {
        videoTone->setVolume( volumeIndex );
    } else if ( obj == messageTone )
        messageTone->setVolume( volumeIndex );
}

void ProfileEditDialog::toneSelected( const QContent& tone )
{
    // mutually exclusive
    if ( sender() == ringTone && tone.isValid() )
        videoTone->setTone( QContent() );
    else if ( sender() == videoTone && tone.isValid() )
        ringTone->setTone( QContent() );
}
#endif

void ProfileEditDialog::setSoftMenu( int tab )
{
    if ( tab != tabWidget->indexOf(settingTab) ) {
        actionView->setVisible(false);
        actionDelete->setVisible(false);
        actionCapture->setVisible(false);
    } else {
        if ( settingListWidget->count() > 0 ) {
            actionView->setVisible(true);
            actionDelete->setVisible(true);
        } else {
            actionView->setVisible(false);
            actionDelete->setVisible(false);
        }
        actionCapture->setVisible(true);
    }
}

void ProfileEditDialog::viewSetting()
{
    int row = settingListWidget->currentRow();
    if ( row == -1 )
        return;
    QPhoneProfile::Setting selectedSetting = settings[(settingListWidget->item(row))->data(Qt::UserRole).toString()];

    // XXX - move to QDS?
    QtopiaIpcEnvelope e( "QPE/Application/" + selectedSetting.applicationName(), "Settings::setStatus(bool,QString)" );
    e << isActive /* Is active profile */
      << selectedSetting.data() /* details */;
}

void ProfileEditDialog::deleteSetting()
{
    int row = settingListWidget->currentRow();
    if ( row == -1 )
        return;
    // activate setting to default state if profile is active
    // XXX - move to QDS
    if ( isActive )
        QtopiaIpcEnvelope e( "QPE/Application/"
            + (settingListWidget->item(row))->data(Qt::UserRole).toString(),
            "Settings::activateDefault()" );
    // remove selected setting using application name saved in QListWidgetItem::data()
    settings.remove( (settingListWidget->item(row))->data(Qt::UserRole).toString() );

    delete settingListWidget->takeItem( settingListWidget->currentRow() );
    setSoftMenu( tabWidget->currentIndex() );
}

void ProfileEditDialog::showEditScheduleDialog()
{
    if ( isLoading )
        return;

    QDialog *dlg = new QDialog( this );
    initDialog( dlg );
    if ( QtopiaApplication::execDialog( dlg ) == QDialog::Accepted )
        processSchedule();
    else
        autoActivation->setChecked( false );
    delete dlg;
}

void ProfileEditDialog::enableEditSchedule(bool on)
{
    editSchedule->setEnabled( on );
    if ( !on )
        schedule.setActive(false);
}

void ProfileEditDialog::initDialog( QDialog *dlg )
{
    dlg->setObjectName( "auto-on" );
    dlg->setModal( true );
    dlg->setWindowTitle( tr( "Set Day and Time" ) );
    dlg->showMaximized();
    QVBoxLayout *scheduleVLayout = new QVBoxLayout( dlg );
    scheduleVLayout->setMargin( 10 );
    scheduleVLayout->setSpacing( 10 );
    QHBoxLayout *scheduleHLayout = new QHBoxLayout();

    days = new QCheckBox * [7];
    deleteDays = true;

    for ( int i = 0; i < 7; i++ ) {
        days[i] = new QCheckBox( dlg );
        scheduleVLayout->addWidget( days[i] );
        days[i]->setChecked( false );
        days[i]->setFocusPolicy( Qt::StrongFocus );
        days[i]->setText( QTimeString::nameOfWeekDay( i + 1, QTimeString::Long ) );
    }
    time = new QTimeEdit( );
    time->setTime(QTime(8,0));
    time->setWrapping(true);
    QLabel *timeLabel = new QLabel( tr( "Time" ), dlg );
    timeLabel->setBuddy(time);
    scheduleHLayout->addWidget( timeLabel );
    scheduleHLayout->addWidget( time );

    scheduleVLayout->addLayout( scheduleHLayout );

    for(int ii = Qt::Monday; ii <= Qt::Sunday; ++ii) {
        if(schedule.scheduledOnDay((Qt::DayOfWeek)ii))
            days[ii - 1]->setChecked(true);
    }
    // use current time if no previous value exists.
    if (schedule.time() == QTime())
        time->setTime(QTime::currentTime());
    else
        time->setTime(schedule.time());
}

void ProfileEditDialog::processSchedule()
{
    bool scheduled = false;
    for ( int ii = 0; ii < 7; ii++ ) {
        if ( days[ii]->isChecked() ) {
            scheduled = true;
            schedule.setScheduledDay((Qt::DayOfWeek)(ii + 1));
        } else {
            schedule.unsetScheduledDay((Qt::DayOfWeek)(ii + 1));
        }
    }
    schedule.setActive(scheduled);

    if(!schedule.isActive()) {
        autoActivation->setChecked( false );
    } else {
        schedule.setTime(time->time());
    }
}

void ProfileEditDialog::tabChanged( int index )
{
    if ( index == tabWidget->indexOf(infoTab) ) {
        if ( profileName->isEnabled() )
            profileName->setFocus();
        else
            masterVolume->setFocus();
#if defined(QTOPIA_TELEPHONY)
    } else if ( index == tabWidget->indexOf(toneTab) ) {
        ringType->setFocus();
#endif
    } else if ( index == tabWidget->indexOf(settingTab) ) {
        settingListWidget->setFocus();
    }
#if defined(QTOPIA_TELEPHONY)
    if ( !ringTone->isFinished() )
        ringTone->stopSound();
    else if ( !videoTone->isFinished() )
        videoTone->stopSound();
#endif
}

bool ProfileEditDialog::eventFilter( QObject *, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent*) e;
        switch ( ke->key() ) {
            case Qt::Key_Select:
                editVolume = !editVolume;
#if defined(QTOPIA_TELEPHONY)
                if ( !editVolume ) {
                    ringTone->stopSound();
                    videoTone->stopSound();
                }
#endif
                return false;
                break;
            case Qt::Key_Back:
#if defined(QTOPIA_TELEPHONY)
                if ( !ringTone->isFinished() ) {
                    ringTone->stopSound();
                } else if ( !videoTone->isFinished() ) {
                    videoTone->stopSound();
                }
#endif
                if ( editVolume ) {
                    editVolume = false;
                }
                return false;
                break;
        }
#if defined(QTOPIA_TELEPHONY)
    } else if ( e->type() == QEvent::KeyRelease ) {
        QKeyEvent *ke = (QKeyEvent*) e;
        // play current ringtone at the master volume
        switch ( ke->key() ) {
            case Qt::Key_Left:
            case Qt::Key_Right:
                if ( editVolume ) {
                    if ( ringTone->tone().isValid() )
                        ringTone->playCurrentSound();
                    else if ( videoTone->tone().isValid() )
                        videoTone->playCurrentSound();
                }
                return false;
                break;
        }
#endif
    } else if ( e->type() == QEvent::MouseButtonPress ) {
        masterVolume->setEditFocus( true );
        editVolume = true;
        return false;
    } else if ( e->type() == QEvent::MouseButtonRelease ) {
#if defined(QTOPIA_TELEPHONY)
        if ( ringTone->tone().isValid() )
            ringTone->playCurrentSound();
        else if ( videoTone->tone().isValid() )
            videoTone->playCurrentSound();
#endif
        return false;
    } else if ( e->type() == QEvent::FocusOut ) {
        if ( editVolume ) {
            editVolume = false;
#if defined(QTOPIA_TELEPHONY)
            if ( !ringTone->isFinished() ) {
                ringTone->stopSound();
            } else if ( !videoTone->isFinished() ) {
                videoTone->stopSound();
            }
#endif
        }
    }
   return false;
}

void ProfileEditDialog::pullSettingStatus()
{
    // XXX - move to QDS
    QStringList st = QtopiaService::apps("Settings");
    if ( showSettingList( st ) ) {
        for ( int i = 0; i < st.count(); i++ ) {
            if (!st.at( i ).isEmpty())
                QtopiaIpcEnvelope e( "QPE/Application/" + st.at( i ), "Settings::pullSettingStatus()" );
        }
    }
}

bool ProfileEditDialog::showSettingList( QStringList &settingList )
{
    QDialog *dlg = new QDialog( this );
    dlg->setObjectName( "settings" );
    dlg->setModal( true );
    dlg->setWindowTitle( tr( "Available properties" ) );
    QVBoxLayout *vLayout = new QVBoxLayout( dlg );
    vLayout->setMargin( 4 );

    QList<QCheckBox *> addressList;

    // XXX - move to QDS
    for ( int i = 0; i < settingList.count(); i++ ) {
        QString config = QtopiaService::appConfig( "Settings", settingList.at(i) );
        QTranslatableSettings cfg( config, QSettings::IniFormat );
        cfg.beginGroup( "Standard" );
        addressList.append( new QCheckBox( cfg.value( "Name" ).toString().trimmed(), dlg) );
        vLayout->addWidget( addressList.at( i ) );
    }
    bool accepted = false;
    QtopiaApplication::execDialog( dlg );
    if ( dlg->result() == QDialog::Accepted ) {
        for ( int i = 0; i < addressList.count(); i++ ) {
            if ( !addressList.at(i)->isChecked() )
                settingList.replace( i, "" );
        }
        accepted = true;
    }
    delete dlg;
    return accepted;
}

void ProfileEditDialog::addSetting( const QPhoneProfile::Setting s )
{
    settings.insert( s.applicationName(), s );

    // ensure not to add same item again
    int i = 0;
    for ( ; i < settingListWidget->count(); i++ )
        if ( settingListWidget->item( i )->text() != s.description() )
            continue;
        else
            break;

    if ( i == settingListWidget->count() ) {
        QListWidgetItem *item = new QListWidgetItem( s.description(), settingListWidget );
        item->setData( Qt::UserRole, QVariant( s.applicationName() ) );
    }

    if ( settingListWidget->currentRow() < 0 )
        settingListWidget->setCurrentRow( 0 );

    setSoftMenu( tabWidget->currentIndex() );
}

bool ProfileEditDialog::event(QEvent *e)
{
#if defined(QTOPIA_TELEPHONY)
    if ( e->type() == QEvent::WindowDeactivate ) {
        ringTone->stopSound();
        videoTone->stopSound();
        messageTone->stopSound();
    }
#endif
    return QDialog::event(e);
}

//==========================================================================

PhoneProfileItem::PhoneProfileItem(const QPhoneProfile &pr, QListWidget *l)
: QListWidgetItem(pr.name(), l), rpp(pr)
{
    // Try to load the icon nominated by the profile, otherwise
    // fall back to the default system or custom icon.

    // We have to use QImage to see if we'll get a valid icon, as QIcon::isNull()
    // can still return false if the file doesn't exist.
    QImage image(pr.icon());
    if (!image.isNull())
        cache = QIcon(pr.icon());
    else if ( rpp.isSystemProfile() )
        cache = *systemowned;
    else
        cache = *note;
    setIcon(cache);
}

int PhoneProfileItem::height( const QListWidget* lb ) const
{
    int h = lb ? lb->fontMetrics().lineSpacing() + 2 : 0;
    h = qMax( h, QApplication::globalStrut().height() );
    if( Qtopia::mousePreferred() )
    h = qMax( h, 24 );
    return h;
}

int PhoneProfileItem::width( const QListWidget* lb ) const
{
    int w = lb ? lb->fontMetrics().width( profile().name() ) + 6 : 0;
    return qMax( w, QApplication::globalStrut().width() );
}

//==========================================================================

// Bottom part of first display
class ActiveProfileDisplay : public QWidget
{
    Q_OBJECT
public:
    ActiveProfileDisplay( QWidget *parent, const char *name = 0 );
    ActiveProfileDisplay( const QString &label, QWidget *parent, const char *name = 0 );
    void setText( const QString &txt );
    void setPlaneMode( bool p, bool a );

protected:
    void showEvent( QShowEvent *e );

private:
    void init();
    QPixmap notePix;
    QLabel *pixLA, *textLA, *planePix, *planeLabel;
};

ActiveProfileDisplay::ActiveProfileDisplay( QWidget *parent, const char *name )
    : QWidget( parent )
{
    this->setObjectName( name );
    init();
}

ActiveProfileDisplay::ActiveProfileDisplay( const QString &label, QWidget *parent, const char *name )
    : QWidget( parent )
{
    this->setObjectName( name );
    init();
    setText( label );
}

void ActiveProfileDisplay::init()
{
    QGridLayout *l = new QGridLayout( this );
    pixLA = new QLabel( this );
    pixLA->setMargin( 0 );
    pixLA->setAlignment(Qt::AlignRight);
    textLA = new QLabel( this );
    textLA->setMargin( 0 );
    planePix = new QLabel( this );
    planePix->setMargin( 0 );
    planePix->setAlignment(Qt::AlignRight);
    planeLabel = new QLabel( this );
    planeLabel->setMargin( 0 );
    l->setColumnStretch(1, 100);
    l->addWidget( pixLA, 0, 0, Qt::AlignHCenter );
    l->addWidget( textLA, 0, 1 );
    l->addWidget( planePix, 1, 0, Qt::AlignHCenter );
    l->addWidget( planeLabel, 1, 1 );
    QPixmap pm( ":icon/Note" );
    pixLA->setPixmap( pm.scaled( qApp->style()->pixelMetric(QStyle::PM_SmallIconSize),
                    qApp->style()->pixelMetric(QStyle::PM_SmallIconSize),
                    Qt::KeepAspectRatio ) );
}

void ActiveProfileDisplay::setText( const QString &txt )
{
    QFont f = font();
    f.setItalic( txt.isEmpty() );
    setFont( f );
    textLA->setText( (txt.isEmpty() ? tr("No Profile") : txt) );
}

void ActiveProfileDisplay::setPlaneMode(bool p, bool avail)
{
    if (avail) {
        QPixmap pm( ":icon/aeroplane" );
        planeLabel->setText(p ? tr("Airplane Safe Mode") : QString());
        planePix->setPixmap(p ? pm.scaled( qApp->style()->pixelMetric(QStyle::PM_SmallIconSize),
                    qApp->style()->pixelMetric(QStyle::PM_SmallIconSize),
                    Qt::KeepAspectRatio ) : QPixmap());
    } else {
        planeLabel->hide();
        planePix->hide();
    }
}

void ActiveProfileDisplay::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    setFixedHeight( sizeHint().height() );
}

//===========================================================================

ProfileSelect::ProfileSelect( QWidget *parent, Qt::WFlags f, const char *name )
: QDialog( parent, f ), editDialog(0), activeProfile(0), origPlaneMode(false)
{
#ifndef QT_NO_TRANSLATION
    QStringList trans;
    trans << "QtopiaDefaults";
    QtopiaApplication::loadTranslations(trans);
#endif

    note = new QIcon(":image/Note");
    systemowned = new QIcon(":icon/systemowned");

    this->setObjectName(name);

    isLoading = false;

    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(2);
    l->setSpacing(2);
    itemlist = new QListWidget(this);
    itemlist->setItemDelegate(new QtopiaItemDelegate);
    itemlist->setSelectionMode(QAbstractItemView::SingleSelection);
    itemlist->setFrameStyle(QFrame::NoFrame);
    itemlist->setSpacing(1);
    activeDisplay = new ActiveProfileDisplay( this );

    l->addWidget(itemlist);
    l->addWidget(activeDisplay);

    loadConfig();
#ifdef QTOPIA_CELL // airplane mode only makes sense in cell network environment.
    activeDisplay->setPlaneMode(profMan.planeMode(), profMan.planeModeAvailable());
#endif

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );

    new ProfilesService( this );

    actionNew = new QAction( QIcon( ":icon/new" ), tr( "New" ), this );
    connect( actionNew, SIGNAL(triggered()), this, SLOT(createNewProfile()) );
    actionNew->setWhatsThis( tr("Enter a new profile.") );

    actionEdit = new QAction( QIcon( ":icon/edit" ), tr("Edit" ), this );
    connect( actionEdit, SIGNAL(triggered()), this, SLOT(editProfile()) );

    actionRemove = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
    actionRemove->setWhatsThis( tr("Delete the selected profile.") );
    connect( actionRemove, SIGNAL(triggered()), this, SLOT(removeCurrentProfile()) );
    actionRemove->setEnabled(false);

    actionActivate = new QAction( *note, tr("Activate"), this );
    connect( actionActivate, SIGNAL(triggered()), this, SLOT(activateProfile()) );

    actionFavorites = new QAction( QIcon(":icon/phone/favorite"), tr("Add to Favorites"), this );
    connect( actionFavorites, SIGNAL(triggered()), this, SLOT(addToFavorites()) );

#ifdef QTOPIA_CELL
    actionPlane = new QAction( QIcon( ":icon/aeroplane" ), tr("Airplane Safe Mode"), this );
    actionPlane->setCheckable(true);
    actionPlane->setChecked(profMan.planeMode());
    connect( actionPlane, SIGNAL(toggled(bool)), this, SLOT(setPlaneMode(bool)) );
    actionPlane->setEnabled(profMan.planeModeAvailable());
    actionPlane->setVisible(profMan.planeModeAvailable());
#endif

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(actionNew);
    contextMenu->addAction(actionEdit);
    contextMenu->addAction(actionRemove);
    contextMenu->addAction(actionActivate);
    contextMenu->addAction(actionFavorites);

    rightClickMenu = new QMenu(this);
    rightClickMenu->addAction(actionActivate);
    rightClickMenu->addAction(actionEdit);
    QtopiaApplication::setStylusOperation(itemlist->viewport(), QtopiaApplication::RightOnHold);

#ifdef QTOPIA_CELL
    if (actionPlane->isEnabled())
        contextMenu->insertSeparator(actionFavorites);
    contextMenu->addAction(actionPlane);
#endif

    if (!style()->inherits("QThumbStyle")) {
        connect(itemlist, SIGNAL(itemActivated(QListWidgetItem*)),
                this, SLOT(itemActivated(QListWidgetItem*)));
        connect(itemlist, SIGNAL(itemPressed(QListWidgetItem*)),
                this, SLOT(itemPressed(QListWidgetItem*)));
    }
    connect(itemlist, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateIcons()));
    updateIcons();

    if (itemlist->count() > 0 && itemlist->currentRow() == -1 )
        itemlist->setItemSelected(itemlist->item(0), true);

    setWindowTitle( tr("Profiles") );

    QDrmContentPlugin::initialize();
}

ProfileSelect::~ProfileSelect()
{
}


void ProfileSelect::loadConfig()
{
    isLoading = true;
    setActiveProfile( 0 );
    profMan.sync();

    QList<int> ids = profMan.profileIds();
    for(int ii = 0; ii < ids.count(); ++ii) {
#ifndef QTOPIA_CELL
        if ( profMan.profile(ids.at(ii)).planeMode() )
            continue;
#endif
        PhoneProfileItem *pItem = new PhoneProfileItem(profMan.profile(ids.at(ii)), itemlist);

        if(profMan.profile(ids.at(ii)).id() == profMan.activeProfile().id()) {
            setActiveProfile(pItem);
        }
    }

#ifdef QTOPIA_CELL
    origPlaneMode = profMan.planeMode();
#endif
    isLoading = false;
}

void ProfileSelect::closeEvent(QCloseEvent *e)
{
#ifdef QTOPIA_CELL
    if ( profMan.planeMode() != origPlaneMode )
        origPlaneMode = profMan.planeMode();
#endif
    QDialog::closeEvent(e);
}

void ProfileSelect::addToFavorites()
{
    int c = itemlist->currentRow();
    if( c != -1 ) {
        PhoneProfileItem *pItem = (PhoneProfileItem *)itemlist->item(c);
        QPhoneProfile p = pItem->profile();
        QtopiaServiceRequest req("Profiles","setProfile(int)");
        req << p.id();
        QtopiaServiceDescription desc(req,p.name(),"Note");
        QtopiaServiceRequest favReq("Favorites","add(QtopiaServiceDescription)");
        favReq << desc;
        favReq.send();
    }
}

void ProfileSelect::itemActivated(QListWidgetItem* i)
{
    activateProfile(i);
    close();
}

void ProfileSelect::itemPressed(QListWidgetItem*)
{
    if (QApplication::mouseButtons () & Qt::RightButton)
        rightClickMenu->popup(QCursor::pos());
}

void ProfileSelect::activateProfile()
{
    if( itemlist->currentRow() != -1 )
        activateProfile( itemlist->item( itemlist->currentRow() ) );
}

void ProfileSelect::activateProfile(int id)
{
    for (int i = 0; i < (int)itemlist->count(); i++) {
        PhoneProfileItem *pItem = (PhoneProfileItem *)itemlist->item(i);
        if ( pItem->profile().id() == id ) {
            activateProfile(pItem);
            return;
        }
    }
    // Fail. User is probably not looking at phone. Give warning?
    qApp->beep();
}

void ProfileSelect::activateProfile(QListWidgetItem *activeProfileItem )
{
    setActiveProfile( (PhoneProfileItem *)activeProfileItem );
}

// don't call this before the first call to loadConfig()
void ProfileSelect::setActiveProfile( PhoneProfileItem *pItem, bool force )
{
    if ( pItem && ( force || isLoading || activeProfile != pItem ) ) {
        PhoneProfileItem *prevActiveProfile = activeProfile;
        activeProfile = pItem;
        if( activeProfile )
            itemlist->setCurrentItem( activeProfile );

        activeDisplay->setText( pItem ? pItem->text() : QString() );

#ifdef QTOPIA_CELL
        bool prevPlaneMode = profMan.planeMode();
#endif

        if ( !isLoading )
            profMan.activateProfile(activeProfile?activeProfile->profile().id():-1);
        QFont f;
        f.setBold( true );
        activeProfile->setFont( f );

        //it has really changed, not just setting an active profile from loadConfig()
        if ( !isLoading && prevActiveProfile ) {
            // change fonts.
            f.setBold( false );
            prevActiveProfile->setFont( f );

#ifdef QTOPIA_CELL
            // give warning when changing profiles from airplane mode to other.
            if ( prevActiveProfile->profile().planeMode()
                && !activeProfile->profile().planeMode() ) {
                if ( QMessageBox::warning( this, tr( "Airplane Mode" ),
                        tr( "<qt>Do you wish to stay in Airplane Safe Mode?</qt>" ),
                        QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
                    setPlaneMode( true );
                else
                    setPlaneMode( false );
            } else if ( !prevActiveProfile->profile().planeMode()
                    && !activeProfile->profile().planeMode() ) {
                // inherit previous plane mode.
                setPlaneMode( prevPlaneMode );
            } else if ( activeProfile->profile().planeMode() ) {
                // airplane profile, always on
                setPlaneMode( true );
            }
#endif
        }
    }
}

void ProfileSelect::editProfile()
{
    if( itemlist->currentRow() != -1 )
        editProfile( itemlist->item( itemlist->currentRow() ) );
}

void ProfileSelect::editProfile(QListWidgetItem *i)
{
    editProfile((PhoneProfileItem *)i);
}

void ProfileSelect::editProfile(PhoneProfileItem *i)
{
    editDialog = new ProfileEditDialog(this, false);
    editDialog->setWindowTitle( tr( "Edit profile" ) );
    editDialog->setModal( true );

    editDialog->setProfile(i);
    if ( i == activeProfile )
        editDialog->setActive( true );

    if (QtopiaApplication::execDialog(editDialog) == QDialog::Accepted ) {
        itemlist->currentItem()->setText(i->profile().name());
        // Make the profile name unique
        makeProfileNameUnique( i );
        // Save the profile - also updates stuff if it is currently active
        profMan.saveProfile(i->profile());
    }

    delete editDialog;
}

void ProfileSelect::createNewProfile()
{
    editDialog = new ProfileEditDialog(this, true);
    editDialog->setWindowTitle( tr( "Create profile" ) );
    editDialog->setModal( true );

    QPhoneProfile p = profMan.newProfile();
    p.setName( findUniqueName( tr("Custom"), itemlist->count() ) );

    PhoneProfileItem *li = new PhoneProfileItem(p, itemlist);
    editDialog->setProfile( li );

    // apply contents of i to dialog.
    if (QtopiaApplication::execDialog(editDialog) == QDialog::Accepted ) {
        itemlist->setCurrentItem(li);
        itemlist->currentItem()->setText(li->profile().name());
        // make the profile name unique
        makeProfileNameUnique( li );
        // save profile
        profMan.saveProfile( li->profile() );
    } else
        delete li;

    delete editDialog;
}

QString ProfileSelect::findUniqueName( const QString &name, int curIndex )
{
    int suffix = 1;
    QString underscore( "_" );
    QString curName;
    QStringList nameList;

    for ( int i = 0; i < itemlist->count(); i++ ) {
        // exclude own name from comparison
        if ( i != curIndex )
            nameList.append( itemlist->item( i )->text() );
    }
    // sort names
    nameList.sort();

    for( int i = 0; i < nameList.count(); i++ ) {
        curName = nameList.at( i );
        if ( curName == name ) {
            suffix++;
        } else if ( curName.startsWith( name ) ) {
            curName = curName.remove( name );
            if ( curName.startsWith( underscore ) ) {
                curName = curName.remove( underscore );
                bool ok;
                int num = curName.toInt( &ok );
                if ( ok ) {
                    if ( num >= suffix + 1 )
                        break;
                    else
                        suffix = num + 1;
                }
            }
        }
    }
    if ( suffix > 1 )
        return name + underscore + QString::number( suffix );
    else
        return name;
}

void ProfileSelect::makeProfileNameUnique( PhoneProfileItem *pItem )
{
    QString name = pItem->profile().name();
    QString uniqueName = findUniqueName( name, itemlist->currentRow() );

    if ( name != uniqueName ) {
        pItem->profile().setName(uniqueName);
        itemlist->currentItem()->setText(uniqueName);
    }
}

void ProfileSelect::removeCurrentProfile()
{
    int ci = itemlist->currentRow();
    if (ci < 0) return;
    PhoneProfileItem *pItem = (PhoneProfileItem *)itemlist->item(ci);

    int result = QMessageBox::warning( this, tr( "Delete profile" ),
            tr( "<qt>Would you like to delete profile %1?</qt>", "%1 = profile name" ).arg( pItem->profile().name() ),
            QMessageBox::Yes, QMessageBox::No );

    if ( result == (int)QMessageBox::No )
        return;

    // if it is in Favorites, remove the entry
    QPhoneProfile p = pItem->profile();
    QtopiaServiceRequest req("Profiles","setProfile(int)");
    req << p.id();
    QtopiaServiceDescription desc(req,p.name(),"Note");

    //If it doesn't exist, nothing happens
    QtopiaServiceRequest favReq("Favorites", "remove(QtopiaServiceDescription)");
    favReq << desc;
    favReq.send();

    itemlist->takeItem(ci);

    if (pItem->profile().id() == profMan.activeProfile().id()) {
        profMan.removeProfile(pItem->profile());
        activateProfile(itemlist->item(0));
        delete pItem;
    }

}

void ProfileSelect::setPlaneMode(bool p)
{
#ifdef QTOPIA_CELL
    if ( activeProfile->profile().planeMode() && !p ) {
        QMessageBox::warning( this, tr( "Airplane Mode" ),
                tr( "<qt>Cannot turn off Airplane Safe Mode when the profile <b>Airplane</b> is active.</qt>" ) );
        actionPlane->setChecked( true );
        return;
    }
    actionPlane->setChecked(p);
    profMan.setPlaneModeOverride(p);
    activeDisplay->setPlaneMode(p, profMan.planeModeAvailable());
#else
    Q_UNUSED(p);
#endif
}

void ProfileSelect::updateIcons()
{
    if (itemlist->currentItem() < 0)
        return;

    QPhoneProfile p = ((PhoneProfileItem *)itemlist->item(itemlist->currentRow()))->profile();
    bool removable = !p.isSystemProfile() && itemlist->count() > 1;

    actionRemove->setEnabled(removable);
    actionRemove->setVisible(removable);
}

void ProfileSelect::activatePlaneMode()
{
#ifdef QTOPIA_CELL
    bool active = profMan.planeMode();
    if ( active ) {
        if ( activeProfile->profile().planeMode() ) {
            if ( QMessageBox::warning( this, tr( "Airplane Mode" ),
                    tr( "<qt>Cannot disable Airplane Safe Mode when profile <b>Airplane</b> is used.\n"
                       "Do you want to choose other profile?</qt>" ),
                    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
                QtopiaApplication::instance()->showMainWidget();
            }
            return;
        } else {
            if ( QMessageBox::warning( this, tr( "Airplane Mode" ),
                        tr( "<qt>Do you want to disable Airplane Safe Mode?</qt>" ),
                        QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No ) {
                return;
            }
        }
    }

    setPlaneMode( !active );

    QString result;
    result = profMan.planeMode() ?
        tr( "<qt>Airplane Safe Mode is enabled.</qt>" ) :
        tr( "<qt>Airplane Safe Mode is disabled.</qt>" );
    QMessageBox::warning( this, tr( "Airplane Mode" ), result );
#endif
}

void ProfileSelect::appMessage(const QString &msg, const QByteArray &data)
{
    // XXX - move to QDS
    if (msg == "SettingsManager::pushSettingStatus(QString,QString,QString)") {
        hide();
        QDataStream ds(data);
        ds >> appName;
        ds >> appTitle;
        ds >> details;
        capture();
    } else if (msg == "SettingsManager::pullSettingStatus(QString,QString,QString)") {
        int id = itemlist->currentRow();
        if ( id != -1 ) {
            QDataStream ds(data);
            ds >> appName;
            ds >> appTitle;
            ds >> details;
            capture();
        }
    }
}

void ProfileSelect::capture()
{
    PhoneProfileItem *pItem = (PhoneProfileItem *)itemlist->currentItem();
    QPhoneProfile::Setting s;
    s.setApplicationName(appName);
    s.setDescription(appTitle);
    s.setData(details);
    s.setNotifyOnChange(true);

    // update setting list on Edit Dialog
    if(editDialog)
        editDialog->addSetting( s );
    else {
        pItem->profile().setApplicationSetting( s );
        profMan.saveProfile( pItem->profile() );
    }
}

/*!
    \service ProfilesService Profiles
    \inpublicgroup QtTelephonyModule
    \brief The ProfilesService class provides the Profiles service.

    The \i Profiles service enables applications to activate "plane mode",
    or to display the profile editor.
*/

/*!
    \internal
*/
ProfilesService::~ProfilesService()
{
}

/*!
    Activate "plane mode".

    This slot corresponds to the QCop service message
    \c{Profiles::activatePlaneMode()}.
*/
void ProfilesService::activatePlaneMode()
{
#ifdef QTOPIA_CELL
    parent->activatePlaneMode();
#endif
}

/*!
    Show the profile editor.

    This slot corresponds to the QCop service message
    \c{Profiles::showProfiles()}.
*/
void ProfilesService::showProfiles()
{
    parent->raise();
    QtopiaApplication::instance()->showMainWidget();
}

/*!
    Set the current profile to that identified by \a id.

    This slot corresponds to the QCop service message
    \c{Profiles::setProfile(int)}.
*/
void ProfilesService::setProfile( int id )
{
    parent->activateProfile(id);
}

#include "ringprofile.moc"
