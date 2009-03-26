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
#include "conferencewidget.h"

#include "callcontrols.h"

#include <QAction>
#include <QBoxLayout>
#include <QCameraDevice>
#include <QCameraDeviceLoader>
#include <QHostAddress>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QSoftMenuBar>
#include <QtopiaApplication>
#include <QVideoSurface>

ConferenceWidget::ConferenceWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_camera(0)
    , m_videoConnections(0)
    , m_depressedKey(-1)
{
    if (QCameraDeviceLoader::instance()->cameraDevicesAvailable()) {
        QCameraDevice *device = QCameraDeviceLoader::instance()->allAvailableCameras().first();

        m_camera = device->previewCapture();
    }

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addLayout(createCallLayout(5000, 5002));
    layout->addLayout(createCallLayout(5004, 5006));
    setLayout(layout);

    if (m_camera) {
        QMenu *menu = QSoftMenuBar::menuFor(this);

        QAction *cameraAction = menu->addAction(tr("Camera"), this, SLOT(setCameraOn(bool)));
        cameraAction->setCheckable(true);
    }
}

QBoxLayout *ConferenceWidget::createCallLayout(int audioPort, int videoPort)
{
    QMediaRtpAudioStream *audio = m_session.addAudioStream(QMediaRtpStream::SendReceive);

    if (audio) {
        connect(this, SIGNAL(startTelephonyEvent(int,int)),
                audio, SLOT(startTelephonyEvent(int,int)));
        connect(this, SIGNAL(stopTelephonyEvent()),
                audio, SLOT(stopTelephonyEvent()));
    }

    QMediaRtpVideoStream *video = 0;

    QWidget *videoOutput = 0;

    if (m_camera) {
        video = m_session.addVideoStream(QMediaRtpStream::SendReceive);

        video->setCamera(m_camera);

        connect(video, SIGNAL(inboundStateChanged(QMediaRtpStream::State)),
                this, SLOT(videoInStateChanged(QMediaRtpStream::State)));

        videoOutput = video->videoSurface()->videoWidget();
    } else {
        videoOutput = new QLabel(tr("No Video"));
    }
    videoOutput->setMinimumSize(120, 120);

    CallControls *audioControls = new CallControls(audio);
    audioControls->setInboundLabel(tr("Ai", "Audio In"));
    audioControls->setInboundHost(QHostAddress::Any, audioPort);
    audioControls->setOutboundLabel(tr("Ao", "Audio Out"));
    audioControls->setOutboundHost(QHostAddress::LocalHost, audioPort);
    audioControls->setOutboundPayloads(m_session.supportedOutboundPayloads(QMediaRtpStream::Audio));

    CallControls *videoControls = new CallControls(video);
    videoControls->setInboundLabel(tr("Vi", "Video In"));
    videoControls->setInboundHost(QHostAddress::Any, videoPort);
    videoControls->setOutboundLabel(tr("Vo", "Video Out"));
    videoControls->setOutboundHost(QHostAddress::LocalHost, videoPort);
    videoControls->setOutboundPayloads(m_session.supportedOutboundPayloads(QMediaRtpStream::Video));

    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(videoOutput);
    layout->addWidget(audioControls);
    layout->addWidget(videoControls);

    return layout;
}

void ConferenceWidget::keyPressEvent(QKeyEvent *event)
{
    int tone = 0;

    switch (event->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        tone = event->key() - Qt::Key_0;
        break;
    case Qt::Key_NumberSign:
        tone = 10;
        break;
    case Qt::Key_Asterisk:
        tone = 11;
        break;
    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
        tone = event->key() - Qt::Key_A + 12;
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    event->accept();

    if (!event->isAutoRepeat()) {
        if (m_depressedKey != -1)
            emit stopTelephonyEvent();

        m_depressedKey = event->key();

        emit startTelephonyEvent(tone, 0);
    }
}

void ConferenceWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
    case Qt::Key_NumberSign:
    case Qt::Key_Asterisk:
    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
        event->accept();

        if (!event->isAutoRepeat() && event->key() == m_depressedKey) {
            m_depressedKey = -1;

            emit stopTelephonyEvent();
        }
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

void ConferenceWidget::setCameraOn(bool on)
{
    if (on) {
        QtopiaCamera::FormatResolutionMap formats = m_camera->formats();

        uint format = formats.keys().first();

        m_camera->start(format, formats.value(format).first(), 30);
    } else {
        m_camera->stop();
    }
}

void ConferenceWidget::videoInStateChanged(QMediaRtpStream::State state)
{
    switch (state) {
    case QMediaRtpStream::Connecting:
        if (m_videoConnections++ == 0)
            QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
        break;
    case QMediaRtpStream::Disconnected:
        if (--m_videoConnections == 0)
            QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
        break;
    default:
        break;
    }
}
