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

#include "basicmedia.h"

#include <QContent>
#include <QMediaControlNotifier>
#include <QMediaVideoControl>
#include <QVBoxLayout>
#include <QApplication>
#include <QMediaControl>
#include <QResizeEvent>
#include <QSize>
#include <QDebug>

/*
   m_state 0  constructed
   m_state 1  set filename
   m_state 2  mediaControlValid
   m_state 3  mediaVideoControlValid
   m_state 4  stopped
   */

BasicMedia::BasicMedia(QWidget* parent)
    : QWidget(parent), m_mediaContent(0), m_control(0), m_video(0), m_state(0), videoWidget(0)
{
    layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    // First create a container for all player objects
    context = new QMediaContentContext(this);

    // watches a media content object for availability of a given media control
    QMediaControlNotifier* notifier = new QMediaControlNotifier(QMediaControl::name(), this);
    connect(notifier, SIGNAL(valid()), this, SLOT(mediaControlValid()));
    context->addObject(notifier);

    // watches a media content object for availability of a given media control
    QMediaControlNotifier* video = new QMediaControlNotifier(QMediaVideoControl::name(), this);
    connect(video, SIGNAL(valid()), this, SLOT(mediaVideoControlValid()));
    context->addObject(video);
    volume=100;
}

void BasicMedia::keyReleaseEvent( QKeyEvent *ke )
{
    ke->ignore();
}

void BasicMedia::setFilename(QString f)
{
    // Step 1: must set a file to play
    m_state = 1;
    vfile = f;
}

void BasicMedia::mediaVideoControlValid()
{
    if(m_state == 2) {
        m_state = 3;

        delete m_video;
        delete videoWidget;

        // create video widget
        m_video = new QMediaVideoControl(m_mediaContent);

        videoWidget = m_video->createVideoWidget(this);
        if (!videoWidget) {
            qWarning("Failed to create video widget");
            return;
        }
        videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // display the video widget on the screen
        layout->addWidget(videoWidget);
    } else {
        qWarning("(%d) ERROR BasicMedia::mediaVideoControlValid, should appear after mediaControlValid???",m_state);
    }
}

void BasicMedia::mediaControlValid()
{
    if(m_state == 1) {
        m_state = 2;

        delete m_control;

        m_control = new QMediaControl(m_mediaContent);
        m_control->start();
    } else {
        qWarning("(%d) ERROR BasicMedia::mediaControlValid, must call setFilename() before start()!!!",m_state);
    }
}

BasicMedia::~BasicMedia()
{
    delete m_mediaContent;
    delete m_control;
}

void BasicMedia::stop()
{
    if(m_state == 3) {
        //Normal stop request
        m_control->stop();
        m_state=1;
    } else if(m_control) {
        //Video start play failed???
        m_control->stop();
        m_state=1;
    }
    if(m_state > 1) {
        if(m_video!=NULL) {
            delete m_video;
            delete videoWidget;
            delete m_mediaContent;

            m_video        = NULL;
            videoWidget    = NULL;
            m_mediaContent = NULL;
        }
    }
}

void BasicMedia::start()
{
    if(m_state == 1) {
        QContent content(vfile);
        if (!content.isValid()) {
            qWarning() << "Failed to load" << vfile;
            return;
        }
        m_mediaContent = new QMediaContent(content);
        context->setMediaContent(m_mediaContent);
    } else {
        qWarning("(%d) BasicMedia::start() must stop() and setFilename() before calling start!!!",m_state);
    }
}

void BasicMedia::volumeup()
{
    if(volume<90) {
        volume=volume+10;
        m_control->setVolume(volume);
    }
}

void BasicMedia::volumedown()
{
    if(volume>10) {
        volume=volume-10;
        m_control->setVolume(volume);
    }
}

