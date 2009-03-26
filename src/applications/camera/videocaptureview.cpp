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

#include "cameravideosurface.h"
#include "videocaptureview.h"
#include <math.h>

#include <QtopiaFeatures>

#include <QCameraDeviceLoader>
#include <QValueSpaceItem>
#include <QCameraDevice>
#include <qtopiaapplication.h>
#include <QContentSet>
#include <QSoftMenuBar>
#include <QTimer>
#include <qimage.h>
#include <qpainter.h>
#include <qevent.h>
#include <QRectF>
#include <QDebug>
#include <QLayout>
#include <QImageReader>
#include <QVideoFrame>

#include <QWSEmbedWidget>


using namespace QtopiaVideo;

class InformationWidget : public QWidget
{
    Q_OBJECT;
public:
    InformationWidget(QWidget *parent = 0);
    ~InformationWidget();

    void setMessage(QString txt);
protected:
    void paintEvent(QPaintEvent* paintEvent);
    void moveEvent(QMoveEvent* e);
    void resizeEvent(QResizeEvent* e);

private:
    QString m_message;
};

InformationWidget::InformationWidget(QWidget *parent):
    QWidget(parent),
    m_message(QString())
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    // Optimize paint event
    setAttribute(Qt::WA_NoSystemBackground);

    QPalette  pal(palette());
    pal.setBrush(QPalette::Window, Qt::black);
    setPalette(pal);
}

InformationWidget::~InformationWidget()
{}

void InformationWidget::setMessage(QString txt)
{
    m_message = txt;
    update();
}

void InformationWidget::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    update();
}

void InformationWidget::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
    update();
}

void InformationWidget::paintEvent(QPaintEvent* paintEvent)
{

    QPainter  painter(this);
    QPoint   brushOrigin = painter.brushOrigin();
    // Paint window background
    painter.setBrushOrigin(-mapToGlobal(QPoint(0, 0)));
    painter.fillRect(paintEvent->rect(), window()->palette().brush(QPalette::Window));
    painter.setBrushOrigin(brushOrigin);

    painter.drawText(paintEvent->rect(), Qt::AlignCenter | Qt::TextWordWrap, m_message);

}

class ViewPrivate : public QObject
{
    Q_OBJECT;
public:
    ViewPrivate(QObject *parent = 0) :
        QObject(parent),
        m_surface(0),
        canEmbedWidget(false),
        m_lastFrame(0),
        hidden(false),
        embedded(0),
        captureStarted(false),
        embeddedActive(false),
        cameraRunning(false)
    {}

    ~ViewPrivate()  { }

    CameraVideoSurface *m_surface;

    bool canEmbedWidget;
    QVideoFrame* m_lastFrame;


    QValueSpaceItem *m_rotation;
    int m_currentRotation;
    int m_defaultRotation;
    QValueSpaceItem *m_lensCover;
    bool m_lensIsOpen ;
    bool hidden;
    QWSEmbedWidget* embedded;
    InformationWidget *messageWidget;
    QRect savedGeometry;
    bool captureStarted;
    bool embeddedActive;
    bool cameraRunning;
    QTimer m_lastFrameTimer;
};

int VideoCaptureView::minZoom() const {  return (m_state) ? m_state->minZoom() : 0; }
int VideoCaptureView::maxZoom() const {  return (m_state) ? m_state->maxZoom() : 0; }

bool VideoCaptureView::hasZoom() const
{
    return (m_state) ? ((m_state->maxZoom() - m_state->minZoom()) > 0) : false;
}

void VideoCaptureView::zoomIn()
{
    if(m_state)
        m_state->zoomIn();
}

void VideoCaptureView::zoomOut()
{
    if(m_state)
        m_state->zoomOut();
}


VideoCaptureView::VideoCaptureView(QWidget *parent):
    QFrame(parent),
    m_hasCamera(true),
    d(new ViewPrivate)
{
    m_still = false;

    d->messageWidget = new InformationWidget(this);
    QVBoxLayout * layout = new QVBoxLayout (this);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(d->messageWidget);

    setLayout(layout);

    d->m_lensCover = new QValueSpaceItem("/Hardware/Devices/LensCover",this);
    d->m_lensIsOpen  = d->m_lensCover->value("Open",true).toBool();
    connect(d->m_lensCover, SIGNAL(contentsChanged()), this, SLOT(lensCoverStateChanged()));

    d->messageWidget->setMessage( d->m_lensIsOpen ? tr("Initializing") : tr("Lens Cover Closed"));

    d->m_rotation = new QValueSpaceItem("/UI/Rotation", this);

    d->m_currentRotation = d->m_rotation->value("Current",0).toInt();
    d->m_defaultRotation = d->m_rotation->value("Default",0).toInt();

    connect(d->m_rotation, SIGNAL(contentsChanged()), this, SLOT(rotationChanged()));

    QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);

    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    setSizePolicy(sizePolicy);
    setLineWidth(5);

    connect(&d->m_lastFrameTimer, SIGNAL(timeout()), this, SLOT(renderBlankFrame()));
}

VideoCaptureView::~VideoCaptureView()
{
    delete d;
}

void VideoCaptureView::rotationChanged()
{
    d->m_currentRotation = d->m_rotation->value("Current",0).toInt();
    d->m_surface->setRotation( d->m_currentRotation );
}

void VideoCaptureView::lensCoverStateChanged()
{
    d->m_lensIsOpen = d->m_lensCover->value("Open",true).toBool();

    if (!d->m_lensIsOpen)
        d->messageWidget->setMessage(tr("Lens Cover Closed"));
    switchToEmbeddedWidget( d->m_lensIsOpen );

    emit lensCoverChanged();
}

bool VideoCaptureView::lensCoverState()
{
    return d->m_lensIsOpen;
}

void VideoCaptureView::autoFocus()
{
    if(m_hasCamera && m_state) {
        m_state->autoFocus();
    }
}

void VideoCaptureView::toggleVideo(bool recording, QSize resolution)
{
    if (m_state && recording)
        m_state->startVideoCapture(resolution);
    else
        m_state->endVideoCapture();
}

QSize VideoCaptureView::stillDefaultSize()
{
    if(m_state)
        return m_state->defaultStillSize();
    else
        return QSize(-1,-1);
}

QSize VideoCaptureView::videoDefaultSize()
{
    if(m_state)
        return m_state->defaultVideoSize();
    else
        return QSize(-1,-1);
}

QSize VideoCaptureView::previewDefaultSize()
{
    return m_state->defaultPreviewSize();
}

void VideoCaptureView::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
}

void VideoCaptureView::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
}

void VideoCaptureView::mousePressEvent(QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent);
}

void VideoCaptureView::setLive(int state)
{
    if(!m_hasCamera)
        return;

    if(state)  {
        connect(m_state, SIGNAL(previewFrameReady(QVideoFrame const& )), this, SLOT(displayPreviewFrame(QVideoFrame const&)));
    }
    else {
        disconnect(m_state, SIGNAL(previewFrameReady(QVideoFrame const& )), this, SLOT(displayPreviewFrame(QVideoFrame const&)));
    }
}

bool VideoCaptureView::available() const
{
    return m_hasCamera;
}

void VideoCaptureView::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
}

void VideoCaptureView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
}

void VideoCaptureView::showBlankFrame(bool b)
{
    if (b)
        d->m_lastFrameTimer.start(0);
    else
        d->m_lastFrameTimer.stop();
}

void VideoCaptureView::renderBlankFrame()
{
    d->m_surface->surface()->renderFrame(QVideoFrame());
}

void VideoCaptureView::displayPreviewFrame(QVideoFrame const &frame)
{
    if (d->embeddedActive && d->cameraRunning) {
        d->m_surface->surface()->renderFrame(frame);
    }
}

void VideoCaptureView::cameraError(QtopiaCamera::CameraError error, QString errorString)
{
    switch(error)
    {
        case QtopiaCamera::FatalError:
        {
            qWarning()<<"Camera fatal error: " << errorString;
            m_hasCamera = false;
            d->messageWidget->setMessage(tr("No Camera"));
            switchToEmbeddedWidget(false);
            emit noCamera();
        }
        case QtopiaCamera::Warning:
        {
            qWarning()<<"Camera warning: " << errorString;
            m_hasCamera = false;
            d->messageWidget->setMessage(tr("No Camera"));
            switchToEmbeddedWidget(false);
            emit noCamera();
        }
        default:
            break;
    }
}

/*
    Switch between Video Surface Widget for preview data and basic widget for
    informational messages
*/
void VideoCaptureView::switchToEmbeddedWidget(bool s)
{
    if (s) {
        if (d->embedded && !d->embeddedActive )  {
            d->messageWidget->hide();
            layout()->removeWidget(d->messageWidget);
            layout()->addWidget(d->embedded);
            d->embedded->show();
            d->embedded->raise();
            d->embeddedActive = true;
            setLive(1);
        }
    } else {
        if (d->embedded && d->embeddedActive) {
            d->embedded->hide();
            layout()->removeWidget(d->embedded);
            layout()->addWidget(d->messageWidget);
            d->messageWidget->show();
            d->messageWidget->raise();
            d->embeddedActive = false;
            setLive(0);
        }
    }
}

bool VideoCaptureView::hasStill()
{
    if(m_state)
        return m_state->hasStill();
    return false;
}

bool VideoCaptureView::hasVideo()
{
    if(m_state)
        return  m_state->hasVideo();
    return false;
}

bool VideoCaptureView::initializeCamera()
{
    static int once = 1;

    if (!once)  return m_hasCamera;
    once = 0;

    m_loader = QCameraDeviceLoader::instance();

    if (m_hasCamera = m_loader->cameraDevicesAvailable())
    {
        m_device = 0;

        if ( (m_device = m_loader->deviceWithOrientation(QCameraDevice::BackFacing)) != 0)
        {}
        else if ((m_device = m_loader->deviceWithOrientation(QCameraDevice::FrontFacing)) != 0)
        {}
        else if ((m_device = m_loader->deviceWithOrientation(QCameraDevice::Changing)) != 0)
        {}

        if (m_device == 0) {
            d->messageWidget->setMessage( tr("No Camera"));
            return (m_hasCamera = false);
        }

        connect(m_device, SIGNAL(cameraError(QtopiaCamera::CameraError, QString)), this, SLOT(cameraError(QtopiaCamera::CameraError, QString)));

        m_state = new CameraStateProcessor(m_device, this);

        // check for valid video surface and camera device
        if (!m_state->initialize (d->m_currentRotation)) {
            d->messageWidget->setMessage( tr("No Camera"));
            return (m_hasCamera=false);
        }

        d->m_surface = m_state->surface();

        if (m_state->hasVideo())
            connect(m_state, SIGNAL(videoDone(QContent&)), this, SIGNAL(videoReadyForSaving(QContent&)));

        if (m_state->hasStill()) {
            connect(m_state, SIGNAL(stillDone(QContent&)), this, SIGNAL(imageReadyForSaving(QContent&)));
            connect(m_state, SIGNAL(stillDoneRaw(QImage&)), this, SIGNAL(imageReadyRaw(QImage&)));
        }
     }

    if (!m_hasCamera)
        d->messageWidget->setMessage(tr("No Camera"));
    return m_hasCamera;
}

void VideoCaptureView::startCapture()
{
    if(m_state) {
        if (m_hasCamera) {
            d->embedded = static_cast<QWSEmbedWidget*>(d->m_surface->surface()->videoWidget());
            if (d->embedded) {
                d->embedded->setFocusPolicy(Qt::NoFocus);
                if (d->m_lensIsOpen)
                switchToEmbeddedWidget(true);
            }
            m_state->start();
            d->cameraRunning = true;
        }
    }
}

void VideoCaptureView::endCapture()
{
    if(m_state) {
        m_state->stop();
        d->cameraRunning = false;
    }
}

QList<QSize> VideoCaptureView::stillSizes()
{
    if(m_state)
        return m_state->stillSizes();
    return  QList<QSize>();

}

QList<QSize> VideoCaptureView::previewSizes()
{
    if(m_state)
        return m_state->previewSizes();
    return QList<QSize>();
}

QList<QSize> VideoCaptureView::videoSizes()
{
    if(m_state)
        return m_state->videoSizes();
    return QList<QSize>();
}


void VideoCaptureView::takePhoto(QSize resolution,int count)
{
    if(m_state && d->m_lensIsOpen)
        m_state->takeStillImage(resolution, count);
}


#include "videocaptureview.moc"
