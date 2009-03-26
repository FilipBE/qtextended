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
#ifndef CONFERENCEWIDGET_H
#define CONFERENCEWIDGET_H

#include <QWidget>
#include <QMediaRtpSession>

class QBoxLayout;
class QCameraPreviewCapture;
class QLabel;


class ConferenceWidget : public QWidget
{
    Q_OBJECT
public:
    ConferenceWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);

signals:
    void startTelephonyEvent(int event, int volume);
    void stopTelephonyEvent();

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void setCameraOn(bool on);
    void videoInStateChanged(QMediaRtpStream::State state);

private:
    QBoxLayout *createCallLayout(int audioPort, int videoPort);

    QMediaRtpSession m_session;
    QCameraPreviewCapture *m_camera;
    int m_videoConnections;
    int m_depressedKey;
};

#endif
