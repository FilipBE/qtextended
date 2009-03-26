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
#include "homescreen.h"

#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QSettings>
#include <QDesktopWidget>

#include <QtopiaApplication>
#include <QtopiaChannel>
#include <QFormLayout>
#include <QImageSourceSelectorDialog>
#include <QStorageMetaInfo>
#include <QDrmContent>
#include <QDrmContentPlugin>
#include <QDebug>
#include <QValueSpace>


// a hidden file name, used to copy an image from any removable storage
#define HOMESCREEN_IMAGE_NAME ".HomescreenImage"
#define HOMESCREEN_IMAGE_PATH Qtopia::documentDir() + HOMESCREEN_IMAGE_NAME

#define HOMESCREEN_IMAGE_NAME2 ".SecondaryHomescreenImage"
#define HOMESCREEN_IMAGE_PATH2 Qtopia::documentDir() + HOMESCREEN_IMAGE_NAME2

HomescreenSettings::HomescreenSettings(QWidget* parent, Qt::WFlags fl)
    : QDialog( parent, fl)
{
    setWindowTitle(tr("Homescreen"));

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QTabWidget *tabWidget = new QTabWidget(this);

    //appearance tab
    QWidget *appearance = new QWidget;
    QScrollArea *appearanceWrapper = new QScrollArea;
    appearanceWrapper->setFocusPolicy(Qt::NoFocus);
    appearanceWrapper->setFrameStyle(QFrame::NoFrame);
    appearanceWrapper->setWidget(appearance);
    appearanceWrapper->setWidgetResizable(true);
    appearanceWrapper->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFormLayout *appearanceLayout = new QFormLayout(appearance);

    QSettings config("Trolltech", "qpe");
    config.beginGroup( "HomeScreen" );
    QString hsImgName = config.value("HomeScreenPicture").toString();
    int hsDisplayMode = config.value("HomeScreenPictureMode", 0).toInt();

    image = new QPushButton();
    image->setIconSize(QSize(50,75));
    image->setMinimumHeight(80);

    imageMode = new QComboBox;
    imageMode->addItem(tr("Scale & Crop"));
    imageMode->addItem(tr("Stretch"));
    imageMode->addItem(tr("Tile"));
    imageMode->addItem(tr("Center"));
    imageMode->addItem(tr("Scale"));
    imageMode->setCurrentIndex(hsDisplayMode);

    hsImage = QContent(hsImgName);
    if (!hsImage.isValid()) {
        image->setText(tr("No image"));
        imageMode->setVisible(false);
    }
    else {
        image->setIcon(QIcon(hsImage.fileName()));
    }
    connect( image, SIGNAL(clicked()), this, SLOT(editPhoto()) );

    QVBoxLayout *imageLayout = new QVBoxLayout;
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(0);
    imageLayout->addWidget(image);
    imageLayout->addWidget(imageMode);
    appearanceLayout->addRow(tr("Image"), imageLayout);

    time = new QCheckBox(tr("Time"));
    date = new QCheckBox(tr("Date"));
    op = new QCheckBox(tr("Operator"));
    profile = new QCheckBox(tr("Profile"));
    location = new QCheckBox(tr("Location"));

    time->setCheckState(config.value("ShowTime", "true").toBool() ? Qt::Checked : Qt::Unchecked);
    date->setCheckState(config.value("ShowDate", "true").toBool() ? Qt::Checked : Qt::Unchecked);
    op->setCheckState(config.value("ShowOperator", "true").toBool() ? Qt::Checked : Qt::Unchecked);
    profile->setCheckState(config.value("ShowProfile", "true").toBool() ? Qt::Checked : Qt::Unchecked);
    location->setCheckState(config.value("ShowLocation", "true").toBool() ? Qt::Checked : Qt::Unchecked);

    QVBoxLayout *checkLayout = new QVBoxLayout;
    checkLayout->setContentsMargins(0, 0, 0, 0);
    checkLayout->setSpacing(0);
    checkLayout->addWidget(time);
    checkLayout->addWidget(date);
    checkLayout->addWidget(op);
    checkLayout->addWidget(profile);
    checkLayout->addWidget(location);

    appearanceLayout->addRow(tr("Display"), checkLayout);

    //idle tab
    QWidget *idle = new QWidget;
    QVBoxLayout *idleLayout = new QVBoxLayout(idle);

    QLabel *label = new QLabel(tr("Return to homescreen:"));

    QHBoxLayout *h1 = new QHBoxLayout;
    QHBoxLayout *h2 = new QHBoxLayout;
    QHBoxLayout *h3 = new QHBoxLayout;

    homeScreen = new QComboBox;
    homeScreen->addItem(tr("On display off"));
    homeScreen->addItem(tr("On suspend"));
    homeScreen->addItem(tr("Never"));
    label->setBuddy(homeScreen);
    connect(homeScreen, SIGNAL(activated(int)), this, SLOT(homeScreenActivated(int)));

    QString showHomeScreen = config.value("ShowHomeScreen", "Never").toString();
    if (showHomeScreen == "DisplayOff")
        homeScreen->setCurrentIndex(0);
    else if (showHomeScreen == "Suspend")
        homeScreen->setCurrentIndex(1);
    else
        homeScreen->setCurrentIndex(2);

    if (Qtopia::mousePreferred())
        lock = new QCheckBox(tr("Lock screen"));
    else
        lock = new QCheckBox(tr("Lock keys"));
    lock->setCheckState(config.value("AutoKeyLock", false).toBool() ? Qt::Checked : Qt::Unchecked);
    lock->setEnabled(homeScreen->currentIndex() == homeScreen->count()-1 ? false : true);

    powerNote = new QLabel;
    powerNote->setWordWrap( true );
    QFont font = QApplication::font();
    font.setItalic( true );
    powerNote->setFont( font );
    homeScreenActivated( homeScreen->currentIndex() );

    screenSaver_vsi = new QValueSpaceItem( "/Hardware/ScreenSaver/State", this );
    connect( screenSaver_vsi, SIGNAL(contentsChanged()), this, SLOT(homeScreenActivated()) );
    h1->addSpacing(20);
    h1->addWidget(homeScreen);
    h2->addSpacing(20);
    h2->addWidget(lock);
    h3->addSpacing(20);
    h3->addWidget(powerNote);

    idleLayout->addWidget(label);
    idleLayout->addLayout(h1);
    idleLayout->addLayout(h2);
    idleLayout->addLayout(h3);
    idleLayout->addStretch(1);

    //secondary screen tab
    //TODO: reduce amount of duplicated code between tabs
    if (QApplication::desktop()->numScreens() > 1) {
        QWidget *secondary = new QWidget;
        QScrollArea *secondaryWrapper = new QScrollArea;
        secondaryWrapper->setFocusPolicy(Qt::NoFocus);
        secondaryWrapper->setFrameStyle(QFrame::NoFrame);
        secondaryWrapper->setWidget(secondary);
        secondaryWrapper->setWidgetResizable(true);
        secondaryWrapper->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QFormLayout *secondaryLayout = new QFormLayout(secondary);

        hsImgName = config.value("SecondaryHomeScreenPicture").toString();
        hsDisplayMode = config.value("SecondaryHomeScreenPictureMode", 0).toInt();

        secondaryImage = new QPushButton();
        secondaryImage->setIconSize(QSize(50,75));
        secondaryImage->setMinimumHeight(80);

        secondaryImageMode = new QComboBox;
        secondaryImageMode->addItem(tr("Scale & Crop"));
        secondaryImageMode->addItem(tr("Stretch"));
        secondaryImageMode->addItem(tr("Tile"));
        secondaryImageMode->addItem(tr("Center"));
        secondaryImageMode->addItem(tr("Scale"));
        secondaryImageMode->setCurrentIndex(hsDisplayMode);

        secondaryHsImage = QContent(hsImgName);
        if (!secondaryHsImage.isValid()) {
            secondaryImage->setText(tr("No image"));
            secondaryImageMode->setVisible(false);
        }
        else {
            secondaryImage->setIcon(QIcon(secondaryHsImage.fileName()));
        }
        connect( secondaryImage, SIGNAL(clicked()), this, SLOT(editSecondaryPhoto()) );

        QVBoxLayout *secondaryImageLayout = new QVBoxLayout;
        secondaryImageLayout->setContentsMargins(0, 0, 0, 0);
        secondaryImageLayout->setSpacing(0);
        secondaryImageLayout->addWidget(secondaryImage);
        secondaryImageLayout->addWidget(secondaryImageMode);
        secondaryLayout->addRow(tr("Image"), secondaryImageLayout);

        tabWidget->addTab(secondaryWrapper, tr("Secondary","Secondary Display"));
    }

    tabWidget->insertTab(0, appearanceWrapper, tr("Appearance"));
    tabWidget->insertTab(1, idle, tr("Idle"));
    tabWidget->setCurrentIndex(0);

    layout->addWidget(tabWidget);

    QDrmContentPlugin::initialize();
}

void HomescreenSettings::accept()
{
    //write settings

    //appearance
    QSettings config("Trolltech", "qpe");
    config.beginGroup("HomeScreen");
    config.setValue("ShowTime", (time->checkState() == Qt::Checked));
    config.setValue("ShowDate", (date->checkState() == Qt::Checked));
    config.setValue("ShowOperator", (op->checkState() == Qt::Checked));
    config.setValue("ShowProfile", (profile->checkState() == Qt::Checked));
    config.setValue("ShowLocation", (location->checkState() == Qt::Checked));
    config.setValue("HomeScreenPicture", hsImage.fileName());
    config.setValue("HomeScreenPictureMode", imageMode->currentIndex());
    if (QApplication::desktop()->numScreens() > 1) {
        config.setValue("SecondaryHomeScreenPicture", secondaryHsImage.fileName());
        config.setValue("SecondaryHomeScreenPictureMode", secondaryImageMode->currentIndex());
    }
    config.setValue("AutoKeyLock", (lock->checkState() == Qt::Checked));

    switch (homeScreen->currentIndex()) {
    case 0:
        config.setValue("ShowHomeScreen", "DisplayOff");
        break;
    case 1:
        config.setValue("ShowHomeScreen", "Suspend");
        break;
    default:
        config.setValue("ShowHomeScreen", "Never");
    }

    config.sync();

    QtopiaChannel::send("QPE/System", "updateHomeScreenImage()");
    QtopiaChannel::send("QPE/System", "updateHomeScreenInfo()");

    QDialog::accept();
}

void HomescreenSettings::reject()
{
    QDialog::reject();
}

void HomescreenSettings::editPhoto()
{
    QImageSourceSelectorDialog *dlg = new QImageSourceSelectorDialog( this );
    dlg->setMaximumImageSize( QApplication::desktop()->availableGeometry().size()  );
    dlg->setContent( hsImage );
    dlg->setModal( true );
    dlg->setWindowTitle( tr("Homescreen Photo") );
    if( QtopiaApplication::execDialog( dlg ) == QDialog::Accepted ) {
        if (!dlg->content().isValid()) {
            hsImage = QContent();
            image->setIcon(QIcon());
            image->setText(tr("No image"));
            imageMode->setVisible(false);
            return;
        }

        hsImage = dlg->content();
        image->setIcon(QIcon(hsImage.fileName()));
        image->setText(QString());
        imageMode->setVisible(true);

        //BUG: shouldn't change files here (in case use cancels dialog)
        //     should only change document filename
        qLog(Resource) << "Selected Document:" << hsImage.fileName();
        // remove old image
        QFile::remove( HOMESCREEN_IMAGE_PATH );
        // check if the file is from a removable storage
        const QFileSystem *fs = QStorageMetaInfo::instance()->fileSystemOf(hsImage.fileName());

        if (fs && fs->isRemovable()) {
            qLog(Resource) << hsImage.fileName() << "is from a removable storage," << hsImage.media();
            // check if the file has Distribute right
            QDrmContent imgContent( QDrmRights::Distribute, QDrmContent::NoLicenseOptions );
            if ( imgContent.requestLicense( hsImage ) ) {
                qLog(Resource) << "File has Distribute right";
                // save a copy
                QFile oldFile( hsImage.fileName() );
                if ( oldFile.copy( HOMESCREEN_IMAGE_PATH ) ) {
                    hsImage = QContent( HOMESCREEN_IMAGE_PATH );
                    qLog(Resource) << "Successfully copied" << hsImage.fileName() << "to" << HOMESCREEN_IMAGE_PATH;
                } else {
                    qLog(Resource) << "Failed to copy" << hsImage.fileName() << ". Need to use as is.";
                }
            } else {
                qLog(Resource) << "File does not have Distribute right";
            }
        }
    }
    delete dlg;
}

//TODO: reduce duplicated code
void HomescreenSettings::editSecondaryPhoto()
{
    QImageSourceSelectorDialog *dlg = new QImageSourceSelectorDialog( this );
    dlg->setMaximumImageSize( QApplication::desktop()->availableGeometry(1).size()  );
    dlg->setContent( secondaryHsImage );
    dlg->setModal( true );
    dlg->setWindowTitle( tr("Homescreen Photo") );
    if( QtopiaApplication::execDialog( dlg ) == QDialog::Accepted ) {
        if (!dlg->content().isValid()) {
            secondaryHsImage = QContent();
            secondaryImage->setIcon(QIcon());
            secondaryImage->setText(tr("No image"));
            secondaryImageMode->setVisible(false);
            return;
        }

        secondaryHsImage = dlg->content();
        secondaryImage->setIcon(QIcon(secondaryHsImage.fileName()));
        secondaryImage->setText(QString());
        secondaryImageMode->setVisible(true);

        //BUG: shouldn't change files here (in case use cancels dialog)
        //     should only change document filename
        qLog(Resource) << "Selected Document:" << secondaryHsImage.fileName();
        // remove old image
        QFile::remove( HOMESCREEN_IMAGE_PATH2 );
        // check if the file is from a removable storage
        const QFileSystem *fs = QStorageMetaInfo::instance()->fileSystemOf(hsImage.fileName());

        if (fs && fs->isRemovable()) {
            qLog(Resource) << secondaryHsImage.fileName() << "is from a removable storage," << secondaryHsImage.media();
            // check if the file has Distribute right
            QDrmContent imgContent( QDrmRights::Distribute, QDrmContent::NoLicenseOptions );
            if ( imgContent.requestLicense( secondaryHsImage ) ) {
                qLog(Resource) << "File has Distribute right";
                // save a copy
                QFile oldFile( secondaryHsImage.fileName() );
                if ( oldFile.copy( HOMESCREEN_IMAGE_PATH2 ) ) {
                    secondaryHsImage = QContent( HOMESCREEN_IMAGE_PATH2 );
                    qLog(Resource) << "Successfully copied" << secondaryHsImage.fileName() << "to" << HOMESCREEN_IMAGE_PATH2;
                } else {
                    qLog(Resource) << "Failed to copy" << secondaryHsImage.fileName() << ". Need to use as is.";
                }
            } else {
                qLog(Resource) << "File does not have Distribute right";
            }
        }
    }
    delete dlg;
}

void HomescreenSettings::homeScreenActivated(int item)
{
    int index = item;
    if ( index < 0 )
        index = homeScreen->currentIndex();

    lock->setEnabled(index == homeScreen->count()-1 ? false : true);

    QSettings config("Trolltech", "qpe");
    bool onBattery = !( powerStatus.wallStatus() == QPowerStatus::Available );
    if ( onBattery )
        config.beginGroup( QLatin1String("BatteryPower") );
    else
        config.beginGroup( QLatin1String("ExternalPower") );
    switch ( index ) {
        case 0:  //Display off
            if ( !config.value( QLatin1String("LightOff"), false ).toBool() ) {
                powerNote->setText(tr("Note: 'Display off' option is currently "
                        "disabled by the power management."));
                powerNote->setVisible( true );
            } else {
                powerNote->setVisible( false );
            }
            break;
        case 1:  //On Suspend
            if ( !config.value( QLatin1String("Suspend"), true ).toBool() ) {
                powerNote->setText(tr("Note: Suspension is currently disabled by the power management."));
                powerNote->setVisible( true );
            } else {
                powerNote->setVisible( false );
            }
            break;
        default:
            powerNote->setVisible( false );
    }


}

void HomescreenSettings::appMessage( const QString &msg, const QByteArray & )
{
    if ( msg == "HomescreenSettings::configure()" ) {
        QtopiaApplication::instance()->showMainWidget();
    }
}
