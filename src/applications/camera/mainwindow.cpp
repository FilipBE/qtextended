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
#include "mainwindow.h"
#include "videocaptureview.h"
#include "ui_camerasettings.h"
#include "phototimer.h"
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qcategorymanager.h>
#include <qstorage.h>

#include <qcontent.h>
#include <qcontentset.h>
#include <qvaluespace.h>

#include <QAction>
#include <QToolButton>
#include <QPushButton>

#include <QSignalMapper>
#include <QImage>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QKeyEvent>
#include <QPainter>
#include <QDSAction>
#include <QDSData>
#include <QDSServices>
#include <QDataStream>
#include <QByteArray>
#include <QDesktopWidget>
#include <QMenu>
#include <QSlider>
#include <QScreen>
#include <QToolBar>
#include <QWaitWidget>

// Customize autofocus key here
static int CUSTOM_AUTOFOCUS_KEY  = Qt::Key_F25;

#ifdef USE_PICTUREFLOW
#include "imagebrowser.h"
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

class MainWindowPrivate
{
public:
#ifdef USE_PICTUREFLOW
    ImageBrowser *browser;
    QAction *a_browse;
#endif
    QAction *a_zoom;
    QSlider *m_zoom;
    int m_currzoom;
    bool zoomActive;
    QAction *a_cancelTimer;
    bool m_inTimerMode;
    CameraSettings settings;

    VideoCaptureView *videocaptureview;
    QToolBar *toolBar;
    PhotoTimer *timer;
    QAction* timerAction;
    bool isTakingPhoto;
    bool hideVideo;
    QWidget *controlsWidget;
};

CameraMainWindow::CameraMainWindow(QWidget *parent, Qt::WFlags f):
    QMainWindow(parent, f),
    settings( NULL ),
    settingsDialog( NULL ),
    a_timer(0),
    a_settings(0),
    snapRequest( 0 ),
    videoOnSecondary(false),
    m_photoContentSet( QContentSet::Asynchronous ),
    m_photoModel( 0 ),
    m_contextMenuActive(false),
    d(new MainWindowPrivate)
{
    setWindowTitle(tr("Camera"));

    namehint=0;
    recording = false;
    hasCamera = false;
    d->a_zoom = 0;
    d->m_inTimerMode = false;
    picfile = Qtopia::tempDir() + "image.jpg";
    m_iswaiting = false;
    d->m_currzoom = 0;
    d->zoomActive = false;
    d->isTakingPhoto = false;
    d->hideVideo = false;
    a_vview = 0;
    shutdown_camera = false;

    launchService();

    d->controlsWidget = new QWidget(this);
    basicControls = new Ui::CameraBase();
    basicControls->setupUi(d->controlsWidget);
    basicControls->photo->hide();
    basicControls->video->hide();

    QTimer::singleShot(1, this, SLOT(init()));
}


CameraMainWindow::~CameraMainWindow()
{
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
    delete d;
}

void CameraMainWindow::setupCameraCategory()
{
    camcat = QLatin1String("Camera");
    // Ensure the Camera system categoy exists
    QCategoryManager catman("Documents");
    // For new code a more unique id should be used instead of using the untranslated text
    // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
    catman.ensureSystemCategory(camcat, camcat);
}

void CameraMainWindow::launchService()
{
    new CameraService(this);

}

void CameraMainWindow::setupSnapshotViewer()
{


    m_photoContentSet.setCriteria( QContentFilter( QContent::Document )
            & QContentFilter::category( QLatin1String( "Camera" ) )
            & QContentFilter::mimeType( QLatin1String( "image/jpeg" ) ) );

    m_photoContentSet.setSortCriteria( QContentSortCriteria( QContentSortCriteria::LastUpdated, Qt::DescendingOrder ) );

    m_photoModel = new QContentSetModel( &m_photoContentSet, this );
#ifdef USE_PICTUREFLOW
    d->browser = new ImageBrowser(m_photoModel,this);
#endif
}

void CameraMainWindow::setupSettingsDialog()
{
    settingsDialog = new QDialog( this );
    settingsDialog->setModal( true );
    settings = new Ui::CameraSettings();
    settings->setupUi( settingsDialog );
    settingsDialog->setObjectName( "settings" );    // To pick up correct help.
    connect( settings->photo_quality, SIGNAL(valueChanged(int)),
        settings->photo_quality_n, SLOT(setNum(int)) );
    connect( settings->video_quality, SIGNAL(valueChanged(int)),
        settings->video_quality_n, SLOT(setNum(int)) );

    QFileSystemFilter *fsf = new QFileSystemFilter;
    fsf->documents = QFileSystemFilter::Set;
    settings->location->setFilter(fsf);

    if(hasCamera) {
        for(int i = 0; i < photo_sizes.size(); ++i) {
            settings->photo_size->addItem(tr("%1 x %2","picture size e.g. 640 x 480")
            .arg(photo_sizes[i].width())
            .arg(photo_sizes[i].height()));
        photoSizeMap.insert(i, photo_sizes[i]);
        }
        if(hasVideo) {
            for(int i = 0; i < video_sizes.size(); ++i) {
                settings->video_size->addItem(tr("%1 x %2","picture size e.g. 640 x 480")
                .arg(video_sizes[i].width())
                .arg(video_sizes[i].height()));
            videoSizeMap.insert(i, video_sizes[i]);
            }
        }else
            settings->video_size->setDisabled(true);
            settings->video_quality->setDisabled(true);
            settings->video_framerate->setDisabled(true);

    }
}

void CameraMainWindow::doInitialCameraSetup()
{

    if (!d->videocaptureview->initializeCamera()) {
        hasCamera = false;
        hasVideo = false;
    }

    else {
        hasCamera = true;
        hasVideo = d->videocaptureview->hasVideo();
        hasStill = d->videocaptureview->hasStill();
    }

    // Load the allowable sizes from the camera hardware.

    if(hasCamera) {
        photo_sizes = d->videocaptureview->stillSizes();
        if(hasVideo) {
            video_sizes = d->videocaptureview->videoSizes();
        }
        preview_sizes = d->videocaptureview->previewSizes();

        QString dpr1,dpr2,dvr1,dvr2;
        dpr1.setNum(d->videocaptureview->stillDefaultSize().width());
        dpr2.setNum(d->videocaptureview->stillDefaultSize().height());
        if(hasVideo) {
            dvr1.setNum(d->videocaptureview->videoDefaultSize().width());
            dvr2.setNum(d->videocaptureview->videoDefaultSize().height());
        } else {
            dvr1.setNum(0);
            dvr2.setNum(0);
        }

        //Settings
        d->settings.load((dpr1+"x"+dpr2), (dvr1+"x"+dvr2), 50, 50, 15);

        setupSettingsDialog();

        settings->location->setLocation(d->settings.location());
        storagepath = settings->location->documentPath();
        photoSize = d->settings.photo();
        videoSize = d->settings.video();

        connect(d->videocaptureview, SIGNAL(imageReadyForSaving(QContent&)), this, SLOT(imageReady(QContent&)));
        connect(d->videocaptureview, SIGNAL(imageReadyRaw(QImage&)), this, SLOT(imageReadyRaw(QImage&)));
    }
}

void CameraMainWindow::init()
{
    QWidget *central = new QWidget(this);

    d->videocaptureview = new VideoCaptureView(central);
    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::Window;
    flags ^= Qt::SubWindow;
    d->videocaptureview->setWindowFlags(flags);
    d->videocaptureview->setWindowState(d->videocaptureview->windowState() | Qt::WindowFullScreen);

    d->videocaptureview->setFocusProxy(central);

    basicControls->photo->show();
    if (!d->hideVideo)
        basicControls->video->show();

    QVBoxLayout * layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(d->videocaptureview);
    layout->addWidget(d->controlsWidget);

    central->setLayout(layout);

    setCentralWidget(central);

    QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableLightOff);

    connect(d->videocaptureview, SIGNAL(lensCoverChanged()), this, SLOT(lensCoverStateChanged()));
    connect(d->videocaptureview, SIGNAL(noCamera()), this, SLOT(noCamera()));

    doInitialCameraSetup();
    setupCameraCategory();
    setupSnapshotViewer();

    connect(basicControls->photo, SIGNAL(clicked()), this, SLOT(takePhoto()));
    connect(basicControls->video, SIGNAL(clicked()), this, SLOT(toggleVideo()));

    d->timer = new PhotoTimer(this);
    connect(d->timer, SIGNAL(takePhoto()), this, SLOT(takePhoto()) );
    connect(d->timer, SIGNAL(takePhoto()), this, SLOT(updateTimerActions()) );


    d->toolBar = new QToolBar(this);
    d->toolBar->setFloatable(false);
    d->toolBar->setMovable(false);
    d->toolBar->setAllowedAreas(Qt::LeftToolBarArea);
    d->timerAction = d->toolBar->addWidget( d->timer);
    d->toolBar->hide();
    addToolBar(Qt::LeftToolBarArea, d->toolBar);

    // Room for longer text
    basicControls->photo->setText(tr("Take Photo"));

    if(!hasCamera) {
        basicControls->photo->hide();
        basicControls->video->hide();
    }
    else {
        basicControls->photo->setFocus();
    }

    if(!hasVideo)
        basicControls->video->setEnabled(false);
    if(!hasStill)
        basicControls->photo->setEnabled(false);

    installEventFilter(basicControls->photo);
    installEventFilter(basicControls->video);
    basicControls->photo->installEventFilter(this);
    basicControls->video->installEventFilter(this);

    if (QApplication::desktop()->numScreens() > 1) {
        // We have a secondary display - watch for the clamshell open/close
        clamshellVsi = new QValueSpaceItem("/Hardware/Devices/ClamshellOpen", this);
        connect(clamshellVsi, SIGNAL(contentsChanged()), this, SLOT(clamshellChanged()));
        if (!clamshellVsi->value().toBool()) {
            videoToScreen(1);
        }
    }

    m_wait = 0;

    QMimeType m( QLatin1String( "image/jpeg" ));
    QContent a = m.application();
    QIcon picViewIcon = a.icon();
    if ( picViewIcon.isNull() )
        picViewIcon = QIcon( QLatin1String( ":icon/view" ));

    if (hasVideo && a.isValid() )
    {
        a_vview = new QAction( QIcon(":image/"+a.iconName()), QString("%1...").arg(a.name()), this );
        connect( a_vview, SIGNAL(triggered()), this, SLOT(viewVideos()) );
    }

#ifndef USE_PICTUREFLOW
    a_pview = new QAction( QIcon(), tr( "View pictures" ), this );
    a_pview->setIcon(picViewIcon);
    connect( a_pview, SIGNAL(triggered()), this, SLOT(viewPictures()) );
#endif

    a_settings = new QAction( QIcon( ":icon/settings" ) , tr("Settings..."), this );
    connect( a_settings, SIGNAL(triggered()), this, SLOT(doSettings()) );

    a_timer = new QAction( QIcon( ":icon/wait" ) , tr("Timer"), this );

    connect( a_timer, SIGNAL(triggered()), this, SLOT(takePhotoTimer()) );
    // get initial lense cover state

    d->a_cancelTimer = new  QAction(QIcon(), tr("Cancel Timer"), this);
    connect(d->a_cancelTimer, SIGNAL(triggered()), d->timer, SLOT(cancelTimer()));
    connect(d->a_cancelTimer, SIGNAL(triggered()), this, SLOT(updateTimerActions()));

    d->a_zoom = new QAction( QIcon( ), tr("Zoom"), this);
    connect(d->a_zoom, SIGNAL(triggered()), this, SLOT(showZoom()));

    a_settings->setVisible(hasCamera);
    a_timer->setVisible(hasCamera);
    d->a_zoom->setVisible(hasCamera);
    d->a_cancelTimer->setVisible(false);

    d->m_zoom  = basicControls->zoomSlider;
    d->m_zoom->installEventFilter(this);
    d->m_zoom->setSliderPosition(0);
    d->m_zoom->setOrientation(Qt::Horizontal);
    d->m_zoom->setRange(d->videocaptureview->minZoom(), d->videocaptureview->maxZoom());
    d->m_zoom->setValue(0);
    connect(d->m_zoom, SIGNAL(selected()), this, SLOT(exitZoomState()));
    connect(d->m_zoom, SIGNAL(valueChanged(int)), this, SLOT(zoomChanged(int)));

#ifdef USE_PICTUREFLOW
    d->a_browse = new QAction( QIcon(), tr("Quick browse snaps..."), this);
    connect(d->a_browse, SIGNAL(triggered()), this, SLOT(pictureviewBrowser()));
#endif

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    if(hasCamera)
    {
        if(hasVideo)
            contextMenu->addAction( a_vview );
#ifdef USE_PICTUREFLOW
        contextMenu->addAction( d->a_browse );
#else
        contextMenu->addAction( a_pview );
#endif

        //XXX removed because of issues with video surface where timer widget doesn't get brought above the surface
        contextMenu->addAction( a_timer );
        contextMenu->addAction( a_settings );
        contextMenu->addAction( d->a_zoom );
        contextMenu->addAction( d->a_cancelTimer );
    }

    connect(contextMenu, SIGNAL(aboutToHide()),
            this, SLOT(contextMenuAboutToHide()));
    connect(contextMenu, SIGNAL(aboutToShow()),
            this, SLOT(contextMenuAboutToShow()));

    lensCoverStateChanged();
    d->videocaptureview->startCapture();
}

void CameraMainWindow::exitZoomState()
{
    if (d->m_zoom->isVisible()){
        d->m_zoom->hide();
        d->m_zoom->setVisible(false);
        d->zoomActive = false;
        if(hasStill) {basicControls->photo->setEnabled(true); basicControls->photo->setFocus();}
        if(hasVideo) basicControls->photo->setEnabled(true);
    }
}

void CameraMainWindow::showZoom()
{
    if(d->zoomActive) return;
    d->zoomActive = true;
    d->m_zoom->show();
    d->m_zoom->setEditFocus(true);
    d->m_zoom->setVisible(true);
    if(hasStill)basicControls->photo->setEnabled(false);
    if(hasVideo)basicControls->video->setEnabled(false);

}

void CameraMainWindow::lensCoverStateChanged()
{
    bool state = d->videocaptureview->lensCoverState();
    if (!d->a_cancelTimer->isVisible()) {
        a_timer->setVisible( state );
        d->a_zoom->setVisible( state );
    }
}

void CameraMainWindow::noCamera()
{
    a_timer->setVisible(false);
    a_settings->setVisible(false);
    d->a_zoom->setVisible(false);
}

void CameraMainWindow::contextMenuAboutToShow()
{
    m_contextMenuActive = true;
}

void CameraMainWindow::contextMenuAboutToHide()
{
    m_contextMenuActive = false;
}

void CameraMainWindow::clamshellChanged()
{
    if (clamshellVsi->value().toBool()) {
        videoToScreen(QApplication::desktop()->primaryScreen());
    } else {
        videoToScreen(1);
    }
}

void CameraMainWindow::videoToScreen(int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop->screenNumber(d->videocaptureview) == screen)
        return;

    if (screen == desktop->primaryScreen()) {
        videoOnSecondary = false;
        d->videocaptureview->hide();
        //XXX: TODO
        //d->videocaptureview->setParent(basicControls->videocapturecontainer);
        //basicControls->videocapturecontainer->layout()->addWidget(d->videocaptureview);
        d->videocaptureview->show();
    } else {
        videoOnSecondary = true;
        d->videocaptureview->setFocusPolicy(Qt::NoFocus);
        d->videocaptureview->setParent(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        d->videocaptureview->setGeometry(desktop->screenGeometry(screen));
        d->videocaptureview->show();
    }
}

void CameraMainWindow::pictureviewBrowser()
{
#ifdef USE_PICTUREFLOW
    if(!d->browser)
        return;
    d->browser->show();
#endif
}

void CameraMainWindow::zoomChanged(int val)
{
    if( val > d->m_currzoom)
        d->videocaptureview->zoomIn();
    else if(val < d->m_currzoom)
        d->videocaptureview->zoomOut();
    d->m_currzoom = val;
}

void CameraMainWindow::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
}

void CameraMainWindow::focusOutEvent(QFocusEvent* e)
{
    Q_UNUSED(e)
}

bool CameraMainWindow::event(QEvent* e)
{
    if ( e->type() == QEvent::WindowActivate ) {
        // d->videocaptureview->startCapture()
    } else if ( e->type() == QEvent::WindowDeactivate ) {
        if(shutdown_camera)
            d->videocaptureview->endCapture();
    }
    return QMainWindow::event(e);
}

bool CameraMainWindow::eventFilter(QObject* o, QEvent* e)
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent*)e;

        if (ke->key() == CUSTOM_AUTOFOCUS_KEY ) {
            d->videocaptureview->autoFocus();
            return true;
        }

        if (o == this) {
            if(ke->key() == Qt::Key_Back) {
                shutdown_camera = true;
            }
        }
        if (!ke->isAutoRepeat()) {

            if (ke->key() == Qt::Key_1) {
                takePhoto();
                return true;
            }
            else if(ke->key() == Qt::Key_2) {
                if (hasVideo) {
                    toggleVideo();
                    return true;
                }
            }
        }

        if(hasCamera && d->videocaptureview->lensCoverState())
        {
            int key = ke->key();
            if(key == Qt::Key_4 )
            {
                if(d->videocaptureview->minZoom() < d->m_currzoom)
                {
                    d->m_currzoom--;
                    d->m_zoom->setValue(d->m_currzoom);

                    showZoom();
                }
                d->videocaptureview->zoomOut();

                return true;
            }
            if(key == Qt::Key_6 )
            {
                if(d->videocaptureview->maxZoom() > d->m_currzoom)
                {
                    d->m_currzoom++;
                    d->m_zoom->setValue(d->m_currzoom);

                    showZoom();
                }
                d->videocaptureview->zoomIn();
                return true;
            }
        }
    }
    else if (!m_contextMenuActive)
    {
        if (e->type() == QEvent::FocusIn)
        {
            if (o == basicControls->photo || o == basicControls->video || o == basicControls->zoomSlider) {
                QtopiaApplication::setPowerConstraint( QtopiaApplication::Disable );
            }
        }
        else if (e->type() == QEvent::FocusOut)
        {
            if (o == basicControls->photo || o == basicControls->video || o == basicControls->zoomSlider) {
                QtopiaApplication::setPowerConstraint( QtopiaApplication::Enable );
            }
        }
    }

    return QWidget::eventFilter(o,e);
}

void CameraMainWindow::viewPictures()
{
    QtopiaServiceRequest req("PhotoEdit","showCategory(QString)");
    req << camcat;
    req.send();
}

void CameraMainWindow::viewVideos()
{
    QMimeType m( QLatin1String( "video/mpeg" ));
    QContent a = m.application();
    if ( a.isValid() )
        a.execute();
}

void CameraMainWindow::doSettings()
{
    if (hasVideo)
        settings->video->show();
    else
        settings->video->hide();
    if (hasStill)
        settings->photo->show();
    else
        settings->photo->hide();

    settings->photo_size->setCurrentIndex( photoSizeMap.key(d->settings.photo()) );
    settings->video_size->setCurrentIndex( videoSizeMap.key(d->settings.video()) );
    settings->photo_quality->setValue( d->settings.photoquality() );
    settings->video_quality->setValue( d->settings.videoquality() );
    settings->video_framerate->setValue( d->settings.videoframerate() );
    settings->video_quality_n->setFixedWidth(fontMetrics().width("100"));
    settings->photo_quality_n->setFixedWidth(fontMetrics().width("100"));
    settings->location->setLocation(storagepath);

    if ( QtopiaApplication::execDialog(settingsDialog) )
        saveSettings();
}

void CameraMainWindow::saveSettings()
{
    // save settings
    d->settings.setStorageLocation( settings->location->documentPath() );
    d->settings.setVideoSize( videoSizeMap[settings->video_size->currentIndex()] );
    d->settings.setVideoQuality( settings->video_quality->value() );
    d->settings.setVideoFrameRate( settings->video_framerate->value() );
    d->settings.setPhotoSize( photoSizeMap[settings->photo_size->currentIndex()] );
    d->settings.setPhotoQuality( settings->photo_quality->value() );
    d->settings.save();

}

void CameraMainWindow::takePhotoTimer()
{
    PhotoTimerDialog* dialog = new PhotoTimerDialog( this );
    if ( dialog->exec() == QDialog::Accepted && d->videocaptureview->lensCoverState()) {
        d->m_inTimerMode = true;

        d->timer->start(dialog->timeout(),
                        dialog->number(),
                        dialog->interval());

#ifdef USE_PICTUREFLOW
        d->a_browse->setVisible(false);
#else
        a_pview->setVisible(false);
#endif
        d->a_cancelTimer->setVisible(true);
        a_timer->setVisible(false);
        a_settings->setVisible(false);
        d->a_zoom->setVisible(false);

        basicControls->photo->hide();
        basicControls->video->hide();

        d->toolBar->show();
        d->timerAction->setVisible(true);
    }

    delete dialog;
}

void CameraMainWindow::updateTimerActions()
{

#ifdef USE_PICTUREFLOW
    d->a_browse->setVisible(true);
#else
    a_pview->setVisible(true);
#endif
    a_timer->setVisible(true);
    a_settings->setVisible(true);
    d->a_zoom->setVisible(true);
    d->a_cancelTimer->setVisible(false);

    d->timerAction->setVisible(false);
    d->toolBar->hide();

    basicControls->photo->show();
    basicControls->video->show();

    d->m_inTimerMode = false;
}

void CameraMainWindow::takePhoto()
{
    if(!hasCamera || d->isTakingPhoto)
        return;
    d->isTakingPhoto = true;
    d->videocaptureview->setLive(0);
    d->videocaptureview->showBlankFrame(true);

    showWaitScreen();
    // Change the camera size and then wait for the camera to refocus.
    d->videocaptureview->takePhoto(d->settings.photo());
    hideWaitScreen();

}


void CameraMainWindow::imageReadyRaw(QImage& img)
{
    if ( snapRequest != 0 ) {

        // Rescale the image and pop it into a QDSData object
        QImage scaledimg = img.scaled( snap_max,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        QByteArray savedImageData;
        {
            QDataStream stream( &savedImageData, QIODevice::WriteOnly );
            stream << QPixmap::fromImage( scaledimg );
        }
        QDSData snappedImage( savedImageData, QMimeType( "image/x-qpixmap" ) );

        // Send response with the data
        snapRequest->respond( snappedImage );

        // Reset snap mode
        setSnapMode( false );
        delete snapRequest;
        snapRequest = 0;

        // Finished serving QDS request so close the application
        close();
        hideWaitScreen();
    } else {
        showWaitScreen("Saving image...");
        QContent content;
        QList<QString> c;
        content.setType("image/jpeg");
        content.setName(tr("Photo, %1","date").arg(QTimeString::localYMDHMS(QDateTime::currentDateTime(),QTimeString::Short)));
        content.setMedia( settings->location->documentPath() );

        c.append(camcat);
        content.setCategories(c);

        QIODevice*  contentDevice = content.open(QIODevice::WriteOnly);

        if (contentDevice != 0)
        {
            QImage  temp = img.convertToFormat(QImage::Format_RGB32);

            temp.save(contentDevice, "JPEG", d->settings.photoquality());

            contentDevice->close();

            content.commit();
            hideWaitScreen();

        } else {
            QString errorText = content.errorString();
            if (errorText.isEmpty())
                errorText = tr("Unknown error");

            QMessageBox::warning(0, tr("Error saving photo"), tr("Could not save photo: %1").arg(errorText));
        }
    }

    d->videocaptureview->showBlankFrame(false);
    d->videocaptureview->setLive(1);
    d->isTakingPhoto = false;
}

void CameraMainWindow::imageReady(QContent& content)
{
    QList<QString> c;

    content.setName(tr("Photo, %1","date").arg(QTimeString::localYMDHMS(QDateTime::currentDateTime(),QTimeString::Short)));
    content.setMedia( settings->location->documentPath() );

    c.append(camcat);
    content.setCategories(c);

    bool ret = content.commit();

   if(!ret) {
      QString errorText = content.errorString();
      if (errorText.isEmpty())
          errorText = tr("Unknown error");

      QMessageBox::warning(0, tr("Error saving photo"), tr("Could not save photo: %1").arg(errorText));
   }

    d->videocaptureview->showBlankFrame(false);
    d->videocaptureview->setLive(1);
    d->isTakingPhoto = false;
}

void CameraMainWindow::videoReadyForSaving(QContent& content)
{
    showWaitScreen("Saving video...");
    QList<QString> c;

//XXX set previously:  content.setType("image/jpeg");
    content.setName(tr("Video, %1","date").arg(QTimeString::localYMDHMS(QDateTime::currentDateTime(),QTimeString::Short)));
    content.setMedia( settings->location->documentPath() );

    c.append(camcat);
    content.setCategories(c);

    bool ret =  content.commit();
    hideWaitScreen();

    if(!ret) {
        QString errorText = content.errorString();
        if (errorText.isEmpty())
            errorText = tr("Unknown error");
        QMessageBox::warning(0, tr("Error saving video"), tr("Could not save video: %1").arg(errorText));
    }


}

void CameraMainWindow::setSnapMode(bool snapMode)
{
    if (snapMode) {

        basicControls->photo->show();
        d->hideVideo = true;

        // in snapshot mode, change back to "cancel taking a photo"
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
    } else {

        basicControls->photo->show();
        basicControls->video->show();

        // normal back button
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
    }
}

void CameraMainWindow::toggleVideo()
{
    d->videocaptureview->toggleVideo(!recording, d->settings.video());
    recording = !recording;
    basicControls->video->setText(recording ? tr("Stop") : tr("Video"));
    basicControls->photo->setEnabled(!recording);
}

void CameraMainWindow::getImage( const QDSActionRequest& request )
{
    if ( !request.isValid() ) {
        qWarning( "Camera: received invalid QDS request" );
        return;
    }

    if ( snapRequest != 0 ) {
        qWarning( "Camera: already processing another QDS request" );
        return;
    }

    // Read snap parameters from request
    QDataStream stream( request.requestData().toIODevice() );
    stream >> snap_max;

    // Set the camera for snap mode
    snapRequest = new QDSActionRequest( request );
    setSnapMode( true );
    showMaximized();
}


void CameraMainWindow::showWaitScreen(const QString &descr)
{
    if (!m_wait)
        m_wait = new QWaitWidget(this);

    if (m_wait) {
        if (!descr.isEmpty())
            m_wait->setText(descr);
        m_wait->show();
    }

}

void CameraMainWindow::hideWaitScreen()
{
    if (m_wait) {
        m_wait->hide();
        m_wait->deleteLater();
        m_wait = 0;
    }
}
/*!
    \service CameraService Camera
    \inpublicgroup QtEssentialsModule
    \brief The CameraService class provides the Camera service.

    The \i Camera service enables applications to access features of
    the Camera application.
*/

/*!
    \internal
*/
CameraService::~CameraService()
{
}

/*!
    Instructs the camera to take a photo of the dimensions provided in \a request.

    This slot corresponds to a QDS service with a request data type of
    "x-size/x-qsize" and a response data type of "image/x-qpixmap".

    This slot corresponds to the QCop service message
    \c{Camera::getImage(QDSActionRequest)}.
*/
void CameraService::getImage( const QDSActionRequest& request )
{
    parent->getImage( request );
}

/*!
  Instructs the camera to activate the shutter.  If the application isn't visible, this
  raise the camera and puts it into photo mode.  Otherwise it takes a photo.
*/

void CameraService::shutter()
{
    if (parent->isVisible() && parent->isActiveWindow())
        parent->takePhoto();
    else {
        parent->showMaximized();
        parent->raise();
        parent->activateWindow();
    }
}

