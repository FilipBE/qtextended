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

#include "voicemailviewer.h"

#include <qtopialog.h>
#include <private/homewidgets_p.h>

#include <QContactModel>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QMailMessage>
#include <QMediaContent>
#include <QMediaControl>
#include <QMediaVideoControl>
#include <QMenu>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QtopiaApplication>

VoicemailViewer::VoicemailViewer(QWidget* parent)
    : QMailViewerInterface(parent),
      mainWidget(new QWidget(parent)),
      fromButton(new HomeContactButton(tr("From:"), sizer)),
      replyButton(new HomeActionButton(tr("Return call"), QtopiaHome::Green)),
      forwardButton(new HomeActionButton(tr("Forward"), QtopiaHome::Green)),
      deleteButton(new HomeActionButton(tr("Delete"), QtopiaHome::Red)),
      backButton(new HomeActionButton(tr("Back"), mainWidget->palette().color(QPalette::Button), mainWidget->palette().color(QPalette::ButtonText))),
      playButton(new HomeActionButton()),
      contactLabel(new QLabel),
      slider(new QSlider),
      sliderLabel(new QLabel),
      videoPlaceholder(new QLabel),
      videoLayout(0),
      media(0),
      video(0),
      videoWidget(0),
      message(0)
{
    QSizePolicy expandingPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSizePolicy minimumPolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // TODO - configure the size/spacing/margins for all these controls...

    fromButton->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    connect(fromButton, SIGNAL(clicked()), this, SLOT(senderActivated()));

    replyButton->setSizePolicy(minimumPolicy);
    connect(replyButton, SIGNAL(clicked()), this, SIGNAL(replyToSender()));

    forwardButton->setSizePolicy(minimumPolicy);
    connect(forwardButton, SIGNAL(clicked()), this, SIGNAL(forwardMessage()));

    deleteButton->setSizePolicy(minimumPolicy);
    connect(deleteButton, SIGNAL(clicked()), this, SIGNAL(deleteMessage()));

    backButton->setSizePolicy(minimumPolicy);
    connect(backButton, SIGNAL(clicked()), this, SIGNAL(finished()));

    playButton->setText(tr("Play"));
    playButton->setSizePolicy(minimumPolicy);
    connect(playButton, SIGNAL(clicked()), this, SLOT(playback()));

    contactLabel->setSizePolicy(minimumPolicy);

    slider->setOrientation(Qt::Horizontal);
    slider->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    connect(slider, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(slider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));

    sliderLabel->setSizePolicy(minimumPolicy);

    videoPlaceholder = new QLabel(tr("Loading"));
    videoPlaceholder->setAlignment(Qt::AlignCenter);
    videoPlaceholder->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    videoPlaceholder->setSizePolicy(expandingPolicy);

    widget()->installEventFilter(this);
}

VoicemailViewer::~VoicemailViewer()
{
    clear();
}

QWidget* VoicemailViewer::widget() const
{
    return mainWidget;
}

static void extractAsteriskMedia(const QMailMessage& mail, QContent& recording)
{
    // Asterisk:
    if (mail.partCount() > 0) {
        // Part 2 is a WAV file containing the recorded message
        const QMailMessagePart &part = mail.partAt(1);
        QString path(part.writeBodyTo(Qtopia::tempDir()));
        if (!path.isEmpty()) {
            recording = QContent(path);

            if (part.hasBody()) {
                QMailMessageContentType type(part.contentType());

                if (recording.drmState() == QContent::Unprotected)
                    recording.setType(part.contentType().content());
            }

            recording.setName(part.displayName());
        }
    }
}

bool VoicemailViewer::extractMedia()
{
    if (content() == QMailMessage::VoicemailContent) {
        // NOTE: the following code works with voicemail email notifications from Asterisk:
        extractAsteriskMedia(*message, recording);
    } else if (content() == QMailMessage::VideomailContent) {
        // Videomail from our demonstration composer is formatted equivalently to asterisk voicemail
        extractAsteriskMedia(*message, recording);
    }

    if (!recording.isNull()) {
        recording.setRole(QContent::Data);
        recording.commit();
        return true;
    }

    return false;
}

bool VoicemailViewer::setMessage(const QMailMessage& mail)
{
    clear();

    message = &mail;
    layout();

    replyButton->setEnabled(false);

    QString timeStamp;
    QDateTime dateTime = message->date().toLocalTime();
    timeStamp.append(QTimeString::localHM(dateTime.time(), QTimeString::Medium));
    timeStamp.append(' ');
    timeStamp.append(QTimeString::localMD(dateTime.date(), QTimeString::Medium));

    contact = message->from().matchContact();
    if (!contact.uid().isNull()) {
        replyButton->setEnabled(true);

        contactLabel->setPixmap(contact.portrait());
        fromButton->setValues(contact, contact.label(), timeStamp);
    } else {
        fromButton->setValues(contact, message->from().displayName(), timeStamp);
    }

    if (extractMedia()) {
        // We have a recording to play back
        QMediaContent *content = new QMediaContent(recording);
        connect(content, SIGNAL(mediaError(QString)), this, SLOT(mediaError(QString)));
        connect(content, SIGNAL(controlAvailable(QString)), this, SLOT(controlAvailable(QString)));
    }

    return true;
}

void VoicemailViewer::clear()
{
    playButton->setEnabled(false);
    playButton->setText(tr("Play"));

    message = 0;
    contact = QContact();

    if (media) {
        if ((media->playerState() == QtopiaMedia::Playing) ||
            (media->playerState() == QtopiaMedia::Paused)) {
            media->stop();
        }

        if (video) {
            videoWidget->deleteLater();
            videoWidget = 0;

            video->deleteLater();
            video = 0;

            videoPlaceholder->show();
        }

        media->deleteLater();
        media = 0;
    }

    if (!recording.isNull()) {
        recording.removeFiles();
        recording = QContent();
    }
}

QMailMessage::ContentType VoicemailViewer::content() const
{
    return (message ? message->content() : QMailMessage::UnknownContent);
}

void VoicemailViewer::layout()
{
    if (QLayout *layout = mainWidget->layout())
        delete layout;

    if (content() == QMailMessage::VoicemailContent) {
        QVBoxLayout *vb1 = new QVBoxLayout;
        vb1->setSpacing(0);
        vb1->setContentsMargins(0, 0, 0, 0);
        vb1->addStretch();
        vb1->addWidget(replyButton);
        vb1->addStretch();
        vb1->addWidget(forwardButton);
        vb1->addStretch();
        vb1->addWidget(deleteButton);
        vb1->addStretch();
        
        QVBoxLayout *vb2 = new QVBoxLayout;
        vb2->setSpacing(0);
        vb2->setContentsMargins(0, 0, 0, 0);
        vb2->addStretch();
        vb2->addWidget(contactLabel);
        vb2->addStretch();
        vb2->addWidget(playButton);
        vb2->addStretch();
        
        QHBoxLayout *hb1 = new QHBoxLayout;
        hb1->setSpacing(0);
        hb1->setContentsMargins(0, 0, 0, 0);
        hb1->addWidget(fromButton);
        hb1->addWidget(backButton);

        QHBoxLayout *hb2 = new QHBoxLayout;
        hb2->setSpacing(0);
        hb2->setContentsMargins(0, 0, 0, 0);
        hb2->addStretch();
        hb2->addLayout(vb1);
        hb2->addStretch();
        hb2->addLayout(vb2);
        hb2->addStretch();

        QHBoxLayout *hb3 = new QHBoxLayout;
        hb3->setSpacing(0);
        hb3->setContentsMargins(0, 0, 0, 0);
        hb3->addWidget(slider);
        hb3->addWidget(sliderLabel);

        QVBoxLayout *vb3 = new QVBoxLayout(mainWidget);
        vb3->setSpacing(0);
        vb3->setContentsMargins(0, 0, 0, 0);
        vb3->addLayout(hb1);
        vb3->addLayout(hb2);
        vb3->addLayout(hb3);
    } else if (content() == QMailMessage::VideomailContent) {
        QVBoxLayout *vb1 = new QVBoxLayout;
        vb1->setSpacing(0);
        vb1->setContentsMargins(0, 0, 0, 0);
        vb1->addStretch();
        vb1->addWidget(playButton);
        vb1->addStretch();
        vb1->addWidget(replyButton);
        vb1->addStretch();
        vb1->addWidget(forwardButton);
        vb1->addStretch();
        vb1->addWidget(deleteButton);
        vb1->addStretch();
        
        QHBoxLayout *hb1 = new QHBoxLayout;
        hb1->setSpacing(0);
        hb1->setContentsMargins(0, 0, 0, 0);
        hb1->addWidget(slider);
        hb1->addWidget(sliderLabel);

        videoLayout = new QVBoxLayout;
        videoLayout->setSpacing(0);
        videoLayout->setContentsMargins(0, 0, 0, 0);
        videoLayout->addWidget(videoPlaceholder);
        videoLayout->addLayout(hb1);
        
        QHBoxLayout *hb2 = new QHBoxLayout;
        hb2->setSpacing(0);
        hb2->setContentsMargins(0, 0, 0, 0);
        hb2->addWidget(fromButton);
        hb2->addWidget(backButton);

        QHBoxLayout *hb3 = new QHBoxLayout;
        hb3->setSpacing(0);
        hb3->setContentsMargins(0, 0, 0, 0);
        hb3->addLayout(vb1);
        hb3->addLayout(videoLayout);

        QVBoxLayout *vb3 = new QVBoxLayout(mainWidget);
        vb3->setSpacing(0);
        vb3->setContentsMargins(0, 0, 0, 0);
        vb3->addLayout(hb2);
        vb3->addLayout(hb3);
    }
}

void VoicemailViewer::senderActivated()
{
    if (!contact.uid().isNull()) {
        emit contactDetails(contact);
    } else {
        emit saveSender();
    }
}

void VoicemailViewer::playback()
{
    if (media) {
        if (media->playerState() == QtopiaMedia::Playing) {
            playButton->setText(tr("Play"));
            media->pause();
        } else {
            playButton->setText(tr("Pause"));
            media->start();
        }
    }
}

void VoicemailViewer::mediaError(const QString &error)
{
    // TODO: message box to report error
    Q_UNUSED(error);
}

void VoicemailViewer::controlAvailable(const QString &id)
{
    if (id == QMediaControl::name() && !media) {
        media = new QMediaControl(static_cast<QMediaContent*>(sender()));

        // Should be overriden by real values shortly...
        mediaLengthChanged(1);
        mediaPositionChanged(0);

        connect(media, SIGNAL(valid()), this, SLOT(controlValid()));
        connect(media, SIGNAL(playerStateChanged(QtopiaMedia::State)), this, SLOT(playerStateChanged(QtopiaMedia::State)));
        connect(media, SIGNAL(lengthChanged(quint32)), this, SLOT(mediaLengthChanged(quint32)));
        connect(media, SIGNAL(positionChanged(quint32)), this, SLOT(mediaPositionChanged(quint32)));

        if (content() == QMailMessage::VideomailContent) {
            videoPlaceholder->setText(tr("Ready"));
        }
    } else if (id == QMediaVideoControl::name() && !video) {
        video = new QMediaVideoControl(static_cast<QMediaContent*>(sender()));

        if(videoWidget)
        {
            videoWidget->deleteLater();
            videoLayout->removeWidget(videoWidget);
        }

        videoWidget = video->createVideoWidget();
        videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        videoLayout->insertWidget(0, videoWidget);
        videoPlaceholder->hide();
    }
}

void VoicemailViewer::controlValid()
{
    media->disconnect(SIGNAL(valid()));

    media->setVolume(100);
    media->setMuted(false);

    playButton->setEnabled(true);
}

void VoicemailViewer::mediaLengthChanged(quint32 length)
{
    slider->setRange(0, length);
}

void VoicemailViewer::mediaPositionChanged(quint32 ms)
{
    int value = static_cast<int>(ms);
    slider->setValue(value);

    QTime time(0, 0);
    time = time.addMSecs(ms);
    sliderLabel->setText(time.toString("m:ss"));
}

void VoicemailViewer::sliderPressed()
{
    if (media->playerState() == QtopiaMedia::Playing)
        media->pause();
}

void VoicemailViewer::sliderReleased()
{
    media->seek(slider->value());
    if ((media->playerState() == QtopiaMedia::Paused) && playButton->text() == tr("Pause"))
        media->start();
}

void VoicemailViewer::playerStateChanged(QtopiaMedia::State state)
{
    if (state == QtopiaMedia::Stopped) {
        mediaPositionChanged(0);
        playButton->setText(tr("Play"));
        video->deleteLater();
        video = 0;
    }
}

bool VoicemailViewer::eventFilter(QObject*, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Back) {
                emit finished();
                return true;
            }
        }
    }

    return false;
}


QTOPIA_EXPORT_PLUGIN( VoicemailViewerPlugin )

VoicemailViewerPlugin::VoicemailViewerPlugin()
    : QMailViewerPlugin()
{
}

QString VoicemailViewerPlugin::key() const
{
    return "VoicemailViewer";
}

static QList<QMailMessage::ContentType> supportedTypes() 
{
    return QList<QMailMessage::ContentType>() << QMailMessage::VoicemailContent
                                              << QMailMessage::VideomailContent;
}

bool VoicemailViewerPlugin::isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const
{
    if ((pres != QMailViewerFactory::AnyPresentation) && (pres != QMailViewerFactory::StandardPresentation))
        return false;

    static QList<QMailMessage::ContentType> types(supportedTypes());
    return types.contains(type);
}

QMailViewerInterface *VoicemailViewerPlugin::create(QWidget *parent)
{
    return new VoicemailViewer(parent);
}

