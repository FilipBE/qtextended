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

#ifndef PHONELAUNCHER_H
#define PHONELAUNCHER_H

#include <QFrame>
#include <qlist.h>
#include <qbasictimer.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#include <qtopiaservices.h>
#include <quniqueid.h>
#include <qvaluespace.h>
#include "qabstractmessagebox.h"
#include "applicationmonitor.h"

#include "qabstractserverinterface.h"

class HomeScreen;
class QAbstractHomeScreen;
class ContextLabel;
class PhoneMainMenu;
class PhoneHeader;
class PhoneBrowser;
class LazyContentStack;
class QCategoryDialog;
class QAbstractMessageBox;
class QLabel;
class QSettings;
class QAction;
class QAbstractCallScreen;
class QAbstractCallHistory;
class CallHistory;
class QExportedBackground;
class QAbstractDialerScreen;
class QAbstractBrowserScreen;
class QAbstractSecondaryDisplay;
class QtopiaServiceDescription;
class QAbstractCallScreen;
class QAbstractHeader;
class QAbstractContextLabel;

class QSpeedDialFeedback : public QFrame {
    Q_OBJECT
public:
    QSpeedDialFeedback();

    void setBlindFeedback(bool on);
    void show(QWidget* center, const QString& input, const QtopiaServiceDescription&);

signals:
    void requestSent();
protected:
    void keyReleaseEvent(QKeyEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void timerEvent(QTimerEvent*);

private:
    QtopiaServiceRequest req;
    QLabel *icon;
    QLabel *label;
    int timerId;
    bool blind;
};

class PhoneLauncher : public QAbstractServerInterface
{
    Q_OBJECT
public:
    PhoneLauncher(QWidget *parent = 0, Qt::WFlags fl = Qt::FramelessWindowHint);
    ~PhoneLauncher();

#ifdef QTOPIA_TELEPHONY
    void showAlertDialogs();

    QAbstractCallScreen *callScreen(bool create = true) const;
    QAbstractDialerScreen *dialer(bool create = true) const;
    QPointer<QAbstractCallHistory> callHistory() const { return mCallHistory; }
#endif
    QAbstractHomeScreen *homeScreen() const;

    QAbstractBrowserScreen* phoneBrowser(bool create = true) const;
    QAbstractSecondaryDisplay *secondaryDisplay(bool create = true) const;

    void hideAll();

public slots:
#ifdef QTOPIA_TELEPHONY
    void showDialer(const QString &, bool speedDial = false);
    void showCallHistory(bool missed = false);
    void showMissedCalls();
    void showCallScreen();
    void showSpeedDialer(const QString &);
#endif
    bool activateSpeedDial( const QString& input );

protected slots:
    void showEvent(QShowEvent *e);
    void sysMessage(const QString& message, const QByteArray&);
    void showHomeScreen(int);
    void showPhoneLauncher();
#ifdef QTOPIA_TELEPHONY
    void missedCount(int);
    void activeCallCount(int);
    void initializeCallHistory();
    void initializeDialerScreen() const;
    void resetDialer();
#endif
    void callPressed();
    void hangupPressed();
    void multitaskPressed();
    void showRunningTasks();
    void keyStateChanged(bool);
protected:
    void showContentSet();
    void initInfo();
    void resizeEvent(QResizeEvent *);
    void timerEvent(QTimerEvent *);

private:
    void raiseMe();
    void layoutViews();

private slots:
    void loadTheme();
#ifdef QTOPIA_TELEPHONY
    void messageBoxDone(int);
    void stateChanged();
    void resetMissedCalls();
#endif
    void speedDialActivated();
    void speedDial( const QString& input );
    void rejectModalDialog();
    void resetView();

private:
    int updateTid;

    QAbstractHeader *header();
    QAbstractHeader *m_header;

    void createContext();
    QAbstractContextLabel *context();
    QAbstractContextLabel *m_context;

    mutable QAbstractBrowserScreen *stack;
    mutable QAbstractHomeScreen *m_homeScreen;
    QBasicTimer multitaskingMultipressTimer;
    int multitaskingcursor;
#ifdef QTOPIA_TELEPHONY
    int activeCalls;
    mutable QAbstractCallScreen *mCallScreen;
    mutable QAbstractDialerScreen *m_dialer;
#endif
    mutable QAbstractSecondaryDisplay *secondDisplay;

#ifdef QTOPIA_TELEPHONY
    QPointer<QAbstractCallHistory> mCallHistory;

    QAbstractMessageBox *missedMsgBox;
    int alertedMissed;
#endif

    QSpeedDialFeedback *speeddialfeedback;
    UIApplicationMonitor appMon;

    bool dialerSpeedDialFeedbackActive;
};

#endif
