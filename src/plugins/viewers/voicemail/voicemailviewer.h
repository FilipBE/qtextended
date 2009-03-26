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

#ifndef VOICEMAILVIEWER_H
#define VOICEMAILVIEWER_H

#include <qmailviewer.h>
#include <qmailviewerplugin.h>

#include <private/homewidgets_p.h>

#include <QContent>
#include <QMailMessage>
#include <QObject>
#include <QString>
#include <QtopiaMedia>

class QAction;
class QBoxLayout;
class QLabel;
class QMediaControl;
class QMediaVideoControl;
class QSlider;
class Browser;

class VoicemailViewer : public QMailViewerInterface
{
    Q_OBJECT

public:
    VoicemailViewer(QWidget* parent = 0);
    virtual ~VoicemailViewer();

    virtual QWidget *widget() const;

public slots:
    virtual bool setMessage(const QMailMessage& mail);
    virtual void clear();

protected slots:
    virtual void senderActivated();
    virtual void playback();

    virtual void mediaError(const QString &);
    virtual void controlAvailable(const QString &);
    virtual void controlValid();
    virtual void mediaLengthChanged(quint32);
    virtual void mediaPositionChanged(quint32);
    virtual void sliderPressed();
    virtual void sliderReleased();
    virtual void playerStateChanged(QtopiaMedia::State);

private:
    bool eventFilter(QObject* watched, QEvent* event);
    bool extractMedia();

    QMailMessage::ContentType content() const;

    void layout();

    QWidget *mainWidget;
    ColumnSizer sizer;
    HomeContactButton *fromButton;
    HomeActionButton *replyButton;
    HomeActionButton *forwardButton;
    HomeActionButton *deleteButton;
    HomeActionButton *backButton;
    HomeActionButton *playButton;
    QLabel *contactLabel;
    QSlider *slider;
    QLabel *sliderLabel;
    QLabel *videoPlaceholder;
    QBoxLayout *videoLayout;
    QMediaControl *media;
    QMediaVideoControl *video;
    QWidget *videoWidget;

    const QMailMessage *message;
    QContact contact;
    QContent recording;
};

class VoicemailViewerPlugin : public QMailViewerPlugin
{
    Q_OBJECT

public:
    VoicemailViewerPlugin();

    virtual QString key() const;
    virtual bool isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const;

    QMailViewerInterface *create(QWidget *parent);
};

#endif
