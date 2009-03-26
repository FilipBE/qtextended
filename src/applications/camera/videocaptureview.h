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

#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QFrame>
#include <qimage.h>
#include <qlist.h>
#include <qtabwidget.h>
#include "camerastateprocessor.h"
#include <QCameraDevice>
#include <QCameraDeviceLoader>


class QCameraControl;
class CameraStateProcessor;
class ViewPrivate;
class QVideoFrame;

class VideoCaptureView : public QFrame
{
    Q_OBJECT

public:
    VideoCaptureView(QWidget *parent = 0);
    ~VideoCaptureView();


    bool available() const;

    void startCapture();
    void endCapture();

    void setLive(int);
    void showBlankFrame(bool);

    QList<QSize> stillSizes();
    QList<QSize> previewSizes();
    QList<QSize> videoSizes();

    QSize stillDefaultSize();
    QSize videoDefaultSize();
    QSize previewDefaultSize();


    int minZoom() const;
    int maxZoom() const;
    bool hasZoom() const;

    bool initializeCamera();

    void toggleVideo(bool recording, QSize resolution);

    void takePhoto(QSize resolution, int count = 1);

    bool hasVideo();
    bool hasStill();

    bool lensCoverState();

    void autoFocus();
protected:
    void moveEvent(QMoveEvent* moveEvent);
    void resizeEvent(QResizeEvent* resizeEvent);
    void mousePressEvent(QMouseEvent* mouseEvent);
    void hideEvent(QHideEvent*);
    void showEvent(QShowEvent*);

public slots:
    void cameraError(QtopiaCamera::CameraError, QString errorString);
    void lensCoverStateChanged();
    void rotationChanged();
    void zoomIn();
    void zoomOut();

private slots:
    void displayPreviewFrame(QVideoFrame const&);
    void renderBlankFrame();

signals:
    void cameraUnavailable();
    void cameraReady();
    void imageReadyForSaving(QContent&);
    void imageReadyRaw(QImage&);
    void videoReadyForSaving(QContent&);
    void lensCoverChanged();
    void noCamera();

private:
    void switchToEmbeddedWidget(bool);

    bool m_hasCamera;

    QCameraDevice *m_device;
    QCameraDeviceLoader *m_loader;
    CameraStateProcessor *m_state;
    bool m_still;

    ViewPrivate *d;
};


#endif

