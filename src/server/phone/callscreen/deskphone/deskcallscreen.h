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

#ifndef DESKCALLSCREEN_H
#define DESKCALLSCREEN_H

#include "qabstractcallscreen.h"
#include "ui_deskcallscreen.h"

class DeskphoneCallScreenPrivate;
class CallData;
class QPhoneCall;
class DeskphoneCallData;
class LargeCallerView;

class DeskphoneCallScreen : public QAbstractCallScreen, Ui_DeskphoneCallScreen
{
    Q_OBJECT
public:
    DeskphoneCallScreen(QWidget *parent = 0, Qt::WFlags f = 0);
    ~DeskphoneCallScreen();

    static QString callDurationString(int elapsed, bool showSeconds = true);

public slots:
    virtual void stateChanged();

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);

private slots:
    void callDropped(const QPhoneCall &call);
    void callIncoming(const QPhoneCall &call);
    void callConnected(const QPhoneCall &call);
    void callPutOnHold(const QPhoneCall &call);
    void finishedIncomingCallDialog(int result);
    void checkForIncomingCall();
    void updateDuration();
    void videoStateChanged();
    void endCalls();
    void acceptCall();
    void addCall();
    void toggleVideo();
    void toggleSpeaker();
    void toggleMute();
    void muteChanged();
    void switchWindows();
    void toggleZoom();
    void conference();
    void hold();
    void unhold();
    void swap();
    void join();
    void split();
    void toggleHold();
    void audioStateChanged();
    void publishVideoGeometry();

private:
    void addLargeCallerView(DeskphoneCallData *callData);
    void removeLargeCallerView(DeskphoneCallData *callData);
    void setVideoEnabledLargeCallerView(LargeCallerView *view);
    void updateCallerViewSizes();

    friend class DeskphoneCallScreenPrivate;
    DeskphoneCallScreenPrivate *d;
};

#endif
