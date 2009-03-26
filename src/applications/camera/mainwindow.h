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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qpixmap.h>
#include "ui_camerabase.h"
#include "ui_camerasettings.h"
#include <qmainwindow.h>
#include <qcontent.h>
#include <QDSActionRequest>
#include <QtopiaAbstractService>
#include <QContentSet>
#include <QSettings>
#include <QFileSystem>
#include <QFocusEvent>
#include <QCameraDevice>

class CameraSettings;
class QAction;
class QTimer;
class QValueSpaceItem;
class QWaitWidget;
class  MainWindowPrivate;

class CameraMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    CameraMainWindow( QWidget *parent=0, Qt::WFlags fl=0 );
    ~CameraMainWindow();

public slots:
    void takePhoto();
    void toggleVideo();
    void getImage( const QDSActionRequest& request );
private slots:
    void viewPictures();
    void viewVideos();
    void doSettings();
    void takePhotoTimer();
    void clamshellChanged();
    void contextMenuAboutToShow();
    void contextMenuAboutToHide();
    void init();
    void pictureviewBrowser();
    void zoomChanged(int);
    void exitZoomState();
    void showZoom();
    void updateTimerActions();
    void lensCoverStateChanged();

    void noCamera();

    void imageReady(QContent&);
    void imageReadyRaw(QImage&);
    void videoReadyForSaving(QContent&);

private:

    void doInitialCameraSetup();
    void setupCameraCategory();
    void launchService();
    void setupSnapshotViewer();
    void setupSettingsDialog();

    bool event(QEvent* e);
    void resizeEvent(QResizeEvent*);
    void focusOutEvent(QFocusEvent*);

    bool eventFilter(QObject*, QEvent*);
    QString nextFileName();


    // Settings
    void saveSettings();
    Ui::CameraSettings *settings;
    QDialog *settingsDialog;
    QString storagepath;
    QString media;

    QSize photoSize;
    QSize videoSize;
    //int videoFramerate;
    QMap<int, QSize> photoSizeMap;
    QMap<int, QSize> videoSizeMap;

    // Snap
    QSize snap_max;
    void setSnapMode( bool snapMode );

    Ui::CameraBase *basicControls;

    int namehint;
    QAction *a_pview, *a_vview, *a_timer, *a_settings;
    QAction *a_th_edit, *a_th_del, *a_th_add;
    QAction *a_send;
    QAction *a_mode;
    QCameraDevice::CaptureMode mode;

    QList<QSize> photo_sizes;
    QList<QSize> video_sizes;
    QList<QSize> preview_sizes;

    // QAction a_stillCapture;
    // QAction a_videoCapture;

    QString picfile;

    QDSActionRequest* snapRequest;

    bool recording;

    void preview();

    void videoToScreen(int screen);
    bool videoOnSecondary;
    QValueSpaceItem *clamshellVsi;

    QContentSet m_photoContentSet;
    QContentSetModel *m_photoModel;
    QString camcat;
    bool    m_contextMenuActive;
    QWaitWidget *m_wait;
    bool m_iswaiting;
    void showWaitScreen(const QString& s = "");
    void hideWaitScreen();

    bool hasCamera;
    bool hasVideo;
    bool hasStill;

    bool shutdown_camera;
    MainWindowPrivate *d;
};

class CameraService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class CameraMainWindow;
private:
    CameraService( CameraMainWindow *parent )
        : QtopiaAbstractService( "Camera", parent )
        { this->parent = parent; publishAll(); }

public:
    ~CameraService();

public slots:
    void getImage( const QDSActionRequest& request );
    void shutter();

private:
    CameraMainWindow *parent;
};


class CameraSettings
{
public:
    CameraSettings() {
       m_settings = new QSettings("Trolltech", "Camera");
    }

    ~CameraSettings() {
        delete m_settings;
    }

    QSize video() const {
        return m_video;
    }

    int videoframerate() const {
        return m_videoFPS;
    }

    QSize photo() const {
        return m_photo;
    }

    int photoquality() const {
        return m_photoQuality;
    }

    int videoquality() const {
        return m_videoQuality;
    }

    QString location() const {
        return m_storageLocation;
    }

    void setVideoSize(QSize v) {
        m_video = v;
    }

    void setPhotoSize(QSize p) {
        m_photo = p;
    }

    void setPhotoQuality(int q) {
        m_photoQuality = q;
    }

    void setVideoQuality(int q) {
        m_videoQuality = q;
    }

    void setVideoFrameRate(int r) {
        m_videoFPS = r;
    }

    void setStorageLocation(QString l) {
        m_storageLocation = l;
    }



    void load(QString dpresolution, QString dvresolution, int  dpquality,
              int dvquality, int dvframerate)
    {
        QString res;
        QStringList r;
        m_settings->sync();
        m_settings->beginGroup("General");
        m_storageLocation = m_settings->value("location", QFileSystem::documentsFileSystem().documentsPath()).toString();
        m_settings->endGroup();
        //photo
        m_settings->beginGroup("Photo");
        res = m_settings->value("resolution", dpresolution).toString();
        r = res.split("x", QString::SkipEmptyParts, Qt::CaseInsensitive);
        m_photo = QSize( (!r[0].isEmpty()) ? r[0].toInt() : -1, (!r[1].isEmpty()) ? r[1].toInt() : -1);
        m_photoQuality = m_settings->value("quality", dpquality).toInt();
        m_settings->endGroup();

        r.clear();
        //Video
        m_settings->beginGroup("Video");
        res = m_settings->value("resolution", dvresolution).toString();
        r = res.split("x",QString::SkipEmptyParts, Qt::CaseInsensitive);
        m_video = QSize( (!r[0].isEmpty()) ? r[0].toInt() : -1, (!r[1].isEmpty()) ? r[1].toInt() : -1);
        m_videoQuality = m_settings->value("quality", dvquality).toInt();
        m_videoFPS = m_settings->value("framerate", dvframerate).toInt();
        m_settings->endGroup();

    }

    void save()
    {
        QString resX,resY;
        m_settings->beginGroup("General");
        m_settings->setValue("location", m_storageLocation);
        m_settings->endGroup();

        //Photo
        m_settings->beginGroup("Photo");
        resX.setNum(m_photo.width());
        resY.setNum(m_photo.height());
        m_settings->setValue("resolution", resX+"x"+resY);
        m_settings->setValue("quality", m_photoQuality);
        m_settings->endGroup() ;

        //Video
        m_settings->beginGroup("Video");
        resX.setNum(m_video.width());
        resY.setNum(m_video.height());
        m_settings->setValue("resolution", resX+"x"+resY);
        m_settings->setValue("quality", m_videoQuality);
        m_settings->setValue("framerate", m_videoFPS);
        m_settings->endGroup();
    }


private:
    QSettings* m_settings;
    QSize m_photo;
    QSize m_video;
    QString m_storageLocation;
    int m_photoQuality;    //0..100
    int m_videoQuality;
    int m_videoFPS;

};

#endif

