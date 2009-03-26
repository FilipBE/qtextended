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

#ifndef SIMAPP_H
#define SIMAPP_H

#include <QMainWindow>
#include <QSimTerminalResponse>
#include <QtopiaAbstractService>

#include "simwidgets.h"

class QStackedWidget;
class QSimCommand;
class QSimToolkit;
class QSimIconReader;
class SimText;
class QPhoneCall;
class QPhoneCallManager;
class QTimer;
class QValueSpaceObject;
class QLabel;

#ifndef QTOPIA_TEST
#include "applicationlauncher.h"
#endif

class SimApp : public QMainWindow
{
    Q_OBJECT
    friend class SimAppService;

public:
    SimApp(QWidget *parent = 0, Qt::WFlags f=0);
    ~SimApp();

    SimCommandView *currentView() const { return view; }
    static QWidget* homescreen();

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void showEvent(QShowEvent *);
    bool eventFilter(QObject *, QEvent *);

private slots:
    void activate();
    void beginFailed();
    void simCommand(const QSimCommand &);
    void simRemoved();
    void updateValueSpace();
    void iconAvailable(int iconId);
    void removeView(QWidget *);
#ifndef QTOPIA_TEST
    void applicationTerminated(const QString &app, ApplicationTypeLauncher::TerminationReason reason, bool filtered);
#endif
    void userActivityOccurred();
    void sendEnvelope(const QSimEnvelope&);
    void sendResponse(const QSimTerminalResponse&);

public slots:
    void terminateSession();
    void hideApp();

signals:
    void viewChanged(SimCommandView *);

private:
    void cmdMenu(const QSimCommand &);
    void cmdDisplayText(const QSimCommand &);
    void cmdInKey(const QSimCommand &);
    void cmdInput(const QSimCommand &);
    void cmdSetupCall(const QSimCommand &);
    void cmdTone(const QSimCommand &);
    void cmdRefresh(const QSimCommand &);
    void cmdChannel(const QSimCommand &);
    void cmdIdleModeText(const QSimCommand &);
    void cmdSetupEventList(const QSimCommand &);
    void cmdLaunchBrowser(const QSimCommand &);
    void showNotification(const QSimCommand &);
    void hideNotification();
    void setView(SimCommandView *);
    bool listViewPreferred(const QSimCommand &);
    void softKeysMenu(const QSimCommand &);

private:
    QPhoneCallManager *callManager;
    QSimToolkit *stk;
    QSimIconReader *iconReader;
    QStackedWidget *stack;
    SimCommandView *view;
    SimText *notification;
    bool hasStk;
    bool simToolkitAvailable;
    QSimCommand idleModeText;
    QImage idleModeImage;
    QString mainMenuTitle;
    QValueSpaceObject *status;
    int eventList;
    QLabel *failLabel;
    bool commandOutsideMenu;
    int idleModeMsgId;
    bool hasSustainedDisplayText;

    void createIconReader();
    void changeEventList(int newEvents);
    void changeUserActivityEvent(bool value);
    void changeIdleScreenEvent(bool value);
    void changeBrowserTerminationEvent(bool value);
};

#endif

